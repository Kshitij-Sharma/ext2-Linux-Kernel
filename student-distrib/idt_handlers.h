#ifndef _IDT_HANDLERS
#define _IDT_HANDLERS

#include "lib.h"
#include "syscall_handlers.h"
#include "i8259.h"
#include "idt_interrupt_wrappers.h"

/* KEYBOARD SCANCODE TABLE */
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
#define FUNCTION_ONE_PRESSED        0x3B
#define FUNCTION_TWO_PRESSED        0x3C
#define FUNCTION_THREE_PRESSED      0x3D      
#define C_SCANCODE                  0x2E
#define SPECIAL_CHARS_START_1       0x27
#define SPECIAL_CHARS_END_1         0x2B
#define SPECIAL_CHARS_START_2       0x33
#define SPECIAL_CHARS_END_2         0x35
#define SQUARE_BRACKET_OPEN         0x1A
#define SQUARE_BRACKET_CLOSE        0x1B

#define NUM_EXCEPTIONS          20
#define NUM_CODES               116

#define KB_DATA_PORT            0x60 
#define KB_STATUS_REGISTER      0x64        
#define KB_CMD_REGISTER         0x64       
#define RTC_CMD_PORT            0x70
#define RTC_DATA_PORT           0x71
#define RTC_STATUS_REGISTER_A   0X8A
#define RTC_STATUS_REGISTER_B   0X8B
#define RTC_STATUS_REGISTER_C   0X0C
#define PIC_COMMAND_PORT        0x20
#define NUM_TERMINALS           3
/* some interrupt handlers */
/* empty is used for interrupts/exceptions we dont handle and it does nothing */
extern void empty();
/* reserved is used for Intel reserved interrupts/exceptions and does nothing */
extern void reserved();
/* used to handle rtc interrupts */
extern void rtc_interrupt();
/* used to handle keyboard interrupts */
extern void keyboard_interrupt();
/* used to handle pit interrupts  */
extern void pit_interrupt();
/* used to handle exceptions */
void exception_handler(int32_t index);
/* used for testing but doesn't rly do much anymore */
int32_t RTC_ON_FLAG[NUM_TERMINALS];
/* used to handle errors */
uint32_t error_flag[NUM_TERMINALS];


#endif


