#include "ns16550.h"



struct ns16550* ns16550_init(void *base) {
  struct ns16550 *ns = base;
  // disable all interrupt
  ns->IER = 0;

  // enable DLAB
  ns->LCR = 0x80;

  // set divisor
  ns->LSR = 0x03;
  ns->MSR = 0x00;

  // 8 bits no parity, one stop bit
  ns->LCR = 0x03;

  // FIFO
  ns->FCR = 0x01;
  
  return ns;
}


void ns16550_send(struct ns16550 *ns, char ch) {
  ns->THR = ch;
}

char ns16550_recv(struct ns16550 *ns) {
  return ns->RBR;
}
