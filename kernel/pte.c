#include "pte.h"
#include "kustd.h"
#include "defs.h"
#include "pg.h"
#include "panic.h"
#include "earlylog.h"
#include "utils.h"
#include "pmem.h"

pte_t *pte_create() {
  pte_t *p = pmem_alloc(0);
  if(p) {
    memset(p, 0, PAGE_SIZE);
  }
  return p;
}

void pte_map(pte_t *p, void *va, void *pa, uint64_t flags, int page_size) {
  pte_t *p2 = p + VA_VPN2(va);
  pte_t xwrv2 = PTE_EXTRACT_XWRV(*p2);

  if(page_size == PTE_PAGE_4K) {

    if(xwrv2 == PTE_NON_LEAF) {
      pte_t *p1 = ((pte_t*)PTE_EXTRACT_NADDR(*p2)) + VA_VPN1(va);
      pte_t xwrv1 = PTE_EXTRACT_XWRV(*p1);

      if(xwrv1 == PTE_NON_LEAF) {
        pte_t *p0 = ((pte_t*)PTE_EXTRACT_NADDR(*p1)) + VA_VPN0(va);

        pte_t xwrv0 = PTE_EXTRACT_XWRV(*p0);

        if(xwrv0 == PTE_INVALID) {
          *p0 = PTE_NADDR(pa) | flags;
        }else {
          // seems a remap
          KERNEL_PANIC();
        }
        
      } else if(xwrv1 == PTE_INVALID){

        pte_t *p0_aligned = pmem_alloc(0);
        memset(p0_aligned, 0, PAGE_SIZE);
        pte_t *p0 = p0_aligned + VA_VPN0(va);
        *p0 = PTE_NADDR(pa) | flags;

        *p1 = PTE_NON_LEAF | PTE_NADDR(p0_aligned);
      } else {
        // seems a remap
        KERNEL_PANIC();
      }

    }else if(xwrv2 == PTE_INVALID) {
      pte_t *p1_aligned = pmem_alloc(0);
      memset(p1_aligned, 0, PAGE_SIZE);
      pte_t *p0_aligned = pmem_alloc(0);
      memset(p0_aligned, 0, PAGE_SIZE);
      
      *p2 = PTE_NADDR(p1_aligned) | PTE_NON_LEAF;

      pte_t *p1 = p1_aligned + VA_VPN1(va);
      *p1 = PTE_NADDR(p0_aligned) | PTE_NON_LEAF;

      pte_t *p0 = p0_aligned + VA_VPN0(va);
      *p0 = PTE_NADDR(pa) | flags;

    }else {
      // seems a remap
      KERNEL_PANIC();
    }
    
  }else if(page_size == PTE_PAGE_2M) {
    if(xwrv2 == PTE_NON_LEAF) {
      pte_t *p1 = ((pte_t*)PTE_EXTRACT_NADDR(*p2)) + VA_VPN1(va);
      pte_t xwrv1 = PTE_EXTRACT_XWRV(*p1);

      if(xwrv1 == PTE_INVALID) {
        *p1 = flags | PTE_NADDR(pa);
      }else {
        KERNEL_PANIC();
      }
      
    }else if(xwrv2 == PTE_INVALID) {
      
      pte_t *p1_aligned = pmem_alloc(0);
      memset(p1_aligned, 0, PAGE_SIZE);
      *p2 = PTE_NADDR(p1_aligned) | PTE_NON_LEAF;
      pte_t *p1 = p1_aligned + VA_VPN1(va);
      *p1 = PTE_NADDR(pa) | flags;

    }else {
      // seems a remap
      KERNEL_PANIC();
    }
    
  }else if(page_size == PTE_PAGE_1G) {
    if(xwrv2 != PTE_INVALID) {
      // invalid va, it seems a remap?
      KERNEL_PANIC();
    }
    *p2 = flags | PTE_NADDR(pa);
  }else {
    // invalid page_size argument.
    KERNEL_PANIC();
  }
}

