/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */
#include "multiprocessing.h"
#include "lib.h"
// #include "syscall_handlers.h"

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x7
#define CURSOR_COMMAND_PORT     0x3D4
#define CURSOR_DATA_PORT        0X3D5
#define HALF_BYTE_MASK          0X0F
#define FULL_BYTE_MASK          0XFF
#define BYTE_SHIFT              0X8

// static uint32_t video[NUM_TERMINAL] = {TERMINAL_ONE_BUFFER, TERMINAL_TWO_BUFFER, TERMINAL_THREE_BUFFER};

/** log_base_two
 *  
 * Inputs: integer to take log of
 * Outputs: log of number passed in   
 * Side Effects: 
 */
static int screen_x[NUM_TERMINALS] = {0, 0, 0};
static int screen_y[NUM_TERMINALS] = {0, 0, 0};
static char* video_mem = (char *)VIDEO;
int forward_next[NUM_TERMINALS] = {0, 0, 0};       // global var used for wraparound/backspace corner case
int backward_next[NUM_TERMINALS] = {0, 0, 0};
char* video_buf[3]= {(char *) TERMINAL_ONE_BUFFER, (char *)TERMINAL_TWO_BUFFER, (char *)TERMINAL_THREE_BUFFER};


// void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
// {
//  outb(0x0A, 0x3D4);
//  outb((inb(0x3D5) & 0xC0) | cursor_start, 0x3D5);
 
//  outb(0x0B, 0x3D4);
//  outb((inb(0x3D5) & 0xE0) | cursor_end, 0x3D5);
// }

// void disable_cursor()
// {
//  outb(0x0A, 0x3D4);
//  outb(0x20, 0x3D5);
// }

/* void modify_vid_mem
 * Inputs: Address of terminal to be dispalyed
 * Return Value: none
 * Function: Change location of video memory */
void modify_vid_mem(uint32_t address){
    video_mem = (char *) address;
}

/* void backspace(void);
 * Inputs: void
 * Return Value: none
 * Function: Backspace functionality */
int right_arrow(void){
    if ((screen_x[visible_terminal] == NUM_COLS) && (screen_y[visible_terminal] == NUM_ROWS - 1)) return 0;
    if (keyboard_cursor_idx[visible_terminal] == KEYBOARD_BUFFER_SIZE - 1){
        return 0;
    }
    if (screen_x[visible_terminal] != NUM_COLS){
        // moves cursor back
        screen_x[visible_terminal]++;
        // deletes char at cursor
        forward_next[visible_terminal] = screen_x[visible_terminal] + 1;
        backward_next[visible_terminal] = screen_x[visible_terminal] - 1;
        if (screen_x[visible_terminal] == 80) {
            backward_next[visible_terminal] = 0;
            forward_next[visible_terminal] = 79;
        }
        update_cursor();
        return 1;
    }
    // case where we go up a line
    if (screen_x[visible_terminal] == NUM_COLS && screen_y[visible_terminal] < NUM_ROWS - 1){
        if (*(uint8_t *)(VIDEO + ((NUM_COLS * (screen_y[visible_terminal] + 1) + NUM_COLS-1) << 1)) == '\n') return 0;
        if (backward_next == 0){
            screen_x[visible_terminal] = 0;
            backward_next[visible_terminal] = NUM_COLS;
            forward_next[visible_terminal] = 1;
        }
        screen_y[visible_terminal]++;
        // move cursor up
        update_cursor();
        return 1;
    }
    update_cursor();
    return 0;
}



/* void backspace(void);
 * Inputs: void
 * Return Value: none
 * Function: Backspace functionality */
