#ifndef _K_RTX_MEMORY_H_
#define _K_RTX_MEMORY_H_

#include "utils.h"

typedef struct mem_blk {
    struct mem_blk* next;
    struct mem_blk* prev;
    void* data;
    uint32_t padding;
} mem_blk_t;

typedef struct mem_table_entry {
    int32_t owner_pid;
    mem_blk_t* blk;
} mem_table_entry_t;

void* k_request_memory_block(void);
uint32_t k_release_memory_block(void*);
uint32_t k_init_memory_blocks(void);
uint32_t* k_alloc_stack(uint32_t size);

#endif
