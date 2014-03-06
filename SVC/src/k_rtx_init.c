#include <LPC17xx.h>
#include "rtx_init.h"
#include "uart.h"
#include "uart_polling.h"
#include "k_memory.h"
#include "k_process.h"
#include "printf.h"
#include "timer.h"

void k_rtx_init(void) {
    __disable_irq();
    init_printf(NULL, putc);
		uart_irq_init(0);       // uart0, interrupt-driven 
		uart1_init();           // uart1, polling
    k_init_memory_blocks();
    k_init_processor();
    timer_init(0);
    __enable_irq();

    k_release_processor();
}
