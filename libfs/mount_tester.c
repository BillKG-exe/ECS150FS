#include <stdio.h>

#include "fs.h"


int main(void) {
    printf("\n File System Mounting status: %d", fs_mount("disk.fs"));

    printf("\n File System Mounting status: %d", fs_mount("disk"));

    printf("\n File System Mounting status: %d", fs_mount("dk.fs"));

    return 0;
}