#include "printf.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "rtx.h"

proc_image_t proc_table[7];

int32_t test_results[6];

// Init function
void set_procs() {
    uint32_t i;

    for (i = 0; i < 7; i++) {
        proc_table[i].pid        = i;
        proc_table[i].priority   = LOWEST;
        proc_table[i].stack_size = STACK_SIZE;
    }
    for (i = 0; i < 6; i++) {
        test_results[i] = 1;
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

    // Start the test output required
    uart0_put_string("G005_test: START\r\n");
    uart0_put_string("G005_test: total 6 tests\r\n");

}

/*  Process 1:
*      Process prints five uppercase letters
*      Yields CPU
*/
void usr_proc_1() {
    static int i = 0;


    while (1) {
        if (i != 0 && i % 5 == 0) {
            uart0_put_string("test 1 OK\r\n");
            test_results[1] = 1;
            release_processor();
        }
    }
}

/*
    This process will request 2 memory blocks, print their value
    then release them. Finally it will output the return value;
*/
void usr_proc_2() {
    int ret;
    int i = 0;
    int proc = 2;
    void* memory1 = 0;
    void* memory2 = 0;

    while (1) {
        memory1 = request_memory_block();
        memory2 = request_memory_block();

        *((int*)memory1) = 0xDEAD10CC;
        *((int*)memory2) = 0xDEADC0DE;

        if (*((int*)memory1) != 0xDEAD10CC) {
            test_results[proc_table[proc].pid] = 0;
        }
        ret = release_memory_block(memory1);
        if (ret != 0 && i == 0) {
            test_results[proc_table[proc].pid] = 0;
        }

        if (*((int*)memory2) != 0xDEADC0DE) {
            test_results[proc_table[proc].pid] = 0;
        }
        ret = release_memory_block(memory2);
        if (ret != 0) {
            test_results[proc_table[proc].pid] = 0;
        }
        if (i == 0) {
            if (test_results[proc] == 1) {
                uart0_put_string("test 2 OK\r\n");
            } else {
                uart0_put_string("test 2 FAIL\r\n");
            }
        }
        i = 1;
        ret = release_processor();
    }
}

void usr_proc_3() {
    int i = 0;
    int ret;
    int proc = 3;
    void* memory = 0;


    while (1) {
        memory = (void*) 0xDEADC0DE;

        ret = release_memory_block(memory);
        if (ret == 0) {
            test_results[proc_table[proc].pid] = 0;
        }
        if (i == 0) {
            if (test_results[proc] == 1) {
                uart0_put_string("test 2 OK\r\n");
            } else {
                uart0_put_string("test 2 FAIL\r\n");
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
    int proc = 4; // TODO enum
    while (1) {
        ret = get_process_priority(proc_table[proc].pid);
        if (ret != LOWEST) {
            test_results[proc_table[proc].pid] = 0;
        }
        ret = set_process_priority(proc_table[proc + 1].pid, HIGHEST);
        if (ret != 0) {
            test_results[proc_table[proc].pid] = 0;
        }
        ret = get_process_priority(proc_table[proc + 1].pid);
        if (ret != HIGHEST) {
            test_results[proc_table[proc].pid] = 0;
        }
        if (i == 0) {
            if (test_results[proc] == 1) {
                uart0_put_string("test 2 OK\r\n");
            } else {
                uart0_put_string("test 2 FAIL\r\n");
            }
        }
        i = 1;
        ret = release_processor();
    }
}

void usr_proc_5() {
    int i = 0;
    int ret;
    int proc = 5; // TODO enum

    while (1) {
        ret = get_process_priority(proc_table[proc].pid);
        if (ret != proc_table[proc].priority) {
            test_results[proc_table[proc].pid] = 0;
        }
        ret = set_process_priority(proc_table[proc].pid, 10);

        if (ret == 0) {
            test_results[proc_table[proc].pid] = 0;
        }

        ret = set_process_priority(proc_table[proc].pid, LOWEST);

        if (ret != 0) {
            test_results[proc_table[proc].pid] = 0;
        }

        ret = get_process_priority(proc_table[proc].pid);

        if (ret != 0) {
            test_results[proc_table[proc].pid] = 0;
        }

        if (i == 0) {

            if (test_results[proc] == 1) {
                uart0_put_string("test 2 OK\r\n");
            } else {
                uart0_put_string("test 2 FAIL\r\n");
            }
        }
        i = 1;
        ret = release_processor();
    }
}
void usr_proc_6() {
    int i;
    int ranOnce = 0;
    int passed = 0;
    while (1) {
        if (ranOnce) {
            for (i = 0; i < 6; i++) {
                if (test_results[i] == 1) {
                    passed++;
                }
            }
            uart0_put_string("G005_test: ");
            uart0_put_char((char)passed);
            uart0_put_string("/6 OK\r\n");
            uart0_put_string("G005_test: ");
            uart0_put_char((char)(6 - passed));
            uart0_put_string("/6 FAIL\r\n");
            uart0_put_string("G005_test: END\r\n");
        }
    }
}

/* Have 1 NULL process */
void null_proc() {
    while (1) {
        printf("Process NULL\r\n");
        printf("NULL PROCESS IS RUNNING\r\n");
        release_processor();
    }
}
