#ifndef _RTC_H
#define _RTC_H

#include "lib.h"
#include "idt_handlers.h"

#define RTC_CMD_PORT            0x70
#define RTC_DATA_PORT           0x71
#define RTC_STATUS_REGISTER_A   0X8A
#define RTC_STATUS_REGISTER_B   0X8B
#define RTC_STATUS_REGISTER_C   0X0C
#define PIC_COMMAND_PORT        0x20
#define FREQ_CONVERSION_CONST   32768
#define RTC_MAX_FREQ            512
#define RTC_MAX_RATE            0x7
#define PIT_DATA_PORT_ZERO      0X40
#define PIT_COMMAND_PORT        0X43
#define PIT_FREQ_DIVISOR        1193180

/* initialize RTC */
void rtc_init();
/* initialize PIT */
void pit_init(double frequency);

/* used to block scheduling from happening till we want it to */
int32_t pit_flag;


#endif /* _RTC_H */
