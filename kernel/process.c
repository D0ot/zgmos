#include "process.h"
#include "kmem.h"
#include "kustd.h"
#include "list.h"
#include "pg.h"
#include "pmem.h"
#include "pte.h"
#include "defs.h"
#include "elf.h"
#include "earlylog.h"
#include "utils.h"

bool task_aux_check_elf(Elf64_Ehdr *ehdr) {
  // check magic
  if(ehdr->e_ident[EI_MAG0] != ELFMAG0  ||
      ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
      ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
      ehdr->e_ident[EI_MAG3] != ELFMAG3 ) {
    return false;
  }

  // check class
  if(ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
    return false;
  }
  
  // check data encoding 
  if(ehdr->e_ident[EI_DATA] !=  ELFDATALSB) {
    return false;
  }

  // check version
  if(ehdr->e_ident[EI_VERSION] != EV_CURRENT) {
    return false;
  }

  // we did not check ELFOSABI


  // check if it is a executable
  if(ehdr->e_type != ET_EXEC) {
    return false;
  }

  // check e_machine
  if(ehdr->e_machine != EM_RISCV) {
    return false;
  }

  // check e_version
  if(ehdr->e_version != EV_CURRENT) {
    return false;
  }

  // check ELF header size 
  if(ehdr->e_ehsize != sizeof(Elf64_Ehdr)) {
    return false;
  }

  // check program header size
  if(ehdr->e_phentsize != sizeof(Elf64_Phdr)) {
    return false;
  }

  // check program header count, if it is zero, there must be an error
  if(ehdr->e_phnum == 0) {
    return false;
  }

  return true;
}

pte_t task_aux_map_flags(Elf64_Word seg_flags) {
  pte_t flags = 0;

  if(seg_flags & PF_X) {
    flags |= PTE_X_SET;
  }

  if(seg_flags & PF_R) {
    flags |= PTE_R_SET;
  }

  if(seg_flags & PF_W) {
    flags |= PTE_W_SET;
  }

  if(!flags) {
    return 0;
  }

  flags |= PTE_V_SET | PTE_U_SET;

  return flags;
}


struct task_struct *task_create(struct vnode *image, struct task_struct *parent) {

  if(!image) {
    goto after_none;
  }

  // alloc space for struct task_struct
  struct task_struct *task = kmalloc(sizeof(struct task_struct));
  if(!task) {
    goto after_none;
  }

  list_init(&task->children);
  list_init(&task->pages);

  // if parent == NULL, the process is init process
  task->parent = parent;
  if(parent) {
    list_add(&task->ubp, &parent->children);
  }

  task->user_pte = pte_create();
  if(!task->user_pte) {
    goto after_task;
  }

  task->tfp = pmem_alloc(0);
  if(!task->tfp) {
    goto after_pte;
  }
  pte_map(task->user_pte, (void*)PROC_VA_TRAPFRAME, task->tfp, PTE_RW_SET, PTE_PAGE_4K);

  // currently not used
  task->kstack_pa = pmem_alloc(1);
  if(!task->kstack_pa) {
    goto after_tfp;
  }
  // map stack page, currently not used
  pte_range_map(task->user_pte, (void*)PROC_VA_KSTACK, task->kstack_pa, PTE_RW_SET, PAGE_SIZE * 2);

  
  task->ustack_pa = pmem_alloc(0);
  if(!task->ustack_pa) {
    goto after_kstack;
  }
  pte_map(task->user_pte, (void*)PROC_VA_USTACK, task->ustack_pa, PTE_RW_SET | PTE_U_SET, PTE_PAGE_4K);


  
  // ELF LOADER
  Elf64_Ehdr *ehdr = kmalloc(sizeof(Elf64_Ehdr));
  if(!ehdr) {
    goto after_ustack;
  }
  

  uint64_t cnt = vfs_read(global_vfs, image, 0, (void*)(ehdr), sizeof(Elf64_Ehdr));

  if(cnt != sizeof(Elf64_Ehdr)) {
    goto after_ehdr;
  }

  // check a lots of thing here
  if(!task_aux_check_elf(ehdr)) {
    goto after_ehdr;
  }

  __auto_type phcnt = 0;
  __auto_type once_load = PAGE_SIZE / sizeof(Elf64_Phdr);

  uint64_t offset = ehdr->e_phoff;

