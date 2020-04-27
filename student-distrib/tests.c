#include "tests.h"
#include "x86_desc.h"
#include "idt_handlers.h"
#include "lib.h"
#include "idt.h"
#include "rtc.h"
#include "paging.h"
#include "syscall_handlers.h"
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < NUM_VEC; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* IDT Test - Check Entries
 * 
 * Checks all IDT entry member values for first 20 exceptions
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: check that seg_selector, reserved 0-4, size, dpl, present are all correct
 * Files: idt.h/.c
 */
int test_idt_entries(){
	TEST_HEADER;
	void * int_ex[NUM_VEC] = { /* array containing function pointers for exceptions*/
        divide_by_zero,             /*excpetion start*/
        debug,
        non_maskable_interrupt,
        breakpoint,
        into_detected_overflow,
        out_of_bounds,
        invalid_opcode,
        no_coprocessor,
        double_fault,
        coprocessor_segment_overrun,
        bad_tss,
        segment_not_present,
        stack_fault,
        general_protection_fault,
        page_fault,
        unknown_interrupt,
        coprocessor_fault,
        alignment_check,
        machine_check,
        SIMD_floating_point,        /*excpetion end*/
        reserved,                   /*reserved by Intel start*/
        reserved,
        reserved,
        reserved,
        reserved,
        reserved,
        reserved,
        reserved,
        reserved,
        reserved,
        reserved,
        reserved,                   /*reserved by Intel end */
        empty,              /*interrupt start */
        keyboard_interrupt_asm,
        empty,                      /* cascade to slave */
        empty, 
        empty,                      /* serial port */
        empty,
        empty,
        empty,
        rtc_interrupt_asm,          /* real time clock */
        empty,
        empty,
        empty,
        empty,
        empty,
        empty,
        empty                       /* interrupt end */
    };
	int i;
	int result = PASS;

	for (i = OTHER_INTERRUPTS_IDX; i< NUM_VEC; i++){
        if (i == SYSTEM_CALL_IDX)     int_ex[i] =  system_call_asm;
        else                          int_ex[i] =  empty;
    }
	for(i = 0; i < NUM_VEC; i++)
	{
		/* check function pointers */
		result = ((void *)((idt[i].offset_31_16<<16)+(idt[i].offset_15_00)) == int_ex[i]) ? PASS : FAIL;
		if(result == FAIL)				assertion_failure();

		/* check seg_selector */
		result = (idt[i].seg_selector == KERNEL_CS) ? PASS : FAIL;
		if(result == FAIL)				assertion_failure();

		/* check reserved4 */
		result = (idt[i].reserved4 == 0) ? PASS : FAIL;
		if(result == FAIL)				assertion_failure();
		
		/* check reserved3 */
		if(i < FIRST_INTERRUPT_IDX || i > LAST_INTERRUPT_IDX)
			result = (idt[i].reserved3 == 1) ? PASS : FAIL;
		else
			result = (idt[i].reserved3 == 0) ? PASS : FAIL;
		if(result == FAIL)				assertion_failure();
		
		/* check reserved2 */
		result = (idt[i].reserved2 == 1) ? PASS : FAIL;
		if(result == FAIL)				assertion_failure();

		/* check reserved1 */
		result = (idt[i].reserved1 == 1) ? PASS : FAIL;
		if(result == FAIL)				assertion_failure();

		/* check reserved0 */
		result = (idt[i].reserved0 == 0) ? PASS : FAIL;
		if(result == FAIL)				assertion_failure();

		/* check size */
		result = (idt[i].size == 1) ? PASS : FAIL;
		if(result == FAIL)				assertion_failure();

		/* check dpl */
		if (i == SYSTEM_CALL_IDX)
			result = (idt[i].dpl == 3) ? PASS : FAIL;
		else			
			result = (idt[i].dpl == 0) ? PASS : FAIL;
		if(result == FAIL)		assertion_failure();

		/* check present */
		if(i < KB_INTERRUPT_IDX + 1 || i == SYSTEM_CALL_IDX || i == RTC_INTERRUPT_IDX)
			result = (idt[i].present == 1) ? PASS : FAIL;
		else 
			result = (idt[i].present == 0) ? PASS : FAIL;
		if(result == FAIL)				assertion_failure();

	}
	return result;
}

