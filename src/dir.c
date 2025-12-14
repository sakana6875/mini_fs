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
                dir->size += 1;
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
                    entries[j].inode_id = 0;
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
    for (int i = 0; i < DIRECT_BLOCKS; i++){
        int blk = dir->blocks[i];
        if (blk == -1) continue;
        else{
            dir_entry_t* entries = (dir_entry_t*)&data_blocks[blk];
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
                if (entries[j].inode_id != -1){
                    printf("%s\n", entries[j].name);
                }
            }
        }
    }
}