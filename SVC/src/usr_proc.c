#include "printf.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "rtx.h"

static const int NUM_PROCS = 6;
proc_image_t usr_proc_table[NUM_PROCS];
int32_t test_results[NUM_PROCS];

// Init function
void usr_set_procs() {
    uint32_t i;

    for (i = 0; i < NUM_PROCS; i++) {
        usr_proc_table[i].pid        = i + 1;
        usr_proc_table[i].priority   = LOW;
        usr_proc_table[i].stack_size = STACK_SIZE;
    }

    for (i = 0; i < NUM_PROCS; i++) {
        test_results[i] = 1;
    }

    usr_proc_table[0].proc_start = &usr_proc_1;
    usr_proc_table[1].proc_start = &usr_proc_2;
    usr_proc_table[2].proc_start = &usr_proc_3;
    usr_proc_table[3].proc_start = &usr_proc_4;
    usr_proc_table[4].proc_start = &usr_proc_5;
    usr_proc_table[5].proc_start = &usr_proc_6;

    // Start the G005_test: test output required
    uart0_put_string("G005_test: START\r\n");
    uart0_put_string("G005_test: total 5 tests\r\n");
}

/*  Process 1:
*      Process prints five uppercase letters
*      Yields CPU
*/
void usr_proc_1() {
    int i = 0;
    int ranOnce = 0;
    while (1) {
        //printf("Process 1\r\n");
        if (i != 0 && i % 5 == 0) {
            test_results[1] = 1;
            if (ranOnce == 0) {
                uart0_put_string("G005_test: test 1 OK\r\n");
            }
            ranOnce++;
            release_processor();
        }
        i++;
    }
}

/*
    This process will request 2 memory blocks, print their value
    then release them. Finally it will output the return value;
*/
void usr_proc_2() {
    int ret;
    int i = 0;
    int proc = 1;
    void* memory1 = 0;
    void* memory2 = 0;

    while (1) {
        //printf("Process 2\r\n");
        memory1 = request_memory_block();
        memory2 = request_memory_block();

        *((int*)memory1) = 0xDEAD10CC;
        *((int*)memory2) = 0xDEADC0DE;

        if (*((int*)memory1) != 0xDEAD10CC) {
            test_results[usr_proc_table[proc].pid] = 0;
        }
        ret = release_memory_block(memory1);
        if (ret != 0 && i == 0) {
            test_results[usr_proc_table[proc].pid] = 0;
        }

        if (*((int*)memory2) != 0xDEADC0DE) {
            test_results[usr_proc_table[proc].pid] = 0;
        }
        ret = release_memory_block(memory2);
        if (ret != 0 && i == 0) {
            test_results[usr_proc_table[proc].pid] = 0;
        }
        if (i == 0) {
            if (test_results[proc] == 1) {
                uart0_put_string("G005_test: test 2 OK\r\n");
            } else {
                uart0_put_string("G005_test: test 2 FAIL\r\n");
            }
        }
        i = 1;
        ret = release_processor();
    }
}

void usr_proc_3() {
    int i = 0;
    int ret;
    int proc = 2;
    void* memory = 0;

    while (1) {
        //printf("Process 3\r\n");
        memory = (void*) 0xDEADC0DE;

        ret = release_memory_block(memory);
        if (ret == 0) {
            test_results[usr_proc_table[proc].pid] = 0;
        }
        if (i == 0) {
            if (test_results[proc] == 1) {
                uart0_put_string("G005_test: test 3 OK\r\n");
            } else {
                uart0_put_string("G005_test: test 3 FAIL\r\n");
            }
        }
        i = 1;
        ret = release_processor();
    }
}

/*
    Process 4
        Set the process priority from LOWEST to HIGHEST, check if returns correct
*/
void usr_proc_4() {
    int i = 0;
    int ret;
    int proc = 3;

    while (1) {
        //printf("Process 4\r\n");
        ret = get_process_priority(usr_proc_table[proc].pid);
        if (ret != LOW) {
            test_results[usr_proc_table[proc].pid] = 0;
        }
        ret = set_process_priority(usr_proc_table[proc].pid, HIGH);
        if (ret != 0) {
            test_results[usr_proc_table[proc].pid] = 0;
        }
        ret = get_process_priority(usr_proc_table[proc].pid);
        if (ret != HIGH) {
            test_results[usr_proc_table[proc].pid] = 0;
        }

        ret = set_process_priority(usr_proc_table[proc].pid, LOW);
        if (ret != 0) {
            test_results[usr_proc_table[proc].pid] = 0;
        }
        if (i == 0) {
            if (test_results[proc] == 1) {
                uart0_put_string("G005_test: test 4 OK\r\n");
            } else {
                uart0_put_string("G005_test: test 4 FAIL\r\n");
            }
        }
        i = 1;
        ret = release_processor();
    }
}

void usr_proc_5() {
    int i = 0;
    int ret;
    int proc = 4;

    while (1) {
        //printf("Process 5\r\n");
        ret = get_process_priority(usr_proc_table[proc].pid);
        if (ret != LOW) {
            test_results[usr_proc_table[proc].pid] = 0;
        }
        ret = set_process_priority(usr_proc_table[proc].pid, 10);

        if (ret == 0) {
            test_results[usr_proc_table[proc].pid] = 0;
        }

        ret = set_process_priority(usr_proc_table[proc].pid, HIGH);

        if (ret != 0) {
            test_results[usr_proc_table[proc].pid] = 0;
        }

        ret = get_process_priority(usr_proc_table[proc].pid);

        if (ret != HIGH) {
            test_results[usr_proc_table[proc].pid] = 0;
        }
        ret = set_process_priority(usr_proc_table[proc].pid, LOW);
        if (ret != 0) {
            test_results[usr_proc_table[proc].pid] = 0;
        }
        if (i == 0) {

            if (test_results[proc] == 1) {
                uart0_put_string("G005_test: test 5 OK\r\n");
            } else {
                uart0_put_string("G005_test: test 5 FAIL\r\n");
            }
        }
        i = 1;
        ret = release_processor();
    }
}
void usr_proc_6() {
    int i;
    static int ranOnce = 0;
    static int passed = 0;
    while (1) {
        if (ranOnce == 0) {
            for (i = 0; i < 5; i++) {
                if (test_results[i] == 1) {
                    passed++;
                }
            }
            uart0_put_string("G005_test: ");
            uart0_put_char('0' + passed);
            uart0_put_string("/5 OK\r\n");
            uart0_put_string("G005_test: ");
            uart0_put_char('0' + (5 - passed));
            uart0_put_string("/5 FAIL\r\n");
            uart0_put_string("G005_test: END\r\n");
            ranOnce = 1;
        }
        release_processor();
    }
}
