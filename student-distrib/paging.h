#include "lib.h"

#define _4MB_PAGE           4194304
#define _4KB_PAGE           4096
#define NUMBER_OF_ENTRIES   1024
#define SIZE_OF_PAGE        4096
#define VIDEO_OFFSET        0xB8
#define VIDEO               0xB8000
#define KERNEL_START        0x400000 // starting at 4MB
#define PRESENT             0x1
#define READ_WRITE          0x2
#define USER                0x4
#define GLOBAL_BIT          0x100
#define PAGE_SIZE           0x80

// testing globals
#define ABOVE_KERNEL_MEM        (KERNEL_START + _4MB_PAGE + 4)
#define BELOW_KERNEL_MEM        (KERNEL_START - 12)
#define IN_KERNEL_MEM           (KERNEL_START + 12)

#define ABOVE_VIDEO_MEM        (VIDEO + _4KB_PAGE + 4)
#define BELOW_VIDEO_MEM        (VIDEO - 12)
#define IN_VIDEO_MEM           (VIDEO + 8)

uint32_t page_directory [NUMBER_OF_ENTRIES] __attribute__((aligned (SIZE_OF_PAGE)));
uint32_t page_table [NUMBER_OF_ENTRIES] __attribute__((aligned (SIZE_OF_PAGE)));

/*function prototype needed to initialize paging */
void paging_init();

