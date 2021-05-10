#include "virtio.h"
#include "../kernel/earlylog.h"
#include "../kernel/panic.h"
#include "../kernel/defs.h"
#include "../kernel/utils.h"
#include "../kernel/kustd.h"
#include "virtio_blk.h"
#include "../kernel/kmem.h"
#include "../kernel/pmem.h"



void virtio_dev_init(struct virtio_regs *regs) {
  if(RWV32(regs->magic_value) != VIRTIO_MAGIC) {
    KERNEL_PANIC();
  }

  if(RWV32(regs->version) != VIRTIO_VERSION) {
    KERNEL_PANIC();
  }
  
  RWV32(regs->status) = 0;

  RWV32(regs->status) = RWV32(regs->status) | VIRTIO_STATUS_ACKNOWLEDGE;

  RWV32(regs->status) = RWV32(regs->status) | VIRTIO_STATUS_DRIVER;

  if(RWV32(regs->device_id) == VIRTIO_DEV_BLK) {
    virtio_blk_init(regs);
  }else {
    printf("unsupported device with device_id : %x\n", (uint64_t)RWV32(regs->device_id));
  }

}


struct virtio_queue *virtio_queue_init() {
  void *pm = pmem_alloc(0);
  memset(pm, 0, PAGE_SIZE);
  struct virtio_queue *p = kmalloc(sizeof(struct virtio_queue));
  p->idx = 0;
  p->desc = pm;
  p->avail = (struct virtqueue_avail*)((void*)p->desc + sizeof(struct virtqueue_desc) * VIRTIO_QUEUE_SIZE);
  p->used = (struct virtqueue_used*)((void*)p->avail + sizeof(struct virtqueue_avail) + 2);
  p->avail->idx = 0;
  p->used->idx = 0;
  return p;
}


void virtio_queue_free(struct virtio_queue *vq) {
  pmem_free((void*)vq->desc);
  kfree(vq);
}

void virtio_add_queue(struct virtio_regs *regs, struct virtio_queue *vq) {
  RWV32(regs->queue_sel) = 0;
  __sync_synchronize();
  RWV32(regs->queue_num) = VIRTIO_QUEUE_SIZE;
  RWV32(regs->queue_desc_low) = (uint32_t)vq->desc;
  RWV32(regs->queue_desc_high) = (uint32_t)0;
  RWV32(regs->queue_driver_low) = (uint32_t)vq->avail;
  RWV32(regs->queue_driver_high) = (uint32_t)0;
  RWV32(regs->queue_device_low) = (uint32_t)vq->used;
  RWV32(regs->queue_device_high) = (uint32_t)0;
  __sync_synchronize();
  RWV32(regs->queue_ready) = 1;
}



