#define ASM     1
#include "idt_exception_wrappers.h"
.text 

.globl divide_by_zero, debug, non_maskable_interrupt, breakpoint
.globl into_detected_overflow, out_of_bounds, invalid_opcode
.globl no_coprocessor, double_fault, coprocessor_segment_overrun
.globl bad_tss, segment_not_present, stack_fault, general_protection_fault
.globl page_fault, unknown_interrupt, coprocessor_fault, alignment_check
.globl machine_check, reserved, SIMD_floating_point

.align 4

/* generic exception wrappers:
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: calls appropriate exception
        MAGIC NUMBERS: numbers pushed prior to exception handler call are the number corresponding to the exception that occurred
*/
divide_by_zero:     
    pushal
    
    push $0
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret

debug:     
    pushal
    
    push $1
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret

non_maskable_interrupt:     
    pushal
    
    push $2
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret

breakpoint:     
    pushal
    
    push $3
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret
    
into_detected_overflow:     
    pushal
    
    push $4
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret

out_of_bounds:     
    pushal
    
    push $5
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret

invalid_opcode:     
    pushal
    
    push $6
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret

no_coprocessor:     
    pushal
    
    push $7
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret


double_fault:     
    pushal
    
    push $8
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret
    
coprocessor_segment_overrun:     
    pushal
    
    push $9
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret
    
    
bad_tss:     
    pushal
    
    push $10
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret
    
segment_not_present:     
    pushal
    
    push $11
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret


stack_fault:     
    pushal
    
    push $12
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret

general_protection_fault:     
    pushal
    
    push $13
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret


page_fault:     
    pushal
    
    push $14
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret
    
unknown_interrupt:     
    pushal
    
    push $15
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret

coprocessor_fault:     
    pushal
    
    push $16
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret

alignment_check:     
    pushal
    
    push $17
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret

machine_check:     
    pushal
    
    push $18
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret
    
SIMD_floating_point:     
    pushal
    
    push $19
    /*call C handler*/
    call exception_handler
    
    pop %ebx

    popal
    iret
/*void system_call()                  { return; }*/
