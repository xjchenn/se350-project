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
    //proc_table[1].priority   = HIGHEST;
    //proc_table[2].priority   = HIGHEST;
		
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
			printf("Process 1\r\n");
        if (i == 0) {
            uart0_put_string("PROCESS 1 WORKS!\r\n");
            release_processor();
        }
    }
}

/*
    This process will request 2 memory blocks, print their value
    then release them. Finally it will output the return value;
*/
void usr_proc_2(){	
    int ret;
    static int i = 0;
    int didPass = 1;

    static void* memory1 = 0;
    static void* memory2 = 0;
		
    while (1) {
				printf("Process 2\r\n");    
        if (i % 2 == 0){
						memory1 = request_memory_block();
            memory2 = request_memory_block();

            if (memory1 == NULL || memory2 == NULL) {
                uart0_put_string("Test 2 RAN OUT OF MEM\r\n");   
                didPass = 0;
            } else {
								didPass = 1;
                uart0_put_string("some mem allocated \r\n");
            }

            //if (*((int*)memory1) != 0xDEAD10CC) {
            //    didPass = 0;
            //    uart0_put_string("Test 2 FAIL\r\n");
            //}
//
            ////ret = release_memory_block(memory1);
            //if (ret != 0){
            //    didPass = 0;
            //    uart0_put_string("Test 2 FAIL\r\n");
            //}
//
            //if (*((int*)memory2) != 0xDEADC0DE) {
            //    didPass = 0;
            //    uart0_put_string("Test 2 FAIL\r\n");
            //}
//
            ////ret = release_memory_block(memory2);
            //if (ret != 0){
            //    didPass = 0;
            //    uart0_put_string("Test 2 FAIL\r\n");
            //}
//
            if (didPass > 0) {
                uart0_put_string("Test 2 PASS\r\n");
            }

            ret = release_processor();
        } else {
						uart0_put_string("Releasing memory\r\n");
            release_memory_block(memory1);
            release_memory_block(memory2);
					  release_processor();
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
					printf("Process 3\r\n");
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
        			printf("Process 4\r\n");
            such_memory_wow = (void *)((int *) request_memory_block() + 0x100);
            ret = release_memory_block(such_memory_wow);
            if (ret == 3) {
                uart0_put_string("Test 4 PASS\r\n");
            } else {
                uart0_put_string("Test 4 FAIL\r\n");
            }
						such_memory_wow = (int *) such_memory_wow - 0x100;
			release_memory_block(such_memory_wow);
            ret = release_processor();
    }
}

void usr_proc_5(){	
    int i = 0;
    int ret;
    int didPass = 1;
    int proc = 6; // TODO enum

    while (1) {
        if (i == 0) {
					printf("Process 5\r\n");
            ret = get_process_priority(proc_table[proc].pid);
            if (ret != proc_table[proc].priority) {
                uart0_put_string("Test 5 FAIL\r\n");
                didPass = 0;
            }
            ret = set_process_priority(proc_table[proc].pid, 10);
            
            if (ret == 0) {
                uart0_put_string("Test 5 FAIL\r\n");
                didPass = 0;
            }
            
            //ret = set_process_priority(proc_table[proc].pid, 0);
            //
            //if (ret != 0){
            //    uart0_put_string("Test 5 FAIL\r\n");
            //    didPass = 0;
            //}
            //
            //ret = get_process_priority(proc_table[proc].pid);
            //
            //if (ret != 0) {
            //    uart0_put_string("Test 5 FAIL\r\n");
            //    didPass = 0;
            //}
            if(didPass > 0) {
                uart0_put_string("Test 5 PASS\r\n");
            }
            
            ret = release_processor();
        }
    }
}
void usr_proc_6(){
    int i = 0;
    int ret;
	
    while (1) {
        if (i == 0) {
						printf("Process 6\r\n");
            for (i = 0; i < 7; i++) {
                printf("%d\r\n", get_process_priority(i));
            }
            i = 0;
            uart0_put_string("Test 6 PASS\r\n");
            ret = release_processor();
        }
    }
}

/* Have 1 NULL process */
void null_proc(){
    while (1) {
				printf("Process NULL\r\n");
				printf("NULL PROCESS IS RUNNING\r\n");
        release_processor();
    }
}
