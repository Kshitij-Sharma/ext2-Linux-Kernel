
#include "idt_handlers.h"
#include "syscall_handlers.h"
#include "idt_interrupt_wrappers.h"

/* flags to deal with shift, caps lock, control, alt, and flags for arrow key functionality */
int shift_on[NUM_TERMINALS] = {0, 0, 0}; 
int caps_lock_on[NUM_TERMINALS] = {0, 0, 0}; 
int control_on[NUM_TERMINALS] = {0, 0, 0}; 
int alt_on[NUM_TERMINALS] = {0, 0, 0}; 
int echo_flag[NUM_TERMINALS] = {1, 1, 1}; 
int distance_from_right[NUM_TERMINALS] = {0, 0, 0}; 

/* scancodes for lowercase letters */
static char scancode_to_char[NUM_CODES] = {
    0, 0,
    0, 0,
    '1', '!',
    '2', '@',
    '3', '#',
    '4', '$',
    '5', '%',
    '6', '^',
    '7', '&',
    '8', '*',
    '9', '(',
    '0', ')',
    '-', '_',
    '=', '+',
      0, 0, /* BACKSPACE */
      0, 0, /* TAB */
    'q', 'Q',
    'w', 'W',
    'e', 'E',
    'r', 'R',
    't', 'T',
    'y', 'Y',
    'u', 'U',
    'i', 'I',
    'o', 'O',
    'p', 'P',
    '[', '{',
    ']', '}',
    '\n', '\n',
      0, 0, /* LEFT CONTROL */
    'a', 'A',
    's', 'S',
    'd', 'D',
    'f', 'F',
    'g', 'G',
    'h', 'H',
    'j', 'J',
    'k',  'K',
    'l',  'L',
    ';',  ':',
    '\'', '\"',
    '`', '~',
      0, 0, /* LEFT SHIFT */
    '\\', '|',
    'z', 'Z',
    'x', 'X',
    'c', 'C',
    'v', 'V',
    'b', 'B',
    'n', 'N',
    'm', 'M',
    ',', '<',
    '.', '>',
    '/', '?',
      0, 0, /* RIGHT SHIFT */
    '*', ' ',
      0, 0, /* LEFT ALT */
    ' ', ' '
};

/*  void reserved()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: none, just a placeholder
*/
void reserved() { return; }

/**  void reserved()
 *      inputs: none
 *      outputs: none
 *      side effects: none, just a placeholder
*/
void empty()    { return; }

