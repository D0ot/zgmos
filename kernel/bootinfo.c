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
  printf("bootinfo done.\n");
}

