#ifndef __FAT32_H_
#define __FAT32_H_

#include <stdint.h>
#include <stdbool.h>
#include "../hal/disk_hal.h"
#include "list.h"


// abstruction file system operation

struct mbr_part_entry {
  uint8_t boot_flags;
  uint8_t chs_begin[3];
  uint8_t type_code;
  uint8_t chs_end[3];
  uint32_t lba_begin;
  uint32_t num_of_sec;
};


struct mbr_info {
  struct mbr_part_entry part_entries[3];
  uint16_t magic;
};

void get_mbr_info(struct mbr_info, uint8_t *sector);





// Boot sector and BPB structure
// the member nameing conforms to fat spec
struct fat32_bpb {
  // eb 58 90 fix
  uint8_t   jmpBoot[3];

  // a string, any is valid
  char      OEMName[8];

  // always 512
  uint16_t  BytePerSec;

  // Sectors per Cluster
  uint8_t   SecPerClus;

  // number of reserved sectors in
  // the reserved region of the volume
  uint16_t  RsvdSecCnt;

  // number of FAT
  uint8_t   NumFATs;

  // for FAT32, must be zero
  uint16_t  RootEntCnt;

  // when zero, ToSec32 must not be zero
  uint16_t  ToSec16;

  // fixed, 0xf8
  uint8_t   Media;

  // zero
  uint16_t  FATSz16;

  // do not care
  uint16_t  SecPerTrk;

  // do not care
  uint16_t  NumHeads;

  // sector before this fat partition, is same with lab_begin in MBR
  uint32_t  HiddSec;

  // total sector number
  uint32_t  ToSec32;

  // at offset 36
  // one FAT size
  uint32_t  FATSz32;
  // can be zero, we do not care
  uint16_t  ExtFlags;
  // fs version
  uint16_t  FSVer;
  // root directory cluster
  uint32_t  RootClus;

  // file system info 
  uint16_t  FSInfo;

  // backup boot sector 
  uint16_t  BkBootSec;
  uint8_t   Reserved[12];

  // os specific
  uint8_t   DrvNum;
  uint8_t   Reserved1;

  // 0x29
  uint8_t   BootSig;

  // volume series number
  uint32_t  VolID;

  // volume lable
  uint8_t   VolLab[11];

  // file system ascii, "FAT32   "
  char      FilSysType[8];

} __attribute__((packed));

struct fat32_fsinfo {
  // 0x41615252
  uint32_t LeadSig;

  uint8_t Reserved1[480];

  // 0x61412727
  uint32_t StrucSig;

  // the last known free cluster count on the volume
  uint32_t Free_Count;

  // cluster number at which the driver should start looking for free cluster
  uint32_t Nxt_Free;

  uint8_t Reserved2[12];

  uint32_t TrailSig;
} __attribute__((packed));

static const uint8_t FAT32_ATTR_READ_ONLY = 0x01;
static const uint8_t FAT32_ATTR_HIDDEN = 0x02;
static const uint8_t FAT32_ATTR_SYSTEM = 0x04;
static const uint8_t FAT32_ATTR_VOLUME_ID = 0x08;
static const uint8_t FAT32_ATTR_DIRECTORY = 0x10;
static const uint8_t FAT32_ATTR_ARCHIVE = 0x20;
static const uint8_t FAT32_ATTR_LONG_NAME = FAT32_ATTR_READ_ONLY |
                                            FAT32_ATTR_HIDDEN |
                                            FAT32_ATTR_SYSTEM |
                                            FAT32_ATTR_VOLUME_ID;

static const uint8_t FAT32_ATTR_LONG_NAME_MASK = FAT32_ATTR_LONG_NAME | FAT32_ATTR_DIRECTORY | FAT32_ATTR_ARCHIVE;

struct fat32_dir_entry {
  char Name[11];
  uint8_t Attr;
  // reserved for NT
  uint8_t NTRes;
  uint8_t CrtTimeTenth;
  uint16_t CrtTime;
  uint16_t CrtDate;
  uint16_t LstAccDate;
  // entry first cluster high
  uint16_t FstClusHI;
  uint16_t WrtTime;
  uint16_t WrtDate;

  // entry first cluster low 
  uint16_t FstClusLO;

  // file size in bytes
  uint32_t FileSize;
} __attribute__((packed));


static const uint8_t FAT32_LAST_LONG_ENTRY = 0x40;

struct fat32_long_name_entry{
  uint8_t Ord;
  uint16_t Name1[5];
  uint8_t Attr;
  uint8_t Type;
  uint8_t Chksum;
  uint16_t Name2[6];
  uint16_t FstClusLO;
  uint16_t Name3[2];
}__attribute__((packed));


static const uint8_t FAT32_BIO_FLAG_INVALID = 0;
static const uint8_t FAT32_BIO_FLAG_CLEAN = 1;
static const uint8_t FAT32_BIO_FLAG_DIRTY = 2;


