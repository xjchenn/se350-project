#include "process.h"
#include "memory.h"
#include "printf.h"
#include "utils.h"
#include "linkedlist.h"

linkedlist_t** ready_pqs;
linkedlist_t** mem_blocked_pqs;
pcb_t** pcbs;

int k_init_processor(void) {
    return 0;
}

int k_set_process_priority(int process_id, int priority) {
    return 0;
}

int k_get_process_priority(int process_id) {
    return 0;
}
