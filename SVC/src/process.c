#include "k_process.h"
#include "process.h"
#include "k_memory.h"
#include "printf.h"
#include "utils.h"
#include "linkedlist.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "k_proc.h"
#include "memory.h"
#include <LPC17xx.h>
#include <system_LPC17xx.h>

node_t** pcb_nodes;

pcb_t** pcbs;
node_t* current_pcb_node = NULL;
linkedlist_t** ready_pqs;
linkedlist_t** mem_blocked_pqs;
linkedlist_t** msg_blocked_pqs;

extern proc_image_t k_proc_table[NUM_K_PROCESSES];
extern proc_image_t usr_proc_table[NUM_USR_PROCESSES];

// whether to continue running the process after UART interrupt
// the UART handler is responsbile for setting this var
// - 1 to switch (calls release_processor)
// - 0 to restore current process
uint32_t g_switch_flag = 0;

uint32_t k_pcb_msg_unblock(pcb_t* pcb) {
    node_t* pcb_node;
    
    pcb->state = READY;
    pcb_node = linkedlist_remove(msg_blocked_pqs[pcb->priority], pcb);
    linkedlist_push_back(ready_pqs[pcb->priority], pcb_node);
    
    return 0;
}

/**
 * Checks to see if there is a higher priority process to preempt (with the exception) of the kernel process
 * @return      1 if need to preempt, else 0
 */
