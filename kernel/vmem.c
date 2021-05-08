#include "vmem.h"
#include "kustd.h"
#include "defs.h"
#include "pg.h"
#include "panic.h"
#include "earlylog.h"
#include "utils.h"
#include "pmem.h"

pte_t *vmem_create() {
  pte_t *p = pmem_alloc(0);
  if(p) {
    memset(p, 0, PAGE_SIZE);
  }
  return p;
}

void vmem_map(pte_t *p, void *va, void *pa, uint64_t flags, int page_size) {
  pte_t *p2 = p + VA_VPN2(va);
  pte_t xwrv2 = PTE_EXTRACT_XWRV(*p2);

  if(page_size == VMEM_PAGE_4K) {

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
    
  }else if(page_size == VMEM_PAGE_2M) {
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
    
  }else if(page_size == VMEM_PAGE_1G) {
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

void vmem_unmap(pte_t *p, void *va) {
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

void *vmem_walk(pte_t *p, void *va, uint64_t *flags, int *page_size) {
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
      pstmp = VMEM_PAGE_4K;
    } else {
      // 2MiB leaf
      res = p1;
      pstmp = VMEM_PAGE_2M;
    }

  } else {
    // 1GiB leaf
    res = p2;
    pstmp = VMEM_PAGE_1G;
  }

  if(page_size) {
    *page_size = pstmp;
  }

  if(flags) {
    *flags = PTE_EXTRACT_FLAGS(*res);
  }
  return (void*)PTE_EXTRACT_NADDR(*res);
}

void vmem_destory(pte_t *p) {
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

void vmem_test() {
  pte_t *p;

  printf("VMEM_TEST >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  pmem_debug_stub();

  p = vmem_create();
  vmem_debug_print(p);
  for(uint64_t i = 0; i < 1024; ++i) {
    vmem_map(p, (void*)(i * PAGE_SIZE * 16), (void*)(i * PAGE_SIZE), PTE_RO_SET, VMEM_PAGE_4K);
  }

  vmem_debug_print(p);

  for(uint64_t i = 0; i < 1024; ++i) {
    vmem_unmap(p, (void*)(i * PAGE_SIZE * 16));
  }
  
  vmem_debug_print(p);
  
  vmem_destory(p);

  p = vmem_create();
  for(uint64_t i = 0; i < 1024; ++i) {
    vmem_map(p, (void*)(i * PAGE_SIZE * POWER_OF_2(10)), (void*)(i * PAGE_SIZE), PTE_RO_SET, VMEM_PAGE_2M);
  }

  vmem_debug_print(p);

  for(uint64_t i = 0; i < 1024; ++i) {
    vmem_unmap(p, (void*)(i * PAGE_SIZE * POWER_OF_2(10)));
  }
  vmem_debug_print(p);

  vmem_destory(p);
 
  pmem_debug_stub();

  printf("VMEM_TEST <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
}

void vmem_debug_print(pte_t *p) {
  printf("VMEM_DEBUG_PRINT >>>>>>>>>>>>>>>>>>>>\n");
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

  printf("VMEM_DEBUG_PRINT <<<<<<<<<<<<<<<<<<<<\n");
}