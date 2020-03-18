/* idt_interrupt_handlers.h - Defines for various x86 descriptors, descriptor tables,
 * and selectors
 * vim:ts=4 noexpandtab
 */

#ifndef _IDT_INTERRUPT_HANDLERS_H
#define _IDT_INTERRUPT_HANDLERS_H
#ifndef ASM


// #include "types.h"
#include "i8259.h"
#include "lib.h"
#include "idt_entry_functions.h"


// #ifndef _IDT_INTERRUPT_HANDLERS_H
// #define _IDT_INTERRUPT_HANDLERS_H

extern void divide_by_zero_test();
extern void keyboard_interrupt_asm();
extern void rtc_interrupt_asm();


#endif /* _IDT_INTERRUPT_HANDLERS_H */
#define RTC_CMD_PORT        0x70
#define RTC_DATA_PORT       0x71
// #define STATUS_REGISTER_A   0x8A
// #define STATUS_REGISTER_B   0x8B
// #define STATUS_REGISTER_C   0x0C
#define PIC_COMMAND_PORT    0x20
#define RTC_IRQ             8

#endif
