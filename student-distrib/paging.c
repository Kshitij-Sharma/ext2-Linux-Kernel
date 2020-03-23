#include "paging.h"

/*
		paging_init()
		Description: paging initialization
		Inputs: None
		Outputs: None
		Side Effects: initializes paging
*/
void paging_init(){

 	int i;	//for loop index

 	/* This loop initializes the page directory and oen page table as empty, 
	 		grants read/write permissions */
 	for(i = 0; i < NUMBER_OF_ENTRIES; i++){

     /* This sets the following flags to the pages:
		 	* Supervisor: Only kernel-mode can access them (when the 2nd bit is 1: user mode, 0: kernel mode)
			* Write Enabled: It can be both read from and written to
			* Not Present: The page table is not present (Based on the 0th bit)
			*/
 		page_directory[i] = READ_WRITE;
		page_table[i] = (i * SIZE_OF_PAGE) | READ_WRITE; // attributes: supervisor level, read/write, not present.
 	}

	page_directory[0] =  (uint32_t) page_table;
	page_directory[0] |=  (USER | READ_WRITE | PRESENT);

	/* loading at 4MB physical address */
	page_directory[1] =  (KERNEL_START | PAGE_SIZE | READ_WRITE | PRESENT) | GLOBAL_BIT;
	page_table[VIDEO_OFFSET] |=  (USER | READ_WRITE | PRESENT);


    asm volatile(
                 "movl %0, %%eax;" 			/* clear eax*/
                 "movl %%eax, %%cr3;" 		/* sets the page directory pointer */
                 "movl %%cr4, %%eax;"
                 "orl $0x00000010, %%eax;" 	/* mask used to set bit to 1 */
                 "movl %%eax, %%cr4;"		/* seting PSE to enable 4 MiB pages */
                 "movl %%cr0, %%eax;"
                 "orl $0x80000000, %%eax;" 	/* mask used to set correct bit to 1 */
                 "movl %%eax, %%cr0;"		/* set paging bit */
                 :                      	/* no outputs */
                 :"r"(page_directory)   	/* input */
                 :"%eax"               		/* clobbered register */
                 );
                 
 }		

