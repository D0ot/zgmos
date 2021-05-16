#include "vfs.h"
#include "kmem.h"
#include "kustd.h"
#include "pmem.h"
#include "defs.h"
#include "utils.h"


struct vfs_block *vblk_alloc() {
  struct vfs_block *blk = kmalloc(sizeof(struct vfs_block));
  blk->buf = pmem_alloc(0);
  return blk;
}

void vblk_free(struct vfs_block *blk) {
  pmem_free(blk->buf);
  kfree(blk);
}

void vbf_bind(struct vfs_block *blk, struct vnode *node, uint32_t blkoff) {
  blk->node = node;
  blk->blkoff = blkoff;
  blk->dirty = 0;
  list_add(&blk->list_vnode, &node->bbf);
  node->bkd->read(node->bkd->lfs, node->lfs_obj, blkoff * VFS_BLOCK_SIZE, blk->buf, VFS_BLOCK_SIZE);
}
void vbf_unbind(struct vfs_block *blk) {
  vbf_flush(blk); 
  list_del(&blk->list_vnode);
}

void vbf_flush(struct vfs_block *blk) {
  if(!blk->dirty) {
    return;
  }
  struct vnode *node = blk->node;
  node->bkd->write(node->bkd->lfs, node->lfs_obj, blk->blkoff * VFS_BLOCK_SIZE, blk->buf, VFS_BLOCK_SIZE);
  blk->dirty = 0;
}

void vbf_activate(struct vfs_t *vfs, struct vfs_block *blk) {
  list_del(&blk->list_vfs);
  list_add_tail(&blk->list_vfs, &vfs->bbf_used);
}

struct vfs_block *vbf_borrow(struct vfs_t *vfs) {
  struct vfs_block *blk = NULL;
  if(vfs->bbf_free_cnt > 0) {
    // there are some blocks which are free, just use
    blk = container_of(&vfs->bbf_free.next, struct vfs_block, list_vfs);
    list_del(&blk->list_vfs);
    vfs->bbf_free_cnt--;
    list_add_tail(&blk->list_vfs, &vfs->bbf_used);
    vfs->bbf_used_cnt++;
  }else if(vfs->bbf_used_cnt < vfs->bbf_total_max) {
    blk = vblk_alloc();
    vfs->bbf_used_cnt++;
    list_add_tail(&blk->list_vfs, &vfs->bbf_used);
  }else {
    blk = container_of(&vfs->bbf_used.next, struct vfs_block, list_vfs);
    list_del(&blk->list_vfs);
    vbf_unbind(blk);
    list_add_tail(&blk->list_vfs, &vfs->bbf_used);
  }
  return blk;
}

void vbf_return(struct vfs_t *vfs, struct vfs_block *blk) {
  list_del(&blk->list_vfs);
  list_add(&blk->list_vfs, &vfs->bbf_free);
  vfs->bbf_free_cnt++;
  vfs->bbf_used_cnt--;
}


void vbf_shrink(struct vfs_t *vfs) {
  struct list_head *iter, *n;
  struct vfs_block *blk;
  list_for_each_safe(iter, n, &vfs->bbf_free) {
    blk = container_of(iter, struct vfs_block, list_vfs);
    pmem_free(blk->buf);
    list_del(iter);
  }
}

// refresh size, name
void vnode_ref(struct vnode *node) {
  node->size = node->bkd->size(node->bkd->lfs, node->lfs_obj);
  node->name = node->bkd->name(node->bkd->lfs, node->lfs_obj);
  node->type = node->bkd->type(node->bkd->lfs, node->lfs_obj);
}

void vnode_add(struct vnode *node, struct vnode *parent, void *lfs_obj) {
  node->bkd = parent->bkd;
  node->parent = parent;
  
  list_add(&node->list, &parent->children);
  
  node->lfs_obj = lfs_obj;

  // if it is directory
  // -1 means, the node is not updated from lower file system
  node->child_cnt = -1;

  vnode_ref(node);
  list_init(&node->children);
  list_init(&node->bbf);
}

