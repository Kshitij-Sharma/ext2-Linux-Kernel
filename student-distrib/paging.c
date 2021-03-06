#include "paging.h"

/*
		paging_init()
		Description: paging initialization
		Inputs: None
		Outputs: None
		Side Effects: initializes paging
*/
void paging_init() {
	/* looping variable */
 	int32_t i;

 	/* This loop initializes the page directory and oen page table as empty, 
	 		grants read/write permissions */
 	for(i = 0; i < NUMBER_OF_ENTRIES; i++){

     /* This sets the following flags to the pages:
		 	* Supervisor: Only kernel-mode can access them (when the 2nd bit is 1: user mode, 0: kernel mode)
			* Write Enabled: It can be both read from and written to
			* Not Present: The page table is not present (Based on the 0th bit)
			*/
 		page_directory[i] = READ_WRITE & ~PRESENT;
		page_table[i] = (i * SIZE_OF_PAGE) | (READ_WRITE & ~PRESENT); /* attributes: supervisor level, read/write, not present. */
		page_table_vidmap[i] = READ_WRITE & ~PRESENT;
 	}

	/* the 0th page directory entry has a page table, since we break it down to 4KB pages */
	page_directory[0] =  (uint32_t) page_table;
	page_directory[0] |=  (READ_WRITE | PRESENT);
	
	/* sets up 4KB pages for video memory */
	page_table[VIDEO_OFFSET] |=  (READ_WRITE | PRESENT);
	page_table[TERMINAL_ONE_BUFFER_OFFSET] |=  (READ_WRITE | PRESENT);
	page_table[TERMINAL_TWO_BUFFER_OFFSET] |=  (READ_WRITE | PRESENT);
	page_table[TERMINAL_THREE_BUFFER_OFFSET] |=  (READ_WRITE | PRESENT);
	
	/* loading at 4MB physical address */
	page_directory[1] =  (KERNEL_START | PAGE_SIZE | READ_WRITE | PRESENT) | GLOBAL_BIT;


    asm volatile(
                 "movl %0, %%eax;" 			/* move page_directory into eax*/
                 "movl %%eax, %%cr3;" 		/* sets the page directory pointer */
                 "movl %%cr4, %%eax;"
                 "orl $0x00000010, %%eax;" 	/* mask used to set bit to 1 */
                 "movl %%eax, %%cr4;"		/* seting PSE to enable 4 MiB pages */
                 "movl %%cr0, %%eax;"
                 "orl $0x80000000, %%eax;" 	/* mask used to set correct bit to 1 */
                 "movl %%eax, %%cr0;"		/* set paging bit */
                 :                      	/* no outputs */
                 :"r"(page_directory)   	/* input */
                 :"eax"               		/* clobbered register */
                 );
                 
 }

	/*
		flush_tlb()
		Description: flushes tlb
		Inputs: None
		Outputs: None
		Side Effects: none
	*/
  void flush_tlb() {
    asm volatile(
      "movl %%cr3, %%eax;"
      "movl %%eax, %%cr3;"
      :
      :
      :"eax"
    );
}

/*
	program_paging()
	Description: Sets up paging for program
	Inputs: None
	Outputs: None
	Side Effects: none
	*/
void program_paging(uint32_t physical_address) {
	/* maps spot in virtual memory to appropriate physical memory */
	page_directory[SYS_VIRTUAL_MEM] = physical_address;
	/* assigns approriate attributes to the page */
	page_directory[SYS_VIRTUAL_MEM] |= PAGE_SIZE | USER | PRESENT | READ_WRITE;
	flush_tlb();
}

/*
	vidmap_paging()
	Description: Sets up paging for video memory defined at 132 MB
	Inputs: None
	Outputs: None
	Side Effects: None
 */
 void vidmap_paging() {
    page_directory[VIDEO_START / _4MB_PAGE] = ((uint32_t)(page_table_vidmap)) | (READ_WRITE | PRESENT | USER);
    /* then set up the page table */
    page_table_vidmap[0] = VIDEO;
	page_table_vidmap[0] |= (READ_WRITE | PRESENT | USER);
	flush_tlb();
 }
/*
	vidmap_paging_modify()
	Description: Sets up paging for video memory defined at the address passed in
	Inputs: None
	Outputs: None
	Side Effects: None
 */
 void vidmap_paging_modify(uint32_t terminal_address) {
    page_directory[VIDEO_START / _4MB_PAGE] = ((uint32_t)(page_table_vidmap)) | (READ_WRITE | PRESENT | USER);
    /* then set up the page table */
    page_table_vidmap[0] = terminal_address;
	page_table_vidmap[0] |= (READ_WRITE | PRESENT | USER);
	flush_tlb();
 }

