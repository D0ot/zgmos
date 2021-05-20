#ifndef __SCHEDULER_H_
#define __SCHEDULER_H_

#include "process.h"
#include "riscv.h"


void scheduler_init();
void scheduler_run();

void scheduler_add(struct task_struct *task);

#endif // __SCHEDULER_H_
