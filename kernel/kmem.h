#ifndef __KMEM_H_
#define __KMEM_H_
#include <stdint.h>
#include <stddef.h>
#include "slab.h"
#include "spinlock.h"

static const size_t KMEM_CACHE_SIZE_LIST[] = { 8, 16, 32, 64, 128, 256, 512, 1024, 2048};
static const size_t KMEM_CACHE_SIZE_LIST_LEN = sizeof(KMEM_CACHE_SIZE_LIST) / sizeof(KMEM_CACHE_SIZE_LIST[0]);

// Page order of kmem_cache_slab
static const size_t KMEM_CACHE_SLAB_ORDER = 0;

// Page order of kmem_cache pslab
static const size_t KMEM_CACHE_PSLAB_ORDER = 2;

// Page order of kmem_cache cslab child slab (the slab really do the storages)
static const size_t KMEM_CACHE_CSLAB_ORDER = 1;

typedef struct kmem_cache {
  struct list_head list;
  struct list_head slab_full;
  struct list_head slab_partial;
  struct list_head slab_free;
  size_t objsize;
} kmem_cache_t;

typedef struct kmem_chain {
  struct list_head cache_list;

  // slab to allocate memory for kmem_cache
  // a slab allocator which allocates some kmem_cache objects
  slab_t *kmem_cache_slab;

  // slab to allocate memory for other slab
  // a slab allocator which allocates some slab objects, pslab is parent slab
  slab_t *pslab;

  struct spinlock lock;
} kmem_chain_t;

kmem_cache_t *kmem_cache_create(size_t size);

// caller must ensure there are no used object in this kmem_cache
// caller must ensure that the kmem_cache has been deleted from kmem_chain
void kmem_cache_destory(kmem_cache_t *kc);
void kmem_cache_shrink(kmem_cache_t *kc);
void *kmem_cache_alloc(kmem_cache_t *kc);
void kmem_cache_free(kmem_cache_t *kc, void *obj);

void kmem_init();
// only these three functions have lock operations
void *kmem_alloc(size_t objsize);
void kmem_free(void *addr);
void kmem_shrink();

// called must ensure that 2 kmem_cache with same objsize can not be added
void kmem_add(kmem_cache_t *kc);
void kmem_del(kmem_cache_t *kc);

void kmem_debug_print();
void kmem_test();

// usually, we should use this 2 functions to allocate memory for kernel
void *kmalloc(size_t objsize);
void kfree(void *obj);



#endif // __KMEM_H_
