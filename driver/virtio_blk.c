#include "virtio_blk.h"
#include "../kernel/earlylog.h"
#include "../kernel/panic.h"
#include "../kernel/pmem.h"
#include "../kernel/kmem.h"
#include "../kernel/kustd.h"
#include "virtio.h"



struct virtio_blk *virtio_blk_init(struct virtio_regs *regs) {
  uint32_t device_id = virtio_dev_init(regs);
  if(device_id != VIRTIO_DEV_BLK) {
    printf("virtio_blk @ %x, not a blk dev", regs);
    return NULL;
  }
  virtio_blk_set_feature(regs);
  
  struct virtio_blk *blk = kmalloc(sizeof(struct virtio_blk));
  blk->vq = virtio_queue_alloc(VIRTIO_QUEUE_MAX_SIZE);
  blk->config = (struct virtio_blk_config*)&regs->config;
  blk->regs = regs;
  
  virtio_add_queue(regs, blk->vq, 0);
  return blk;
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
//        ( 1 << VIRTIO_F_RING_EVENT_IDX ) |
        ( 1 << VIRTIO_F_RING_INDIRECT_DESC);
  if(fea & (1 << VIRTIO_BLK_F_RO)) {
    printf("virtio_blk @ %x feature read-only bit.\n", regs);
  }

  RWV32(regs->driver_features) = fea_sel;

  RWV32(regs->status) = RWV32(regs->status)  | VIRTIO_STATUS_FEATURES_OK;

  if(! (RWV32(regs->status) & VIRTIO_STATUS_FEATURES_OK )) {
    printf("virtio_blk @ %x is feature_ok is not set\n", regs);
  }
  
  RWV32(regs->status) = RWV32(regs->status)  | VIRTIO_STATUS_DRIVER_OK;
}

void virtio_blk_submit(struct virtio_blk *blk, struct virtio_blk_req *req, void *buf) {

  uint32_t datamode = 0;
  if(req->type == VIRTIO_BLK_T_IN) {
    datamode = VIRTQ_DESC_F_WRITE;
  }
  uint32_t d1 = virtio_alloc_desc(blk->vq);
  uint32_t d2 = virtio_alloc_desc(blk->vq);
  uint32_t d3 = virtio_alloc_desc(blk->vq);

  blk->vq->desc[d1].flags = VIRTQ_DESC_F_NEXT;
  blk->vq->desc[d1].addr = (uint64_t)req;
  blk->vq->desc[d1].len = 16;
  blk->vq->desc[d1].next = d2;
  
  blk->vq->desc[d2].addr = (uint64_t)buf;
  blk->vq->desc[d2].len = 512;
  blk->vq->desc[d2].flags = VIRTQ_DESC_F_NEXT | datamode;
  blk->vq->desc[d2].next = d3;

  blk->vq->desc[d3].addr = (uint64_t)(&(req->status));
  blk->vq->desc[d3].len = 1;
  blk->vq->desc[d3].flags = VIRTQ_DESC_F_WRITE;

  req->d1 = d1;
  req->d2 = d2;
  req->d3 = d3;

  virtio_blk_send(blk, d1);
}

bool virtio_blk_wait(struct virtio_blk *blk, struct virtio_blk_req *req) {
  uint64_t cnt = 0;
  while(RWV32(blk->regs->interrupt_status) == 0) {
    //printf("wait\n");
    cnt ++;
  }

  printf("used_idx:%l, desc_chain_id: %l\n", (uint64_t)blk->vq->used->idx, 
        (uint64_t)(blk->vq->used->ring[blk->vq->used->idx -1].id));
  
  blk->vq->used_event[0] = blk->vq->used->idx;
  
  RWV32(blk->regs->interrupt_ack) = RWV32(blk->regs->interrupt_status);

  virtio_free_desc(blk->vq, req->d1);
  virtio_free_desc(blk->vq, req->d2);
  virtio_free_desc(blk->vq, req->d3);

  return req->status == VIRTIO_BLK_S_OK;
}

void virtio_blk_send(struct virtio_blk *blk, uint32_t desc) {
  blk->vq->avail->ring[blk->vq->avail->idx % blk->vq->len] = desc;
  __sync_synchronize();
  blk->vq->avail->idx++;
  __sync_synchronize();
  RWV32(blk->regs->queue_notify) = 0;
}

