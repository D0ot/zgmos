#include "fat32.h"
#include "kmem.h"
#include "list.h"
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
uint32_t fat32_bio_match_buffered(struct fat32_fs *fs, uint32_t sidx) {
  for(int i = 0; i < fs->buf_num; ++i) {
    if(sidx == fs->buf_sidx[i] && fs->buf_flags[i] != FAT32_BIO_FLAG_INVALID) {
      return i;
    }
  }
  return FAT32_BUF_INDEX_INVALID;
}

uint32_t fat32_bio_match_unbuffered(struct fat32_fs *fs) {
  for(int i = 0; i < fs->buf_num; ++i) {
    if(fs->buf_flags[i] == FAT32_BIO_FLAG_INVALID) {
      return i;
    }
  }
  return FAT32_BUF_INDEX_INVALID;
}

// return a lowest activity buf
// it will be called only when there are no available buffer
uint32_t fat32_bio_get_low_activity(struct fat32_fs *fs) {
  return fs->buf_seq.next - fs->buf_lists;
}

void fat32_bio_set_activity(struct fat32_fs *fs, uint32_t bbidx) {
  list_del(fs->buf_lists + bbidx);
  list_add_tail(fs->buf_lists + bbidx, &fs->buf_seq);
}

// no check, just writeback
void fat32_bio_writeback_no_check(struct fat32_fs *fs, uint32_t bidx) {
  fs->disk->ops.write_op(fs->disk, fs->buf_sidx[bidx] + fs->dsidx, fat32_bio_bidx2addr(fs, bidx));
  fs->buf_flags[bidx] = FAT32_BIO_FLAG_CLEAN;
}

// remove the selected buf from buf array
// write back may occur
void fat32_bio_remove(struct fat32_fs *fs, uint32_t bidx) {
  // printf("fat32_bio, sector %d at buf %d removed\n", fs->buf_sidx[bidx], bidx);
  if(fs->buf_flags[bidx] == FAT32_BIO_FLAG_DIRTY) {
    fat32_bio_writeback_no_check(fs, bidx);
  }
  fs->buf_flags[bidx] = FAT32_BIO_FLAG_INVALID;
  list_del(fs->buf_lists + bidx);
}

uint32_t fat32_bio_fetch(struct fat32_fs *fs, uint32_t sidx) {
  uint32_t bidx = fat32_bio_match_unbuffered(fs);
  if(bidx == FAT32_BUF_INDEX_INVALID) {
    bidx = fat32_bio_get_low_activity(fs);
    fat32_bio_remove(fs, bidx);
  }
  fs->disk->ops.read_op(fs->disk, sidx + fs->dsidx, fat32_bio_bidx2addr(fs, bidx));
  fs->buf_flags[bidx] = FAT32_BIO_FLAG_CLEAN;
  fs->buf_sidx[bidx] = sidx;
  list_add_tail(fs->buf_lists + bidx, &fs->buf_seq);
  // printf("fat32_bio, sector %d at buf %d add\n", sidx, bidx);
  return bidx;
}

void fat32_bio_flush(struct fat32_fs *fs) {
  for(int i = 0; i < fs->buf_num; ++i) {
    if(fs->buf_flags[i] == FAT32_BIO_FLAG_DIRTY) {
      fat32_bio_writeback_no_check(fs, i);
    }
  }
}

uint32_t fat32_bio_access(struct fat32_fs *fs, uint32_t cidx) {
  uint32_t bidx = fat32_bio_match_buffered(fs, cidx);

  if(bidx == FAT32_BUF_INDEX_INVALID) {
    // no buffer of cidx found, do real IO
    bidx = fat32_bio_fetch(fs, cidx);
  }
  fat32_bio_set_activity(fs, bidx);
  return bidx;
}

void *fat32_bio_read(struct fat32_fs *fs, uint32_t sidx) {
  uint32_t bidx = fat32_bio_access(fs, sidx);
  return fat32_bio_bidx2addr(fs, bidx);
}


void *fat32_bio_write(struct fat32_fs *fs, uint32_t sidx) {
  uint32_t bidx = fat32_bio_access(fs, sidx);
  fs->buf_flags[bidx] = FAT32_BIO_FLAG_DIRTY;
  return fat32_bio_bidx2addr(fs, bidx);
}


