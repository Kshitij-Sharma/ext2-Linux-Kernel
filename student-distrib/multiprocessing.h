#ifndef _MULTIPROCESSING
#define _MULTIPROCESSING

#include "syscall_handlers.h"
#include "lib.h"
#include "paging.h"
#include "i8259.h"
#include "PCB.h"
#include "idt_interrupt_wrappers.h"

#define NUM_TERMINALS       3

/* function to switch video memory */
extern void switch_terminal(int32_t terminal_num);
/* function to context switch to a new process */
extern void scheduling();
/* variable that tells us which terminal we see right now */
extern volatile uint32_t visible_terminal;

/* variable that tells us which terminal the current process is running on */
extern volatile uint32_t process_terminal;
/* array of current pcb pointer of each terminal */
extern pcb_t* cur_pcb_ptr[NUM_TERMINALS];

#endif
