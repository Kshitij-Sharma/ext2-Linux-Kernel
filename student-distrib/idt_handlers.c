
#include "idt_handlers.h"

/* scancodes for lowercase letters */
static char scancode_to_char[NUM_CODES/2] = {
    0, 
    0, 
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=', 
    0,
    0,
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n',
    0,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k', 
    'l', 
    ';', 
    '\'',
    '`',
    0, 
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0, 
    '*',
    0,
    ' '
};

int RTC_ON_FLAG = 0;

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
    // inb KBDR (if my 220 knowledge is correct)
    char pressed;
    pressed = inb(0x60);
    /* if tilde, we want to halt RTC spazzing */
    if (scancode_to_char[(unsigned)pressed] == '`')
        RTC_ON_FLAG = (RTC_ON_FLAG) ? 0 : 1;
    printf("%c", scancode_to_char[(unsigned)pressed]);
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

void system_call()  
{ 
    printf("System call triggered! \n"); // prints out which exception was triggered
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
void exception_handler(int index)
{
    cli();
    printf("EXCEPTION: %s \n EXCEPTION #: %d \n", error_messages[index], index); // prints out which exception was triggered
    while(1); // holds the program in an indefinite loop due to the exception
    sti();
    return;
}

