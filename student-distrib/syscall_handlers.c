#include "idt_handlers.h"
#include "multiprocessing.h"
#include "syscall_handlers.h"

// /**
//  * Bugs:
//  * Fish on terminal 1 then switch while running (will probably fix when we get video mem working)
//  * Exiting base shell on terminal 2 or 3
//  * 
//  * */


// /***********************PCB related structures and variables***********************/
// // pcb_t *cur_pcb_ptr[visible_terminal] = NULL;
// // rtc fops table
file_ops_t rtc_fops = {_sys_read_rtc, _sys_write_rtc, _sys_open_rtc, _sys_close_rtc};
//file fops table
file_ops_t file_fops = {_sys_read_file, _sys_write_file, _sys_open_file, _sys_close_file};
//directory fops table
file_ops_t dir_fops = {_sys_read_directory, _sys_write_directory, _sys_open_directory, _sys_close_directory};
//stdin fops table
file_ops_t std_in_fops = {_sys_read_terminal, _sys_dummy_write, _sys_dummy_open, _sys_dummy_close};
//stdout fops table
file_ops_t std_out_fops = {_sys_dummy_read, _sys_write_terminal, _sys_dummy_open, _sys_dummy_close};

int8_t buf_executable_header[NUM_TERMINALS][HEADER_INFO];
int read_dir_flag[NUM_TERMINALS] = {0, 0, 0};
int arg_flag[NUM_TERMINALS] = {1, 1, 1};

/** sys_halt
 * Halt System Call 
 * 
 * Inputs: int8_t status of program
 * Outputs: int32_t
 * Side Effects: halts current process, resetting program paging, adjusts tss, and returns to the parent prograam, 
 */
int32_t sys_halt(int8_t status)
{
    // cli();
    int32_t i;
    int32_t term = process_terminal;
    /* closes all file sin PCB */
    // if (cur_pcb_ptr[process_terminal] == NULL) sys_execute("shell");
    // if (cur_pcb_ptr[process_terminal]->parent_pcb == NULL)  
    //     return -1;
    for (i = 0; i < FILE_DESC_ARR_SIZE; i++)
        sys_close(i);
    cur_pcb_ptr[term]->vidmap_terminal = -1;
    /* decrements current process number */
    process_num--;

    /* sets pcb to be the parent's pcb*/
    cli();
    cur_pcb_ptr[term] = cur_pcb_ptr[term]->parent_pcb;
    sti();
    modify_vid_mem(VIDEO);
    vidmap_paging_modify(VIDEO);
    
    /* cant close shell! */
    if (process_num == 0 || cur_pcb_ptr[term] == NULL){
        sys_execute("shell"); 
    } 

    /* sets program paging to previous process */
    program_paging(((cur_pcb_ptr[term]->process_id) * _4MB_PAGE) + _8_MB);

    /* returns to previous program's execution */
    tss.ss0 = KERNEL_DS; // switch stack context
    tss.esp0 = (uint32_t)(_8_MB - ((cur_pcb_ptr[term]->process_id)*_8_KB) - _4_BYTES);
    shell_flag[term] = 1;
    // sti();
    asm volatile(
        "mov %0, %%esp;" /* store esp*/
        "mov %1, %%ebp;" /* store ebp */
        "mov %2, %%eax;" /* store return value */
        "jmp return_from_prog;"
        :
        : "r"(cur_pcb_ptr[term]->esp), "r"(cur_pcb_ptr[term]->ebp), "r"((uint32_t)status)
        );
    return 0;
}

/** sys_execute
 * Execute System Call 
 * 
 * Inputs: int8_t command
 * Outputs: int32_t --> 0 on success, failure if otherwise
 * Side Effects: executes a program by: parsing arguments, checking executable, setting up paging for the program and loading it into memory, switching contexts to user program
 */
