#ifndef DIR_H
#define DIR_H

#include "fs.h"
#include <stdint.h>
#include <string.h>

// 初始化根目录条目
void init_root_dir_entries();

// 在目录中添加一个新条目
int add_dir_entry(inode_t* dir, const char* name, int inode_id);

// 在目录 inode 中查找指定名称的条目，返回对应的 inode 编号
int dir_lookup(inode_t* dir, const char* name);

// 删除目录 inode 中的指定条目
int dir_remove_entry(inode_t* dir, const char* name);

// 列出目录内容
void dir_list(inode_t* dir);

#endif