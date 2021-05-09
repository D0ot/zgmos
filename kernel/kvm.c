#include "defs.h"
#include "extdef.h"
#include "pg.h"
#include "kvm.h"
#include "vmem.h"
#include "riscv.h"
#include "../driver/virtio_blk_mmio.h"


void kvm_init(pte_t *kp) {
  // map sbi QEMU
  vmem_map(kp, SBI_START, SBI_START, PTE_XWR_SET, VMEM_PAGE_2M);

  // map text and rodata
  vmem_range_map(kp, TEXT_START, TEXT_START, PTE_XR_SET, RODATA_END - TEXT_START);

  // map data and bss
  vmem_range_map(kp, DATA_START, DATA_START, PTE_XWR_SET, RAM_END - DATA_START);

#ifdef QEMU
  // map virtio_blk_mmio
  vmem_map(kp, VIRTIO_BLK_MMIO_BASE, VIRTIO_BLK_MMIO_BASE, PTE_RW_SET, VMEM_PAGE_4K);
#endif

  
}
void kvm_install(pte_t *kp) {
  w_satp(RISCV_MAKE_SATP(kp));
  sfence_vma();
}
