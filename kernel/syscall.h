#ifndef __SYSCALL_H_
#define __SYSCALL_H_

#include <stdint.h>

int syscall();

uint64_t syscall_arg(int i);
void syscall_arg_check(int i);
uint64_t syscall_num();

#define SYSCALL_GET_ARG_FUNC_DEC(type, suffix) \
  type syscall_arg_##suffix(int i);

SYSCALL_GET_ARG_FUNC_DEC(void*, ptr);
SYSCALL_GET_ARG_FUNC_DEC(int, int);

#endif // __SYSCALL_H_
