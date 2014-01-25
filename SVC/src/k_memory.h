#ifndef _K_RTX_MEMORY_H_
#define _K_RTX_MEMORY_H_

#define END_OF_MEM 0x10008000

void* k_request_memory_block(void);
int k_release_memory_block(void *);
int k_init_memory_blocks(void);

#endif
