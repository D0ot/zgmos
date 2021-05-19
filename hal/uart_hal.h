#ifndef __UART_HAL_H_
#define __UART_HAL_H_


void uart_init();
void uart_send(char ch);
char uart_recv();

#endif // __UART_HAL_H_
