#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "kmem.h"
#include "slab.h"
#include "list.h"
#include "earlylog.h"
#include "panic.h"

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
  slab_t *kmem_cache_slab;
  slab_t *pslab;
} kmem_chain_t;

kmem_chain_t kmem_chain;

kmem_cache_t *kmem_cache_create(size_t size) {
  kmem_cache_t *kc = slab_alloc(kmem_chain.kmem_cache_slab);
  list_init(&kc->list);
  list_init(&kc->slab_full);
  list_init(&kc->slab_partial);
  list_init(&kc->slab_free);
  kc->objsize = size;
  return kc;
}

void kmem_cache_destory(kmem_cache_t *kc) {
}

void *kmem_cache_alloc(kmem_cache_t *kc) {
  slab_t *slab;
  void *ret = NULL;
  if(!list_empty(&kc->slab_partial)) {
    // allocate from slab_partial
    slab = container_of(kc->slab_partial.next, slab_t, list);
    ret = slab_alloc(slab);
    if(slab_full(slab)) {
      list_del(&(slab->list));
      list_add(&(slab->list), &kc->slab_partial);
    }
  } else if(!list_empty(&kc->slab_free)) {
    // allocate from slab_free

    slab = container_of(kc->slab_free.next, slab_t, list);
    ret = slab_alloc(slab);

    list_del(&slab->list);
    if(slab_empty(slab)) {
      list_add(&slab->list, &kc->slab_full);
    } else {
      list_add(&slab->list, &kc->slab_partial);
    }

  } else {
    // add new slab to this kmem_cache
    // child slab
    slab = slab_create(kmem_chain.pslab, kc->objsize, KMEM_CACHE_CSLAB_ORDER);
    if(slab) {
      ret = slab_alloc(slab);
      list_add(&slab->list, &kc->slab_partial);
    }
  }
  return ret;
}

void kmem_cache_free(kmem_cache_t *kc, void *obj) {
  struct list_head *pos;
  slab_t *res = NULL;
  list_for_each(pos, &(kc->slab_partial)) {
    slab_t *slab = container_of(pos, slab_t, list);
    if(slab_contain(slab, obj)) {
      res = slab;
      break;
    }
  }
  if(res) {
    slab_free(res, obj);
    if(slab_empty(res)) {
      // move res from slab_partital to slab_free
      list_del(&res->list);
      list_add(&res->list, &(kc->slab_free));
    }
    return;
  }

  list_for_each(pos, &(kc->slab_full)) {
    slab_t *slab = container_of(pos, slab_t, list);
    if(slab_contain(slab, obj)) {
      res = slab;
      break;
    }
  }

  if(res) {
    slab_free(res, obj);
    // move res from slab_full to slab_partital
    list_del(&res->list);
    list_add(&res->list, &(kc->slab_partial));
    return;
  } else {
    // Delivering obj to this kmem_cache is not right!
    // becase we can not find a slab containing the obj
    // Panic!
    KERNEL_PANIC();
  }
}

void kmem_init() {
  slab_static_init();
  kmem_chain.kmem_cache_slab = slab_create_static(sizeof(kmem_cache_t), KMEM_CACHE_SLAB_ORDER);
  list_init(&kmem_chain.cache_list);
  kmem_chain.pslab = slab_create_static(sizeof(slab_t), KMEM_CACHE_PSLAB_ORDER);
  for(int i = 0; i < KMEM_CACHE_SIZE_LIST_LEN; ++i) {
    kmem_cache_t *kc = kmem_cache_create(KMEM_CACHE_SIZE_LIST[i]);
    list_add_tail(&kc->list ,&kmem_chain.cache_list);
  }
}

void *kmem_alloc(size_t objsize) {
  void *ret = NULL;

  if(objsize > 1024) {
    return NULL;
  }

  if(objsize <= 8) {
    return kmem_cache_alloc(container_of(kmem_chain.cache_list.next, kmem_cache_t, list));
  }

  // TODO
  struct list_head *pos;
  list_for_each(pos, &kmem_chain.cache_list) {
    kmem_cache_t *kc = container_of(pos, kmem_cache_t, list);
    if(objsize > kc->objsize) {
      break;
    }
  }
  return ret;
}

void kmem_free(void *addr, size_t objsize) {
  if(objsize == 8) {
    kmem_cache_free(container_of(kmem_chain.cache_list.next, kmem_cache_t, list), addr);
  }
}
