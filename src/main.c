// main.c
#include "fs.h"
#include "file.h"
#include "dir.h"
#include "shell.h"
#include <stdio.h>

int main() {
    fs_init();
    file_table_init();
    init_root_dir_entries(); // 初始化根目录的 . 和 ..

    start_shell(); // 启动交互式 shell

    return 0;
}