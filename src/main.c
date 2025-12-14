#include "fs.h"
#include <stdio.h>

int main(){
    fs_init();
    printf("File system initialized.\n");

    printf("Root inode type: %d\n", inodes_table[0].type);
    return 0;
}