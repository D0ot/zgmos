#include "scheduler.h"
#include "process.h"
#include "list.h"
#include "klog.h"
#include "cpu.h"
#include "sbi.h"
#include "defs.h"


struct list_head process_runnable;
struct list_head process_sleep;
struct list_head process_exit;

void scheduler_add(struct task_struct *task) {
  list_add(&task->ubs, &process_runnable);
}

void scheduler_runnable(struct task_struct *task) {
  list_del(&task->ubs);
  list_add(&process_runnable, &task->ubs);
}

void scheduler_sleep(struct task_struct *task) {
  list_del(&task->ubs);
  list_add(&process_sleep, &task->ubs);
}
void scheduler_exit(struct task_struct *task) {
  list_del(&task->ubs);
  list_add(&process_exit, &task->ubs);
}

void scheduler_mark(struct task_struct *task) {
  list_del(&task->ubs);
  list_add_tail(&process_runnable, &task->ubs);
}

void scheduler_init() {
  list_init(&process_runnable);
  list_init(&process_sleep);
  list_init(&process_exit);
}

void scheduler_remove(struct task_struct *task) {
  list_del(&task->ubs);
}


void scheduler_run() {
  
  struct task_struct *cur;
  struct cpu_t *cpu = get_cpu();
  sbi_legacy_set_timer(r_time() + TIMER_DIFF);
  for(;;) {
    if(process_runnable.next == &process_runnable) {
      continue;
    }
    cur = container_of(process_runnable.next, struct task_struct, ubs);
    task_set_current(cur);
    // LOG_DEBUG("enter into swtch");
    swtch(&cpu->schduler_ctx, &cur->ctx);
    // LOG_DEBUG("exit from swtch");
    scheduler_mark(cur);
  }
}