int left_arrow(void){
    if (screen_x[visible_terminal] == 0 && screen_y[visible_terminal] == 0) return 0;
    if (keyboard_cursor_idx[visible_terminal] == 0){
        return 0;
    }
    if (screen_x[visible_terminal] != 0){
        // moves cursor back
        screen_x[visible_terminal]--;
        // deletes char at cursor
        forward_next[visible_terminal] = screen_x[visible_terminal] + 1;
        backward_next[visible_terminal] = screen_x[visible_terminal] - 1;
        if (screen_x[visible_terminal] == 0) {
            backward_next[visible_terminal] = 80;
            forward_next[visible_terminal] = 1;
        }
        update_cursor();
        return 1;
    }
    // case where we go up a line
    if (screen_x[visible_terminal] == 0 && screen_y[visible_terminal] != 0){
        if (*(uint8_t *)(VIDEO + ((NUM_COLS * (screen_y[visible_terminal] - 1) + NUM_COLS-1) << 1)) == '\n') return 0;
        if (backward_next[visible_terminal] == 80){
            screen_x[visible_terminal] = NUM_COLS-1;
            backward_next[visible_terminal] = 79;
            forward_next[visible_terminal] = 0;
        }
        else{
            screen_x[visible_terminal] = NUM_COLS-1;
            forward_next[visible_terminal] = 80;
            backward_next[visible_terminal] = 78;
        }
        screen_y[visible_terminal]--;
        // move cursor up
        update_cursor();
        return 1;
    }
    update_cursor();
    return 0;
}


/* void update_cursor(void);
 * Inputs: void
 * Return Value: none
 * Function: Moves cursor to screen_x and screen_y  */
void update_cursor() // *************************MAKE SURE YOU CHECK CONSTANT NAMES, FIX MAGIC NUMBERS, AND COMMENT 
{
    uint16_t pos = screen_y[visible_terminal] * NUM_COLS + screen_x[visible_terminal];
 
    outb(HALF_BYTE_MASK, CURSOR_COMMAND_PORT);
    outb((uint8_t) (pos & FULL_BYTE_MASK), CURSOR_DATA_PORT);
    outb(0x0E, CURSOR_COMMAND_PORT);
    outb((uint8_t) ((pos >> BYTE_SHIFT) & FULL_BYTE_MASK), CURSOR_DATA_PORT);
}




/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(VIDEO + (i << 1)) = '\0';
        *(uint8_t *)(VIDEO + (i << 1) + 1) = ATTRIB;
    }
    screen_y[visible_terminal] = 0;
    screen_x[visible_terminal] = 0;
    memset(keyboard_buffer, NULL, KEYBOARD_BUFFER_SIZE);
    update_cursor();
}

/* void scroll_down(void);
 * Inputs: void
 * Return Value: none
 * Function: Scrolls the page down */
void scroll_down(void) {
    int term = (putc_to_visible_flag == 0) ? process_terminal : visible_terminal;
    // printf("%d %d", screen_x, screen_y);
    if (screen_y[term] >= NUM_ROWS){
        int32_t i;
        /* move text on screen up */
        for (i = NUM_COLS; i < NUM_ROWS * NUM_COLS; i++) {
            *(uint8_t *)(video_buf[term] + ((i-NUM_COLS) << 1)) =  *(uint8_t *)(video_buf[term] + (i << 1));
            *(uint8_t *)(video_buf[term] + (i << 1) + 1) = ATTRIB;
            if (visible_terminal == term){
                *(uint8_t *)(VIDEO + ((i-NUM_COLS) << 1)) =  *(uint8_t *)(VIDEO + (i << 1));
                *(uint8_t *)(VIDEO + (i << 1) + 1) = ATTRIB;
            }
        }
        /* fill in the bottom row with blanks */
        for (i = (NUM_ROWS-1) * NUM_COLS; i < NUM_ROWS*NUM_COLS; i++){
            *(uint8_t *)(video_buf[term] + (i << 1)) = '\0';
            *(uint8_t *)(video_buf[term] + (i << 1) + 1) = ATTRIB;
            if (visible_terminal == term){
                *(uint8_t *)(VIDEO + (i << 1)) = '\0';
                *(uint8_t *)(VIDEO + (i << 1) + 1) = ATTRIB;
            }
        }
        screen_y[term] = NUM_ROWS-1;
        screen_x[term] = 0;
    }
}

/* void clear_line(void);
 * Inputs: void
 * Return Value: none
 * Function: clears a line of text  */
// void clear_line(void) {
//     int i;
//     screen_x = NUM_COLS;
//     for(i = 0; i <= NUM_COLS; i++)      backspace();
// }


/* void wraparound(void);
 * Inputs: void
 * Return Value: none
 * Function: wraps curos around to next  */
void wraparound(void) {
    int term = (putc_to_visible_flag == 0) ? process_terminal : visible_terminal;
    if (forward_next[term] == 80){
        forward_next[term] = 0;
    }
    if (screen_x[term] == NUM_COLS - 1)       forward_next[term] = 80;
    if(screen_x[term] == 0 && forward_next[term] == 0)
    {
        screen_y[term]++;
        backward_next[term] = 80;
        forward_next[term] = 1;
    }
}

