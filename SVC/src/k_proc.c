#include "k_proc.h"
#include "utils.h"
#include "process.h"

proc_image_t k_proc_table[1];

void k_set_procs() {
    k_proc_table[0].pid = 0;
    k_proc_table[0].stack_size = STACK_SIZE;
    k_proc_table[0].priority = LOWEST;
    k_proc_table[0].proc_start = &null_proc;
}

void null_proc() {
    while (1) {
        release_processor();
    }
}
