
#include "idt_handlers.h"
#include "syscall_handlers.h"


#define ENTER_PRESSED               0x1C
#define LEFT_SHIFT_PRESSED          0X2A
#define LEFT_SHIFT_RELEASED         0XAA
#define RIGHT_SHIFT_PRESSED         0X36
#define RIGHT_SHIFT_RELEASED        0XB6
#define CAPS_LOCK_PRESSED           0X3A
#define CAPS_LOCK_RELEASED          0XBA
#define START_RELEASED              0X81
#define START_IGNORE                0X3B
#define LEFT_CONTROL_RELEASED       0X9D
#define RIGHT_CONTROL_TAG           0XE0
#define LEFT_ALT_PRESSED            0X38
#define LEFT_ALT_RELEASED           0XB8
#define L_SCANCODE                  0x26
#define BACKSPACE                   0X0E
#define CAPS_IGNORE_START           0x27
#define IGNORE_BRACKET              0x1A
#define LEFT_CONTROL_PRESSED        0X1D
#define CAPS_IGNORE_END             0x2B
#define LETTER_START                0x10
#define LETTER_END                  0x32
#define LEFT_ARROW_PRESSED          0x4B
#define LEFT_ARROW_RELEASED         0XCB
#define RIGHT_ARROW_PRESSED         0x4D
#define RIGHT_ARROW_RELEASED        0XCD
#define UP_ARROW_PRESSED            0x48
#define UP_ARROW_RELEASED           0xC8
#define DOWN_ARROW_PRESSED          0x50
#define DOWN_ARROW_RELEASED         0xD0
#define FUNCTION_ONE_PRESED         0x3B
#define FUNCTION_TWO_PRESSED        0x3C
#define FUNCTION_THREE_PRESSED      0x3D      

/* enter, left control, 1a 1b*/

// #define END_RELEASED              

/* flags to deal with shift and caps lock */
int shift_on = 0; 
int caps_lock_on = 0;
int control_on = 0;
int alt_on = 0;
int echo_flag = 1;
int distance_from_right = 0;
int total_chars_in_buf = 0;
// keyboard_cursor_idx = 0;
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
    0, 0, // BACKSPACE
    0, 0, // TAB
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
    0, 0, // LEFT CONTROL
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
    0, 0, // LEFT SHIFT
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
    0, 0, // RIGHT SHIFT
    '*', ' ',
    0, 0, // LEFT ALT
    ' ', ' '
};

void reserved() { return; }
void empty()    { return; }

