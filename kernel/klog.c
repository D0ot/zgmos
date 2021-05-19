#include "klog.h"
#include "kustd.h"
#include <stdarg.h>
#include "spinlock.h"


void uart_putchar(char ch);

void klog_putchar(char ch)
{
  uart_putchar(ch);
}

void klog_init() {
  klog_lock_init();
}

int klog(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int ret = v_printf_callback(format, klog_putchar, ap);
  va_end(ap);
  return ret;
}


int klog_va(const char *format, va_list ap) {
  return v_printf_callback(format, klog_putchar, ap);
}

void klog_level(int level, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  if(level < 0 || level > 3)
  {
    while(1)
    {
      // usage error.
    }
  }
  char *level_str[4] = {"Debug", "Info", "Warning", "Error"};
  char *output = level_str[level];
  while(*output)
  {
    klog_putchar(*output++);
  }
  output = " | ";
  while (*output)
  {
    klog_putchar(*output++);
  }
  
  v_printf_callback(format, klog_putchar, ap);
  va_end(ap);
}


struct spinlock klog_spinlock;

void klog_lock_init() {
  spinlock_init(&klog_spinlock, "klog");
}

void klog_lock_acquire() {
  spinlock_acquire(&klog_spinlock);
}
void klog_lock_release() {
  spinlock_release(&klog_spinlock);
}
