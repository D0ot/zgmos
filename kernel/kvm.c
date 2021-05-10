#include "defs.h"
#include "extdef.h"
#include "pg.h"
#include "kvm.h"
#include "vmem.h"
#include "riscv.h"
#include "../driver/virtio.h"


void kvm_init(pte_t *kp) {
  // map sbi QEMU
  vmem_map(kp, SBI_START, SBI_START, PTE_XWR_SET, VMEM_PAGE_2M);

  // all kernel text rodata data bss
  vmem_range_map(kp, TEXT_START, TEXT_START, PTE_XWR_SET, BSS_END - TEXT_START);

  // map rest ram
  vmem_range_map(kp, BSS_END, BSS_END, PTE_RW_SET, RAM_END - BSS_END);


#ifdef QEMU
  // map virtio_blk_mmio
  vmem_map(kp, VIRTIO_BLK_MMIO_BASE, VIRTIO_BLK_MMIO_BASE, PTE_RW_SET, VMEM_PAGE_4K);
#endif

  
}
void kvm_install(pte_t *kp) {
  w_satp(RISCV_MAKE_SATP(kp));
  sfence_vma();
}
