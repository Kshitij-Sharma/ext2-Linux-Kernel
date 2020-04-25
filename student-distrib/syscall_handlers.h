#ifndef _SYSCALL_HANDLERS_H
#define _SYSCALL_HANDLERS_H

#include "idt_handlers.h"
#include "lib.h"
#include "rtc.h"
#include "filesys.h"
#include "paging.h"
#include "PCB.h"
#include "multiprocessing.h"
#include "x86_desc.h"

#define MAX_INTERRUPT_FREQUENCY     512 
#define ERROR_VAL                   256
#define MIN_RATE                    3


extern int32_t sys_halt(int8_t status); // uint
int32_t sys_execute(const int8_t* command); // uint
int32_t _execute_parse_args(const int8_t* command, char * program_name, char * argument);
int32_t _execute_executable_check(int8_t * prog_name, int8_t * buf);
int32_t _execute_setup_program_paging();
int32_t _execute_user_program_loader();
pcb_t * _execute_create_PCB(char* argument, uint32_t term);
void _execute_context_switch(uint32_t term);


int32_t sys_read (int32_t fd, void* buf, int32_t nbytes);
int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t sys_open (const uint8_t* filename); // uint
int32_t sys_close (int32_t fd);
int32_t sys_getargs(int8_t* buf, int32_t nbytes); // uint
int32_t sys_vidmap (uint8_t** screen_start);
int32_t sys_set_handler (int32_t signum, void* handler_address);
int32_t sys_sigreturn (void);

/* helper functions for sys read */
int32_t _sys_read_terminal (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_read_file (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_read_rtc (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_read_directory (int32_t fd, void* buf, int32_t nbytes);

/* helper functions for sys read */
extern int32_t _sys_write_terminal (int32_t fd, const void* buf, int32_t nbytes);
int32_t _sys_write_file (int32_t fd, const void* buf, int32_t nbytes);
int32_t _sys_write_rtc (int32_t fd, const void* buf, int32_t nbytes);
int32_t _sys_write_directory (int32_t fd, const void* buf, int32_t nbytes);

int32_t _sys_open_terminal (const uint8_t* filename);// uint
int32_t _sys_open_file (const uint8_t* filename); // uint
int32_t _sys_open_rtc (const uint8_t* filename);// uint
int32_t _sys_open_directory (const uint8_t* filename); // uint

int32_t _sys_close_terminal(int32_t fd);
int32_t _sys_close_file(int32_t fd);
int32_t _sys_close_rtc(int32_t fd);
int32_t _sys_close_directory(int32_t fd);

int32_t _sys_dummy_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t _sys_dummy_read(int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_dummy_open(const uint8_t* filename);
int32_t _sys_dummy_close(int32_t fd);

#endif
