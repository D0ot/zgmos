#ifndef _BUDDY_H_
#define _BUDDY_H_

#include <stddef.h>
#include <stdint.h>

#define MAX_BLOCK_POW  (12)

void buddy_init(void *pa_start, void *pa_end);
void *buddy_alloc(uint8_t pow);
void buddy_free(void *pa);
uint64_t buddy_get_free_pages_count();

void buddy_debug_print();

#endif
