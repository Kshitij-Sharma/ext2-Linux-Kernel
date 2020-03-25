#include "lib.h"
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
#define KEYBOARD_BUFFER_SIZE    128
// extern void keyboard_interrupt();
extern void empty();
extern void reserved();
extern void rtc_interrupt();
// extern void system_call(int call_number, int first_arg, int second_arg, int third_arg);
extern void SIMD_floating_point();

void exception_handler(int index);
char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
volatile int sys_read_flag;
