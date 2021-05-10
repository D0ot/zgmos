#include <stdarg.h>
#include "kustd.h"
#include "sbi.h"
#include "spinlock.h"

// lock to avoid interleaving concurrent printf's.
static struct {
  struct spinlock lock;
  int locking;
} pr;

void printf_lock_init(void) {
  spinlock_init(&pr.lock, "pr");
  pr.locking = 1;
}

int printf(const char *format, ...) {
  int ret;
  va_list ap;
  int locking;

  locking = pr.locking;
  if (locking) {
    spinlock_acquire(&pr.lock);
  }

  va_start(ap, format);
  ret = v_printf_callback(format, (out_func_ptr)sbi_console_putchar, ap);
  va_end(ap);

  if (locking) {
    spinlock_release(&pr.lock);
  }
  return ret;
}
