#include "utils.h"
#include "rtx.h"

#ifndef MEMORY_H_
#define MEMORY_H_

static const unsigned int end_of_mem = 0x10008000;

extern void *k_request_memory_block(void);
#define request_memory_block() _request_memory_block((U32)k_request_memory_block)
extern void *_request_memory_block(U32 p_func) __SVC_0;
//__SVC_0 can also be put at the end of the function declaration

extern int k_release_memory_block(void *);
#define release_memory_block(p_mem_blk) _release_memory_block((U32)k_release_memory_block, p_mem_blk)
extern int _release_memory_block(U32 p_func, void *p_mem_blk) __SVC_0;

extern int k_init_memory_blocks(void);
#define init_memory_blocks() _init_memory_blocks((U32)k_init_memory_blocks);
extern int _init_memory_blocks(U32 p_func) __SVC_0;

#endif
