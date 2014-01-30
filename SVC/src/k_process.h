#ifndef _K_PROCESS_H_
#define _K_PROCESS_H_

#include "utils.h"
#include "linkedlist.h"

typedef enum {
    NEW = 0,
    READY,
    RUNNING,
    BLOCKED,
    EXIT
} PROCESS_STATE;

typedef struct {
    uint32_t pid;
    PROCESS_STATE state;
    uint32_t priority;
    uint32_t* stack_ptr;
} pcb_t;

extern linkedlist_t** ready_pqs;
extern linkedlist_t** mem_blocked_pqs;
extern pcb_t** pcbs;
extern pcb_t* current_pcb;

extern void __rte(void);

int32_t k_should_preempt_current_process(void);

uint32_t k_init_processor(void);
uint32_t k_release_processor(void);
int32_t k_set_process_priority(int32_t process_id, int32_t priority);
int32_t k_get_process_priority(int32_t process_id);

#endif
