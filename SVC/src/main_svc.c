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

void test_memory() {
    volatile unsigned int ret_val = 1234;
    void* memory1 = 0;
    void* memory2 = 0;

    memory1 = request_memory_block();
    memory2 = request_memory_block();

    printf("Allocated1: %x\r\n", memory1);
    printf("Allocated2: %x\r\n", memory2);
    printf("Value1: %x\r\n", (*(int *)memory1));
    printf("Value2: %x\r\n", (*(int *)memory2));

    ret_val = release_memory_block(memory1);
    printf("Ret: %d\r\n",ret_val);
    ret_val = release_memory_block(memory2);
    printf("Ret: %d\r\n",ret_val);
}

int main() {
  volatile unsigned int ret_val = 1234;

  SystemInit();  /* initialize the system */
  rtx_init();

  // transit to unprivileged level, default MSP is used
  __set_CONTROL(__get_CONTROL() | BIT(0));

  // ret_val = release_processor();
  // test_memory();

  return 0;
}
