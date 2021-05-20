#include "pid.h"
#include "bitmap.h"
#include "pmem.h"
#include "defs.h"

struct bitmap *pid_bitmap;

void pid_init() {
  void *pg = pmem_alloc(0);
  pid_bitmap = bitmap_init(pg, PAGE_SIZE);
}

int pid_alloc() {
  return bitmap_alloc_bit(pid_bitmap);
}

void pid_free(int pid) {
  bitmap_free_bit(pid_bitmap, pid);
}
