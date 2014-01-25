#include "usr_proc.h"
#include "printf.h"
#include "rtx.h"
#include "uart_polling.h"
#include "utils.h"
#include "process.h"

// TODO rename
PROC_INIT test_procs[NUM_PROCS];

// Init function
void set_procs(){
    int i;
    for (i = 0; i < NUM_PROCS; i++){
        test_procs[i].pid           = (uint32_t)(i+1);
        test_procs[i].priority      = LOWEST;
        test_procs[i].stack_size    = 0x100; // TODO change?
    }
    test_procs[0].start_pc = &usr_proc1;
    test_procs[1].start_pc = &usr_proc2;
    test_procs[2].start_pc = &usr_proc3;
    test_procs[3].start_pc = &usr_proc4;
    test_procs[4].start_pc = &usr_proc5;
    test_procs[5].start_pc = &usr_proc6;
}

/*  Process 1:
*      Process prints five uppercase letters
*      Yields CPU
*/
void usr_proc_1(){
    int i = 0;
    int ret;
    while (1) {
        if (i != 0 && i%5 == 0) {
            uart0_put_string("\n\r");
            ret = release_processor();
            if (ret != 0) {
                uart0_put_string("Test 1 FAIL\r\n");
            } else {
                uart0_put_string("Test 1 PASS\r\n");
            }
        }
        uart0_put_char('A' + i % 26);
        i++;
    }
}

/*
    This process will request 2 memory blocks, print their value
    then release them. Finally it will output the return value;
*/
void usr_proc_2(){
    int ret;
    int i = 0;

    void* memory1 = 0;
    void* memory2 = 0;

    while (1) {
        if (i == 0){
            memory1 = request_memory_block();
            memory2 = request_memory_block();

            ret = release_memory_block(memory1);
            if (ret != 0){
                uart0_put_string("Test 3 FAIL\r\n");
            }
            ret = release_memory_block(memory2);
            if (ret != 0){
                uart0_put_string("Test 3 FAIL\r\n");
            }

            ret = release_processor();
            if (ret != 0){
                uart0_put_string("Test 3 FAIL\r\n");
            } else {
                uart0_put_string("Test 3 PASS\r\n");
            }
        }
        i++;
    }
}

void usr_proc_3(){
    int i = 0;
    int ret;
    void* memory = 0;
    while (1) {
        if (i == 0){
            memory = request_memory_block();
            memory = SWAP_UINT32(INVALID_MEMORY);

            ret = release_memory_block(memory);
            if (ret == 0) {
                uart0_put_string("Test 3 FAIL\r\n");
            }
            ret = release_processor();
            if (ret != 0) {
                uart0_put_string("Test 3 FAIL\r\n");
            }
            uart0_put_string("Test 3 PASS\r\n");
        }
        i++;
    }
}

/*
    Process 4
        Set the process priority from LOWEST to HIGHEST, check if returns correct
*/
void usr_proc_4(){
    int i = 0;
    int ret;
    int proc = 3; // TODO enum
    while (1) {
        if (i == 0) {
            ret = get_process_priority(test_procs[proc].pid);
            if (ret != LOWEST) {
                uart0_put_string("Test 3 FAIL\r\n");
            }
            ret = set_process_priority(test_procs[proc].pid, HIGHEST);
            if (ret != 0) {
                uart0_put_string("Test 3 FAIL\r\n");
            }
            ret = get_process_priority(test_procs[proc].pid);
            if (ret != HIGHEST) {
                uart0_put_string("Test 3 FAIL\r\n");
            }
            ret = release_processor();
            if (ret != 0) {
                uart0_put_string("Test 3 FAIL\r\n");
            }
            uart0_put_string("Test 3 PASS\r\n");
        }
        i++;
    }
}

void usr_proc_5(){
    int i = 0;
    int ret;
    int proc = 4; // TODO enum

    while (1) {
        if (i == 0) {
            ret = get_process_priority(test_procs[proc].pid);
            if (ret != LOWEST) {
                uart0_put_string("Test 3 FAIL\r\n");
            }
            ret = set_process_priority(test_procs[proc].pid, 10);
            if (ret != 0) {
                uart0_put_string("Test 3 PASS\r\n");
            } else {
                uart0_put_string("Test 3 FAIL\r\n");
            }
            ret = release_processor();
            if (ret != 0){
                uart0_put_string("Test 3 FAIL\r\n");
            }
            uart0_put_string("Test 3 PASS\r\n");
        }
        i++;
    }

}
void usr_proc_6(){
    // TODO
}
/* Have 1 NULL process */
void null_proc(){
    while (1) {
        release_processor();
    }
}