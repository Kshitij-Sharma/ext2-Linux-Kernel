#ifndef _PCB
#define _PCB
#include "lib.h"

#define FILE_DESC_ARR_SIZE      8
#define MAX_FD_IDX              7
#define MIN_FD_IDX              2
#define KEYBOARD_BUFFER_SIZE    128
#define MAX_NUM_PROCESSES       6

/* variables to keep track of processes*/
extern uint32_t active_processes[MAX_NUM_PROCESSES];
int process_num;

/* struct containing function pointers for sys call abstraciton */
typedef struct file_ops_struct{
   int32_t (*read)  (int32_t fd, void* buf, int32_t nbytes);
   int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
   int32_t (*open)  (const uint8_t* filename);
   int32_t (*close) (int32_t fd);
} file_ops_t;

/* file descriptor */
typedef struct file_descriptor_struct{
  file_ops_t * file_ops_table;  /* table of operations associated with a given file type */
  uint32_t inode;               /* identifier for the process */
  uint32_t file_position;       /* location in program */
  uint32_t flags;               /* set to 1 if file is active*/

} file_desc_t;

/* process control block */
typedef struct pcb_t{
  file_desc_t file_desc_array[FILE_DESC_ARR_SIZE];      /* array of file descriptors for a given process*/
  int8_t argument_array[KEYBOARD_BUFFER_SIZE];          /* array containing arguments passed to program, comes from parseargs */
  struct pcb_t* parent_pcb;                             /* parent pcb of this process, set to NULL for base shell */
  uint32_t process_id;                                  /* id of the current process */
  uint32_t ebp;                                         /* saved ebp value of current process */
  uint32_t esp;                                         /* saved esp value of current process */
  uint32_t eip;                                         /* saved eip value of current process */
  int32_t vidmap_terminal;                              /* variable that is set if the process uses vidmap, this is checked in scheduling when switching processes */
  uint32_t rtc_interrupt_divider;                       /* for rtc virtualization, number of 512Hz interrupts required to produce the program's desired interrupt frequency */
  uint32_t rtc_counter;                                 /* for rtc virtualization, number of 512Hz interrupts that have occured; counts down from rtc_interrupt_divider to 0 */
} pcb_t;

#endif
