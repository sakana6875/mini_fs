#ifndef FILE_H
#define FILE_H

#include "fs.h"

#include <stddef.h>
#include <stdint.h>

typedef struct{
    inode_t* inode; // 对应的 inode
    size_t offset;  // 当前文件偏移量
    int flags; // 文件打开的标志
    int refnct; // 文件引用计数
} file_t;

extern file_t* fd_table[MAX_FD]; // 文件描述符表

// 初始化文件描述符表
void file_table_init();

// 读取文件，返回实际读取的字节数，失败返回 -1
int file_read(int fd, char* buf, size_t size);

// 写入文件
int file_write(int fd, const char* buf, size_t size);

// 打开文件
int file_open(const char* path, int flags);

// 关闭文件
int file_close(int fd);

// 删除文件
int file_remove(const char* path);

// 关闭文件
int file_close(int fd);

// 文件追加
size_t file_append(int fd, const char* buf, size_t size);

#endif