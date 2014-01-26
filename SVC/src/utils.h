#ifndef UTILS_H_
#define UTILS_H_

typedef unsigned int uint32_t;
typedef int int32_t;
typedef void (*func_ptr_t)();

#define NULL                    0

#define MAX_MEM_BLOCKS          50
#define MEM_OFFSET_SIZE 		8
#define MEM_BLOCK_SIZE 			0x100 // 0x100 bytes / sizeof(mem_blk_t)
#define MEM_BLOCK_HEADER_SIZE	0x10
#define USER_DATA_BLOCK_SIZE    MEM_BLOCK_SIZE - MEM_BLOCK_HEADER_SIZE

#define INVALID_MEMORY			0xDEADBEEF
#define END_OF_MEM              0x10008000
#define XPSR                    0x01000000 // default processor state register value

#define STACK_SIZE              0x100
#define NUM_PROCESSES           7
#define NUM_PRIORITIES          5

#define ASSERT_FALSE(i)         i = 0 / 0
#define SWAP_UINT16(x)          (((x) >> 8 ) | ((x) << 8))
#define SWAP_UINT32(x)          (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

#endif
