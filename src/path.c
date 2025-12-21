#include "fs.h"
#include "dir.h"
#include "path.h"
#include <string.h>

int lookup_path(const char* path){
    int cur = ROOT_INODE;

    char buf[256];
    strncpy(buf, path, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    char* token = strtok(buf, "/");
    while(token){
        inode_t* dir = &inodes_table[cur];

        if (dir->type != INODE_DIR){
            return -1;
        }

        int next = dir_lookup(dir, token);
        if (next < 0) return -1;

        cur = next;
        token = strtok(NULL, "/");
    }
    return cur;
}

int mkdir_path(const char* path){
    if (!path || path[0] == '\0'){
        return -1;
    }

    char buf[256];
    strncpy(buf, path, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    char* slash = strrchr(buf, '/');

    char* name;
    int parent_ino;

    if (slash == NULL){
        parent_ino = ROOT_INODE;
        name = buf;
    } else {
        if (slash == buf){
            parent_ino = ROOT_INODE;
        } else {
            *slash = '\0';
            parent_ino = lookup_path(buf);
        }
        name = slash + 1;
    }

    if (parent_ino < 0 || name[0] == '\0'){
        return -1;
    }

    inode_t* parent = &inodes_table[parent_ino];

    return dir_create(parent, name);
}

int touch_path(const char* path){
    if (!path || path[0] == '\0'){
        return -1;
    }

    char buf[256];
    strncpy(buf, path, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    char* slash = strrchr(buf, '/');

    char* name;
    int parent_ino;

    if (slash == NULL){
        name = buf;
        parent_ino = ROOT_INODE;
    } else {
        if (slash == buf){
            parent_ino = ROOT_INODE;
        } else {
            *slash = '\0';
            parent_ino = lookup_path(buf);
        }
        name = slash + 1;
    }

    if (parent_ino < 0 || name[0] == '\0'){
        return -1;
    }

    inode_t* parent = &inodes_table[parent_ino];

    if (parent->type != INODE_DIR){
        return -1;
    }

    if (dir_lookup(parent, name) >= 0){
        return dir_lookup(parent, name);
    }

    int ino = alloc_inode();
    if (ino < 0){
        return -1;
    }

    init_inode(&inodes_table[ino], INODE_FILE);

    if (add_dir_entry(parent, name, ino) < 0){
        free_inode(ino);
        return -1;
    }

    return ino;
}