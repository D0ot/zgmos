#ifndef __PANIC_H_
#define __PANIC_H_

#define KERNEL_PANIC() \
  do { \
    while(1); \
  } while(0)


#endif // __PANIC_H_
