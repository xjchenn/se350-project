#include "test_proc.h"
#include "rtx.h"
#include "timer.h"

proc_image_t g_test_procs[NUM_TEST_PROCESSES];

int32_t test_results[NUM_TEST_PROCESSES];
int32_t test_ran[NUM_TEST_PROCESSES];

extern volatile uint32_t test_timer_count;
extern volatile uint32_t g_timer_count;

void set_test_procs() {
    uint32_t i;

    for (i = 0; i < NUM_TEST_PROCESSES; ++i) {
        g_test_procs[i].pid = (i + 1);
        test_results[i] = 1;
        test_ran[i] = 0;
    }
    test_ran[5] = 1;

    /*
    // Testing p1

    // a
    //g_test_procs[0].priority = MEDIUM;
    //g_test_procs[1].priority = MEDIUM;

    // b
    //g_test_procs[0].priority = MEDIUM;
    //g_test_procs[1].priority = HIGH;

    g_test_procs[0].proc_start = &test_proc_p1_b_1;
    g_test_procs[1].proc_start = &test_proc_p1_b_2;
    g_test_procs[2].proc_start = &test_proc_p1_b_3;
    g_test_procs[3].proc_start = &test_proc_p1_b_4;
    g_test_procs[4].proc_start = &test_proc_p1_b_5;
    g_test_procs[5].proc_start = &test_proc_p1_b_6;
    */

    g_test_procs[0].proc_start = &test_proc_p2_1;
    g_test_procs[0].priority = MEDIUM;

    g_test_procs[1].proc_start = &test_proc_p2_2;
    g_test_procs[1].priority = MEDIUM;

    g_test_procs[2].proc_start = &test_proc_p2_3;
    g_test_procs[2].priority = MEDIUM;

    g_test_procs[3].proc_start = &test_proc_p2_4;
    g_test_procs[3].priority = MEDIUM;

    g_test_procs[4].proc_start = &test_proc_p2_5;
    g_test_procs[4].priority = MEDIUM;

    g_test_procs[5].proc_start = &test_proc_p1_6; // always use p1 test_proc for printing our test results
    g_test_procs[5].priority = LOW;

    g_test_procs[6].pid = PID_TIMER_TEST_PROC;
    g_test_procs[6].proc_start = &timer_test_proc_p4;
    g_test_procs[6].priority = LOW;

    printf("G005_test: START\r\n");
    printf("G005_test: total 5 tests\r\n");
}

/******************************************************************************
 * Part 1 : Our user procs
 ******************************************************************************/

/**
 * Process prints five uppercase letters
 */
void test_proc_p1_1() {
    int i = 0;
    int ranOnce = 0;
    while (1) {
        //printf("Process 1\r\n");
        if (i != 0 && i % 5 == 0) {
            test_results[1] = 1;
            if (ranOnce == 0) {
                printf("G005_test: test 1 OK\r\n");
            }
            ranOnce++;
            release_processor();
        }
        i++;
    }
}

/**
 * This process will request 2 memory blocks, print their value
 * then release them. Finally it will output the return value;
 */
void test_proc_p1_2() {
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
            test_results[g_test_procs[proc].pid] = 0;
        }
        ret = release_memory_block(memory1);
        if (ret != 0 && i == 0) {
            test_results[g_test_procs[proc].pid] = 0;
        }

        if (*((int*)memory2) != 0xDEADC0DE) {
            test_results[g_test_procs[proc].pid] = 0;
        }
        ret = release_memory_block(memory2);
        if (ret != 0 && i == 0) {
            test_results[g_test_procs[proc].pid] = 0;
        }
        if (i == 0) {
            if (test_results[proc] == 1) {
                printf("G005_test: test 2 OK\r\n");
            } else {
                printf("G005_test: test 2 FAIL\r\n");
            }
        }
        i = 1;
        ret = release_processor();
    }
}

/**
 * Tries to free invalid memory location
 */
void test_proc_p1_3() {
    int i = 0;
    int ret;
    int proc = 2;
    void* memory = 0;

    while (1) {
        //printf("Process 3\r\n");
        memory = (void*) 0xDEADC0DE;

        ret = release_memory_block(memory);
        if (ret == 0) {
            test_results[g_test_procs[proc].pid] = 0;
        }
        if (i == 0) {
            if (test_results[proc] == 1) {
                printf("G005_test: test 3 OK\r\n");
            } else {
                printf("G005_test: test 3 FAIL\r\n");
            }
        }
        i = 1;
        ret = release_processor();
    }
}

/**
 * Set the process priority from LOWEST to HIGHEST, check if returns correct
 */
