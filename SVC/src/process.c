#include "k_process.h"
#include "process.h"
#include "k_memory.h"
#include "printf.h"
#include "utils.h"
#include "linkedlist.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "k_proc.h"
#include <LPC17xx.h>
#include <system_LPC17xx.h>

node_t** pcb_nodes;

pcb_t** pcbs;
node_t* current_pcb_node = NULL;
linkedlist_t** ready_pqs;
linkedlist_t** mem_blocked_pqs;

extern proc_image_t k_proc_table[NUM_K_PROCESSES];
extern proc_image_t usr_proc_table[NUM_USR_PROCESSES];

/**
 * Checks to see if there is a higher priority process to preempt (with the exception) of the kernel process
 * @return      1 if need to preempt, else 0
 */
uint32_t k_should_preempt_current_process(void) {
    uint32_t i = 0;
    pcb_t* current_pcb = (pcb_t*)current_pcb_node->value;

    for (i = 0; i < current_pcb->priority; i++) {
        if (mem_blocked_pqs[i]->first != NULL) {
            return 1;
        }
    }

    return 0;
}

/**
 *  This initializes the process control blocks for the user and null processes
 * @return      0 if successful
 */
uint32_t k_init_processor(void) {
    uint32_t i;
    uint32_t j;
    uint32_t* stack_ptr;

    usr_set_procs();
    k_set_procs();

    for (i = 0; i < NUM_PROCESSES; ++i) {
        stack_ptr = k_alloc_stack(STACK_SIZE);
        *(--stack_ptr) = XPSR;
        // Since the NULL process is not a user process we need to have a special case to initialize
        if (i < NUM_K_PROCESSES) {
            *(--stack_ptr) = (uint32_t)(k_proc_table[i].proc_start);
            pcbs[i]->pid = k_proc_table[i].pid;
            pcbs[i]->priority = k_proc_table[i].priority;
        } else {
            // Here we initialize the user processes
            *(--stack_ptr) = (uint32_t)(usr_proc_table[i - NUM_K_PROCESSES].proc_start);
            pcbs[i]->pid = usr_proc_table[i - NUM_K_PROCESSES].pid;
            pcbs[i]->priority = usr_proc_table[i - NUM_K_PROCESSES].priority;
        }

        for (j = 0; j < 6; j++) {
            *(--stack_ptr) = NULL;
        }

        pcbs[i]->stack_ptr = stack_ptr;
        pcbs[i]->state = NEW;
        pcb_nodes[i]->value = pcbs[i];
        linkedlist_push_back(ready_pqs[pcbs[i]->priority], pcb_nodes[i]);
    }

    return 0;
}

/**
 * Gets the next process that should be dispatched
 * @return      The next process to be dispatched (pcb_t*)
 */
node_t* get_next_process(void) {
    uint32_t i;
    node_t* unblocked_pcb_node;

    for (i = 0; i < NUM_PRIORITIES; i++) {
        // check for mem blocked procs and set them to ready if we have blocks available
        if (blocks_allocated < MAX_MEM_BLOCKS && mem_blocked_pqs[i]->first != NULL) {
			unblocked_pcb_node = linkedlist_pop_front(mem_blocked_pqs[i]);
            ((pcb_t*)unblocked_pcb_node->value)->state = READY;
            linkedlist_push_back(ready_pqs[i], unblocked_pcb_node);
        }

        // get first available ready proc
        if (ready_pqs[i]->first != NULL) {
            return (node_t*) linkedlist_pop_front(ready_pqs[i]);
        }
    }

    return NULL;
}

/**
 * Scheduler (assume current_pcb_node has already been changed)
 * @param  old_pcb_node     node for old pcb to switch from 
 * @return                  0 if successful
 */
