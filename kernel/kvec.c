#include "stddef.h"
#include "stdbool.h"
#include "stdint.h"
#include "earlylog.h"
#include "riscv.h"
#include "panic.h"
#include "sbi.h"
#include "cpu.h"
#include "klog.h"
#include "defs.h"
#include "uvec.h"

void kvec() {
  uint64_t scause = r_scause();
  uint64_t stval = r_stval();
  uint64_t sstatus = r_sstatus();
  uint64_t spp = sstatus & SSTATUS_SPP;
  uint64_t sepc = r_sepc();

  if(spp == 0) {
    // from user mode... panic
    KERNEL_PANIC();
  }
  

  if(scause == SCAUSE_SUPV_TIMER) {
    sbi_legacy_set_timer(r_time() + TIMER_DIFF);
    LOG_INFO("timer!");
  } else if(scause == SCAUSE_ECALL_USER) {
    LOG_INFO("ecall from user panic");
    KERNEL_PANIC();
    yield();
  } else {
    // unhandled interrupt or exception
    LOG_INFO("unhandled interrupt, sscause: %x, sstval: %x\n", scause, stval);
    KERNEL_PANIC();
  }
  w_sepc(sepc);
  w_sstatus(sstatus);
}
