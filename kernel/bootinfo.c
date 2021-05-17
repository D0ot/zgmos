#include <stdint.h>
#include <stddef.h>
#include "extdef.h"
#include "earlylog.h"

void print_bootinfo(uint64_t hartid) {
  printf("  ______   _____   __  __      ____     _____ \n");
  printf(" |___  /  / ____| |  \\/  |    / __ \\   / ____|\n");
  printf("    / /  | |  __  | \\  / |   | |  | | | (___  \n");
  printf("   / /   | | |_ | | |\\/| |   | |  | |  \\\\___ \\ \n");
  printf("  / /__  | |__| | | |  | |   | |__| |  ____) |\n");
  printf(" /_____|  \\_____| |_|  |_|    \\____/  |_____/ \n");

  printf("ZGM OS is booting, hartid: %l\n", hartid);
  printf("KERNEL_START = %x\n", KERNEL_START);
  printf("KERNEL_END = %x\n", KERNEL_END);

  printf("kernel: %x ~ %x\n", KERNEL_START, KERNEL_END);
  printf("text: %x ~ %x\n", TEXT_START, TEXT_END);
  printf("\tuvec: %x ~ %x\n", UVEC_START, UVEC_END);
  printf("rodata: %x ~  %x\n", RODATA_START, RODATA_END);
  printf("data: %x ~  %x\n", DATA_START, DATA_END);
  printf("bss: %x ~  %x\n", BSS_START, BSS_END);

  printf("bootinfo done.\n");
}

