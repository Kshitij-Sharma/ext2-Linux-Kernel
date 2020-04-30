#include "idt.h"

/* idt_init()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: initializes IDT with:
                                        0-19 for exceptions
                                        33 for keyboard interrupt
                                        40 for RTC interrupts
*/
void idt_init(){
    int i;          // loop variable
    /* Array that stores pointers to the exception/interrupt handlers*/
    void * interrupts_and_excpetions[NUM_VEC] = {
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
        pit_interrupt_asm,              /*interrupt start */
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
    /* fills in all other as empty except for system call idx */
    for (i = OTHER_INTERRUPTS_IDX; i < NUM_VEC; i++){
        if (i == SYSTEM_CALL_IDX)     interrupts_and_excpetions[i] =  system_call_asm;
        else                          interrupts_and_excpetions[i] =  empty;
    }
    
    for (i = 0; i < NUM_VEC; i++)
    {
        /* set values in IDT */
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        // reserved 3 should be 1 when we are setting a trap gate. 0 for an interrupt gate
        idt[i].reserved3 = (i < FIRST_INTERRUPT_IDX || i > LAST_INTERRUPT_IDX ) ? 1 : 0;
        if (i == 128) idt[i].reserved3 = 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved0 = 0;
        idt[i].size = 1;                                                // size is 32-bit, so we set to 1
        idt[i].dpl = (i == SYSTEM_CALL_IDX) ? 3 : 0;                    // DPL value set to 3 for index 0x80 b/c that needs to be called by user code
        // idt[i].present = (i < (KB_INTERRUPT_IDX + 1) || i == SYSTEM_CALL_IDX || i == RTC_INTERRUPT_IDX) ? 1 : 0;
        idt[i].present = 1;
        SET_IDT_ENTRY(idt[i], interrupts_and_excpetions[i]);            // stores the function address in the idt
    }    
}
