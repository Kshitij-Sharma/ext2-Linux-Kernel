#include "idt_handlers.h"
#include "multiprocessing.h"
#include "syscall_handlers.h"

/* ---------------------- PCB related structures and variables -------------------------- */

/* rtc fops table */
file_ops_t rtc_fops = {_sys_read_rtc, _sys_write_rtc, _sys_open_rtc, _sys_close_rtc};
/* file fops table */
file_ops_t file_fops = {_sys_read_file, _sys_write_file, _sys_open_file, _sys_close_file};
/* directory fops table */
file_ops_t dir_fops = {_sys_read_directory, _sys_write_directory, _sys_open_directory, _sys_close_directory};
/* stdin fops table */
file_ops_t std_in_fops = {_sys_read_terminal, _sys_dummy_write, _sys_dummy_open, _sys_dummy_close};
/* stdout fops table */
file_ops_t std_out_fops = {_sys_dummy_read, _sys_write_terminal, _sys_dummy_open, _sys_dummy_close};

int8_t buf_executable_header[NUM_TERMINALS][HEADER_INFO];
int32_t read_dir_flag[NUM_TERMINALS] = {0, 0, 0};
int32_t arg_flag[NUM_TERMINALS] = {1, 1, 1};
uint32_t active_processes[MAX_NUM_PROCESSES] = {0, 0, 0, 0, 0, 0};

/** sys_halt
 * Halt System Call 
 * 
 * Inputs: int8_t status of program
 * Outputs: int32_t
 * Side Effects: halts current process, resetting program paging, adjusts tss, and returns to the parent prograam, 
 */
int32_t sys_halt(int8_t status) {
    int32_t i;
    too_many_shells_flag = 0;
    
    /* closes all file sin PCB */
    for (i = MIN_FD_IDX; i < FILE_DESC_ARR_SIZE; i++)
        if (cur_pcb_ptr[process_terminal]->file_desc_array[i].flags) sys_close(i);

    cur_pcb_ptr[process_terminal]->vidmap_terminal = 0;
    /* makes the process inactive */
    active_processes[cur_pcb_ptr[process_terminal]->process_id] = 0;

    /* sets pcb to be the parent's pcb*/
    cur_pcb_ptr[process_terminal] = cur_pcb_ptr[process_terminal]->parent_pcb;

    /* re-does vidmap to main video mem, just in case */
    vidmap_paging_modify(VIDEO);

    /* cant close shell! */
    if (cur_pcb_ptr[process_terminal] == NULL) sys_execute("shell");

    /* sets program paging to previous process */
    program_paging(((cur_pcb_ptr[process_terminal]->process_id) * _4MB_PAGE) + _8_MB);
    shell_flag[process_terminal] = 1;
    /* if we typed during the execution of a program, add a newline and turn the flag off*/
    if (typing_during_prog_flag[process_terminal] == 1){
        putc('\n');
        typing_during_prog_flag[process_terminal] = 0;
    }

    /* returns to previous program's execution */
    /* switch stack context */
    tss.ss0 = KERNEL_DS;  
    tss.esp0 = (uint32_t)(_8_MB - ((cur_pcb_ptr[process_terminal]->process_id) * _8_KB) - _4_BYTES);

    /* jumps into end of execute */
    asm volatile(
        "mov %0, %%esp;" /* store esp*/
        "mov %1, %%ebp;" /* store ebp */
        "mov %2, %%eax;" /* store return value */
        "jmp return_from_prog;"
        :
        : "r"((cur_pcb_ptr[process_terminal]->esp)), "r"((cur_pcb_ptr[process_terminal]->ebp)), "r"((uint32_t)status));

    return 0;
}

/** sys_execute
 * Execute System Call 
 * 
 * Inputs: int8_t command
 * Outputs: int32_t --> 0 on success, failure if otherwise
 * Side Effects: executes a program by: parsing arguments, checking executable, setting up paging for the program and loading it into memory, switching contexts to user program
 */