// check if a block is buffered
struct vfs_block *vbf_chkbufed(struct vfs_t *vfs, struct vnode *node, uint32_t blkoff) {
  struct list_head *iter;
  struct vfs_block *blk;
  list_for_each(iter, &node->bbf) {
    blk = container_of(iter, struct vfs_block, list_vnode);
    if(blk->blkoff == blkoff) {
      return blk;
    }
  }
  return NULL;
}

void vfs_buffer(struct vfs_t *vfs, struct vnode *node, uint32_t blkoff) {
  struct vfs_block *blk = vbf_borrow(vfs);
  vbf_bind(blk, node, blkoff);
}

void vfs_unbuffer(struct vfs_t *vfs, struct vfs_block *blk) {
  vbf_unbind(blk);
  vbf_return(vfs, blk);
}

void vfs_unbuffer_all(struct vfs_t *vfs, struct vnode *node) {
  struct list_head *pos, *n;
  list_for_each_safe(pos, n, &node->bbf) {
    struct vfs_block *blk = container_of(pos, struct vfs_block, list_vnode);
    vfs_unbuffer(vfs, blk);
  }
}

struct vfs_t *vfs_init(uint32_t buffer_max) {

  struct vfs_t *vfs = kmalloc(sizeof(struct vfs_t));
  if(vfs) {
    vfs->root.lfs_obj = NULL;
    vfs->root.name = NULL;
    // if it is directory
    // -1 means, the node is not updated from lower file system
    vfs->root.child_cnt = -1;
    vfs->root.type = VNODE_UNDEF;

    vfs->root.parent = &vfs->root;

    list_init(&vfs->root.list);
    list_init(&vfs->root.children);

    list_init(&vfs->bkd);

    list_init(&vfs->bbf_free);
    vfs->bbf_free_cnt = 0;

    list_init(&vfs->bbf_used);
    vfs->bbf_used_cnt = 0;

    vfs->bbf_total_max = buffer_max;
  }
  return vfs;
}


bool vfs_is_spaned(struct vnode *node) {
  return node->child_cnt >= 0;
}

// span the directory and do a search
struct vnode *vfs_span_search(struct vnode *node, char *name) {
  void *obj = kmalloc(node->bkd->lfs_obj_size);
  void *iter = node->bkd->iterate(node->bkd->lfs, node->lfs_obj, NULL, obj);
  struct vnode *ret = NULL;
  node->child_cnt = 0;
  while(iter != NULL) {
    // if iter is not NULL, the obj is valid
    struct vnode *child = kmalloc(sizeof(struct vnode));
    vnode_add(child, node, obj);
    if(name && strcmp(name, child->name) == 0) {
      ret = child;
    }
    obj = kmalloc(node->bkd->lfs_obj_size);
    iter = node->bkd->iterate(node->bkd->lfs, node->lfs_obj, iter, obj);
    node->child_cnt++;
  }
  kfree(obj);
  return ret;
}


struct vnode *vfs_search(struct vnode *node, char *name) {
  struct list_head *iter;
  struct vnode *child;
  list_for_each(iter, &node->children) {
    child = container_of(iter, struct vnode, list);
    if(strcmp(child->name, name) == 0) {
      return child;
    }
  }
  return NULL;
}

// can only be called on a directory
// TODO, use a software stack to fold recursively
// currently use a hardware stack, stackoverflow can occurs
// TODO, how to fold a mount point
void vfs_fold(struct vfs_t *vfs, struct vnode *node) {
  struct list_head *iter, *n;
  struct vnode *child;
  list_for_each_safe(iter, n, &node->children) {
    child = container_of(iter, struct vnode, list);
    if(child->type == VNODE_DIR && vfs_is_spaned(node)) {
      vfs_fold(vfs, child);
    }

    if(child->type == VNODE_FILE) {
      vfs_unbuffer_all(vfs, child);
    }
    kfree(child->lfs_obj);
    list_del(iter);
    kfree(child);
  }
  node->child_cnt = -1;
}

