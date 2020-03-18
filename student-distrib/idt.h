#include "idt_entry_functions.h"
#include "x86_desc.h"
#include "idt_interrupt_handlers.h"
#include "lib.h"



#define FIRST_INTERRUPT_IDX     0x20
#define LAST_INTERRUPT_IDX      0x2F
#define PIT_INTERRUPT_IDX       0x20
#define KB_INTERRUPT_IDX        0x21
#define RTC_INTERRUPT_IDX       0x28
#define SYSTEM_CALL_IDX         0x80



void idt_init();


