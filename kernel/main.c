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


int main(void) {
  print_bootinfo();
  pmem_init(KERNEL_END, RAM_END);
  return 0;
}
