#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "extdef.h"
#include "sbi.h"
#include "static_check.h"
#include "earlylog.h"
#include "bootinfo.h"
#include "pmem.h"
#include "kmem.h"
#include "pte.h"
#include "kvm.h"
#include "riscv.h"
#include "fat32.h"
#include "vfs_fat32.h"
#include "vfs.h"
#include "process.h"
#include "klog.h"
#include "cpu.h"
#include "scheduler.h"
#include "filesystem.h"

// hal
#include "../hal/disk_hal.h"
#include "../hal/uart_hal.h"

volatile static int started = 0;
pte_t* kpte;


int main(uint64_t hartid) {
  set_hartid(hartid);

  if (hartid == 0) {
    // initialize the cpu structs, must before any lock operations
    cpu_struct_init();

    w_stvec((uint64_t)kvec_asm);
    s_sstatus(SSTATUS_SIE);
    s_sie(SIE_SSIE | SIE_STIE);
    uart_hal_init();
    klog_init();

    printf("hart %d enter main()...\n", hartid);
    // Print LOGO.
    print_bootinfo(hartid);
    // initialize the physical memory allocator.
    pmem_init(KERNEL_END, RAM_END);
    // initialize slab allocator 
    // kmalloc() and kfree() are avaliable
    kmem_init();

    kpte = pte_create();
    kvm_init(kpte);
    kvm_install(kpte);

    fs_init(); 
    task_init();
    struct vfs_t *vfs = fs.vfs;

    struct vnode *image = vfs_get_recursive(vfs, NULL, "testcase/test1");
    struct task_struct *task = task_create(image, NULL);

    struct vnode *image2 = vfs_get_recursive(vfs, NULL, "testcase/write");
    struct task_struct *task2 = task_create(image2, NULL);


    pte_debug_print(task->user_pte);
    
    scheduler_init();
    // scheduler_add(task);
    scheduler_add(task2);
    scheduler_run();

    while(1);

    printf("hart %d init done\n", hartid);
    // muticore start
    unsigned long mask = 1 << 1;
    sbi_legacy_send_ipi(&mask);
    __sync_synchronize();
    started = 1;
  } else {
    // hart 1
    while (started == 0)
      ;
    __sync_synchronize();
    w_stvec((uint64_t)kvec_asm);
    s_sstatus(SSTATUS_SIE);
    s_sie(SIE_SSIE | SIE_STIE);
    printf("hart %d enter main()...\n", hartid);
    kvm_install(kpte);
    printf("hart 1 init done\n");
  }

  
  while(1) {
    LOG_INFO("hello from hart, id = %l", get_hartid());
  }
  return 0;
}


