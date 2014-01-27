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
pcb_t kernel_pcb;
pcb_t* stashed_pcb;

extern proc_image_t proc_table[7];

void swap_pcb_to_kernel_pcb(void) {
    stashed_pcb = current_pcb;
    current_pcb = &kernel_pcb;
}

void restore_current_pcb(void) {
    current_pcb = stashed_pcb;
}

void k_linkedlist_push_back(linkedlist_t* list, void* value) {
    swap_pcb_to_kernel_pcb();
    linkedlist_push_back(list, value);
    restore_current_pcb();
}

void* k_linkedlist_pop_front(linkedlist_t* list) {
    void* ret;
    swap_pcb_to_kernel_pcb();
    ret = linkedlist_pop_front(list);
    restore_current_pcb();
    return ret;
}

void* k_linkedlist_remove(linkedlist_t* list, void* value) {
    void* ret = 0;
    swap_pcb_to_kernel_pcb();
    ret = linkedlist_remove(list, value);
    restore_current_pcb();
    return ret;
}

pcb_t* get_next_process(void) {
    uint32_t i;

    for (i = 0; i < NUM_PRIORITIES; i++) {
        if (blocks_allocated < MAX_MEM_BLOCKS && mem_blocked_pqs[i]->first != NULL) {
            return (pcb_t*) k_linkedlist_pop_front(mem_blocked_pqs[i]);
        }

        if(ready_pqs[i]->first != NULL) {
            return (pcb_t*) k_linkedlist_pop_front(ready_pqs[i]);
        }
    }

    return (pcb_t*)NULL;
}

uint32_t switch_process(pcb_t *old_pcb) {
    PROCESS_STATE current_state = current_pcb->state;
    
    if (current_state == NEW) {
        if (current_pcb != old_pcb && old_pcb->state != NEW) {
            if(old_pcb->state != BLOCKED) {
                old_pcb->state = READY;
            }
            old_pcb->stack_ptr = (uint32_t *)__get_MSP();

            if(old_pcb->state == READY) {
                k_linkedlist_push_back(ready_pqs[old_pcb->priority], old_pcb);
            } else {
                k_linkedlist_push_back(mem_blocked_pqs[old_pcb->priority], old_pcb);
            }
        }
        current_pcb->state = RUNNING;
        __set_MSP((uint32_t)current_pcb->stack_ptr);
        __rte();
    } else if (current_pcb != old_pcb) {
        if (current_state == READY || current_state == BLOCKED) {
            if(old_pcb->state != BLOCKED) {
                old_pcb->state = READY;
            }
                            
            old_pcb->stack_ptr = (uint32_t *)__get_MSP();
                
            if(old_pcb->state == READY) {
                k_linkedlist_push_back(ready_pqs[old_pcb->priority], old_pcb);
            } else {
                k_linkedlist_push_back(mem_blocked_pqs[old_pcb->priority], old_pcb);
            }

            current_pcb->state = RUNNING;
            __set_MSP((uint32_t) current_pcb->stack_ptr);
        } else {
            current_pcb = old_pcb;
            return 1;
        }
    }
    return 0;
}

uint32_t k_init_processor(void) {
    uint32_t i;
    uint32_t j;
    uint32_t* stack_ptr;

    set_procs();
    kernel_pcb.pid = KERNEL_MEM_BLOCK_PID;

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
        k_linkedlist_push_back(ready_pqs[pcbs[i]->priority], pcbs[i]);
    }
    return 0;
}

uint32_t k_release_processor(void) {
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

int32_t k_set_process_priority(int32_t process_id, int32_t priority) {
    pcb_t* to_change;
    pcb_t* to_find = NULL;
    uint32_t old_priority;
    
    if (process_id < 0 || process_id >= NUM_PROCESSES || priority < 0 || priority >= NUM_PRIORITIES) {
        return -1;
    }
    
    to_change = pcbs[process_id];
    old_priority = to_change->priority;
    
    if(old_priority == priority) {
        return 0;
    }
    
    to_change->priority = priority;
    
    if(to_change == current_pcb) {
        return 0;
    }
    
    to_find = (pcb_t *)k_linkedlist_remove(ready_pqs[old_priority], to_change);
    
    if(to_find == NULL) {
        to_find = (pcb_t *)k_linkedlist_remove(mem_blocked_pqs[old_priority], to_change);
        
        if (to_find == NULL) {
            return 1;
        }
        
        k_linkedlist_push_back(mem_blocked_pqs[priority], to_change);
    } else {
        k_linkedlist_push_back(ready_pqs[priority], to_change);
    }
        
    return 0;
}

int32_t k_get_process_priority(int32_t process_id) {
    if (process_id < 0 || process_id >= NUM_PROCESSES) {
        return -1;
    }
    
    return pcbs[process_id]->priority;
}
