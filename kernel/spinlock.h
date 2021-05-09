#ifndef __SPINLOCK_H_
#define __SPINLOCK_H_

// struct cpu;

// Mutual exclusion lock.
struct spinlock {
  int locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  // struct cpu *cpu;   // The cpu holding the lock.
  int cpu_id;           // The cpu holding the lock.
};

// Initialize a spinlock 
void spinlock_init(struct spinlock*, char*);

// Acquire the spinlock
// Must be used with release()
void spinlock_acquire(struct spinlock*);

// Release the spinlock 
// Must be used with acquire()
void spinlock_release(struct spinlock*);

// Check whether this cpu is holding the lock 
// Interrupts must be off 
int spinlock_holding(struct spinlock*);

#endif
