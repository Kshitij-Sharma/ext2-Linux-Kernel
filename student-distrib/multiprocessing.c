#include "syscall_handlers.h"
#include "multiprocessing.h"

uint32_t terminal_id = 0;
pcb_t* cur_pcb_ptr[NUM_TERMINALS] = {NULL, NULL, NULL};

