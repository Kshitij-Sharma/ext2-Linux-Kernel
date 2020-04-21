#ifndef _MULTIPROCESSING_
#define _MULTIPROCESSING_
#include "lib.h"
#include "syscall_handlers.h"
#include "paging.h"
#include "i8259.h"

#define NUM_TERMINALS   3

// uint32_t switch_terminals(int32_t terminal_num);
extern uint32_t terminal_id;
extern pcb_t* cur_pcb_ptr[NUM_TERMINALS];


#endif
