#ifndef __SBI_H_
#define __SBI_H_

// SBI version : 0.3-rc0

#include <stddef.h>
#include <stdint.h>

static const long SBI_SUCCESS = 0;
static const long SBI_ERR_FAILED = -1;
static const long SBI_ERR_NOT_SUPPORTED  = -2;
static const long SBI_ERR_INVALID_PARAM = -3;
static const long SBI_ERR_DENIED = -4;
static const long SBI_ERR_INVALID_ADDRESS = -5;
static const long SBI_ERR_ALREADY_AVALIABLE = -6;

static const int32_t SBI_EID_LEGACY_SET_TIMER = 0;
static const int32_t SBI_EID_LEGACY_CONSOLE_PUTCHAR = 1;
static const int32_t SBI_EID_LEGACY_CONSOLE_GETCHAR = 2;
static const int32_t SBI_EID_LEGACY_CLEAR_IPI = 3;
static const int32_t SBI_EID_LEGACY_SNED_IPI = 4;
static const int32_t SBI_EID_LEGACY_REMOTE_FENCE_I = 5;
static const int32_t SBI_EID_LEGACY_REMOTE_SFENCE_VMA = 6;
static const int32_t SBI_EID_LEGACY_REMOTE_SFENCE_VMA_ASID = 7;
static const int32_t SBI_EID_LEGACY_SYSTEM_SHUTDOWN = 8;


static const int32_t SBI_EID_BASE = 0x10;
static const int32_t SBI_BASE_FID_SPEC_VERSION = 0;
static const int32_t SBI_BASE_FID_IMPL_ID = 1;
static const int32_t SBI_BASE_FID_IMPL_VERSION = 2;
static const int32_t SBI_BASE_FID_PROBE_SBI = 3;
static const int32_t SBI_BASE_FID_GET_MACHINE_VENDOR_ID = 4;
static const int32_t SBI_BASE_FID_GET_MACHINE_ARCHITECTURE_ID = 5;
static const int32_t SBI_BASE_FID_GET_MACHINE_IMPL_ID = 6;


static const int32_t SBI_EID_TIMER = 0x54494D45;
static const int32_t SBI_TIMER_FID_SET_TIMER = 0;


static const int32_t SBI_EID_IPI = 0x735049;
static const int32_t SBI_EID_RFENCE = 0x52464E43;
static const int32_t SBI_EID_HSM = 0x48534D;
static const int32_t SBI_EID_SRST = 0x53525354;


struct sbiret {
  long error;
  long value;
};

static inline struct sbiret sbicall(int32_t eid, int32_t fid, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3) {
  register uintptr_t a0 asm ("a0") = (uintptr_t)(arg0); 
  register uintptr_t a1 asm ("a1") = (uintptr_t)(arg1);
  register uintptr_t a2 asm ("a2") = (uintptr_t)(arg2);
  register uintptr_t a3 asm ("a3") = (uintptr_t)(arg3);
  
  register uintptr_t a6 asm ("a6") = (uintptr_t)(fid);
  register uintptr_t a7 asm ("a7") = (uintptr_t)(eid);
  asm volatile ("ecall" 
                : "+r"(a0), "+r"(a1)
                : "r"(a2), "r"(a3), "r"(a6),"r"(a7)
                : "memory");

  struct sbiret ret;
  ret.error = a0;
  ret.value = a1;
  return ret;
}

#define SBICALL0(_eid, _fid) sbicall( (int32_t)(_eid), (int32_t)(_fid), 0, 0, 0, 0)

#define SBICALL1(_eid, _fid, _arg0) sbicall( (int32_t)(_eid), (int32_t)(_fid), (_arg0), 0, 0, 0)

#define SBICALL2(_eid, _fid, _arg0, _arg1) sbicall( (int32_t)(_eid), (int32_t)(_fid), (_arg0), (_arg1), 0, 0)

#define SBICALL3(_eid, _fid, _arg0, _arg1, _arg2) sbicall( (int32_t)(_eid), (int32_t)(_fid), (_arg0), (_arg1), (_arg2), 0)

#define SBICALL4(_eid, _fid, _arg0, _arg1, _arg2, _arg3) sbicall( (int32_t)(_eid), (int32_t)(_fid), (_arg0), (_arg1), (_arg2), (_arg3))

static inline void sbi_console_putchar(int ch) {
  SBICALL1(SBI_EID_LEGACY_CONSOLE_PUTCHAR, 0, ch);
}

static inline int sbi_sonsole_getchar(int ch) {
  return SBICALL0(SBI_EID_LEGACY_CONSOLE_GETCHAR, 0).error;
}

static inline void sbi_legacy_set_timer(uint64_t stime_value) {
  SBICALL1(SBI_EID_LEGACY_SET_TIMER, 0, stime_value);
}

static inline struct sbiret sbi_set_timer(uint64_t stime_value) {
  return SBICALL1(SBI_EID_TIMER, SBI_TIMER_FID_SET_TIMER, stime_value);
}


#endif // __SBI_H_
