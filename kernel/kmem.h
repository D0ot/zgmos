#ifndef __KMEM_H_
#define __KMEM_H_
#include <stdint.h>
#include <stddef.h>
#include "slab.h"

static const size_t KMEM_CACHE_SIZE_LIST[] = { 8, 16, 32, 64, 128, 256, 512, 1024};
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
  slab_t *kmem_cache_slab;

  // slab to allocate memory for other slab
  slab_t *pslab;
} kmem_chain_t;

kmem_cache_t *kmem_cache_create(size_t size);
// TODO
void kmem_cache_destory(kmem_cache_t *kc);
void kmem_cache_shrink(kmem_cache_t *kc);
void *kmem_cache_alloc(kmem_cache_t *kc);
void kmem_cache_free(kmem_cache_t *kc, void *obj);

void kmem_init();
void *kmem_alloc(size_t objsize);
void kmem_free(void *addr);
void kmem_shrink();
void kmem_add(kmem_cache_t *kc);
void kmem_del(kmem_cache_t *kc);

void kmem_debug_print();
void kmem_test();




#endif // __KMEM_H_
