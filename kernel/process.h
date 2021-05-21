#ifndef __PROCESS_H_
#define __PROCESS_H_

#include <stdint.h>
#include "pg.h"
#include "list.h"
#include "defs.h"
#include "vfs.h"
#include "file.h"


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

void swtch(struct context *oldctx, struct context *newctx);

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
  uint64_t hartid;
  uint64_t utrap_entry;
};


static const uint64_t TASK_UNUSED = 0;
static const uint64_t TASK_RUNNABLE = 1;
static const uint64_t TASK_RUNNING = 3;
static const uint64_t TASK_ZOMBIE = 4;

// last page is not used, and the last page is also a guard page
#define PROC_VA_END             (0xfffff000)

// trampoline code page
#define PROC_VA_TRAMPOLINE      (PROC_VA_END - PAGE_SIZE)

// guard  page
#define PROC_VA_GUARD           (PROC_VA_TRAMPOLINE - PAGE_SIZE)

// a process has a 8KiB kernel stack, this is the start address
#define PROC_VA_KSTACK          (PROC_VA_GUARD - PAGE_SIZE * 2)
// the addres put into the sp register
#define PROC_VA_KSTACK_SP       (PROC_VA_GUARD - 32)

// guard page 
#define PROC_VA_GUARD2          (PROC_VA_KSTACK - PAGE_SIZE)

// trapframe
#define PROC_VA_TRAPFRAME       (PROC_VA_GUARD2 - PAGE_SIZE)

// the page address of the last page which is user avaliable
#define PROC_VA_USER_MAX        (PROC_VA_TRAPFRAME - PAGE_SIZE)

// the user stack
#define PROC_VA_USTACK          (PROC_VA_USER_MAX - PAGE_SIZE)
#define PROC_VA_USTACK_SP       (PROC_VA_USER_MAX - 32)


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

  // user stack
  void *ustack_pa;

  // entry address
  void *entry;

  // trapframe pointer,
  // when interrupt occurs in U-Mode,
  // all regs are saved into this location,
  // and it is on kstack
  struct trapframe *tfp;

  struct context ctx;

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

  // process id
  int pid;

  // files
  struct files_struct *files;
};

// initlize the task management
void task_init();

struct task_struct *task_create(struct vnode *image, struct task_struct *parent);

// it is called when process exit
void task_exit(struct task_struct *task);

// it is called when parent process called wait
void task_clean(struct task_struct *task);

void task_create_ret();

// alloc a page for the process
// caller must ensure the va is 4K aligned
// return new pages pa
struct task_page *task_add_page(struct task_struct *task, void *va, pte_t flags);
// free a page for the proces
// caller must ensure the va is 4K aligned
bool task_remove_page(struct task_struct *task, void *va);

// called when the process is exiting, normally or abnormal
// or when the task_create failed
void task_remove_all_page(struct task_struct *task);

void task_add(struct task_struct *task);
void task_del(struct task_struct *task);


struct task_struct *task_get_current();
void task_set_current(struct task_struct *task);

#endif // __PROCESS_H_
