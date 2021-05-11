#include "fat32.h"
#include "kmem.h"
#include "pmem.h"
#include "earlylog.h"
#include "../hal/disk_hal.h"


struct fat32_fs *fat32_init(struct disk_hal *disk, uint32_t start_sector, uint32_t total_sector) {
  struct fat32_fs *fs = kmalloc(sizeof(struct fat32_fs));
  if(!fs) {
    printf("fat32, kmalloc failed\n");
    return NULL;
  }

  fs->start_sector = start_sector;
  fs->buf = pmem_alloc(0);

  if(!fs->buf) {
    printf("fat32, pmem_alloc failed\n");
    kfree(fs);
    return NULL;
  }

  fs->disk = disk;
  fs->total_sector = total_sector;


  disk->ops.read_op(disk, start_sector, fs->buf);
  
  struct fat32_bpb * bpb= (struct fat32_bpb*)fs->buf;

  fs->sector_per_cluster = bpb->SecPerClus;
  fs->reserved_sec_cnt = bpb->RsvdSecCnt;

  if(bpb->jmpBoot[0] != 0xeb || bpb->jmpBoot[2] != 90) {
    printf("fat32, jmpBoot mismatch\n");
  }

  if(bpb->HiddSec != start_sector) {
    fs->start_sector = bpb->HiddSec;
    printf("fat32, HiddSec != start_sector\n");
  }

  if(bpb->ToSec32 != total_sector) {
    fs->total_sector = bpb->ToSec32;
    // if total_sector is zero, we totallly depends on data read from disk
    if(total_sector) {
      printf("fat32, ToSec32 != total_sector\n");
    }
  }

  fs->fat_size = bpb->FATSz32;
  fs->fat_num = bpb->NumFATs;
  fs->root_cluster = bpb->RootClus;

  fs->fat_sector = fs->reserved_sec_cnt;
  fs->first_cluster = fs->fat_size * fs->fat_num + fs->reserved_sec_cnt;

  while(1);
  return fs;
}

void fat32_destory(struct fat32_fs *fs) {
  pmem_free(fs->buf);
  kfree(fs);
}
