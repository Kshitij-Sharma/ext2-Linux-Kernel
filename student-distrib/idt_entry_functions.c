
#include "idt_entry_functions.h"



void divide_by_zero()               { exception_handler(0); }
void debug()                        { exception_handler(1); }
void non_maskable_interrupt()       { exception_handler(2); }   
void breakpoint()                   { exception_handler(3); }
void into_detected_overflow()       { exception_handler(4); }
void out_of_bounds()                { exception_handler(5); }
void invalid_opcode()               { exception_handler(6); }
void no_coprocessor()               { exception_handler(7); }
void double_fault()                 { exception_handler(8); }
void coprocessor_segment_overrun()  { exception_handler(9); }
void bad_tss()                      { exception_handler(10); }
void segment_not_present()          { exception_handler(11); }
void stack_fault()                  { exception_handler(12); }
void general_protection_fault()     { exception_handler(13); }
void page_fault()                   { exception_handler(14); }
void unknown_interrupt()            { exception_handler(15); }
void coprocessor_fault()            { exception_handler(16); }
void alignment_check()              { exception_handler(17); }
void machine_check()                { exception_handler(18); } 
void SIMD_floating_point()          { exception_handler(19); } 
void empty()                        { return; }
void reserved()                     { return; }
void pit_interrupt()                
{ 
    
    // FILL THIS IN
    return;     
}
void keyboard_interrupt() // send eoi, assembly linkage, read the char    
{ 
    
    // FILL THIS IN    
    printf("**************************************************************************");
}
void rtc_interrupt() // put this into an assembly file instead -- use iret and pushal popal -- also send EOI
{ 
    printf("RTC HANDLER \n");
    outb(STATUS_REGISTER_C, RTC_CMD_PORT); 
    inb(RTC_DATA_PORT); 
}

void system_call()                  { return; }

char * error_messages[NUM_EXCEPTIONS] = {"Cannot divide by zero.", "Debug exception!", "Non-maskable interrupt exception.",
"Breakpoint exception.", "Into Detected Overflow Exception!", "Out of bounds exception.", "Invalid opcode.",
"No coprocessor exception.", "Double fault exception.", "Coprocessor segment overrun.", "Bad TSS.", "Segment not present.", 
"Stack fault.", "General protection fault.", "Page fault.", "Unknown interrupt!", "Coprocessor fault.", "Alignment check.", "Machine check.",
"SIMD Floating point exception."};

void exception_handler(int index)
{
    // cli();
    printf("EXCEPTION: %s \n EXCEPTION #: %d \n", error_messages[index], index);
	// printf("idt div 0 test*******************************************************************************************************************************************************************************************************\n");
    while(1);
    // sti();
    return;
}

