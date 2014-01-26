#include "process.h"
#include "k_memory.h"
#include "printf.h"
#include "utils.h"
#include "linkedlist.h"
#include "uart_polling.h"
#include <LPC17xx.h>
#include <system_LPC17xx.h>
// #include "usr_proc.h"

linkedlist_t** ready_pqs;
linkedlist_t** mem_blocked_pqs;

pcb_t** pcbs;
pcb_t* current_pcb = NULL;

void null_proc(){
    while (1) {

        //if (i % 10000 == 0) {
            printf("PROCESSING NULL\r\n");
        //}
        //i++;
        k_release_processor();
    }
}

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
    }

    if (current_pcb != old_pcb) {
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

    //null_proc();

    return 0;
}

int k_init_processor(void) {
    int32_t i = 0;
    uint32_t stack_size = 0x100;
    uint32_t* stack_ptr;
    pcbs[0]->pid = 0;
    pcbs[0]->priority = LOWEST;
    stack_ptr = k_alloc_stack(stack_size);
    *(--stack_ptr) = XPSR;
    *(--stack_ptr) = (uint32_t)(&null_proc);

    for (i = 0; i < 6; i++) {
        *(--stack_ptr) = NULL;
    }

    pcbs[0]->stack_ptr = stack_ptr;
    pcbs[0]->state = NEW;
    current_pcb = pcbs[0];
		null_proc();
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