void pte_range_map(pte_t *p, void *va, void *pa, uint64_t flags, uint64_t length) {
  uint64_t offset;
  for(offset = 0; offset + POWER_OF_2(12) <= length; offset += POWER_OF_2(12)) {
    pte_map(p, va + offset, pa + offset, flags, PTE_PAGE_4K);
  }

  if(offset < length) {
    pte_map(p, va + offset, pa + offset, flags, PTE_PAGE_4K);
  }
}


void pte_unmap(pte_t *p, void *va) {
  pte_t *p2 = p + VA_VPN2(va);
  pte_t xwrv2 = PTE_EXTRACT_XWRV(*p2);
  if(xwrv2 == PTE_INVALID) {
    // unmap a map which is not mapped... panic
    KERNEL_PANIC();
  }
  
  if(xwrv2 == PTE_NON_LEAF) {
    pte_t *p1 = (pte_t*)PTE_EXTRACT_NADDR(*p2) + VA_VPN1(va);
    pte_t xwrv1 = PTE_EXTRACT_XWRV(*p1);

    if(xwrv1 == PTE_INVALID) {
      // unmap a map which is not mapped... panic
      KERNEL_PANIC();
    }
    
    if(xwrv1 == PTE_NON_LEAF) {
      pte_t *p0 = (pte_t*)PTE_EXTRACT_NADDR(*p1) +VA_VPN0(va);
      pte_t xwrv0 = PTE_EXTRACT_XWRV(*p0);
      
      if(xwrv0 == PTE_INVALID) {
        KERNEL_PANIC();
      }else {
        // 4KiB leaf
        *p0 = 0;
      }

    } else {
      // 2MiB leaf
      *p1 = 0;
    }

  } else {
    // 1GiB leaf
    *p2 = 0;
  }
}

void pte_range_unmap(pte_t *p, void *va, int64_t length) {
  uint64_t offset;
  for(offset = 0; offset + POWER_OF_2(12) <= length; offset += POWER_OF_2(12)) {
    pte_unmap(p, va + offset);
  }

  if(offset < length) {
    pte_unmap(p, va + offset);
  }

}
void *pte_walk(pte_t *p, void *va, uint64_t *flags, int *page_size) {
  pte_t *p2 = p + VA_VPN2(va);
  pte_t xwrv2 = PTE_EXTRACT_XWRV(*p2);
  pte_t *res = NULL;
  // page size tmp
  int pstmp;
  
  if(xwrv2 == PTE_NON_LEAF) {
    pte_t *p1 = (pte_t*)PTE_EXTRACT_NADDR(*p2) + VA_VPN1(va);
    pte_t xwrv1 = PTE_EXTRACT_XWRV(*p1);

    if(xwrv1 == PTE_NON_LEAF) {
      pte_t *p0 = (pte_t*)PTE_EXTRACT_NADDR(*p1) +VA_VPN0(va);

      // 4KiB leaf
      res = p0;
      pstmp = PTE_PAGE_4K;
    } else {
      // 2MiB leaf
      res = p1;
      pstmp = PTE_PAGE_2M;
    }

  } else {
    // 1GiB leaf
    res = p2;
    pstmp = PTE_PAGE_1G;
  }

  if(page_size) {
    *page_size = pstmp;
  }

  if(flags) {
    *flags = PTE_EXTRACT_FLAGS(*res);
  }
  return (void*)PTE_EXTRACT_NADDR(*res);
}

void pte_destory(pte_t *p) {
  for(int i = 0; i < 512; ++i) {
    if(PTE_EXTRACT_XWRV(p[i]) == PTE_NON_LEAF) {
      pte_t *p1 = (pte_t*)PTE_EXTRACT_NADDR(p[i]);

      for(int j = 0; j < 512; ++j) {
        if(PTE_EXTRACT_XWRV(p1[j]) == PTE_NON_LEAF) {
          pmem_free((void*)PTE_EXTRACT_NADDR(p1[j]));
        }
      }

      pmem_free(p1);
    }
  }
  pmem_free(p);

}

