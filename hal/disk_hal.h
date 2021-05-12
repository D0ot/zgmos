#ifndef __DISK_HAL_H_
#define __DISK_HAL_H_

#include <stdint.h>
#include <stdbool.h>

struct disk_hal;

typedef bool(*disk_hal_read_func)(struct disk_hal *params, uint32_t sector, void *buf);
typedef bool(*disk_hal_write_func)(struct disk_hal *params, uint32_t sector, void *buf);

struct disk_hal_ops {
  disk_hal_read_func read_op;
  disk_hal_write_func write_op;
};

struct disk_hal {
  void *dev;
  uint64_t len;
  struct disk_hal_ops ops;
};

struct disk_hal *disk_hal_init();
bool disk_hal_read(struct disk_hal *disk, uint32_t sector, void *buf);
bool disk_hal_write(struct disk_hal *disk, uint32_t sector, void *buf);
bool disk_hal_destory(struct disk_hal *disk);


#endif // __DISK_HAL_H_