int32_t sys_execute(const int8_t *command) {
    int8_t tempret = 0, prog_name[FILENAME_LEN], arg[KEYBOARD_BUFFER_SIZE];
    uint32_t return_value = 0, term = visible_terminal, i;
    process_num = 0;

    /* parses arguments passed into command, stores them in prog_name and arg */
    memset(prog_name, '\0', FILENAME_LEN);
    memset(arg, '\0', KEYBOARD_BUFFER_SIZE);
    tempret = _execute_parse_args(command, prog_name, arg);
    if (tempret == -1) return -1;

    /* checks if we are executing from scheduling vs. from command line */
    if (cur_pcb_ptr[process_terminal] == NULL) term = process_terminal;

    /* set flags based on argument and error condition */
    error_flag[term] = 0;
    arg_flag[term] = 1;
    if (tempret == -2) arg_flag[term] = 0;


    /* saves the old base and stack pointer prior to execution to return to later*/
    if (cur_pcb_ptr[term] != NULL)
    {
        asm volatile(
            "mov %%esp, %0;" /* save esp */
            "mov %%ebp, %1;" /* save ebp */
            : "=r"((cur_pcb_ptr[term]->esp)), "=r"((cur_pcb_ptr[term]->ebp)));
    }

    /* checks that the file is an executable*/
    tempret = _execute_executable_check(prog_name, buf_executable_header[term]);
    if (tempret == -1) return -1;

    /* computes the process ID and sets process to active */
    for (i = 0; i < MAX_NUM_PROCESSES; i++)
    {
        if (active_processes[i] == 1) continue;
        else
        {
            process_num = i;
            break;
        }
    }
    if (i >= MAX_NUM_PROCESSES) {
        too_many_shells_flag = 1;
        return -1;
    }
    active_processes[process_num] = 1;

    /* sets up paging scheme for current program */
    tempret = _execute_setup_program_paging();
    if (tempret == -1) return -1;
    /* loads the user program */
    tempret = _execute_user_program_loader(prog_name);
    if (tempret == -1) return -1;
    /* checks if we are executing shell */
    if (strncmp(prog_name, "shell", 5) == 0) shell_flag[term] = 1;
    else shell_flag[term] = 0;

    /* creates PCB for process */
    cur_pcb_ptr[term] = _execute_create_PCB(arg, term);

    /* switches context to user program */
    _execute_context_switch(term);

    /* place for a program to return to while being halted */
    asm volatile(
        "return_from_prog:;"
        "leave;"
        "ret;"
        : "=a"(return_value));

    return 0;
}

/** _execute_parse_args
 * Execute System Call Helper 
 * 
 * Inputs: command, name of program, arguments 
 * Outputs: 0 on success, -1 on failure
 * Side Effects: parses the command line arguments for the program
 */
int32_t _execute_parse_args(const int8_t *command, int8_t *prog_name, int8_t *arg) {

    int32_t i = 0, j, k = 0;
    /* error checking */
    if (command == NULL || prog_name == NULL || arg == NULL) return -1;

    /* moves past NULL characters */
    while (command[i] == '\0') if (++i == KEYBOARD_BUFFER_SIZE) return -1;
    /* skips spaces in front of program name */
    while (command[i] == ' ' && command[i] != '\0' && command[i] != '\n') i++;
    /* -1 implies bad command name */
    if (command[i] == '\0' || command[i] == '\n') return -1;

    /* keeps track of where command starts */
    j = i;
    /* copies name of program into prog_name buffer -- only 32 chars */
    while (command[i] != ' ' && command[i] != '\0' && command[i] != '\n' && (i - j) < FILENAME_LEN)
        memcpy(prog_name++, command + (i++), 1);
    /* if there are still chars left in the program name, skip over them */
    while (command[i] != ' ' && command[i] != '\0' && command[i] != '\n') i++;
    /* skips spaces in front of argument name */
    while (command[i] == ' ' && command[i] != '\0') i++;

    /* if there is no arg, return -2 */
    if (command[i] == '\0') return -2;
    /* copies arguments into arg buffer */
    while (command[i] != '\0') memcpy(&(arg[k++]), command + (i++), 1);

    return 0;
}

