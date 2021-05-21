#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cpu.h"
#include "syscall.h"
#include "process.h"
#include "panic.h"
#include "klog.h"
#include "uvec.h"

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
}

int syscall_getppid() {
  return task_get_current()->parent ? task_get_current()->parent->pid : 0;
}


int syscall_sched_yield() {
  intr_off();
  yield();
  return 0;
}
