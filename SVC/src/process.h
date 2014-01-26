#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "utils.h"

#define __SVC_0  __svc_indirect(0)

typedef enum {
    HIGHEST = 0,
    HIGH,
    MEDIUM,
    LOW,
    LOWEST
} PROCESS_PRIORITY;

typedef struct proc_image {
    uint32_t pid;
    uint32_t stack_size;
    PROCESS_PRIORITY priority;
    func_ptr_t proc_start;
} proc_image_t;

extern uint32_t k_release_processor(void);
#define release_processor() _release_processor((uint32_t)k_release_processor)
extern uint32_t __SVC_0 _release_processor(uint32_t p_func);

extern uint32_t k_set_process_priority(uint32_t, uint32_t);
#define set_process_priority() _release_processor((uint32_t)k_set_process_priority, process_id, priority)
extern uint32_t __SVC_0 _set_process_priority(uint32_t p_func, uint32_t process_id, uint32_t priority);

extern uint32_t k_get_process_priority(uint32_t);
#define get_process_priority() _release_processor((uint32_t)k_get_process_priority, process_id)
extern uint32_t __SVC_0 _get_process_priority(uint32_t p_func, uint32_t process_id);

#endif
