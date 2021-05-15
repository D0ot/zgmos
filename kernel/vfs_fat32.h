#ifndef __VFS_FAT32_H_
#define __VFS_FAT32_H_

#include "vfs.h"
#include "fat32.h"

struct vfs_backend fat32bkd(struct fat32_fs *lfs);

int fat32_vfs_root(void *fs, void *lobj);
int fat32_vfs_create(void *fs);
int fat32_vfs_unlink(void *fs, void *lobj);
int fat32_vfs_flush(void *fs);

int fat32_vfs_mkdir(void *fs, void *p_lobj, char *name);
int fat32_vfs_rmdir(void *fs, void *p_lobj, void *lobj);

uint64_t fat32_vfs_read(void *fs, void *lobj, uint64_t offset, void *buf, uint64_t buf_len);
uint64_t fat32_vfs_write(void *fs, void *lobj, uint64_t offset, void *buf, uint64_t buf_len);

int fat32_vfs_trunate(void *fs, void *lobj, uint64_t new_sz);
int fat32_vfs_enlarge(void *fs, void *lobj, uint64_t new_sz);
uint64_t fat32_vfs_size(void *fs, void *lobj);
char *fat32_vfs_name(void *fs, void *lobj);
void *fat32_vfs_iterate(void *fs, void *dir_obj, void *iter_obj, void *lobj);
void fat32_vfs_end_iterate(void *fs, void *iter_obj);
void fat32_vfs_lock(void *fs);
void fat32_vfs_unlock(void *fs);


#endif // __VFS_FAT32_H_
