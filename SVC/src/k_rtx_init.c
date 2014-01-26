#include "rtx_init.h"
#include "uart_polling.h"
#include "k_memory.h"
#include "process.h"
#include "printf.h"

void k_rtx_init(void) {
    __disable_irq();
    uart0_init();
    init_printf(NULL, putc);
		k_init_memory_blocks();
    k_init_processor();
    __enable_irq();
  
    k_release_processor();
}
