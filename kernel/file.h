#ifndef __FILE_H_
#define __FILE_H_

#include <stdint.h>
#include "vfs.h"

struct files_struct {
  struct vnode **nodes;
  uint64_t *seek;
  uint64_t size;
};


struct files_struct *files_struct_create();
struct files_struct *files_struct_clone(struct files_struct *files);
int files_struct_alloc(struct files_struct *files, struct vnode *parent, const char *path);
bool files_struct_check(struct files_struct *files, int fd);
void files_struct_free(struct files_struct *files, int fd);
struct vnode *files_struct_get(struct files_struct *files, int fd);
void files_struct_destroy(struct files_struct *files);

int files_struct_dup(struct files_struct *files, int old_fd, int new_fd);

int files_struct_read(struct files_struct *files, int fd, void *buf, uint64_t buf_len);
int files_struct_write(struct files_struct *files, int fd, void *buf, uint64_t buf_len);


#endif // __FILE_H_
