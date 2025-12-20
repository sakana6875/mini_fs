#ifndef DIR_H
#define DIR_H

#include "fs.h"
#include <stdint.h>
#include <string.h>

typedef struct dir_entry{
    char name[28];
    int inode_id;
}dir_entry_t;


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

// 判断目录是否为空
int dir_is_empty(inode_t* dir);

// 返回这个目录的父 inode
int dir_parent(const char* path, char* child_name);

// 在一个目录里创建一个新 inode
int dir_create(inode_t* parent, const char* name);

#endif