void test_proc_p1_4() {
    int i = 0;
    int ret;
    int proc = 3;

    while (1) {
        //printf("Process 4\r\n");
        ret = get_process_priority(g_test_procs[proc].pid);
        if (ret != LOW) {
            test_results[g_test_procs[proc].pid] = 0;
        }
        ret = set_process_priority(g_test_procs[proc].pid, HIGH);
        if (ret != 0) {
            test_results[g_test_procs[proc].pid] = 0;
        }
        ret = get_process_priority(g_test_procs[proc].pid);
        if (ret != HIGH) {
            test_results[g_test_procs[proc].pid] = 0;
        }

        ret = set_process_priority(g_test_procs[proc].pid, LOW);
        if (ret != 0) {
            test_results[g_test_procs[proc].pid] = 0;
        }
        if (i == 0) {
            if (test_results[proc] == 1) {
                printf("G005_test: test 4 OK\r\n");
            } else {
                printf("G005_test: test 4 FAIL\r\n");
            }
        }
        i = 1;
        ret = release_processor();
    }
}

/**
 * Changes priority of p4
 */
void test_proc_p1_5() {
    int i = 0;
    int ret;
    int proc = 4;

    while (1) {
        //printf("Process 5\r\n");
        ret = get_process_priority(g_test_procs[proc].pid);
        if (ret != LOW) {
            test_results[g_test_procs[proc].pid] = 0;
        }
        ret = set_process_priority(g_test_procs[proc].pid, 10);

        if (ret == 0) {
            test_results[g_test_procs[proc].pid] = 0;
        }

        ret = set_process_priority(g_test_procs[proc].pid, HIGH);

        if (ret != 0) {
            test_results[g_test_procs[proc].pid] = 0;
        }

        ret = get_process_priority(g_test_procs[proc].pid);

        if (ret != HIGH) {
            test_results[g_test_procs[proc].pid] = 0;
        }
        ret = set_process_priority(g_test_procs[proc].pid, LOW);
        if (ret != 0) {
            test_results[g_test_procs[proc].pid] = 0;
        }
        if (i == 0) {

            if (test_results[proc] == 1) {
                printf("G005_test: test 5 OK\r\n");
            } else {
                printf("G005_test: test 5 FAIL\r\n");
            }
        }
        i = 1;
        ret = release_processor();
    }
}

/**
 * Prints results of our tests
 */
void test_proc_p1_6() {
    int i;
    static int ranOnce = 0;
    static int passed = 0;
    while (1) {
        if (ranOnce == 0) {
            for (i = 0; i < 5; i++) {
                if (test_results[i] == 1) {
                    passed++;
                }
                while (test_ran[i] == 0) {
                    release_processor();
                }
            }
            printf("G005_test: ");
            printf("%d", passed);
            printf("/5 OK\r\n");
            printf("G005_test: ");
            printf("%d", (5 - passed));
            printf("/5 FAIL\r\n");
            printf("G005_test: END\r\n");
            ranOnce = 1;
            set_process_priority(PID_P6, LOW);
            set_process_priority(PID_TIMER_TEST_PROC, MEDIUM);
        }
        release_processor();
    }
}
/******************************************************************************
 * Part 2 : Our user procs
 ******************************************************************************/

/*
*   1) Delayed send to itself
*   2) Send message to user process 3
*   3) Send message to user process 2
*   4) Hogs all memory and then releases it all
*   4) Swap priorities with process 5
*   5) Swap priorities with process 4
*
*   For process 4 and process 5, process 4 should finish first.
*/

/* Delayed Send Test */
void test_proc_p2_1(void) {
    int result_pid = 0;
    msg_buf_t* msg_envelope = NULL;
    char* msg = "Hello";

    msg_envelope = (msg_buf_t*)request_memory_block();

    strncpy(msg_envelope->msg_data, msg, 5);
    delayed_send(PID_P1, msg_envelope, CLOCK_INTERVAL);
    msg_envelope = (msg_buf_t*)receive_message(NULL);

    if (strcmp(msg_envelope->msg_data, msg) == 5) {
        printf("G005_test: test 1 OK\r\n");
    } else {
        printf("G005_test: test 1 FAIL\r\n");
        test_results[result_pid] = 0;
    }

    release_memory_block(msg_envelope);
    test_ran[result_pid] = 1;
    set_process_priority(g_test_procs[result_pid].pid, 3);

    while (1) {
        release_processor();
    }
}
/* Send message to user process 3 */
void test_proc_p2_2(void) {
    int result_pid = 1;
    msg_buf_t* msg_envelope = 0;
    char* sent_msg = "SE350";
    char* received_msg = "OS";

    msg_envelope = (msg_buf_t*)request_memory_block();

    strncpy(msg_envelope->msg_data, sent_msg, 5);
    send_message(PID_P3, msg_envelope);

    // Blocking until P3 sends
    msg_envelope = (msg_buf_t*)receive_message(NULL);

    if (strcmp(msg_envelope->msg_data, received_msg) == 2) {
        printf("G005_test: test 2 OK\r\n");
    } else {
        printf("G005_test: test 2 FAIL\r\n");
        test_results[result_pid] = 0;
    }

    release_memory_block(msg_envelope);
    test_ran[result_pid] = 1;
    set_process_priority(g_test_procs[result_pid].pid, 3);

    while (1) {
        release_processor();
    }
}

