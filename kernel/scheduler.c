#include "scheduler.h"
#include "process.h"
#include "list.h"
#include "klog.h"
#include "cpu.h"
#include "sbi.h"
#include "defs.h"


struct list_head process_chain;

void scheduler_add(struct task_struct *task) {
  list_add(&task->ubs, &process_chain);
}

// take to process to list tail
void scheduler_mark(struct task_struct *task) {
  struct list_head *tmp = &task->ubs;
  list_del(tmp);
  list_add_tail(tmp, &process_chain);
}

void scheduler_init() {
  list_init(&process_chain);
}

void scheduler_run() {
  
  struct list_head *iter;
  struct task_struct *cur;
  struct cpu_t *cpu = get_cpu();
  sbi_legacy_set_timer(r_time() + TIMER_DIFF);
  for(;;) {
    cur = NULL;
    list_for_each(iter, &process_chain) {
      struct task_struct *task = container_of(iter, struct task_struct, ubs);
      if(task->state == TASK_RUNNABLE) {
        cur = task;
        break;
      }
    }

    if(cur) {
      task_set_current(cur);
      // LOG_DEBUG("enter into swtch");
      swtch(&cpu->schduler_ctx, &cur->ctx);
      // LOG_DEBUG("exit from swtch");
      scheduler_mark(cur);
    }
  }
}

