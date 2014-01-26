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

extern int k_release_processor(void);
#define release_processor() _release_processor((uint32_t)k_release_processor)
extern int __SVC_0 _release_processor(uint32_t p_func);

extern int k_set_process_priority(int, int);
#define set_process_priority() _release_processor((uint32_t)k_set_process_priority, process_id, priority)
extern int __SVC_0 _set_process_priority(uint32_t p_func, int process_id, int priority);

extern int k_get_process_priority(int);
#define get_process_priority() _release_processor((uint32_t)k_get_process_priority, process_id)
extern int __SVC_0 _get_process_priority(uint32_t p_func, int process_id);

#endif
