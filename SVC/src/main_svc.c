/**
 * @brief:  main_svc.c, demonstrate svc as a gateway to os functions
 * @author: Yiqing Huang
 * @date:   2014/01/09
 * NOTE: Standard C library is not allowed in the final kernel code.
 *       A tiny printf function for embedded application development
 *       taken from http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
 *       and is configured to use UART0 to output.
 *
 */

#include <LPC17xx.h>
#include "printf.h"
#include "uart_polling.h"
#include "rtx.h"
#include "memory.h"

int main() {
  volatile unsigned int ret_val = 1234;

  SystemInit();  /* initialize the system */
  __disable_irq();
  uart0_init();
  init_printf(NULL, putc);
  __enable_irq();

  // transit to unprivileged level, default MSP is used
  __set_CONTROL(__get_CONTROL() | BIT(0));

  ret_val = init_memory_blocks();
  ret_val = release_processor();
  
  return 0;
}

void test_memory() {
  volatile unsigned int ret_val = 1234;
  void* memory1 = 0;
  void* memory2 = 0;

  memory1 = request_memory_block();
  memory2 = request_memory_block();

  *((int*) memory2) = 0xDEADBEEF;
  printf("Allocated1: %x\n", memory1);
  printf("Allocated2: %x\n", memory2);
  printf("Value1: %x\n", (*(int *)memory1));
  printf("Value2: %x\n", (*(int *)memory2));
  printf("Ret: %d\n",ret_val);

  ret_val = release_memory_block(memory1);
  ret_val = release_memory_block(memory2);
}