  // we load 4 program header once
  Elf64_Phdr *phdr = pmem_alloc(0);
  if(!phdr) {
    goto after_ehdr;
  }

  
  while(phcnt != ehdr->e_phnum) {
    __auto_type to_load = min(ehdr->e_phnum - phcnt, once_load);

    uint64_t byte_read = vfs_read(global_vfs, image, offset, phdr, sizeof(Elf64_Phdr) * to_load);

    if(byte_read != sizeof(Elf64_Phdr) * to_load) {
      goto in_load_fail;
    }

    for(int i = 0; i < to_load; ++i) {
      // check if it is a LOAD segment
      if(phdr[i].p_type != PT_LOAD) {
        continue;
      }

      // we can not load page which is not 4KiB aligned
      if(phdr[i].p_align != PAGE_SIZE) {
        goto in_load_fail;
      }

      pte_t flags = task_aux_map_flags(phdr[i].p_flags);
      if(!flags) {
        goto in_load_fail;
      }
      
      // we only calculate va, pa is not considered
      // the memory range is [start_va, end_va], both closed range
      uint64_t start_va = phdr[i].p_vaddr;
      uint64_t end_va = start_va + phdr[i].p_memsz - 1;
      
      uint64_t start_pg = ALIGN_4K(start_va);
      uint64_t end_pg = ALIGN_4K(end_va);

      uint64_t byte_cnt = 0;

      uint64_t byte_offset_init = ALL_ONE_MASK(12) & start_va;
      // get space for load
      for(uint64_t va = start_pg; va <= end_pg; va += PAGE_SIZE){
        struct task_page *pg = task_add_page(task, (void*)va, flags);
        if(!pg) {
          goto in_load_fail;
        }
        // load, if filesz reached, we only alloc the page, and do not load
        if(byte_cnt < phdr[i].p_filesz) {
          uint64_t len = min(phdr[i].p_filesz - byte_cnt, PAGE_SIZE - byte_offset_init);
          if(len != vfs_read(global_vfs, image, phdr[i].p_offset + byte_cnt, (void*)(pg->pa + byte_offset_init), len)) {
            goto in_load_fail;
          }
          byte_cnt += len;
          byte_offset_init = 0;
        }
      }

    }

    offset += byte_read;
    phcnt += to_load;
  }

  task->entry = (void*)ehdr->e_entry;
  task->state = TASK_RUNNABLE;

  task->tfp->sp = PROC_VA_USTACK_SP;
  

  return task;

in_load_fail:
  task_remove_all_page(task);
// comment to supress warning
// after_phdr:
  kfree(phdr);
after_ehdr:
  kfree(ehdr);
after_ustack:
  pmem_free(task->ustack_pa);
after_kstack:
  pmem_free(task->kstack_pa);
after_tfp:
  pmem_free(task->tfp);
after_pte:
  pte_destory(task->user_pte);
after_task:
  kfree(task);
after_none:
  printf("process, create process failed\n");
  return NULL;
  
}

void task_destroy(struct task_struct *task) {
}

struct task_page *task_add_page(struct task_struct *task, void *va, pte_t flags) {
  struct task_page *page = kmalloc(sizeof(struct task_page));
  if(!page) {
    return NULL;
  }

  void *pa = pmem_alloc(0);
  if(!pa) {
    kfree(page);
    return NULL;
  }

  page->va = va;
  page->pa = pa;
  
  pte_map(task->user_pte, va, pa, flags, PTE_PAGE_4K);
  list_add(&page->list, &task->pages);
  
  return page;
}

bool task_remove_page(struct task_struct *task, void *va) {
  struct list_head *iter, *n;
  list_for_each_safe(iter, n, &task->pages) {
    struct task_page *pg = container_of(iter, struct task_page, list);
    if(pg->va == va) {
      list_del(iter);
      pte_unmap(task->user_pte, va);
      pmem_free(pg->pa);
      kfree(pg);
      return true;
    }
  }
  return false;
}

void task_remove_all_page(struct task_struct *task) {
  struct list_head *iter, *n;
  list_for_each_safe(iter, n, &task->pages) {
    struct task_page *pg = container_of(iter, struct task_page, list);
    list_del(iter);
    pmem_free(pg->pa);
    kfree(pg);
  }
}

void tasK_add(struct task_struct *task);
void task_del(struct task_struct *task);