int32_t sys_execute(const int8_t *command)
{
    int8_t tempret = 0;
    int8_t prog_name[FILENAME_LEN];
    int8_t arg[KEYBOARD_BUFFER_SIZE];
    uint32_t return_value = 0;
    uint32_t term = visible_terminal;

    /* parses arguments passed into command, stores them in prog_name and arg */
    memset(prog_name, '\0', FILENAME_LEN);
    memset(arg, '\0', KEYBOARD_BUFFER_SIZE);
    tempret = _execute_parse_args(command, prog_name, arg);
    if (tempret == -1)
        return -1;
    /* checks if we are executing from scheduling vs. from command line */
    if (cur_pcb_ptr[process_terminal] == NULL) term = process_terminal;
    arg_flag[term] = 1;
    if (tempret == -2)
        arg_flag[term] = 0;
        
    error_flag[term] = 0;
    if (process_num > 6) return -2;

    /* saves the old base and stack pointer prior to execution to return to later*/
    if (cur_pcb_ptr[term] != NULL)
    {
        asm volatile(
            "mov %%esp, %%eax;" /* push user_ds */
            "mov %%ebp, %%ebx;"
            : "=a"(cur_pcb_ptr[term]->esp), "=b"(cur_pcb_ptr[term]->ebp));
    }


    /* checks that the file is an executable*/
    tempret = _execute_executable_check(prog_name, buf_executable_header[term]);
    if (tempret == -1) return -1;

    /* sets up paging scheme for current program */
    tempret = _execute_setup_program_paging();
    if (tempret == -1) return -1;


    /* loads the user program */
    tempret = _execute_user_program_loader(prog_name);
    if (tempret == -1) return -1;
    if (strncmp(prog_name, "shell", 5) == 0) shell_flag[term] = 1;
    else shell_flag[term] = 0;
    /* creates PCB for process */
    cur_pcb_ptr[term] = _execute_create_PCB(arg, term);
    // process_terminal = term;
    /* switches context to user program */
    _execute_context_switch(term);

    keyboard_cursor_idx[term] = 0;
    memset(keyboard_buffer[term], '\0', KEYBOARD_BUFFER_SIZE);
    /* place for a program to return to while being halted */
    asm volatile(
        "return_from_prog:;" /* push user_ds */
        "leave;"
        "ret;"
        : "=a"(return_value));

    /* checks if there's an error*/
    if (error_flag[term])
    {
        return_value = ERROR_VAL;
    }
    shell_flag[term] = 1;
    return return_value;
}

/** _execute_parse_args
 * Execute System Call Helper 
 * 
 * Inputs: command, name of program, arguments 
 * Outputs: 0 on success, -1 on failure
 * Side Effects: parses the command line arguments for the program
 */
int32_t _execute_parse_args(const int8_t *command, int8_t *prog_name, int8_t *arg)
{
    int i = 0, j, k = 0;
    if (command == NULL || prog_name == NULL || arg == NULL)
        return -1;

    while (command[i] == '\0'){
        i++;
        if(i == KEYBOARD_BUFFER_SIZE) return -1;
    } 
    /* skips spaces in front of program name */
    while (command[i] == ' ' && command[i] != '\0' && command[i] != '\n')
        i++;
    if (command[i] == '\0' || command[i] == '\n')
        return -1; // -1 implies bad command name
    j = i;
    /* copies name of program into prog_name buffer -- only 32 chars */
    while (command[i] != ' ' && command[i] != '\0' && command[i] != '\n' && (i - j) < FILENAME_LEN)
        memcpy(prog_name++, command + (i++), 1);
    /* if there are still chars left in the program name, skip over them */
    while (command[i] != ' ' && command[i] != '\0' && command[i] != '\n')
        i++;
    /* skips spaces in front of argument name */
    while (command[i] == ' ' && command[i] != '\0')
        i++;

    if (command[i] == '\0')
        return -2; // -2 implies no arg
    /* copies arguments into arg buffer */
    while (command[i] != '\0')
        memcpy(&(arg[k++]), command + (i++), 1);

    return 0;
}

/** _execute_executable_check
 * Execute System Call Helper 
 * 
 * Inputs: program mane and a buffer to fill with the header information (for later use) 
 * Outputs: 0 on success, -1 on failure
 * Side Effects: checks that the file is an executable via ELF constant
 * */
