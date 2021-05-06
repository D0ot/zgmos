#ifndef __PMEN_H_
#define __PMEN_H_

#include <stdint.h>

void pmem_init(void *pmem_start, void *pmem_end);
void* pmem_alloc(int64_t order);
void pmem_free(void *pa);
void pmem_debug_stub();
void pmem_test_stub();

// pa is page aligned
void pmem_set_adat(void *pa, void *adat);

// pa is page aligned
void *pmem_get_adat(void *pa);

#endif
