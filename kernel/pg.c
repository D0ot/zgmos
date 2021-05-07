#include "pg.h"
#include "earlylog.h"


void pg_test() {
  pte_t pte;
  pte = PTE_PPN(0xb2, 2) | PTE_PPN(0xb1, 1) |  PTE_PPN(0xb0, 0) | PTE_XWR_SET |
    PTE_U_SET | PTE_A_SET;
  printf("PTE : %x\n", pte);
  printf("PPN0 : %x\n", PTE_EXTRACT_PPN(pte, 0));
  printf("PPN1 : %x\n", PTE_EXTRACT_PPN(pte, 1));
  printf("PPN2 : %x\n", PTE_EXTRACT_PPN(pte, 2));
  printf("FLAGS : %x\n", PTE_EXTRACT_XWRV(pte));
  printf("D : %d\n", PTE_EXTRACT_D(pte));
  printf("A : %d\n", PTE_EXTRACT_A(pte));
  printf("G : %d\n", PTE_EXTRACT_G(pte));
  printf("U : %d\n", PTE_EXTRACT_U(pte));

  pte_t naddr;

  naddr = PTE_NADDR(0x90000000);
  
  printf("PTE : %x\n", naddr);
  printf("NADDR : %x\n", PTE_EXTRACT_NADDR(naddr));
 
}
