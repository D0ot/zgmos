#ifndef __SCHEDULER_H_
#define __SCHEDULER_H_

#include "process.h"
#include "riscv.h"


void scheduler_init();
void scheduler_run();

void scheduler_add(struct task_struct *task);
void scheduler_runnable(struct task_struct *task);
void scheduler_sleep(struct task_struct *task);
void scheduler_exit(struct task_struct *task);
void scheduler_remove(struct task_struct *task);

// if the process was run, add it to tail.
// so it is a simple scheduler
void scheduler_mark(struct task_struct *task);

#endif // __SCHEDULER_H_
