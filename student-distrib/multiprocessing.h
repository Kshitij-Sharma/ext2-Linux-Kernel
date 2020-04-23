#ifndef _MULTIPROCESSING
#define _MULTIPROCESSING

#include "syscall_handlers.h"
#include "lib.h"
#include "paging.h"
#include "i8259.h"
#include "PCB.h"

#define NUM_TERMINAL   3

// uint32_t switch_terminals(int32_t terminal_num);
extern uint32_t terminal_id;
extern pcb_t* cur_pcb_ptr[NUM_TERMINAL];
// extern pcb_t* cur_pcb_ptr[NUM_TERMINALS];
#endif
