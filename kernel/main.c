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
#include "../driver/virtio.h"
#include "../driver//virtio_blk.h"
#include "fat32.h"
#include "vfs_fat32.h"
#include "vfs.h"
#include "process.h"

volatile static int started = 0;
pte_t* kpte;

struct context schectx;

int main(uint64_t hartid) {
  set_hartid(hartid);

  if (hartid == 0) {
    w_stvec((uint64_t)kvec_asm);
    s_sstatus(SSTATUS_SIE);
    s_sie(SIE_SSIE | SIE_STIE);

    printf_lock_init();
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


    //sbi_legacy_set_timer(r_time() + 30000000);


    
    struct disk_hal *hal = disk_hal_init();

    struct fat32_fs *fs = fat32_init(hal, 0, 0, 1);

    struct vfs_t *vfs = vfs_init(10);
    global_vfs = vfs;

    struct vfs_backend bkd = fat32bkd(fs);

    vfs_mount(vfs, vfs_root(vfs), bkd);


    struct vnode *image = vfs_get_recursive(vfs, vfs_root(vfs), "testcase/brk");
    struct task_struct *task = task_create(image, NULL);

    pte_debug_print(task->user_pte);
    
    task_set_current(task);
    swtch(&schectx, &task->ctx);

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
  while(1);
  return 0;
}