/*  void keyboard_interrupt()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: (should) read which character is printed and print it to screen
*/
void keyboard_interrupt()     
{ 
    // read character --> print to screen
    unsigned int pressed;
    char output_char;
    pressed = inb(0x60);
    // printf("%x",pressed);
    if (pressed == UP_ARROW_PRESSED && shell_flag == 1){
        if (last_buf[0] != '\0'){
            while (distance_from_right > 0){
                right_arrow(); 
                keyboard_cursor_idx++;
                distance_from_right--;
            }
            memcpy(keyboard_buffer, last_buf, last_buf_index);
            int temp_idx = keyboard_cursor_idx + 1;
            while (temp_idx-- > 1) backspace();
            _sys_write_terminal(0, keyboard_buffer, last_buf_index);
            keyboard_cursor_idx = last_buf_index;
            keyboard_buffer_end_idx = keyboard_cursor_idx;
        }
        // return;
    }
    if (pressed == DOWN_ARROW_PRESSED && shell_flag == 1){
        while (distance_from_right > 0){
                right_arrow(); 
                keyboard_cursor_idx++;
                distance_from_right--;
        }
        memset(keyboard_buffer, '\0', KEYBOARD_BUFFER_SIZE);
        memcpy(keyboard_buffer, current_buf, current_buf_index);
        keyboard_cursor_idx++;
        while (keyboard_cursor_idx-- > 1) backspace();
        _sys_write_terminal(0, keyboard_buffer, current_buf_index);
        keyboard_cursor_idx = current_buf_index;
        keyboard_buffer_end_idx = keyboard_cursor_idx;
        // return;
    }
    /* backspace */
    if (pressed == BACKSPACE){
        backspace();
        if (keyboard_cursor_idx > 0){
            keyboard_buffer[keyboard_cursor_idx--] = '\0';
            keyboard_buffer_end_idx--;
            echo_flag = 1;
        }
        return;
    }
    if (pressed == LEFT_ARROW_PRESSED){
        if (keyboard_cursor_idx > 0){
            distance_from_right += left_arrow();
            keyboard_cursor_idx--;
            echo_flag = 1;
        }
        return;
    }
    if (pressed == RIGHT_ARROW_PRESSED && distance_from_right > 0 && keyboard_cursor_idx < KEYBOARD_BUFFER_SIZE - 1){
        int ret_from_right = right_arrow();
        distance_from_right -= ret_from_right;
        keyboard_cursor_idx += ret_from_right;
        echo_flag = 1;
        return;
    }
    /* caps lock */
    if (pressed == CAPS_LOCK_PRESSED){
        caps_lock_on = (caps_lock_on) ? 0 : 1;
        return;
    }
    /* shift pressed */
    if ((pressed == LEFT_SHIFT_PRESSED) || (pressed == RIGHT_SHIFT_PRESSED)){
        shift_on = 1;
        return;
    }
    /* shift released */
    if ((pressed == LEFT_SHIFT_RELEASED) || (pressed == RIGHT_SHIFT_RELEASED)){
        shift_on = 0;
        return;
    }
    /* control pressed */
    if (pressed == LEFT_CONTROL_PRESSED){
        control_on = 1;
        return;
    }
    /* control released */
    if (pressed == LEFT_CONTROL_RELEASED){
        control_on = 0;
        return;
    }
    /* alt pressed */
    if (pressed == LEFT_ALT_PRESSED){
        alt_on = 1;
        return;
    }
    /* alt released */
    if (pressed == LEFT_ALT_RELEASED){
        alt_on = 0;
        return;
    }
    // if (alt_on)
    // {
    //     switch(pressed){
    //         case(FUNCTION_ONE_PRESED):
    //             return /* switch_terminal(1); */
    //         case(FUNCTION_TWO_PRESSED):
    //             return /* switch_terminal(2);*/
    //         case(FUNCTION_THREE_PRESSED):
    //             return /*switch_terminal(3);*/
    //         default:
    //             break;
    //     }
    // }
    /* if we are releasing a key we don't do anything */
    /* ctrl+L clears screen */
    if (control_on && pressed == L_SCANCODE){
        memset(temp_kbd_buf, '\0', KEYBOARD_BUFFER_SIZE);
        memcpy(temp_kbd_buf, keyboard_buffer, keyboard_cursor_idx);
        temp_kbd_idx = keyboard_cursor_idx;
        clear();
        sys_read_flag = 0;
        echo_flag = 1;
        re_echo_flag = 1;
        keyboard_cursor_idx = 0;
        keyboard_buffer_end_idx = 0;
        memset(keyboard_buffer, '\0', KEYBOARD_BUFFER_SIZE);
        return;
    }
    /* if tilde, we want to halt RTC spazzing */
    // if (scancode_to_char[pressed*2] == '`')
    //     RTC_ON_FLAG = (RTC_ON_FLAG) ? 0 : 1;
    if (pressed >= START_RELEASED || pressed == RIGHT_CONTROL_TAG || pressed > START_IGNORE)
        return;
    /* uses the uppercase character in the scancode if shift ^ caps lock is on*/
    if (!caps_lock_on && shift_on)     // shift_on ^ caps_lock_on
        output_char = scancode_to_char[pressed*2+1];
    
    // don't let caps lock affect shift
    else if ((caps_lock_on && !shift_on && pressed >= BACKSPACE) 
            && (pressed >= LETTER_START && pressed <= LETTER_END)
            && !(pressed >= CAPS_IGNORE_START && pressed <= CAPS_IGNORE_END) 
            && !(pressed >= IGNORE_BRACKET && pressed <= LEFT_CONTROL_PRESSED))
                output_char = scancode_to_char[pressed*2+1];
    
    else if ((caps_lock_on && shift_on && pressed < BACKSPACE))
                output_char = scancode_to_char[pressed*2+1];
    
    else        output_char = scancode_to_char[pressed*2]; // else print out the lowercase or unshifted version of the scancode char
    
    /* interaction with _sys_read_terminal */
    if(keyboard_cursor_idx == KEYBOARD_BUFFER_SIZE-1) echo_flag = 0;
    if (keyboard_cursor_idx < KEYBOARD_BUFFER_SIZE-1 && pressed != ENTER_PRESSED){
        keyboard_buffer[keyboard_cursor_idx++] = output_char;
        keyboard_buffer_end_idx++;
        if (distance_from_right > 0) distance_from_right--;
    }
    if(pressed == ENTER_PRESSED){
        if (keyboard_buffer_end_idx < KEYBOARD_BUFFER_SIZE) keyboard_buffer[keyboard_buffer_end_idx++] = output_char;
        keyboard_cursor_idx = keyboard_buffer_end_idx;
        if (shell_flag == 1){
            memset(last_buf, '\0', KEYBOARD_BUFFER_SIZE);
            memcpy(last_buf, keyboard_buffer, keyboard_cursor_idx - 1);
            last_buf_index = keyboard_cursor_idx - 1;
        }
        sys_read_flag = 0;
        keyboard_cursor_idx = 0;
        keyboard_buffer_end_idx = 0;
        echo_flag = 1;
        distance_from_right = 0;
    }
        memset(current_buf, '\0', KEYBOARD_BUFFER_SIZE);
        memcpy(current_buf, keyboard_buffer, keyboard_cursor_idx);
        // current_buf_index = (keyboard_cursor_idx == 0) ? 0 : keyboard_cursor_idx - 1;
        current_buf_index = keyboard_cursor_idx;

    if (echo_flag == 1) putc(output_char);
    wraparound();
    scroll_down();
    
}

/* void rtc_interrupt()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: clears register C in RTC, outputs message to screen
*/
void rtc_interrupt() 
{ 
    // printf("RTC HANDLER\n");
    // if (RTC_ON_FLAG)            test_interrupts();
    if (RTC_ON_FLAG)                 printf("A");
    if (RTC_READ_FLAG) 
    {
        // printf("RTC INTERRUPT\n");
        RTC_READ_FLAG ^= RTC_READ_FLAG;   
    }            
    outb(RTC_STATUS_REGISTER_C, RTC_CMD_PORT); 
    inb(RTC_DATA_PORT); 
}

// void system_call(int call_number, int first_arg, int second_arg, int third_arg)  
// { 
//     printf("System call triggered! \n"); // prints out which exception was triggered
// }

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
void exception_handler(int index)
{
    cli();
    error_flag = 1;
    printf("EXCEPTION: %s \n EXCEPTION #: %d \n", error_messages[index], index); // prints out which exception was triggered
    // sys_halt(index);
    sys_halt(index);
    sti();
    return;
}

