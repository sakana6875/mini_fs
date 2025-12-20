#include "fs.h"
#include "dir.h"
#include <string.h>

// 初始化根目录
void init_root_dir_entries(){
    inode_t* root = &inodes_table[0];
    int blk = root->blocks[0];

    dir_entry_t* entries = (dir_entry_t*)&data_blocks[blk][0];
    int slots = BLOCK_SIZE / sizeof(dir_entry_t);

    for (int j = 0; j < slots; j++){
        entries[j].inode_id = -1;
        entries[j].name[0] = '\0';
    }

    strcpy(entries[0].name, ".");
    entries[0].inode_id = 0;

    strcpy(entries[1].name, "..");
    entries[1].inode_id = 0;
}

// 在目录中添加一个新条目
int add_dir_entry(inode_t* dir, const char* name, int ino){
    if (!dir || dir->type != INODE_DIR){
        return -1;
    }
    if (dir_lookup(dir, name) >= 0){
        return -1;
    }

    for (int i = 0; i < DIRECT_BLOCKS; i++){
        int blk = dir->blocks[i];

        // 如果当前块为空，分配新块
        if (blk == -1){
            blk = alloc_block();
            if (blk == -1){
                printf("Error: no free blocks available\n");
                return -1;
            }
            dir->blocks[i] = blk;

            // 初始化 block 内容为空（使用 -1 表示空槽）
            dir_entry_t* entries = (dir_entry_t*)&data_blocks[blk][0]; 
            for (int j = 0; j < BLOCK_SIZE / sizeof(dir_entry_t); j++){
                entries[j].inode_id = -1;
                entries[j].name[0] = '\0';
            }
        }   
        // 找到当前块的空槽
        dir_entry_t* entries = (dir_entry_t*)&data_blocks[blk][0];
        for (int j = 0; j < BLOCK_SIZE / sizeof(dir_entry_t); j++){
            if (entries[j].inode_id == -1){
                entries[j].inode_id = ino;
                strncpy(entries[j].name, name, sizeof(entries[j].name)-1);
                entries[j].name[sizeof(entries[j].name)-1] = '\0';
                dir->size += 1; // size 表示目录项数量
                return 0;
            }
        }
    }

    printf("Error: direction is full\n");
    return -1;
}

// 删除目录 inode 中的指定条目
int dir_remove_entry(inode_t* dir, const char* name){
    for (int i = 0; i < DIRECT_BLOCKS; i++){
        int blk = dir->blocks[i];
        if (blk == -1) continue;
        else{
            dir_entry_t* entries = (dir_entry_t*)&data_blocks[blk][0];
            for (int j = 0; j < BLOCK_SIZE / sizeof(dir_entry_t); j++){
                if (entries[j].inode_id != -1 && 
                    strcmp(entries[j].name, name) == 0){
                    entries[j].inode_id = -1;
                    entries[j].name[0] = '\0';
                    dir->size -= 1;
                    return 0;// 删除成功
                }
            }
        }
    }
    return -1;
}

// 在目录 inode 中查找指定名称的条目，返回对应的 inode 编号
int dir_lookup(inode_t* dir, const char* name){
    if (!dir || dir->type != INODE_DIR){
        return -1;
    }
    for (int i = 0; i < DIRECT_BLOCKS; i++){
        int blk = dir->blocks[i];
        if (blk == -1) continue;
        else{
            dir_entry_t* entries = (dir_entry_t*)&data_blocks[blk][0];
            for(int j = 0; j < BLOCK_SIZE / sizeof(dir_entry_t); j++){
                if (entries[j].inode_id != -1 && 
                    strcmp(entries[j].name, name) == 0){
                    return entries[j].inode_id;
                }
            }
        }
    }
    return -1;
}

// 列出目录内容
void dir_list(inode_t* dir){
    for (int i = 0; i < DIRECT_BLOCKS; i++){
        int blk = dir->blocks[i];
        if (blk == -1) continue;
        else{
            dir_entry_t* entries = (dir_entry_t*)&data_blocks[blk][0];
            for (int j = 0; j < BLOCK_SIZE / sizeof(dir_entry_t); j++){
                if (entries[j].inode_id != -1  &&
                    entries[j].name[0] != '\0' && 
                    strcmp(entries[j].name, ".") != 0 && 
                    strcmp(entries[j].name, "..") != 0)
                    {
                        printf("%s\n", entries[j].name);
                    }
            }
        }
    }
}

// 判断目录是否为空
// 返回 1 表示空目录，返回 0 表示非空
int dir_is_empty(inode_t* dir){
    for (int i = 0; i < DIRECT_BLOCKS; i++){
        int blk = dir->blocks[i];
        if (blk == -1) continue;

        dir_entry_t* entries = (dir_entry_t*)&data_blocks[blk][0];
        for (int j = 0; j < BLOCK_SIZE / sizeof(dir_entry_t); j++){
            if (entries[j].inode_id != -1){
                if (strcmp(entries[j].name, "..") != 0 &&
            strcmp(entries[j].name, ".") != 0){
                return 0;
                }
            }
        }
    }
    return 1;
}

// 之后用 lookup_path 加上 路径拆分 替代
// 返回这个目录的父 inode
int dir_parent(const char* path, char* child_name){
    if (!path || path[0] != '/'){
        return -1;
    }

    char buf[256];
    strncpy(buf, path, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    int cur_ino = (path[0] == '/') ? ROOT_INODE : ROOT_INODE;
    inode_t* cur = &inodes_table[cur_ino];

    char* token;
    char* saveptr;

    token = strtok_r(buf, "/", &saveptr);
    if (!token) return -1;

    char* next;
    while ((next = strtok_r(NULL, "/", &saveptr)) != NULL){
        if (strcmp(token, ".") == 0){
            // 当前目录
        }
        else if (strcmp(token, "..") == 0){
            int parent_ino = dir_lookup(cur, "..");
            if (cur_ino < 0) return -1;
            cur_ino = parent_ino;
            cur = &inodes_table[cur_ino];
        } else {
            int ino = dir_lookup(cur, token);
            if (ino < 0) return -1;
            cur_ino = ino;
            cur = &inodes_table[cur_ino];
        }
        token = next;
    }

    strncpy(child_name, token, 256);
    child_name[255] = '\0';

    return cur_ino;
}

// 在一个目录里创建一个新的 inode
int dir_create(inode_t* parent, const char* name){
    if (!parent || parent->type != INODE_DIR)
        return -1;

    if (dir_lookup(parent, name) >= 0)
        return -1;

    int ino = alloc_inode();
    if (ino < 0)
        return -1;

    init_inode(&inodes_table[ino], INODE_DIR);

    int blk = alloc_block();
    if (blk < 0){
        free_inode(ino);
        return -1;
    }

    inodes_table[ino].blocks[0] = blk;

    dir_entry_t* entries = (dir_entry_t*)&data_blocks[blk][0];
    int slots = BLOCK_SIZE / sizeof(dir_entry_t);

    for (int i = 0; i < slots; i++){
        entries[i].inode_id = -1;
        entries[i].name[0] = '\0';
    }

    strcpy(entries[0].name, ".");
    entries[0].inode_id = ino;

    strcpy(entries[1].name, "..");
    entries[1].inode_id = parent - inodes_table;

    if (add_dir_entry(parent, name, ino) < 0){
        free_block(blk);
        free_inode(ino);
        return -1;
    }

    return ino;
}