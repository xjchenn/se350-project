#include "k_memory.h"
#include "printf.h"
#include "utils.h"
#include "linkedlist.h"
#include "process.h"

/*
  This symbol is defined in the scatter file,
  refer to RVCT Linker User Guide
*/
extern uint32_t Image$$RW_IRAM1$$ZI$$Limit;

uint32_t END_OF_IMAGE = (uint32_t) &Image$$RW_IRAM1$$ZI$$Limit;

uint32_t heap_start;

mem_blk_t* free_mem;
mem_blk_t* alloc_mem;

void allocate_memory_to_queue(linkedlist_t** ll) {
    uint32_t i = 0;
    ll = (linkedlist_t **)heap_start;

    heap_start += NUM_PRIORITIES * sizeof(linkedlist_t *);

    for (i = 0; i < NUM_PRIORITIES; i++) {
        ll[i] = (linkedlist_t *)heap_start;
        heap_start += sizeof(linkedlist_t);
    }
}

void allocate_memory_to_pcbs() {
    uint32_t i = 0;
    pcbs = (pcb_t **)heap_start;

    heap_start += NUM_PROCESSES * sizeof(pcb_t *);

    for (i = 0; i < NUM_PROCESSES; i++) {
        pcbs[i] = (pcb_t *)heap_start;
        heap_start += sizeof(pcb_t);
    }

}

int k_init_memory_blocks(void) {
    uint32_t i;

    heap_start = END_OF_IMAGE + MEM_OFFSET_SIZE;

    //Zero out all of the memory we have to avoid garbage
    for(i = heap_start; i < END_OF_MEM - 8; i += 4) {
        *((uint32_t *)i) = SWAP_UINT32(INVALID_MEMORY);
    }

    allocate_memory_to_queue(ready_pqs);
    allocate_memory_to_queue(mem_blocked_pqs);

    heap_start += 0x100 - (heap_start % 0x100); // align for mem_blk

    free_mem = (mem_blk_t *)heap_start;
    alloc_mem = NULL;

    free_mem->prev = NULL;

    for(i = heap_start; i < heap_start + MEM_BLOCK_SIZE * 49; i+= MEM_BLOCK_SIZE) {
        ((mem_blk_t *)i)->next = (mem_blk_t *)(i + MEM_BLOCK_SIZE);
        ((mem_blk_t *)i)->next->prev = (mem_blk_t *)i;
        ((mem_blk_t *)i)->data = (void *)(i + MEM_BLOCK_HEADER_SIZE);
        ((mem_blk_t *)i)->padding = SWAP_UINT32(0xABAD1DEA);
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
        *((uint32_t *)i) = 0;
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
