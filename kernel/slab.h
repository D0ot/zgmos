#ifndef __SLAB_H_
#define __SLAB_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// IT IS NOT THE REAL SLAB, SUPER SIMPLIFIED IMPL.

typedef struct slab_struct_tag {
  void *pg;
  void *objs;
  int size_obj;
  int free_obj;
  int total_obj;
} slab_t;

void slab_static_init();

void slab_static_init();
void slab_static_deinit();


slab_t *slab_create(int obj_size, int page_count);
void* slab_alloc(slab_t *slab);
bool slab_contain(slab_t *slab, void* obj_addr);
void slab_free(slab_t *slab, void *obj_addr);
int slab_get_total_obj(slab_t *slab);
int slab_get_free_obj(slab_t *slab);
void slab_destory(slab_t *slab);
bool slab_empty(slab_t *slab);


// simple test function
void slab_test();
void slab_test_case(int obj_size);

#endif // __SLAB_H_
