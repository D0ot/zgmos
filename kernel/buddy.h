#ifndef _BUDDY_H_
#define _BUDDY_H_

#include <stddef.h>
#include <stdint.h>

#define MAX_BLOCK_POW  (12)

// initlize the buddy allocator
// args are physical start address and physical end address
void buddy_init(void *pa_start, void *pa_end);


void *buddy_alloc(uint8_t pow);
void buddy_free(void *pa);

uint64_t buddy_get_free_pages_count();
uint64_t buddy_get_total_pages_count();


// will not check which pa is valid or not
// the range depends on the pa's power
void buddy_set_adat(void *pa, void *adat);

// will not check which pa is valid or not
// the range depends on the pa's power
void *buddy_get_adat(void *pa);

void buddy_debug_print();

#endif