uint32_t fat32_bio_read_copy(struct fat32_fs *fs, uint32_t cidx, uint32_t offset, void *buf, uint32_t buf_len) {
  uint32_t bidx = fat32_bio_match_buffered(fs, cidx);
  uint32_t len = min(fs->byte_per_sector - offset, buf_len);

  if(bidx == FAT32_BUF_INDEX_INVALID) {
    // no buffer of cidx found, do real IO
    bidx = fat32_bio_fetch(fs, cidx);
  }
  memcpy(buf, fat32_bio_bidx2addr(fs, bidx) + offset, len);
  fat32_bio_set_activity(fs, bidx);
  return len;
}

uint32_t fat32_bio_write_copy(struct fat32_fs *fs, uint32_t cidx, uint32_t offset, void *buf, uint32_t buf_len) {
  uint32_t bidx = fat32_bio_match_buffered(fs, cidx);
  uint32_t len = min(fs->byte_per_sector - offset, buf_len);

  if(bidx == FAT32_BUF_INDEX_INVALID) {
    // no buffer of cidx found, do real IO
    bidx = fat32_bio_fetch(fs, cidx);
  }

  memcpy(fat32_bio_bidx2addr(fs, bidx) + offset, buf, len);
  fs->buf_flags[bidx] = FAT32_BIO_FLAG_DIRTY;
  fat32_bio_set_activity(fs, bidx);
  return len;
}

void fat32_bio_test(struct fat32_fs *fs) {
  uint8_t buf[512];
  char str[] = "fat32_bio_test";
  fat32_bio_read_copy(fs, 0, 0, buf, 512);

  for(int i = 0; i < 20; ++i) {
    fat32_bio_read_copy(fs, i, 0, buf, 512);
    for(int j = 0; j < 20; ++j) {
      fat32_bio_write_copy(fs, i, j * 16, str, strlen(str));
    }
  }
  fat32_bio_flush(fs);
}

// get cluster index from a sector index
uint32_t fat32_sec2clu(struct fat32_fs *fs, uint32_t sidx) {
  return (sidx - fs->first_cluster_sidx) / (fs->sec_per_cluster) + 2;
}

// get sector index from a cluster index
uint32_t fat32_clu2sec(struct fat32_fs *fs, uint32_t cidx) {
  return (cidx - 2) * fs->sec_per_cluster + fs->first_cluster_sidx;
}

static const uint32_t FAT32_CHAIN_END_FLAG = 0x0ffffff8;

