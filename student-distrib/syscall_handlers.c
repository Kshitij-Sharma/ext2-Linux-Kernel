#include "syscall_handlers.h"

pcb_t * curr_pcb_ptr = NULL;

// rtc fops table
file_ops_t rtc_fops = {_sys_read_rtc, _sys_write_rtc, _sys_open_rtc, _sys_close_rtc};
//stdin fops table
file_ops_t std_in = {_sys_read_terminal, _sys_dummy_read_write, _sys_dummy_open, _sys_dummy_close};
//stdout fops table
file_ops_t std_out = {_sys_dummy_read_write, _sys_write_terminal, _sys_dummy_open, _sys_dummy_close};
//file fops table
file_ops_t file_fops = {_sys_read_file, _sys_write_file, _sys_open_file, _sys_close_file};
//directory fops table
file_ops_t dir_fops = {_sys_read_directory, _sys_write_directory, _sys_open_directory, _sys_close_directory};

/** sys_halt
 *  
 * Halt system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_halt (int8_t status){
    printf("halt called\n");
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
int32_t sys_execute (const int8_t* command){
    // printf("execute called\n");
    // int8_t tempret;
    // int8_t prog_name[32];
    // int8_t arg[128];

    // memset(prog_name, '\0', 32);
    // memset(arg, '\0', 128);
    
    // tempret = _execute_parse_args(command, prog_name, arg);
    // if(tempret == -1)   return -1;
    
    // tempret = _execute_executable_check(prog_name);
    // if(tempret == -1)   return -1;

    // tempret = _execute_setup_program_paging();
    // if(tempret == -1)   return -1;

    // int8_t* buf;
    // _execute_executable_check(prog_name, buf);
    /** Execute:
     * - user level tasks share a common mapping for kernel page
     * - only one page needed for each tasks' use memory
     * 1. parse arguments (DONE)
     * 2. check if executable (DONE)
     * 3. set up paging for the program
     *  - all user programs 128MB in virtual mem
     *  - physical memory for programs start at 8MB + (process no * 4MB)
     *  - flush TLB after page stop
     * 4. program loader
     *  - checks ELF constant (done in executable check)
     *  - copies file contents to correct location
     *      - fs driver should copy a program image from random 4kB disk block in the file system into contiguous physica mem
     *  - needs to correctly  
     *  - finds addr of first instruction
    */
    return 0;
}

int32_t _execute_parse_args(int8_t* command, int8_t* prog_name, int8_t* arg){
    if(command == NULL || prog_name == NULL || arg == NULL)  return -1;

    /* skips spaces in front of program name */
    while (*command == ' ' && *command != '\0') command++;
    if (*command == '\0') return -1; // -1 implies bad command name
    /* copies name of program into prog_name buffer */
    while (*command != ' ' && *command != '\0') memcpy(prog_name++, command++, 1);
    /* skips spaces in front of argument name */
    while (*command == ' ' && *command != '\0') command++;

    if (*command == '\0') return -2; // -2 implies no arg
    /* copies arguments into arg buffer */
    while (*command != ' ' && *command != '\0') memcpy(arg++, command++, 1);

    return 0;
}

int32_t _execute_executable_check(int8_t * prog_name, int8_t * buf){
    int ret;
    dentry_t prog_dentry;
    // prog_dentry.file_name = 0;
    // prog_dentry.file_type = 0;
    /* 4 values @ start signifying executable */ 
    int8_t elf[4] = {0x7f, 0x45, 0x4c, 0x46};
    /* buffer used for checking */
    memset((void *)buf, '\0', EXECUTABLE_CHECK);
    
    /* fills buf with 40 bytes of program info, we use read_dentry/data b/c we don't want to move our pointer forward */
    ret = read_dentry_by_name(prog_name, &prog_dentry);
    if(prog_dentry.file_type != REGULAR_FILE || ret == -1)   return -1;
    read_data(prog_dentry.inode, 0, (uint8_t *) buf, HEADER_INFO);
 
    /* checks "ELF" */ 
    ret = strncmp((const int8_t*) elf, (const int8_t*) buf, EXECUTABLE_CHECK);

    /* checks return value */
    return (ret == 0) ? 0 : -1;

  }

int32_t _execute_setup_program_paging(){
    program_paging((process_num * _4MB_PAGE) + USER_START);
    flush_tlb();
    return 0;
}

int32_t _execute_user_program_loader(int8_t * prog_name){
    // put the contents of the file into memory location 0x8048000
    int32_t fd = _sys_open_file(prog_name);
    int out = _sys_read_file(fd, PROGRAM_IMAGE, _4KB_);
    return (out == -1) ? -1 : 0;
}

