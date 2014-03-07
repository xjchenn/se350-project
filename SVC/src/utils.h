#ifndef UTILS_H_
#define UTILS_H_

typedef unsigned int uint32_t;
typedef int int32_t;
typedef void (*func_ptr_t)();

#define NULL                    0

#define MEM_OFFSET_SIZE         8       // padding between kernel and heap
#define MEM_BLOCK_SIZE          0x100   // 0x100 bytes / sizeof(mem_blk_t)
#define MAX_MEM_BLOCKS          0x30    // 48 blocks uses 0x3000 bytes of memory
#define MEM_BLOCK_HEADER_SIZE   0x10
#define KERNEL_MSG_HEADER_SIZE  20
#define USER_DATA_BLOCK_SIZE    MEM_BLOCK_SIZE - MEM_BLOCK_HEADER_SIZE - KERNEL_MSG_HEADER_SIZE

#define INVALID_MEMORY          0xDEADBEEF
#define END_OF_MEM              0x10008000
#define XPSR                    0x01000000 // default processor state register value

#define STACK_SIZE              0x100
#define NUM_PRIORITIES          5

#define NUM_K_PROCESSES         5
#define NUM_I_PROCESSES         2
#define NUM_USR_PROCESSES       6
#define NUM_PROCESSES           16//(uint32_t)(NUM_USR_PROCESSES + NUM_K_PROCESSES + NUM_I_PROCESSES)

#define MSG_LOG_BUFFER_SIZE     10

#define SWAP_UINT16(x)          (((x) >> 8 ) | ((x) << 8))
#define SWAP_UINT32(x)          (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

#ifdef DEBUG
#define DEBUG_PRINT(msg)        printf(msg)
#else
#define DEBUG_PRINT(msg)        /* nop */
#endif



#define PID_NULL                0
#define PID_P1                  1
#define PID_P2                  2
#define PID_P3                  3
#define PID_P4                  4
#define PID_P5                  5
#define PID_P6                  6
#define PID_A                   7
#define PID_B                   8
#define PID_C                   9
#define PID_SET_PRIO            10
#define PID_CLOCK               11
#define PID_KCD                 12
#define PID_CRT                 13
#define PID_TIMER_IPROC         14
#define PID_UART_IPROC          15
#define KEY_READY_QUEUE         '1'
#define KEY_BLOCKED_MEM_QUEUE   '2'
#define KEY_BLOCKED_MSG_QUEUE   '3'
#define KEY_MSG_LOG_BUFFER      '4'

#define PRINT_NEWLINE           printf("\r\n")
#define PRINT_HEADER            printf("----------------------------------------------------------\r\n")

#endif
