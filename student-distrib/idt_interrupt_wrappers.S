#define ASM     1
#include "idt_interrupt_wrappers.h"
.text 

.globl keyboard_interrupt_asm, rtc_interrupt_asm, divide_by_zero_test, system_call_asm

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
        NOTE: this is an assembly wrapper function for rtc interrupts 
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

/* system_call_asm()
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: prints out "SYSTEM CALL" to screen
        NOTE: this is an assembly wrapper function for system calls 
*/  
system_call_asm:

       pushal
        
        /* error check parameter */
        cmp $0, %eax
        jle error_ret
        cmp $10, %eax
        jg  error_ret
        
        add $-1, %eax /* subtract 1 to account for the fact that jump tables are 0 indexed while the number we are provided goes from 1-10 */
        
        
        /* push params */
        pushl %edx
        pushl %ecx
        pushl %ebx

        /* jump to function */
        call     *sys_call_table(,%eax,4)

        /* pop params */
        popl %ebx
        popl %ecx
        popl %edx
        movl %eax, (sys_call_eax)
        jmp cont

        error_ret:
        movl $-1, (sys_call_eax)

        cont:
        /* send EOI signal */
        pushl $IRQ_RTC
        call send_eoi               /* sends the EOI */
        popl %ebx

        popal
        movl (sys_call_eax), %eax

        /* Return control to user stack */
        iret

sys_call_table:
        .long   sys_halt
        .long   sys_execute
        .long   sys_read
        .long   sys_write
        .long   sys_open
        .long   sys_close
        .long   sys_getargs
        .long   sys_vidmap
        .long   sys_set_handler
        .long   sys_sigreturn
        

sys_call_eax:
        .long