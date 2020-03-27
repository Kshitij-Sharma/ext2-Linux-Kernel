/* filesys.c - supports operations on the file system image, 
* including opening and reading from files, opening and reading the directory,
* and copying program images into contiguous physical memory
*/

#include "filesys.h"

/*
    filesys_start()
    Description: Initializes the filsystem at the proper starting address
    Inputs: offset - starting address of the file system
    Outputs: None
    Return Value: 0 if success
*/
int32_t filesys_start(uint32_t offset){
  boot_block = (boot_block_t*) offset;
  inode_head = (inode_t *)(offset + _4KB_);
  data_blocks = (uint8_t *)(offset + DATA_BLOCK_HEAD);
  return 0;
}
/*
    read_dentry_by_name()
    Description: Using directory entry name, this function copies the file information
    to a dentry pointer
    Inputs: fname - string of the filename
            dentry - holds file 
    Outputs: None
    Return Value: 
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
  // checks for invalid parameters as well as an empty string for the name
  if((dentry == NULL) || (fname == NULL) || (fname[0] == (uint8_t) '\0'))
    return -1;

  // loops through all 63 files and checks for the proper name match
  uint32_t i;

  for(i = 0; i < DIR_ENTRIES; i++)
  {
    //using strcmp to check equality between the file names
    if(!strncmp((int8_t*) fname, (int8_t*) (boot_block -> dir_entries)[i].file_name, FILENAME_LEN))
      return read_dentry_by_index(i, dentry);
  }
  return -1;
}

/*
*   read_dentry_by_index()
*   Description: Using an index, this function copies the file information
*   to a dentry pointer
*   Return Value
*   Inputs: index - index value of file
*           dentry - holds file 
*   Outputs: None
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
  if(!(index < DIR_ENTRIES) || (dentry == NULL))
    return -1;

  dentry_t *temp = &((boot_block -> dir_entries)[index]);

  //populate dentry
  (dentry -> file_type) = (temp -> file_type);
  (dentry -> inode) = (temp -> inode);
  //copy the contents of filename
  uint32_t i;
  for(i = 0; i < FILENAME_LEN; i++)
  {
    (dentry -> file_name)[i] = (temp -> file_name)[i];
  }
  return 0;
}

/*
*   read_data()
*   Description: 
*   Return Value
*   Inputs: inode - index node number
            offset - position in file we want to read from
            buf - buffer we want to copy the data to
            length - number of bytes to read starting at offset
*   Outputs: None
*/

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
  inode_t* file_cur;
  uint8_t* data_cur;
  int data_block_num = 0;
  /* parameter checks  */
  if (inode < 0 || inode >= DIR_ENTRIES || buf == NULL 
      || length < 0 || length > (4*INODE_SIZE))
    return -1;

  /* clear buf */
  memset(buf, NULL, length);

  /* get to the proper inode */
  file_cur = &(inode_head[inode]);
  /* get to the spot we want to start reading from */ 
  data_cur = data_blocks +       // start of data blocks  
              (_4KB_ * file_cur->datablock_nums[offset / _4KB_]) +  // data block we want to read from  
              (offset % _4KB_);   // where in that data block we start
  
  /* copies data from data blocks to buf */
  while(length != 0)
  {
    //do read
    memcpy(buf++, data_cur, 1);
    /* move forward */
    data_cur++;
    length--;
    if((data_cur - data_blocks) % _4KB_ == 0 && length > 0){
        data_block_num++;
        data_cur = data_blocks +       // start of data blocks  
                   (_4KB_ * file_cur->datablock_nums[data_block_num]);  // data block we want to read from   
    }
  }
  
  // data block index = offset/4KB --> offset % 4KB is where in the data block we start at
  return 0;
}


