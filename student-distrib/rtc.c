#include "rtc.h"

/* rtc_init()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: sets RTC rate and initializes it
*/
void rtc_init(){
    // sets the RTC rate to 2Hz
    char prev;
    int rate;
    rate = 0x0F;            
    // MAGIC NUMBER: 0x0F set rate selector bits in register A
    outb(STATUS_REGISTER_A, RTC_CMD_PORT);          // set index to register A, disable NMI
    prev = inb(RTC_DATA_PORT);                      // get initial value of register A
    outb(STATUS_REGISTER_A, RTC_DATA_PORT);         // reset index to A
    outb(((prev & 0xF0) | rate), RTC_DATA_PORT);    // writes rate (bottom 4 bits) to A
    // MAGIC NUMBER: 0xF0 is used to clear top 4 bits before setting rate

}

/* rtc_enable()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: enables periodic interrupts on RTC
*/
void rtc_enable(){
    char prev;
    outb(STATUS_REGISTER_B, RTC_CMD_PORT);              // set index to register B
    prev = inb(RTC_DATA_PORT);                          // get initial value from register B
    outb(STATUS_REGISTER_B, RTC_CMD_PORT);              // reset index to register B
    outb((prev | 0x40), RTC_DATA_PORT);                 // turns on interrupts in RTC
    // MAGIC NUMBER: x040 is used to turn on PIE (periodic interrupt enable) in RTC register B
}

