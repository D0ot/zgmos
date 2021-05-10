#include "virtio.h"
#include "../kernel/earlylog.h"

void virtio_dev_init(volatile struct virtio_regs *regs) {
  printf("magic_value : %x\n", (uint64_t)(regs->magic_value));
}
