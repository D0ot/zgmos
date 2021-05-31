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
    // here, we get 2 page
    files->nodes = pmem_alloc(1);
    if(files->nodes) {
      files->seek = (void*)files->nodes + PAGE_SIZE;

      files->size = (PAGE_SIZE * POWER_OF_2(0)) / sizeof(struct vnode*);
      for(int i = 0; i < files->size; ++i) {
        files->nodes[i] = NULL;
        files->seek[i] = 0;
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

struct files_struct *files_struct_clone(struct files_struct *files) {
  struct files_struct *newfiles = kmalloc(sizeof(struct files_struct));
  newfiles->nodes = pmem_alloc(1);
  newfiles->seek = (void*)files->nodes + PAGE_SIZE;
  newfiles->size = (PAGE_SIZE * POWER_OF_2(0)) / sizeof(struct vnode*);
  for(int i = 0; i < files->size; ++i) {
    newfiles->nodes[i] = files->nodes[i];
    newfiles->seek[i] = files->seek[i];
  }
  return newfiles;
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
    return -1;
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
  files->seek[fd] = 0;
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
  // we get 2 page at once, so just free once
  pmem_free(files->nodes);
  kfree(files);
}


int files_struct_dup(struct files_struct *files, int old_fd, int new_fd) {
  if(files_struct_check(files, old_fd)) {

    if(new_fd == -1) {
      for(int i = 0; i < files->size; ++i) {
        if(files->nodes[i] == NULL) {
          files->nodes[i] = vfs_reopen(fs.vfs, files->nodes[old_fd]);
          return i;
        }
      }
      return -1;
    }else {
      if(files->nodes[new_fd]) {
        return -1;
      } else {
        files->nodes[new_fd] = vfs_reopen(fs.vfs, files->nodes[old_fd]);
        return new_fd;
      }
    }
  }
  return -1;
}

int files_struct_read(struct files_struct *files, int fd, void *buf, uint64_t buf_len) {
  int ret = vfs_read(fs.vfs, files->nodes[fd], files->seek[fd], buf, buf_len);
  if(ret > 0) {
    files->seek[fd] += ret;
  }
  return ret;
}

int files_struct_write(struct files_struct *files, int fd, void *buf, uint64_t buf_len) {
  int ret = vfs_write(fs.vfs, files->nodes[fd], files->seek[fd], buf, buf_len);
  if(ret > 0) {
    files->seek[fd] += ret;
  }
  return ret;
}
