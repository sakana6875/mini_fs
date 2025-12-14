#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stdio.h>

#define MAX_INODES 128
#define MAX_BLOCKS 1024
#define BLOCK_SIZE 512
#define DIRECT_BLOCKS 8

#define INODE_FILE 1
#define INODE_DIR 2

typedef struct inode{
    uint32_t size;
    uint8_t type;
    int32_t blocks[DIRECT_BLOCKS]; 
}inode_t;

typedef struct dir_entry{
    char name[28];
    int inode_id;
}dir_entry_t;

//全局数据
extern uint8_t inode_bitmap[MAX_INODES];
extern uint8_t block_bitmap[MAX_BLOCKS];
extern inode_t inodes_table[MAX_INODES];
extern uint8_t data_blocks[MAX_BLOCKS][BLOCK_SIZE];

//函数声明
int alloc_inode();
int alloc_block();
void init_inode(inode_t* node, uint8_t type);
void free_inode(int ino);
void free_block(int blk);
void fs_init();

#endif