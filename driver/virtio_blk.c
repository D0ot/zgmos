#include "virtio_blk.h"
#include "../kernel/earlylog.h"
#include "../kernel/panic.h"
#include "../kernel/pmem.h"
#include "../kernel/kmem.h"
#include "virtio.h"



void virtio_blk_init(struct virtio_regs *regs) {
  virtio_blk_set_feature(regs);
}

void virtio_blk_set_feature(struct virtio_regs *regs) {
  uint32_t fea, fea_sel= 0;
  fea = RWV32(regs->magic_value);
  printf("virtio_blk @ %x, supported feature %x\n", regs, (uint64_t)fea);
  fea_sel = ( 1 << VIRTIO_BLK_F_BLK_SIZE  ) |
        ( 1 << VIRTIO_BLK_F_CONFIG_WCE  ) |
        ( 1 << VIRTIO_BLK_F_DISCARD  ) |
        ( 1 << VIRTIO_BLK_F_FLUSH  ) |
        ( 1 << VIRTIO_BLK_F_GEOMETRY  ) |
        ( 1 << VIRTIO_BLK_F_SEG_MAX  ) |
        ( 1 << VIRTIO_BLK_F_SIZE_MAX  ) |
        ( 1 << VIRTIO_BLK_F_TOPOLOGY ) | 
        ( 1 << VIRTIO_F_RING_EVENT_IDX ) |
        ( 1 << VIRTIO_F_RING_INDIRECT_DESC);
  if(fea & (1 << VIRTIO_BLK_F_RO)) {
    printf("virtio_blk @ %x is read-only\n", regs);
    fea_sel |= ( 1 << VIRTIO_BLK_F_RO);
  }

  RWV32(regs->driver_features) = fea_sel;

  RWV32(regs->status) = RWV32(regs->status)  | VIRTIO_STATUS_FEATURES_OK;

  if(! (RWV32(regs->status) & VIRTIO_STATUS_FEATURES_OK )) {
    printf("virtio_blk @ %x is feature_ok is not set\n", regs);
  }
  
  RWV32(regs->status) = RWV32(regs->status)  | VIRTIO_STATUS_DRIVER_OK;
}

void virtio_blk_read(struct virtio_blk *blk) {
  struct virtio_blk_req *req = kmalloc(16);

  req->type = VIRTIO_BLK_T_IN;
  req->sector = 0;

  blk->virtq->desc[0].flags = VIRTQ_DESC_F_NEXT;
  blk->virtq->desc[0].addr = (uint64_t)req;
  blk->virtq->desc[0].len = 16;
  blk->virtq->desc[0].next = 1;

  void *dat = pmem_alloc(0);
  blk->virtq->desc[1].addr = (uint64_t)dat;
  blk->virtq->desc[1].len = 512;
  blk->virtq->desc[1].flags = VIRTQ_DESC_F_WRITE | VIRTQ_DESC_F_NEXT;
  blk->virtq->desc[1].next = 2;

  blk->virtq->desc[2].addr = (uint64_t)(dat + 512);
  blk->virtq->desc[2].len = 1;
  blk->virtq->desc[2].flags = VIRTQ_DESC_F_WRITE;

  blk->virtq->avail->ring[0] = 0;
  __sync_synchronize();
  blk->virtq->avail[0].idx++;
  __sync_synchronize();
  RWV32(blk->regs->queue_notify) = 0;
  while(1);

}

void virtio_blk_write(struct virtio_blk *blk) {
}
