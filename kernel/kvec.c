#include "stddef.h"
#include "stdbool.h"
#include "stdint.h"
#include "earlylog.h"
#include "riscv.h"
#include "panic.h"
#include "sbi.h"

void kvec() {
  uint64_t scause = r_scause();
  uint64_t stval = r_stval();
  uint64_t spp = r_sstatus() & SSTATUS_SPP;

  if(spp == 0) {
    // from user mode... panic
    KERNEL_PANIC();
  }

  if(scause == SCAUSE_SUPV_TIMER) {
    sbi_legacy_set_timer(r_time() + 30000000);
    printf("timer!\n");
  } else {
    // unhandled interrupt or exception
    printf("unhandled interrupt, sscause: %l, sstval: %l\n", scause, stval);
    KERNEL_PANIC();
  }
}
