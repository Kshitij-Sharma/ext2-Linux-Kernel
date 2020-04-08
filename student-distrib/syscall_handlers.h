#include "lib.h"
#include "idt_handlers.h"
#include "rtc.h"
#include "filesys.h"
#include "paging.h"
#include "PCB.h"
#include "x86_desc.h"

#define MAX_INTERRUPT_FREQUENCY     1024   
#define MAX_NAME_LENGTH             32
#define ELF_ONE                     0x7F
#define ELF_TWO                     0x45
#define ELF_THREE                   0x4C
#define ELF_FOUR                    0x46
int32_t sys_halt(int8_t status);




int32_t sys_execute(const int8_t* command);
int32_t _execute_parse_args(const int8_t* command, char * program_name, char * argument);
int32_t _execute_executable_check(int8_t * prog_name, int8_t * buf);
int32_t _execute_setup_program_paging();
int32_t _execute_user_program_loader();
pcb_t * _execute_create_PCB();
void _execute_context_switch();


int32_t sys_read (int32_t fd, void* buf, int32_t nbytes);
int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t sys_open (const int8_t* filename);
int32_t sys_close (int32_t fd);
int32_t sys_getargs(int8_t* buf, int32_t nbytes);
int32_t sys_vidmap (int8_t** screen_start);
int32_t sys_set_handler (int32_t signum, void* handler_address);
int32_t sys_sigreturn (void);

/* helper functions for sys read */
int32_t _sys_read_terminal (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_read_file (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_read_rtc (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_read_directory (int32_t fd, void* buf, int32_t nbytes);

/* helper functions for sys read */
int32_t _sys_write_terminal (int32_t fd, const void* buf, int32_t nbytes);
int32_t _sys_write_file (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_write_rtc (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_write_directory (int32_t fd, void* buf, int32_t nbytes);

int32_t _sys_open_terminal (const int8_t* filename);
int32_t _sys_open_file (const int8_t* filename);
int32_t _sys_open_rtc (const int8_t* filename);
int32_t _sys_open_directory (const int8_t* filename);

int32_t _sys_close_terminal(int32_t fd);
int32_t _sys_close_file(int32_t fd);
int32_t _sys_close_rtc(int32_t fd);
int32_t _sys_close_directory(int32_t fd);



int32_t _sys_dummy_read_write(int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_dummy_open(const int8_t* filename);
int32_t _sys_dummy_close(int32_t fd);
