#ifndef __RISCV_H_
#define __RISCV_H_

#include "stdint.h"
#include "utils.h"

#define RISCV_SV39 ((uint64_t)8 << 60)
#define RISCV_MAKE_SATP(addr) ((RISCV_SV39 | ((uint64_t)addr >> 12)))

#define RISCV_CSR_R_FUNC(x) \
  static inline uint64_t r_##x() { \
    uint64_t x; \
    asm volatile("csrr %0, " #x : "=r"(x)); \
    return x; \
  }

#define RISCV_CSR_W_FUNC(x) \
  static inline void w_##x(uint64_t x) { \
    asm volatile("csrw " #x ", %0" : : "r"(x)); \
  } 

#define RISCV_CSR_SWAP_FUNC(x) \
  static inline uint64_t swap_##x(uint64_t x) { \
    uint64_t ret; \
    asm volatile("csrrw %0, " #x ", %1" : "=r"(ret) : "r"(x)); \
    return ret; \
  }

#define RISCV_CSR_S_FUNC(x) \
  static inline void s_##x(uint64_t x) { \
    asm volatile("csrs " #x ", %0" : : "r"(x)); \
  } 

#define RISCV_CSR_C_FUNC(x) \
  static inline void c_##x(uint64_t x) { \
    asm volatile("csrc " #x ", %0" : : "r"(x)); \
  }

#define RISCV_CSR_FUNC(x) \
  RISCV_CSR_R_FUNC(x) \
  RISCV_CSR_W_FUNC(x) \
  RISCV_CSR_C_FUNC(x) \
  RISCV_CSR_S_FUNC(x) \
  RISCV_CSR_SWAP_FUNC(x)

#define RISCV_DEF_BIT(csr, bitname, bitpos) \
  static const uint64_t csr##_##bitname = ((uint64_t)1 << bitpos)

#define RISCV_DEF_BITS(csr, bitname, bitpos, bitwid) \
  static const uint64_t csr##_##bitname = ((uint64_t)(ALL_ONE_MASK(bitwid)) << bitpos)

static inline void set_hartid(uint64_t hartid) {
  asm volatile("mv tp, %0" : : "r"(hartid));
}

static inline uint64_t get_hartid() {
  uint64_t ret;
  asm volatile("mv %0, tp": "=r"(ret));
  return ret;
}

static inline void sfence_vma() {
  asm volatile("sfence.vma");
}

static inline uint64_t r_time() {
  uint64_t ret;
  asm volatile("rdtime %0": "=r"(ret));
  return ret;
}

// USER = user
// SUPV = supervisor

static const uint64_t SCAUSE_USER_SOFT = (1LL << 63) | 0;
static const uint64_t SCAUSE_SUPV_SOFT = (1LL << 63) | 1;
static const uint64_t SCAUSE_USER_TIMER = (1LL << 63) | 4;
static const uint64_t SCAUSE_SUPV_TIMER = (1LL << 63) | 5;
static const uint64_t SCAUSE_USER_EXT = (1LL << 63) | 8;
static const uint64_t SCAUSE_SUPV_EXT = (1LL << 63) | 9;

// MA = miss aligned
// AF = access fault
// PF = page fault
static const uint64_t SCAUSE_INST_MA= 0;
static const uint64_t SCAUSE_INST_AF = 1;
static const uint64_t SCAUSE_ILLEGAL_INST = 2;
static const uint64_t SCAUSE_BREAKPOINT = 3;
static const uint64_t SCAUSE_LOAD_ADDR_MA = 4;
static const uint64_t SCAUSE_LOAD_AF = 5;
static const uint64_t SCAUSE_STORE_ADDR_MA = 6;
static const uint64_t SCAUSE_STORE_AF = 6;
static const uint64_t SCAUSE_ECALL_USER = 8;
static const uint64_t SCAUSE_ECALL_SUPV = 9;
static const uint64_t SCAUSE_ECALL_MACH = 11;
static const uint64_t SCAUSE_INST_PF = 12;
static const uint64_t SCAUSE_LOAD_PF = 13;
static const uint64_t SCAUSE_STORE_PF = 15;

RISCV_DEF_BIT(SSTATUS, SIE, 1);
RISCV_DEF_BIT(SSTATUS, SPIE, 5);
RISCV_DEF_BIT(SSTATUS, SPP, 8);

RISCV_DEF_BIT(SIE, SSIE, 1);
RISCV_DEF_BIT(SIE, STIE, 5);
RISCV_DEF_BIT(SIE, SEIE, 9);

RISCV_DEF_BIT(SIP, SSIE, 1);
RISCV_DEF_BIT(SIP, STIE, 5);
RISCV_DEF_BIT(SIP, SEIE, 9);

RISCV_CSR_FUNC(satp);
RISCV_CSR_FUNC(sip);
RISCV_CSR_FUNC(sie);
RISCV_CSR_FUNC(sepc);
RISCV_CSR_FUNC(stvec);
RISCV_CSR_FUNC(sscratch);
RISCV_CSR_FUNC(sstatus);

RISCV_CSR_R_FUNC(scause);


#endif
