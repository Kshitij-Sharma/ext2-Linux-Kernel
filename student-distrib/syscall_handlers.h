// #include "lib.h"
#include "idt_handlers.h"
#include "rtc.h"

#define MAX_INTERRUPT_FREQUENCY     1024   
int32_t sys_halt(int8_t status);
int32_t sys_execute(const int8_t* command);
// int32_t sys_read (int32_t fd, void* buf, int32_t nbytes);
// int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t sys_open (const int8_t* filename);
int32_t sys_close (int32_t fd);
int32_t sys_getargs(int8_t* buf, int32_t nbytes);
int32_t sys_vidmap (int8_t** screen_start);
int32_t sys_set_handler (int32_t signum, void* handler_address);
int32_t sys_sigreturn (void);

/* flag variable specifying what type of read we want to do:
    't' for terminal
    'r' for rtc
    'f' for filesystem  */
// extern char sys_read_type = NULL;
/* helper functions for sys read */
int32_t _sys_read_terminal (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_read_filesystem (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_read_rtc (int32_t fd, void* buf, int32_t nbytes);

/* flag variable specifying what type of write we want to do:
    't' for terminal
    'r' for rtc
    'f' for filesystem  */
// extern char sys_write_type = NULL;
/* helper functions for sys read */
int32_t _sys_write_terminal (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_write_filesystem (int32_t fd, void* buf, int32_t nbytes);
int32_t _sys_write_rtc (int32_t fd, void* buf, int32_t nbytes);
