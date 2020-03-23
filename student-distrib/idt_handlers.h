#include "lib.h"


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
extern void divide_by_zero();
extern void debug();
extern void non_maskable_interrupt();
extern void breakpoint();
extern void into_detected_overflow();
extern void out_of_bounds();
extern void invalid_opcode();
extern void no_coprocessor();
extern void double_fault();
extern void coprocessor_segment_overrun();
extern void bad_tss();
extern void segment_not_present();
extern void stack_fault();
extern void general_protection_fault();
extern void page_fault();
extern void unknown_interrupt();
extern void coprocessor_fault();
extern void alignment_check();
extern void machine_check();
extern void reserved();
extern void pit_interrupt();
extern char keyboard_interrupt();
extern void empty();
extern void rtc_interrupt();
extern void system_call();
extern void SIMD_floating_point();
void exception_handler(int index);
