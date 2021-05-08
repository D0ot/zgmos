#ifndef __EXTDEF_H_
#define __EXTDEF_H_

#include <stdint.h>


#define EXTDEF(ext, sym) \
  extern const uintptr_t ext; \
  static void *const sym = (void*)&(ext) \

EXTDEF(_text_start, TEXT_START);
EXTDEF(_text_end, TEXT_END);

EXTDEF(_data_start, DATA_START);
EXTDEF(_data_end, DATA_END);

EXTDEF(_rodata_start, RODATA_START);
EXTDEF(_rodata_end, RODATA_END);

EXTDEF(_bss_start, BSS_START);
EXTDEF(_bss_end, BSS_END);

EXTDEF(_kernel_start, KERNEL_START);
EXTDEF(_kernel_end, KERNEL_END);
EXTDEF(_ram_end, RAM_END);
EXTDEF(_sbi_start, SBI_START);
EXTDEF(_sbi_end, SBI_END);


extern void kvec_asm();

#endif // __EXTDEF_H_
