#include "process.h"
#include "stdint.h"
#include "spinlock.h"
#include "sleeplock.h"

void
initsleeplock(struct sleeplock *lk, char *name)
{
  spinlock_init(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

void
acquiresleep(struct sleeplock *lk)
{
  spinlock_acquire(&lk->lk);
  while (lk->locked) {
    // sleep(lk, &lk->lk);
  }
  lk->locked = 1;
  lk->pid = task_get_current()->pid;
  spinlock_release(&lk->lk);
}

void
releasesleep(struct sleeplock *lk)
{
  spinlock_acquire(&lk->lk);
  lk->locked = 0;
  lk->pid = 0;
  // wakeup(lk);
  spinlock_release(&lk->lk);
}

int
holdingsleep(struct sleeplock *lk)
{
  int r;
  spinlock_acquire(&lk->lk);
  r = lk->locked && (lk->pid == task_get_current()->pid);
  spinlock_release(&lk->lk);
  return r;
}
