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
#include "vmem.h"
#include "kvm.h"
#include "riscv.h"

volatile static int started = 0;

int main(uint64_t hartid) {
  set_hartid(hartid);

  pte_t* kpte;

  if (hartid == 0) {
    printf_lock_init();
    printf("hart %d enter main()...\n", hartid);
    // Print LOGO.
    print_bootinfo(hartid);
    // initialize the physical memory allocator.
    pmem_init(KERNEL_END, RAM_END);
    // initialize slab allocator 
    // kmalloc() and kfree() are avaliable
    kmem_init();

    kpte = vmem_create();
    kvm_init(kpte);
    kvm_install(kpte);


    w_stvec((uint64_t)kvec_asm);
    s_sstatus(SSTATUS_SIE);
    s_sie(SIE_SSIE | SIE_STIE);
    sbi_legacy_set_timer(r_time() + 30000000);

    printf("hart %d init done\n", hartid);
    // muticore start
    unsigned long mask = 1 << 1;
    sbi_send_ipi(&mask);
    __sync_synchronize();
    started = 1;
  } else {
    // hart 1
    while (started == 0)
      ;
    __sync_synchronize();
    printf("hart %d enter main()...\n", hartid);
    // kvm_install(kpte);
    printf("hart 1 init done\n");
  }
  
  while(1);
  return 0;
}


