#include "rtx_init.h"
#include "uart_polling.h"
#include "k_memory.h"
#include "process.h"
#include "printf.h"

void k_rtx_init(void) {
    __disable_irq();
    uart0_init();
    k_init_memory_blocks();
    init_printf(NULL, putc);
    __enable_irq();
  
    k_release_processor();
}
