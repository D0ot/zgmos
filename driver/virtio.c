#include "virtio.h"
#include "../kernel/earlylog.h"
#include "../kernel/panic.h"
#include "virtio_blk.h"



void virtio_dev_init(volatile struct virtio_regs *regs) {
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
    virtio_blk_init();
  }else {
    printf("unsupported device with device_id : %x\n", (uint64_t)RWV32(regs->device_id));
  }

}
