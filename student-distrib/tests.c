#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "idt.h"
#include "rtc.h"
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
	for (i = 0; i < 20; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* IDT Entries Test
 * 
 * Checks all IDT entry member values for first 20 exceptions
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: check that seg_selector, reserved 0-4, size, dpl, present are all correct
 * Files: x86_desc.h/S
 */
int idt_entries_test()
{
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
        pit_interrupt,              /*interrupt start */
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

    int_ex[SYSTEM_CALL_IDX] =  system_call;
	int i;
	int result = PASS;
	for(i = 0; i < 20; i++)
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

/* IDT divide by zero test 
 * Shows that IDT exceptions work
 * Inputs: argument to divide by 0
 * Outputs: PASS/FAIL
 * Side Effects: freeze screen (while 1)
 * Coverage: IDT exception 
 * Files: x86_desc.h/S
 */
int idt_divide_by_zero(int test_arg){
	int zero, arg;
	zero = 0;
	TEST_HEADER;
	arg = test_arg/zero;
	arg &= 0x0;				// clear arg for warning removal
	return PASS | arg;
}

int rtc_frequency_test()
{
	int i;
	for(i = 0; i < 1500000; i++) printf("starting in %d\n", i);
	rtc_set_frequency(0x04);
	return PASS;
}

/* Paging test
 * 
 * Shows that paging works when page fault occurs
 * Inputs: none
 * Outputs: PASS/FAIL
 * Side Effects: page fault
 * Coverage: paging fault
 * Files: x86_desc.h/S
 */
int test_paging()
{
	int * test, magic;
	test = NULL;
	TEST_HEADER;
	magic = *test;
	magic &= 0x00;			// clear magic for warning removal
	return PASS | magic;
}



// add more tests here

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	TEST_OUTPUT("idt_entries_test", idt_entries_test());
	// TEST_OUTPUT("idt_div_0", idt_divide_by_zero(5));
	// TEST_OUTPUT("paging test", test_paging());
	TEST_OUTPUT("rtc_frequency test", rtc_frequency_test());
	// exception_handler(0);
	// launch your tests here
}
