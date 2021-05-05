/*
 * filename : pmem.c
 * desc : physical memory allocator
 */

#include <stddef.h>
#include "utils.h"
#include "buddy.h"
#include "panic.h"


// the argument is algned to 4K 
// memory area is [pmem_start, pmem_end)
void pmem_init(void *pmem_start, void *pmem_end) {
  buddy_init(pmem_start, pmem_end);
}

void* pmem_alloc(int64_t order) {
  void *ret = buddy_alloc(order);
  return ret;
}

void pmem_free(void *pa) {
  buddy_free(pa);
}

void pmem_debug_stub() {
  buddy_debug_print();
}
