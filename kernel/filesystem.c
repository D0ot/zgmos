#include "filesystem.h"
#include "fat32.h"
#include "vfs_fat32.h"
#include "vfs.h"


struct filesystem fs;

void fs_init() {
  struct disk_hal *hal = disk_hal_init();
  fs.hal = hal;
  struct fat32_fs *phyfs = fat32_init(hal, 0, 0, 1);
  fs.phyfs = phyfs;
  
  struct vfs_t *vfs = vfs_init(10);
  struct vfs_backend bkd = fat32bkd(phyfs);
  vfs_mount(vfs, vfs_root(vfs), bkd);

  fs.vfs = vfs;

 
}
