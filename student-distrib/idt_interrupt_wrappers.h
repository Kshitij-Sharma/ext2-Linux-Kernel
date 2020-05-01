/* idt_interrupt_wrappers.h - Defines for various x86 descriptors, descriptor tables,
 * and selectors
@ * vim:ts=4 noexpandtab
 */

#ifndef _IDT_INTERRUPT_WRAPPERS_H
#define _IDT_INTERRUPT_WRAPPERS_H
#ifndef ASM


#include "i8259.h"
#include "lib.h"
#include "idt_handlers.h"

/* calls C function handlers for all 3 of the below */
extern void keyboard_interrupt_asm();
extern void rtc_interrupt_asm();
extern void pit_interrupt_asm();

/* handles the system call and calls the appropriate function */
extern void system_call_asm();


#endif /* _IDT_INTERRUPT_WRAPPERS_H */
#define RTC_CMD_PORT        0x70
#define RTC_DATA_PORT       0x71
#define PIC_COMMAND_PORT    0x20

#define IRQ_SLAVE           2
#define IRQ_RTC             8
#define IRQ_KEYBOARD        1
#define IRQ_PIT             0

#endif
