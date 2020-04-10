#include "lib.h"

#define FILE_DESC_ARR_SIZE  8
#define MAX_FD_IDX          7
#define MIN_FD_IDX          2

typedef struct file_ops_struct{
   int32_t (*read)  (int32_t fd, void* buf, int32_t nbytes);
   int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
   int32_t (*open)  (const int8_t* filename);
   int32_t (*close) (int32_t fd);
} file_ops_t;
// typedef int32_t (*file_ops_t)(); 

typedef struct file_descriptor_struct{
  file_ops_t * file_ops_table;
  uint32_t inode;               // identifier for the process
  uint32_t file_position;       // location in program
  uint32_t flags; 

} file_desc_t;

typedef struct pcb_t{
  file_desc_t file_desc_array[FILE_DESC_ARR_SIZE];    // files opened (children)
  struct pcb_t* parent_pcb;
  uint32_t process_id; // which number process is it
  uint32_t next_open_index;
  uint32_t ebp;
  uint32_t esp;
  uint32_t eip;
  // current PCB
  // something for registers (esp ebp eip) -- just need one of these
  // PID: index of child process in parent process FD array
  // parent process (one pointer to an inode)
  // start and end of program (?)
} pcb_t;

