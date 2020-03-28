/* filesys.h - Defines a set of files needed for the file system 
* 1. Contains a struct for the boot block
* 2. Contains an index node for a proper file
*/

#include "types.h"
#include "lib.h"
#include "multiboot.h"

// for boot block
#define BOOT_RESERVED    52
#define DIR_ENTRIES      63
#define FILENAME_LEN     32
#define DENTRY_RESERVED  24

// for index nodes
#define INODE_SIZE       1023 //[(4KB - 4B)/4B)] because first 4B holds length
#define _4KB_            4096

// for data blocks
// #define DATA_BLOCK_HEAD  (_4KB_ + (DIR_ENTRIES * _4KB_))

// file type macros
#define RTC_FILE        0
#define DIRECTORY       1
#define REGULAR_FILE    2


typedef int32_t bytes_offset;       
bytes_offset data_bytes_read;                                                                                                                                                                             ;
/* This structure is used to hold file attributes such as 
 * file name, size, index node value, and reserved data */
typedef struct dentry_struct{
  uint8_t file_name[FILENAME_LEN];
  uint32_t file_type;
  uint32_t inode;
  uint8_t reserved[DENTRY_RESERVED];
} dentry_t;

/* This structure is used to hold file system statistics
 * and directory entries */
typedef struct boot_block_struct{
  uint32_t entries;
  uint32_t inodes;
  uint32_t datablocks;
  uint32_t reserved[13];
  // char reserved[BOOT_RESERVED];
  dentry_t dir_entries[DIR_ENTRIES];
} boot_block_t;
/* This structure is used to hold file length
 * and file data block numbers */
typedef struct inode_struct{
  uint32_t length;
  uint32_t datablock_nums[INODE_SIZE];
}inode_t;

boot_block_t* boot_block;
inode_t* inode_head;
uint8_t* data_blocks;

//
//  File system routines
//

/* Initialize the start of the file system */
int32_t filesys_start(module_t *offset);

/* Read the directory entry by given name */
int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry);

/* Read the directory enty by given index */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

/* Read data from the files */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
