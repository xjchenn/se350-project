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
#include "uart_def.h"
#include "rtx.h"

int main() {
    SystemInit();  /* initialize the system */
    rtx_init();

    // transit to unprivileged level, default MSP is used
    __set_CONTROL(__get_CONTROL() | BIT(0));

    return 0;
}