struct vnode *vfs_root(struct vfs_t *vfs) {
  return &vfs->root;
}

int64_t vfs_mount(struct vfs_t *vfs, struct vnode *node, struct vfs_backend bkd) {
  if(!node) {
    node = &vfs->root;
  }
  node->bkd = kmalloc(sizeof(struct vfs_backend));
  memcpy(node->bkd, &bkd, sizeof(struct vfs_backend));

  node->lfs_obj = kmalloc(node->bkd->lfs_obj_size);
  node->bkd->root(node->bkd->lfs, node->lfs_obj);
  node->type = VNODE_MP;
  list_add(&node->bkd->list, &vfs->bkd);

  node->type = VNODE_MP;
  return 0;
}

int64_t vfs_umount(struct vfs_t *vfs, struct vnode *node) {
  while(1);
}


void vfs_create(struct vfs_t *vfs, struct vnode *parent, char *name) {
  while(1);
}

struct vnode *vfs_get(struct vfs_t *vfs, struct vnode *parent, char *name) {
  if(parent->type != VNODE_DIR && parent->type != VNODE_MP) {
    return NULL;
  }

  static char *dot = ".";
  static char *dotdot = "..";

  if(strcmp(dot, name) == 0){
    return parent;
  }

  if(strcmp(dotdot, name) == 0){
    return parent->parent;
  }

  if(vfs_is_spaned(parent)) {
    return vfs_search(parent, name);
  }else {
    return vfs_span_search(parent, name);
  }
}

struct vnode *vfs_get_recursive(struct vfs_t *vfs, struct vnode *parent, char *path) {
  

  struct vnode *p = parent;
  uint64_t s = 0;
  uint64_t e = 0;
  while(p && path[s]) {
    while(path[e] && path[e] != '/') {
      e++;
    }

    if(path[e] == 0) {
      return vfs_get(vfs, p, path + s);
    }
    
    if(path[e] == '/') {
      char tmp = path[e];
      path[e] = 0;
      p = vfs_get(vfs, p, path + s);
      path[e] = tmp;
    }
    s = ++e;

  }
  
  if(p->type == VNODE_DIR || p->type == VNODE_MP) {
    return p;
  }
  return NULL;
}

void vfs_unlink(struct vfs_t *vfs, struct vnode *node) {
  //TODO
  while(1);
}

void *vfs_access(struct vfs_t *vfs, struct vnode *node, uint64_t blkoff) {
  struct vfs_block *blk = vbf_chkbufed(vfs, node, blkoff);
  if(blk) {
    vbf_activate(vfs, blk);
  }else {
    blk = vbf_borrow(vfs);
    vbf_bind(blk, node, blkoff);
  }
  return blk->buf;
}

uint64_t vfs_read(struct vfs_t *vfs, struct vnode *node, uint64_t offset, void *buf, uint64_t buf_len) {
  if(offset >= node->size) {
    return 0;
  }
  // read can not exceed the file size
  buf_len = min(buf_len, node->size - offset);

  uint64_t blkoff_init = offset / VFS_BLOCK_SIZE;
  uint64_t byteoff = offset % VFS_BLOCK_SIZE;
  uint64_t byte_cnt = 0;
  uint64_t len;
  void *dat;

  for(uint64_t blkoff = blkoff_init; byte_cnt != buf_len; ++blkoff) {
    dat = vfs_access(vfs, node, blkoff);
    len = min(VFS_BLOCK_SIZE - byteoff, buf_len - byte_cnt);
    memcpy(buf +byte_cnt , dat + byteoff, len);
    byte_cnt += len;
    byteoff = 0;
  }
  return byte_cnt;
}

void vfs_shrink(struct vfs_t *vfs) {
  vbf_shrink(vfs);
}
