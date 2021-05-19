// Mutual exclusion spin locks.

#include "spinlock.h"
#include "panic.h"
#include "earlylog.h"
#include "riscv.h"

void spinlock_init(struct spinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  // lk->cpu = 0;
  lk->cpu_id = -1;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
void spinlock_acquire(struct spinlock *lk)
{
  // push_off(); // disable interrupts to avoid deadlock.
  w_sstatus(r_sstatus() & ~SSTATUS_SIE);

  if(spinlock_holding(lk)) {
    KERNEL_PANIC();
  }

  // On RISC-V, sync_lock_test_and_set turns into an atomic swap:
  //   a5 = 1
  //   s1 = &lk->locked
  //   amoswap.w.aq a5, a5, (s1)
  while(__sync_lock_test_and_set(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen strictly after the lock is acquired.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize();

  // Record info about lock acquisition for holding() and debugging.
  // lk->cpu = mycpu();
  lk->cpu_id = get_hartid();
}

// Release the lock.
void spinlock_release(struct spinlock *lk)
{
  if(!spinlock_holding(lk)) {
    KERNEL_PANIC();
  }

  lk->cpu_id = -1;

  // Tell the C compiler and the CPU to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other CPUs before the lock is released,
  // and that loads in the critical section occur strictly before
  // the lock is released.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code doesn't use a C assignment, since the C standard
  // implies that an assignment might be implemented with
  // multiple store instructions.
  // On RISC-V, sync_lock_release turns into an atomic swap:
  //   s1 = &lk->locked
  //   amoswap.w zero, zero, (s1)
  __sync_lock_release(&lk->locked);

  // pop_off();
  w_sstatus(r_sstatus() | SSTATUS_SIE);
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
int spinlock_holding(struct spinlock *lk)
{
  int r;
  // r = (lk->locked && lk->cpu == mycpu());
  r = (lk->locked && lk->cpu_id == get_hartid());
  return r;
}
