#include "fs.h"
#include "dir.h"
#include "path.h"
#include "file.h"
#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#define CWD_INO ROOT_INODE

char* trim(char* str){
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

void cmd_ls(const char* arg){
    int ino = CWD_INO;
    if (arg && strlen(arg) > 0){
        ino = lookup_path(arg);
        if (ino < 0){
            printf("ls: cannot access '%s': No such file or directory\n", arg);
            return;
        }
    }

    inode_t* dir = &inodes_table[ino];
    if (dir->type != INODE_DIR){
        printf("ls: '%s' is not a directory\n", arg ? arg : "/");
        return;
    }
    dir_list(dir);
}

void cmd_mkdir(const char* arg){
    if (!arg || strlen(arg) == 0){
        printf("mkdir: missing operand\n");
        return;
    }
    
    char path[256];
    if (arg[0] != '/'){
        snprintf(path, sizeof(arg), "/%s", arg);
    } else {
        strncpy(path, arg, sizeof(path) - 1);
    }
    path[sizeof(path)-1] = '\0';

    int ino = mkdir_path(path);
    if (ino < 0){
        printf("mkdir: cannot create direction '%s'\n", path);
    }
}

void cmd_touch(const char* arg){
    if (!arg || strlen(arg) == 0){
        printf("touch: missing file operand\n");
        return;
    }

    char path[256];
    if (arg[0] != '/'){
        snprintf(path, sizeof(arg), "/%s", arg);
    } else {
        strncpy(path, arg, sizeof(arg) - 1);
    }
    path[sizeof(path)-1] = '\0';

    int ino = touch_path(path);
    if (ino < 0){
        printf("touch: cannot create file '%s'\n", path);
    }
}

void cmd_cat(const char* arg){
    if (!arg || strlen(arg) == 0){
        printf("cat: missing file operand\n");
        return;
    }

    char path[256];
    if (arg[0] != '/'){
        snprintf(path, sizeof(arg), "/%s", arg);
    } else {
        strncpy(path, arg, sizeof(arg) - 1);
    }
    path[sizeof(path) - 1] = '\0';

    int fd = file_open(path, O_RDONLY);
    if (fd < 0){
        printf("cat: %s: No such file or directory\n", path);
        return;
    }

    char buf[256];
    ssize_t n;
    while ((n = file_read(fd, buf, sizeof(buf))) > 0){
        fwrite(buf, 1, n, stdout);
    }
    file_close(fd);
    fflush(stdout);
}

void cmd_echo_redirect(const char* line) {
    // 1. 检查是否包含 ">"
    const char* redirect = strstr(line, ">");
    if (!redirect || redirect == line) {
        printf("Usage: echo \"text\" > file\n");
        return;
    }

    // 2. 找第一个引号（在 "echo" 之后）
    const char* echo_part = line;
    if (strncmp(line, "echo", 4) == 0) {
        echo_part = line + 4;
        while (*echo_part && isspace((unsigned char)*echo_part)) {
            echo_part++;
        }
    }

    const char* start_quote = strchr(echo_part, '"');
    if (!start_quote) {
        printf("echo: missing opening quote\n");
        return;
    }

    const char* end_quote = strchr(start_quote + 1, '"');
    if (!end_quote) {
        printf("echo: missing closing quote\n");
        return;
    }

    // 3. 提取文本内容
    size_t text_len = end_quote - start_quote - 1;
    char text[256];
    if (text_len >= sizeof(text)) text_len = sizeof(text) - 1;
    memcpy(text, start_quote + 1, text_len);
    text[text_len] = '\0';

    // 4. 提取文件名（">" 之后的部分）
    const char* filename_start = redirect + 1; // 跳过 '>'
    while (*filename_start && isspace((unsigned char)*filename_start)) {
        filename_start++;
    }

    if (*filename_start == '\0') {
        printf("echo: missing filename after '>'\n");
        return;
    }

    // 支持 "> file" 或 ">> file"（忽略 >> 的追加语义，按覆盖处理）
    if (*filename_start == '>') {
        filename_start++; // 跳过第二个 '>'
        while (*filename_start && isspace((unsigned char)*filename_start)) {
            filename_start++;
        }
    }

    // 复制文件名（最多 255 字符）
    char filename[256];
    const char* space_after_filename = strpbrk(filename_start, " \t\n\r");
    size_t fname_len = space_after_filename ? (size_t)(space_after_filename - filename_start) : strlen(filename_start);
    if (fname_len >= sizeof(filename)) fname_len = sizeof(filename) - 1;
    memcpy(filename, filename_start, fname_len);
    filename[fname_len] = '\0';

    if (filename[0] == '\0') {
        printf("echo: invalid filename\n");
        return;
    }

    // 5. 构造绝对路径
    char path[256];
if (filename[0] != '/') {
    if (strlen(filename) >= sizeof(path) - 1) {
        printf("echo: filename too long (max %zu chars)\n", sizeof(path) - 2);
        return;
    }
    snprintf(path, sizeof(path), "/%s", filename);
} else {
    if (strlen(filename) >= sizeof(path)) {
        printf("echo: filename too long (max %zu chars)\n", sizeof(path) - 1);
        return;
    }
    strcpy(path, filename);
}

    // 6. 写入文件
    int fd = file_open(path, O_CREAT | O_WRONLY);
    if (fd < 0) {
        printf("echo: cannot write to '%s'\n", path);
        return;
    }
    file_write(fd, text, strlen(text));
    file_close(fd);
    printf("Wrote %zu bytes to %s\n", strlen(text), path);
}

void cmd_rm(const char* arg){
    if (!arg || strlen(arg) == 0){
        printf("rm: missing operand\n");
        return;
    }
    char path[256];
    if (arg[0] != '/'){
        snprintf(path, sizeof(path), "/%s", arg);
    } else {
        strncpy(path, arg, sizeof(path) - 1);
    }
    path[sizeof(path) - 1] = '\0';

    if (file_remove(path) < 0){
        printf("rm: cannot remove '%s': No such file or it's open\n", path);
    }
}

void cmd_rmdir(const char* arg){
    if (!arg || strlen(arg) == 0){
        printf("rmdir: missing operand\n");
        return;
    }
    char path[256];
    if (arg[0] != '/'){
        snprintf(path, sizeof(path), "/%s", arg);
    } else {
        strncpy(path, arg, sizeof(path) - 1);
    }
    path[sizeof(path) - 1] = '\0';

    int ino = lookup_path(path);
    if (ino < 0){
        printf("rmdir: '%s': No such directory\n", path);
        return;
    }

    inode_t* dir = &inodes_table[ino];
    if (dir->type != INODE_DIR){
        printf("rmdir: '%s': Not a directory\n", path);
        return;
    }

    // 获取父目录并删除目录
    char child_name[256];
    int parent_ino = dir_parent(path, child_name);
    if (parent_ino < 0){
        printf("rmdir: failed to get parent of '%s'\n", path);
        return;
    }

    if (dir_remove_entry(&inodes_table[parent_ino], child_name) < 0){
        printf("rmdir: failed to remove entry\n");
        return;
    }

    for (int i = 0; i < DIRECT_BLOCKS; i++){
        if (dir->blocks[i] != -1){
            free_block(dir->blocks[i]);
        }
        free_inode(ino);
        printf("Directory '%s' removed\n", path);
    }
}

void cmd_help(){
    printf("Available commands:\n");
    printf("  ls [path]          List directory contents\n");
    printf("  mkdir <path>       Create a directory\n");
    printf("  touch <path>       Create an empty file\n");
    printf("  cat <path>         Display file content\n");
    printf("  echo \"text\" > file Write text to file\n");
    printf("  rm <path>          Remove a file\n");
    printf("  rmdir <path>       Remove an empty directory\n");
    printf("  help, ?            Show this help\n");
    printf("  exit, quit         Exit the shell\n");
}

void start_shell(){
    printf("Welcome to mini_fs shell nya~ Type 'help' for commands.\n");
    char line[512];

    while (1){
        printf("mini_fs$ ");
        if (!fgets(line, sizeof(line), stdin)) break;

        // 去除换行符
        line[strcspn(line, "\n")] = '\0';
        char* cmd_line = trim(line);
        if (strlen(cmd_line) == 0) continue;

        // 检查特殊命令
        if (strcmp(cmd_line, "exit") == 0 || strcmp(cmd_line, "quit") == 0){
            printf("Goodbye! nya~\n");
            break;
        }
        if (strcmp(cmd_line, "help") == 0 || strcmp(cmd_line, "?") == 0){
            cmd_help();
            continue;
        }
        if (strcmp(cmd_line, "pwd") == 0){
            printf("/\n");
            continue;
        }

        // 解析命令
        char* token = strtok(cmd_line, " ");
        if (!token) continue;

        char* arg = strtok(NULL, "");
        if (arg) arg = trim(arg);

        if (strcmp(token, "ls") == 0){
            cmd_ls(arg);
        } else if (strcmp(token, "mkdir") == 0){
            cmd_mkdir(arg);
        } else if (strcmp(token, "touch") == 0){
            cmd_touch(arg);
        } else if (strcmp(token, "cat") == 0){
            cmd_cat(arg);
        } else if (strcmp(token, "rm") == 0){
            cmd_rm(arg);
        } else if (strcmp(token, "rmdir") == 0){
            cmd_rmdir(arg);
        } else if (strncmp(cmd_line, "echo", 4) == 0){
            cmd_echo_redirect(cmd_line);
        } else {
            printf("mini_fs: command not found: %s\n", token);
        }
      }
}