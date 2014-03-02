#include "k_memory.h"
#include "printf.h"
#include "utils.h"
#include "linkedlist.h"
#include "k_process.h"

/*
  This symbol is defined in the scatter file,
  refer to RVCT Linker User Guide
*/
extern uint32_t Image$$RW_IRAM1$$ZI$$Limit;

uint32_t end_of_image = (uint32_t) &Image$$RW_IRAM1$$ZI$$Limit;
uint32_t start_of_heap;
uint32_t end_of_heap;
uint32_t* stack;

uint32_t blocks_allocated;
mem_blk_t* free_mem;
mem_blk_t* alloc_mem;

/**
 * Allocates memory for the ready/blocked queues (implemented as doubly linked lists) at the top of the heap
 * @param ll    Reference to array of linkedlist*
 */
void allocate_memory_to_queue(linkedlist_t** * ll) {
    uint32_t i = 0;
    *ll = (linkedlist_t**)start_of_heap;

    start_of_heap += NUM_PRIORITIES * sizeof(linkedlist_t*);

    for (i = 0; i < NUM_PRIORITIES; i++) {
        (*ll)[i] = (linkedlist_t*)start_of_heap;
        linkedlist_init((*ll)[i]);
        start_of_heap += sizeof(linkedlist_t);
    }
}

/**
 * Allocates memory for all of our PCBs at the top of the heap
 */
void allocate_memory_to_pcbs(void) {
    uint32_t i = 0;

    pcbs = (pcb_t**)start_of_heap;
    start_of_heap += 7 * sizeof(pcb_t*); //  NUM_PROCESSES * sizeof(pcb_t*); apparently compiler thinks this is equal to 0xA instead of 28 so this is hardcoded

    for (i = 0; i < NUM_PROCESSES; i++) {
        pcbs[i] = (pcb_t*)start_of_heap;
        start_of_heap += sizeof(pcb_t);
    }
}

/**
 * Allocates the top of the heap for all of our kernel objects then use the remaining space
 * to setup the memory blocks which are then later allocated through request_memory_block()
 * @return  Zero if successful
 */
uint32_t k_init_memory_blocks(void) {
    uint32_t i;
    uint32_t j;
    uint32_t heap_end;
    blocks_allocated = 0;
    start_of_heap = end_of_image + MEM_OFFSET_SIZE;

    //Zero out all of the memory we have to avoid garbage
    for (i = start_of_heap; i < END_OF_MEM - 8; i += 4) {
        *((uint32_t*)i) = SWAP_UINT32(INVALID_MEMORY);
    }

    allocate_memory_to_queue(&ready_pqs);
    allocate_memory_to_queue(&mem_blocked_pqs);
    allocate_memory_to_pcbs();

    stack = (uint32_t*)END_OF_MEM;
    if ((uint32_t)stack & 0x04) {
        --stack;
    }

    // align for mem_blk
    start_of_heap += 0x100 - (start_of_heap % 0x100);

    // prepare heap blocks
    free_mem = (mem_blk_t*)start_of_heap;
    alloc_mem = NULL;

    free_mem->prev = NULL; // first block

    heap_end = start_of_heap + MAX_MEM_BLOCKS * MEM_BLOCK_SIZE;
    i = start_of_heap;
    j = 0;
    for (; i < heap_end; i += MEM_BLOCK_SIZE, j++) {
        if (i < heap_end - MEM_BLOCK_SIZE) {
            // not last block
            ((mem_blk_t*)i)->next = (mem_blk_t*)(i + MEM_BLOCK_SIZE);
            ((mem_blk_t*)i)->next->prev = (mem_blk_t*)i;
        } else {
            // last block
            ((mem_blk_t*)i)->next = NULL;
        }
        ((mem_blk_t*)i)->data = (void*)(i + MEM_BLOCK_HEADER_SIZE);
        ((mem_blk_t*)i)->padding = SWAP_UINT32(0xABAD1DEA);
    }
    end_of_heap = i + MEM_BLOCK_SIZE;

    return 0;
}

/**
 * Allocates space for a stack
 * @param  size     Size for the allocated stack in bytes
 * @return          Address to the start of allocated stack
 */
uint32_t* k_alloc_stack(uint32_t size) {
    uint32_t* stack_ptr = stack;

    if ((uint32_t)stack - size < end_of_heap) {
        return NULL;
    }

    stack = (uint32_t*)((uint32_t)stack_ptr - size);

    if ((uint32_t)stack & 0x04) {
        --stack;
    }

    return stack_ptr;
}


/**
 * Requests a memory block
 * @return           Returns a pointer to a memory block (void*) that can be used
 */
void* k_request_memory_block(void) {
    uint32_t i = 0;
    mem_blk_t* ret_blk = free_mem;

    // if we can't get free memory, block the current process
    while (ret_blk == NULL) {
        current_pcb->state = BLOCKED;
        k_release_processor();
        ret_blk = free_mem;
    }
    // otherwise increment global memory pointer
    free_mem = free_mem->next;
    if (free_mem != NULL) {
        free_mem->prev = NULL;
    }
    ret_blk->next = alloc_mem;
    if (alloc_mem != NULL) {
        alloc_mem->prev = ret_blk;
    }

    alloc_mem = ret_blk;

    for (i = (uint32_t)ret_blk->data; i < ((uint32_t)ret_blk + MEM_BLOCK_SIZE - 1); i += 4) {
        *((uint32_t*)i) = 0;
    }
    // assigns the current process to the memory blocks given
            blocks_allocated++;
            return (void*)ret_blk->data;
}
/**
 * Releases a memory block from control by a process
 * @param  p_mem_blk        the memory block owned by the process
 * @return                  returns a status code as to the success of release
 */
uint32_t k_release_memory_block(void* p_mem_blk) {
    mem_blk_t* to_del = (mem_blk_t*)((uint32_t)p_mem_blk - MEM_BLOCK_HEADER_SIZE);

    // if we get a null block we return a status code telling us that
    if (to_del == NULL) {
        return 1;
    // otherwise we want to verify the range of the memory block
    } else if ((uint32_t)to_del < start_of_heap || (uint32_t)to_del > END_OF_MEM) {
        return 2;
    } else {
             if (to_del->prev != NULL) {
                    to_del->prev->next = to_del->next;
                }

                if (to_del->next != NULL) {
                    to_del->next->prev = to_del->prev;
                }

                to_del->next = free_mem;
                to_del->prev = NULL;
				blocks_allocated--;
                free_mem = to_del;

                // release the processor if we have to preempt a blocked process
                if (k_should_preempt_current_process()) {
                    k_release_processor();
                }

                return 0;
    }
}
