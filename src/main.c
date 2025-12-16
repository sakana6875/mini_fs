#include "fs.h"
#include "dir.h"
#include "path.h"
#include <stdio.h>

static void print_dir(const char* path){
    int ino = lookup_path(path);
    if (ino < 0){
        printf("ls %s: not found\n", path);
        return;
    }

    inode_t* dir = &inodes_table[ino];
    printf("ls %s:\n", path);
    dir_list(dir);
    printf("\n");
}

int main(){
    fs_init();
    printf("File system initialized.\n");

    // 1. root inode
    printf("Root inode type: %d (expect %d)\n",
           inodes_table[ROOT_INODE].type, INODE_DIR);

    // 2. mkdir tests
    mkdir_path("/a");
    mkdir_path("/a/b");
    mkdir_path("/a/b/c");

    // 3. touch tests
    touch_path("/a/file1");
    touch_path("/a/b/file2");
    touch_path("/a/b/c/file3");

    // 重复 touch（应无副作用）
    touch_path("/a/file1");

    // 4. lookup tests
    printf("lookup /a        = %d\n", lookup_path("/a"));
    printf("lookup /a/b/c    = %d\n", lookup_path("/a/b/c"));
    printf("lookup /noexist  = %d (expect -1)\n",
           lookup_path("/noexist"));

    printf("\n");

    // 5. dir_list tests
    print_dir("/");
    print_dir("/a");
    print_dir("/a/b");
    print_dir("/a/b/c");

    return 0;
}
