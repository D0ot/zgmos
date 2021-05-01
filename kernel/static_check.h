#ifndef __STATIC_CHECK_H_
#define __STATIC_CHECK_H_

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

static_assert( (sizeof(uintptr_t) == sizeof(long) ), "sizeof(uintptr_t) != sizeof(long)");

#endif // __STATIC_CHECK_H_



