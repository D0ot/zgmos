#include "file.h"
#include "kmem.h"
#include "defs.h"
#include "kmem.h"
#include "pmem.h"
#include "utils.h"
#include "filesystem.h"

struct files_struct *files_struct_create() {
  struct files_struct *files = kmalloc(sizeof(struct files_struct));
  if(files) {
    files->nodes = pmem_alloc(0);
    if(files->nodes) {
      files->size = (PAGE_SIZE * POWER_OF_2(0)) / sizeof(struct vnode*);
      for(int i = 0; i < files->size; ++i) {
        files->nodes[i] = NULL;
      }
    } else {
      kfree(files);
      files = NULL;
    }
  }
  // open three default fd
  files_struct_alloc(files, NULL, "dev/stdin");
  files_struct_alloc(files, NULL, "dev/stdout");
  files_struct_alloc(files, NULL, "dev/stderr");
  return files;
}

int files_struct_alloc(struct files_struct *files, struct vnode *parent, const char *path) {
  int ret = -1;
  for(int i = 0; i < files->size; ++i) {
    if(files->nodes[i] == NULL) {
      ret = i;
      break;
    }
  }
  if(ret == -1) {
    return ret;
  }

  struct vnode *node = vfs_open(fs.vfs, parent, path);
  if(node) {
    files->nodes[ret] = node;
  }else {
    ret = -1;
  }
  return ret;
}

bool files_struct_check(struct files_struct *files, int fd) {
  return (fd >= 0) && (fd < files->size) && (files->nodes[fd]);
}

void files_struct_free(struct files_struct *files, int fd) {
  vfs_close(fs.vfs, files->nodes[fd]);
  files->nodes[fd] = NULL;
}

struct vnode *files_struct_get(struct files_struct *files, int fd) {
  return files->nodes[fd];
}

void files_struct_destroy(struct files_struct *files) {
  for(int i = 0; i < files->size; ++i) {
    if(files->nodes[i]) {
      vfs_close(fs.vfs, files->nodes[i]);
    }
  }
  pmem_free(files->nodes);
  kfree(files);
}