// dsidx means disk sector index with MBR as 0
// sidx means sector index with volume boot sector as 0
// cidx means cluster index with first data cluster as zero

struct fat32_fs {

  // which would pass to ops
  struct disk_hal *disk;

  // fat32 volume start position, same with HiddSec
  uint32_t dsidx;

  // fat32 volume sector count
  uint32_t total_sector;

  // sector size 
  uint32_t byte_per_sector;

  // chain per sector
  uint32_t chain_per_sector;

  uint32_t sec_per_cluster;

  // reserved sector count
  uint32_t reserved_sec_cnt;

  // one table size in sector
  uint32_t sec_per_table;

  // number of file alloc table
  uint32_t table_num;

  // root directory cluster number
  uint32_t root_cidx;

  // internal buffer
  //[buf_start_sidx, buf_end_sidx);
  void *buf;
  uint32_t buf_num;
  //uint32_t *buf_activity;
  uint8_t *buf_flags;
  uint32_t *buf_sidx;
  // buffer access sequence
  struct list_head buf_seq;
  struct list_head *buf_lists;

  // FOLLOWING IS CALCULATED PARAMS

  // first file alloc table sector number
  uint32_t table_sidx;


  // first data cluster sector number
  uint32_t first_cluster_sidx;

  // cluster number
  uint32_t cluster_num;

  // for FAT32 FSInfo sector
  uint32_t free_count;
  uint32_t nxt_free;
};

static const uint32_t FAT32_OBJ_FILE = 1;
static const uint32_t FAT32_OBJ_DIRECTORY = 2;



// a long filename entry can store 13 char
#define FAT32_LONG_FN_LEN  (13)

// a short filename entry can store 11 char
#define FAT32_SHORT_FN_LEN  (11)

// buffer size
#define FAT32_LONG_FN_STACK_NUM  (10)

struct fat32_obj{
  // cluster index in File Alloc Table
  uint32_t cidx;
  // file or directory
  uint32_t type;

  uint32_t table_index;

  char long_fn[FAT32_LONG_FN_LEN * FAT32_LONG_FN_STACK_NUM + 1];
  char short_fn[FAT32_SHORT_FN_LEN + 1];

  // for directory, it is undefined
  uint32_t file_size;
};

// cur == current
struct fat32_directory_iter{
  uint32_t cur_cidx;
  uint32_t cur_sidx_offset;
  uint32_t cur_byte_offset;
};

// File system ops
struct fat32_fs *fat32_init(struct disk_hal *disk, uint32_t start_sector, uint32_t total_sector, uint8_t buf_order);
void fat32_destory(struct fat32_fs *fs);

// get the obj of root directory
void fat32_get_root_dir(struct fat32_fs *fs, struct fat32_obj *obj);

// check if obj is a file
bool fat32_is_file(struct fat32_fs *fs, struct fat32_obj *obj);

// check if obj is a directory
bool fat32_is_directory(struct fat32_fs *fs, struct fat32_obj *obj);



// start a iteration through a directory
void fat32_iter_start(struct fat32_fs *fs, struct fat32_obj *parent, struct fat32_directory_iter *iter);

// get next obj in directory
// return false when there are no next 
bool fat32_iter_next(struct fat32_fs *fs, struct fat32_directory_iter *iter, struct fat32_obj *obj);

// just file name, not path
bool fat32_find_in_dir(struct fat32_fs *fs, struct fat32_obj *parent, char *fn, struct fat32_obj *obj);

// path is the full path, no leading '/' and no trailing '/'
bool fat32_get(struct fat32_fs *fs, struct fat32_obj *parent, char *path, struct fat32_obj *obj);

// name is the directory name
bool fat32_mkdir(struct fat32_fs *fs, struct fat32_obj *parent_dir, char *name);

// create a new file
bool fat32_create_file(struct fat32_fs *fs, struct fat32_obj *parent_dir, char *name);

// get the file size
uint32_t fat32_get_file_size(struct fat32_obj *obj);

// get the file name
char *fat32_get_obj_name(struct fat32_obj *obj);

// read the file content;
// return the real byte read
// buf_len max is 4096
uint32_t fat32_read(struct fat32_fs *fs, struct fat32_obj *obj, void *buf, uint64_t buf_len, uint64_t seek);


// write the file content;
// return the real byte written
// buf_len max is 4096
uint32_t fat32_write(struct fat32_fs *fs, struct fat32_obj *obj, void *buf, uint64_t buf_len, uint64_t seek);

void fat32_flush(struct fat32_fs *fs);


// bio test
void fat32_bio_test(struct fat32_fs *fs);

// fat32 fs test
void fat32_test(struct fat32_fs *fs);
#endif // __FAT32_H_
