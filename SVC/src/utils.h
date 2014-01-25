#ifndef UTILS_H_
#define UTILS_H_

typedef unsigned int uint32_t;
typedef int int32_t;

#define NULL 0
#define MEM_OFFSET_SIZE 		8
#define MEM_BLOCK_SIZE 			0x100 // 0x100 bytes / sizeof(mem_blk_t)
#define MEM_BLOCK_HEADER_SIZE	0x10
#define INVALID_MEMORY			0xDEADBEEF
#define USER_DATA_BLOCK_SIZE	MEM_BLOCK_SIZE - MEM_BLOCK_HEADER_SIZE


#endif
