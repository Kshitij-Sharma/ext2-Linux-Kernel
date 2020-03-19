#define ASM     1
#include "idt_interrupt_wrappers.h"
.text 

.globl keyboard_interrupt_asm, rtc_interrupt_asm, divide_by_zero_test

.align 4

/* Keyboard Interrupt handler*/
/* rtc_interrupt_asm()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: prints out key pressed to screen
*/keyboard_interrupt_asm:     
    pushal
    
    /*call C handler*/
    call keyboard_interrupt
    /* send EOI signal */
    pushl $IRQ_KEYBOARD
    call send_eoi               /* sends the EOI */
    popl %ebx


    popal
    iret

/* rtc_interrupt_asm()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: prints out "RTC INTERRUPT" to screen
        NOTE: this is an assembly wrapper function for 
*/
rtc_interrupt_asm:  

    pushal
    /* call the C handler*/
    call rtc_interrupt 

    /* send EOI signal */
    pushl $IRQ_RTC
    call send_eoi               /* sends the EOI */
    popl %ebx



    popal
    
    /* Return control */
    iret

/*void system_call()                  { return; }*/
