#include "virtio.h"
#include "virtio_blk.h"
#include "../kernel/earlylog.h"
#include "../kernel/panic.h"
#include "../kernel/defs.h"
#include "../kernel/utils.h"
#include "../kernel/kustd.h"
#include "../kernel/kmem.h"
#include "../kernel/pmem.h"


uint64_t virtio_dev_init(struct virtio_regs *regs) {
  if(RWV32(regs->magic_value) != VIRTIO_MAGIC) {
    KERNEL_PANIC();
  }

  if(RWV32(regs->version) != VIRTIO_VERSION) {
    KERNEL_PANIC();
  }
  
  RWV32(regs->status) = 0;

  RWV32(regs->status) = RWV32(regs->status) | VIRTIO_STATUS_ACKNOWLEDGE;

  RWV32(regs->status) = RWV32(regs->status) | VIRTIO_STATUS_DRIVER;

  return RWV32(regs->device_id);
}

// max queue__size for one page is about 156
struct virtio_queue *virtio_queue_alloc(uint32_t queue_size) {

  if(queue_size > VIRTIO_QUEUE_MAX_SIZE) {
    queue_size = VIRTIO_QUEUE_MAX_SIZE;
  }
  uint64_t desc_len = queue_size * sizeof(struct virtqueue_desc);
  uint64_t avail_len = queue_size * 2 + 6;

  uint64_t avail_offset = (uint64_t)align_next((void*)(desc_len), 2);
  uint64_t used_offset = (uint64_t)align_next((void*)avail_offset + avail_len, 4);

  void *pm = pmem_alloc(0);
  memset(pm, 0, PAGE_SIZE);
  struct virtio_queue *p = kmalloc(sizeof(struct virtio_queue));

  p->len = queue_size;
  p->desc = pm;
  p->avail = pm + avail_offset;
  p->avail->flags = 0;

  p->used = pm + used_offset;
  p->avail->idx = 0;
  p->used->idx = 0;
  p->free_idx = 0;

  p->avail_event = (uint16_t*)(p->used->ring + p->len);
  p->avail_event[0] = 1;
  p->used_event = p->avail->ring + p->len;
  p->used_event[0] = 0;

  for(uint64_t i = 0; i < queue_size; ++i) {
    p->desc[i].next = i + 1;
  }
  return p;
}


uint64_t virtio_alloc_desc(struct virtio_queue *vq) {
  if(vq->free_idx == vq->len) {
    KERNEL_PANIC();
  } else {
    uint64_t free_idx = vq->free_idx;
    vq->free_idx = vq->desc[free_idx].next;
    return free_idx;
  }
}

void virtio_free_desc(struct virtio_queue *vq, uint64_t desc) {
  vq->desc[desc].next = vq->free_idx;
  vq->free_idx = desc;
}

void virtio_queue_free(struct virtio_queue *vq) {
  pmem_free((void*)vq->desc);
  kfree(vq);
}

void virtio_add_queue(struct virtio_regs *regs, struct virtio_queue *vq, uint32_t vq_id) {
  RWV32(regs->queue_sel) = vq_id;
  __sync_synchronize();
  RWV32(regs->queue_num) = (uint32_t)vq->len;
  RWV32(regs->queue_desc_low) = (uint32_t)vq->desc;
  RWV32(regs->queue_desc_high) = (uint32_t)0;
  RWV32(regs->queue_driver_low) = (uint32_t)vq->avail;
  RWV32(regs->queue_driver_high) = (uint32_t)0;
  RWV32(regs->queue_device_low) = (uint32_t)vq->used;
  RWV32(regs->queue_device_high) = (uint32_t)0;
  __sync_synchronize();
  RWV32(regs->queue_ready) = 1;
}



