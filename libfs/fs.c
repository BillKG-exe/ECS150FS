#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"


struct superBlock {
  int8_t signature[FS_FILENAME_LEN];
  int16_t dsk_block_length;
  int16_t root_dir_index;
  int16_t data_blck_index;
  int16_t data_blck_length;
  int8_t fat_length;
};

struct root_dirs {
  char filename[FS_FILENAME_LEN];
  int32_t file_size;
  int16_t file_first_index;
};

struct fs_system {
  struct superBlock sp;
  struct root_dirs rootDirectores[FS_FILE_MAX_COUNT];
  int* fat_blocks;
};

struct fs_system* file_system;

/* TODO: Phase 1 */

int fs_mount(const char *diskname)
{
  /* TODO: Phase 1 */
  int file_size = strlen(diskname);
  
  if(file_size > FS_FILENAME_LEN || file_size <= 0) return -1;

  int success = block_disk_open(diskname);

  if(!success) return -1;

  file_system = malloc(sizeof(struct fs_system));

  
  
  
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