/* IDT Test - Divide by Zero Exception
 * 
 * Shows that exceptions are handled when triggered
 * Inpu
	TEST_HEADER;ts: none
 * Outputs: Should just freeze the sysytem after printing that a divide by 0 exception was triggered
 * Side Effects: divide by zero exception
 * Coverage: exceptions
 * Files: idt.h/.c idt_handlers.h/.c
 */
int test_idt_div_zero(int test_arg){
	TEST_HEADER;
	/* attempts to divide by zero */
	int zero, arg;
	int * test = 0;
	zero = *test;
	zero = 0;
	arg = test_arg/zero;
	arg &= 0x0;				// clear arg for warning removal
	return PASS | arg;
}


/* IDT Test - Other Exception
 * 
 * Shows that exceptions are handled when triggered
 * Inputs: none
 * Outputs: Should freeze the screen and say which exception was triggered (uncomment the exception you want to test)
 * Side Effects: SIMD exception (or others, uncomment) exception
 * Coverage: exceptions
 * Files: idt.h/.c. idt_handlers.h/.c
 */
int test_idt_exceptions(){
	TEST_HEADER;
	// asm volatile ( "int $0x00" ); // this 0x00 is the divide by 0 exception
	// asm volatile ( "int $0x01" ); // this 0x01 is the debug exception
	// asm volatile ( "int $0x05" ); // this 0x05 is the out of bounds exception
	// asm volatile ( "int $0x08" ); // this 0x08 is the general protection fault exception
	asm volatile ( "int $0x13" ); // this 0x13 is the SIMBD page fault exception
	return PASS;
}

/* IDT Test - System Call
 * 
 * Shows that System Calls work
 * Inputs: none
 * Outputs: PASS/FAIL
 * Side Effects: system call
 * Coverage: system call
 * Files: paging.c/.h
 */
int test_system_call(){
	TEST_HEADER;
	/* makes a system call */
	asm volatile (
			// "movl $0, %eax;"
			// "int $0x80;"
			// "cmpl $-1, %eax;"
			// "je wrong;"

			"movl $1, %eax;"
			"int $0x80;"
			"movl $2, %eax;"
			"int $0x80;"
			"movl $3, %eax;"
			"int $0x80;"
			"movl $4, %eax;"
			"int $0x80;"
			"movl $5, %eax;"
			"int $0x80;"
			"movl $6, %eax;"
			"int $0x80;"
			"movl $7, %eax;"
			"int $0x80;"
			"movl $8, %eax;"
			"int $0x80;"
			"movl $9, %eax;"
			"int $0x80;"
			"movl $10, %eax;"
			"int $0x80;"
			);
	return PASS;
}

/* Paging Test - Dereferencing Invalid Pointer
 * 
 * Shows that dereferencing a NULL pointer produces a page fault
 * Inputs: none
 * Outputs: Page fault -- should freeze screen
 * Side Effects: page fault
 * Coverage: paging fault
 * Files: paging.c/.h
 */
int test_paging_dereference_null(){
	TEST_HEADER;
	int * test, magic;
	/* attempt to dereference NULL */
	test = NULL;
	magic = *test;
	magic = 0x00;			// clear magic for warning removal
	return PASS | magic;
}


/* Paging Test - Accessing Kernel Memory
 * 
 * Asserts that kernel memory is accessible within the 4MB to 8MB locations
 * Inputs: None
 * Outputs: PASS
 * Side Effects: When testing the ABOVE_KERNEL_MEM and BELOW_KERNEL_MEM cases, a page fault will occur, otherwise it should pass
 * Coverage: Page Directory set up
 * Files: paging.h/c
 */
