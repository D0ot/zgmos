#include "vfs_fat32.h"
#include "fat32.h"
#include "kmem.h"
#include "kustd.h"


#define DEF_FAT32FS \
  struct fat32_fs *fs = lfs

#define DEF_FAT32_OBJ \
  struct fat32_obj *obj = (struct fat32_obj *)lobj

#define DEF_FAT32_POBJ \
  struct fat32_obj *p_obj = (struct fat32_obj *)p_lobj




int fat32_vfs_root(void *lfs, void *lobj) {
  DEF_FAT32FS;
  DEF_FAT32_OBJ;
  fat32_get_root_dir(fs, obj);
  return 0;
}

int fat32_vfs_create(void *lfs);
int fat32_vfs_unlink(void *lfs, void *ojb);

int fat32_vfs_flush(void *lfs) {
  DEF_FAT32FS;
  fat32_flush(fs);
  return 0;
}

int fat32_vfs_mkdir(void *lfs, void *p_lobj, char *name);
int fat32_vfs_rmdir(void *lfs, void *p_lobj, void *lobj);

uint64_t fat32_vfs_read(void *lfs, void *lobj, uint64_t offset, void *buf, uint64_t buf_len) {
  DEF_FAT32FS;
  DEF_FAT32_OBJ;
  return fat32_read(fs, obj, buf, buf_len, offset);
}
uint64_t fat32_vfs_write(void *lfs, void *lobj, uint64_t offset, void *buf, uint64_t buf_len) {
  DEF_FAT32FS;
  DEF_FAT32_OBJ;
  return fat32_write(fs, obj, buf, buf_len, offset);
}

int fat32_vfs_trunate(void *lfs, void *lobj, uint64_t new_sz);
int fat32_vfs_enlarge(void *lfs, void *lobj, uint64_t new_sz);

uint64_t fat32_vfs_size(void *lfs, void *lobj) {
  DEF_FAT32_OBJ;
  return fat32_get_file_size(obj);
}

char *fat32_vfs_name(void *lfs, void *lobj) {
  DEF_FAT32_OBJ;
  return fat32_get_obj_name(obj);
}

void *fat32_vfs_iterate(void *lfs, void *dir_obj, void *iter_obj, void *lobj) {
  DEF_FAT32FS;
  DEF_FAT32_OBJ;

  if(!iter_obj) {
    iter_obj = kmalloc(sizeof(struct fat32_directory_iter));
    fat32_iter_start(fs, dir_obj, iter_obj);
  }

  if(fat32_iter_next(fs, iter_obj, obj)) {
    return iter_obj;
  }else {
    kfree(iter_obj);
    return NULL;
  }
}

void fat32_vfs_end_iterate(void *lfs, void *iter_obj) {
  kfree(iter_obj);
}

void fat32_vfs_lock(void *lfs);
void fat32_vfs_unlock(void *lfs);



struct vfs_backend fat32bkd(struct fat32_fs *lfs) {
  struct vfs_backend bkd;
  memset(&bkd, 0, sizeof(struct vfs_backend));

  bkd.root = fat32_vfs_root;
  bkd.flush = fat32_vfs_flush;
  bkd.read = fat32_vfs_read;
  bkd.write = fat32_vfs_write;
  bkd.size = fat32_vfs_size;
  bkd.name = fat32_vfs_name;
  bkd.iterate = fat32_vfs_iterate;
  bkd.end_iterate = fat32_vfs_end_iterate;

  bkd.lfs = lfs;
  bkd.lfs_obj_size = sizeof(struct fat32_obj);
  return bkd;
}

