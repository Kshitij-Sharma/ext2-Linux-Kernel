#include "syscall_handlers.h"
#include "multiprocessing.h"

uint32_t terminal_id = 0;
pcb_t* cur_pcb_ptr[NUM_TERMINALS] = {NULL, NULL, NULL};

/** switch_terminal
 * 
 * Inputs: int32_t terminal_num
 * Outputs: none
 * Side Effects: switches terminals on screen, swaps video memory
 * 
 */

void switch_terminal(int32_t terminal_num)
{
    /* error checking */
    if(terminal_num < 0 || terminal_num >= NUM_TERMINAL)    return; 
    /* gets the video buffer address of the current terminal */
    uint32_t prev_terminal_video = (terminal_id == 0) ? TERMINAL_ONE_BUFFER : (terminal_id == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER; 
    /* gets the video buffer address of the terminal we are switching to */
    uint32_t new_terminal_video = (terminal_num == 0) ? TERMINAL_ONE_BUFFER : (terminal_num == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER; 
    /* copies current video memory contents into the buffer for the previous one */
    memcpy((void*)prev_terminal_video, (void*)VIDEO, _4KB_PAGE);
    /* clears video memory */
    memset((void*)VIDEO, '\0', _4KB_PAGE);
    /* copy the terminal we are swtiching to's buffer into actual video memory */
    memcpy((void*)VIDEO, (void*)new_terminal_video, _4KB_PAGE);
    terminal_id = terminal_num;
    update_cursor();
    // sys_execute("shell");
    return;
}