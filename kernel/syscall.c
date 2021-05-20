#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "syscall.h"
#include "process.h"
#include "panic.h"
#include "klog.h"

int syscall() {
  LOG_INFO("syscall arg0: %x, arg1: %x, arg3 %x", 
      syscall_arg(0),
      syscall_arg(1),
      syscall_arg(2));
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

uint64_t syscall_arg(int i) {
  syscall_arg_check(i);
  struct task_struct *task = task_get_current();
  return (&task->tfp->a0)[i];
}


uint64_t syscall_num() {
  return task_get_current()->tfp->a7;
}


