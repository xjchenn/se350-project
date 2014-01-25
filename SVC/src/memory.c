#include "memory.h"
#include "printf.h"
#include "utils.h"

/*
  This symbol is defined in the scatter file,
  refer to RVCT Linker User Guide
*/
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;

typedef struct mem_blk {
    struct mem_blk* next;
    struct mem_blk* prev;
		void* data;
		uint32_t padding;
} mem_blk_t;

unsigned int END_OF_IMAGE = (unsigned int) &Image$$RW_IRAM1$$ZI$$Limit;

unsigned int heap_start;

mem_blk_t* free_mem;
mem_blk_t* alloc_mem;

int k_init_memory_blocks(void) {
    uint32_t i;

    heap_start = END_OF_IMAGE + MEM_OFFSET_SIZE;
		free_mem = (mem_blk_t *)heap_start;
		alloc_mem = NULL;

  //Zero out all of the memory we have to avoid garbage
    for(i = heap_start; i < END_OF_MEM - 8; i += 4) {
        *((unsigned int *)i) = INVALID_MEMORY;
    }

    free_mem->prev = NULL;
	for(i = heap_start; i < heap_start + MEM_BLOCK_SIZE * 49; i+= MEM_BLOCK_SIZE) {
		((mem_blk_t *)i)->next = (mem_blk_t *)(i + MEM_BLOCK_SIZE);
		((mem_blk_t *)i)->next->prev = (mem_blk_t *)i;
		((mem_blk_t *)i)->data = (void *)(i + MEM_BLOCK_HEADER_SIZE);
		((mem_blk_t *)i)->padding = 0xABAD1DEA;
	}

	return 0;
}

void* k_request_memory_block(void) {
	mem_blk_t* ret_blk = free_mem;
    uint32_t i = 0;

    if (ret_blk == NULL) {
        return NULL; // WE ARE RETURNING NULL HERE
    }

    free_mem = free_mem->next;

    if (free_mem != NULL) {
        free_mem->prev = NULL;
    }

    ret_blk->next = alloc_mem;
    // ret_blk->prev = NULL;

    if (alloc_mem != NULL) {
        alloc_mem->prev = ret_blk;
    }

    alloc_mem = ret_blk;

    for (i = (uint32_t)ret_blk->data; i < ((uint32_t)ret_blk + MEM_BLOCK_SIZE - 1); i += 4) {
        *((unsigned int *)i) = 0;
    }

    return (void *)ret_blk->data;
}

int k_release_memory_block(void* p_mem_blk) {
    mem_blk_t *to_del = (mem_blk_t *)((uint32_t)p_mem_blk - MEM_BLOCK_HEADER_SIZE);

    if (p_mem_blk == NULL || (uint32_t)p_mem_blk < heap_start || (uint32_t)p_mem_blk > END_OF_MEM) {
        printf("memory invalid\r\n");
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

    printf("k_release_memory_block: releasing block @ 0x%x\r\n", p_mem_blk);

    return 0;
}
