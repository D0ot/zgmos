#ifndef __NS16550A_H_
#define __NS16550A_H_

#include <stdint.h>

#define NS16550_BASE_ADDR ((void*)(0x10000000))

struct ns16550 {
  // 0
  union {
    volatile uint8_t RBR;
    volatile uint8_t THR;
    volatile uint8_t DLL;
  };

  union {
    volatile uint8_t IER;
    volatile uint8_t DLM;
  };

  union {
    volatile uint8_t IIR;
    volatile uint8_t FCR;
  };

  volatile uint8_t LCR;
  volatile uint8_t MCR;
  volatile uint8_t LSR;
  volatile uint8_t MSR;
  volatile uint8_t SCR;
} __attribute__((packed));

struct ns16550* ns16550_init(void *base);
void ns16550_send(struct ns16550 *ns, char ch);
char ns16550_recv(struct ns16550 *ns);



#endif
