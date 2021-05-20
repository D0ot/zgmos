#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "riscv.h"
#include "process.h"
#include "extdef.h"
#include "uvec.h"
#include "earlylog.h"

typedef void (*uvec_ret_func)(uint64_t, uint64_t);

void utrap_entry() {
  uint64_t scause = r_scause();

  w_stvec((uint64_t)kvec_asm);
  

  if(scause == SCAUSE_ECALL_USER) {
    printf("ecall from user\n");
  } else if(scause == SCAUSE_SUPV_TIMER) {
    printf("timer from user\n");
  } else {
    printf("unknown int from user, scause: %l\n", scause);
  }

  utrap_ret();
}

void utrap_ret() {
  // clear SPP
  c_sstatus(SSTATUS_SPP);

  w_stvec(PROC_VA_TRAMPOLINE + ((uint64_t)uvec_enter_asm - (uint64_t)UVEC_START));


  struct task_struct *task = task_get_current();

  w_sepc(task->tfp->epc);
  
  uint64_t satp = RISCV_MAKE_SATP(task->user_pte);

  void *fn = (void*)((uint64_t)(uvec_ret_asm) - (uint64_t)(uvec_enter_asm) + (uint64_t)(PROC_VA_TRAMPOLINE));
  ((uvec_ret_func)(fn))(PROC_VA_TRAPFRAME, satp);
}