int32_t _execute_executable_check(int8_t *prog_name, int8_t *buf)
{
    int ret;
    dentry_t prog_dentry;
  
    /* 4 values @ start signifying executable */
    int8_t elf[4] = {ELF_ONE, ELF_TWO, ELF_THREE, ELF_FOUR};

    /* buffer used for checking */
    memset((void *)buf, '\0', EXECUTABLE_CHECK);

    /* fills buf with 40 bytes of program info, we use read_dentry/data b/c we don't want to move our pointer forward */
    ret = read_dentry_by_name((uint8_t *)prog_name, &prog_dentry);
    if (prog_dentry.file_type != REGULAR_FILE || ret == -1)
        return -1;
    read_data(prog_dentry.inode, 0, (uint8_t *)buf, _4KB_);

    /* checks "ELF" */
    ret = strncmp((const int8_t *)elf, (const int8_t *)buf, EXECUTABLE_CHECK);

    /* checks return value */
    return (ret == 0) ? 0 : -1;
}

/** _execute_setup_program_paging
 * Execute System Call Helper 
 * 
 * Inputs: command, name of program, arguments 
 * Outputs: 0 on success, -1 on failure
 * Side Effects: allocates the appropriate paging attributes for the virtual memory at which the program will be
 */
int32_t _execute_setup_program_paging()
{
    /* sets up paging for the current program */
    program_paging((process_num * _4MB_PAGE) + _8_MB);
    return 0;
}

/** _execute_user_program_loader
 * Execute System Call Helper 
 * 
 * Inputs: name of program
 * Outputs: 0 on success, -1 on failure
 * Side Effects: opens the program and reads teh program into its space in memory
 */
int32_t _execute_user_program_loader(int8_t *prog_name)
{
    /* loads contents of file into program image in virtual memory */
    int32_t inode = _sys_open_file((uint8_t *)prog_name); // inode
    int32_t out;
    
    /* gets size of program */
    dentry_t this_dentry; 
    read_dentry_by_name((const uint8_t*)prog_name, &this_dentry);
    int32_t program_size = (int32_t)inode_head[this_dentry.inode].length;

    memset((void *)PROGRAM_IMAGE, 0, program_size); // changed for fish -- make dynamic based on file size
    // int out = _sys_read_file(fd, (void *) PROGRAM_IMAGE, _4KB_);

    out = read_data(inode, 0, (void *)PROGRAM_IMAGE, program_size);
    /* moves forward in file corresponding to how many bytes are read */

    return (out == -1) ? -1 : 0;
}

/** _execute_create_PCB
 * Execute System Call Helper 
 * 
 * Inputs: none  
 * Outputs: new PCB for the program
 * Side Effects: creates the PCB for this program and stores appropriate information
 */
pcb_t *_execute_create_PCB(char* argument, uint32_t term)
{

    /* creates the PCB for this at the right location */
    pcb_t *new_pcb = (pcb_t *)((int)_8_MB - ((int)_8_KB * (process_num + 1)) - _4_BYTES);

    new_pcb->parent_pcb = cur_pcb_ptr[term];   // sets the parent of PCB for this process
    new_pcb->process_id = process_num++; // gives our process a PID

    /* first 2 file descriptors in PCB are stdin/stdout */
    new_pcb->file_desc_array[0] = (file_desc_t){&std_in_fops, 0, 0, 1};  //SET EQUAL TO STDIN
    new_pcb->file_desc_array[1] = (file_desc_t){&std_out_fops, 0, 0, 1}; //SET EQUAL TO STDOUT

    /* next open location to place a file */
    new_pcb->next_open_index = 2;
    new_pcb->vidmap_terminal = -1;
    new_pcb->rtc_counter = 1;
    new_pcb->rtc_interrupt_divider = 1;
    memset(new_pcb->argument_array, '\0', KEYBOARD_BUFFER_SIZE);
    memcpy(new_pcb->argument_array, argument, strlen(argument));

    if (cur_pcb_ptr[term] == NULL) new_pcb->number = 1;
    else new_pcb->number = (cur_pcb_ptr[term]->number + 1);
    return new_pcb;
}

/** _execute_context_switch
 * Execute System Call Helper 
 * 
 * Inputs: none 
 * Outputs: none
 * Side Effects: switches context to the user program
 */