uint32_t switch_process(node_t* old_pcb_node) {
    pcb_t* old_pcb              = (pcb_t*)old_pcb_node->value;
    pcb_t* current_pcb          = (pcb_t*)current_pcb_node->value;
    PROCESS_STATE current_state = current_pcb->state;
    // If the current state is new, then we put the process given into the ready queue
    // If the process's state is not READY, we push it on to the memory blocked queue
    
    if (current_state == NEW) {
        if (current_pcb != old_pcb && old_pcb->state != NEW) {
            if (old_pcb->state != BLOCKED) {
                old_pcb->state = READY;
            }
            old_pcb->stack_ptr = (uint32_t*)__get_MSP();

            if (old_pcb->state == READY) {
                linkedlist_push_back(ready_pqs[old_pcb->priority], old_pcb_node);
            } else {
                linkedlist_push_back(mem_blocked_pqs[old_pcb->priority], old_pcb_node);
            }
        }
        current_pcb->state = RUNNING;
        __set_MSP((uint32_t)current_pcb->stack_ptr);
        __rte();
    } else if (current_pcb != old_pcb) {
        if (current_state == READY || current_state == BLOCKED) {
            if (old_pcb->state != BLOCKED) {
                old_pcb->state = READY;
            }

            old_pcb->stack_ptr = (uint32_t*)__get_MSP();

            if (old_pcb->state == READY) {
                linkedlist_push_back(ready_pqs[old_pcb->priority], old_pcb_node);
            } else {
                linkedlist_push_back(mem_blocked_pqs[old_pcb->priority], old_pcb_node);
            }

            current_pcb->state = RUNNING;
            __set_MSP((uint32_t) current_pcb->stack_ptr);
        } else {
            current_pcb_node = old_pcb_node; // (???)
            return 1;
        }
    }
    return 0;
}

/**
 * This gets the next process from the queue and then calls switch process to switch
 * @return      0 if successful
 */
uint32_t k_release_processor(void) {
    node_t* old_pcb_node = current_pcb_node;
    current_pcb_node = get_next_process();

    // if the process received from get_next_process() is NULL, 
    // keep current_pcb to what was currently running
    if (current_pcb_node == NULL) {
        current_pcb_node = old_pcb_node;
        DEBUG_PRINT("k_release_processor failed");
        return 1;
    }

    // first time calling release_processor()
    if (old_pcb_node == NULL) {
        old_pcb_node = current_pcb_node;
    }

    return switch_process(old_pcb_node);
}

/**
 * Changes the priority of a process
 * @param  process_id   The target process's id
 * @param  priority     The new priority of the target process
 * @return              Zero if successful
 */
int32_t k_set_process_priority(int32_t process_id, int32_t new_priority) {
    pcb_t* target_pcb;
    pcb_t* current_pcb = (pcb_t*)current_pcb_node->value;
    node_t* target_pcb_node = NULL;
    uint32_t old_priority;

    if (process_id < 1 || process_id >= NUM_PROCESSES || new_priority < 0 || new_priority >= (NUM_PRIORITIES - 1)) {
        return -1;
    }

    target_pcb = pcbs[process_id];
    old_priority = target_pcb->priority;

    if (old_priority == new_priority) {
        return 0;
    }

    target_pcb->priority = new_priority;

    if (target_pcb == current_pcb) {
        return 0;
    }

    target_pcb_node = (node_t*)linkedlist_remove(ready_pqs[old_priority], target_pcb);

    if (target_pcb_node == NULL) {
        target_pcb_node = (node_t*)linkedlist_remove(mem_blocked_pqs[old_priority], target_pcb);

        if (target_pcb_node == NULL) {
            return 1;
        }

        linkedlist_push_back(mem_blocked_pqs[new_priority], target_pcb_node);
    } else {
        linkedlist_push_back(ready_pqs[new_priority], target_pcb_node);

        if (new_priority < current_pcb->priority) {
            k_release_processor();
        }
    }

    return 0;
}

/**
 * Returns the priority of a process
 * @param  process_id   The garget process's id
 * @return              The priority of the target process
 */
int32_t k_get_process_priority(int32_t process_id) {
    if (process_id < 0 || process_id >= NUM_PROCESSES) {
        return -1;
    }

    return pcbs[process_id]->priority;
}
