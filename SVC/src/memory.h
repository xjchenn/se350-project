#ifndef MEMORY_H_
#define MEMORY_H_

#include "utils.h"

#define __SVC_0  __svc_indirect(0)

extern uint32_t blocks_allocated;

extern void* k_request_memory_block(void);
#define request_memory_block() _request_memory_block((void*)k_request_memory_block)
extern void* _request_memory_block(void* p_func) __SVC_0;

extern int32_t k_release_memory_block(void*);
#define release_memory_block(p_mem_blk) _release_memory_block((int32_t)k_release_memory_block, p_mem_blk)
extern int32_t _release_memory_block(int32_t p_func, void* p_mem_blk) __SVC_0;

#endif
