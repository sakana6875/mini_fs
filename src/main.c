#include "fs.h"
#include "dir.h"
#include "path.h"
#include <stdio.h>

int main(){
    fs_init();
    printf("File system initialized.\n");

    printf("Root inode type: %d\n", inodes_table[0].type);

    mkdir_path("/a");
    mkdir_path("/a/b");
    mkdir_path("/a/b/c");

    dir_list(&inodes_table[ROOT_INODE]);
    return 0;
}