#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cpu.h"
#include "file.h"
#include "syscall.h"
#include "process.h"
#include "panic.h"
#include "klog.h"
#include "utils.h"
#include "uvec.h"
#include "scheduler.h"
#include "pte.h"
#include "vfs.h"
#include "filesystem.h"

int syscall() {
  /*LOG_INFO("syscall arg0: %x, arg1: %x, arg3 %x", 
      syscall_arg(0),
      syscall_arg(1),
      syscall_arg(2));
  return -1;*/
  

  switch(syscall_num()) {
    case SYS_getpid:
      return syscall_getpid();
    case SYS_getppid:
      return syscall_getppid();
    case SYS_sched_yield:
      return syscall_sched_yield();
    case SYS_exit:
      return syscall_exit();
    case SYS_read:
      return syscall_read();
    case SYS_write:
      return syscall_write();
    case SYS_openat:
      return syscall_openat();
    case SYS_chdir:
      return syscall_chdir();
    default:
      return -1;
  }


  return -1;
};

void syscall_arg_check(int i) {
  if(i < 0 || i > 6) {
    LOG_ERROR("syscall arg out of range");
    KERNEL_PANIC();
  }
}


#define SYSCALL_GET_ARG_FUNC_DEF(type, suffix) \
  type syscall_arg_##suffix(int i) { \
    syscall_arg_check(i); \
    void *ptr = &(task_get_current()->tfp->a0) + i; \
    return *(type*)(ptr); \
  }

SYSCALL_GET_ARG_FUNC_DEF(void*, ptr);
SYSCALL_GET_ARG_FUNC_DEF(int, int);
SYSCALL_GET_ARG_FUNC_DEF(int ,fd);
SYSCALL_GET_ARG_FUNC_DEF(size_t, size);

uint64_t syscall_arg(int i) {
  syscall_arg_check(i);
  struct task_struct *task = task_get_current();
  return (&task->tfp->a0)[i];
}


uint64_t syscall_num() {
  return task_get_current()->tfp->a7;
}


int syscall_getpid() {
  return task_get_current()->pid;
}

int syscall_exit() {
  struct task_struct *task = task_get_current();
  task->exit_status = task->tfp->a0;
  task->state = TASK_ZOMBIE;
  scheduler_exit(task);
  return 0;
}

int syscall_chdir() {
  struct task_struct *task = task_get_current();
  char *fn = syscall_arg_ptr(0);
  char *pa = pte_walk(task->user_pte, fn, NULL, NULL) + (ALL_ONE_MASK(12) & (uint64_t)fn);
  struct vnode *node;
  if(pa[0] == '/') {
    node = vfs_open(fs.vfs, NULL, pa + 1);
  }else {
    node = vfs_open(fs.vfs, task->cwd, pa);
  }

  if(node && (node->type == VNODE_DIR || node->type == VNODE_MP)) {
    task->cwd = node;
    return 0;
  }else {
    return -1;
  }
}

#define AT_FDCWD (-100)
int syscall_openat() {
  struct task_struct *task = task_get_current();
  char *fn = syscall_arg_ptr(1);
  char *pa = pte_walk(task->user_pte, fn, NULL, NULL) + (ALL_ONE_MASK(12) & (uint64_t)fn);
  int dirfd = syscall_arg_fd(0);
  int fd = -1;
  if(pa[0] == '/') {
    fd = files_struct_alloc(task->files, NULL, pa + 1);
  } else {
    if(dirfd == AT_FDCWD) {
      fd = files_struct_alloc(task->files, task->cwd, pa); 
    } else {
      if(files_struct_check(task->files, dirfd)) {
        struct vnode *node = files_struct_get(task->files, dirfd);
        if(node->type != VNODE_DIR && node->type != VNODE_MP) {
          fd = 1;
        }else {
          fd = files_struct_alloc(task->files, node, pa);
        }
      } else {
        fd = -1;
      }
    }
  }
  return fd;
}

int syscall_read() {
  int fd = syscall_arg_fd(0);
  struct task_struct *task = task_get_current();
  if(!files_struct_check(task->files, fd)) {
    return -1;
  }
  void *va = syscall_arg_ptr(1);
  size_t size = syscall_arg_size(2);

  void *pa = pte_walk(task->user_pte, va, NULL, NULL) + (ALL_ONE_MASK(12) & (uint64_t)va);
  return files_struct_read(task->files, fd, pa, size);
}

int syscall_write() {
  int fd = syscall_arg_fd(0);
  struct task_struct *task = task_get_current();
  if(!files_struct_check(task->files, fd)) {
    return -1;
  }
  void *va = syscall_arg_ptr(1);
  size_t size = syscall_arg_size(2);

  void *pa = pte_walk(task->user_pte, va, NULL, NULL) + (ALL_ONE_MASK(12) & (uint64_t)va);
  return files_struct_write(task->files, fd, pa, size);
}

int syscall_getppid() {
  return task_get_current()->parent ? task_get_current()->parent->pid : 0;
}


int syscall_sched_yield() {
  intr_off();
  yield();
  return 0;
}
