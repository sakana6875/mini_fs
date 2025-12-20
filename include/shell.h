#ifndef SHELL_H
#define SHELL_H

void start_shell();

// 去除字符串首尾空格
char* trim(char* str);

// 命令：ls
void cmd_ls(const char* arg);

// 命令：mkdir
void cmd_mkdir(const char* arg);

// 命令：touch
void cmd_touch(const char* arg);

// 命令：cat
void cmd_cat(const char* arg);

// 命令：echo > file (只支持 echo "text" > file)
void cmd_echo_redirect(const char* line);

// 命令：rm
void cmd_rm(const char* arg);

// 命令：rmdir
void cmd_rmdir(const char* arg);

// 命令：显示帮助
void cmd_help();

// 主 shell 循环
void start_shell();

#endif