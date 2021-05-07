#ifndef __PG_H_
#define __PG_H_

#include "stdint.h"
#include "utils.h"

typedef uint64_t pte_t;

static const pte_t PTE_V_SET = (1 << 0);

static const pte_t PTE_R_SET = (1 << 1);
static const pte_t PTE_W_SET = (1 << 2);
static const pte_t PTE_X_SET = (1 << 3);

static const pte_t PTE_U_SET = (1 << 4);

static const pte_t PTE_G_SET = (1 << 5);

static const pte_t PTE_A_SET = (1 << 6);
static const pte_t PTE_D_SET = (1 << 7);

static const pte_t PTE_RSW_OFFSET = 8;

static const pte_t PTE_PPN0_OFFSET = 10;
static const pte_t PTE_PPN_LEN = 9;

#define PTE_PPN(v, l) ( ((pte_t)v) << (PTE_PPN0_OFFSET + PTE_PPN_LEN * l) )

// next level address
#define PTE_NADDR(addr) ( ALIGN_4K( (pte_t)(addr) ) >> (2))

#define PTE_RSW(rsw) ( (rsw) << 8 )


static const pte_t PTE_NEXT_LEVEL = PTE_V_SET;

static const pte_t PTE_RO_SET = PTE_R_SET | PTE_V_SET;

static const pte_t PTE_RW_SET = PTE_R_SET | PTE_W_SET | PTE_V_SET;

static const pte_t PTE_XO_SET = PTE_X_SET | PTE_V_SET;

static const pte_t PTE_XR_SET = PTE_X_SET | PTE_R_SET | PTE_V_SET;

static const pte_t PTE_XWR_SET = PTE_R_SET | PTE_W_SET | PTE_X_SET | PTE_V_SET;

#define PTE_EXTRACT_XWRV(pte) ( (pte) & ALL_ONE_MASK(4) )

#define PTE_EXTRACT_NADDR(pte) ( ( (pte) << (2) ) & (~ALL_ONE_MASK(12) ) )

#define PTE_EXTRACT_PPN(v, l) ( (((pte_t)v) >> (PTE_PPN0_OFFSET + PTE_PPN_LEN * l)) & (ALL_ONE_MASK(9)) )

#define PTE_EXTRACT_D(pte) (( (pte) >> 7 ) & 1)
#define PTE_EXTRACT_A(pte) (( (pte) >> 6 ) & 1)
#define PTE_EXTRACT_G(pte) (( (pte) >> 5 ) & 1)
#define PTE_EXTRACT_U(pte) (( (pte) >> 4 ) & 1)

void pg_test();


#endif
