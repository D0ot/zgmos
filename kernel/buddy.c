#include <stdint.h>
#include <stdbool.h>
#include "buddy.h"
#include "utils.h"
#include "earlylog.h"
#include "defs.h"
#include "panic.h"
#include "spinlock.h"


// next is the next block with the same pow
// pow is like "order" in linux kernel
typedef struct buddy_block_tag{
  int64_t next;
  // additional data
  void *adat;
  uint8_t pow;
  // used flag, current 0 and 1 are valid value
  // 0 for free
  // 1 for used
  uint8_t used;
} buddy_block;

static const uint8_t BUDDY_BLOCK_USED = 1;
static const uint8_t BUDDY_BLOCK_FREE = 0;


struct buddy_system {
  void *pa_start;
  void *pa_end;
  uint64_t pa_length;
  buddy_block *buddies;

  void *managed_start;
  void *managed_end;
  uint64_t m_length;
  uint64_t free_pages;
  uint64_t total_pages;
  int64_t indices[MAX_BLOCK_POW];

  struct spinlock lock;
} bs;


uint64_t buddy_get_self_memreq() {
  return ( bs.pa_length / PAGE_SIZE ) * sizeof(buddy_block);
}

uint8_t buddy_aux_get_max_true(void *pa) {
  uint64_t pi = (uint64_t)(pa) >> 12;
  for(int i = 0; i <= MAX_BLOCK_POW; ++i) {
    if(pi & POWER_OF_2(i)) {
      return i;
    }
  }
  return 0;
}

void buddy_aux_data_init(void *pa_start, void *pa_end) {
  bs.pa_start = pa_start;
  bs.pa_end = pa_end;
  bs.pa_length = pa_end - pa_start;
  bs.buddies = pa_start;
  bs.managed_start = (void*)align_to((uint64_t)(pa_start) + buddy_get_self_memreq(), PAGE_SIZE);
  bs.managed_end = pa_end;
  bs.m_length = bs.managed_end - bs.managed_start;
  bs.total_pages = bs.m_length / PAGE_SIZE;
  bs.free_pages = bs.total_pages;

  for(int i = 0; i < MAX_BLOCK_POW; ++i) {
    bs.indices[i] = -1;
  }
}

uint64_t buddy_aux_addr2index(void *pa) {
  return (pa - bs.managed_start) / PAGE_SIZE;
}

void *buddy_aux_index2addr(uint64_t index) {
  return bs.managed_start + index * PAGE_SIZE;
}


void buddy_set_used_index(int64_t index) {
  int8_t pow = bs.buddies[index].pow;
  for(int64_t i = 0; i < POWER_OF_2(pow); ++i) {
    bs.buddies[index + i].used = BUDDY_BLOCK_USED;
  }
}

void buddy_clear_used(int64_t index) {
  int8_t pow = bs.buddies[index].pow;
  for(int64_t i = 0; i < POWER_OF_2(pow); ++i) {
    bs.buddies[index + i].used = BUDDY_BLOCK_FREE;
  }
}

int8_t buddy_get_used(int64_t index) {
  int8_t pow = bs.buddies[index].pow;
  uint8_t used = bs.buddies[index].used;
  for(int64_t i = 0; i < POWER_OF_2(pow); ++i) {
    if(used != bs.buddies[index + i].used) {
      // unconsistent used state, there must be some bugs.
      // could be a bug in buddy allocator
      // could be a unexpected memory write to bs.buddies
      KERNEL_PANIC();
    }
  }
  return used;
}


void buddy_init(void *pa_start, void *pa_end) {
  spinlock_init(&bs.lock, "buddy system");
  buddy_aux_data_init(pa_start, pa_end);
  uint64_t cur_page_pos = 0;
  for(int i = MAX_BLOCK_POW - 1; i >= 0; --i) {
    if(bs.total_pages & (POWER_OF_2(i))) {
      bs.buddies[cur_page_pos].pow = i;
      bs.buddies[cur_page_pos].next = -1;
      bs.indices[i] = cur_page_pos;
      cur_page_pos += POWER_OF_2(i);
    }
  }

  // set all pages to free state
  for(int64_t i = 0; i < bs.total_pages; ++i) {
    bs.buddies[i].used = BUDDY_BLOCK_FREE;
  }
}


int64_t buddy_alloc_index(uint8_t pow) {
  int64_t ret;
  if(bs.indices[pow] != -1) {
    ret = bs.indices[pow];
    bs.indices[pow] = -1;
  } else {
    int i;
    for(i = pow + 1; i < MAX_BLOCK_POW; ++i) {
      if(bs.indices[i] != -1) {
        break;
      }
    }

    if(i == MAX_BLOCK_POW) {
      // no avaliable block found;
      ret = -1;
    }

    // do the split
    for(int j = i; j > pow; --j) {
      // split block having pow == j;
      uint64_t p1 = bs.indices[j];
      uint64_t p2 = bs.indices[j] + POWER_OF_2(j - 1);

      bs.indices[j] = bs.buddies[p1].next;

      bs.buddies[p1].pow = j - 1;
      bs.buddies[p2].pow = j - 1;

      bs.buddies[p1].next = p2;
      bs.buddies[p2].next = -1;

      bs.indices[j - 1] = p1;
    }
    ret = bs.indices[pow];
    bs.indices[pow] = bs.buddies[ret].next;
  }
  buddy_set_used_index(ret);
  return ret;
}

