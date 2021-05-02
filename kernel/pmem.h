#ifndef __PMEN_H_
#define __PMEN_H_

#include <stdint.h>

void pmem_init(void *pmem_start, void *pmem_end);
void* pmem_alloc(int64_t page_count);
void pmem_free(void *pa);
void peme_debug_stub();

#endif
