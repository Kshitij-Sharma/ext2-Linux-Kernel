#ifndef _IDT_H
#define _IDT_H

#include "idt_handlers.h"
#include "x86_desc.h"
#include "idt_interrupt_wrappers.h"
#include "idt_exception_wrappers.h"
#include "lib.h"



#define FIRST_INTERRUPT_IDX     0x20
#define LAST_INTERRUPT_IDX      0x2F
#define KB_INTERRUPT_IDX        0x21
#define RTC_INTERRUPT_IDX       0x28
#define SYSTEM_CALL_IDX         0x80
#define OTHER_INTERRUPTS_IDX    0x30



void idt_init();

#endif
