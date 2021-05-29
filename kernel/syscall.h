#ifndef __SYSCALL_H_
#define __SYSCALL_H_

#include <stdint.h>
#include <stddef.h>

int syscall();

uint64_t syscall_arg(int i);
void syscall_arg_check(int i);
uint64_t syscall_num();

#define SYSCALL_GET_ARG_FUNC_DEC(type, suffix) \
  type syscall_arg_##suffix(int i);

SYSCALL_GET_ARG_FUNC_DEC(void*, ptr);
SYSCALL_GET_ARG_FUNC_DEC(int, int);

SYSCALL_GET_ARG_FUNC_DEC(int ,fd);
SYSCALL_GET_ARG_FUNC_DEC(size_t, size);


#define SYS_getcwd        (17)
#define SYS_dup           (23)
#define SYS_dup3          (24)
#define SYS_mkdirat       (35)
#define SYS_unlinkat      (35)
#define SYS_linkat        (37)
#define SYS_umount2       (39)
#define SYS_mount         (40)
#define SYS_chdir         (49)
#define SYS_openat        (56)
int syscall_openat();

#define SYS_pipe2         (59)
#define SYS_close         (57)
#define SYS_getdents64    (61)
#define SYS_read          (63)
int syscall_read();
#define SYS_write         (64)
int syscall_write();
#define SYS_fstat         (80)

#define SYS_clone         (220)
#define SYS_execve        (221)
#define SYS_wait4         (260)

#define SYS_exit          (93)
int syscall_exit();


#define SYS_getppid       (173)
int syscall_getppid();
#define SYS_getpid        (172)
int syscall_getpid();

#define SYS_brk           (214)
#define SYS_munmap        (215)
#define SYS_mmap          (222)

#define SYS_times         (153)
#define SYS_uname         (160)
#define SYS_sched_yield   (124)
int syscall_sched_yield();

#define SYS_gettimeofday  (169)
#define SYS_nanosleep     (101)

#endif // __SYSCALL_H_
