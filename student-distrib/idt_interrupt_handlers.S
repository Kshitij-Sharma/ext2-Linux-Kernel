#define ASM     1
#include "idt_interrupt_handlers.h"
.text 

.globl keyboard_interrupt_asm, rtc_interrupt_asm, divide_by_zero_test


.align 4

/* send eoi, assembly linkage, read the char */
keyboard_interrupt_asm:     
    pushal

    push $0
    call exception_handler
    pop %edx 

    call keyboard_interrupt
    /* send EOI signal */
    push $RTC_IRQ
    call send_eoi               /* sends the EOI */
    pop %ebx


    popal
    iret

/* also send EOI */
rtc_interrupt_asm:  

    /* pushfl */
    pushal
    
    call rtc_interrupt

    /* send EOI signal */
    push $RTC_IRQ
    call send_eoi               /* sends the EOI */
    pop %ebx

    popal
    /* popfl */

    iret

/*void system_call()                  { return; }*/