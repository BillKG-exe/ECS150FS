#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "disk.h"
#include "fs.h"

/** API Value Definitions **/
#define SUPERBLOCK_INDEX 0
#define SUPERBLOCK_PADDING 4079
#define ROOT_DIR_ARRAY_SIZE 128
#define ROOT_DIR_ENTRY_SIZE 32
#define ROOT_DIR_PADDING_SIZE 10
#define BLOCK_SIZE 4096
#define FAT_EOC -1

/* TODO: Attach the attribute "packed" to these data structs */

// The first block of the disk and contains info about the filesystem
struct superBlock {
    int8_t signature[FS_FILENAME_LEN];
    int16_t dsk_blck_amount;
    int16_t root_dir_index;
    int16_t data_blck_index;
    int16_t data_blck_amount;
    int8_t fat_blck_amount;
    int8_t padding[SUPERBLOCK_PADDING];
};

// An entry in the root directory
struct root_entry {
    int8_t filename[FS_FILENAME_LEN];
    int32_t file_size;
    int16_t file_first_index;
    int8_t padding[ROOT_DIR_PADDING_SIZE];
};

// All information about the filesystem - super block, FAT, and root directory
struct fs_system {
    struct superBlock sp;
    struct root_entry root_dir[ROOT_DIR_ARRAY_SIZE];
    uint16_t* fat_blocks;
};

// Create file system struct pointer
struct fs_system* file_system;

// System error detection function
int sys_error_check(int file_size, const char *diskname) {

    /* Check if file signature matches the diskname */
    for (int i = 0; i < file_size; i++) {
        if ((char) file_system->sp.signature[i] != diskname[i]) {
            fprintf(stderr, "Error: File signature is invalid\n");
            return -1;
        }
    }

    /* Check if the total block size is correct */
    // disk_blocks = the disk's block count from "block_disk_count()
    int disk_blocks = block_disk_count();
    if (disk_blocks != file_system->sp.dsk_blck_amount) {
        fprintf(stderr, "Error: Disk Block Length is invalid\n");
        return -1;
    }

    /* Check if the FAT length is correct */
    int disk_fat_count = disk_blocks * 2 / BLOCK_SIZE;
    if (disk_fat_count != file_system->sp.fat_blck_amount) {
        fprintf(stderr, "Error: FAT Length is invalid\n");
        return -1;
    }

    /* Calculate the amount of data blocks */
    int data_blck_len = blocks - (2 + fat_count);

    /* Check if the amount of data block matches that of the loaded disk */
    if (data_blck_len != file_system->sp.data_blck_amount) {
        fprintf(stderr, "Error: Data Block Length is invalid\n");
        return -1;
    }

    /* Check if the index of the root directory is valid */
    if (fat_count + 1 != file_system->sp.root_dir_index) {
        fprintf(stderr, "Error: Root Directory index is invalid\n");
        return -1;
    }

    /* Check if the index of the data block is valid */
    if (fat_count + 2 != file_system->sp.data_blck_index) {
        fprintf(stderr, "Error: Data Block index is invalid\n");
        return -1;
    }

    return 0;
}

/** Open virtual disk and load metadata information **/
int fs_mount(const char *diskname) {
    /* TODO: Phase 1 */

    // Determine valid disk name length
    int file_size = strlen(diskname);
    if (file_size > FS_FILENAME_LEN || !file_size) {
        fprintf(stderr, "Invalid filename\n");
        return -1;
    }

    // Attempt to open given disk
    int success = !block_disk_open(diskname);
    if (!success) {
        fprintf(stderr, "Failed to open disk\n");
        return -1;
    }

    // Allocate memory for the filesystem struct
    file_system = malloc(sizeof(struct fs_system));

    /* Read the first block and store the data in sp object */
    block_read(SUPERBLOCK_INDEX, &file_system->sp);

    // Determine the validity of the superblock data
    if (sys_error_check(file_size, diskname) == -1) {
        return -1;
    }

    /* Create the FAT array with the corresponding size of elements */
    file_system->fat_blocks = malloc(file_system->sp.fat_blck_amount * sizeof(uint16_t *));

    /* Go through the FAT blocks and store the data in the FAT array */
    for (int i = 1; i < file_system->sp.fat_blck_amount + 1; i++) {
        block_read(i, &file_system->fat_blocks[i - 1]);
    }

    // Read root directory block and write into root_entries
    // There the root directory is one block big. No for loop needed
    block_read(file_system->sp.root_dir_index, &file_system->root_dir);

    return 0;
}

/** Close the virtual disk and clean internal data structures **/
int fs_unmount(void) {
    /* TODO: Phase 1 */

    // Persistent Storage - Write all FAT data out to the disk
    for (int i = 2; i < file_system->sp.fat_blck_amount + 1; i++) {
        block_write(i, &file_system->fat_blocks[i - 1]);
    }

    // Persistent Storage - Write all root directory data out to the disk
    block_write(file_system->sp.root_dir_index, &file_system->root_dir);

    // Clean internal data structures - Deallocate memory
    free(file_system->fat_blocks);
    free(file_system);

    // Close virtual disk
    int success = !block_disk_close(diskname);
    if (!success) {
        fprintf(stderr, "No virtual disk is open to close\n");
        return -1;
    }

    return 0;
}

