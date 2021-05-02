#ifndef __EXTDEF_H_
#define __EXTDEF_H_

#include <stdint.h>

extern const uintptr_t _kernel_start;
extern const uintptr_t _kernel_end;
extern const uintptr_t _ram_end;

static void *const KERNEL_START = (void*)&(_kernel_start);
static void *const KERNEL_END = (void*)&(_kernel_end);
static void *const RAM_END = (void*)&(_ram_end);


#endif // __EXTDEF_H_
