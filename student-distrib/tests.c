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
	zero = 0;
	arg = test_arg/zero;
	arg &= 0x0;				// clear arg for warning removal
	return PASS | arg;
}


// int test_kbd(){
// 	char out;
// 	TEST_HEADER;
// 	// outb(30,KB_CMD_REGISTER);
// 	// outb(30,KB_STATUS_REGISTER);
// 	outb(30,KB_DATA_PORT);
// 	out = keyboard_interrupt();
// 	printf("%c\n", out);
// 	if (out == 'a')
// 		return PASS;
// 	else return FAIL;
// }



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
int test_system_call()
{
	TEST_HEADER;
	/* makes a system call */
	asm volatile ("int $0x80");
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
int test_paging_dereference_null()
{
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
	int nb = _sys_read_terminal(0, (void*) buf, 200);
	if(nb != 0)				assertion_failure();
	nb = _sys_write_terminal(0, (void *) buf, 128);
	if(nb != 0)				assertion_failure();
	return PASS;
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
int test_sys_write_rtc()
{
	TEST_HEADER;
	int i;
	/* sets RTC frequency after delay */
	for(i = 0; i < 15000; i++) printf("%d", i);
	_sys_write_rtc(2);
	return PASS;
}/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
	/* CP1 Tests */
// launch your tests here
void launch_tests(){
	/* tests for IDT */
	// TEST_OUTPUT("idt_test", gay());
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
	/* CP2 Tests */

	/* tests for terminal driver */
	TEST_OUTPUT("test_sys_rw_terminal", test_sys_rw_terminal());
	TEST_OUTPUT("test_sys_write_rtc", test_sys_write_rtc());

	/* CP3 Tests */
	/* CP4 Tests */
	/* CP5 Tests */

	/* CP2 Tests */
	/* CP2 Tests */
	/* CP2 Tests */

	/* CP2 Tests */
	/* CP2 Tests */
	/* CP2 Tests */
}
