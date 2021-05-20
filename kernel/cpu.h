#ifndef __CPU_H_
#define __CPU_H_

#include "process.h"
#include "riscv.h"

struct cpu_t {
  struct task_struct *curent_task;
  struct context schduler_ctx;
  int64_t int_depth;
  uint64_t int_enable;
};

struct cpu_t *get_cpu();

void cpu_struct_init();
void cpu_int_push();
void cpu_int_pop();

void intr_off();
void intr_on();
uint64_t intr_get();


#endif // __CPU_H_
