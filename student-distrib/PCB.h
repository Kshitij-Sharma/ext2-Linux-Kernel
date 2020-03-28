// #include "lib.h"
// #include "syscall_handlers.h"

// #define FILE_DESC_ARR_SIZE 8
// #define TABLE_SIZE

// // typedef int32_t (*file_ops_t)(); 

// typedef struct file_descriptor_struct{
//   file_ops_t* file_ops_table;
//   uint32_t inode;
//   uint32_t file_position;
//   uint32_t flags;

// } file_desc_t;

// typedef struct pcb_struct{
//   file_desc_t pcb[FILE_DESC_ARR_SIZE];
// } pcb_t;

// typedef struct file_ops_struct {
//    int32_t (*read) (int32_t fd, void* buf, int32_t nbytes);
//    int32_t (*write) (int32_t fd, void* buf, int32_t nbytes);
//    int32_t (*open) (const int8_t* filename);
//    int32_t (*close) (int32_t fd);
// }file_ops_t;