// Prints information about the mounted file system
int fs_info(void) {
    /* TODO: Phase 1 */

    /* Returns -1 if file system was not mounted */
    if (file_system == NULL) {
        fprintf(stderr, "Error: No file system mounted\n");
        return -1;
    }

    printf("FS Info: \n");

    // Total number of blocks for the fs
    printf("total_blk_count=%s\n", file_system->sp.dsk_blck_amount);

    // Fat block index
    unsigned fatBytes = file_system->sp.data_blck_amount * 2);
    printf("fat_blk_count=%d\n", fatBytes / BLOCK_SIZE);

    // Root directory index
    printf("rdir_blk=%d\n", file_system->sp.root_dir_index);

    // Data block index
    printf("data_blk%d\n", file_system->sp.data_blck_index);

    // Number of data blocks
    printf("data_blk_count=%d\n", file_system->sp.data_blk_amount);

    /* TODO: Determine and Caluclate Ratios */
    printf("fat_free_ratio=%d\n", (file_system->sp.fat_blck_amount)/(file_system->sp.padding));
    printf("rdir_free_ratio=%d\n", ((file_system->root_dir.file_size)/BLOCK_SIZE))/(file_system->sp.padding));
    return 0;
}

int isValidName(const char *filename) {
    int count = 0;
    bool isNullTerminated = false;

    /* Check it the filename string is null terminated */
    /* and keep track of the character count of the string */
    for (int i = 0; i < strlen(filename); i++) {
        if (filename[i] == '\0') {
            isNullTerminated = true;
        }
        count++;
    }

    /* Returns -1 if the filename exceeds the filename limit size */
    /* IReturns -1 if string is not null terminated  */
    if (count > FS_FILENAME_LEN || !isNullTerminated) {
        fprintf(stderr, "Filename is either too large or not null terminated\n");
        return -1;
    }

    return 0;
}

int fs_create(const char *filename) {
    /* TODO: Phase 2 */

    /* Check if the file system was mounted. If not then return -1. */
    if (file_system == NULL) {
        fprintf(stderr, "File System not mounted\n");
        return -1;
    }

    /* Check if the provided filename is valid. If not then return -1 */
    if (isValidName(filename) != 0) {
        return -1;
    }

    /* Index of the free entry in the root directory */
    int free_entry_index = -1;
    /* Temporary variable to hold the filename of the different entries
       in the root directory */
    char *entry_filename;

    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        /* Converts the int8_t filename array to an array of characters */
        entry_filename = (char *) file_system->root_dir[i].filename;

        /* Compare the different entries filename of the root directory with
        provided filename. Return -1 if the filename matches one of the entires'  */
        if (strcmp(filename, entry_filename) == 0) {
            fprintf(stderr, "File already exist in the system\n");
            return -1;
        }

        /* if the entry filename is empty assign the index of that entry to
            free_entry_index.*/
        if (entry_filename == '\0') {
            free_entry_index = i;
            break;
        }

        /* Clean entry_filename before being reused */
        memset(entry_filename, 0, sizeof(char));
    }

    if (free_entry_index != -1) {
        for (int i = 0; i < strlen(filename); i++) {
            /* Convert each character of filename to int8_t */
            file_system->root_dir[free_entry_index].filename[i] = (int8_t) filename[i];
        }

        file_system->root_dir[free_entry_index].file_size = 0;
        file_system->root_dir[free_entry_index].file_first_index = FAT_EOC;
    } else {
        fprintf(stderr, "Root directory contains maximum number of file, 128.\n");
        return -1;
    }

    return 0;
}

int fs_delete(const char *filename) {
    /* TODO: Phase 2 */
    return 0;
}

int fs_ls(void) {
    /* TODO: Phase 2 */
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        char *entry_filename = (char *) file_system->root_dir[i].filename;

        fprintf(stdout, "Root Directory Used Entries: \n");

        if (strlen(entry_filename) != 0) {
            fprintf(stdout, "Entry[%d]: filename -> %s\n", i, entry_filename);
        }
    }
    return 0;
}

int fs_open(const char *filename) {
    /* TODO: Phase 3 */
    return 0;
}

int fs_close(int fd) {
    /* TODO: Phase 3 */
    free(file_system);
    return 0;
}

int fs_stat(int fd) {
    /* TODO: Phase 3 */
    return 0;
}

int fs_lseek(int fd, size_t offset) {
    /* TODO: Phase 3 */
    return 0;
}

int fs_write(int fd, void *buf, size_t count) {
    /* TODO: Phase 4 */
    return 0;
}

int fs_read(int fd, void *buf, size_t count) {
    /* TODO: Phase 4 */
    return 0;
}
