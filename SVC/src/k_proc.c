#include "k_proc.h"
#include "uart.h"
#include "utils.h"
#include "process.h"

proc_image_t k_proc_table[NUM_K_PROCESSES];

void k_set_procs(void) {
    uint32_t i = 0;
    
    for (i = 0; i < NUM_K_PROCESSES; i++) {
        k_proc_table[0].stack_size = STACK_SIZE;
        k_proc_table[0].priority = LOWEST;
    }
    
    k_proc_table[0].pid = 0;
    k_proc_table[0].proc_start = &null_proc;
    
    k_proc_table[1].pid = 14;
    k_proc_table[1].proc_start = &timer_i_process;
    
    k_proc_table[2].pid = 15;
    k_proc_table[2].proc_start = &c_UART0_IRQHandler;
}

void null_proc(void) {
    while (1) {
        release_processor();
    }
}

// TODO move to own file with other message related functions
void timer_i_process(void) {
    return;
}