void _execute_context_switch(uint32_t term)
{
    /* switch TSS context (stack segment and esp) */
    tss.ss0 = KERNEL_DS; // switch stack context
    tss.esp0 =(uint32_t) (_8_MB - (_8_KB * cur_pcb_ptr[term]->process_id) - _4_BYTES); // pointer to the top of stack/pcb
    /* maps the esp of user space */
    uint32_t user_esp = _132_MB; // maps to the esp of user space
    /* gets the eip from the executable header */
    uint32_t eip = 0;
    // buf_executable_header from 27 to 24 used as specified in the documentation. the 24, 16, and 8 are used to shift the bytes to the correct spot in the eip int
    eip += (((uint8_t)buf_executable_header[term][ELF_BYTE_ONE]) << 24) | (((uint8_t)buf_executable_header[term][ELF_BYTE_TWO]) << 16) 
         | ((uint8_t)(buf_executable_header[term][ELF_BYTE_THREE]) << 8) | ((uint8_t)buf_executable_header[term][ELF_BYTE_FOUR]);
    cur_pcb_ptr[term]->eip = eip;
    /* performs context stack in assembly */
    asm volatile(
        "push %0;" /* push user_ds */
        "push %1;" /* push user_esp */
        "pushfl;"  /* push EFLAGS */
        "popl %%eax;"
        "orl  $0x0200, %%eax;" /* enable interrupts */
        "pushl %%eax;"
        "push %%cs;"
        "push %2;" /* push eip value stored in esp */
        "iret;"
        :
        : "r"(USER_DS), "r"(user_esp), "r"(eip)
        : "eax"
        );
}

/** sys_read
 * Read System Call 
 * 
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: number of bytes readud 
 * Side Effects: fills buffer with contents reaud
 */
int32_t sys_read(int32_t fd, void *buf, int32_t nbytes)
{
    // return _sys_read_terminal(fd, buf, nbytes);
    /* checking bounds of the arguments */
    if (nbytes <= 0 || buf == NULL || fd < 0 || fd > MAX_FD_IDX)
        return -1;
    /* checks that the file is active */
    if (cur_pcb_ptr[process_terminal]->file_desc_array[fd].flags == 0)
        return -1;

    /* reads using the correct file operation. we get the inode b/c _sys_read_file/directory use an inode */
    return cur_pcb_ptr[process_terminal]->file_desc_array[fd].file_ops_table->read(fd, buf, nbytes);
}

/** sys_write
 * Write System Call 
 * 
 * Inputs: file descriptor, buffer we write from, number of bytes to write
 * Outputs: bytes written
 * Side Effects: can change RTC frequency or output stuff to screen
 */
int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes)
{

    /* checking bounds of the arguments */
    if (nbytes <= 0 || buf == NULL || fd < 0 || fd > MAX_FD_IDX)
        return -1;
    /* checks that the file is active */
    if (cur_pcb_ptr[process_terminal]->file_desc_array[fd].flags == 0)
        return -1;

    /* reads using the correct file operation. we get the inode b/c _sys_read_file/directory use an inode */
    uint32_t this_inode = cur_pcb_ptr[process_terminal]->file_desc_array[fd].inode;
    return cur_pcb_ptr[process_terminal]->file_desc_array[fd].file_ops_table->write(this_inode, buf, nbytes);
}

/** sys_open()
 * Open System Call
 *  
 * Inputs: int8_t file name
 * Outputs: current pcb index
 * Side Effects: opens the file
 */
int32_t sys_open(const uint8_t *filename)
{ // terminal, rtc, file, directory
    int i;
    int ret_val;
    /* assign the appropriate file descriptor to the PCB */
    for (i = 2; i <= MAX_FD_IDX; i++)
    {
        if (cur_pcb_ptr[process_terminal]->file_desc_array[i].flags == 1)
            continue;
        else
        {
            cur_pcb_ptr[process_terminal]->next_open_index = i;
            break;
        }
    }
    /* if the PCB is full, we fail this */
    if (i > MAX_FD_IDX)
        return -1;
    int32_t cur_pcb_idx = cur_pcb_ptr[process_terminal]->next_open_index;

    /* find the dentry for the file */
    dentry_t this_dentry;
    this_dentry.inode = 0;
    this_dentry.file_type = 0;
    ret_val = read_dentry_by_name(filename, &this_dentry);
    if (ret_val == -1)
        return -1;

    /* figures out which type of file it is */
    file_ops_t *this_fops;
    uint32_t this_inode;
    /* switches based on type of file */
    switch (this_dentry.file_type)
    {
    case RTC_FILE:
        this_fops = &rtc_fops;
        this_inode = _sys_open_rtc(filename); // returns 0
        break;
    case DIRECTORY:
        this_fops = &dir_fops;
        this_inode = _sys_open_directory(filename); // returns 0
        break;
    case REGULAR_FILE:
        this_fops = &file_fops;
        this_inode = _sys_open_file(filename); // returns inode
        break;
    default:
        return -1;
    }
    /* assigns appropriate file descriptor */
    cur_pcb_ptr[process_terminal]->file_desc_array[cur_pcb_idx] = (file_desc_t){this_fops, this_inode, 0, 1};

    return cur_pcb_idx; // index in the file descriptor array
}

