#ifndef __UART_HAL_H_
#define __UART_HAL_H_


void uart_hal_init();
void uart_hal_send(char ch);
char uart_hal_recv();

#endif // __UART_HAL_H_
