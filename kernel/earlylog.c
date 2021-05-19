#include <stdarg.h>
#include "kustd.h"
#include "sbi.h"
#include "spinlock.h"
#include "klog.h"

int printf(const char *format, ...) {
  klog_lock_acquire();
  int ret;
  va_list ap;
  va_start(ap, format);
  ret = klog_va(format, ap);
  va_end(ap);
  klog_lock_release();
  return ret;
}


int puts(const char *str) {
  klog_lock_acquire();
  while(*str) {
    klog_putchar(*str);
    str++;
  }
  klog_putchar('\n');
  klog_lock_release();
  return 0;
}
