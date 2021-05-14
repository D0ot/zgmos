#include "vfs.h"
#include "kmem.h"
#include "kustd.h"

void vnode_init(struct vnode* node,struct vfs_backend *bkd, struct vnode *parent) {
  node->bkd = bkd;
  node->lfs_obj = NULL;
  node->name = NULL;
  node->parent = parent;
  // if it is directory
  // -1 means, the node is not updated from lower file system
  node->child_cnt = -1;

  list_init(&node->list);
  list_init(&node->children);
}

struct vfs_t *vfs_init() {
  struct vfs_t *vfs = kmalloc(sizeof(struct vfs_t));
  if(vfs) {
    vnode_init(&vfs->root, NULL, NULL);
    list_init(&vfs->bkd);
  }
  return vfs;
}


struct vnode *vfs_root(struct vfs_t *vfs) {
  return &vfs->root;
}

int64_t vfs_mount(struct vfs_t *vfs, struct vnode *node, struct vfs_backend bkd) {
  node->bkd = kmalloc(sizeof(struct vfs_backend));
  memcpy(node->bkd, &bkd, sizeof(struct vfs_backend));
  node->lfs_obj = node->bkd->root(node->bkd->lfs);
}

int64_t vfs_umount(struct vfs_t *vfs, struct vnode *node) {
}
