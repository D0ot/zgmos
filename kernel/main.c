#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "sbi.h"
#include "static_check.h"
#include "kustd.h"
#include "earlylog.h"
#include "bootinfo.h"

int main(void) {
  print_bootinfo();
  return 0;
}
