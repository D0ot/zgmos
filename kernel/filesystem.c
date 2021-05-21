#include <stddef.h>
#include "filesystem.h"
#include "fat32.h"
#include "vfs_fat32.h"
#include "vfs.h"
#include "console.h"
#include "kmem.h"


struct filesystem fs;

void fs_init() {
  struct disk_hal *hal = disk_hal_init();
  fs.hal = hal;
  struct fat32_fs *phyfs = fat32_init(hal, 0, 0, 1);
  fs.phyfs = phyfs;
  
  struct vfs_t *vfs = vfs_init(10);
  struct vfs_backend bkd = fat32bkd(phyfs);
  vfs_mount(vfs, vfs_root(vfs), bkd);

  // make fat32's root to be spaned
  vfs_get_recursive(vfs, NULL, "dev");
  fs.vfs = vfs;

  // make some dirty magic
  
  struct vnode *root = vfs_root(fs.vfs);

  struct vnode *devdir = kmalloc(sizeof(struct vnode));

  struct vfs_backend *con = console_vfs_bkd();

  list_add(&devdir->list, &root->children);

  devdir->name = "dev";
  devdir->bkd = NULL;
  devdir->child_cnt = 3;
  devdir->parent = root;
  devdir->type = VNODE_MP;
  list_init(&devdir->children);


  struct vnode *indev = kmalloc(sizeof(struct vnode));
  indev->name = "stdin";
  indev->bkd = con;
  indev->parent = devdir;
  indev->type = VNODE_DEV;
  indev->ref_cnt = 0;
  
  struct vnode *outdev = kmalloc(sizeof(struct vnode));
  outdev->name = "stdout";
  outdev->bkd = con;
  outdev->parent = devdir;
  outdev->type = VNODE_DEV;
  outdev->ref_cnt = 0;

  struct vnode *errdev = kmalloc(sizeof(struct vnode));
  errdev->name = "stderr";
  errdev->bkd = con;
  errdev->parent = devdir;
  errdev->type = VNODE_DEV;
  errdev->ref_cnt = 0;
  
  list_add(&indev->list, &devdir->children);
  list_add(&outdev->list, &devdir->children);
  list_add(&errdev->list, &devdir->children);
}
