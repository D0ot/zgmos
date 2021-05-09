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
#include "../driver/virtio.h"


int main(uint64_t hartid) {
  set_hartid(hartid);
  // Print LOGO.
  print_bootinfo(hartid);
  // initialize the physical memory allocator.
  pmem_init(KERNEL_END, RAM_END);
  // initialize slab allocator 
  // kmalloc() and kfree() are avaliable
  kmem_init();

  pte_t *kpte = vmem_create();
  kvm_init(kpte);
  kvm_install(kpte);

  w_stvec((uint64_t)kvec_asm);
  s_sstatus(SSTATUS_SIE);
  s_sie(SIE_SSIE | SIE_STIE);
  sbi_legacy_set_timer(r_time() + 30000000);


  virtio_dev_init(VIRTIO_BLK_MMIO_BASE);
  while(1);
  return 0;
}


