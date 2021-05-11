#include "fat32.h"
#include "kmem.h"
#include "pmem.h"
#include "earlylog.h"
#include "../hal/disk_hal.h"
#include "utils.h"
#include "kustd.h"
#include "defs.h"

// internal function declarations

static const uint32_t FAT32_BUF_INDEX_INVALID = 0xffffffff;

// buf idnex to buf addr
uint8_t *fat32_bio_bidx2addr(struct fat32_fs *fs, uint32_t bidx) {
  return (uint8_t*)fs->buf + fs->byte_per_sector * bidx;
}

// return a buf index of the specifed sector
uint32_t fat32_bio_match_buffered(struct fat32_fs *fs, uint32_t cidx) {
  for(int i = 0; i < fs->sec_per_buf; ++i) {
    if(cidx == fs->buf_cidx[i] && fs->buf_flags[i] != FAT32_BIO_FLAG_INVALID) {
      return i;
    }
  }
  return FAT32_BUF_INDEX_INVALID;
}

uint32_t fat32_bio_match_unbuffered(struct fat32_fs *fs) {
  for(int i = 0; i < fs->sec_per_buf; ++i) {
    if(fs->buf_flags[i] == FAT32_BIO_FLAG_INVALID) {
      return i;
    }
  }
  return FAT32_BUF_INDEX_INVALID;
}

// return a lowest activity buf
uint32_t fat32_bio_get_low_activity(struct fat32_fs *fs) {
  uint32_t low_activity = 0xffffffff;
  uint32_t buf_index = FAT32_BUF_INDEX_INVALID;
  for(int i = 0; i < fs->sec_per_buf; ++i) {
    if(fs->buf_flags[i] != FAT32_BIO_FLAG_INVALID) {
      if(fs->buf_activity[i] <= low_activity) {
        buf_index = i;
        low_activity = fs->buf_activity[i];
      }
    }
  }
  return buf_index;
}

// no check, just writeback
void fat32_bio_writeback_no_check(struct fat32_fs *fs, uint32_t bidx) {
  fs->disk->ops.write_op(fs->disk, fs->buf_cidx[bidx], fat32_bio_bidx2addr(fs, bidx));
  fs->buf_flags[bidx] = FAT32_BIO_FLAG_CLEAN;
}

// remove the selected buf from buf array
// write back may occur
void fat32_bio_remove(struct fat32_fs *fs, uint32_t bidx) {
  if(fs->buf_flags[bidx] == FAT32_BIO_FLAG_DIRTY) {
    fat32_bio_writeback_no_check(fs, bidx);
  }
  fs->buf_flags[bidx] = FAT32_BIO_FLAG_INVALID;
}

uint32_t fat32_bio_fetch(struct fat32_fs *fs, uint32_t cidx) {
  uint32_t bidx = fat32_bio_match_unbuffered(fs);
  if(bidx == FAT32_BUF_INDEX_INVALID) {
    bidx = fat32_bio_get_low_activity(fs);
    fat32_bio_remove(fs, bidx);
  }
  fs->disk->ops.read_op(fs->disk, cidx, fat32_bio_bidx2addr(fs, bidx));
  fs->buf_flags[bidx] = FAT32_BIO_FLAG_CLEAN;
  fs->buf_cidx[bidx] = cidx;
  return bidx;
}

void fat32_bio_flush(struct fat32_fs *fs) {
  for(int i = 0; i < fs->sec_per_buf; ++i) {
    if(fs->buf_flags[i] == FAT32_BIO_FLAG_DIRTY) {
      fat32_bio_writeback_no_check(fs, i);
    }
  }
}


uint32_t fat32_bio_read(struct fat32_fs *fs, uint32_t cidx, uint32_t offset, void *buf, uint32_t buf_len) {
  uint32_t bidx = fat32_bio_match_buffered(fs, cidx);
  uint32_t len = min(fs->byte_per_sector - offset, buf_len);

  if(bidx == FAT32_BUF_INDEX_INVALID) {
    // no buffer of cidx found, do real IO
    bidx = fat32_bio_fetch(fs, cidx);
  }
  memcpy(buf, fat32_bio_bidx2addr(fs, bidx) + offset, len);
  fs->buf_activity[bidx]++;
  return len;
}

uint32_t fat32_bio_write(struct fat32_fs *fs, uint32_t cidx, uint32_t offset, void *buf, uint32_t buf_len) {
  uint32_t bidx = fat32_bio_match_buffered(fs, cidx);
  uint32_t len = min(fs->byte_per_sector - offset, buf_len);

  if(bidx == FAT32_BUF_INDEX_INVALID) {
    // no buffer of cidx found, do real IO
    bidx = fat32_bio_fetch(fs, cidx);
  }

  memcpy(fat32_bio_bidx2addr(fs, bidx) + offset, buf, len);
  fs->buf_flags[bidx] = FAT32_BIO_FLAG_DIRTY;
  fs->buf_activity[bidx]++;
  return len;
}

void fat32_bio_test(struct fat32_fs *fs) {
  uint8_t buf[512];
  char str[] = "fat32_bio_test";
  fat32_bio_read(fs, 0, 0, buf, 512);

  for(int i = 0; i < 10; ++i) {
    fat32_bio_read(fs, i, 0, buf, 512);
    for(int j = 0; j < 10; ++j) {
      fat32_bio_write(fs, i, j * 16, str, strlen(str));
    }
  }
  fat32_bio_flush(fs);
}

// get cluster index from a sector index
uint32_t fat32_sec2clu(struct fat32_fs *fs, uint32_t sec_index) {
  return (sec_index - fs->first_cluster_sidx) / (fs->sec_per_cluster);
}

