#include "defs.h"
#include "extdef.h"
#include "pg.h"
#include "kvm.h"
#include "vmem.h"
#include "riscv.h"


void kvm_init(pte_t *kp) {
  // map sbi QEMU
  vmem_map(kp, SBI_START, SBI_START, PTE_XWR_SET, VMEM_PAGE_2M);

  vmem_range_map(kp, TEXT_START, TEXT_START, PTE_XR_SET, RODATA_END - TEXT_START);

  vmem_range_map(kp, DATA_START, DATA_START, PTE_XWR_SET, RAM_END - DATA_START);
  
}
void kvm_install(pte_t *kp) {
  w_satp(RISCV_MAKE_SATP(kp));
  sfence_vma();
}
