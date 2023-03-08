#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

/* TODO: Phase 1 */

#define SUPERBLOCK_INDEX 0
#define ROOT_DIR_ENTRY_SIZE 32

struct superBlock {
  int8_t signature[FS_FILENAME_LEN];
  int16_t dsk_block_length;
  int16_t root_dir_index;
  int16_t data_blck_index;
  int16_t data_blck_length;
  int8_t fat_length;
};

struct root_dirs {
  int8_t filename[FS_FILENAME_LEN];
  int32_t file_size;
  int16_t file_first_index;
};

struct fs_system {
  struct superBlock sp;
  struct root_dirs rootDirectores[FS_FILE_MAX_COUNT];
  uint16_t* fat_blocks;
};

struct fs_system* file_system;

int sys_error_check(int file_size,const char* diskname) {
  int blocks = block_disk_count();
  int fat_count = blocks * 2 / BLOCK_SIZE;

  /* Checks if file signature matches the diskname */
  for(int i = 0; i < file_size; i++) {
    if((char)file_system->sp.signature[i] != diskname[i]) {
      fprintf(stderr, "Error: File signature is invalid\n");
      return -1;
    }
  }
  
  /* Checks if the total block size is correct */
  if(blocks != file_system->sp.dsk_block_length) {
    fprintf(stderr, "Error: Disk Block Length is invalid\n");
    return -1;
  }

  /* Checks is the FAT length is correct */
  if(fat_count != file_system->sp.fat_length) {
    fprintf(stderr, "Error: FAT Length is invalid\n");
    return -1;
  }

  /* Calculate the amount of data block */
  int data_blck_len = blocks - (2 + fat_count);

  /* Checks if amount of data block matches that of the loded disk */
  if(data_blck_len != file_system->sp.data_blck_length) {
    fprintf(stderr, "Error: Data Block Length is invalid\n");
    return -1;
  }

  /* Checks if the index of the root directory is correct */
  if(fat_count + 1 != file_system->sp.root_dir_index) {
    fprintf(stderr, "Error: Root Directory index is invalid\n");
    return -1;
  }

  /* Checks if the index of the data block is correct */
  if(fat_count + 2 != file_system->sp.data_blck_index) {
    fprintf(stderr, "Error: Data Block index is invalid\n");
    return -1;
  }
  
  return 0;  
}

int fs_mount(const char *diskname)
{
  /* TODO: Phase 1 */
  int file_size = strlen(diskname);
  
  if(file_size > FS_FILENAME_LEN || !file_size) {
    fprintf(stderr, "Invalid filename\n");
    return -1;
  }

  int success = !block_disk_open(diskname);
  if(!success) {
    fprintf(stderr, "Failed to open disk\n");
    return -1;
  }

  file_system = malloc(sizeof(struct fs_system));

  /* Reads the first block and store the data in sp object */
  block_read(SUPERBLOCK_INDEX, &file_system->sp);

  if(sys_error_check(file_size, diskname) == -1) return -1;

  char* root_block;

  /* Reads the entire block for the root directory int root_block
     buffer */
  block_read(file_system->sp.root_dir_index, root_block);

  /* Creates the FAT array with the corresponding size of elements */
  file_system->fat_blocks = malloc(file_system->sp.fat_length *       
                                  sizeof(uint16_t*));

  /* Go through FAT blocks and stores the data in the FAT array */
  for(int i = 1; i < file_system->sp.fat_length+1; i++) {
     block_read(i, &file_system->fat_blocks[i]);
  }

  
  /* 
    Not quite sure about the logic we have to use to get the different
    entries of the root directory.
  */
  for(int i = 0; i < BLOCK_SIZE; i+=ROOT_DIR_ENTRY_SIZE) {
    char temp_buffer[ROOT_DIR_ENTRY_SIZE+1];
    /* What logic should we apply?*/
    //sscanf()
  }
    
  return 0;
}

int fs_umount(void)
{
  /* TODO: Phase 1 */
  return 0;
}

int fs_info(void)
{
  /* TODO: Phase 1 */
  
  /* Returns -1 if file system was not mounted */
  if(file_system == NULL) {
    fprintf(stderr, "Error: No file system mounted\n");
    return -1;
  }

  printf("FILE SYSTEM IMFORMATION: \n");
  printf("\n");

  printf("File System Signature:     %s\n", file_system->sp.signature);
  printf("Total Number of Blocks:    %d\n", file_system->sp.dsk_block_length);
  printf("Root Directory Index:      %d\n", file_system->sp.root_dir_index);
  printf("Data Block Index:          %d\n", file_system->sp.data_blck_index);
  printf("Data Block Length          %d\n", file_system->sp.data_blck_length);
  printf("Fat length                 %d\n", file_system->sp.fat_length);
  return 0;
}

int fs_create(const char *filename)
{
  /* TODO: Phase 2 */
  return 0;
}

int fs_delete(const char *filename)
{
  /* TODO: Phase 2 */
  return 0;
}

int fs_ls(void)
{
  /* TODO: Phase 2 */
  return 0;
}

int fs_open(const char *filename)
{
  /* TODO: Phase 3 */
  return 0;
}

int fs_close(int fd)
{
  /* TODO: Phase 3 */
  return 0;
}

int fs_stat(int fd)
{
  /* TODO: Phase 3 */
  return 0;
}

int fs_lseek(int fd, size_t offset)
{
  /* TODO: Phase 3 */
  return 0;
}

int fs_write(int fd, void *buf, size_t count)
{
  /* TODO: Phase 4 */
  return 0;
}

int fs_read(int fd, void *buf, size_t count)
{
        /* TODO: Phase 4 */
  return 0;
}