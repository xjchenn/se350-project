#include "memory.h"
#include "printf.h"
#include "utils.h"

/*
  This symbol is defined in the scatter file,
  refer to RVCT Linker User Guide
*/
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;

typedef struct mem_blk {
    struct mem_blk *next;
    struct mem_blk *prev;
} mem_blk_t;

mem_blk_t* k_get_next_memory_block(mem_blk_t* blk);

unsigned int end_addr = (unsigned int) &Image$$RW_IRAM1$$ZI$$Limit;

unsigned int heap_start;

mem_blk_t* free_mem;
mem_blk_t* alloc_mem;

int k_init_memory_blocks(void) {
    unsigned int i;

    heap_start = end_addr + MEM_OFFSET_SIZE;
	free_mem = (mem_blk_t *)heap_start;
	alloc_mem = NULL;

    //Zero out all of the memory we have to avoid garbage
    for(i = (unsigned int)free_mem; i < end_of_mem; i++) {
        *((int *)free_mem) = 0;
    }

	return 0;
}

void* k_request_memory_block(void) {
	mem_blk_t* ret;

    ret = free_mem;
    free_mem = k_get_next_memory_block(free_mem);
    free_mem->prev = NULL;

    ret->next = alloc_mem;

    if (alloc_mem != NULL) {
        alloc_mem->prev = ret;
    }
    alloc_mem = ret;

    printf("k_request_memory_block: image ends at 0x%x\n", end_addr);

    printf("Allocated: %x\n", (int)ret);

    return (void *)ret;
}


//Returns the next memory block available after blk
mem_blk_t* k_get_next_memory_block(mem_blk_t *block) {
	mem_blk_t* ret;

    ret = block->next;

    if (ret != NULL) {
        return ret;
    } else {
        ret = block + MEM_BLOCK_SIZE;

        if (ret < (mem_blk_t *)end_of_mem) {
            printf("Next memory block is: %x\n", ret);
            return ret;
        }

        return (mem_blk_t *)NULL;
    }
}

int k_release_memory_block(void* p_mem_blk) {
    mem_blk_t *to_del = (mem_blk_t *)p_mem_blk;

    if (p_mem_blk == NULL || (unsigned int)p_mem_blk < heap_start || (unsigned int)p_mem_blk > end_of_mem) {
        return 1;
    }

    if (to_del->prev != NULL) {
        to_del->prev->next = to_del->next;
    }

    if (to_del->next != NULL) {
        to_del->next->prev = to_del->prev;
    }

    to_del->next = free_mem;
    to_del->prev = NULL;
    free_mem = to_del;

    //printf("Deleted: %x\n", (int)free_mem);

    printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);

    return 0;
}