int test_paging_ker_mem(int val) {
	TEST_HEADER;

	int* test;
	int magic;
	int result = PASS;
	
	switch(val) {
		case ABOVE_KERNEL_MEM:
			printf("OUTSIDE KERNEL MEMORY (ABOVE): ");
			/* attempts to dereference value above kernel memory */
			test = (int*) (val);
			magic = *test;
			break;
		case BELOW_KERNEL_MEM:
			printf("OUTSIDE KERNEL MEMORY (BELOW): ");
			/* attempts to dereference value below kernel memory */
			test = (int*) (val);
			magic = *test;
			break;
		case IN_KERNEL_MEM:
			printf("IN_BOUNDS: ");
			/* attempts to dereference value inside kernel memory */
			test = (int*) (val);
			magic = *test;
			
			/* attempts to dereference the start of kernel memory */
			test = (int*) (KERNEL_START);
			magic = *test;
			break;
		default:
			printf("INVALID TEST PARAMETER");
			break;
	}
	magic = result;
	return magic;
}


/* Paging Test - Accessing Video Meemory
 * 
 * Asserts that video memory is accessible within the xBF to (xBF + 4KB) space
 * Inputs: None
 * Outputs: PASS
 * Side Effects: When testing the ABOVE_VIDEO_MEM and BELOW_VIDEO_MEM cases, 
 * 							 a page fault will occur
 * Coverage: Page Directory set up
 * Files: paging.h/c
 */
int test_paging_video_mem(int val) {
	TEST_HEADER;

	int* test;
	int magic;
	int result = PASS;
	
	switch(val) {
		case ABOVE_VIDEO_MEM:
			printf("OUTSIDE VIDEO MEMORY (ABOVE): ");
			/* attempts to dereference value above video memory */
			test = (int*) (val);
			magic = *test;
			break;
		case BELOW_VIDEO_MEM:
			printf("OUTSIDE VIDEO MEMORY (BELOW): ");
			/* attempts to dereference value below video memory */
			test = (int*) (val);
			magic = *test;
			break;
		case IN_VIDEO_MEM:
			printf("IN_BOUNDS: ");
			/* attempts to dereference value inside video memory */
			test = (int*) (val);
			magic = *test;

			/* attempts to dereference start of video memory */
			test = (int*) (VIDEO);
			magic = *test;
			break;
		default:
			printf("INVALID TEST PARAMETER");
			break;
	}
	magic = result;
	return magic;
}

// add more tests here

/* Checkpoint 2 tests */

/* System Read/Write Test - Terminal
 * 
 * Shows that you can do system reads and writes from terminal
 * Inputs: none
 * Outputs: PASS or assertion failure 
 * Side Effects: none
 * Coverage: system read/write
 * Files: syscall_handlers.h/.c
 */
int test_sys_rw_terminal(){
	TEST_HEADER;

	char buf[128];
	while (1){
	int nb = _sys_read_terminal(0, (void*) buf, 400);
	_sys_write_terminal(0, (void *) buf, nb);
	}
	// printf("%s", buf);
	// if(nb != 0)				assertion_failure();
	// if(nb != 0)				assertion_failure();
	return PASS;
}

/* System Read/Write Test - Terminal
 * 
 * Shows that you can do system reads and writes from terminal
 * Inputs: none
 * Outputs: PASS or assertion failure 
 * Side Effects: none
 * Coverage: system read/write
 * Files: syscall_handlers.h/.c
 */
int test_sys_write_terminal_overflow(){
	TEST_HEADER;
	int i;
	i = _sys_write_terminal(0, "1234", 5);
	return (i == 4) ? PASS : FAIL;
}

/* System Write Test - RTC
 * 
 * Shows that you can set RTC frequency
 * Inputs: none
 * Outputs: Should see a visual difference in the frequency of the rtc interrupt (Returns PASS regardless)
 * Side Effects: change RTC frequency
 * Coverage: RTC frequency
 * Files: syscall_handlers.h/.c
 */