/**  void keyboard_interrupt()
 *      inputs none
 *       outputs: none
 *     side effects: (should) read which character is printed and print it to screen, handles keyboard interrupts
*/
void keyboard_interrupt()     
{ 
    /* read character --> print to screen */
    uint32_t pressed; char output_char;
    /* enable keyboard interrupt flag for putc */
    putc_to_visible_flag = 1;
    /* grabs the scancode of what was pressed */
    pressed = inb(0x60);
    
    /* if we are in shell (not a different user program) and we press up arrow, populate cmd line with previous command */
    if (pressed == UP_ARROW_PRESSED && shell_flag[visible_terminal] == 1)
    {
        /* if the buffer containing the last command is not null */
        if (last_buf[visible_terminal][0] != '\0')
        {

            /** moves the cursor all the way to the right if it is not already there
             * distance_from_right tells us how far from the right the cursor is
             * right_arrow moves the cursor right
             * increments cursor index and decrements distance_from_right */
            while (distance_from_right[visible_terminal] > 0)
            {
                right_arrow(); 
                keyboard_cursor_idx[visible_terminal]++;
                distance_from_right[visible_terminal]--;
            }

            /* clears keyboard buffer and copies the last command into the keyboard buffer */
            memset(keyboard_buffer[visible_terminal], '\0', keyboard_buffer_end_idx[visible_terminal]);
            memcpy(keyboard_buffer[visible_terminal], last_buf[visible_terminal], last_buf_index[visible_terminal]);
            
            /* backspaces everything currently on the screen and then writes the last command on the screen, also updating keyboard index */
            int32_t temp_idx = keyboard_cursor_idx[visible_terminal] + 1;
            while (temp_idx-- > 1) backspace(); 
            _sys_write_terminal(0, keyboard_buffer[visible_terminal], last_buf_index[visible_terminal]);
            
            /* adjusts cursor and buffer*/
            keyboard_cursor_idx[visible_terminal] = last_buf_index[visible_terminal];
            keyboard_buffer_end_idx[visible_terminal] = keyboard_cursor_idx[visible_terminal];
        }
    }
    
    /* if we are in shell (not a different user program) and we press up down, populate cmd line with current command */
    if (pressed == DOWN_ARROW_PRESSED && shell_flag[visible_terminal] == 1)
    {
        /* clears line by moving all the way right and then backspaces all */
        while (distance_from_right[visible_terminal] > 0)
        {
                right_arrow(); 
                keyboard_cursor_idx[visible_terminal]++;
                distance_from_right[visible_terminal]--;
        }
        
        /* fills keyboard buffer */
        memset(keyboard_buffer[visible_terminal], '\0', KEYBOARD_BUFFER_SIZE);
        memcpy(keyboard_buffer[visible_terminal], current_buf[visible_terminal], current_buf_index[visible_terminal]);
        keyboard_cursor_idx[visible_terminal]++;  
        
        /* clear line with backspace */
        while (keyboard_cursor_idx[visible_terminal]-- > 1) backspace();
        
        /* writes buf to terminal */
        _sys_write_terminal(0, keyboard_buffer[visible_terminal], current_buf_index[visible_terminal]);
        keyboard_cursor_idx[visible_terminal] = current_buf_index[visible_terminal];
        keyboard_buffer_end_idx[visible_terminal] = keyboard_cursor_idx[visible_terminal];
    }

    /* backspace */
    if (pressed == BACKSPACE){
        backspace();
        /* if index is not 0, replace the current position with a null and decrement the index */
        if (keyboard_cursor_idx[visible_terminal] > 0){
            keyboard_buffer[visible_terminal][keyboard_cursor_idx[visible_terminal]--] = '\0';
            keyboard_buffer_end_idx[visible_terminal]--;
            echo_flag[visible_terminal] = 1;
        }
        /* handles the case where we backspace when typing things during execution of a program but not for the program*/
        if (typing_during_prog_flag[visible_terminal] == 1 && temp_kbd_idx[visible_terminal] > 0){
            temp_kbd_idx[visible_terminal]--;
            temp_kbd_buf[visible_terminal][temp_kbd_idx[visible_terminal]] = '\0';
        }
        /* turn flag back off since we are returning */
        putc_to_visible_flag = 0;
        return;
    }

    /* left arrow key */
    if (pressed == LEFT_ARROW_PRESSED)
    {
        if (keyboard_cursor_idx[visible_terminal] > 0)
        {
            distance_from_right[visible_terminal] += left_arrow();
            keyboard_cursor_idx[visible_terminal]--;
            echo_flag[visible_terminal] = 1;
        }
        /* turn flag back off since we are returning */
        putc_to_visible_flag = 0;
        return;
    }

    /* right arrow key */
    if (pressed == RIGHT_ARROW_PRESSED && distance_from_right[visible_terminal] > 0 
        && keyboard_cursor_idx[visible_terminal] < KEYBOARD_BUFFER_SIZE - 1)
    {
        /* call right arrow and update cursor */
        int32_t ret_from_right = right_arrow();
        distance_from_right[visible_terminal] -= ret_from_right;
        keyboard_cursor_idx[visible_terminal] += ret_from_right;
        echo_flag[visible_terminal] = 1;
     
        /* turn flag back off since we are returning */
        putc_to_visible_flag = 0;
        return;
    }
    
    /* caps lock */
    if (pressed == CAPS_LOCK_PRESSED)
    { 
        int32_t i;
        /* turns on capslock on all terminals */
        for (i = 0; i < NUM_TERMINALS; i++) 
            caps_lock_on[i] = (caps_lock_on[i]) ? 0 : 1;
        
        /* turn flag back off since we are returning */
        putc_to_visible_flag = 0;
        return;
    }

    /* shift pressed */
    if ((pressed == LEFT_SHIFT_PRESSED) 
        || (pressed == RIGHT_SHIFT_PRESSED))
    {
        int32_t i;
        /* turns on shift on all terminals */
        for (i = 0; i < NUM_TERMINALS; i++) shift_on[i] = 1;
        
        /* turn flag back off since we are returning */
        putc_to_visible_flag = 0;
        return;
    }

    /* shift released */
    if ((pressed == LEFT_SHIFT_RELEASED) 
        || (pressed == RIGHT_SHIFT_RELEASED))
    {
        int32_t i;
        /* turns off shift on all terminals */
        for (i = 0; i < NUM_TERMINALS; i++) shift_on[i] = 0;
        
        /* turn flag back off since we are returning */
        putc_to_visible_flag = 0;
        return;
    }

    /* control pressed */
    if (pressed == LEFT_CONTROL_PRESSED)
    {
        int32_t i;
        /* turns on control on all terminals */
        for (i = 0; i < NUM_TERMINALS; i++) control_on[i] = 1;
        
        /* turn flag back off since we are returning */
        putc_to_visible_flag = 0;
        return;
    }
    /* control released */
    if (pressed == LEFT_CONTROL_RELEASED)
    {
        int32_t i;
        /* turns off conrol on all terminals */
        for (i = 0; i < NUM_TERMINALS; i++) control_on[i] = 0;

        /* turn flag back off since we are returning */
        putc_to_visible_flag = 0;
        return;
    }

    /* alt pressed */
    if (pressed == LEFT_ALT_PRESSED)
    {
        int32_t i;
        /* turns on alt on all terminals */
        for (i = 0; i < NUM_TERMINALS; i++) alt_on[i] = 1;
        
        /* turn flag back off since we are returning */
        putc_to_visible_flag = 0;
        return;
    }
    
    /* alt released */
    if (pressed == LEFT_ALT_RELEASED)
    {
        int32_t i;
        /* turns on shift on all terminals */
        for (i = 0; i < NUM_TERMINALS; i++) alt_on[i] = 0;
        
        /* turn flag back off since we are returning */
        putc_to_visible_flag = 0;
        return;
    }

    /* handles the case where we are trying to switch terminals */
    if (alt_on[visible_terminal])
    {
        /* turn flag back off since we are returning */
        putc_to_visible_flag = 0;
        switch(pressed){
            case(FUNCTION_ONE_PRESSED):
                /* if we are already in the terminal we are trying to switch to, do nothing */
                if (visible_terminal == 0) return;
                switch_terminal(0);
                return;
            case(FUNCTION_TWO_PRESSED):
                /* if we are already in the terminal we are trying to switch to, do nothing */
                if (visible_terminal == 1) return;
                switch_terminal(1);
                return;
            case(FUNCTION_THREE_PRESSED):
                /* if we are already in the terminal we are trying to switch to, do nothing */
                if (visible_terminal == 2) return;
                switch_terminal(2);
                return;
            default:
                /* sets the flag back to 1 if we dont end up returning */
                putc_to_visible_flag = 1;
                break;
        }
    }
    
    /* ctrl+L clears screen and preserves what was typed on the cmd line before the press */  
    if (control_on[visible_terminal] && pressed == L_SCANCODE)
    {
        /* clear our temp buf (this is used to save what was in the kbd buf before we clear) and put the values from kbd buf into it */
        memset(temp_kbd_buf[visible_terminal], '\0', KEYBOARD_BUFFER_SIZE);
        memcpy(temp_kbd_buf[visible_terminal], keyboard_buffer[visible_terminal], keyboard_cursor_idx[visible_terminal]);
        temp_kbd_idx[visible_terminal] = keyboard_cursor_idx[visible_terminal];
        /* clear the screen*/
        clear();
    
        /* return from any terminal reads */
        sys_read_flag[visible_terminal] = 0;
        /* make sure we can still echo */
        echo_flag[visible_terminal] = 1;
        /* tell the program that we need to write the temp buf onto the screen */
        re_echo_flag[visible_terminal] = 1;
    
        /* reset the indexes */
        keyboard_cursor_idx[visible_terminal] = 0;
        keyboard_buffer_end_idx[visible_terminal] = 0;
    
        /* clear keyboard buf */
        memset(keyboard_buffer[visible_terminal], '\0', KEYBOARD_BUFFER_SIZE);
        /* reset this flag to 0 since we are returning */
        putc_to_visible_flag = 0;
        return;
    }
   
    /* if we are releasing a key we don't do anything */
    if (pressed >= START_RELEASED 
        || pressed == RIGHT_CONTROL_TAG 
        || pressed > START_IGNORE)
    {
        putc_to_visible_flag = 0;
        return;
    }

    /* uses the uppercase character in the scancode if shift ^ caps lock is on */
    if (!caps_lock_on[visible_terminal] && shift_on[visible_terminal])    
        output_char = scancode_to_char[pressed*2+1];
    
    /* don't let caps lock affect shift */
    else if ((caps_lock_on[visible_terminal] 
            && !shift_on[visible_terminal] && pressed >= BACKSPACE) 
            && (pressed >= LETTER_START && pressed <= LETTER_END)
            && !(pressed >= CAPS_IGNORE_START && pressed <= CAPS_IGNORE_END) 
            && !(pressed >= IGNORE_BRACKET && pressed <= LEFT_CONTROL_PRESSED))
                output_char = scancode_to_char[pressed*2+1];

    /* else if caps is on and shift is pressed, print out the lowercase or unshifted version of the scancode char */
    else if ((caps_lock_on[visible_terminal] && shift_on[visible_terminal] && pressed < BACKSPACE))
                output_char = scancode_to_char[pressed*2+1];

    /* else print out the lowercase or unshifted version of the scancode char */
    else        output_char = scancode_to_char[pressed*2]; 
    
    
    /* interaction with _sys_read_terminal */
    /* if the buffer is full, dont echo anything to screen and don't store it in the buffer */
    if(keyboard_cursor_idx[visible_terminal] == KEYBOARD_BUFFER_SIZE-1) 
        echo_flag[visible_terminal] = 0;
    
    /* if there is still space in the buffer to type, put the char int the buffer, increment index */
    if (keyboard_cursor_idx[visible_terminal] < KEYBOARD_BUFFER_SIZE-1 
        && pressed != ENTER_PRESSED)
    {
        keyboard_buffer[visible_terminal][keyboard_cursor_idx[visible_terminal]++] = output_char;
        keyboard_buffer_end_idx[visible_terminal]++;
        /* if distance from right isn't 0, decrease it */
        if (distance_from_right[visible_terminal] > 0) 
            distance_from_right[visible_terminal]--;
    }

    /* case we press enter */
    if(pressed == ENTER_PRESSED){
        /* if there is still room in the buffer to type, increment index and put the char in the buffer */
        if (keyboard_buffer_end_idx[visible_terminal] < KEYBOARD_BUFFER_SIZE) 
            keyboard_buffer[visible_terminal][keyboard_buffer_end_idx[visible_terminal]++] = output_char;
        
        /* update index of cursor -- used for arrow key presses*/
        keyboard_cursor_idx[visible_terminal] = keyboard_buffer_end_idx[visible_terminal];
        
        /* if we are in shell, store the command into the last_buf for use with up arrow key */
        if (shell_flag[visible_terminal] == 1)
        {
            memset(last_buf[visible_terminal], '\0', KEYBOARD_BUFFER_SIZE);
            memcpy(last_buf[visible_terminal], keyboard_buffer[visible_terminal], keyboard_cursor_idx[visible_terminal] - 1);
            last_buf_index[visible_terminal] = keyboard_cursor_idx[visible_terminal] - 1;
        }
        
        /* set sys_read_flag to 0 so that we can break out of the while loop there*/
        sys_read_flag[visible_terminal] = 0;
        /* reset all indexes, distance_from_right, and echo flag */
        keyboard_cursor_idx[visible_terminal] = 0;
        keyboard_buffer_end_idx[visible_terminal] = 0;
        echo_flag[visible_terminal] = 1;
        distance_from_right[visible_terminal] = 0;
    }

    /* clear current buf and store what was typed so far into it -- happens with every key press that is echoed, used for down arrow key press */
        memset(current_buf[visible_terminal], '\0', KEYBOARD_BUFFER_SIZE);
        memcpy(current_buf[visible_terminal], keyboard_buffer[visible_terminal], keyboard_cursor_idx[visible_terminal]);
        current_buf_index[visible_terminal] = keyboard_cursor_idx[visible_terminal];
    
    /* if echo flag is on, print char to screen */
    if (echo_flag[visible_terminal] == 1) putc(output_char);

    if (shell_flag[visible_terminal] != 1 && temp_kbd_idx[visible_terminal] < KEYBOARD_BUFFER_SIZE && output_char != '\n' && sys_read_flag[visible_terminal] != 1) {
    temp_kbd_buf[visible_terminal][temp_kbd_idx[visible_terminal]++] = output_char;
    typing_during_prog_flag[visible_terminal] = 1;
    }
    /* these are pretty self explanatory */
    wraparound();
    scroll_down();
    /* turn this flag back off since we are exiting the keyboard interrupt */
    putc_to_visible_flag = 0;
}