pcb_t * _execute_create_PCB(){
    file_desc_t stdin;
    file_desc_t stdout;
    pcb_t new_pcb;
    new_pcb.parent_pcb = curr_pcb_ptr;
    new_pcb.process_id = process_num++;
    stdin.file_ops_table = &std_in;
    stdout.file_ops_table = &std_out;
    new_pcb.file_desc_array[0] = stdin;//SET EQUAL TO STDIN
    new_pcb.file_desc_array[1] = stdout;//SET EQUAL TO STDOUT
    new_pcb.next_open_index = 2;
    curr_pcb_ptr = &new_pcb;
    return &new_pcb;
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
    printf("read called\n");
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
    printf("write called\n");
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
    printf("open called\n");
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
    printf("close called\n");
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
    printf("getargs called\n");
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
    printf("vidmap called\n");
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
    printf("set handler called\n");
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
    printf("sigreturn called\n");
    return 0;
}





/**
 *  HELPER FUNCTIONS 
 **/
/**
 *  Terminal 
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
/** _sys_write_terminal
 *  
 * Terminal helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_write_terminal (int32_t fd, void* buf, int32_t nbytes){
    int i, bytes_written;
    char write_string[nbytes];
    int enter_flag = 0;

    /* check edge cases */
    if(NULL == buf || nbytes < 0)       return -1;
    if(nbytes == 0)                     return 0;
    /* adjusts nbytes if overflow */
    // nbytes = (nbytes > KEYBOARD_BUFFER_SIZE) ? KEYBOARD_BUFFER_SIZE : nbytes;

    /* put passed in buffer into an appropriately sized buffer */
    memset(write_string, NULL, nbytes); 
    memcpy(write_string, buf, nbytes);

    /* prints all non-null characters */
    bytes_written = 0;
    for(i = 0; i < nbytes; i++) { 
        /* writes non null characters */   
        if(write_string[i] != NULL)
        {
            if (enter_flag)  continue;
            putc(write_string[i]);
            bytes_written++;
        }                 
        if (write_string[i] == '\n') enter_flag = 1;
    }
    if (!enter_flag) putc('\n');

    return bytes_written;
}




/**
 *  RTC  HELPERS 
 **/
/** _sys_open_RTC
 *  
 * RTC helper function for system open
 * Inputs: same as sys_open
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_open_rtc (const int8_t* filename){
    int freq = 2;
    _sys_write_rtc(NULL, (void *) freq, 0); // sets the RTC frequency to 2Hz
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
    
    int32_t frequency = (int32_t) buf;
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




/**
 *  FILE  HELPERS 
 **/
/** _sys_open_file
 *  
 * File helper function for system open
 * Inputs: same as sys_open
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_open_file (const int8_t* filename){ 
    // dentry_t * this_file; // ***************ADD RETURN VAL CHECKING *****************
    // this_file->inode = -1;
    int8_t file[32];
    int i;
    int ret_val;
    for (i = 0; i < 32; i++)
    { 
        if(filename[i] == '\0')        break;
        file[i] = filename[i];
    }
    file[i] = '\0';
    /* sets up a blank dentry to be filled by read */
    dentry_t this_file;
    this_file.inode = 0;
    this_file.file_type = 0;
    data_bytes_read = 0;
    
    /* populates dentry with read call */
    ret_val = read_dentry_by_name(file, &this_file); 
     if (ret_val == -1)         return -1;
    /* returns inode number with associated file */
    return this_file.inode;
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
/** _sys_read_file
 *  
 * Terminal helper function for system read
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: 0 if success (read nbytes), -1 if fail (read < nbytes)
 * Side Effects: moves forward in file
 */
int32_t _sys_read_file (int32_t fd, void* buf, int32_t nbytes){
    int32_t data_read;
    /* condition checks */
    if(nbytes <= 0 || buf == NULL || fd < 0) return -1;
    /* does the data read */
    data_read = read_data(fd, data_bytes_read, buf, nbytes);
    /* moves forward in file corresponding to how many bytes are read */
    if(data_read > 0)                       data_bytes_read += data_read;
    /* checks that we have read ALL bytes requested */
    // if(data_read != nbytes)                 return -1;
    return data_read;
}
/** sys_write_file
 *  
 * RTC helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: sets RTC rate
 */

int32_t _sys_write_file (int32_t fd, void* buf, int32_t nbytes){
    return -1;
}



/**
 *  DIRECTORY  HELPERS 
 **/
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
/** _sys_read_directory
 *  
 * Reads the directory and puts all the filenames in a buffer
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: Returns number of bytes read 
 * Side Effects: Buffer holds a filename from the directory every 32 bytes
 */
int32_t _sys_read_directory (int32_t fd, void* buf, int32_t nbytes){
    dentry_t curr_dentry;
    read_dentry_by_index(fd, &curr_dentry);
    strncpy((int8_t*) ((fd * FILENAME_LEN) + buf), 
            (int8_t*) curr_dentry.file_name, FILENAME_LEN);
    return 0;
}
/** sys_write_directory
 *  
 * RTC helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: sets RTC rate
 */

int32_t _sys_write_directory (int32_t fd, void* buf, int32_t nbytes){
    return -1;
}






















////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
