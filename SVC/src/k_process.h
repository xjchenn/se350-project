#ifndef _K_PROCESS_H_
#define _K_PROCESS_H_

#include "utils.h"
#include "linkedlist.h"

typedef enum {
    NEW = 0,
    READY,
    RUNNING,
    WAITING,
    EXIT
} PROCESS_STATE;

typedef struct {
    uint32_t pid;
    PROCESS_STATE state;
    uint32_t priority;
    uint32_t *stack_ptr;
} pcb_t;

extern linkedlist_t** ready_pqs;
extern linkedlist_t** mem_blocked_pqs;
extern pcb_t** pcbs;
extern void __rte(void);

int k_init_processor(void);
int k_release_processor(void);
int k_set_process_priority(int process_id, int priority);
int k_get_process_priority(int process_id);

#endif
