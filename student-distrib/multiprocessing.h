#ifndef _MULTIPROCESSING
#define _MULTIPROCESSING

#include "syscall_handlers.h"
#include "lib.h"
#include "paging.h"
#include "i8259.h"
#include "PCB.h"
#include "idt_interrupt_wrappers.h"

#define NUM_TERMINAL   3

extern void switch_terminal(int32_t terminal_num);
extern void scheduling();
extern volatile uint32_t visible_terminal;
extern volatile uint32_t process_terminal;
extern pcb_t* cur_pcb_ptr[NUM_TERMINAL];
extern int scheduler_calls;
#endif