uint32_t fat32_search_avail_chain(struct fat32_fs *fs, uint32_t start) {
  uint32_t sidx = start / fs->chain_per_sector + fs->reserved_sec_cnt;
  uint32_t offset_in_sector = start % fs->chain_per_sector;
  uint32_t *table = NULL;
  while(sidx < fs->sec_per_table) {
    table = fat32_bio_read(fs, sidx);
    for(uint32_t i = offset_in_sector; i < fs->chain_per_sector; ++i) {
      if(table[i] < FAT32_CHAIN_END_FLAG) {
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
  uint32_t sidx = index / fs->chain_per_sector + fs->reserved_sec_cnt;
  uint32_t offset_in_sector = index % fs->chain_per_sector;
  uint32_t *table = fat32_bio_write(fs, sidx);
  table[offset_in_sector] = val;
}

// set the chain automatically, return next chain pos
uint32_t fat32_set_chain_auto(struct fat32_fs *fs, uint32_t cur) {
  uint32_t nxt_aval = fat32_search_avail_chain(fs, cur);
  fat32_set_chain(fs, cur, nxt_aval);
  fat32_set_chain(fs, nxt_aval, FAT32_CHAIN_END_FLAG);
  return nxt_aval;
}

// get the next chain
uint32_t fat32_get_chain(struct fat32_fs *fs, uint32_t index) {
  uint32_t sidx = index / fs->chain_per_sector + fs->reserved_sec_cnt;
  uint32_t offset_in_sector = index % fs->chain_per_sector;
  uint32_t *table = fat32_bio_read(fs, sidx);
  return table[offset_in_sector];
}

static const uint8_t FAT32_DIRNAME_FREE = 0xe5;
static const uint8_t FAT32_DIRNAME_RES_FREE = 0x00;

struct fat32_dir_entry_pos {
  uint32_t entry_cidx;

  // entry index offset in cluster
  uint32_t entry_offset;
};

// search enough space in directory entry
// num is amount of entries we want
// return true when success,false when fail
bool fat32_search_in_dir_entry(struct fat32_fs *fs, struct fat32_obj *dir, uint32_t num, struct fat32_dir_entry_pos *pos) {
  uint32_t cidx = dir->cidx;
  uint32_t avail_entry_cidx;

  // entry index offset in cluster
  uint32_t avail_entry_offset = 0;
  uint32_t avail_entry_cnt = 0;
  uint32_t cidx_cnt = 0;
  bool last_avail = false;
  while(cidx < FAT32_CHAIN_END_FLAG) {
    uint32_t sidx = fat32_clu2sec(fs, cidx);
    
    for(int sidx_offset = 0; sidx_offset < fs->sec_per_cluster; ++sidx_offset) {
      uint8_t *dat = fat32_bio_read(fs, sidx + sidx_offset);
      for(int i = 0; i < fs->byte_per_sector; i = i + 32) {
        if(dat[i] == FAT32_DIRNAME_FREE || dat[i] == FAT32_DIRNAME_RES_FREE) {
          if(!last_avail) {
            avail_entry_offset = ((sidx_offset * fs->byte_per_sector) + i) / 32;
            avail_entry_cnt = 1;
            avail_entry_cidx = cidx;
          }else {
            avail_entry_cnt++;
          }

          if(avail_entry_cnt == num) {
            pos->entry_cidx = avail_entry_cidx;
            pos->entry_offset = avail_entry_offset;
            return true;
          }
          last_avail = true;
        } else {
          last_avail = false;
        }
      }
    }
    cidx_cnt++;
    cidx = fat32_get_chain(fs, cidx);
  }
  return false;
}



// caller must make sure buf is more than (3 + 8 + 1 = 12) byte long
// return count of uint8_t
uint32_t fat32_copy_name_short(struct fat32_dir_entry *short_dir, uint8_t *buf) {
  int i = 0;
  int j = 0;
  for(; i < 8 && short_dir->Name[i] != 0x20; ++i) {
    buf[j++] = short_dir->Name[i];
  }

  buf[j++] = '.';

  i = 8;

  for(; i < 11 && short_dir->Name[i] != 0x20; ++i) {
    buf[j++] = short_dir->Name[i];
  }

  buf[j++] = 0;

  return j;
}

// caller must make sure buf is more than (13 + 1 = 14) byte long
// return count of uint16_t
uint32_t fat32_copy_name_long(struct fat32_long_name_entry *long_dir, uint8_t *buf) {
  int i = 0;
  int j = 0;

  for(; i < 5 ; ++i) {
    if(long_dir->Name1[i] == 0xffff) {
      return j;
    }
    buf[j++] = long_dir->Name1[i];
  }

  i = 0;
  for(; i < 6 ; ++i) {
    if(long_dir->Name2[i] == 0xffff) {
      return j;
    }
    buf[j++] = long_dir->Name2[i];
  }
  
  i = 0;
  for(; i < 2 ; ++i) {
    if(long_dir->Name3[i] == 0xffff) {
      return j;
    }
    buf[j++] = long_dir->Name3[i];
  }
  buf[j++] = 0;
  return j;
}


void fat32_iter_start(struct fat32_fs *fs, struct fat32_obj *parent, struct fat32_directory_iter *iter) {
  iter->cur_byte_offset = 0;
  iter->cur_sidx_offset = 0;
  iter->cur_cidx = parent->cidx;
}


bool fat32_iter_next(struct fat32_fs *fs, struct fat32_directory_iter *iter, struct fat32_obj *obj) {
  uint32_t cidx = iter->cur_cidx;
  uint32_t sidx_offset_init = iter->cur_sidx_offset;
  uint32_t byte_offset_init = iter->cur_byte_offset;


  uint8_t fns[FAT32_LONG_FN_STACK_NUM][FAT32_LONG_FN_LEN + 1];
  int32_t fn_num = FAT32_LONG_FN_STACK_NUM;
  

  // long filename flag
  bool flag = false;

  while(cidx < FAT32_CHAIN_END_FLAG) {
    uint32_t sidx = fat32_clu2sec(fs, cidx);
    

    for(int sidx_offset = sidx_offset_init; sidx_offset < fs->sec_per_cluster; ++sidx_offset) {
      uint8_t *dat = fat32_bio_read(fs, sidx + sidx_offset);

      for(int i = byte_offset_init; i < fs->byte_per_sector; i = i + 32) {
        
        if(dat[i] == FAT32_DIRNAME_FREE) {
          continue;
        }

        if(dat[i] == FAT32_DIRNAME_RES_FREE) {
          // the can not rely the semetic in the fat32 spec
          // if we adhere to fat32 spec, we shoud "break" here.
          // we did not, because on linux, delete a file or directory is just mark the first byte of directory_entry as 0xe5;
          continue;
        }


        if((dat[i + 11] & FAT32_ATTR_LONG_NAME_MASK) == FAT32_ATTR_LONG_NAME) {
          // long name sub compoent
          if(!flag) {
            flag = true;
            fn_num = FAT32_LONG_FN_STACK_NUM;
          }
          fn_num--;
          fat32_copy_name_long((struct fat32_long_name_entry*)(dat + i), fns[fn_num]);

        }else {
          if(dat[i] != FAT32_DIRNAME_FREE && dat[i] != FAT32_DIRNAME_RES_FREE) {
            if((dat[i + 11] & (FAT32_ATTR_VOLUME_ID | FAT32_ATTR_DIRECTORY)) == 0x00) {
              // found a file

              struct fat32_dir_entry *entry = (struct fat32_dir_entry*)(dat + i);
              
              obj->cidx = ((uint32_t)entry->FstClusHI << 16) + entry->FstClusLO;
              obj->file_size = entry->FileSize;
              
              if(flag) {
                char *buf = obj->long_fn;
                for(int j = fn_num; j < FAT32_LONG_FN_STACK_NUM; ++j) {
                  buf = strcpy_end(buf, (void*)fns[j]);
                }
              }else {
                obj->long_fn[0] = 0;
              }

              fat32_copy_name_short(entry, (uint8_t*)obj->short_fn);

             
              
              obj->type = FAT32_OBJ_FILE;
              iter->cur_byte_offset = (i + 32) % fs->byte_per_sector;
              iter->cur_sidx_offset = sidx_offset + ( (i + 32) / fs->byte_per_sector ? 1 : 0);
              iter->cur_cidx = cidx + ( (iter->cur_sidx_offset / fs->sec_per_cluster) ? 1 : 0);
              
              if(iter->cur_sidx_offset / fs->sec_per_cluster) {
                iter->cur_cidx = fat32_get_chain(fs, cidx);
                iter->cur_sidx_offset = 0;
              }
              return true;
            } else if((dat[i+11] & (FAT32_ATTR_VOLUME_ID | FAT32_ATTR_DIRECTORY)) == FAT32_ATTR_DIRECTORY) {
              // found a directory

              struct fat32_dir_entry *entry = (struct fat32_dir_entry*)(dat + i);
              obj->cidx = ((uint32_t)entry->FstClusHI << 16) + entry->FstClusLO;

              if(flag) {
                char *buf = obj->long_fn;
                for(int j = fn_num; j < FAT32_LONG_FN_STACK_NUM; ++j) {
                  buf = strcpy_end(buf, (void*)fns[j]);
                }
              }else {
                obj->long_fn[0] = 0;
              }

              fat32_copy_name_short(entry, (uint8_t*)obj->short_fn);

              obj->type = FAT32_OBJ_DIRECTORY;
              iter->cur_byte_offset = (i + 32) % fs->byte_per_sector;
              iter->cur_sidx_offset = sidx_offset + ( (i + 32) / fs->byte_per_sector ? 1 : 0);
              iter->cur_cidx = cidx + ( (iter->cur_sidx_offset / fs->sec_per_cluster) ? 1 : 0);
              iter->cur_sidx_offset = iter->cur_sidx_offset % fs->sec_per_cluster;

              return true;
            } else if((dat[i+11] & (FAT32_ATTR_VOLUME_ID | FAT32_ATTR_DIRECTORY)) == FAT32_ATTR_VOLUME_ID) {
              // found a volume label

            } else {
              // other unknown things
            }
            flag = false;
          }
        }
      }
      byte_offset_init = 0;
    }
    sidx_offset_init = 0;
    cidx = fat32_get_chain(fs, cidx);
  }
  return false;
}

bool fat32_find_in_dir(struct fat32_fs *fs, struct fat32_obj *parent, char *fn, struct fat32_obj *obj) {
  struct fat32_directory_iter iter;
  fat32_iter_start(fs, parent, &iter);
  while(fat32_iter_next(fs, &iter, obj)) {
    if(strcmp(fn, obj->long_fn) == 0) {
      return true;
    }
  }
  return false;
}

bool fat32_get(struct fat32_fs *fs, struct fat32_obj *parent, char *path, struct fat32_obj *obj) {
  uint32_t s = 0;
  uint32_t e = 0;
  char tmp;
  
  while(path[s]) {
    while(path[e] && path[e] != '/') {
      ++e;
    }
    
    tmp = path[e];
    path[e] = 0;
    bool ret = fat32_find_in_dir(fs, parent, path + s, obj);
    path[e] = tmp;
    
    if(ret) {
      if(path[e] == '\0') {
        return true;
      }
    }else {
      break;
    }

    if(tmp == '\0') {
      break;
    }else {
      s = ++e;
    }

    if(!fat32_is_directory(fs, obj)) {
      break;
    }else {
      parent = obj;
    }

  }
  return false;
}


bool fat32_mkdir(struct fat32_fs *fs, struct fat32_obj *parent_dir, char *name) {
  while(1);
}

bool fat32_create_file(struct fat32_fs *fs, struct fat32_obj *parent_dir, char *name) {
  while(1);
}

uint32_t fat32_get_file_size(struct fat32_obj *obj) {
  return obj->file_size;
}

char *fat32_get_obj_name(struct fat32_obj *obj) {
  return obj->long_fn;
}

uint32_t fat32_read(struct fat32_fs *fs, struct fat32_obj *obj, void *buf, uint64_t buf_len, uint64_t seek) {
  if(seek >= obj->file_size) {
    return 0;
  }

  uint64_t byte_offset_init = seek % fs->byte_per_sector;
  uint32_t sidx_offset_init = (seek / fs->byte_per_sector);
  uint32_t cidx_cnt = sidx_offset_init / fs->sec_per_cluster;
  sidx_offset_init = sidx_offset_init % fs->sec_per_cluster;
  
  uint64_t byte_cnt = 0;

  uint32_t cidx = obj->cidx;
  while(cidx_cnt--) {
    cidx = fat32_get_chain(fs, cidx);
  }

  while(cidx < FAT32_CHAIN_END_FLAG) {
    uint32_t sidx_base = fat32_clu2sec(fs, cidx);

    for(uint32_t sidx_offset = sidx_offset_init; sidx_offset < fs->sec_per_cluster; ++sidx_offset) {
      void *dat = fat32_bio_read(fs, sidx_base + sidx_offset);
      uint64_t len = min(fs->byte_per_sector - byte_offset_init, buf_len - byte_cnt);
      memcpy(buf + byte_cnt, dat + byte_offset_init, len);
      byte_cnt += len;
      if(byte_cnt == buf_len) {
        return buf_len;
      }
    }
    sidx_offset_init = 0;
    byte_offset_init = 0;
    cidx = fat32_get_chain(fs, cidx);
  }
  return byte_cnt;
}


void fat32_flush(struct fat32_fs *fs) {
  fat32_bio_flush(fs);
}


struct fat32_fs *fat32_init(struct disk_hal *disk, uint32_t start_sector, uint32_t total_sector, uint8_t buf_order) {
  struct fat32_fs *fs = kmalloc(sizeof(struct fat32_fs));
  if(!fs) {
    printf("fat32, kmalloc failed\n");
    return NULL;
  }

  fs->dsidx = start_sector;
  fs->buf = pmem_alloc(buf_order);

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

  if(bpb->jmpBoot[0] != 0xeb || bpb->jmpBoot[2] != 0x90) {
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

  // fs->byte_per_sector = 512;
  
  // in fact, it is also 512
  fs->byte_per_sector = bpb->BytePerSec;
  

  fs->chain_per_sector = bpb->BytePerSec / sizeof(uint32_t);

  fs->buf_num = POWER_OF_2(buf_order) * PAGE_SIZE / fs->byte_per_sector;

  list_init(&fs->buf_seq);

  fs->cluster_num = (fs->byte_per_sector * fs->sec_per_table) / sizeof(uint32_t) - 2;

  fs->buf_sidx = kmalloc(sizeof(uint32_t) * fs->buf_num);
  if(!fs->buf_sidx) {
    printf("fat32, buf_sidx alloc failed\n");
    pmem_free(fs->buf);
    kfree(fs);
    return NULL;
  }

  fs->buf_flags = kmalloc(sizeof(uint8_t) * fs->buf_num);
  if(!fs->buf_flags) {
    printf("fat32, buf_flags alloc failed\n");
    pmem_free(fs->buf);
    kfree(fs->buf_sidx);
    kfree(fs);
    return NULL;
  }

  fs->buf_lists= kmalloc(sizeof(struct list_head) * fs->buf_num);
  if(!fs->buf_lists) {
    printf("fat32, buf_head_ptrs alloc failed\n");
    pmem_free(fs->buf);
    kfree(fs->buf_flags);
    kfree(fs->buf_sidx);
    kfree(fs);
    return NULL;
  }

  for(int i = 0; i < fs->buf_num; ++i) {
    fs->buf_flags[i] = FAT32_BIO_FLAG_INVALID;
    fs->buf_sidx[i] = 0;
    list_init(fs->buf_lists + i);
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
  obj->short_fn[0] = 0;
  obj->long_fn[0] = 0;
}

bool fat32_is_file(struct fat32_fs *fs, struct fat32_obj *obj) {
  return obj->type == FAT32_OBJ_FILE;
}

bool fat32_is_directory(struct fat32_fs *fs, struct fat32_obj *obj) {
  return obj->type == FAT32_OBJ_DIRECTORY;
}


void fat32_test(struct fat32_fs *fs) {
  struct fat32_dir_entry_pos pos;
  struct fat32_obj root;
  fat32_get_root_dir(fs, &root);
  fat32_search_in_dir_entry(fs, &root, 3, &pos);
  struct fat32_directory_iter iter;
  fat32_iter_start(fs, &root, &iter);
  struct fat32_obj obj;

  uint32_t cks1;
  char *pa = pmem_alloc(3); // 32K
  while(fat32_iter_next(fs, &iter, &obj)) {
    printf(obj.long_fn);
    printf(":");
    if(fat32_is_file(fs, &obj)) {
      fat32_read(fs, &obj, pa, PAGE_SIZE * POWER_OF_2(3), 0);
      cks1 = util_sum((uint8_t*)pa, obj.file_size);
      printf("%d\n", cks1);
    }
  }

  char *pa2 = pmem_alloc(3);
  fat32_iter_start(fs, &root, &iter);
  while(fat32_iter_next(fs, &iter, &obj)) {
    printf(obj.long_fn);
    printf(":");
    uint32_t fsz = obj.file_size;
    uint32_t offset = 0;

    while(fat32_read(fs, &obj, pa2 + offset, 1, offset)){
      offset += 1;
    }
    cks1 = util_sum((uint8_t*)pa2, obj.file_size);
    printf("%d\n", cks1);
  }

  char *pa3 = pmem_alloc(3);
  fat32_iter_start(fs, &root, &iter);
  while(fat32_iter_next(fs, &iter, &obj)) {
    printf(obj.long_fn);
    printf(":");
    uint32_t fsz = obj.file_size;
    uint32_t offset = 0;

    while(fat32_read(fs, &obj, pa3 + offset, 333, offset)){
      offset += 333;
    }
    cks1 = util_sum((uint8_t*)pa3, obj.file_size);
    printf("%d\n", cks1);
  }

  fat32_get(fs, &root, "nice/bro/file.c", &obj);
  uint32_t sz = fat32_read(fs, &obj, pa3, 20, 0);
  pa3[sz] = 0;
  puts(pa3);
  


  while(1);
}

