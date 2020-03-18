/* idt_interrupt_wrappers.h - Defines for various x86 descriptors, descriptor tables,
 * and selectors
 * vim:ts=4 noexpandtab
 */

#ifndef _IDT_INTERRUPT_WRAPPERS_H
#define _IDT_INTERRUPT_WRAPPERS_H
#ifndef ASM


// #include "types.h"
#include "i8259.h"
#include "lib.h"
#include "idt_handlers.h"




extern void divide_by_zero_test();
extern void keyboard_interrupt_asm();
extern void rtc_interrupt_asm();


#endif /* _IDT_INTERRUPT_WRAPPERS_H */
#define RTC_CMD_PORT        0x70
#define RTC_DATA_PORT       0x71
#define PIC_COMMAND_PORT    0x20
#define RTC_IRQ             8

#endif
