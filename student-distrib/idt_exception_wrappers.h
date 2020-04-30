#ifndef _IDT_EXCEPTION_WRAPPERS_H
#define _IDT_EXCEPTION_WRAPPERS_H
#ifndef ASM


#include "i8259.h"
#include "lib.h"
#include "idt_handlers.h"

/* handlers for each of the exceptions -- names are pretty self explanatory*/
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
extern void SIMD_floating_point();

#endif
#endif
