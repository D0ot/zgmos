#ifndef __VIRTIO_BLK_H_
#define __VIRTIO_BLK_H_

#include "stdint.h"
#include "stdbool.h"
#include "virtio.h"

#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1
#define VIRTIO_BLK_T_FLUSH 4
#define VIRTIO_BLK_T_DISCARD 11
#define VIRTIO_BLK_T_WRITE_ZEROES 13

#define VIRTIO_BLK_S_OK 0
#define VIRTIO_BLK_S_IOERR 1
#define VIRTIO_BLK_S_UNSUPP 2





struct virtio_blk_req {
  uint32_t type;
  uint32_t reserved;
  uint64_t sector;
  uint8_t status;
  uint32_t d1, d2, d3;
};

struct __attribute__((__packed__)) virtio_blk_config {
  uint64_t capacity;
  uint32_t size_max;
  uint32_t seg_max;
  struct virtio_blk_geometry {
    uint16_t cylinders;
    uint8_t heads;
    uint8_t sectors;
  } geometry;
  uint32_t blk_size;
  struct virtio_blk_topology {
    // # of logical blocks per physical block (log2)
    uint8_t physical_block_exp;
    // offset of first aligned logical block
    uint8_t alignment_offset;
    // suggested minimum I/O size in blocks
    uint16_t min_io_size;
    // optimal (suggested maximum) I/O size in blocks
    uint32_t opt_io_size;
  } topology;
  uint8_t writeback;
  uint8_t unused0[3];
  uint32_t max_discard_sectors;
  uint32_t max_discard_seg;
  uint32_t discard_sector_alignment;
  uint32_t max_write_zeroes_sectors;
  uint32_t max_write_zeroes_seg;
  uint8_t write_zeroes_may_unmap;
  uint8_t unused1[3];
};

static_assert(sizeof(struct virtio_blk_config) == 15 * sizeof(uint32_t), "sizeof(struct virtio_blk_config) == 15 * sizeof(uint32_t)");


struct virtio_blk {
	struct virtio_regs *regs;
	struct virtio_blk_config *config;
	struct virtio_queue *vq;
};


static const uint32_t VIRTIO_BLK_F_SIZE_MAX = (1);   // Maximum size of any single segment is in size_max.
static const uint32_t VIRTIO_BLK_F_SEG_MAX = (2);    // Maximum number of segments in a request is in seg_max.
static const uint32_t VIRTIO_BLK_F_GEOMETRY = (4);   // Disk-style geometry specified in geometry.
static const uint32_t VIRTIO_BLK_F_RO = (5);         // Device is read-only.
static const uint32_t VIRTIO_BLK_F_BLK_SIZE = (6);   // Block size of disk is in blk_size.
static const uint32_t VIRTIO_BLK_F_FLUSH = (9);      // Cache flush command support.
static const uint32_t VIRTIO_BLK_F_TOPOLOGY = (10);  // Device exports information on optimal I/O alignment.
static const uint32_t VIRTIO_BLK_F_CONFIG_WCE = (11); // Device can toggle its cache between writeback and writethrough modes.
static const uint32_t VIRTIO_BLK_F_DISCARD = (13);  // Device can support discard command, maximum discard sectors size in max_discard_sectors and maximum discard segment number in max_discard_seg.
static const uint32_t VIRTIO_BLK_F_WRITE_ZEROES = (14); /* Device can support write zeroes command, maximum write zeroes
sectors size in max_write_zeroes_sectors and maximum write zeroes segment number in max_write_-
zeroes_seg.*/



struct virtio_blk *virtio_blk_init(struct virtio_regs *regs);
void virtio_blk_set_feature(struct virtio_regs *regs);

void virtio_blk_submit(struct virtio_blk *blk, struct virtio_blk_req *req, void *buf);
bool virtio_blk_wait(struct virtio_blk *blk, struct virtio_blk_req *req);
void virtio_blk_send(struct virtio_blk *blk, uint32_t desc);
 

#endif // __VIRTIO_BLK_MMIO_H_

