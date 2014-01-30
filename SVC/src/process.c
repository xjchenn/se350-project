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

linkedlist_t** ready_pqs;
linkedlist_t** mem_blocked_pqs;

pcb_t** pcbs;
pcb_t* current_pcb = NULL;
pcb_t kernel_pcb;
pcb_t* stashed_pcb;

extern proc_image_t k_proc_table[NUM_K_PROCESSES];
extern proc_image_t usr_proc_table[NUM_USR_PROCESSES];


// necessary for saving the pcb state when changing possession of memory
void swap_pcb_to_kernel_pcb(void) {
    stashed_pcb = current_pcb;
    current_pcb = &kernel_pcb;
}

// restores current pcb after kernel memory operations have been performed
void restore_current_pcb(void) {
    current_pcb = stashed_pcb;
}

/*  necessary to save the kernel state before pushing memory to the list
 *  linkedlist_t* list:
 *      the list we want to push to
 *  void* value:
 *      the value we want to push
**/
void k_linkedlist_push_back(linkedlist_t* list, void* value) {
    swap_pcb_to_kernel_pcb();
    linkedlist_push_back(list, value);
    restore_current_pcb();
}

/*  necessary to save the kernel state and return the memory block retrieved
 *  linkedlist_t* list:
 *      the list we want to pop from
 *  returns:
 *      a void* to the value we popped
**/
void* k_linkedlist_pop_front(linkedlist_t* list) {
    void* ret;
    swap_pcb_to_kernel_pcb();
    ret = linkedlist_pop_front(list);
    restore_current_pcb();
    return ret;
}

/*  necessary to save the kernel state before removing from the middle of a list
 *  linkedlist_t* list:
 *      the list we want to remove from
 *  void* value:
 *      the value we want to remove
 *  returns:
 *      a void* to the value we removed
**/
void* k_linkedlist_remove(linkedlist_t* list, void* value) {
    void* ret = 0;
    swap_pcb_to_kernel_pcb();
    ret = linkedlist_remove(list, value);
    restore_current_pcb();
    return ret;
}

/*  checks to see if there is a higher priority process to preempt (with the exception)
 *  of the kernel process
 *  returns:
 *      an int32_t which represents a boolean; 1 being true, 0 being false
**/
int32_t k_should_preempt_current_process(void) {
    uint32_t i;

    if (current_pcb == &kernel_pcb) {
        return 0;
    }

    for (i = 0; i < current_pcb->priority; i++) {
        if (mem_blocked_pqs[i]->first != NULL) {
            return 1;
        }
    }

    return 0;
}

/*  Gets the next process that should be dispatched
 *  returns:
 *      The next process to be dispatched (pcb_t*)
**/
pcb_t* get_next_process(void) {
    uint32_t i;

    for (i = 0; i < NUM_PRIORITIES; i++) {
        if (blocks_allocated < MAX_MEM_BLOCKS && mem_blocked_pqs[i]->first != NULL) {
            return (pcb_t*) k_linkedlist_pop_front(mem_blocked_pqs[i]);
        }

        if (ready_pqs[i]->first != NULL) {
            return (pcb_t*) k_linkedlist_pop_front(ready_pqs[i]);
        }
    }

    return (pcb_t*)NULL;
}

/*  The process switcher
 *  pcb_t* old_pcb:
 *      takes the old pcb to switch to the next pcb from
 *  returns:
 *      a uint32_t which represents a status code
**/
uint32_t switch_process(pcb_t* old_pcb) {
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
            if (old_pcb->state != BLOCKED) {
                old_pcb->state = READY;
            }

            old_pcb->stack_ptr = (uint32_t*)__get_MSP();

            if (old_pcb->state == READY) {
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

/*  This initializes the process control blocks for the user and null processes
 *  returns:
 *      a uint32_t which represents a status code
 *  
**/
uint32_t k_init_processor(void) {
    uint32_t i;
    uint32_t j;
    uint32_t* stack_ptr;

    usr_set_procs();
    k_set_procs();
    kernel_pcb.pid = KERNEL_MEM_BLOCK_PID;

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
        k_linkedlist_push_back(ready_pqs[pcbs[i]->priority], pcbs[i]);
    }

    return 0;
}
/*  This gets the next process from the queue and then calls switch process to switch
 *  returns:
 *      a uint32_t which represents a status code
 *
**/
uint32_t k_release_processor(void) {
    pcb_t* old_pcb = current_pcb;

    current_pcb = get_next_process();

    // If the process received from get_next_process() is NULL, keep current_pcb to what was currently running
    // return not successful return code
    if (current_pcb == NULL) {
        current_pcb = old_pcb;
        return 1;
    }

    if (old_pcb == NULL) {
        old_pcb = current_pcb;
    }

    return switch_process(old_pcb);
}

/**
 * Changes the priority of a process
 * @param  process_id   The target process's id
 * @param  priority     The new priority of the target process
 * @return              Zero if successful
 */
int32_t k_set_process_priority(int32_t process_id, int32_t priority) {
    pcb_t* to_change;
    pcb_t* to_find = NULL;
    uint32_t old_priority;

    if (process_id < 1 || process_id >= NUM_PROCESSES || priority < 0 || priority >= (NUM_PRIORITIES - 1)) {
        return -1;
    }

    to_change = pcbs[process_id];
    old_priority = to_change->priority;

    if (old_priority == priority) {
        return 0;
    }

    to_change->priority = priority;

    if (to_change == current_pcb) {
        return 0;
    }

    to_find = (pcb_t*)k_linkedlist_remove(ready_pqs[old_priority], to_change);

    if (to_find == NULL) {
        to_find = (pcb_t*)k_linkedlist_remove(mem_blocked_pqs[old_priority], to_change);

        if (to_find == NULL) {
            return 1;
        }

        k_linkedlist_push_back(mem_blocked_pqs[priority], to_change);
    } else {
        k_linkedlist_push_back(ready_pqs[priority], to_change);

        if (priority < current_pcb->priority) {
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
