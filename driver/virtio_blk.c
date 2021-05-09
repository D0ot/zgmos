#include "virtio_blk.h"
#include "../kernel/earlylog.h"
#include "../kernel/panic.h"
#include "virtio.h"



void virtio_blk_init(struct virtio_regs *regs) {
  virtio_blk_set_feature(regs);
}

void virtio_blk_set_feature(struct virtio_regs *regs) {
  uint32_t fea = 0;
  fea = RWV32(regs->device_features);
  printf("supported feature %x\n", (uint64_t)fea);
  fea = VIRTIO_BLK_F_BLK_SIZE |
        VIRTIO_BLK_F_CONFIG_WCE |
        VIRTIO_BLK_F_DISCARD |
        VIRTIO_BLK_F_FLUSH |
        VIRTIO_BLK_F_GEOMETRY |
        VIRTIO_BLK_F_RO | 
        VIRTIO_BLK_F_SEG_MAX |
        VIRTIO_BLK_F_SIZE_MAX |
        VIRTIO_BLK_F_TOPOLOGY;
  RWV32(regs->driver_features) = fea;

  RWV32(regs->status) = RWV32(regs->status) | VIRTIO_STATUS_FEATURES_OK;
  
  RWV32(regs->status) = RWV32(regs->status) | VIRTIO_STATUS_DRIVER_OK;
}
