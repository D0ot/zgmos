#include <stdbool.h>
#include "panic.h"
#include "slab.h"
#include "pmem.h"
#include "defs.h"
#include "utils.h"
#include "earlylog.h"

#define SLAB_SET_NEXT(ptr, next) *(void**)(ptr) = (next)
#define SLAB_GET_NEXT(ptr) *(void**)(ptr)

slab_t meta_slab;

void slab_create_ex(slab_t *slab, void *page_addr, int size_obj, int order) {
  slab->objs = page_addr;
  slab->pg = page_addr;
  slab->total_obj = (POWER_OF_2(order) * PAGE_SIZE) / size_obj;
  slab->free_obj = slab->total_obj;
  slab->size_obj = size_obj;
  slab->order = order;

  void *ptr = page_addr;
  for(;;) {
    if(ptr + size_obj < (void*)page_addr + PAGE_SIZE * POWER_OF_2(order)) {
      SLAB_SET_NEXT(ptr, ptr+size_obj);
      ptr = ptr + size_obj;
    } else {
      SLAB_SET_NEXT(ptr, NULL);
      break;
    }
  }
}

// size_obj == size of object
slab_t *slab_create_static(int size_obj, int order) {
  return slab_create(&meta_slab, size_obj, order);
}

// pslab = parent slab
slab_t *slab_create(slab_t *pslab, int size_obj, int order) {
  slab_t *slab = slab_alloc(pslab);
  if(slab == NULL) {
    return NULL;
  }
  void *pg = pmem_alloc(order);
  if(pg == NULL) {
    slab_free(pslab, slab);
    return NULL;
  }
  slab_create_ex(slab, pg, size_obj, order);
  return slab;
}

bool slab_contain(slab_t *slab, void* obj_addr) {
  return (slab->pg <= obj_addr) &&
    (obj_addr  < slab->pg + PAGE_SIZE * (slab->order));
}

void *slab_alloc(slab_t *slab) {
  void *ret = NULL;
  if(slab->free_obj != 0) {
    ret = slab->objs;
    // slab->objs = *(void**)ret;
    slab->objs = SLAB_GET_NEXT(ret);
    slab->free_obj--;
  }
  return ret;
}

// caller must ensure that the obj_addr is valid!
void slab_free(slab_t *slab, void *obj_addr) {
  if(slab->objs == NULL) {
    slab->objs = obj_addr;
    SLAB_SET_NEXT(obj_addr, NULL);
  } else if(obj_addr < slab->objs) {
    // *(void**)obj_addr = slab->objs;
    SLAB_SET_NEXT(obj_addr, slab->objs);
    slab->objs = obj_addr;
  } else {
    // slab->objs < obj_addr, iteration is needed
    void *p_last = NULL;
    void *ptr = slab->objs;
    while(ptr != NULL && ptr < obj_addr) {
      p_last = ptr;
      ptr = SLAB_GET_NEXT(ptr);
    }
    SLAB_SET_NEXT(obj_addr, ptr);
    SLAB_SET_NEXT(p_last, obj_addr);
  }
  slab->free_obj++;
}

int slab_get_total_obj(slab_t *slab) {
  return slab->total_obj;
}
int slab_get_free_obj(slab_t *slab) {
  return slab->free_obj;
}

bool slab_empty(slab_t *slab) {
  return slab->total_obj == slab->free_obj;
}

void slab_destory(slab_t *pslab, slab_t *slab) {
  if(slab_empty(slab)) {
    pmem_free(slab->pg);
  } else {
    KERNEL_PANIC();
  }
  slab_free(pslab, (void*)slab);
}

void slab_destory_static(slab_t *slab) {
  slab_destory(&meta_slab, slab);
}

void slab_static_init() {
  void *pg = pmem_alloc(SLAB_STATIC_PAGE_ORDER);
  slab_create_ex(&meta_slab, pg, sizeof(slab_t), POWER_OF_2(SLAB_STATIC_PAGE_ORDER));
}

void slab_static_deinit() {
  pmem_free(meta_slab.pg);
}


void slab_test() {
  pmem_debug_stub();
  slab_test_case(8);
  pmem_debug_stub();
  slab_test_case(16);
  pmem_debug_stub();
  slab_test_case(24);
  pmem_debug_stub();
  slab_test_case(32);
  pmem_debug_stub();
  slab_test_case(40);
  pmem_debug_stub();
  slab_test_case(64);
  pmem_debug_stub();
  slab_test_case(96);
  pmem_debug_stub();
  slab_test_case(128);
  pmem_debug_stub();
}

void slab_test_case(int obj_size) {
  void **pa = pmem_alloc(4);
  int max_index = 0;
  slab_t *slab = slab_create_static(obj_size, 1);
  printf("SLAB TEST SIZE START >>>>>>>>>>>>>>>>>>>>>>>>>>%d\n", obj_size);
  printf("BEFORE || free : %d\n", slab_get_free_obj(slab));
  printf("BEFORE || total : %d\n", slab_get_total_obj(slab));
  for(int i = 0; true; ++i) {
    pa[i] = slab_alloc(slab);
    if(pa[i] == NULL) {
      max_index = i;
      break;
    }
    printf("alloc_ed addr: %x\n", pa[i]);
    printf("free : %d\n", slab_get_free_obj(slab));
    printf("total : %d\n", slab_get_total_obj(slab));
  }

  for(int i = 0; i < max_index; ++i) {
    slab_free(slab, pa[i]);
    printf("freed addr: %x\n", pa[i]);
    printf("free : %d\n", slab_get_free_obj(slab));
    printf("total : %d\n", slab_get_total_obj(slab));
  }

  printf("AFTER || free : %d\n", slab_get_free_obj(slab));
  printf("AFTER || total : %d\n", slab_get_total_obj(slab));

  slab_destory_static(slab);
  printf("SLAB TEST SIZE END <<<<<<<<<<<<<<<<<<<<<<<<<<<<< %d\n", obj_size);
  pmem_free(pa);
}
 
