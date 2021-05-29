#include "disk_hal.h"
#include "../kernel/pmem.h"
#include "../kernel/kmem.h"
#include "../kernel/panic.h"


#ifdef QEMU
#include "../driver/virtio.h"
#include "../driver/virtio_blk.h"

struct disk_hal *disk_hal_init() {
  struct disk_hal *disk = kmalloc(sizeof(struct disk_hal));
  struct virtio_blk *blk = virtio_blk_init(VIRTIO_BLK_MMIO_BASE);
  disk->dev = blk;
  disk->len = sizeof(*blk);
  disk->ops.read_op = disk_hal_read;
  disk->ops.write_op = disk_hal_write;
  return disk;
}

bool disk_hal_read(struct disk_hal *params, uint64_t sector, void *buf) {
  struct virtio_blk *blk = params->dev;
  struct virtio_blk_req req;
  req.sector = sector;
  req.type = VIRTIO_BLK_T_IN;
  virtio_blk_submit(blk, &req, buf);
  return virtio_blk_wait(blk, &req);
}

bool disk_hal_write(struct disk_hal *params, uint64_t sector, void *buf) {
  struct virtio_blk *blk = params->dev;
  struct virtio_blk_req req;
  req.sector = sector;
  req.type = VIRTIO_BLK_T_OUT;
  virtio_blk_submit(blk, &req, buf);
  return virtio_blk_wait(blk, &req);
}

bool disk_hal_destory(struct disk_hal *disk) {
  KERNEL_PANIC();
}

#endif // QEMU

#ifdef K210

#include "../driver/kendryte/fpioa.h"
#include "../driver/sdcard.h"

struct disk_hal *disk_hal_init() {
  struct disk_hal *disk = kmalloc(sizeof(struct disk_hal));
  disk->ops.read_op = disk_hal_read;
  disk->ops.write_op = disk_hal_write;

  // init the fpio config
  fpioa_set_function(27, FUNC_SPI0_SCLK);
  fpioa_set_function(28, FUNC_SPI0_D0);
  fpioa_set_function(26, FUNC_SPI0_D1);
  fpioa_set_function(32, FUNC_GPIOHS7);
  fpioa_set_function(29, FUNC_SPI0_SS3);

  sdcard_init();
  return disk;
}

bool disk_hal_read(struct disk_hal *params, uint64_t sector, void *buf) {
  sdcard_read_sector(buf, sector);
  return true;
}

bool disk_hal_write(struct disk_hal *params, uint64_t sector, void *buf) {
  sdcard_write_sector(buf, sector);
  return true;
}

bool disk_hal_destory(struct disk_hal *disk) {
  KERNEL_PANIC();
}
#endif



