#ifndef __BITMAP_H_
#define __BITMAP_H_

#include <stdint.h>

struct bitmap {
  uint8_t *map;
  uint64_t length;
};

struct bitmap *bitmap_init(void *addr, uint64_t byte_cnt);
void bitmap_destory(struct bitmap *bm);

int64_t bitmap_alloc_bit(struct bitmap *bm);
void bitmap_free_bit(struct bitmap *bm, int64_t pos);


#endif // __BITMAP_H_