int test_sys_write_rtc(){
	TEST_HEADER;
	long long i;
	int freq[1] = {2};
	/* sets RTC frequency after delay */
	_sys_write_rtc(NULL, (void*) freq, 4);
	RTC_ON_FLAG[visible_terminal] = 1;
	for(i = 0; i < 450000000; i++);
	RTC_ON_FLAG[visible_terminal] = 0;
	clear();

	freq[0] = 4;
	/* sets RTC frequency after delay */
	_sys_write_rtc(NULL, (void*) freq, 4);
	RTC_ON_FLAG[visible_terminal] = 1;
	for(i = 0; i < 450000000; i++);
	RTC_ON_FLAG[visible_terminal] = 0;
	clear();

	freq[0] = 8;
	/* sets RTC frequency after delay */
	RTC_ON_FLAG[visible_terminal] = 1;
	_sys_write_rtc(NULL, (void*) freq, 0);
	for(i = 0; i < 300000000; i++);
	RTC_ON_FLAG[visible_terminal] = 0;
	clear();

	freq[0] = 16;
	/* sets RTC frequency after delay */
	_sys_write_rtc(NULL, (void*) freq, 4);
	RTC_ON_FLAG[visible_terminal] = 1;
	for(i = 0; i < 300000000; i++);
	RTC_ON_FLAG[visible_terminal] = 0;
	clear();
	
	freq[0] = 32;
	/* sets RTC frequency after delay */
	RTC_ON_FLAG[visible_terminal] = 1;
	_sys_write_rtc(NULL, (void*) freq, 0);
	for(i = 0; i < 150000000; i++);
	RTC_ON_FLAG[visible_terminal] = 0;
	clear();

	freq[0] = 64;
	/* sets RTC frequency after delay */
	_sys_write_rtc(NULL, (void*) freq, 4);
	RTC_ON_FLAG[visible_terminal] = 1;
	for(i = 0; i < 150000000; i++);
	RTC_ON_FLAG[visible_terminal] = 0;
	clear();
	
	freq[0] = 128;
	/* sets RTC frequency after delay */
	RTC_ON_FLAG[visible_terminal] = 1;
	_sys_write_rtc(NULL, (void*) freq, 0);
	for(i = 0; i < 150000000; i++);
	RTC_ON_FLAG[visible_terminal] = 0;
	clear();

	freq[0] = 256;
	/* sets RTC frequency after delay */
	_sys_write_rtc(NULL, (void*) freq, 4);
	RTC_ON_FLAG[visible_terminal] = 1;
	for(i = 0; i < 150000000; i++);
	RTC_ON_FLAG[visible_terminal] = 0;
	clear();
	
	// for(i = 0; i < 50000; i++);
	// _sys_write_rtc(NULL, freq, 256);
	// clear();
	freq[0] = 512;
	RTC_ON_FLAG[visible_terminal] = 1;
	_sys_write_rtc(NULL, (void*) freq, 0);
	for(i = 0; i < 150000000; i++);
	RTC_ON_FLAG[visible_terminal] = 0;
	clear();
	RTC_ON_FLAG[visible_terminal] = 1;
	return PASS;
}


/* System Open Test - RTC
 * 
 * Shows that you can open the RTC
 * Inputs: none
 * Outputs: Pass Fail
 * Side Effects: sets RTC frequency to 0
 * Coverage: RTC open
 * Files: syscall_handlers.h/.c
 */
int test_sys_open_rtc(){
	TEST_HEADER;
	long long i;
	int freq;
	freq = 1024;
	clear();
	/* sets RTC frequency after delay */
	_sys_write_rtc(NULL, (void*) freq, 4);
	RTC_ON_FLAG[visible_terminal] = 1;
	for(i = 0; i < 450000000; i++);
	RTC_ON_FLAG[visible_terminal] = 0;
	clear();

	freq = 2;
	/* sets RTC frequency after delay */
	RTC_ON_FLAG[visible_terminal] = 1;
	_sys_open_rtc(NULL);
	for(i = 0; i < 450000000; i++);
	RTC_ON_FLAG[visible_terminal] = 0;
	return PASS;
}

/* System Read Test - RTC
 * 
 * Shows that you can read RTC interrupt
 * Inputs: none
 * Outputs: Pass Fail
 * Side Effects: prints test_interrupts output to screen via RTC handler
 * Coverage: RTC read
 * Files: syscall_handlers.h/.c
 */
int test_sys_read_rtc(){
	TEST_HEADER;
	int freq;
	int i = 450000000;
	freq = 2;
	/* sets RTC frequency after delay */
	_sys_write_rtc(NULL, (void*) freq, 4);
	RTC_ON_FLAG[visible_terminal] = 1;
	for(i = 0; i < 200000000; i++);
	while (i-- > 0){
		_sys_read_rtc(0, NULL, 0);
		test_interrupts();
	}
	return PASS;
}


