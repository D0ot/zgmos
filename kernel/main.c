#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "extdef.h"
#include "sbi.h"
#include "static_check.h"
#include "kustd.h"
#include "earlylog.h"
#include "bootinfo.h"
#include "pmem.h"
#include "slab.h"
#include "list.h"
#include "kmem.h"
#include "pg.h"
#include "vmem.h"

int main(void) {
  // Print LOGO.
  print_bootinfo();
  // initialize the physical memory allocator.
  pmem_init(KERNEL_END, RAM_END);
  kmem_init();

  vmem_test();
  return 0;
}
