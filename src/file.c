#include "fs.h"
#include "file.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#ifndef O_APPEND
#define O_APPEND 0x0008
#endif

int file_read(int fd, char* buf, size_t size){
    if (fd < 0 || fd >= MAX_FD || fd_table[fd] == NULL){
        return -1;
    }

    file_t* f = fd_table[fd];
    inode_t* inode = f->inode;

    if (!inode || inode->type != INODE_FILE){
        return -1;
    }

    size_t to_read = size;
    size_t read = 0;

    if (f->offset + to_read > inode->size){
        to_read = inode->size - f->offset;
    }

    while (to_read > 0){
        int blk_inx = f->offset / BLOCK_SIZE;
        int blk = inode->blocks[blk_inx];

        if (blk == -1){
            break;
        }

        size_t block_offset = f->offset % BLOCK_SIZE;
        size_t to_read_in_block = to_read;
        size_t free_space_in_block = BLOCK_SIZE - block_offset;

        if (to_read_in_block > free_space_in_block){
            to_read_in_block = free_space_in_block;
        }
        
        memcpy(buf + read, &data_blocks[blk][block_offset], to_read_in_block);

        read += to_read_in_block;
        to_read -= to_read_in_block;
        f->offset += to_read_in_block;
    }
    return read;
}

int file_write(int fd, const char* buf, size_t size){
    if (fd < 0 || fd >= MAX_FD || fd_table[fd] == NULL){
        return -1;
    }

    file_t* f = fd_table[fd];
    inode_t* inode = f->inode;
    
    if (!inode || inode->type != INODE_FILE){
        return -1;
    }

    size_t to_write = size;
    size_t written = 0;

    if (f->offset + to_write > inode->size){
        inode->size = f->offset + to_write;
    }

    while (to_write > 0){
        int blk_inx = f->offset / BLOCK_SIZE;
        int blk = inode->blocks[blk_inx];

        if (blk == -1){
            blk = alloc_block();
            if (blk == -1){
                return -1;
            }
            inode->blocks[blk_inx] = blk;
        }

        size_t to_write_in_block = to_write;
        size_t block_offset = f->offset & BLOCK_SIZE;
        size_t free_space_in_block = BLOCK_SIZE - block_offset;

        if (to_write_in_block > free_space_in_block){
            to_write_in_block = free_space_in_block;
        }
        
        memcpy(&data_blocks[blk][block_offset], buf + written, to_write_in_block);

        written += to_write_in_block;
        to_write -= to_write_in_block;
        f->offset += to_write_in_block;
    }
    return written;
}

int file_open(const char* path, int flags){
    int ino = lookup_path(path);
    if (ino < 0){
        return -1;
    }

    inode_t* inode = &inodes_table[ino];
    if (inode->type != INODE_FILE){
        return -1;
    }

    int fd = -1;
    for (int i = 0; i < MAX_FD; i++){
        if (fd_table[i] == NULL){
            fd = i;
            break;
        }
    }

    if (fd == -1){
        return -1;
    }

    file_t* file = (file_t*)malloc(sizeof(file_t));
    if (!file){
        return -1;
    }

    file->inode = inode;
    file->flags = flags;
    file->refnct = 1;

    if (flags & O_APPEND){
        file->offset = inode->size;
    } else {
        file->offset = 0;
    }

    fd_table[fd] = file;

    return fd;
}