/* Sends message to user process 2 */
void test_proc_p2_3(void) {
    int result_pid = 2;
    msg_buf_t* msg_envelope = 0;
    char* sent_msg = "OS";
    char* received_msg = "SE350";

    msg_envelope = (msg_buf_t*)request_memory_block();

    strncpy(msg_envelope->msg_data, sent_msg, 2);
    send_message(PID_P2, msg_envelope);

    msg_envelope = (msg_buf_t*)receive_message(NULL);

    if (strcmp(msg_envelope->msg_data, received_msg) == 5) {
        printf("G005_test: test 3 OK\r\n");
    } else {
        printf("G005_test: test 3 FAIL\r\n");
        test_results[result_pid] = 0;
    }

    release_memory_block(msg_envelope);
    test_ran[result_pid] = 1;
    set_process_priority(g_test_procs[result_pid].pid, 3);

    while (1) {
        release_processor();
    }
}

void test_proc_p2_4(void) {
    int i = 1;
    int counter = 0;
    int result_pid = 3;

    while (1) {
        if (i % 2 == 0) {
            if (++counter == 5) {
                set_process_priority(PID_P5, HIGH);
                break;
            } else {
                release_processor();
            }
        }
        i++;
    }
    test_ran[result_pid] = 1;
    if (test_ran[PID_P5 - 1] == 1) {
        printf("G005_test: test 4 FAIL\r\n");
        test_results[result_pid] = 0;
    } else {
        printf("G005_test: test 4 OK\r\n");
    }
    set_process_priority(PID_P4, LOW);
    while (1) {
        release_processor();
    }
}

void test_proc_p2_5(void) {
    int i = 1;
    int counter = 0;
    int result_pid = 4;

    while (1) {
        if (i % 2 == 0) {
            if (++counter == 10) {
                set_process_priority(PID_P4, HIGH);
                break;
            } else {
                release_processor();
            }
        }
        i++;
    }
    test_ran[result_pid] = 1;
    if (test_ran[PID_P4 - 1] == 0) {
        printf("G005_test: test 5 FAIL\r\n");
        test_results[result_pid] = 0;
    } else {
        printf("G005_test: test 5 OK\r\n");
    }
    set_process_priority(PID_P5, LOW);
    while (1) {
        release_processor();
    }
}

/******************************************************************************
 * Part 1 : Their user procs
 ******************************************************************************/

/**
 * @brief: a process that prints 2x5 lowercase letters
 */
void test_proc_p1_a_1(void) {
    int i = 0;
    int counter = 0;
    int ret_val = 100;
    while ( 1 ) {

        if ( i != 0 && i % 5 == 0 ) {
            uart1_put_string("\n\r");
            counter++;
            if ( counter == 2 ) {
                ret_val = set_process_priority(PID_P2, HIGH);
                break;
            } else {
                ret_val = release_processor();
            }
#ifdef DEBUG_0
            printf("proc1: ret_val = %d \n", ret_val);
#endif /* DEBUG_0 */
        }
        uart1_put_char('a' + i % 10);
        i++;
    }
    uart1_put_string("proc1 end of testing\n\r");
    while ( 1 ) {
        release_processor();
    }
}

/**
 * @brief: a process that prints 4x5 numbers
 */
void test_proc_p1_a_2(void) {
    int i = 0;
    int ret_val = 20;
    int counter = 0;

    while ( 1) {
        if ( i != 0 && i % 5 == 0 ) {
            uart1_put_string("\n\r");
            counter++;
            if ( counter == 4 ) {
                ret_val = set_process_priority(PID_P1, HIGH);
                break;
            } else {
                ret_val = release_processor();
            }
#ifdef DEBUG_0
            printf("proc2: ret_val=%d\n", ret_val);
#endif /* DEBUG_0 */
        }
        uart1_put_char('0' + i % 10);
        i++;
    }
    uart1_put_string("proc2 end of testing\n\r");
    while ( 1 ) {
        release_processor();
    }
}

