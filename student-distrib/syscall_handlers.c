#include "syscall_handlers.h"

int32_t sys_halt (int8_t status){
    return 0;
}
int32_t sys_execute (const int8_t* command){
    return 0;
}
// fd is offset from the start of the file that we want to start reading
// buf is the buffer we are reading the data into
// nbytes is the number of bytes we would like to read
int32_t sys_read (int32_t fd, void* buf, int32_t nbytes){
    // if one of the params is invalid return 0 

    // handles side cases
    // if(nbytes == 0)     return 0;
    // if(NULL == buf)     return -1;

    // /* go to the appropriate read helper function */
    // switch(sys_read_type)
    // {
    //     case 't':
    //         return _sys_read_terminal(buf, nbytes);
    //         break;
    //     case 'f':
    //         return _sys_read_filesystem(fd, buf, nbytes);
    //         break;
    //     case 'r':
    //         return _sys_read_rtc(fd, buf, nbytes);
    //         break;
    //     default:
    //         return -1;
    // }

    // keyboard_buffer_index = 0; // resets index to 0
    // memset(keyboard_buffer, 0, KEYBOARD_BUFFER_SIZE);    // clears buffer
    return 0;
}


int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes){
    
    // /* check side cases */
    // if(n_bytes == 0)                            return 0;
    // if (NULL == buf)      return -1;
    // if (nbytes > KEYBOARD_BUFFER_SIZE) nbytes = KEYBOARD_BUFFER_SIZE; 

    // switch(sys_write_type)
    // {
    //     case 't':
    //         return _sys_write_terminal(buf, nbytes);
    //         break;
    //     case 'f':
    //         return _sys_write_filesystem(fd, buf, nbytes);
    //         break;
    //     case 'r':
    //         return _sys_rwrite_rtc(fd, buf, nbytes);
    //         break;
    //     default:
    //         return -1;
    // }
    return 0;
}

int32_t sys_open (const int8_t* filename){
    return 0;
}

int32_t sys_close (int32_t fd){
    return 0;
}

int32_t sys_getargs(int8_t* buf, int32_t nbytes){
    return 0;
}

int32_t sys_vidmap (int8_t** screen_start){
    return 0;
}

int32_t sys_set_handler (int32_t signum, void* handler_address){
    return 0;
}

int32_t sys_sigreturn (void){
    return 0;
}


/* HELPER FUNCTIONS */
    // reads data from keyboard, RTC, file, or directory
    // return: number of bytes read
    // KB: 
        // return data from one line terminated by enter (or if buffer is full)
        // line returned should include "line feed" character
    // file:
        // data should be read to end of file or end of provided buffer (buf)
    // directory:
        // provide filename
        // subsequent reads should read from successive entries until the last one
        // @ last one: continue returning 0
    // RTC
        // always return 0 after interrupt (set flag --> wait until cleared by handler --> return 0)
    // use jump table to go to appropriate place
/* only called when the enter key is pressed */
int32_t _sys_read_terminal (int32_t fd, void* buf, int32_t nbytes){
    
    /* check edge cases */
    if(NULL == buf || nbytes < 0)       return -1;
    if(nbytes == 0)                     return 0;

    /* clears the keyboard buffer */
    memset(keyboard_buffer, NULL, KEYBOARD_BUFFER_SIZE);
    /* adjusts nbytes if overflow */
    nbytes = (nbytes > KEYBOARD_BUFFER_SIZE) ? KEYBOARD_BUFFER_SIZE: nbytes;

    // cli();
    // if(!sys_kb_flag)

    /* reads data/fills buffer from keyboard */
    sys_read_flag = 1;
    while(sys_read_flag);
    
    /* copies memory from keyboard input to buffer */
    memcpy(buf, keyboard_buffer, nbytes);
    
    // memmove(keyboard_buffer, keyboard_buffer + nbytes, KEYBOARD_BUFFER_SIZE - nbytes);
    // memset(keyboard_buffer + (KEYBOARD_BUFFER_SIZE - nbytes), ' ', nbytes);
    // keyboard_buffer_index = (KEYBOARD_BUFFER_SIZE - nbytes);

    // sti();
    return nbytes;
}

// int32_t _sys_read_filesystem (int32_t fd, void* buf, int32_t nbytes){

// }

int32_t _sys_read_rtc (int32_t fd, void* buf, int32_t nbytes){
    
    return 0;
}

int32_t _sys_write_terminal (int32_t fd, void* buf, int32_t nbytes)
{
    int i;
    char write_string[KEYBOARD_BUFFER_SIZE];

    /* check edge cases */
    if(NULL == buf || nbytes < 0)       return -1;
    if(nbytes == 0)                     return 0;
    /* adjusts nbytes if overflow */
    nbytes = (nbytes > KEYBOARD_BUFFER_SIZE) ? KEYBOARD_BUFFER_SIZE : nbytes;

    /* put passed in buffer into an appropriately sized buffer */
    memset(write_string, NULL, KEYBOARD_BUFFER_SIZE); 
    memcpy(write_string, buf, nbytes);

    // write_string[KEYBOARD_BUFFER_SIZE-1] = '\n';

    for(i = 0; i < nbytes; i++) {    
        if(write_string[i] != NULL)                 putc(write_string[i]);
    }

    return nbytes;
}

int32_t _sys_write_rtc(int32_t fd, void* buf, int32_t nbytes){
    // sets the RTC rate to 2Hz
    char prev;
    int rate;
    int frequency = nbytes;
    
    /* param check */
    // if (buf == NULL)                    return -1;
    if (power_of_two(frequency) || frequency < 0)              return -1;
    
    // gets frequency from buffer
    // frequency = (int) buf[0];
    if (frequency > MAX_INTERRUPT_FREQUENCY){
        frequency = MAX_INTERRUPT_FREQUENCY;
    }
    rate = (log_base_two(FREQ_CONVERSION_CONST/frequency)/log_base_two(2)) + 1;
    if (rate <= 2) rate = 3;

    // MAGIC NUMBER: 0x0F sets rate selector bits in register A
    outb(RTC_STATUS_REGISTER_A, RTC_CMD_PORT);          // set index to register A, disable NMI
    prev = inb(RTC_DATA_PORT);                          // get initial value of register A
    outb(RTC_STATUS_REGISTER_A, RTC_DATA_PORT);         // reset index to A
    outb(((prev & 0xF0) | rate), RTC_DATA_PORT);        // writes rate (bottom 4 bits) to A
    // MAGIC NUMBER: 0xF0 is used to clear bottom 4 bits before setting rate
    return 0;
}

/* @TODO:
figure out the cursor _
move on and help kshitij 
dont go back up the line if you hit enter on it
dont delete all the blanks, just go to text
*/