// get sector index from a cluster index
uint32_t fat32_clu2sec(struct fat32_fs *fs, uint32_t clu_index) {
  return clu_index * fs->sec_per_cluster + fs->first_cluster_sidx;
}

static const uint32_t FAT32_TABLE_FLAG = 0x0ffffff8;

uint32_t fat32_search_avail_chain(struct fat32_fs *fs, uint32_t start) {
  uint32_t sidx = start / fs->chain_per_sector;
  uint32_t offset_in_sector = start % fs->chain_per_sector;
  uint32_t *table = fs->buf;
  while(sidx < fs->sec_per_table) {
    fs->disk->ops.read_op(fs->disk, sidx + fs->dsidx , fs->buf);
    for(uint32_t i = offset_in_sector; i < fs->chain_per_sector; ++i) {
      if(table[i] < FAT32_TABLE_FLAG) {
        return sidx * fs->chain_per_sector + i;
      }
    }
    offset_in_sector = 0;
  }
  // failed
  return 0;
}

// set an entry in chain
void fat32_set_chain(struct fat32_fs *fs, uint32_t index , uint32_t val) {
  uint32_t sidx = index / fs->chain_per_sector;
  uint32_t offset_in_sector = index % fs->chain_per_sector;
  fs->disk->ops.read_op(fs->disk, sidx + fs->dsidx, fs->buf);
  uint32_t *table = fs->buf;
  table[offset_in_sector] = val;
  fs->disk->ops.write_op(fs->disk, sidx + fs->dsidx, fs->buf);
}

// set the chain automatically, return next chain pos
uint32_t fat32_set_chain_auto(struct fat32_fs *fs, uint32_t cur) {
  uint32_t nxt_aval = fat32_search_avail_chain(fs, cur);
  fat32_set_chain(fs, cur, nxt_aval);
  fat32_set_chain(fs, nxt_aval, FAT32_TABLE_FLAG);
  return nxt_aval;
}

// get the next chain
uint32_t fat32_get_chain(struct fat32_fs *fs, uint32_t index) {
  uint32_t sidx = index / fs->chain_per_sector;
  uint32_t offset_in_sector = index % fs->chain_per_sector;
  fs->disk->ops.read_op(fs->disk, sidx + fs->dsidx, fs->buf);
  uint32_t *table = fs->buf;
  return table[offset_in_sector];
}

// search enough space in directory entry
// num is entry we want
// return the cluster index
uint32_t fat32_search_in_dir_entry(struct fat32_fs *fs, struct fat32_obj *dir, uint32_t num) {
}



struct fat32_fs *fat32_init(struct disk_hal *disk, uint32_t start_sector, uint32_t total_sector) {
  struct fat32_fs *fs = kmalloc(sizeof(struct fat32_fs));
  if(!fs) {
    printf("fat32, kmalloc failed\n");
    return NULL;
  }

  fs->dsidx = start_sector;
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

  fs->sec_per_cluster = bpb->SecPerClus;
  fs->reserved_sec_cnt = bpb->RsvdSecCnt;

  if(bpb->jmpBoot[0] != 0xeb || bpb->jmpBoot[2] != 90) {
    printf("fat32, jmpBoot mismatch\n");
  }

  if(bpb->HiddSec != start_sector) {
    fs->dsidx = bpb->HiddSec;
    printf("fat32, HiddSec != start_sector\n");
  }

  if(bpb->ToSec32 != total_sector) {
    fs->total_sector = bpb->ToSec32;
    // if total_sector is zero, we totallly depends on data read from disk
    if(total_sector) {
      printf("fat32, ToSec32 != total_sector\n");
    }
  }

  fs->sec_per_table = bpb->FATSz32;
  fs->table_num = bpb->NumFATs;
  fs->root_cidx = bpb->RootClus;


  fs->table_sidx = fs->reserved_sec_cnt;
  fs->first_cluster_sidx = fs->sec_per_table * fs->table_num + fs->reserved_sec_cnt;

  //fs->byte_per_sector = bpb->BytePerSec;
  fs->byte_per_sector = 512;
  fs->chain_per_sector = bpb->BytePerSec / sizeof(uint32_t);

  fs->sec_per_buf = POWER_OF_2(0) * PAGE_SIZE / fs->byte_per_sector;

  fs->buf_activity = kmalloc(sizeof(uint32_t) * fs->sec_per_buf * 2);
  if(!fs->buf_activity) {
    printf("fat32, buf_activity alloc failed\n");
    pmem_free(fs->buf);
    kfree(fs);
    return NULL;
  }
  
  fs->buf_cidx = fs->buf_activity + fs->sec_per_buf;

  fs->buf_flags = kmalloc(sizeof(uint8_t) * fs->sec_per_buf);
  if(!fs->buf_flags) {
    printf("fat32, buf_flags alloc failed\n");
    pmem_free(fs->buf);
    kfree(fs->buf_activity);
    kfree(fs);
    return NULL;
  }

  for(int i = 0; i < fs->sec_per_buf; ++i) {
    fs->buf_flags[i] = FAT32_BIO_FLAG_INVALID;
    fs->buf_cidx[i] = 0;
    fs->buf_activity[i] = 0;
  }


  return fs;
}

void fat32_destory(struct fat32_fs *fs) {
  pmem_free(fs->buf);
  kfree(fs);
}


void fat32_get_root_dir(struct fat32_fs *fs, struct fat32_obj *obj) {
  obj->cidx = fs->root_cidx;
  obj->type = FAT32_OBJ_DIRECTORY;
}

bool fat32_is_file(struct fat32_fs *fs, struct fat32_obj *obj) {
  return obj->type == FAT32_OBJ_FILE;
}

bool fat32_is_directory(struct fat32_fs *fs, struct fat32_obj *obj) {
  return obj->type == FAT32_OBJ_DIRECTORY;
}



