#include "fs.h"
#include <string.h>


uint8_t inode_bitmap[MAX_INODES];
uint8_t block_bitmap[MAX_BLOCKS];
inode_t inodes_table[MAX_INODES];
uint8_t data_blocks[MAX_BLOCKS][BLOCK_SIZE];

int alloc_inode(){
    for (int i = 0; i < MAX_INODES; i++){
        if (inode_bitmap[i] == 0){
            inode_bitmap[i] = 1;
            return i;
        }
    }
    return -1;
}

int alloc_block(){
    for (int i = 0; i < MAX_BLOCKS; i++){
        if (block_bitmap[i] == 0){
            block_bitmap[i] = 1;
            return i;
        }
    }
    return -1;
}

void init_inode(inode_t* node, uint8_t type){
    node->size =  0;
    node->type = type;
    for (int i = 0; i < DIRECT_BLOCKS; i++){
        node->blocks[i] = -1;
    }
}

void free_inode(int ino){
    if (ino >= 0 && ino < MAX_INODES){
        inode_bitmap[ino] = 0;
    }
}

void free_block(int blk){
    if (blk >= 0 && blk <MAX_BLOCKS){
        block_bitmap[blk] = 0;
    }
}

// 初始化根目录
void init_root_dir_entries(int root_inode, int blk){
    dir_entry_t entries[2];

    strcpy(entries[0].name, ".");
    entries[0].inode_id = root_inode;

    strcpy(entries[1].name, "..");
    entries[1].inode_id = root_inode;

    memcpy(&data_blocks[blk][0], entries, sizeof(entries));
}

// 初始化文件系统
void fs_init(){
    memset(inode_bitmap, 0, sizeof(inode_bitmap));
    memset(block_bitmap, 0, sizeof(block_bitmap));

    int root = alloc_inode();
    init_inode(&inodes_table[root], INODE_DIR);

    int blk = alloc_block();
    inodes_table[root].blocks[0] = blk;
}