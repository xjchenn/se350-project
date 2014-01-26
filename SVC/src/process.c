#include "k_process.h"
#include "process.h"
#include "k_memory.h"
#include "printf.h"
#include "utils.h"
#include "linkedlist.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include <LPC17xx.h>
#include <system_LPC17xx.h>

linkedlist_t** ready_pqs;
linkedlist_t** mem_blocked_pqs;

pcb_t** pcbs;
pcb_t* current_pcb = NULL;

extern proc_image_t proc_table[7];

pcb_t* get_next_process(void) {
    uint32_t i;

    for (i = 0; i < NUM_PRIORITIES; i++) {
        if(ready_pqs[i]->first != NULL) {
            return (pcb_t*)linkedlist_pop_front(ready_pqs[i]);
        }

        if (blocks_allocated < MAX_MEM_BLOCKS && mem_blocked_pqs[i]->first != NULL) {
            return (pcb_t*)linkedlist_pop_front(mem_blocked_pqs[i]);
        }
    }

    return (pcb_t*)NULL;
}

int switch_process(pcb_t *old_pcb) {
    PROCESS_STATE current_state = current_pcb->state;

    if (current_state == NEW) {
        if (current_pcb != old_pcb && old_pcb->state != NEW) {
            old_pcb->state = READY;
            old_pcb->stack_ptr = (uint32_t *)__get_MSP();
            linkedlist_push_back(ready_pqs[old_pcb->priority], old_pcb);
        }
        current_pcb->state = RUNNING;
        __set_MSP((uint32_t)current_pcb->stack_ptr);
        __rte();
    } else if (current_pcb != old_pcb) {
        if (current_state == READY) {
            old_pcb->state = READY;
            old_pcb->stack_ptr = (uint32_t *)__get_MSP();
            linkedlist_push_back(ready_pqs[old_pcb->priority], old_pcb);

            current_pcb->state = RUNNING;
            __set_MSP((uint32_t) current_pcb->stack_ptr);
        } else {
            current_pcb = old_pcb;
            return 1;
        }
    }

    //proc_table[current_pcb->pid].proc_start();
    return 0;
}

int k_init_processor(void) {
    uint32_t i;
    uint32_t j;
    uint32_t* stack_ptr;

    set_procs();

    for (i = 0; i < NUM_PROCESSES; ++i)
    {
        stack_ptr = k_alloc_stack(STACK_SIZE);
        *(--stack_ptr) = XPSR;
        *(--stack_ptr) = (uint32_t)(proc_table[i].proc_start);

        for (j = 0; j < 6; j++) {
            *(--stack_ptr) = NULL;
        }

        pcbs[i]->pid = proc_table[i].pid;
        pcbs[i]->priority = proc_table[i].priority;
        pcbs[i]->stack_ptr = stack_ptr;
        pcbs[i]->state = NEW;

        linkedlist_push_back(ready_pqs[pcbs[i]->priority], pcbs[i]);    
    }

    // k_release_processor();
    return 0;
}

int k_release_processor(void) {
    pcb_t* old_pcb = current_pcb;

    current_pcb = get_next_process();

    if (current_pcb == NULL) {
        current_pcb = old_pcb;
        return 1;
    }

    if (old_pcb == NULL) {
        old_pcb = current_pcb;
    }

    return switch_process(old_pcb);
}

int k_set_process_priority(int process_id, int priority) {
    return 0;
}

int k_get_process_priority(int process_id) {
    return 0;
}
