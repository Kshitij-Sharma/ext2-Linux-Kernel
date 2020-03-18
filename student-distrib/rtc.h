#ifndef _RTC_H
#define _RTC_H

#include "lib.h"

#define RTC_CMD_PORT        0x70
#define RTC_DATA_PORT       0x71
#define STATUS_REGISTER_A   0X8A
#define STATUS_REGISTER_B   0X8B
#define STATUS_REGISTER_C   0X0C
#define PIC_COMMAND_PORT    0x20

void rtc_init();
void rtc_enable();
// void rtc_
#endif /* _RTC_H */
