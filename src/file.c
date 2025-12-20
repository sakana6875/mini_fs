#include "fs.h"
#include "dir.h"
#include "path.h"
#include "file.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

file_t* fd_table[MAX_FD];

void file_table_init(){
    for (int i = 0; i < MAX_FD; i++){
        fd_table[i] = NULL;
    }
}

int file_read(int fd, char* buf, size_t size){
    if (fd < 0 || fd >= MAX_FD || fd_table[fd] == NULL){
        return -1;
    }
    
    file_t* f = fd_table[fd];
    inode_t* inode = f->inode;

    if ((f->flags & O_WRONLY) && !(f->flags & O_RDWR)){
    return -1;
    }


    if (!inode || inode->type != INODE_FILE){
        return -1;
    }

    size_t to_read = size;
    size_t read = 0;

    if (f->offset >= inode->size){
        return 0;
    }

    if (f->offset + to_read > inode->size){
        to_read = inode->size - f->offset;
    }

    while (to_read > 0){
        int blk_inx = f->offset / BLOCK_SIZE;
        int blk = inode->blocks[blk_inx];

        if (blk == -1){
            break;
        }

        if (blk_inx >= DIRECT_BLOCKS){
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

    if ((f->flags & O_RDONLY) && !(f->flags & O_RDWR)){
    return -1;
    }
    
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

        if (blk_inx >= DIRECT_BLOCKS){
            return -1;
        }

        int blk = inode->blocks[blk_inx];

        if (blk == -1){
            blk = alloc_block();
            if (blk == -1){
                return -1;
            }
            inode->blocks[blk_inx] = blk;
            memset(data_blocks[blk], 0, BLOCK_SIZE);
        }

        size_t to_write_in_block = to_write;
        size_t block_offset = f->offset % BLOCK_SIZE;
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
        if (flags & O_CREAT){
            ino = touch_path(path);
            if (ino < 0) return -1;
        } else{
            return -1;
        }
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

int file_close(int fd){
    if (fd < 0 || fd >= MAX_FD || fd_table[fd] == NULL){
        return -1;
    }

    file_t* f = fd_table[fd];

    f->refnct--;
    if (f->refnct == 0){
        free(f);
    }

    fd_table[fd] =NULL;

    return 0;
}

int file_remove(const char* path){
    int ino = lookup_path(path);
    if (ino < 0){
        return -1;
    }
    

    inode_t* inode = &inodes_table[ino];
    if (inode->type != INODE_FILE){
        return -1;
    }


    for (int i = 0; i < MAX_FD; i++){
    if (fd_table[i] && fd_table[i]->inode == inode){
        return -1; // 文件正在被打开
        }
    }

    char child_name[256];
    int parent_ino = dir_parent(path, child_name);
    if (parent_ino < 0){
        return -1;
    }

    inode_t* parent_inode = &inodes_table[parent_ino];

    if (dir_remove_entry(parent_inode, child_name) < 0){
        return -1;
    }

    for (int i = 0; i < DIRECT_BLOCKS; i++){
        if (inode->blocks[i] != -1){
            free_block(inode->blocks[i]);
        }
    }
    free_inode(ino);
    return 0;
}


size_t file_append(int fd, const char* buf, size_t size){

    if (fd < 0 || fd >= MAX_FD || fd_table[fd] == NULL){
        return -1;
    }

    file_t* f = fd_table[fd];
    inode_t* inode = f->inode;

    if (inode->type != INODE_FILE){
        return -1;
    }

    f->offset = inode->size;
    return file_write(fd, buf, size);
}

int file_truncate(int fd, size_t new_size){
    if (fd < 0 || fd >= MAX_FD || fd_table[fd] == NULL){
        return -1;
    }

    file_t* f = fd_table[fd];
    inode_t* inode = f->inode;

    size_t old_size = inode->size;
    inode->size = new_size;

    size_t old_blocks = (old_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    size_t new_blocks = (new_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (new_blocks < old_blocks){
        for (size_t i = new_blocks; i < old_blocks && i < DIRECT_BLOCKS; i++){
            if (inode->blocks[i] != -1){
                free_block(inode->blocks[i]);
                inode->blocks[i] = -1;
            }
        }
    }

    if (new_blocks > old_blocks){
        for (size_t i = old_blocks; i < new_blocks && i < DIRECT_BLOCKS; i++){
            if (inode->blocks[i] == -1){
                int blk = alloc_block();
                if (blk == -1){
                    inode->size = old_size;
                    return -1;
                }
                inode->blocks[i] = blk;
                memset(data_blocks[blk], 0, BLOCK_SIZE);
            }
        }
    }
    return 0;
}


int file_lseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= MAX_FD || fd_table[fd] == NULL) {
        return -1;
    }
    file_t* f = fd_table[fd];
    inode_t* inode = f->inode;

    off_t new_offset;
    switch (whence) {
        case SEEK_SET:
            new_offset = offset;
            break;
        case SEEK_CUR:
            new_offset = f->offset + offset;
            break;
        case SEEK_END:
            new_offset = inode->size + offset;
            break;
        default:
            return -1;
    }

    if (new_offset < 0) {
        return -1;
    }

    f->offset = new_offset;
    return f->offset;
}