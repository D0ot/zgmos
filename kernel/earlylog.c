#include <stdarg.h>
#include "kustd.h"
#include "sbi.h"
#include "spinlock.h"

static struct spinlock printf_lock;

void printf_lock_init(void) {
  spinlock_init(&printf_lock, "pr");
}

int printf(const char *format, ...) {
  int ret;
  va_list ap;

  spinlock_acquire(&printf_lock);

  va_start(ap, format);
  ret = v_printf_callback(format, (out_func_ptr)sbi_console_putchar, ap);
  va_end(ap);

  spinlock_release(&printf_lock);
  return ret;
}


int puts(const char *str) {
  spinlock_acquire(&printf_lock);

  while(*str) {
    sbi_console_putchar(*str);
    str++;
  }
  sbi_console_putchar('\n');

  spinlock_release(&printf_lock);

  return 0;
}
