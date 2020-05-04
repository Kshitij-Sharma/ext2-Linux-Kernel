#include "rtc.h"

/* rtc_init()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: enables periodic interrupts on RTC, sets frequency to 512 Hz by default
*/
void rtc_init(){
    char prev;
    int32_t i;
    for (i = 0; i < NUM_TERMINALS; i++){
        re_echo_flag[i] = 0;
        RTC_ON_FLAG[i] = 0;
    }
    outb(RTC_STATUS_REGISTER_B, RTC_CMD_PORT);          /* set index to register B */
    prev = inb(RTC_DATA_PORT);                          /* get initial value from register B */
    outb(RTC_STATUS_REGISTER_B, RTC_CMD_PORT);          /* reset index to register B */
    outb((prev | 0x40), RTC_DATA_PORT);                 /* turns on interrupts in RTC */
    /* MAGIC NUMBER: x040 is used to turn on PIE (periodic interrupt enable) in RTC register B */

    /* VIRTUALIZATION: STATICALLY SET FREQUENCY TO 512Hz*/
    /* MAGIC NUMBER: 0x0F sets rate selector bits in register A */
    outb(RTC_STATUS_REGISTER_A, RTC_CMD_PORT);              /* set index to register A, disable NMI */
    prev = inb(RTC_DATA_PORT);                              /* get initial value of register A */
    outb(RTC_STATUS_REGISTER_A, RTC_CMD_PORT);              /* reset index to A */
    outb(((prev & 0xF0) | RTC_MAX_RATE), RTC_DATA_PORT);    /* writes rate (bottom 4 bits) to A */
    /* MAGIC NUMBER: 0xF0 is used to clear bottom 4 bits before setting rate */
}

/* pit_init()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: enables periodic interrupts on RTC
*/
void pit_init(double frequency){

    int32_t divisor = (int)(PIT_FREQ_DIVISOR / frequency);
    outb(PIT_INIT_CONSTANT, PIT_COMMAND_PORT);
    /* send the lower byte to the data port (the FF is to grab that first byte) */
    outb(divisor & 0xFF, PIT_DATA_PORT_ZERO);
    /* send the upper byte to the data port (the 8 is to move the byte down to the first 8 bits) */
    outb(divisor >> 8, PIT_DATA_PORT_ZERO);
    /* enables PIC line 0 */
    enable_irq(IRQ_PIT);
}