/* void rtc_interrupt()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: clears register C in RTC, outputs message to screen
*/
void rtc_interrupt() 
{ 
    /* if the counter is at 0, reset it, otherwise decrement it */
    if (cur_pcb_ptr[process_terminal] != NULL && cur_pcb_ptr[process_terminal]->rtc_counter == 0) cur_pcb_ptr[process_terminal]->rtc_counter = cur_pcb_ptr[process_terminal]->rtc_interrupt_divider;
    else if (cur_pcb_ptr[process_terminal] != NULL) cur_pcb_ptr[process_terminal]->rtc_counter--;
    
    /* clears register C so RTC interrupts can continue happening*/
    outb(RTC_STATUS_REGISTER_C, RTC_CMD_PORT); 
    inb(RTC_DATA_PORT); 
}


/* Array of error messages in order so we can index into them based on the argument of the function call*/
char * error_messages[NUM_EXCEPTIONS] = {
    "Dividing by zero is illegal in thirteen countries.", 
    "Debug exception.", 
    "Non-maskable interrupt exception.",
    "Breakpoint exception.", 
    "Into detected overflow exception.", 
    "Out of bounds exception.", 
    "Invalid opcode.",
    "No coprocessor exception.", 
    "Double fault exception.", 
    "Coprocessor segment overrun.", 
    "Bad TSS.", 
    "Segment not present.", 
    "Stack fault.", 
    "General protection fault.", 
    "Page fault.", 
    "Unknown interrupt!!!!!!!!", 
    "Coprocessor fault.", 
    "Alignment check.", 
    "Machine check.",
    "SMD floating point exception."
    };

/* void exception handler
        INPUTS: index of the exception in IDT
        OUTPUTS: none
        SIDE-EFFECTS: prints which exception happened, freezes screen
*/
void exception_handler(int32_t index)
{
    /* mask interrupts */
    cli();
    /* turn on error flag to tell halt to return control back to user */
    error_flag[process_terminal] = 1;
    /* print error message */
    printf("EXCEPTION %d: %s\n", index, error_messages[index]); 
    /* halts program */
    sys_halt(index);
    return;
}

