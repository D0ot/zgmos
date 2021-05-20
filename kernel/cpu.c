#include "cpu.h"
#include "defs.h"
#include "riscv.h"

struct cpu_t cpus[HART_NUM];

struct cpu_t *get_cpu() {
  return &cpus[get_hartid()];
}

void cpu_struct_init() {
  for(int i = 0; i < HART_NUM; ++i) {
    cpus[i].int_depth = 0;
  }
}

void cpu_int_push() {
  struct cpu_t *cpu = get_cpu();
  uint64_t old = intr_get();
  intr_off();
  if(cpu->int_depth == 0) {
    cpu->int_enable = old;
  }
  cpu->int_depth++;
}

void cpu_int_pop() {
  struct cpu_t *cpu = get_cpu();
  cpu->int_depth--;
  if(cpu->int_depth == 0 && intr_get()) {
    intr_on();
  }
}

void intr_off() {
  c_sstatus(SSTATUS_SIE);
}

void intr_on() {
  s_sstatus(SSTATUS_SIE);
}

uint64_t intr_get() {
  return r_sstatus() & SSTATUS_SIE;
}


