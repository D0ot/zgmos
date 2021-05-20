#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "defs.h"
#include "kmem.h"
#include "slab.h"
#include "list.h"
#include "earlylog.h"
#include "panic.h"
#include "utils.h"
#include "buddy.h"
#include "pmem.h"
#include "kmem.h"


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
  kmem_cache_shrink(kc);
  slab_free(kmem_chain.kmem_cache_slab, kc);
}

void kmem_cache_shrink(kmem_cache_t *kc) {
  slab_t *slab = NULL;
  struct list_head *pos, *n;
  list_for_each_safe(pos, n, &kc->slab_free) {
    list_del(pos);
    slab = container_of(pos, slab_t, list);
    slab_destory(kmem_chain.pslab, slab);
  }
}

void *kmem_cache_alloc(kmem_cache_t *kc) {
  slab_t *slab = NULL;
  void *ret = NULL;
  if(!list_is_empty(&kc->slab_partial)) {
    // allocate from slab_partial
    slab = container_of(kc->slab_partial.next, slab_t, list);
    ret = slab_alloc(slab);
    if(slab_full(slab)) {
      list_del(&(slab->list));
      list_add(&(slab->list), &kc->slab_full);
    }
  } else if(!list_is_empty(&kc->slab_free)) {
    // allocate from slab_free
    slab = container_of(kc->slab_free.next, slab_t, list);
    ret = slab_alloc(slab);
    list_del(&slab->list);

    if(slab_full(slab)) {
      list_add(&slab->list, &kc->slab_full);
    } else {
      list_add(&slab->list, &kc->slab_partial);
    }

  } else {
    // add new slab to this kmem_cache
    // child slab
    slab = slab_create(kmem_chain.pslab, kc->objsize, KMEM_CACHE_CSLAB_ORDER, (void*)kc);
    if(slab) {
      ret = slab_alloc(slab);

      // when a slab only has one obj in total
      // an alloc can make it empty
      if(slab_full(slab)) {
        list_add(&slab->list, &kc->slab_full);
      } else {
        list_add(&slab->list, &kc->slab_partial);
      }
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
  spinlock_init(&kmem_chain.lock, "kmem_cache");

  // initialize meta slab object comsumes some physical pages
  slab_static_init();

  // STATIC PHYSICAL PAGE ALLOC
  kmem_chain.kmem_cache_slab = slab_create_static(sizeof(kmem_cache_t), KMEM_CACHE_SLAB_ORDER);
  list_init(&kmem_chain.cache_list);
  // STATIC PHYSICAL PAGE ALLOC
  kmem_chain.pslab = slab_create_static(sizeof(slab_t), KMEM_CACHE_PSLAB_ORDER);
  for(int i = 0; i < KMEM_CACHE_SIZE_LIST_LEN; ++i) {
    kmem_cache_t *kc = kmem_cache_create(KMEM_CACHE_SIZE_LIST[i]);
    kmem_add(kc);
  }
}

void *kmem_alloc(size_t objsize) {
  void *ret = NULL;

  if(objsize > KMEM_CACHE_SIZE_LIST[KMEM_CACHE_SIZE_LIST_LEN - 1]) {
    // obj too big, use pmem_alloc instead
    return NULL;
  }
  spinlock_acquire(&kmem_chain.lock);

  if(objsize <= KMEM_CACHE_SIZE_LIST[0]) {
    ret = kmem_cache_alloc(container_of(kmem_chain.cache_list.next, kmem_cache_t, list));
    goto alloc_success;
  }

  struct list_head *pos;
  kmem_cache_t *res = NULL;
  list_for_each(pos, &kmem_chain.cache_list) {
    kmem_cache_t *kc = container_of(pos, kmem_cache_t, list);
    if(objsize <= kc->objsize) {
      res = kc;
      break;
    }
  }
  ret = kmem_cache_alloc(res);

alloc_success:
  spinlock_release(&kmem_chain.lock);
  return ret;
}

void kmem_free(void *addr) {
  void *aligned_addr = (void*)ALIGN_4K(addr);
  // the adat is set by slab_create
  spinlock_acquire(&kmem_chain.lock);
  kmem_cache_t *kc = pmem_get_adat(aligned_addr);
  kmem_cache_free(kc, addr);
  spinlock_release(&kmem_chain.lock);
}

void kmem_shrink() {
  struct list_head *pos;
  spinlock_acquire(&kmem_chain.lock);
  list_for_each(pos, &kmem_chain.cache_list) {
    kmem_cache_t *kc = container_of(pos, kmem_cache_t, list);
    kmem_cache_shrink(kc);
  }
  spinlock_release(&kmem_chain.lock);
}

void kmem_add(kmem_cache_t *kc) {
  list_add_tail(&kc->list, &kmem_chain.cache_list);
}

void kmem_del(kmem_cache_t *kc) {
  list_del(&kc->list);
}


void *kmalloc(size_t objsize) {
  return kmem_alloc(objsize);
}
void kfree(void *obj) {
  kmem_free(obj);
}


#define KMEM_DEBUG_SLAB_PRINT(list_ptr) \
  do { \
    struct list_head *lptr;\
    printf(#list_ptr">>> \n"); \
    list_for_each(lptr, list_ptr) { \
      slab_t *slab = container_of(lptr, slab_t, list); \
      slab_debug_print(slab); \
      obj_total += slab->total_obj; \
      obj_used += slab->total_obj - slab->free_obj; \
      page_usage += POWER_OF_2(slab->order); \
    } \
    printf(#list_ptr"<<< \n"); \
  }while(0)


void kmem_debug_print() {
  printf("KMEM DEBUG INFO >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  uint64_t page_usage = 0;
  uint64_t obj_used = 0;
  uint64_t obj_total = 0;

  struct list_head *pos;
  list_for_each(pos, &kmem_chain.cache_list) {
    kmem_cache_t *kc = container_of(pos, kmem_cache_t, list);
    printf("kmem_cache_t, objsize: %l>>>\n",kc->objsize);

    KMEM_DEBUG_SLAB_PRINT(&kc->slab_free);
    KMEM_DEBUG_SLAB_PRINT(&kc->slab_partial);
    KMEM_DEBUG_SLAB_PRINT(&kc->slab_full);

    printf("kmem_cache_t, objsize: %l<<<\n",kc->objsize);
  }
  printf("obj usage: %l/%l, page used: %l\n", obj_used, obj_total, page_usage);
  printf("KMEM DEBUG INFO <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
}

void kmem_test() {
  printf("KMEM TEST START >>>>>>>>>>\n");
  buddy_debug_print();
  static const int order = 2;
  void **pa = pmem_alloc(order);

  kmem_debug_print();
  int obj_size_list[] = {1,3,5,7,8,12,31,33,48,64,63,51,99,129,400,500,666};
  const int obj_size_list_len = sizeof(obj_size_list) / sizeof(obj_size_list[0]);

  for(int i = 0; i < PAGE_SIZE * POWER_OF_2(order) / sizeof(void*);  ++i) {
    pa[i] = kmem_alloc(obj_size_list[i % obj_size_list_len]);
    //printf("kmem_alloc address : %x\n", pa[i]);
    *(uint64_t*)(pa[i]) = i;
  }
  kmem_debug_print();

  for(int i = 0; i < PAGE_SIZE * POWER_OF_2(order) / sizeof(void*);  ++i) {
    kmem_free(pa[i]);
    //printf("kmem_free address : %x\n", pa[i]);
  }
  kmem_debug_print();
  for(int i = 0; i < PAGE_SIZE * POWER_OF_2(order) / sizeof(void*);  ++i) {
    pa[i] = kmem_alloc(obj_size_list[(i * 7) % obj_size_list_len]);
    //printf("kmem_alloc address : %x\n", pa[i]);
    *(uint64_t*)(pa[i]) = i;
  }
  kmem_debug_print();

  for(int i = 0; i < PAGE_SIZE * POWER_OF_2(order) / sizeof(void*);  ++i) {
    kmem_free(pa[i]);
    //printf("kmem_free address : %x\n", pa[i]);
  }

  kmem_debug_print();

  kmem_shrink();

  printf("After Shrink\n");
  kmem_debug_print();


  pmem_free(pa);
  buddy_debug_print();
  printf("KMEM TEST END   >>>>>>>>>>\n");


}
