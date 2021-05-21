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

#define PRINT_REG(name) \
  printf(#name": %x\n", tfp->name);

  PRINT_REG(ra);
  PRINT_REG(sp);
  PRINT_REG(gp);
  PRINT_REG(tp);

  PRINT_REG(t0);
  PRINT_REG(t1);
  PRINT_REG(t2);

  PRINT_REG(s0);
  PRINT_REG(s1);

  PRINT_REG(a0);
  PRINT_REG(a1);
  PRINT_REG(a2);
  PRINT_REG(a3);
  PRINT_REG(a4);
  PRINT_REG(a5);
  PRINT_REG(a6);
  PRINT_REG(a7);

  PRINT_REG(s2);
  PRINT_REG(s3);
  PRINT_REG(s4);
  PRINT_REG(s5);
  PRINT_REG(s6);
  PRINT_REG(s7);
  PRINT_REG(s8);
  PRINT_REG(s9);
  PRINT_REG(s10);
  PRINT_REG(s11);

  PRINT_REG(t3);
  PRINT_REG(t4);
  PRINT_REG(t5);
  PRINT_REG(t6);
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
    task_get_current()->tfp->epc = r_sepc() + 4;
    intr_on();
    syscall();
  } else if(scause == SCAUSE_SUPV_TIMER) {
    LOG_DEBUG("timer interrupt from user");
    sbi_legacy_set_timer(r_time() + TIMER_DIFF);
    yield();
  } else {
    LOG_DEBUG("unknown interrupt from user, scause %x, stval: %x", scause, stval);
    regdump();
  }

  utrap_ret();
}

void utrap_ret() {
  // clear SPP
  c_sstatus(SSTATUS_SPP);
  s_sstatus(SSTATUS_SPIE);

  w_stvec(PROC_VA_TRAMPOLINE + ((uint64_t)uvec_enter_asm - (uint64_t)UVEC_START));

  struct task_struct *task = task_get_current();

  w_sepc(task->tfp->epc);
  
  uint64_t satp = RISCV_MAKE_SATP(task->user_pte);

  void *fn = (void*)((uint64_t)(uvec_ret_asm) - (uint64_t)(uvec_enter_asm) + (uint64_t)(PROC_VA_TRAMPOLINE));
  ((uvec_ret_func)(fn))(PROC_VA_TRAPFRAME, satp);
}