/* void backspace(void);
 * Inputs: void
 * Return Value: none
 * Function: Backspace functionality */
void backspace(void){
    if (screen_x[visible_terminal] == 0 && screen_y[visible_terminal] == 0) return;
    if (keyboard_cursor_idx[visible_terminal] == 0){
        return;
    }
    if (screen_x[visible_terminal] != 0){
        // moves cursor back
        screen_x[visible_terminal]--;
        // deletes char at cursor
        *(uint8_t *)(VIDEO + ((NUM_COLS * screen_y[visible_terminal] + screen_x[visible_terminal]) << 1)) = ' ';
        *(uint8_t *)(VIDEO + ((NUM_COLS * screen_y[visible_terminal] + screen_x[visible_terminal]) << 1) + 1) = ATTRIB;
        forward_next[visible_terminal] = screen_x[visible_terminal] + 1;
        backward_next[visible_terminal] = screen_x[visible_terminal] - 1;
        if (screen_x[visible_terminal] == 0) {
            backward_next[visible_terminal] = 80;
            forward_next[visible_terminal] = 1;
        }
        update_cursor();
        return;
    }
    // case where we go up a line
    if (screen_x[visible_terminal] == 0 && screen_y[visible_terminal] != 0){
        if (*(uint8_t *)(VIDEO + ((NUM_COLS * (screen_y[visible_terminal] - 1) + NUM_COLS-1) << 1)) == '\n') return;
        if (backward_next[visible_terminal] == 80){
            screen_x[visible_terminal] = NUM_COLS-1;
            backward_next[visible_terminal] = 79;
            forward_next[visible_terminal] = 0;
        }
        else{
            screen_x[visible_terminal] = NUM_COLS-1;
            forward_next[visible_terminal] = 80;
            backward_next[visible_terminal] = 78;
        }
        // delete last char in prev row
        // *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = ' ';
        // *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        *(uint8_t *)(VIDEO + ((NUM_COLS * (screen_y[visible_terminal] - 1) + screen_x[visible_terminal]) << 1)) = ' ';
        *(uint8_t *)(VIDEO + ((NUM_COLS * (screen_y[visible_terminal] - 1) + screen_x[visible_terminal]) << 1) + 1) = ATTRIB;
        screen_y[visible_terminal]--;
        // move cursor up
        update_cursor();
        return;
    }
    update_cursor();
    return;
}

/* void log_base_two(int n);
 * Inputs: n: number to calculate log of
 * Return Value: log of n 
 * Function: Calculates log base 2 of n */
int log_base_two(int n) 
{ 
    return (n > 1) ? 1 + log_base_two(n / 2) : 0; 
} 
/* void power_of_two(int num);
 * Inputs: num: int to check if it is power of two
 * Return Value: 0 for success, 1 for failure
 * Function: Checks for power of two  */
int power_of_two(int num){
    if (num == 0) return 1;
    while (num != 1){
        if (num % 2 == 1) return 1;
        num /= 2;
    }
    return 0;
}
/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}

/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console */
void putc(uint8_t c) {
    int term = (putc_to_visible_flag == 0) ? process_terminal : visible_terminal;
    if(c == '\n' || c == '\r') {
        screen_y[term]++;
        screen_x[term] = 0;
    } else { /* write info to the respective buffer of each process */
            *(uint8_t *)(video_buf[term] + ((NUM_COLS * screen_y[term] + screen_x[term]) << 1)) = c;
            *(uint8_t *)(video_buf[term] + ((NUM_COLS * screen_y[term] + screen_x[term]) << 1) + 1) = ATTRIB;
        if(visible_terminal == term){ /* If the current process is on screen, write it to video memory */
            *(uint8_t *)(VIDEO + ((NUM_COLS * screen_y[term] + screen_x[term]) << 1)) = c;
            *(uint8_t *)(VIDEO + ((NUM_COLS * screen_y[term] + screen_x[term]) << 1) + 1) = ATTRIB;
         }
            screen_x[term]++;
            screen_x[term] %= (NUM_COLS);
            screen_y[term] = (screen_y[term] + (screen_x[term] / NUM_COLS)) % NUM_ROWS;

    }
    wraparound();
    scroll_down();
    update_cursor();
}

/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}
