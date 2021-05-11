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

bool disk_hal_read(struct disk_hal *params, uint32_t sector, void *buf) {
  struct virtio_blk *blk = params->dev;
  struct virtio_blk_req req;
  req.sector = sector;
  req.type = VIRTIO_BLK_T_IN;
  virtio_blk_submit(blk, &req, buf);
  return virtio_blk_wait(blk, &req);
}

bool disk_hal_write(struct disk_hal *params, uint32_t sector, void *buf) {
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