/** sys_close
 * Close System Call 
 * 
 * Inputs: int32_t file descriptor of an open file
 * Outputs: return value of close function for the given file
 * Side Effects: PCB flags cleared
 */
int32_t sys_close(int32_t fd)
{
    /* checks bounds and if the file is active */
    if (fd < MIN_FD_IDX || fd > MAX_FD_IDX)
        return -1;
    if (cur_pcb_ptr[process_terminal]->file_desc_array[fd].flags == 0)
        return -1;

    /* marks file as closed in PCB */
    cur_pcb_ptr[process_terminal]->file_desc_array[fd].flags = 0;
    cur_pcb_ptr[process_terminal]->next_open_index = fd;
    /* calls the appropriate system close function */
    return cur_pcb_ptr[process_terminal]->file_desc_array[fd].file_ops_table->close(fd);
}

/** sys_halt
 *  
 * Halt system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_getargs(int8_t *buf, int32_t nbytes)
{
    if (arg_flag[process_terminal] == 0) return -1;
    memset(buf, '\0', KEYBOARD_BUFFER_SIZE);
    memcpy(buf, cur_pcb_ptr[process_terminal]->argument_array, strlen(cur_pcb_ptr[process_terminal]->argument_array));
    return 0;
}

/** sys_vidmap
 *  
 * Returns the pointer to the start of video memory to user
 * Inputs: uint8_t** screen_start, pointer to address of video mem
 * Outputs: 0 for success, -1 for failure
 * Side Effects: Address that screen_start points to gets modified to start of video mem (132 MB)
 */
