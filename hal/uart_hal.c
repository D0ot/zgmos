#include "uart_hal.h"


#ifdef QEMU
#include "../driver//ns16550.h"

struct ns16550 *ns16550_uart;

void uart_init() {
  ns16550_uart = ns16550_init(NS16550_BASE_ADDR);
}

void uart_send(char ch) {
  ns16550_send(ns16550_uart, ch);
}

char uart_recv() {
  return ns16550_recv(ns16550_uart);
}

void uart_putchar(char ch) {
  ns16550_send(ns16550_uart, ch);
}

#endif 


#ifdef K210

#include "../kernel/sbi.h"

void uart_init() {
}

void uart_send(char ch) {
  if(ch == '\n') {
    sbi_console_putchar('\r');
  }
  sbi_console_putchar(ch);
}

char uart_recv() {
  return sbi_console_getchar();
}

void uart_putchar(char ch) {
  uart_send(ch);
}


#endif
