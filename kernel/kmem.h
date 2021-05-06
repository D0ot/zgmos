#ifndef __KMEM_H_
#define __KMEM_H_
#include <stdint.h>
#include <stddef.h>


void kmem_init();
void *kmem_alloc(size_t objsize);
void kmem_free(void *addr);

void kmem_debug_print();
void kmem_test();




#endif // __KMEM_H_
