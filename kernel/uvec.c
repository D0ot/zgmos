#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "riscv.h"
#include "process.h"
#include "extdef.h"
#include "uvec.h"
#include "earlylog.h"
#include "cpu.h"
#include "sbi.h"
#include "klog.h"
#include "syscall.h"

typedef void (*uvec_ret_func)(uint64_t, uint64_t);


void regdump() {
  struct trapframe *tfp = task_get_current()->tfp;
  printf("Register Dump:\n");
  printf("stval: %x\n", r_stval());
  printf("epc: %x\n", tfp->epc);
  printf("a0: %x\n", tfp->a0);
  printf("a1: %x\n", tfp->a1);
  printf("a2: %x\n", tfp->a2);
  printf("a3: %x\n", tfp->a3);
}

void yield() {
  struct cpu_t *cpu = get_cpu();
  swtch(&task_get_current()->ctx, &cpu->schduler_ctx);
}

void utrap_entry() {
  uint64_t scause = r_scause();
  uint64_t stval = r_stval();

  w_stvec((uint64_t)kvec_asm);

  if(scause == SCAUSE_ECALL_USER) {
    LOG_DEBUG("syscall from user");
    LOG_DEBUG("user sp: %x", task_get_current()->tfp->sp);
    intr_on();
    task_get_current()->tfp->epc = r_sepc() + 4;
    syscall();
  } else if(scause == SCAUSE_SUPV_TIMER) {
    LOG_DEBUG("timer interrupt from user");
    sbi_legacy_set_timer(r_time() + 30000000);
    yield();
  } else {
    LOG_DEBUG("unknown interrupt from user, scause %l", scause);
    regdump();
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
