#ifndef _IDT_HANDLERS
#define _IDT_HANDLERS
#include "lib.h"
#include "syscall_handlers.h"
#include "i8259.h"
// #include "syscall_handlers.h"


#define RTC_CMD_PORT            0x70
#define RTC_DATA_PORT           0x71
#define RTC_STATUS_REGISTER_A   0X8A
#define RTC_STATUS_REGISTER_B   0X8B
#define RTC_STATUS_REGISTER_C   0X0C
#define PIC_COMMAND_PORT        0x20
#define NUM_EXCEPTIONS          20
#define KB_DATA_PORT            0x60 
#define KB_STATUS_REGISTER      0x64        // read access
#define KB_CMD_REGISTER         0x64        // write access
#define NUM_CODES               116
// extern void keyboard_interrupt();
extern void empty();
extern void reserved();
extern void rtc_interrupt();
// extern void system_call(int call_number, int first_arg, int second_arg, int third_arg);
extern void SIMD_floating_point();
char temp_kbd_buf[KEYBOARD_BUFFER_SIZE];
int re_echo_flag;

void exception_handler(int index);
int RTC_ON_FLAG;
volatile int RTC_READ_FLAG;
uint32_t error_flag;
uint32_t ctrl_l_flag;
#endif