void test_proc_p1_a_3(void) {

    while (1) {
        uart1_put_string("proc3: \n\r");
        release_processor();
    }
}

void test_proc_p1_a_4(void) {
    while (1) {
        uart1_put_string("proc4: \n\r");
        release_processor();
    }
}

void test_proc_p1_a_5(void) {
    while (1) {
        uart1_put_string("proc5: \n\r");
        release_processor();
    }
}

void test_proc_p1_a_6(void) {
    while (1) {
        uart1_put_string("proc6: \n\r");
        release_processor();
    }
}

/**
 * Prints five uppercase letters and request a memory block.
 */
void test_proc_p1_b_1(void) {
    int i = 0;
    void* p_mem_blk;
    while ( 1 ) {
        if ( i != 0 && i % 5 == 0 ) {
            uart1_put_string("\n\r");
            p_mem_blk = request_memory_block();
#ifdef DEBUG_0
            printf("proc1: p_mem_blk=0x%x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart1_put_char('A' + i % 26);
        i++;
    }
}

/**
 * Prints five numbers and then releases a memory block
 */
void test_proc_p1_b_2(void) {
    int i = 0;
    int ret_val = 20;
    void* p_mem_blk;

    p_mem_blk = request_memory_block();
    set_process_priority(PID_P2, MEDIUM);
    while ( 1) {
        if ( i != 0 && i % 5 == 0 ) {
            uart1_put_string("\n\r");
            ret_val = release_memory_block(p_mem_blk);

#ifdef DEBUG_0
            printf("proc2: ret_val=%d\n", ret_val);
#endif /* DEBUG_0 */

            if ( ret_val == -1 ) {
                break;
            }
        }
        uart1_put_char('0' + i % 10);
        i++;
    }
    uart1_put_string("proc2: end of testing\n\r");
    set_process_priority(PID_P2, LOWEST);

    while ( 1 ) {
        release_processor();
    }
}

void test_proc_p1_b_3(void) {
    int i = 0;

    while (1) {
        if ( i < 2 ) {
            uart1_put_string("proc3: \n\r");
        }
        release_processor();
        i++;
    }
}

void test_proc_p1_b_4(void) {
    int i = 0;

    while (1) {
        if ( i < 2 ) {
            uart1_put_string("proc4: \n\r");
        }
        release_processor();
        i++;
    }
}

void test_proc_p1_b_5(void) {
    int i = 0;

    while (1) {
        if ( i < 2 )  {
            uart1_put_string("proc5: \n\r");
        }
        release_processor();
        i++;
    }
}

void test_proc_p1_b_6(void) {
    int i = 0;

    while (1) {
        if ( i < 2 )  {
            uart1_put_string("proc6: \n\r");
        }
        release_processor();
        i++;
    }
}


/** Timer Test Process
*       
*   This process will be testing the speed of our tasks.
*   We'll be testing:
*
*       1) send_message
*       2) reseive_message
*       3) request_memory_block 
*
*/

void timer_test_proc_p4(void) {
    const uint32_t sample_size = 1000;
    uint32_t start_time, end_time;
    uint32_t elapsed_time_send[sample_size];
    uint32_t elapsed_time_receive[sample_size];
    uint32_t elapsed_time_req[sample_size];
    msg_buf_t* envelope;
    char * msg;
    uint32_t i = 0;

    for (i = 0; i < sample_size; i++) {
        start_time = test_timer_count;
        envelope = (msg_buf_t*)request_memory_block();
        end_time = test_timer_count;
        elapsed_time_req[i] = (end_time - start_time);


        envelope->msg_type = DEFAULT;
        msg = "A";
        strncpy(envelope->msg_data, msg, strlen(msg));
        start_time = test_timer_count;
        send_message(PID_TIMER_TEST_PROC, envelope);
        end_time = test_timer_count;
        elapsed_time_send[i] = (end_time - start_time);
        release_memory_block(envelope);

        start_time = test_timer_count;
        envelope = (msg_buf_t*)receive_message(NULL);
        end_time = test_timer_count;
        elapsed_time_receive[i] = (end_time - start_time);
        release_memory_block(envelope);
    }
    printf("Request Memory Block Timings:\r\n");
    for (i = 0; i < sample_size; i++) {
        printf("%d\r\n", elapsed_time_req[i]);
    }
    printf("Send Message Timings:\r\n");
    for (i = 0; i < sample_size; i++) {
        printf("%d\r\n", elapsed_time_send[i]);
    }
    printf("Receive Message Timings: \r\n");
    for (i = 0; i < sample_size; i++) {
        printf("%d\r\n", elapsed_time_receive[i]);
    }
    test_ran[6] = 1;

    while (1) {
        release_processor();
    }
}
