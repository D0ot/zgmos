#include "uart_hal.h"


#ifdef QEMU
#include "../driver//ns16550.h"

struct ns16550 *ns16550_uart;

void uart_hal_init() {
  ns16550_uart = ns16550_init(NS16550_BASE_ADDR);
}

void uart_hal_send(char ch) {
  ns16550_send(ns16550_uart, ch);
}

char uart_hal_recv() {
  return ns16550_recv(ns16550_uart);
}

void uart_hal_putchar(char ch) {
  uart_hal_send(ch);
}

#endif 

#ifdef K210

#include "kendryte/uarths.h"

void uart_hal_init() {
}

void uart_hal_send(char ch) {
  if(ch == '\n') {
    uarths_putchar('\r');
  }
  uarths_putchar(ch);
}

char uart_hal_recv() {
  return uarths_getchar();
}

void uart_hal_putchar(char ch) {
  uart_hal_send(ch);
}

#endif
