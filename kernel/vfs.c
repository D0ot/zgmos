#include "vfs.h"
#include "kmem.h"
#include "kustd.h"


// refresh size, name
void vnode_ref(struct vnode *node) {
  node->size = node->bkd->size(node->bkd->lfs, node->lfs_obj);
  node->name = node->bkd->name(node->bkd->lfs, node->lfs_obj);
}

void vnode_add(struct vnode *node, struct vnode *parent, void *lfs_obj) {
  node->bkd = parent->bkd;
  node->parent = parent;
  
  list_add(&node->list, &parent->list);
  
  node->lfs_obj = lfs_obj;
  vnode_ref(node);

  // if it is directory
  // -1 means, the node is not updated from lower file system
  node->child_cnt = -1;
  node->type = VNODE_UNDEF;

  list_init(&node->children);
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

    list_init(&vfs->root.list);
    list_init(&vfs->root.children);

    list_init(&vfs->bkd);

    list_init(&vfs->buffer);
    vfs->buffer_count = 0;
    vfs->buffer_max = buffer_max;
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
    child = container_of(iter, struct vnode, children);
    if(strcmp(child->name, name) == 0) {
      return child;
    }
  }
  return NULL;
}

struct vnode *vfs_root(struct vfs_t *vfs) {
  return &vfs->root;
}

int64_t vfs_mount(struct vfs_t *vfs, struct vnode *node, struct vfs_backend bkd) {
  node->bkd = kmalloc(sizeof(struct vfs_backend));
  memcpy(node->bkd, &bkd, sizeof(struct vfs_backend));
  node->lfs_obj = node->bkd->root(node->bkd->lfs);
  node->type = VNODE_MP;
  list_add(&node->bkd->list, &vfs->bkd);
}

int64_t vfs_umount(struct vfs_t *vfs, struct vnode *node) {
  while(1);
}


void vfs_create(struct vfs_t *vfs, struct vnode *parent, char *name) {
  while(1);
}

struct vnode *vfs_get(struct vfs_t *vfs, struct vnode *parent, char *name) {
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
  while(p) {
    while(path[e] && path[e] != '\\');

    if(path[e] == 0) {
      return vfs_get(vfs, p, path + s);
    }
    
    if(path[e] == '\\') {
      char tmp = path[e];
      path[e] = 0;
      p = vfs_get(vfs, p, 
    }

  }
  return NULL;
}

