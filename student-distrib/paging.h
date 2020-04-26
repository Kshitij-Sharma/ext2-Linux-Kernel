#ifndef _PAGING_H
#define _PAGING_H

#include "lib.h"

/* CONSTANTS FOR EASY SIZE ACCESS */
#define _4MB_PAGE                   4194304 
#define _4KB_PAGE                   4096
#define _128_MB                     134217728
#define _132_MB                     138412032
#define _8_MB                       8388608
#define _8_KB                       8192
#define _4_BYTES                    4

/* CONSTANTS FOR GENERAL PAGING SETUP */
#define NUMBER_OF_ENTRIES       1024
#define SIZE_OF_PAGE            _4KB_PAGE
#define KERNEL_START            0x400000    // starting at 4MB
#define USER_START              _128_MB     // 8MB
#define USER_END                _132_MB
#define PRESENT                 0x1
#define READ_WRITE              0x2
#define USER                    0x4
#define GLOBAL_BIT              0x100
#define PAGE_SIZE               0x80
/* CONSTANTS FOR VIDEO MEMORY PAGING  */
#define VIDEO_START                     _132_MB
#define VIDEO                           0xB8000
#define VIDEO_OFFSET                    (VIDEO >> 12)
#define TERMINAL_ONE_BUFFER             (VIDEO + _4KB_PAGE) //page_table[0]
#define TERMINAL_ONE_BUFFER_OFFSET      (VIDEO_OFFSET + 1)
#define TERMINAL_TWO_BUFFER             (VIDEO + (_4KB_PAGE*2)) 
#define TERMINAL_TWO_BUFFER_OFFSET      (VIDEO_OFFSET + 2)
#define TERMINAL_THREE_BUFFER           (VIDEO + (_4KB_PAGE*3)) 
#define TERMINAL_THREE_BUFFER_OFFSET    (VIDEO_OFFSET + 3)


/* CONSTANTS FOR PROGRAM PAGING */
#define SYS_VIRTUAL_MEM     0x20 // 128MB equates to x8000000, however page directory goes in 4MB increments,
                                 // so we do x8000000 / x400000 = x20
#define PROGRAM_IMAGE       0x08048000


// testing globals
#define ABOVE_KERNEL_MEM        (KERNEL_START + _4MB_PAGE + 4)
#define BELOW_KERNEL_MEM        (KERNEL_START - 12)
#define IN_KERNEL_MEM           (KERNEL_START + 12)

#define ABOVE_VIDEO_MEM        (VIDEO + _4KB_PAGE + 4)
#define BELOW_VIDEO_MEM        (VIDEO - 12)
#define IN_VIDEO_MEM           (VIDEO + 8)

uint32_t page_directory [NUMBER_OF_ENTRIES] __attribute__((aligned (SIZE_OF_PAGE)));
uint32_t page_table [NUMBER_OF_ENTRIES] __attribute__((aligned (SIZE_OF_PAGE)));
uint32_t page_table_vidmap [NUMBER_OF_ENTRIES] __attribute__((aligned (SIZE_OF_PAGE)));

/*function prototype needed to initialize paging */
void paging_init();
void flush_tlb();
void program_paging();
void vidmap_paging();
void vidmap_paging_modify(uint32_t terminal_address);
void vidmap_terminal_switch();
int process_num;

#endif
