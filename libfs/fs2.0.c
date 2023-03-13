#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

/* TODO: Phase 1 */
#define SUPERBLOCK_INDEX 0
#define SUPERBLOCK_PADDING 4079
#define ROOT_DIR_ENTRY_SIZE 32
#define ROOT_DIR_PADDING_SIZE 10
#define VIRT_DISK_BLOCK_SIZE 4096
#define FAT_EOC -1

// The firts block of the disk and contains info about the filesystem
struct superBlock {
  int8_t signature[FS_FILENAME_LEN];
  int16_t dsk_block_length;
  int16_t root_dir_index;
  int16_t data_blck_index;
  int16_t data_blck_length;
  int8_t fat_length;
  int8_t padding[SUPERBLOCK_PADDING];
};

// An entry in the root directory
struct root_entry {
  int8_t filename[FS_FILENAME_LEN];
  int32_t file_size;
  int16_t file_first_index;
  int8_t padding[ROOT_DIR_PADDING_SIZE];
};


struct fs_system {
  struct superBlock sp;

  // array of root_entry structs
  struct root_entry rootEntries[FS_FILE_MAX_COUNT];
  uint16_t* fat_blocks;
};

// Initialize a filesystem object
struct fs_system* file_system;

// System error detection function
int sys_error_check(int file_size,const char* diskname) {
  int blocks = block_disk_count();
  int fat_count = blocks * 2 / BLOCK_SIZE;

  /* Checks if file signature matches the diskname */
  for (int i = 0; i < file_size; i++) {
    if((char)file_system->sp.signature[i] != diskname[i]) {
      fprintf(stderr, "Error: File signature is invalid\n");
      return -1;
    }
  }

  /* Checks if the total block size is correct */
  if (blocks != file_system->sp.dsk_block_length) {
    fprintf(stderr, "Error: Disk Block Length is invalid\n");
    return -1;
  }

  /* Checks is the FAT length is correct */
  if (fat_count != file_system->sp.fat_length) {
    fprintf(stderr, "Error: FAT Length is invalid\n");
    return -1;
  }

  /* Calculate the amount of data block */
  int data_blck_len = blocks - (2 + fat_count);

  /* Checks if amount of data block matches that of the loded disk */
  if (data_blck_len != file_system->sp.data_blck_length) {
    fprintf(stderr, "Error: Data Block Length is invalid\n");
    return -1;
  }

  /* Checks if the index of the root directory is correct */
  if (fat_count + 1 != file_system->sp.root_dir_index) {
    fprintf(stderr, "Error: Root Directory index is invalid\n");
    return -1;
  }

  /* Checks if the index of the data block is correct */
  if (fat_count + 2 != file_system->sp.data_blck_index) {
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

  // Allocate memory for the filesystem struct
  file_system = malloc(sizeof(struct fs_system));

  /* Reads the first block and store the data in sp object */
  block_read(SUPERBLOCK_INDEX, &file_system->sp);

  // Determines validity of the superblock data
  if (sys_error_check(file_size, diskname) == -1) return -1;

  /* Creates the FAT array with the corresponding size of elements */
  file_system->fat_blocks = malloc(file_system->sp.fat_length * sizeof(uint16_t*));

  /* Go through the FAT blocks and store the data in the FAT array */
  for(int i = 1; i < file_system->sp.fat_length + 1; i++) {
     block_read(i, &file_system->fat_blocks[i-1]);
  }

  // Reads entire 128 entries of the root directory block amd write into
  // root_entries
  block_read(file_system->sp.root_dir_index, &file_system->rootEntries);

  return 0;
}

int fs_unmount(void)
{
  /* TODO: Phase 1 */
  /* Iterate through the data in the FAT array and write to the disk */
  for(int i = 1; i < file_system->sp.fat_length + 1; i++) {
     block_write(i, &file_system->fat_blocks[i - 1]);
    }

  // Deallocate memory
  free(file_system->fat_blocks);

  return 0;
}

int fs_info(void)
{
  /* TODO: Phase 1 */

  /* Returns -1 if file system was not mounted */
  if (file_system == NULL) {
    fprintf(stderr, "Error: No file system mounted\n");
    return -1;
  }

  printf("FILE SYSTEM IMFORMATION: \n");
  printf("\n");

  printf("File System Signature:     %s\n", file_system->sp.signature);
  printf("Total Number of Blocks:    %d\n", file_system->sp.dsk_block_length);
  printf("Root Directory Index:      %d\n", file_system->sp.root_dir_index);
  printf("Data Block Index:          %d\n", file_system->sp.data_blck_index);
  printf("Data Block Length:         %d\n", file_system->sp.data_blck_length);
  printf("Fat length:                %d\n", file_system->sp.fat_length);
  return 0;
}

int isValidName(const char *filename) {
  int count = 0;
  int isNullTerminated = 0;

  /* Check it the filename string is null terminated */
  /* and keep track of the character count of the string */
  for(int i = 0; i < strlen(filename); i++) {
    if(filename[i] == '\0') {
      isNullTerminated = 1;
    }
    count++;
  }

  /* Returns -1 if the filename exceeds the filename limit size */
  /* IReturns -1 if string is not null terminated  */
  if(count > FS_FILENAME_LEN || !isNullTerminated) {
    fprintf(stderr, "Filename is either too large or not null terminated\n");
    return -1;
  }

  return 0;
}

int fs_create(const char *filename)
{
  /* TODO: Phase 2 */
  /* Check if the file system was mounted. If not then return -1. */
  if(file_system == NULL) {
    fprintf(stderr, "File System not mounted\n");
    return -1;
  }

  /* Check if the provided filename is valid. If not then return -1 */
  if(isValidName(filename) != 0) {
    return -1;
  }

  /* Index of the free entry in the root directory */
  int free_entry_index = -1;
  /* Temporary variable to hold the filename of the different entries 
     in the root directory */
  char* entry_filename;

  for(int i = 0; i < FS_FILE_MAX_COUNT; i++) {
    /* Converts the int8_t filename array to an array of characters */
    entry_filename = (char*)file_system->rootEntries[i].filename;

    /* Compare the different entries filename of the root directory with
    provided filename. Return -1 if the filename matches one of the entires'  */
    if(strcmp(filename, entry_filename) == 0) {
        fprintf(stderr, "File already exist in the system\n");
        return -1;
    }

    /* if the entry filename is empty assign the index of that entry to
        free_entry_index.*/
    if(entry_filename == '\0') {
        free_entry_index = i;
        break;
    }

    /* Clean entry_filename before being reused */
    memset(entry_filename, 0, sizeof(char));
  }

  if(free_entry_index != -1) {
    for(int i = 0; i < strlen(filename); i++) {
        /* Convert each character of filename to int8_t */
        file_system->rootEntries[free_entry_index].filename[i] = (int8_t)filename[i];
    }

    file_system->rootEntries[free_entry_index].file_size = 0;
    file_system->rootEntries[free_entry_index].file_first_index = FAT_EOC;
  } else {
    fprintf(stderr, "Root directory contains maximum number of file, 128.\n");
    return -1;
  }

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
  for(int i = 0; i < FS_FILE_MAX_COUNT; i++) {
    char* entry_filename = (char*)file_system->rootEntries[i].filename;

    fprintf(stdout, "Root Directory Used Entries: \n");

    if(strlen(entry_filename) != 0 ) {
        fprintf(stdout, "Entry[%d]: filename -> %s\n", i, entry_filename);
    }
  }
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
  free(file_system);
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