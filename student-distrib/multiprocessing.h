#ifndef _MULTIPROCESSING
#define _MULTIPROCESSING

#include "syscall_handlers.h"
#include "lib.h"
#include "paging.h"
#include "i8259.h"
#include "PCB.h"

#define NUM_TERMINAL   3

extern void switch_terminal(int32_t terminal_num);
extern void scheduling();
extern void switch_process_paging();
extern void set_tss();
extern uint32_t visible_terminal;
extern uint32_t process_terminal;
extern pcb_t* cur_pcb_ptr[NUM_TERMINAL];
// extern pcb_t* cur_pcb_ptr[NUM_TERMINALS];
#endif
