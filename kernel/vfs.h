#ifndef __VFS_H_
#define __VFS_H_

#include <stdint.h>
#include "list.h"

#define VNODE_UNDEF 0

// oridinary file
#define VNODE_FILE  1

// a directory
#define VNODE_DIR   2

// mount point, a mount point must be a directory
#define VNODE_MP    3

// device file
#define VNODE_DEV   4

struct vfs_backend {
  struct list_head list;

  void *lfs;
  uint64_t lfs_obj_size;

  //  get root object
  void *(*root)(void *fs);

  // open a file from a parent obj
  void *(*open)(void *fs, void *p_obj, char *name);

  // close the file
  void (*close)(void *fs, void *obj);

  // create a file
  void (*create)(void *fs, void *p_obj, char *name);

  // delete the file
  void (*unlink)(void *fs, void *obj);
  
  // flush the whole fs
  void (*flush)(void *fs);

  // make a dir
  void (*mkdir)(void *fs, void *p_obj, char *name);

  // remove a dir
  void (*rmdir)(void *fs, void *obj);

  // read from file
  uint64_t (*read)(void *fs, void *obj, uint64_t offset, void *buf, uint64_t buf_len);

  // write to file
  uint64_t (*write)(void *fs, void *obj, uint64_t offset, void *buf, uint64_t buf_len);

  void (*trunate)(void *fs, void *obj, uint64_t new_sz);
  void (*enlarge)(void *fs, void *obj, uint64_t new_sz);

  // get file size
  uint64_t (*size)(void *fs, void *obj);

  // get file name
  char *(*name)(void *fs, void *obj);

  // iterate through directory
  // while the iterate , this function will fill the memory to which the obj points
  void *(*iterate)(void *fs, void *dir_obj, void *iter_obj, void *obj);

  // end the iterate, release the resources
  void (*end_iterate)(void *fs, void *iter_obj);

  void (*lock)(void *fs);
  void (*unlock)(void *fs);
};


struct vnode {
  struct list_head list;

  // parent vnode
  // if parent is NULL, it is a root node
  // root node has no name, no size, no name_len and
  // must be a mount point type
  struct vnode *parent;

  // children vnode
  struct list_head children;
  int64_t child_cnt;


  // max size is 4GiB
  uint32_t size;

  uint32_t type;

  // vnode name
  char *name;


  // THE FOLLOWING IS FOR backend
  // lower fs

  void *lfs_obj;

  struct vfs_backend *bkd;
};

struct vfs_block {
  void *buf;
  struct vnode *node;
  // number in block
  uint32_t offset;
};


struct vfs_t {
  struct vnode root;
  struct list_head bkd;

  // in vfs, one buffer is 4KiB
  struct list_head buffer;
  uint32_t buffer_count;
  uint32_t buffer_max;
};

void vnode_add(struct vnode* node, struct vnode *parent, void *lfs_obj);


struct vfs_t *vfs_init(uint32_t buffer_max);

struct vnode *vfs_root(struct vfs_t *vfs);

// the node must be empty
int64_t vfs_mount(struct vfs_t *vfs, struct vnode *node, struct vfs_backend bkd);

int64_t vfs_umount(struct vfs_t *vfs, struct vnode *node);

void vfs_create(struct vfs_t *vfs, struct vnode *parent, char *name);

struct vnode *vfs_get(struct vfs_t *vfs, struct vnode *parent, char *name);

struct vnode *vfs_get_recursive(struct vfs_t *vfs, struct vnode *parent, char *path);

void vfs_unlink(struct vfs_t *vfs, struct vnode *node);

uint64_t vfs_read(struct vfs_t *vfs, struct vnode *node, uint64_t offset, void *buf, uint64_t buf_len);

uint64_t vfs_write(struct vfs_t *vfs, struct vnode *node, uint64_t offset, void *buf, uint64_t buf_len);

void vfs_mkdir(struct vfs_t *vfs, struct vnode *node, char *name);

void vfs_rmdir(struct vfs_t *vfs, struct vnode *node);

// if node == NULL, flush all vfs
void vfs_flush(struct vfs_t *vfs, struct vnode *node);

// release some of the buffer used
void vfs_squash(struct vfs_t *);

#endif // __VFS_H_