/* System Open/Read Test - text file
 * 
 * Shows that you can open and read a text file
 * Inputs: none
 * Outputs: PASS or assertion failure
 * Side Effects: displays contents of frame0 on screen
 * Coverage: file open/read
 * Files: syscall_handlers.h/.c, filesys.c/.h
 */
int test_file_read_open_text(){
	TEST_HEADER;
	char buf[200];
	int32_t i, ret, filename;
	/* opens frame0.txt */
	filename = _sys_open_file((uint8_t *) "frame0.txt");
	/* reads file */
	ret = _sys_read_file(filename, buf, 187);
	/* checks that read was successful */
	if(ret != 187) 				assertion_failure();
	/* prints out read content */
	for (i = 0; i < 187; i++) {
		if(buf[i] != '\0')	putc(buf[i]);
	}
	printf("\n");
	return PASS;
}

/* System Open/Read Test - EXE file
 * 
 * Shows that you can open and read a non-text file
 * Inputs: none
 * Outputs: PASS or assertion failure
 * Side Effects: displays contents of ls on screen
 * Coverage: file open/read
 * Files: syscall_handlers.h/.c, filesys.c/.h
 */
int test_file_read_open_non_text(){
	TEST_HEADER;
	char buf[7000];
	int32_t i, ret, filename;

	/* opens frame0.txt */
	// filename = _sys_open_file("ls");
	filename = _sys_open_file((uint8_t *)"grep");
	
	/* reads file */
	// ret = _sys_read_file(filename, buf, 5349);
	ret = _sys_read_file(filename, buf, 6149);

	/* checks that read was successful */
	// if(ret != 5349) 				assertion_failure();
	if(ret != 6149) 				assertion_failure();
	
	/* prints out read content */
	// for (i = 0; i < 5349; i++) {
	for (i = 0; i < 6149; i++) {
		if(buf[i] != '\0')	putc(buf[i]);
	}
	printf("\n");
	return PASS;
}


/* System Open/Read Test - long text file
 * 
 * Shows that you can open and read a text file
 * Inputs: none
 * Outputs: PASS or assertion failure
 * Side Effects: displays contents of verylongfilewithverylargename on screen
 * Coverage: file open/read
 * Files: syscall_handlers.h/.c, filesys.c/.h
 */
int test_file_read_open_text_long(){
	TEST_HEADER;
	char buf[37000];
	int32_t i, ret, filename;
	/* opens frame0.txt */
	// filename = _sys_open_file("verylargetextwithverylongname.txt");
	filename = _sys_open_file((uint8_t *) "fish");
	
	// if(filename == -1)				assertion_failure();
	/* reads file */
	// ret = _sys_read_file(filename, buf, 5277);
	ret = _sys_read_file(filename, buf, 36164);
	/* checks that read was successful */
	// if(ret != 5277) 				assertion_failure();
	if(ret != 36164) 				assertion_failure();
	
	/* prints out read content */
	// for (i = 0; i < 5277; i++) 
	for (i = 0; i < 36164; i++) 
	{	if(buf[i] != '\0')	putc(buf[i]); }
	printf("\n");
	return PASS;
}

/* System Read Test - directory
 * 
 * Shows that you can read the directory
 * Inputs: none
 * Outputs: PASS or assertion failure
 * Side Effects: displays contents of the directory
 * Coverage: file open/read
 * Files: syscall_handlers.h/.c, filesys.c/.h
 */
int test_directory_read(){
	TEST_HEADER;
	char buf[32*17];
	int i = 1;

	while (i != 0){
		i =_sys_read_directory(i, buf, 32);
		if (i != 0)
			printf("%s\n",buf);
	}


	return PASS;
}

