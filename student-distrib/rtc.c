#include "rtc.h"

void rtc_init(){
    // cli();
    // outb(STATUS_REGISTER_A, RTC_CMD_PORT);
    // outb(PIC_COMMAND_PORT, RTC_DATA_PORT);
    // sti();
}

/* turns on RTC periodic interrupts */
void rtc_enable(){
    unsigned char prev;
    // cli();                                      // disable interrupts
    outb(STATUS_REGISTER_B, RTC_CMD_PORT);   
    prev = inb(RTC_DATA_PORT);
    outb(STATUS_REGISTER_B, RTC_CMD_PORT);
    outb((prev | 0x40), RTC_DATA_PORT);
    // sti();
}

