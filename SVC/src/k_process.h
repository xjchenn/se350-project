#ifndef _K_PROCESS_H_
#define _K_PROCESS_H_

#include "utils.h"
#include "linkedlist.h"

typedef enum {
    NEW = 0,
    READY,
    RUNNING,
    MEM_BLOCKED,
    MSG_BLOCKED,
    EXIT
} PROCESS_STATE;

typedef struct {
    uint32_t pid;
    PROCESS_STATE state;
    uint32_t priority;
    uint32_t* stack_ptr;
    linkedlist_t msg_queue;
} pcb_t;

// globals in process.c that need to be allocated by memory.c
extern node_t** pcb_nodes;
extern pcb_t** pcbs;
extern node_t* current_pcb_node;
extern linkedlist_t** ready_pqs;
extern linkedlist_t** mem_blocked_pqs;
extern linkedlist_t** msg_blocked_pqs;

// pop off exception stack frame from the stack
extern void __rte(void);

uint32_t k_pcb_msg_unblock(pcb_t*);
uint32_t k_should_preempt_current_process(void);
uint32_t k_init_processor(void);
uint32_t k_release_processor(void);
int32_t k_set_process_priority(int32_t process_id, int32_t new_priority);
int32_t k_get_process_priority(int32_t process_id);

#endif
