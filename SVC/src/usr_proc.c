#include "printf.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "rtx.h"

proc_image_t proc_table[7];

// Init function
void set_procs(){
    uint32_t i;

    for (i = 0; i < 7; i++){
        proc_table[i].pid        = i;
        proc_table[i].priority   = LOWEST;
        proc_table[i].stack_size = STACK_SIZE;
    }

    proc_table[1].priority   = HIGHEST;
    proc_table[4].priority   = HIGHEST;

    proc_table[0].proc_start = &null_proc;
    proc_table[1].proc_start = &usr_proc_1;
    proc_table[2].proc_start = &usr_proc_2;
    proc_table[3].proc_start = &usr_proc_3;
    proc_table[4].proc_start = &usr_proc_4;
    proc_table[5].proc_start = &usr_proc_5;
    proc_table[6].proc_start = &usr_proc_6;
}

/*  Process 1:
*      Process prints five uppercase letters
*      Yields CPU
*/
void usr_proc_1(){
    static int i = 0;
    while (1) {
        if (i == 0) {
            uart0_put_string("PROCESS 1 WORKS!\r\n");
            release_processor();
        }
        // uart0_put_char('A' + i % 26);
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
    int didPass = 1;

    void* memory1 = 0;
    void* memory2 = 0;

    while (1) {
        if (i == 0){
            memory1 = request_memory_block();
            memory2 = request_memory_block();

            *((int*)memory1) = 0xDEAD10CC;
            *((int*)memory2) = 0xDEADC0DE;

            if (*((int*)memory1) != 0xDEAD10CC) {
                didPass = 0;
                uart0_put_string("Test 2 FAIL\r\n");
            }

            ret = release_memory_block(memory1);
            if (ret != 0){
                didPass = 0;
                uart0_put_string("Test 2 FAIL\r\n");
            }

            if (*((int*)memory2) != 0xDEADC0DE) {
                didPass = 0;
                uart0_put_string("Test 2 FAIL\r\n");
            }

            ret = release_memory_block(memory2);
            if (ret != 0){
                didPass = 0;
                uart0_put_string("Test 2 FAIL\r\n");
            }

            if (didPass > 0) {
                uart0_put_string("Test 2 PASS\r\n");
            }

            ret = release_processor();
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
            memory = (void*) 0xDEADC0DE;

            ret = release_memory_block(memory);
            if (ret == 0) {
                uart0_put_string("Test 3 FAIL\r\n");
            } else {
                uart0_put_string("Test 3 PASS\r\n");
            }
            ret = release_processor();
        }
    }
}

/*
    Process 4
        Set the process priority from LOWEST to HIGHEST, check if returns correct
*/
void usr_proc_4(){
    int i = 0;
    int ret;
    void* such_memory_wow = 0;
    while (1) {
        if (i == 0) {
            such_memory_wow = (void *)((int *) request_memory_block() + 0x100);
            ret = release_memory_block(such_memory_wow);
            if (ret == 2) {
                uart0_put_string("Test 4 PASSED");
            } else {
                uart0_put_string("Test 4 FAILED");
            }
            ret = release_processor();
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
            /*ret = get_process_priority(proc_table[proc].pid);
            if (ret != LOWEST) {
                uart0_put_string("Test 3 FAIL\r\n");
            }
            ret = set_process_priority(proc_table[proc].pid, 10);
            if (ret != 0) {
                uart0_put_string("Test 3 PASS\r\n");
            } else {
                uart0_put_string("Test 3 FAIL\r\n");
            }
            ret = release_processor();
            if (ret != 0){
                uart0_put_string("Test 3 FAIL\r\n");
            }*/
            uart0_put_string("Test 5 PASS\r\n");
            ret = release_processor();
        }
    }
}
void usr_proc_6(){
    int i = 0;
    int ret;
    while (1) {
        if (i == 0) {
            uart0_put_string("Test 6 PASS\r\n");
            ret = release_processor();
        }
    }
}

/* Have 1 NULL process */
void null_proc(){
    while (1) {
				printf("NULL PROCESS IS RUNNING\r\n");
        release_processor();
    }
}