/** _execute_executable_check
 * Execute System Call Helper 
 * 
 * Inputs: program mane and a buffer to fill with the header information (for later use) 
 * Outputs: 0 on success, -1 on failure
 * Side Effects: checks that the file is an executable via ELF constant
 * */
int32_t _execute_executable_check(int8_t *prog_name, int8_t *buf) {

    /* 4 values @ start signifying executable */
    int8_t elf[EXECUTABLE_CHECK] = {ELF_ONE, ELF_TWO, ELF_THREE, ELF_FOUR}; int32_t ret; dentry_t prog_dentry;

    /* buffer used for checking */
    memset((void *)buf, '\0', EXECUTABLE_CHECK);

    /* fills buf with 40 bytes of program info, we use read_dentry/data b/c we don't want to move our pointer forward */
    ret = read_dentry_by_name((uint8_t *)prog_name, &prog_dentry);
    if (prog_dentry.file_type != FILE || ret == -1) return -1;
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
int32_t _execute_setup_program_paging() {
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
int32_t _execute_user_program_loader(int8_t *prog_name) {
    /* loads contents of file into program image in virtual memory */
    int32_t inode = _sys_open_file((uint8_t *)prog_name), out, program_size;
    /* gets size of program */
    dentry_t this_dentry;

    read_dentry_by_name((const uint8_t *)prog_name, &this_dentry);
    program_size = (int32_t)inode_head[this_dentry.inode].length;

    /* dynamically write to program image based on size of program */
    memset((void *)PROGRAM_IMAGE, 0, program_size);

    out = read_data(inode, 0, (void *)PROGRAM_IMAGE, program_size);

    return (out == -1) ? -1 : 0;
}

/** _execute_create_PCB
 * Execute System Call Helper 
 * 
 * Inputs: none  
 * Outputs: new PCB for the program
 * Side Effects: creates the PCB for this program and stores appropriate information
 */
pcb_t *_execute_create_PCB(char *argument, uint32_t term) {
    /* creates the PCB for this at the right location */
    pcb_t *new_pcb = (pcb_t *)((int)_8_MB - ((int)_8_KB * (process_num + 1)) - _4_BYTES);

    /* assigns parent PCB and PID */
    new_pcb->parent_pcb = cur_pcb_ptr[term];
    new_pcb->process_id = process_num;

    /* first 2 file descriptors in PCB are stdin/stdout */
    new_pcb->file_desc_array[0] = (file_desc_t){&std_in_fops, 0, 0, 1};  /* SET EQUAL TO STDIN */
    new_pcb->file_desc_array[1] = (file_desc_t){&std_out_fops, 0, 0, 1}; /* SET EQUAL TO STDOUT */

    /* initializes other PCB members */
    new_pcb->vidmap_terminal = 0;
    new_pcb->rtc_counter = 0;
    new_pcb->rtc_interrupt_divider = 0;

    /* sets the argument of the current process*/
    memset(new_pcb->argument_array, '\0', KEYBOARD_BUFFER_SIZE);
    memcpy(new_pcb->argument_array, argument, strlen(argument));

    return new_pcb;
}

/** _execute_context_switch
 * Execute System Call Helper 
 * 
 * Inputs: none 
 * Outputs: none
 * Side Effects: switches context to the user program
 */
void _execute_context_switch(uint32_t term) {
    /* switch TSS context (stack segment and esp) */
    tss.ss0 = KERNEL_DS;                                                               /* switch stack context */
    tss.esp0 = (uint32_t)(_8_MB - (_8_KB * cur_pcb_ptr[term]->process_id) - _4_BYTES); /* pointer to the top of stack/pcb */
    /* maps the esp of user space */
    uint32_t user_esp = _132_MB - _4_BYTES; /* maps to the esp of user space */
    /* gets the eip from the executable header */
    uint32_t eip = 0;
    /* buf_executable_header from 27 to 24 used as specified in the documentation. 
    the 24, 16, and 8 are used to shift the bytes to the correct spot in the eip int */
    eip += (((uint8_t)buf_executable_header[term][ELF_BYTE_ONE]) << 24) 
         | (((uint8_t)buf_executable_header[term][ELF_BYTE_TWO]) << 16) 
         | ((uint8_t)(buf_executable_header[term][ELF_BYTE_THREE]) << 8) 
         | ((uint8_t)buf_executable_header[term][ELF_BYTE_FOUR]);
    cur_pcb_ptr[term]->eip = eip;
    /* performs context stack in assembly */
    asm volatile(
        "push %0;" /* push user_ds */
        "push %1;" /* push user_esp */
        "pushfl;"  /* push EFLAGS */
        "popl %%eax;"
        "orl  $0x0200, %%eax;" /* enable interrupts */
        "pushl %%eax;"
        "push %2;"
        "push %3;" /* push eip value stored in esp */
        "iret;"
        :
        : "r"(USER_DS), "r"(user_esp), "r"(USER_CS), "r"(eip)
        : "eax");
}

/** sys_read
 * Read System Call 
 * 
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: number of bytes readud 
 * Side Effects: fills buffer with contents reaud
 */
int32_t sys_read(int32_t fd, void *buf, int32_t nbytes) {
    /* checking bounds of the arguments */
    if (nbytes <= 0 || buf == NULL || fd < 0 || fd > MAX_FD_IDX) return -1;
    /* checks that the file is active */
    if (cur_pcb_ptr[process_terminal]->file_desc_array[fd].flags == 0) return -1;

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
int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes) {
    /* checking bounds of the arguments */
    if (nbytes <= 0 || buf == NULL || fd < 0 || fd > MAX_FD_IDX) return -1;
    /* checks that the file is active */
    if (cur_pcb_ptr[process_terminal]->file_desc_array[fd].flags == 0) return -1;

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
int32_t sys_open(const uint8_t *filename) {
    int32_t i, ret_val, cur_pcb_idx = 0;
    /* assign the appropriate file descriptor to the PCB */
    for (i = 2; i <= MAX_FD_IDX; i++)
    {
        if (cur_pcb_ptr[process_terminal]->file_desc_array[i].flags == 1) continue;
        else
        {
            cur_pcb_idx = i;
            break;
        }
    }
    /* if the PCB is full, we fail this */
    if (i > MAX_FD_IDX) return -1;

    /* find the dentry for the file */
    dentry_t this_dentry;
    this_dentry.inode = 0;
    this_dentry.file_type = 0;
    ret_val = read_dentry_by_name(filename, &this_dentry);
    if (ret_val == -1) return -1;

    /* figures out which type of file it is */
    file_ops_t *this_fops;
    uint32_t this_inode;
    /* switches based on type of file */
    switch (this_dentry.file_type)
    {
        case RTC:
            this_fops = &rtc_fops;
            this_inode = _sys_open_rtc(filename); /* returns 0 */
            break;
        case DIRECTORY:
            this_fops = &dir_fops;
            this_inode = _sys_open_directory(filename); /* returns 0 */
            break;
        case FILE:
            this_fops = &file_fops;
            this_inode = _sys_open_file(filename); /* returns inode */
            break;
        default:
            return -1;
    }
    /* assigns appropriate file descriptor */
    cur_pcb_ptr[process_terminal]->file_desc_array[cur_pcb_idx] = (file_desc_t){this_fops, this_inode, 0, 1};

    return cur_pcb_idx; /* index in the file descriptor array */
}

/** sys_close
 * Close System Call 
 * 
 * Inputs: int32_t file descriptor of an open file
 * Outputs: return value of close function for the given file
 * Side Effects: PCB flags cleared
 */
int32_t sys_close(int32_t fd) {
    /* checks bounds and if the file is active */
    if (fd < MIN_FD_IDX || fd > MAX_FD_IDX) return -1;
    if (cur_pcb_ptr[process_terminal]->file_desc_array[fd].flags == 0) return -1;

    /* marks file as closed in PCB */
    cur_pcb_ptr[process_terminal]->file_desc_array[fd].flags = 0;
    /* calls the appropriate system close function */
    return cur_pcb_ptr[process_terminal]->file_desc_array[fd].file_ops_table->close(fd);
}

/** sys_getargs
 *  
 * Takes care of arguments in shell
 * Inputs: int8_t buf (buffer to be populated), int32_t nbytes (numbr of bytes to be read)
 * Outputs: 0 for success, -1 for failure
 * Side Effects: populate buf with the properly parsed argument (taken care of in execute)
 */
int32_t sys_getargs(int8_t *buf, int32_t nbytes) {
    /* if there is no arg, return -1 */
    if (arg_flag[process_terminal] == 0) return -1;
    /* clear buf and store arg into buf */
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
int32_t sys_vidmap(uint8_t **screen_start) {
    /* makes sure that screen_start pointer is within the program loading block between 128 MB and 132 MB */
    if ((uint32_t)screen_start < USER_START || (uint32_t)screen_start >= USER_END || (uint32_t)screen_start == NULL) return -1;

    /* maps video memory to appropriate terminal */
    if (visible_terminal == process_terminal) vidmap_paging();

    else
    {
        uint32_t terminal_address = (process_terminal == 0) ? TERMINAL_ONE_BUFFER : (process_terminal == 1) ? TERMINAL_TWO_BUFFER : TERMINAL_THREE_BUFFER;
        vidmap_paging_modify(terminal_address);
    }

    /* video memory will be at 132MB */
    (*screen_start) = (uint8_t *)_132_MB;
    cur_pcb_ptr[process_terminal]->vidmap_terminal = 1;

    return 0;
}

/** sys_set_handler
 *  
 * Halt system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_set_handler(int32_t signum, void *handler_address) {
    printf("set handler called\n");
    return -1;
}

/** sys_sigreturn
 *  
 * Halt system call
 * Inputs: int8_t status
 * Outputs: int32_t
 * Side Effects: None
 * NOT YET IMPLEMENTED
 */
int32_t sys_sigreturn(void) {
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
int32_t _sys_open_terminal(const uint8_t *filename) {
    return 0;
}
/** _sys_close_terminal
 *  
 * Terminal helper function for system close
 * Inputs: same as sys_close
 * Outputs: 0 
 * Side Effects: 
 */
int32_t _sys_close_terminal(int32_t fd) {
    return 0;
}
/** _sys_read_terminal
 *  
 * Terminal helper function for system read
 * Inputs: same as sys_read
 * Outputs: number of bytes read
 * Side Effects: none
 */
int32_t _sys_read_terminal(int32_t fd, void *buf, int32_t nbytes) {
    uint32_t retval = 0, i = 0, j;
    if (NULL == buf || nbytes < 0) return -1;
    if (nbytes == 0) return 0;

    memset(keyboard_buffer[process_terminal], '\0', KEYBOARD_BUFFER_SIZE);

    /* for ctrl L and for typing during execution of a program (saving the chars typed) */
    if (re_echo_flag[process_terminal] == 1 || (shell_flag[process_terminal] == 1 && temp_kbd_idx[process_terminal] != 0))
    {
        keyboard_cursor_idx[process_terminal] = temp_kbd_idx[process_terminal];
        keyboard_buffer_end_idx[process_terminal] = keyboard_cursor_idx[process_terminal];
        memcpy(keyboard_buffer[process_terminal], temp_kbd_buf[process_terminal], keyboard_cursor_idx[process_terminal]);
        memcpy(current_buf[process_terminal], temp_kbd_buf[process_terminal], keyboard_cursor_idx[process_terminal]);
        
        while (i < keyboard_cursor_idx[process_terminal] && keyboard_buffer[process_terminal][i] != '\n')
        {
            putc(keyboard_buffer[process_terminal][i]);
            i++;
        }

        if (keyboard_buffer[process_terminal][i] == '\n') keyboard_cursor_idx[process_terminal]--;
        keyboard_buffer_end_idx[process_terminal] = keyboard_cursor_idx[process_terminal];
        current_buf_index[process_terminal] = keyboard_buffer_end_idx[process_terminal];

        re_echo_flag[process_terminal] = 0;
        memset(temp_kbd_buf[process_terminal], '\0', KEYBOARD_BUFFER_SIZE);
        temp_kbd_idx[process_terminal] = 0;
    }

    /* adjusts nbytes if overflow */
    j = 0;
    nbytes = (nbytes > KEYBOARD_BUFFER_SIZE) ? KEYBOARD_BUFFER_SIZE - 1 : nbytes;

    /* reads data/fills buffer from keyboard */
    sys_read_flag[process_terminal] = 1;
    sti();
    while (sys_read_flag[process_terminal]);
    cli();

    /* copies memory from keyboard input to buffer */
    while (keyboard_buffer[process_terminal][j] != '\0' && keyboard_buffer[process_terminal][j] != '\n' && j < nbytes)
    {
        memcpy(&(((char *)buf)[j]), &(keyboard_buffer[process_terminal][j]), 1);
        retval++;
        j++;
    }
    ((char *)buf)[j] = '\n';
    /* the +1 accounts for the '\n' */
    return retval + 1;
}
/** _sys_write_terminal
 *  
 * Terminal helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_write_terminal(int32_t fd, const void *buf, int32_t nbytes) {
    int32_t i, bytes_written;
    // char write_string[nbytes];
    /* check edge cases */
    if (NULL == buf || nbytes < 0) return -1;
    if (nbytes == 0) return 0;
    /* custom error message for too many processes */
    if (too_many_shells_flag == 1 && strncmp("no such command\n", buf, NO_SUCH_CMD_LEN) == 0) {
        printf("Too many processes are currently active.\nPlease terminate one with the 'exit' command to start more.\n");
        return NO_SUCH_CMD_LEN;
    }

    /* put passed in buffer into an appropriately sized buffer */
    // memset(write_string, '\0', nbytes);
    // memcpy(write_string, buf, nbytes);

    /* prints all non-null characters */
    bytes_written = 0;
    for (i = 0; i < nbytes; i++)
    {
        /* writes non null characters */
        if (((char *)buf)[i] != '\0')
        {
            putc(((char*)buf)[i]);
            bytes_written++;
        }
    }
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
int32_t _sys_open_rtc(const uint8_t *filename) {
    /* puts the number 2 into the buffer to set the frequency to 2Hz */
    int32_t freq[1] = {2};
    _sys_write_rtc(NULL, (void *)freq, 4); /* sets the RTC frequency to 2Hz */
    return 0;
}

/** _sys_close_RTC
 *  
 * RTC helper function for system close
 * Inputs: same as sys_close
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_close_rtc(int32_t fd) {
    return 0;
}

/** sys_write_rtc
 *  
 * RTC helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: sets RTC rate
 */
int32_t _sys_write_rtc(int32_t fd, const void *buf, int32_t nbytes) {
    /* sets the RTC rate to 2Hz */
    int frequency, count;
    if (nbytes != _4_BYTES || buf == NULL) return -1;
    frequency = *((int32_t *)buf);

    /* param check */
    if (power_of_two(frequency) || frequency < 0) return -1;
    /* gets frequency from buffer */
    if (frequency > MAX_INTERRUPT_FREQUENCY) frequency = MAX_INTERRUPT_FREQUENCY;
    /* figure out what multiple of the max frequency the program wants */
    count = MAX_INTERRUPT_FREQUENCY / frequency;
    cur_pcb_ptr[process_terminal]->rtc_interrupt_divider = count;
    cur_pcb_ptr[process_terminal]->rtc_counter = count;
    return 0;
}

/** _sys_read_rtc
 *  
 * RTC helper function for system read
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: 0 when an RTC interrupt occurs
 * Side Effects: none
 */
int32_t _sys_read_rtc(int32_t fd, void *buf, int32_t nbytes) {
    /* reset the counter of the process we are reading for so that we get the full duration of the rtc interrupr */
    cur_pcb_ptr[process_terminal]->rtc_counter = cur_pcb_ptr[process_terminal]->rtc_interrupt_divider;

    /* with virtualized RTC, keep reading until enough interrupts have occurred for desired frequency */
    sti();
    while (cur_pcb_ptr[process_terminal]->rtc_counter > 0);
    cli();

    /* reset the counter for that particular process back to its max */
    cur_pcb_ptr[process_terminal]->rtc_counter = cur_pcb_ptr[process_terminal]->rtc_interrupt_divider;
    return 0;
}

/**
 *  FILE HELPERS 
 **/
/** _sys_open_file
 *  
 * File helper function for system open
 * Inputs: same as sys_open
 * Outputs: 0 @ directory end
 * Side Effects: 
 */
int32_t _sys_open_file(const uint8_t *filename) {
    uint8_t file[FILENAME_LEN];
    int32_t i, ret_val;
    dentry_t this_file;

    for (i = 0; i < FILENAME_LEN; i++)
    {
        if (filename[i] == '\0') break;
        file[i] = filename[i];
    }
    file[i] = '\0';
    /* sets up a blank dentry to be filled by read */
    this_file.inode = 0;
    this_file.file_type = 0;

    /* populates dentry with read call */
    ret_val = read_dentry_by_name(file, &this_file);
    if (ret_val == -1) return -1;
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
int32_t _sys_close_file(int32_t fd) {
    return 0;
}

/** _sys_read_file
 *  
 * Terminal helper function for system read
 * Inputs: file descriptor, buffer (to fill), number of bytes to read
 * Outputs: 0 if success (read nbytes), -1 if fail (read < nbytes)
 * Side Effects: moves forward in file
 */
int32_t _sys_read_file(int32_t fd, void *buf, int32_t nbytes) {
    int32_t data_read, offset = cur_pcb_ptr[process_terminal]->file_desc_array[fd].file_position;
    /* condition checks */
    if (nbytes <= 0 || buf == NULL || fd < 0) return -1;
    /* does the data read */
    data_read = read_data(cur_pcb_ptr[process_terminal]->file_desc_array[fd].inode, offset, buf, nbytes);
    /* moves forward in file corresponding to how many bytes are read */
    if (data_read > 0) cur_pcb_ptr[process_terminal]->file_desc_array[fd].file_position += data_read;
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

int32_t _sys_write_file(int32_t fd, const void *buf, int32_t nbytes) {
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
int32_t _sys_open_directory(const uint8_t *filename) {
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
int32_t _sys_read_directory(int32_t fd, void *buf, int32_t nbytes) {
    dentry_t curr_dentry;
    /* if we read all the files, reset flag and return 0 */
    if (read_dir_flag[process_terminal] >= boot_block->entries)
    {
        read_dir_flag[process_terminal] = 0;
        return 0;
    }
    /* read file into a dentry given an inode */
    read_dentry_by_index(read_dir_flag[process_terminal]++, &curr_dentry);
    /* copy file name into buf */
    strncpy((int8_t *)(buf), (int8_t *)curr_dentry.file_name, FILENAME_LEN);
    return FILENAME_LEN;
}
/** sys_write_directory
 *  
 * RTC helper function for system write
 * Inputs: same as sys_write
 * Outputs: 0 @ directory end
 * Side Effects: sets RTC rate
 */

int32_t _sys_write_directory(int32_t fd, const void *buf, int32_t nbytes) {
    return -1;
}

/** _sys_dummy_read(int32_t fd, void* buf, int32_t nbytes)
 *  
 * Dummy function for stdout
 * Inputs: same as sys_read
 * Outputs: -1 
 * Side Effects: none
 */
int32_t _sys_dummy_read(int32_t fd, void *buf, int32_t nbytes) {
    return -1;
}

/** _sys_dummy_write(int32_t fd, void* buf, int32_t nbytes)
 *  
 * Dummy function for stdin
 * Inputs: same as sys_write
 * Outputs: -1 
 * Side Effects: none
 */
int32_t _sys_dummy_write(int32_t fd, const void *buf, int32_t nbytes) {
    return -1;
}

/** _sys_dummy_open(const int8_t* filename)
 *  
 * Dummy function for stdin/stdout
 * Inputs: same as sys_open
 * Outputs: -1 
 * Side Effects: none
 */
int32_t _sys_dummy_open(const uint8_t *filename) {
    return -1;
}

/** _sys_dummy_close(int32_t fd)
 *  
 * Dummy function for stdin/stdout
 * Inputs: same as sys_close
 * Outputs: -1 
 * Side Effects: none
 */
int32_t _sys_dummy_close(int32_t fd) {
    return -1;
}
