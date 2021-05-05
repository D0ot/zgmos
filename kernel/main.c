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


int main(void) {
  // Print LOGO.
  print_bootinfo();
  // initialize the physical memory allocator.
  pmem_init(KERNEL_END, RAM_END);
  // initialize meta slab object comsumes some physical pages
  slab_static_init();

 return 0;
}
