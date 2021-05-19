#ifndef __VIRTIO_CONSOLE_H_
#define __VIRTIO_CONSOLE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "virtio.h"


struct virtio_console {
  struct virtio_regs *regs;
};
struct virtio_console *virtio_console_init(void *base);



#endif // __VIRTIO_CONSOLE_H_
