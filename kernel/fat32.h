#ifndef __FAT32_H_
#define __FAT32_H_

#include <stdint.h>
#include <stdbool.h>
#include "../hal/disk_hal.h"

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
static const uint8_t FAT32_ATTR_DRECTORY = 0x10;
static const uint8_t FAT32_ATTR_ARCHIVE = 0x20;
static const uint8_t FAT32_ATTR_LONG_NAME = FAT32_ATTR_READ_ONLY |
                                            FAT32_ATTR_HIDDEN |
                                            FAT32_ATTR_SYSTEM |
                                            FAT32_ATTR_VOLUME_ID;

struct fat32_directory {
  char Name[8];
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



struct fat32_fs {

  // which would pass to ops
  struct disk_hal *disk;

  // fat32 volume start position, same with HiddSec
  uint32_t start_sector;

  // fat32 volume sector count
  uint32_t total_sector;

  uint32_t sector_per_cluster;

  // reserved sector count
  uint32_t reserved_sec_cnt;

  // ont file alloc table size
  uint32_t fat_size;

  // number of file alloc table
  uint32_t fat_num;

  // root directory cluster number
  uint32_t root_cluster;

  // internal buffer
  void *buf;

  // FOLLOWING IS CALCULATED PARAMS

  // first file alloc table sector number
  uint32_t fat_sector;


  // first data cluster sector number
  uint32_t first_cluster;

};

struct fat32_fs *fat32_init(struct disk_hal *disk, uint32_t start_sector, uint32_t total_sector);
void fat32_destory(struct fat32_fs *fs);



#endif // __FAT32_H_
