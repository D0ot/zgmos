#include "console.h"
#include "../hal//uart_hal.h"
#include "kmem.h"


uint64_t console_read(void *lfs, void *lobj, uint64_t offset, void *buf, uint64_t buf_len) {
  for(uint64_t i = 0; i < buf_len; ++i) {
    ((char*)(buf))[i] = uart_hal_recv();
  }
  return buf_len;
}

uint64_t console_write(void *lfs, void *lobj, uint64_t offset, void *buf, uint64_t buf_len) {
  for(uint64_t i = 0; i < buf_len; ++i) {
    uart_hal_send(((char*)(buf))[i]);
  }
  return buf_len;
}


struct vfs_backend *console_vfs_bkd() {
  struct vfs_backend *bkd = kmalloc(sizeof(struct vfs_backend));
  bkd->read = console_read;
  bkd->write = console_write;
  return bkd;
}
