#include "bitmap.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "kmem.h"

struct bitmap *bitmap_init(void *addr, uint64_t byte_cnt) {
  struct bitmap *bm = kmalloc(sizeof(struct bitmap));
  bm->map = addr;
  bm->length = byte_cnt;
  return bm;
}

void bitmap_destory(struct bitmap *bm) {
  kfree(bm);
}

int64_t bitmap_alloc_bit(struct bitmap *bm) {
  int64_t ret = -1;
  for(uint64_t i = 0; i < bm->length; ++i) {
    for(int j = 0; j < 8; ++j) {
      if(bm->map[i] & (1 << j)) {
        ret = i * 8 + j;
        break;
      }
    }
  }
  return ret;
}

void bitmap_free_bit(struct bitmap *bm, int64_t pos) {
  int64_t i = pos / 8;
  int boff = pos % 8;
  bm->map[i] |= (1 << boff);
}
