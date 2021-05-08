#include <stdint.h>
#include <stddef.h>
#include "extdef.h"
#include "earlylog.h"

void print_bootinfo() {
  printf("  ______   _____   __  __      ____     _____ \n");
  printf(" |___  /  / ____| |  \\/  |    / __ \\   / ____|\n");
  printf("    / /  | |  __  | \\  / |   | |  | | | (___  \n");
  printf("   / /   | | |_ | | |\\/| |   | |  | |  \\\\___ \\ \n");
  printf("  / /__  | |__| | | |  | |   | |__| |  ____) |\n");
  printf(" /_____|  \\_____| |_|  |_|    \\____/  |_____/ \n");

  printf("ZGM OS is booting\n");
  printf("KERNEL_START = %x\n", KERNEL_START);
  printf("KERNEL_END = %x\n", KERNEL_END);

  printf("kernel: %x ~ %x\n", KERNEL_START, KERNEL_END);
  printf("text: %x ~ %x\n", TEXT_START, TEXT_END);
  printf("rodata: %x ~  %x\n", RODATA_START, RODATA_END);
  printf("data: %x ~  %x\n", DATA_START, DATA_END);
  printf("bss: %x ~  %x\n", BSS_START, BSS_END);

  printf("bootinfo done.\n");
}
