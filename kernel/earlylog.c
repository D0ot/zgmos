#include <stdarg.h>
#include "kustd.h"
#include "sbi.h"

int printf(const char *format, ...) {
  int ret;
  va_list ap;
  va_start(ap, format);
  ret = v_printf_callback(format, (out_func_ptr)sbi_console_putchar, ap);
  va_end(ap);
  return ret;
}
