#ifndef __VFS_H_
#define __VFS_H_

#include <stdint.h>
#include "list.h"
#include <stdbool.h>

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
  // 
  // all the functions below return int as error code
  // 0 for success, other for failed
  //

  struct list_head list;

  void *lfs;
  uint64_t lfs_obj_size;

  //  get root object
  int (*root)(void *lfs, void *lobj);

  // create a file, return error code
  int (*create)(void *lfs, void *p_lobj, char *name);

  // delete the file
  int (*unlink)(void *lfs, void *lobj);
  
  // flush the whole fs
  int (*flush)(void *lfs);

  // make a dir
  int (*mkdir)(void *lfs, void *p_lobj, char *name);

  // remove a dir
  int (*rmdir)(void *lfs, void *p_lobj, void *lobj);

  // read from file
  uint64_t (*read)(void *lfs, void *lobj, uint64_t offset, void *buf, uint64_t buf_len);

  // write to file
  uint64_t (*write)(void *lfs, void *lobj, uint64_t offset, void *buf, uint64_t buf_len);

  int (*trunate)(void *lfs, void *lobj, uint64_t new_sz);
  int (*enlarge)(void *lfs, void *lobj, uint64_t new_sz);

  // get file size
  uint64_t (*size)(void *lfs, void *lobj);

  // get file name
  char *(*name)(void *lfs, void *lobj);

  // iterate through directory
  // while the iterate , this function will fill the memory to which the obj points
  void *(*iterate)(void *lfs, void *dir_obj, void *iter_obj, void *lobj);

  // end the iterate, release the resources
  void (*end_iterate)(void *lfs, void *iter_obj);

  void (*lock)(void *lfs);
  void (*unlock)(void *lfs);
};


struct vnode {
  struct list_head list;

  // block buffer
  struct list_head bbf;

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

#define VFS_BLOCK_SIZE (PAGE_SIZE)

struct vfs_block {
  // used by vfs
  struct list_head list_vfs;
  // used by vnode
  struct list_head list_vnode;
  void *buf;
  struct vnode *node;
  // number in block
  uint32_t blkoff;

  // 0 is clean, 1 is dirty
  uint32_t dirty;
};



struct vfs_t {
  struct vnode root;
  struct list_head bkd;

  // in vfs, one buffer is 4KiB
  
  struct list_head bbf_used;
  struct list_head bbf_free;
  uint32_t bbf_used_cnt;
  uint32_t bbf_free_cnt;

  uint32_t bbf_total_max;
};

// only alloc the memory, other things are not touched, only called by vbf_* series functions
struct vfs_block *vblk_alloc();
// only free the memory, other things are not touched, only called by vbf_* series functions
void vblk_free(struct vfs_block *blk);

// this function is called after vbf_borrow
void vbf_bind(struct vfs_block *blk, struct vnode *node, uint32_t blkoff);

struct vfs_block *vbf_chkbufed(struct vfs_t *vfs, struct vnode *node, uint32_t blkoff);

// this function is called before vbf_return
void vbf_unbind(struct vfs_block *blk);


// flush the block into backend
void vbf_flush(struct vfs_block *blk);

void vbf_activate(struct vfs_t *vfs, struct vfs_block *blk);


// bind must be called after this function call
// use this function to get a unbind vfs_block
struct vfs_block *vbf_borrow(struct vfs_t *vfs);

// unbind must be called before this function call
// use this function to return a unbind vfs_block
void vbf_return(struct vfs_t *vfs, struct vfs_block *blk);

// free all free block
void vbf_shrink(struct vfs_t *vfs);


void vnode_add(struct vnode* node, struct vnode *parent, void *lfs_obj);


void vfs_buffer(struct vfs_t *vfs, struct vnode *node, uint32_t blkoff);

void vfs_unbuffer(struct vfs_t *vfs, struct vfs_block *blk);
void vfs_unbuffer_all(struct vfs_t *vfs, struct vnode *node);

struct vfs_t *vfs_init(uint32_t buffer_max);

struct vnode *vfs_root(struct vfs_t *vfs);

// the node must be empty
int64_t vfs_mount(struct vfs_t *vfs, struct vnode *node, struct vfs_backend bkd);

int64_t vfs_umount(struct vfs_t *vfs, struct vnode *node);

void vfs_create(struct vfs_t *vfs, struct vnode *parent, char *name);

struct vnode *vfs_get(struct vfs_t *vfs, struct vnode *parent, char *name);

struct vnode *vfs_get_recursive(struct vfs_t *vfs, struct vnode *parent, char *path);

void vfs_unlink(struct vfs_t *vfs, struct vnode *node);

void *vfs_access(struct vfs_t *vfs, struct vnode *node, uint64_t offset);

uint64_t vfs_read(struct vfs_t *vfs, struct vnode *node, uint64_t offset, void *buf, uint64_t buf_len);

uint64_t vfs_write(struct vfs_t *vfs, struct vnode *node, uint64_t offset, void *buf, uint64_t buf_len);

void vfs_mkdir(struct vfs_t *vfs, struct vnode *node, char *name);

void vfs_rmdir(struct vfs_t *vfs, struct vnode *node);

// if node == NULL, flush all vfs
// do writeback on specifed node
void vfs_flush(struct vfs_t *vfs, struct vnode *node);


// release some of the buffer
void vfs_shrink(struct vfs_t *vfs);


#endif // __VFS_H_
