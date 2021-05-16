#ifndef __PROCESS_H_
#define __PROCESS_H_

#include <stdint.h>
#include "pg.h"
#include "list.h"
#include "defs.h"
#include "vfs.h"

struct context {

  // stack pointer
  uint64_t sp;

  // global pointer
  uint64_t gp;

  // place holder
  uint64_t tp;

  // save regs, callee saved
  uint64_t s0;
  uint64_t s1;
  uint64_t s2;
  uint64_t s3;
  uint64_t s4;
  uint64_t s5;
  uint64_t s6;
  uint64_t s7;
  uint64_t s8;
  uint64_t s9;
  uint64_t s10;
  uint64_t s11;

  // return address
  uint64_t ra;
};

struct trapframe {
  uint64_t ra;
  uint64_t sp;
  uint64_t gp;
  uint64_t tp;
  uint64_t t0;
  uint64_t t1;
  uint64_t t2;
  uint64_t s0; //fp
  uint64_t s1;
  uint64_t a0;
  uint64_t a1;
  uint64_t a2;
  uint64_t a3;
  uint64_t a4;
  uint64_t a5;
  uint64_t a6;
  uint64_t a7;
  uint64_t s2;
  uint64_t s3;
  uint64_t s4;
  uint64_t s5;
  uint64_t s6;
  uint64_t s7;
  uint64_t s8;
  uint64_t s9;
  uint64_t s10;
  uint64_t s11;
  uint64_t t3;
  uint64_t t4;
  uint64_t t5;
  uint64_t t6;

  uint64_t epc;
  uint64_t kernel_satp;
  uint64_t kernel_sp;
};


static const uint64_t TASK_UNUSED = 0;
static const uint64_t TASK_RUNNABLE = 1;
static const uint64_t TASK_RUNNING = 3;
static const uint64_t TASK_ZOMBIE = 4;

// last page is not used, and the last page is also a guard page
#define PROC_VA_END             (0xfffff000)

// trampoline page
#define PROC_VA_TRAMPOLINE      (PROC_VA_END - PAGE_SIZE)

// guard  page
#define PROC_VA_GUARD           (PROC_VA_TRAMPOLINE - PAGE_SIZE)

// a process has a 8KiB kernel stack, this is the start address
#define PROC_VA_KSTACK          (PROC_VA_GUARD - PAGE_SIZE * 2)

// the addres put into the sp register
#define PROC_VA_KSTACK_SP       (PROC_VA_GUARD - 32)

// guard page
#define PROC_VA_GUARD2          (PROC_VA_KSTACK - PAGE_SIZE)

// the page address of the last page which is user avaliable
#define PROC_VA_USER_MAX        (PROC_VA_GUARD2 - PAGE_SIZE)

// it is 4K in size;
struct task_page{
  struct list_head list;
  void *pa;
  void *va;
};


struct task_struct{
  // used by scheduler
  // Used By Scheduler
  struct list_head ubs;

  // used by parent proc
  // Used By Parent = ubp
  struct list_head ubp;

  // task page 
  struct list_head tpg;

  // task state
  uint64_t state;

  // parent process
  struct task_struct *parent;

  // child process
  struct list_head children;

  // task kernel stack pa, the va is fixed
  void *kstack_pa;

  // entry address
  void *entry;

  // trapframe pointer,
  // when interrupt occurs in U-Mode,
  // all regs are saved into this location,
  // and it is on kstack
  struct trapframe *tfp;

  // user page table
  pte_t *user_pte;

  // used pages
  struct list_head pages;

  // process name
  char *name;
  
  // process image vnode
  struct vnode *image;

  // exit code 
  int exit_status;
};

struct task_struct *task_create(struct vnode *image, struct task_struct *parent);
void task_destroy(struct task_struct *task);

// alloc a page for the process
// caller must ensure the va is 4K aligned
// return new pages pa
struct task_page *task_add_page(struct task_struct *task, void *va, pte_t flags);
// free a pge for the proces
// caller must ensure the va is 4K aligned
bool task_remove_page(struct task_struct *task, void *va);

// called when the process is exiting, normally or abnormal
// or when the task_create failed
void task_remove_all_page(struct task_struct *task);

void tasK_add(struct task_struct *task);
void task_del(struct task_struct *task);

#endif // __PROCESS_H_
