#ifndef _RTC_H
#define _RTC_H

#include "lib.h"
// #include <stdio.h>
// #include <math.h>

#define RTC_CMD_PORT            0x70
#define RTC_DATA_PORT           0x71
#define RTC_STATUS_REGISTER_A   0X8A
#define RTC_STATUS_REGISTER_B   0X8B
#define RTC_STATUS_REGISTER_C   0X0C
#define PIC_COMMAND_PORT        0x20
#define FREQ_CONVERSION_CONST   32768
#define RTC_MAX_FREQ            1024

void rtc_init();
void rtc_set_frequency_from_rate(int rate);
// void rtc_
#endif /* _RTC_H */
