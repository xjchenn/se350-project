/**
 * @brief: uart.h
 * @author: Yiqing Huang
 * @date: 2014/02/08
 */

#ifndef UART_IRQ_H_
#define UART_IRQ_H_

#include <stdint.h>
#include "uart_def.h"

#define uart0_irq_init() uart_irq_init(0)
#define uart1_irq_init() uart_irq_init(1)

int uart_irq_init(int n_uart); /* initialize the n_uart to use interrupt */

void irq_i_process(void);

void read_interrupt(void);

#endif /* ! UART_IRQ_H_ */
