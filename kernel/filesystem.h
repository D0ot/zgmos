#ifndef __FILESYSTEM_H_
#define __FILESYSTEM_H_

#include "fat32.h"
#include "vfs_fat32.h"
#include "vfs.h"
#include "../hal//disk_hal.h"

struct filesystem {
  struct disk_hal *hal;
  struct fat32_fs *phyfs;
  struct vfs_t *vfs;
};

extern struct filesystem fs;

void fs_init();



#endif // __FILESYSTEM_H_
