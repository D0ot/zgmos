#ifndef __VIRTIO_H_
#define __VIRTIO_H_

#include <stdint.h>
#include <assert.h>

#define RWV32(x) (*(volatile uint32_t*)(&x))

static void *const VIRTIO_BLK_MMIO_BASE = (void*)0x10001000;

// when using __attribute__((packed)), data read from it is all zero
// seems a bug?
// problem found : when using packed... gcc will read it byte by byte...
struct __attribute__((packed)) virtio_regs {
  // 0x000
  uint32_t magic_value; 

  // 0x004
  uint32_t version; 

  // 0x008
  uint32_t device_id;

  // 0x00c
  uint32_t vendor_id;

  // 0x010
  uint32_t device_features;

  // 0x014
  uint32_t device_features_sel;

  uint32_t __padding1[2];

  // 0x020
  uint32_t driver_features;

  // 0x024
  uint32_t driver_features_sel;

  uint32_t __padding2[2];

  // 0x030
  uint32_t queue_sel;

  // 0x034
  uint32_t queue_num_max;

  // 0x038
  uint32_t queue_num;

  uint32_t __padding3[2];

  // 0x044
  uint32_t queue_ready;

  uint32_t __padding4[2];

  // 0x050
  uint32_t queue_notify;

  uint32_t __padding5[3];
  
  // 0x060
  uint32_t interrupt_status;

  // 0x064
  uint32_t interrupt_ack;

  uint32_t __padding6[2];

  // 0x070
  uint32_t status;

  uint32_t __padding7[3];

  // 0x080
  uint32_t queue_desc_low;

  // 0x084
  uint32_t queue_desc_high;

  uint32_t __padding8[2];

  // 0x090
  uint32_t queue_driver_low;
  
  // 0x094
  uint32_t queue_driver_high;

  uint32_t __padding9[2];

  // 0x0a0
  uint32_t queue_device_low;
  // 0x0a4
  uint32_t queue_device_high;

  uint32_t __padding10[21];

  // 0x0fc
  uint32_t config_generation;

  // 0x100+
  uint32_t config[0];
};

// because we have not used attribute packed
static_assert(sizeof(struct virtio_regs) == 0x100, "sizeof(virtio_regs) != 0x100 )");

#define VIRTIO_OFFSET_DEF(n, o) \
  static const uint64_t VIRTIO_OFFSET_##n = (o);

VIRTIO_OFFSET_DEF(MAGIC_VALUE, 0x000);
VIRTIO_OFFSET_DEF(VERSION, 0x004);
VIRTIO_OFFSET_DEF(DEVICE_ID, 0x008);
VIRTIO_OFFSET_DEF(VENDOR_ID, 0x00c);
VIRTIO_OFFSET_DEF(DEVICE_FEATURES, 0x010);
VIRTIO_OFFSET_DEF(DEVICE_FEATURES_SEL, 0x014);
VIRTIO_OFFSET_DEF(DRIVER_FEATURES, 0x020);
VIRTIO_OFFSET_DEF(DRIVER_FEATURES_SEL, 0x024);
VIRTIO_OFFSET_DEF(QUEUE_SEL, 0x030);
VIRTIO_OFFSET_DEF(QUEUE_NUM_MAX, 0x034);
VIRTIO_OFFSET_DEF(QUEUE_NUM, 0x038);
VIRTIO_OFFSET_DEF(QUEUE_READY, 0x044);
VIRTIO_OFFSET_DEF(QUEUE_NOTIFY, 0x050);
VIRTIO_OFFSET_DEF(INTERRUPT_STATUS, 0x60);
VIRTIO_OFFSET_DEF(INTERRUPT_ACK, 0x064);
VIRTIO_OFFSET_DEF(STATUS, 0x070);
VIRTIO_OFFSET_DEF(QUEUE_DESC_LOW, 0x080);
VIRTIO_OFFSET_DEF(QUEUE_DESC_HIGH, 0x084);
VIRTIO_OFFSET_DEF(QUEUE_DRIVER_LOW, 0x090);
VIRTIO_OFFSET_DEF(QUEUE_DRIVER_HIGH, 0x094);
VIRTIO_OFFSET_DEF(QUEUE_DEVICE_LOW, 0x0a0);
VIRTIO_OFFSET_DEF(QUEUE_DEVICE_HIGH, 0x0a4);
VIRTIO_OFFSET_DEF(CONFIG_GENERATION, 0x0fc);
VIRTIO_OFFSET_DEF(CONFIG, 0x100);


static const uint32_t VIRTIO_MAGIC    = 0x74726976;
static const uint32_t VIRTIO_VERSION  = 0x2;
static const uint32_t VIRTIO_DEV_NET  = 0x1;
static const uint32_t VIRTIO_DEV_BLK  = 0x2;
static const uint32_t VIRTIO_VENDOR   = 0x554D4551;

static const uint32_t VIRTIO_STATUS_ACKNOWLEDGE       = 1;
static const uint32_t VIRTIO_STATUS_DRIVER            = 2;
static const uint32_t VIRTIO_STATUS_FAILED            = 128;
static const uint32_t VIRTIO_STATUS_FEATURES_OK       = 8;
static const uint32_t VIRTIO_STATUS_DRIVER_OK         = 4;
static const uint32_t VIRTIO_STATUS_DEVICE_NEEDS_RESET= 64;


void virtio_dev_init(volatile struct virtio_regs *regs);

#endif