int test_execute_helpers(){
	TEST_HEADER;
	char buf1[128];
	int i;
	char prog_name[32];
    memset(prog_name, '\0', 32);
	char arg[128];
	memset(arg, '\0', 128);
	char buf[30] = {' ', ' ', ' ', ' ',' ','s',' ',' ',' ','a','p','p','l','e','s',' ', ' '};
	_execute_parse_args(buf, prog_name, arg);
	printf("Program name is:%s\nArgs are:%s\n", prog_name, arg);

	i = _execute_executable_check(prog_name, buf1);

	return (i==0) ? PASS : FAIL;

}

int test_execute(){
	TEST_HEADER;
	char buf[30] = {'s','h','e','l','l', ' ', ' ',' ','a','p','p','l','e','s',' ', ' '};
	sys_execute(buf);
	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */
int pit_test(){
	pit_init(20);
	printf("pit intiialized \n");
    int i = 0;
    for(i = 0; i < 300000000; i++) continue;
    clear();
    printf("change pit");
    pit_init(1000);
	for(i = 0; i < 300000000; i++) continue;
	return PASS;
}
/* Test suite entry point */
// launch your tests here
void launch_tests(){
	/******************************** CP1 TESTS ********************************/ 
	/* tests for IDT */
	// TEST_OUTPUT("test_kbd", test_kbd());
	// TEST_OUTPUT("test_idt_entries", test_idt_entries());
	// TEST_OUTPUT("test_idt_div_zero", test_idt_div_zero(5)); // causes an exception
	// TEST_OUTPUT("test_idt_exceptions", test_idt_exceptions()); // causes an exception
	// TEST_OUTPUT("test_system_call", test_system_call());
	
	/* tests for RTC */
	// TEST_OUTPUT("test_rtc_frequency", test_rtc_frequency()); // changes frequency of RTC interrupts

	/* test keyboard: type and echo characters */
	
	/* tests for paging */
	// TEST_OUTPUT("test_paging_dereference_null", test_paging_dereference_null()); // causes an exception
	// TEST_OUTPUT("test_paging_above_kernel", test_paging_ker_mem(ABOVE_KERNEL_MEM)); // causes an exception
	// TEST_OUTPUT("test_paging_below_kernel", test_paging_ker_mem(BELOW_KERNEL_MEM)); // causes an exception
	// TEST_OUTPUT("test_paging_in_kernel", test_paging_ker_mem(IN_KERNEL_MEM));

	// TEST_OUTPUT("test_paging_above_video", test_paging_video_mem(ABOVE_VIDEO_MEM)); // causes an exception
	// TEST_OUTPUT("test_paging_below_video", test_paging_video_mem(BELOW_VIDEO_MEM)); // causes an exception
	// TEST_OUTPUT("test_paging_in_video", test_paging_video_mem(IN_VIDEO_MEM));
	
	/******************************** CP2 TESTS ********************************/ 
	/* tests for terminal driver */
	// TEST_OUTPUT("test_sys_rw_terminal", test_sys_rw_terminal());
	// TEST_OUTPUT("test_sys_write_terminal_overflow", test_sys_write_terminal_overflow());

	/* tests for files */
	// TEST_OUTPUT("test_file_read_open_non_text", test_file_read_open_non_text());
	// TEST_OUTPUT("test_file_read_open_text_long", test_file_read_open_text_long());
	// TEST_OUTPUT("test_file_read_open_text", test_file_read_open_text());

	// /* tests for directory */
	// TEST_OUTPUT("test_directory_read", test_directory_read());

	/* tests for RTC */
	// TEST_OUTPUT("test_sys_write_rtc", test_sys_write_rtc());
	// TEST_OUTPUT("test_sys_open_rtc", test_sys_open_rtc()); // changes frequency of RTC interrupts
	// TEST_OUTPUT("test_sys_read_rtc", test_sys_read_rtc());

	/* CP3 Tests */
	// TEST_OUTPUT("test_system_call", test_system_call());
	// TEST_OUTPUT("test_execute_helpers", test_execute_helpers());
	// TEST_OUTPUT("test_execute", test_execute());
	TEST_OUTPUT("test_pit", pit_test());

	/* CP4 Tests */
	/* CP5 Tests */

	/* CP2 Tests */
	/* CP2 Tests */
	/* CP2 Tests */

	/* CP2 Tests */
	/* CP2 Tests */
	/* CP2 Tests */
}
