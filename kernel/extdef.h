#ifndef __EXTDEF_H_
#define __EXTDEF_H_

#include <stdint.h>

extern const uintptr_t _kernel_start;
extern const uintptr_t _kernel_end;

static const void *KERNEL_START = (void*)&(_kernel_start);
static const void *KERNEL_END = (void*)&(_kernel_end);


#endif // __EXTDEF_H_
