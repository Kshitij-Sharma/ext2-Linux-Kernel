#include "syscall_handlers.h"


// // rtc fops table
// file_ops_t rtc_fops = {_sys_read_rtc, _sys_write_rtc, _sys_open_rtc, _sys_close_rtc};
// //stdin fops table
// file_ops_t std_in = {_sys_read_terminal, _sys_dummy_read_write, _sys_dummy_open, _sys_dummy_close};
// //stdout fops table
// file_ops_t std_out = {_sys_dummy_read_write, _sys_write_terminal, _sys_dummy_open, _sys_dummy_close};
// //file fops table
// file_ops_t file_fops = {_sys_read_file, _sys_write_file, _sys_open_file, _sys_close_file};
// //directory fops table
// file_ops_t dir_fops = {_sys_read_directory, _sys_write_directory, _sys_open_directory, _sys_close_directory};

/** sys_halt
 *  
 * Halt system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_halt (int8_t status){
    return 0; 
}
int32_t sys_execute (const int8_t* command){
    return 0;
}

/** sys_read
 *  
 * Read system call - reads from terminal, RTC, directory, or file
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: number of bytes read
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_read (int32_t fd, void* buf, int32_t nbytes){
    return 0;
}

/** sys_write
 *  
 * Halt system call - write rate to RTC or terminal 
 * Inputs: file descriptor, buffer we write from, number of bytes to write
 * Outputs: int32_t
 * Side Effects: can change RTC frequency or output stuff to screen
 * NOT YET IMPLEMENTED
 */
int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes){
    return 0;
}

/** sys_open
 *  
 * Open system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_open (const int8_t* filename){
    return 0;
}

/** sys_halt
 *  
 * Halt system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_close (int32_t fd){
    return 0;
}

/** sys_halt
 *  
 * Halt system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_getargs(int8_t* buf, int32_t nbytes){
    return 0;
}

/** sys_halt
 *  
 * Halt system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_vidmap (int8_t** screen_start){
    return 0;
}

/** sys_halt
 *  
 * Halt system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_set_handler (int32_t signum, void* handler_address){
    return 0;
}

/** sys_halt
 *  
 * Halt system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_sigreturn (void){
    return 0;
}


/**
 *  HELPER FUNCTIONS 
 **/


/**
 *  READ HELPERS 
 **/

/** _sys_read_terminal
 *  
 * Terminal helper function for system read
 * Inputs: same as sys_read
 * Outputs: number of bytes read
 * Side Effects: none
 */
int32_t _sys_read_terminal (int32_t fd, void* buf, int32_t nbytes){
    
    /* check edge cases */
    if(NULL == buf || nbytes < 0)       return -1;
    if(nbytes == 0)                     return 0;

    /* clears the keyboard buffer */
    memset(keyboard_buffer, NULL, KEYBOARD_BUFFER_SIZE);
    /* adjusts nbytes if overflow */
    nbytes = (nbytes > KEYBOARD_BUFFER_SIZE) ? KEYBOARD_BUFFER_SIZE: nbytes;

    /* reads data/fills buffer from keyboard */
    sys_read_flag = 1;
    while(sys_read_flag);
    
    /* copies memory from keyboard input to buffer */
    memcpy(buf, keyboard_buffer, nbytes);

    return nbytes;
}
/** _sys_read_rtc
 *  
 * RTC helper function for system read
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: 0 when an RTC interrupt occurs
 * Side Effects: none
 */
int32_t _sys_read_rtc (int32_t fd, void* buf, int32_t nbytes){
    RTC_READ_FLAG = 1;      // sets a global flag
    while(RTC_READ_FLAG);   // waits for interrupt
    // printf("read rtc\n");
    return 0;
}
/** _sys_read_file
 *  
 * Terminal helper function for system read
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: number of bytes read
 * Side Effects: none
 */
int32_t _sys_read_file (int32_t fd, void* buf, int32_t nbytes){
    int32_t data_read;
    if(nbytes <= 0 || buf == NULL || fd < 0) return -1;
    data_read = read_data(fd, data_bytes_read, buf, nbytes);
    if(data_read > 0){
        data_bytes_read += data_read;
        return 0;
    }
    return -1;
}
/** _sys_read_directory
 *  
 * Directory helper function for system read
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: 0 @ directory end
 * Side Effects: none
 */
int32_t _sys_read_directory (int32_t fd, void* buf, int32_t nbytes){
 return 0;
}

/**
 *  WRITE HELPERS 
 */
/** _sys_write_terminal
 *  
 * Terminal helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_write_terminal (int32_t fd, void* buf, int32_t nbytes){
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

    /* prints all non-null characters */
    for(i = 0; i < nbytes; i++) {    
        if(write_string[i] != NULL)                 putc(write_string[i]);
    }

    return nbytes;
}
/** sys_write_rtc
 *  
 * RTC helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: sets RTC rate
 */

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
/** sys_write_rtc
 *  
 * RTC helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: sets RTC rate
 */

int32_t _sys_write_file (int32_t fd, void* buf, int32_t nbytes){
    return 0;
}
/** sys_write_rtc
 *  
 * RTC helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: sets RTC rate
 */

int32_t _sys_write_directory (int32_t fd, void* buf, int32_t nbytes){
    return 0;
}

/**
 *  OPEN HELPERS 
 **/

/** _sys_open_file
 *  
 * File helper function for system open
 * Inputs: same as sys_open
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_open_terminal (const int8_t* filename){
    return 0;
}
/** _sys_open_file
 *  
 * File helper function for system open
 * Inputs: same as sys_open
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_open_file (const int8_t* filename){ 
    dentry_t* this_file;
    data_bytes_read = 0;
    read_dentry_by_name(filename, this_file); 
    return this_file->inode;
}

/** _sys_open_directory
 *  
 * Directory helper function for system write
 * Inputs: same as sys_open
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_open_directory (const int8_t* filename){
    return 0;
}
/** _sys_open_RTC
 *  
 * RTC helper function for system open
 * Inputs: same as sys_open
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_open_rtc (const int8_t* filename){
    return 0;
}

/**
 *  CLOSE HELPERS 
 **/

/** _sys_close_file
 *  
 * File helper function for system close
 * Inputs: same as sys_close
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_close_terminal(int32_t fd){
    return 0;
}
/** _sys_close_file
 *  
 * File helper function for system close
 * Inputs: same as sys_close
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_close_file(int32_t fd){
    return 0;
}
/** _sys_close_RTC
 *  
 * RTC helper function for system close
 * Inputs: same as sys_close
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_close_rtc(int32_t fd){
    return 0;
}
/** _sys_close_directory
 *  
 * Directory helper function for system close
 * Inputs: same as sys_close
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_close_directory(int32_t fd){
    return 0;
}

int32_t _sys_dummy_read_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

int32_t _sys_dummy_open(const int8_t* filename){
    return -1;
}

int32_t _sys_dummy_close(int32_t fd){
    return -1;
}
/* @TODO:
figure out the cursor _
move on and help kshitij 
dont go back up the line if you hit enter on it
dont delete all the blanks, just go to text
*/
