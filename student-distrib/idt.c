#include "idt.h"





void idt_init(){
    int i;          // loop variable
    void * interrupts_and_excpetions[NUM_VEC] = {
        divide_by_zero,
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
        SIMD_floating_point,
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
        reserved,
        reserved,
        pit_interrupt,
        divide_by_zero,
        empty,                  // cascade to slave
        empty, 
        empty,                  // serial port
        empty,
        empty,
        empty,
        rtc_interrupt_asm,      // real time clock
        empty,
        empty,
        empty,
        empty,
        empty,
        empty,
        empty
    };

    interrupts_and_excpetions[SYSTEM_CALL_IDX] =  system_call;
    

    for (i = 0; i < NUM_VEC; i++)
    {
        /* set values in IDT */
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = (i < FIRST_INTERRUPT_IDX && i > LAST_INTERRUPT_IDX) ? 1 : 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved0 = 0;
        idt[i].size = 1;                                                // size is 32-bit, so we set to 1
        idt[i].dpl = (i == SYSTEM_CALL_IDX) ? 3 : 0;                    // DPL value set for index 0x80 b/c that needs to be called by user code
        // present bit set to 1 indicating it's a valid interrupt descriptor
        idt[i].present = (i < (KB_INTERRUPT_IDX + 1) || i == SYSTEM_CALL_IDX || i == RTC_INTERRUPT_IDX) ? 1 : 0;
        SET_IDT_ENTRY(idt[i], interrupts_and_excpetions[i]);            // stores the function address in the idt
    }
        // SET_IDT_ENTRY(idt[40], rtc_interrupt_asm);            // stores the function address in the idt
    

}
