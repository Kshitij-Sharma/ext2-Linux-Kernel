#include "syscall_handlers.h"
#include "multiprocessing.h"

uint32_t visible_terminal = 1;
uint32_t process_terminal = 1;

pcb_t* cur_pcb_ptr[NUM_TERMINALS] = {NULL, NULL, NULL};

/** switch_terminal
 * 
 * Inputs: int32_t terminal_num - terminal we are switching to 
 * Outputs: none
 * Side Effects: switches terminals on screen, swaps video memory
 * 
 */

void switch_terminal(int32_t terminal_num)
{
    /* error checking */
    if(terminal_num < 0 || terminal_num >= NUM_TERMINAL)    return; 
    // if (cur_pcb_ptr[visible_terminal] != NULL) // save the ESP and EBP of the current process right where it left off
    // {
    //     asm volatile(
    //         "mov %%esp, %%eax;" /* push user_ds */
    //         "mov %%ebp, %%ebx;"
    //         : "=a"(cur_pcb_ptr[visible_terminal]->esp), "=b"(cur_pcb_ptr[visible_terminal]->ebp));
    // }
    /* gets the video buffer address of the current terminal */
    uint32_t prev_terminal_video = (visible_terminal == 0) ? TERMINAL_ONE_BUFFER : (visible_terminal == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER; 
    /* gets the video buffer address of the terminal we are switching to */
    uint32_t new_terminal_video = (terminal_num == 0) ? TERMINAL_ONE_BUFFER : (terminal_num == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER; 

    /* copies current video memory contents into the buffer for the previous one */
    memcpy((void*)prev_terminal_video, (void*)VIDEO, _4KB_PAGE);
    /* clears video memory */
    memset((void*)VIDEO, '\0', _4KB_PAGE);
    /* copy the terminal we are swtiching to's buffer into actual video memory */
    memcpy((void*)VIDEO, (void*)new_terminal_video, _4KB_PAGE);

    /* switching AWAY from a terminal using vidmap */
    if(cur_pcb_ptr[visible_terminal] != NULL && cur_pcb_ptr[visible_terminal]->vidmap_terminal == 1){ // the terminal that we are switching AWAY FROM is using Vidmap
        // modify_vid_mem(prev_terminal_video);
        vidmap_paging_modify(prev_terminal_video);
    }

    /* switching TO a terminal using vidmap */
    if(cur_pcb_ptr[terminal_num] != NULL && cur_pcb_ptr[terminal_num]->vidmap_terminal == 1){ // the terminal we are swtiching TO is using vidmap        
    //     modify_vid_mem(VIDEO);
        vidmap_paging_modify(VIDEO);
    }
    visible_terminal = terminal_num;
    update_cursor();
    send_eoi(IRQ_KEYBOARD);
    sti();
    // if (cur_pcb_ptr[terminal_num] == NULL){ // if shell has not yet been started on the terminal
    //     sys_execute("shell");
    //     return;
    // }
    // process_terminal = terminal_num;
    // // program_paging((cur_pcb_ptr[visible_terminal]->process_id * _4MB_PAGE) + _8_MB);
    // // // _execute_user_program_loader("shell");
    // // tss.ss0 = KERNEL_DS; // switch stack context
    // // tss.esp0 = _8_MB - (_8_KB * cur_pcb_ptr[visible_terminal]->process_id); // pointer to the top of stack/pcb
    
    // asm volatile(
    // "movl %0, %%esp;"
    // "movl %1, %%ebp;"
    // "leave;"
    // "ret;"
    // :
    // : "r"(cur_pcb_ptr[terminal_num]->esp), "r"(cur_pcb_ptr[terminal_num]->ebp));
    return;
}




/** scheduling
 * 
 * Inputs: jackshit 
 * Outputs: jackshit
 * Side Effects: hella shit
 * 
 */
// REMEMBER TO SET RTC FREQUENCY EVERY TIME WE CHANGE TASKS
void scheduling(){
    pit_init(20);
    int i = 0;
    for(; i < 300000000; i++)   continue;
    clear();
    printf("change pit");
    pit_init(100);
    // /* gets video memory of process we are switching away from */
    // uint32_t prev_terminal_video = (process_terminal == 0) ? TERMINAL_ONE_BUFFER : (process_terminal == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER;
    // /* gets video memory of process we are switching in to */
    // uint32_t new_terminal_video = (((process_terminal + 1) % 3) == 0) ? TERMINAL_ONE_BUFFER : (((process_terminal + 1) % 3) == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER; 
    // //// OLD PROCESS 
    // /* saves ESP and EBP of current process if there is a process running */
    // if(cur_pcb_ptr[process_terminal] != NULL){
    //     asm volatile(
    //         "mov %%esp, %%eax;" /* push user_ds */
    //         "mov %%ebp, %%ebx;"
    //         : "=a"(cur_pcb_ptr[process_terminal]->esp), "=b"(cur_pcb_ptr[process_terminal]->ebp));
    // }
    // /* switching AWAY from a terminal using vidmap */
    // if(cur_pcb_ptr[process_terminal] != NULL && cur_pcb_ptr[process_terminal]->vidmap_terminal == 1){ // the terminal that we are switching AWAY FROM is using Vidmap
    //     // modify_vid_mem(prev_terminal_video);
    //     vidmap_paging_modify(prev_terminal_video);
    // }// POSSIBLY UNNEEDED

    // // NEW PROCESS 
    // /* increments to the next procress */
    // process_terminal = (process_terminal + 1) % 3;
    // /* if shell has not yet been started on the terminal */  
    // if (cur_pcb_ptr[process_terminal] == NULL){ 
    //     sti();
    //     send_eoi(IRQ_PIT);
    //     sys_execute("shell");
    //     return;
    // }
    
    // program_paging((cur_pcb_ptr[process_terminal]->process_id * _4MB_PAGE) + _8_MB);

    // /* Set TSS */
    // tss.esp0 = _8_MB - (_8_KB * cur_pcb_ptr[process_terminal]->process_id); // pointer to the top of stack/pcb

    // // /* update running video coordinates */
    // // modify_vid_mem(VIDEO);

    // // /* switching TO a terminal using vidmap */
    // if(cur_pcb_ptr[process_terminal]->vidmap_terminal == 1 && process_terminal == visible_terminal){ // the terminal we are swtiching TO is using vidmap        
    //     vidmap_paging_modify(VIDEO);
    // }
    
    // asm volatile(
    //     "mov %0, %%esp;" /* push user_ds */
    //     "mov %1, %%ebp;"
    //     "leave;"
    //     "ret;"
    //     :
    //     : "r"(cur_pcb_ptr[process_terminal]->esp), "r"(cur_pcb_ptr[process_terminal]->ebp)
    //     );
    putc('a');
}