uint32_t k_should_preempt_current_process(void) {
    uint32_t i = 0;
    pcb_t* current_pcb = (pcb_t*)current_pcb_node->value;

    for (i = 0; i <= current_pcb->priority; i++) {
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

    for (i = 0; i < NUM_USR_PROCESSES; ++i) {
        stack_ptr = k_alloc_stack(STACK_SIZE);
        *(--stack_ptr) = XPSR;

        switch (i) {
            case 0:
                // null process
                *(--stack_ptr) = (uint32_t)(k_proc_table[0].proc_start);
                pcbs[i]->pid = k_proc_table[0].pid;
                pcbs[i]->priority = k_proc_table[0].priority;
                break;

            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                // user process
                *(--stack_ptr) = (uint32_t)(usr_proc_table[i - 1].proc_start);
                pcbs[i]->pid = usr_proc_table[i - 1].pid;
                pcbs[i]->priority = usr_proc_table[i - 1].priority;
                break;
            
            default:
                // TODO
                break;
        }

        // add padding to stack?
        for (j = 0; j < 6; j++) {
            *(--stack_ptr) = NULL;
        }

        pcbs[i]->stack_ptr = stack_ptr;
        pcbs[i]->state = NEW;
        linkedlist_init(&pcbs[i]->msg_queue);
        pcb_nodes[i]->value = (void*)pcbs[i];
        linkedlist_push_back(ready_pqs[pcbs[i]->priority], pcb_nodes[i]);
    }
    
    current_pcb_node = NULL;

    return 0;
}

int is_blocked_state(PROCESS_STATE state) {
    return (state == MEM_BLOCKED || state == MSG_BLOCKED);
}

/**
 * Gets the next process that should be dispatched
 * @return      The next process to be dispatched (pcb_t*)
 */
node_t* get_next_process(void) {
    uint32_t i;
    
    node_t* unblocked_pcb_node;
    pcb_t* unblocked_pcb;

    node_t* ret_pcb_node;
    pcb_t* current_pcb = (pcb_t*)current_pcb_node->value;
    
    int loop_max = (is_blocked_state(current_pcb->state) ? NUM_PRIORITIES : (current_pcb->priority + 1));
   
    for (i = 0; i < loop_max; i++) {
        if (blocks_allocated < MAX_MEM_BLOCKS && mem_blocked_pqs[i]->first != NULL) {
            unblocked_pcb_node = linkedlist_pop_front(mem_blocked_pqs[i]);
            unblocked_pcb = (pcb_t*)unblocked_pcb_node->value;
            unblocked_pcb->state = READY;
            linkedlist_push_back(ready_pqs[i], unblocked_pcb_node);
        }

        if (ready_pqs[i]->first != NULL) {
            ret_pcb_node = linkedlist_pop_front(ready_pqs[i]);
            
            if (current_pcb->state == RUNNING) {
                current_pcb->state = READY;
            }

            switch(current_pcb->state) {
                case MEM_BLOCKED:
                    linkedlist_push_back(mem_blocked_pqs[current_pcb->priority], current_pcb_node);
                    break;
                case MSG_BLOCKED:
                    linkedlist_push_back(msg_blocked_pqs[current_pcb->priority], current_pcb_node);
                case READY:
                    linkedlist_push_back(ready_pqs[current_pcb->priority], current_pcb_node);
                    break;
                default:
                    break;
            }
                  
            return ret_pcb_node;
        }
    }

    return current_pcb_node;
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
            old_pcb->stack_ptr = (uint32_t*)__get_MSP();
        }
        
        current_pcb->state = RUNNING;
        __set_MSP((uint32_t)current_pcb->stack_ptr);
        __rte();
    } else if (current_pcb != old_pcb) {
        if (current_state == READY || is_blocked_state(current_state)) {
            old_pcb->stack_ptr = (uint32_t*)__get_MSP();
            current_pcb->state = RUNNING;
            __set_MSP((uint32_t) current_pcb->stack_ptr);
        } else {
            current_pcb = old_pcb;
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
    uint32_t i;
    
    pcb_t* to_change_pcb;
    pcb_t* current_pcb = (pcb_t*)current_pcb_node->value;
    node_t* to_change_pcb_node = NULL;
    uint32_t old_priority;

    if (process_id < 1 || process_id >= NUM_PROCESSES || new_priority < 0 || new_priority >= (NUM_PRIORITIES - 1)) {
        return -1;
    }

    to_change_pcb = pcbs[process_id];
    old_priority = to_change_pcb->priority;

    if (old_priority == new_priority) {
        return 0;
    }

    to_change_pcb->priority = new_priority;

    if (to_change_pcb == current_pcb) {
        // just changed self priority
        // check if there's another ready process with higher priority
        for (i = 0; i <= new_priority; i++) {
            if (ready_pqs[i]->length != 0) {
                k_release_processor();
                break;
            }
        }
        return 0;
    } 
        // just changed another's priority
    to_change_pcb_node = (node_t*)linkedlist_remove(ready_pqs[old_priority], to_change_pcb);

    if (to_change_pcb_node == NULL) {
        to_change_pcb_node = (node_t*)linkedlist_remove(mem_blocked_pqs[old_priority], to_change_pcb);

        if (to_change_pcb_node == NULL) {
            to_change_pcb_node = (node_t*)linkedlist_remove(msg_blocked_pqs[old_priority], to_change_pcb);
            
            linkedlist_push_back(msg_blocked_pqs[new_priority], to_change_pcb_node);
        } else {
            linkedlist_push_back(mem_blocked_pqs[new_priority], to_change_pcb_node);
        }
    } else {
        linkedlist_push_back(ready_pqs[new_priority], to_change_pcb_node);

        if (new_priority <= current_pcb->priority) {
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

void k_print_queues(linkedlist_t** queues) {
    uint32_t i;
    linkedlist_t* queue_itr;
    node_t* pcb_node_itr;
    pcb_t* curr_pcb_itr;

    for (i = 0; i < NUM_PRIORITIES; i++) {
        queue_itr = queues[i];
        pcb_node_itr = queue_itr->first;
        while (pcb_node_itr != NULL) {
            curr_pcb_itr = (pcb_t*) pcb_node_itr->value;
            println("PID:%d Priority:%d SP:%x", curr_pcb_itr->pid, curr_pcb_itr->priority, curr_pcb_itr->stack_ptr);
            pcb_node_itr = pcb_node_itr->next;
        }
    }
}
