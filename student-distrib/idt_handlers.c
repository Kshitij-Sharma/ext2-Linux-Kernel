
#include "idt_handlers.h"

#define ENTER_PRESSED               0x1C
#define LEFT_SHIFT_PRESSED          0X2A
#define LEFT_SHIFT_RELEASED         0XAA
#define RIGHT_SHIFT_PRESSED         0X36
#define RIGHT_SHIFT_RELEASED        0XB6
#define CAPS_LOCK_PRESSED           0X3A
#define CAPS_LOCK_RELEASED          0XBA
#define START_RELEASED              0X81
#define LEFT_CONTROL_PRESSED        0X1D
#define LEFT_CONTROL_RELEASED       0X9D
#define RIGHT_CONTROL_TAG           0XE0
#define LEFT_ALT_PRESSED            0X38
#define LEFT_ALT_RELEASED           0XB8
#define L_SCANCODE                  0x26
#define BACKSPACE                   0X0E

// #define END_RELEASED              

/* flags to deal with shift and caps lock */
int shift_on = 0; 
int caps_lock_on = 0;
int control_on = 0;
int alt_on = 0;
int RTC_ON_FLAG = 0;
int keyboard_buffer_idx = 0;
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
    /* backspace */
    if (pressed == BACKSPACE){
        backspace();
        keyboard_buffer[keyboard_buffer_idx--] = ' ';
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
    if ((pressed == LEFT_CONTROL_PRESSED)){
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
    /* if we are releasing a key we don't do anything */
    if (pressed >= START_RELEASED || pressed == RIGHT_CONTROL_TAG)
        return;
    /* ctrl+L clears screen */
    if (control_on && pressed == L_SCANCODE){
        clear();
        return;
    }
    /* if tilde, we want to halt RTC spazzing */
    if (scancode_to_char[pressed] == '`')
        RTC_ON_FLAG = (RTC_ON_FLAG) ? 0 : 1;
    /* uses the uppercase character in the scancode if shift ^ caps lock is on*/
    if ((caps_lock_on && !shift_on) || (!caps_lock_on && shift_on)){    // shift_on ^ caps_lock_on
        output_char = scancode_to_char[pressed*2+1];
    }
    else{
        output_char = scancode_to_char[pressed*2];
    }
    if(sys_read_flag)
    {
        if (keyboard_buffer_idx < KEYBOARD_BUFFER_SIZE){
            keyboard_buffer[keyboard_buffer_idx] = output_char;
            keyboard_buffer_idx++;
        }
        if(keyboard_buffer_idx == KEYBOARD_BUFFER_SIZE-1 || pressed == ENTER_PRESSED){
            sys_read_flag = 0;
            keyboard_buffer_idx = 0;
        }
    }
    putc(output_char);
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
    if (RTC_ON_FLAG)            test_interrupts();
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
    printf("EXCEPTION: %s \n EXCEPTION #: %d \n", error_messages[index], index); // prints out which exception was triggered
    while(1); // holds the program in an indefinite loop due to the exception
    sti();
    return;
}

