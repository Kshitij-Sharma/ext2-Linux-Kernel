#include "syscall_handlers.h"
#include "multiprocessing.h"

volatile uint32_t visible_terminal = 0;
volatile uint32_t process_terminal = 0;
pcb_t *cur_pcb_ptr[NUM_TERMINALS] = {NULL, NULL, NULL};

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
    if (terminal_num < 0 || terminal_num >= NUM_TERMINALS) return;

    /* gets the video buffer address of the current terminal */
    uint32_t prev_terminal_video = (visible_terminal == 0) ? TERMINAL_ONE_BUFFER : (visible_terminal == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER;
    /* gets the video buffer address of the terminal we are switching to */
    uint32_t new_terminal_video = (terminal_num == 0) ? TERMINAL_ONE_BUFFER : (terminal_num == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER;

    /* copies current video memory contents into the buffer for the previous one */
    memcpy((void *)prev_terminal_video, (void *)VIDEO, _4KB_PAGE);
    /* clears video memory */
    memset((void *)VIDEO, '\0', _4KB_PAGE);
    /* copy the terminal we are swtiching to's buffer into actual video memory */
    memcpy((void *)VIDEO, (void *)new_terminal_video, _4KB_PAGE);

    /* switching AWAY from a terminal using vidmap */
    if (cur_pcb_ptr[visible_terminal] != NULL && cur_pcb_ptr[visible_terminal]->vidmap_terminal == 1)
        vidmap_paging_modify(prev_terminal_video);

    /* switching TO a terminal using vidmap */
    if (cur_pcb_ptr[terminal_num] != NULL && cur_pcb_ptr[terminal_num]->vidmap_terminal == 1)
        vidmap_paging_modify(VIDEO);

    visible_terminal = terminal_num;
    update_cursor();

    return;
}

/** scheduling
 * 
 * Switches context to be able to run a process in the background 
 * 
 * Inputs: None 
 * Outputs: None
 * Side Effects: Processes are being run / handled in background
 * 
 */
void scheduling()
{
    /* make sure pit doesn't occur before first shell runs */
    if (pit_flag == 0) return;

    /** TERMINAL WE ARE SWITCHING AWAY FROM
     *      1. gets video memory of process we are switching away from 
     *      2. saves ESP and EBP of current process if there is a process running
     *      3. switching AWAY from a terminal using vidmap -> switch vidmap
     **/
    uint32_t prev_terminal_video = (process_terminal == 0) ? TERMINAL_ONE_BUFFER : (process_terminal == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER;
    if (cur_pcb_ptr[process_terminal] != NULL)
    {
        asm volatile(
            "movl %%esp, %0;" /* push user_ds */
            "movl %%ebp, %1;"
            : "=r"((cur_pcb_ptr[process_terminal]->esp)), "=r"((cur_pcb_ptr[process_terminal]->ebp)));
    }
    if (cur_pcb_ptr[process_terminal] != NULL && cur_pcb_ptr[process_terminal]->vidmap_terminal == 1) 
        vidmap_paging_modify(prev_terminal_video);


    /** UPDATE TERMINAL 
     *      1. circularly update process terminal
     *      2. execute shell if no processes
    */
    process_terminal = (process_terminal + 1) % 3;
    if (cur_pcb_ptr[process_terminal] == NULL)
    {
        sys_execute("shell");
        return;
    }


    /** TERMINAL WE ARE SWITCHING TO
     *      1. gets video memory of new terminal 
     *      2. sets up program paging
     *      3. set tss for context switch
     *      4. switching TO vidmap -> set vidmap to video  
     *      5. perform context switch in assembly
     **/
    uint32_t new_terminal_video = (process_terminal == 0) ? TERMINAL_ONE_BUFFER : (process_terminal == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER;
    program_paging((cur_pcb_ptr[process_terminal]->process_id * _4MB_PAGE) + _8_MB);

    tss.ss0 = KERNEL_DS;
    tss.esp0 = (uint32_t)(_8_MB - _8_KB * (cur_pcb_ptr[process_terminal]->process_id) - _4_BYTES); 
    
    if (cur_pcb_ptr[process_terminal]->vidmap_terminal == 1 && process_terminal == visible_terminal)
        vidmap_paging_modify(VIDEO);
    else if (cur_pcb_ptr[process_terminal]->vidmap_terminal == 1)
        vidmap_paging_modify(new_terminal_video);

    asm volatile(
        "movl %0, %%esp;" /* push user_ds */
        "movl %1, %%ebp;"
        :
        : "r"((cur_pcb_ptr[process_terminal]->esp)), "r"((cur_pcb_ptr[process_terminal]->ebp)));
}