int32_t sys_vidmap(uint8_t **screen_start)
{
    /* makes sure that screen_start pointer is within the program loading block between 128 MB and 132 MB */
    if ((uint32_t)screen_start < USER_START || (uint32_t)screen_start >= USER_END || (uint32_t)screen_start == NULL)
        return -1;
    /* maps video memory to appropriate terminal */
    if(visible_terminal == process_terminal)        vidmap_paging();
    else{
        uint32_t terminal_address  = (process_terminal == 0) ? TERMINAL_ONE_BUFFER : (process_terminal == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER;
        vidmap_paging_modify(terminal_address);
    }
    /* video memory will be at 132MB */
    (*screen_start) = (uint8_t *)_132_MB; 
    cur_pcb_ptr[process_terminal]->vidmap_terminal = 1;
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
int32_t sys_set_handler(int32_t signum, void *handler_address)
{
    printf("set handler called\n");
    return -1;
}

/** sys_halt
 *  
 * Halt system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_sigreturn(void)
{
    printf("sigreturn called\n");
    return -1;
}

/**
 *  HELPER FUNCTIONS 
 **/
/**
 *  Terminal 
 **/

/** _sys_open_terminal
 *  
 * terminal helper function for system open
 * Inputs: same as sys_open
 * Outputs: 0
 * Side Effects: 
 */
int32_t _sys_open_terminal(const uint8_t *filename)
{
    return 0;
}
/** _sys_close_terminal
 *  
 * Terminal helper function for system close
 * Inputs: same as sys_close
 * Outputs: 0 
 * Side Effects: 
 */
int32_t _sys_close_terminal(int32_t fd)
{
    return 0;
}
/** _sys_read_terminal
 *  
 * Terminal helper function for system read
 * Inputs: same as sys_read
 * Outputs: number of bytes read
 * Side Effects: none
 */
int32_t _sys_read_terminal(int32_t fd, void *buf, int32_t nbytes)
{
    /* check edge cases */
    // sti();
    uint32_t retval = 0;
    int i = 0;
    int32_t term = process_terminal;
    if (NULL == buf || nbytes < 0)
        return -1;
    if (nbytes == 0)
        return 0;

    memset(keyboard_buffer[term], '\0', KEYBOARD_BUFFER_SIZE);

    if (re_echo_flag[term] == 1){
        keyboard_cursor_idx[term] = temp_kbd_idx[term];
        keyboard_buffer_end_idx[term] = keyboard_cursor_idx[term];
        memcpy(keyboard_buffer[term], temp_kbd_buf[term], keyboard_cursor_idx[term]);
        while (i < keyboard_cursor_idx[term]){
            putc(keyboard_buffer[term][i]);
            i++;
        }
        re_echo_flag[term] = 0;
    }

    /* clears the keyboard buffer */
    /* adjusts nbytes if overflow */
    i = 0;
    nbytes = (nbytes > KEYBOARD_BUFFER_SIZE) ? KEYBOARD_BUFFER_SIZE - 1 : nbytes;

    /* reads data/fills buffer from keyboard */
    sys_read_flag[term] = 1;
    while (sys_read_flag[term] || process_terminal != term); // keep track of which terminal called read so that you can differentiate
    // cli();
    /* copies memory from keyboard input to buffer */
    while (keyboard_buffer[term][i] != '\0' && keyboard_buffer[term][i] != '\n' && keyboard_buffer[term][i] != '\0' && i < nbytes)
    {
        memcpy(&(((char *)buf)[i]), &(keyboard_buffer[term][i]), 1);
        retval++;
        i++;
    }
    ((char *)buf)[i] = '\n';
    // while (visible_terminal != process_terminal);
    return retval + 1;
}
/** _sys_write_terminal
 *  
 * Terminal helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_write_terminal(int32_t fd, const void *buf, int32_t nbytes)
{
    int i, bytes_written;
    char write_string[nbytes];
    // int term = process_terminal;
    /* check edge cases */
    if (NULL == buf || nbytes < 0)
        return -1;
    if (nbytes == 0)
        return 0;
    cli();
    /* put passed in buffer into an appropriately sized buffer */
    memset(write_string, '\0', nbytes);
    memcpy(write_string, buf, nbytes);
    
    /* prints all non-null characters */
    bytes_written = 0;
    for (i = 0; i < nbytes; i++)
    {
        /* writes non null characters */
        if (write_string[i] != '\0')
        {
            putc(write_string[i]);
            bytes_written++;
        }
    }
    sti();
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
int32_t _sys_open_rtc(const uint8_t *filename)
{
    int freq[1] = {2};
    _sys_write_rtc(NULL, (void *)freq, 4); // sets the RTC frequency to 2Hz
    return 0;
}

/** _sys_close_RTC
 *  
 * RTC helper function for system close
 * Inputs: same as sys_close
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_close_rtc(int32_t fd)
{
    return 0;
}

/** sys_write_rtc
 *  
 * RTC helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: sets RTC rate
 */
int32_t _sys_write_rtc(int32_t fd, const void *buf, int32_t nbytes)
{
    // sets the RTC rate to 2Hz
    int frequency;
    if (nbytes != _4_BYTES || buf == NULL)
        return -1;
    frequency = *((int32_t *)buf);

    /* param check */
    if (power_of_two(frequency) || frequency < 0)
        return -1;
    cli();
    // gets frequency from buffer
    if (frequency > MAX_INTERRUPT_FREQUENCY)
    {
        frequency = MAX_INTERRUPT_FREQUENCY;
    }
    /* figure out what multiple of the max frequency the program wants */
    int count = MAX_INTERRUPT_FREQUENCY / frequency;
    // int count = (MAX_INTERRUPT_FREQUENCY / frequency / 2) <= 1 ? 1 : (MAX_INTERRUPT_FREQUENCY / frequency / 2);
    cur_pcb_ptr[process_terminal]->rtc_interrupt_divider = count;
    cur_pcb_ptr[process_terminal]->rtc_counter = count;
    sti();
    return 0;
}

/** _sys_read_rtc
 *  
 * RTC helper function for system read
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: 0 when an RTC interrupt occurs
 * Side Effects: none
 */
int32_t _sys_read_rtc(int32_t fd, void *buf, int32_t nbytes)
{
    // sti();
    /* with virtualized RTC, keep reading until enough interrupts have occurred for desired frequency */
    while (cur_pcb_ptr[process_terminal]->rtc_counter > 0);
    cur_pcb_ptr[process_terminal]->rtc_counter = cur_pcb_ptr[process_terminal]->rtc_interrupt_divider;
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
int32_t _sys_open_file(const uint8_t *filename)
{
    // dentry_t * this_file; // ***************ADD RETURN VAL CHECKING *****************
    // this_file->inode = -1;
    uint8_t file[FILENAME_LEN];
    int i;
    int ret_val;
    for (i = 0; i < FILENAME_LEN; i++)
    {
        if (filename[i] == '\0')
            break;
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
    if (ret_val == -1)
        return -1;
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
int32_t _sys_close_file(int32_t fd)
{
    return 0;
}
/** _sys_read_file
 *  
 * Terminal helper function for system read
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: 0 if success (read nbytes), -1 if fail (read < nbytes)
 * Side Effects: moves forward in file
 */
int32_t _sys_read_file(int32_t fd, void *buf, int32_t nbytes)
{
    int32_t data_read;
    int32_t offset = cur_pcb_ptr[process_terminal]->file_desc_array[fd].file_position;
    /* condition checks */
    if (nbytes <= 0 || buf == NULL || fd < 0)
        return -1;
    /* does the data read */
    data_read = read_data(cur_pcb_ptr[process_terminal]->file_desc_array[fd].inode, offset, buf, nbytes);
    /* moves forward in file corresponding to how many bytes are read */
    if (data_read > 0)
        cur_pcb_ptr[process_terminal]->file_desc_array[fd].file_position += data_read;
    /* checks that we have read ALL bytes requested */
    return data_read;
}
/** sys_write_file
 *  
 * RTC helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: sets RTC rate
 */

int32_t _sys_write_file(int32_t fd, const void *buf, int32_t nbytes)
{
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
int32_t _sys_open_directory(const uint8_t *filename)
{
    return 0;
}
/** _sys_close_directory
 *  
 * Directory helper function for system close
 * Inputs: same as sys_close
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_close_directory(int32_t fd)
{
    return 0;
}
/** _sys_read_directory
 *  
 * Reads the directory and puts all the filenames in a buffer
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: Returns number of bytes read 
 * Side Effects: Buffer holds a filename from the directory every 32 bytes
 */
int32_t _sys_read_directory(int32_t fd, void *buf, int32_t nbytes)
{
    dentry_t curr_dentry;
    if (read_dir_flag[process_terminal] >= boot_block->entries)
    {
        read_dir_flag[process_terminal] = 0;
        return 0;
    }
    read_dentry_by_index(read_dir_flag[process_terminal]++, &curr_dentry);
    strncpy((int8_t *)(buf),
            (int8_t *)curr_dentry.file_name, FILENAME_LEN);
    return FILENAME_LEN;
}
/** sys_write_directory
 *  
 * RTC helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: sets RTC rate
 */

int32_t _sys_write_directory(int32_t fd, const void *buf, int32_t nbytes)
{
    return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** _sys_dummy_read(int32_t fd, void* buf, int32_t nbytes)
 *  
 * Dummy function for stdout
 * Inputs: same as sys_read
 * Outputs: -1 
 * Side Effects: none
 */
int32_t _sys_dummy_read(int32_t fd, void *buf, int32_t nbytes)
{
    return -1;
}

/** _sys_dummy_write(int32_t fd, void* buf, int32_t nbytes)
 *  
 * Dummy function for stdin
 * Inputs: same as sys_write
 * Outputs: -1 
 * Side Effects: none
 */
int32_t _sys_dummy_write(int32_t fd, const void *buf, int32_t nbytes)
{
    return -1;
}

/** _sys_dummy_open(const int8_t* filename)
 *  
 * Dummy function for stdin/stdout
 * Inputs: same as sys_open
 * Outputs: -1 
 * Side Effects: none
 */
int32_t _sys_dummy_open(const uint8_t *filename)
{
    return -1;
}

/** _sys_dummy_close(int32_t fd)
 *  
 * Dummy function for stdin/stdout
 * Inputs: same as sys_close
 * Outputs: -1 
 * Side Effects: none
 */
int32_t _sys_dummy_close(int32_t fd)
{
    return -1;
}
