/**
 * kernel u(mu) lib
 */

#ifndef __KUSTD_H_
#define __KUSTD_H_

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef void (*out_func_ptr)(char);

int printf_func(const char *format, out_func_ptr f, ...);

int v_sprintf(char *str, const char *format, va_list ap);
int v_printf(const char *format, va_list ap);
int v_printf_callback(const char *format, out_func_ptr out_func, va_list ap);

int sprintf(char *str, const char *format, ...);
void putchar(char ch);
char *strcpy(char *dest, const char *src);
void *memset(void *ptr, uint8_t value, uint64_t num);
int strcmp(char *s1, char *s2);
void *memcpy(void *dest, const void *src, size_t num);
void *memmove(void *dest, const void *src, size_t num);
size_t strlen(const char *str);

#endif