void pte_test() {
  pte_t *p;

  printf("PTE_TEST >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  pmem_debug_stub();

  p = pte_create();
  pte_debug_print(p);
  for(uint64_t i = 0; i < 1024; ++i) {
    pte_map(p, (void*)(i * PAGE_SIZE * 16), (void*)(i * PAGE_SIZE), PTE_RO_SET, PTE_PAGE_4K);
  }

  pte_debug_print(p);

  for(uint64_t i = 0; i < 1024; ++i) {
    pte_unmap(p, (void*)(i * PAGE_SIZE * 16));
  }
  
  pte_debug_print(p);
  
  pte_destory(p);

  p = pte_create();
  for(uint64_t i = 0; i < 1024; ++i) {
    pte_map(p, (void*)(i * PAGE_SIZE * POWER_OF_2(10)), (void*)(i * PAGE_SIZE), PTE_RO_SET, PTE_PAGE_2M);
  }

  pte_debug_print(p);

  for(uint64_t i = 0; i < 1024; ++i) {
    pte_unmap(p, (void*)(i * PAGE_SIZE * POWER_OF_2(10)));
  }
  pte_debug_print(p);

  pte_destory(p);
 
  pmem_debug_stub();

  printf("PTE_TEST <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
}

void pte_debug_print(pte_t *p) {
  printf("PTE_DEBUG_PRINT >>>>>>>>>>>>>>>>>>>>\n");
  printf("pte_t @ %x\n", p);
  for(int i = 0; i < 512; ++i) {
    if(PTE_EXTRACT_XWRV(p[i]) != PTE_INVALID) {

      if(PTE_EXTRACT_XWRV(p[i]) == PTE_NON_LEAF) {
        // Non-Leaf

        printf("%d, val : %x, Non-Leaf pte\n", i, p[i]);
        pte_t *p1 = (pte_t*)PTE_EXTRACT_NADDR(p[i]);
        for(int j = 0; j < 512; ++j) {
          if(PTE_EXTRACT_XWRV(p1[j]) != PTE_INVALID) {

            if(PTE_EXTRACT_XWRV(p1[j]) == PTE_NON_LEAF) {
              // Non-Leaf
              printf("\t%d, val : %x, Non-Leaf pte\n", j, p1[j]);

              pte_t *p0 = (pte_t*)PTE_EXTRACT_NADDR(p1[j]);

              for(int k = 0; k < 512; ++k) {
                if(PTE_EXTRACT_XWRV(p0[k]) != PTE_INVALID) {
                  printf("\t\t%d, val : %x, map: %x -> %x, flags: %x, 4KiB \n",
                      k, p0[k], ((uint64_t)i << 30) + ((uint64_t)j << 21) + ((uint64_t)k << 12), PTE_EXTRACT_NADDR(p0[k]), PTE_EXTRACT_XWRV(p0[k]));
                }
              }
            }else {
              // 2MiB Leaf
              printf("\t%d, val : %x, map: %x -> %x, flags: %x, 2MiB \n",
                  j, p1[j], ((uint64_t)i << 30) + ((uint64_t)j << 21), PTE_EXTRACT_NADDR(p1[j]), PTE_EXTRACT_XWRV(p1[j]));
            }
          }

        }

      } else {
        // 1GiB Leaf
        printf("%d, val : %x, map: %x -> %x, flags: %x, 1GiB \n",
            i, p[i], (uint64_t)i << 30, PTE_EXTRACT_NADDR(p[i]), PTE_EXTRACT_XWRV(p[i]));
      }
    }
  }

  printf("PTE_DEBUG_PRINT <<<<<<<<<<<<<<<<<<<<\n");
}
