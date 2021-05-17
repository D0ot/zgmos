#include "./kustd.h"
#include "./utils.h"

int v_sprintf(char *str, const char *format, va_list ap)
{
  char *for_ret = str;
  char *temp_str;
  char buf[30];
  while (*format)
  {
    char ch = *format;
    switch (ch)
    {
    case '%':

      switch (*(format + 1))
      {
      case 'd': // integer
        itoa(va_arg(ap, int), buf, 10);
        strcpy(str, buf);
        format += 2;
        str += strlen(buf);
        break;

      case 'l': // long
        itoa(va_arg(ap, long), buf, 10);
        strcpy(str, buf);
        format += 2;
        str += strlen(buf);
        break;
 
      case 'x': // integer, hex form
        utoa(va_arg(ap, unsigned long), buf, 16);
        strcpy(str, "0x");
        strcpy(str + 2, buf);
        format += 2;
        str += strlen(buf) + 2;
        break;

      case 's':
        temp_str = va_arg(ap, char *);
        strcpy(str, temp_str);
        format += 2;
        str += strlen(temp_str);
        break;

      case '%': // single '%'
        *str++ = '%';
        format += 2;
        break;

      default:
        *str++ = ch;
        format++;
        break;
      }

      break;
    default:
      *str++ = ch;
      format++;
      break;
    }
  }

  *(str) = '\0';
  return str - for_ret;
}

int printf_func(const char *format, out_func_ptr f, ...)
{
  int ret;
  va_list ap;
  va_start(ap, f);
  ret = v_printf_callback(format, f, ap);
  va_end(ap);
  return ret;
}

int v_printf_callback(const char *format, out_func_ptr out_func, va_list ap)
{
  char buf[256];
  int ret = v_sprintf(buf, format, ap);
  char *iter = buf;
  while (*iter)
  {
    out_func(*iter++);
  }
  return ret;
}

int sprintf(char *str, const char *format, ...)
{
  int ret;
  va_list ap;
  va_start(ap, format);
  ret = v_sprintf(str, format, ap);
  va_end(ap);
  return ret;
}

size_t strlen(const char *str)
{
  size_t len = 0;

  // count how many non-null char in str
  while (*str++)
  {
    ++len;
  }
  return len;
}

char *strcpy(char *dest, const char *src)
{
  char *ret = dest;
  while (*src)
  {
    *dest++ = *src++;
  }
  *dest = '\0';
  return ret;
}

char *strcpy_end(char *dest, const char *src) {
  while (*src)
  {
    *dest++ = *src++;
  }
  *dest = '\0';
  return dest;
}

int strcmp(const char *s1, const char *s2)
{
  while (*s1 && *s2)
  {
    if (*s1 != *s2)
    {
      break;
    }
    ++s1;
    ++s2;
  }
  return *s1 - *s2;
}

void *memset(void *ptr, uint8_t value, uint64_t num)
{
  uint8_t *iter = ptr;
  for (uint64_t i = 0; i < num; ++i)
  {
    iter[i] = value;
  }
  return ptr;
}

void *memcpy(void *dest, const void *src, uint64_t num)
{
  const uint8_t *si = src;
  uint8_t *di = dest;
  for(uint64_t i = 0; i < num; ++i) {
    di[i] = si[i];
  }
  return dest;
}

void *memmove(void *dest, const void *src, uint64_t num)
{
  return memcpy(dest, src, num);
}
