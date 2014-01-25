/* @brief: rtx.h User API prototype, this is an example only
 * @author: Yiqing Huang
 * @date: 2014/01/07
 */
#include "utils.h"

#ifndef RTX_H_
#define RTX_H_

#define __SVC_0  __svc_indirect(0)

extern int k_release_processor(void);
#define release_processor() _release_processor((uint32_t)k_release_processor)
extern int __SVC_0 _release_processor(uint32_t p_func);

typedef struct {
    uint32_t pid;
    uint32_t stack_size;
    uint32_t priority;
    func_ptr_t proc_start;
} proc_image_t;

#endif // !RTX_H_