void *buddy_alloc(uint8_t pow) {
  spinlock_acquire(&bs.lock);
  uint64_t index = buddy_alloc_index(pow);
  void *ret;
  if(index == -1) {
    ret = NULL;
  } else {
    bs.free_pages -= POWER_OF_2(pow);
    ret = buddy_aux_index2addr(index);
  }
  spinlock_release(&bs.lock);
  return ret;
}

bool buddy_aux_mergeable(int64_t index1, int64_t index2, int8_t pow) {
  return ((index1 >> pow) ^ (index2 >> pow)) == 1;
}

int8_t buddy_free_index(int64_t index) {
  int64_t free_block = index;
  int8_t ret = bs.buddies[index].pow;
  buddy_clear_used(index);

  for(;;) {
    int8_t pow = bs.buddies[free_block].pow;
    int64_t list = bs.indices[pow];

    if(list == -1) {
      bs.indices[pow] = free_block;
      bs.buddies[free_block].next = -1;
      break;
    } else if(free_block < list) {
      // free_block < list
      if(buddy_aux_mergeable(free_block, list, pow)) {
        bs.indices[pow] = bs.buddies[list].next;
        bs.buddies[free_block].pow = pow + 1;
      } else {
        bs.indices[pow] = free_block;
        bs.buddies[free_block].next = list;
        break;
      }
    } else {
      // free_block > list
      int64_t iter_pre = -1;
      int64_t iter = list;
      while(bs.buddies[iter].next != -1 && free_block > bs.buddies[iter].next) {
        iter_pre = iter;
        iter = bs.buddies[iter].next;
      }

      if(buddy_aux_mergeable(iter, free_block, pow)) {
        if(iter_pre == -1) {
          bs.indices[pow] = bs.buddies[iter].next;
        } else {
          bs.buddies[iter_pre].next = bs.buddies[iter].next;
        }

        bs.buddies[iter].pow = pow + 1;
        free_block = iter;
        continue;
      }

      if(bs.buddies[iter].next != -1 && buddy_aux_mergeable(free_block, bs.buddies[iter].next, pow)) {
        bs.buddies[iter].next = bs.buddies[bs.buddies[iter].next].next;
        bs.buddies[free_block].pow = pow + 1;
        continue;
      }

      // can not be merged, add it to indices list
      bs.buddies[free_block].next = bs.buddies[iter].next;
      bs.buddies[iter].next = free_block;
      break;
    }
  }
  return ret;
}



void buddy_free(void *pa) {
  uint64_t index = buddy_aux_addr2index(pa);
  spinlock_acquire(&bs.lock);
  if(buddy_get_used(index) == BUDDY_BLOCK_FREE) {
    // try to free an unused memory block
    KERNEL_PANIC();
  }
  uint8_t p = buddy_free_index(index);
  bs.free_pages += POWER_OF_2(p);
  spinlock_release(&bs.lock);
}

uint64_t buddy_get_free_pages_count() {
  return bs.free_pages;
}

uint64_t buddy_get_total_pages_count() {
  return bs.total_pages;
}


void buddy_set_adat_index(int64_t index, void *adat) {
  int8_t pow = bs.buddies[index].pow;
  for(int i = 0; i < POWER_OF_2(pow); ++i) {
    bs.buddies[index + i].adat = adat;
  }
}

void *buddy_get_adat_index(int64_t index) {
  return bs.buddies[index].adat;
}

void buddy_set_adat(void *pa, void *adat) {
  buddy_set_adat_index(buddy_aux_addr2index(pa), adat);
}

void *buddy_get_adat(void *pa) {
  return buddy_get_adat_index(buddy_aux_addr2index(pa));
}

void buddy_debug_print() {
  printf("Buddy System DEBUG PRINT\n");
  printf("total : %l, free : %l;\n", bs.total_pages, bs.free_pages);
  for(int i = 0; i < MAX_BLOCK_POW; ++i) {
    if(bs.indices[i] != -1) {
      printf("i = %d, pow = %d, next = %l;\n", i, (int)bs.buddies[bs.indices[i]].pow, (uint64_t)bs.buddies[bs.indices[i]].next);
    }
  }
}

void buddy_test() {
  void *pa[100];
  printf("BUDDY ALLOCATOR TEST >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  buddy_debug_print();
  int max_iter = INT32_MAX;
  int random_order[] = {0,1,2,5,4,3,1,2,0,1,0,2,3,0,1,4};
  int random_order_size = sizeof(random_order) / sizeof(random_order[0]);
  for(int i = 0; i < 100; ++i) {
    pa[i] = buddy_alloc(random_order[i % random_order_size]);
    printf("ALLOCED ADDR : %x\n", pa[i]);
    buddy_debug_print();
    if(pa[i] == NULL) {
      max_iter = i;
    }
  }

  for(int i = 0; i < 100 && i < max_iter; ++i) {
    buddy_free(pa[i]);
    printf("FREED ADDR : %x\n", pa[i]);
    buddy_debug_print();
  }
  buddy_debug_print();
  printf("BUDDY ALLOCATOR TEST <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
}
