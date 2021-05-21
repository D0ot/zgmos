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
