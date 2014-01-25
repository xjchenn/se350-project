#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "utils.h"
#include "linkedlist.h"

typedef enum {
    NEW = 0,
    READY,
    RUNNING,
    WAITING,
    EXITED,
} PROCESS_STATE;

typedef enum {
    HIGHEST = 0,
    HIGH,
    MEDIUM,
    LOW,
    LOWEST,
    NUM_PRIORITIES
} PROCESS_PRIORITY;

extern linkedlist_t** ready_pqs;
extern linkedlist_t** mem_blocked_pqs;

typedef struct {
    uint32_t pid;
    PROCESS_STATE state;
    PROCESS_PRIORITY priority;
    uint32_t *stack_ptr;
} pcb_t;

extern pcb_t** pcbs;

int k_init_processor(void);
int k_release_processor(void);

int k_set_process_priority(int process_id, int priority);
int k_get_process_priority(int process_id);


#endif
