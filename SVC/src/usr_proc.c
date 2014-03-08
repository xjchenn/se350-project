#include "printf.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "message.h"
#include "string.h"
#include "rtx.h"

static const int NUM_PROCS = 6;
proc_image_t usr_proc_table[NUM_PROCS];
int32_t test_results[NUM_PROCS];

void usr_set_procs() {
    /*
    // Testing p1

    // a
    //usr_proc_table[0].priority = MEDIUM;
    //usr_proc_table[1].priority = MEDIUM;

    // b
    //usr_proc_table[0].priority = MEDIUM;
    //usr_proc_table[1].priority = HIGH;

    usr_proc_table[0].proc_start = &usr_proc_p1_b_1;
    usr_proc_table[1].proc_start = &usr_proc_p1_b_2;
    usr_proc_table[2].proc_start = &usr_proc_p1_b_3;
    usr_proc_table[3].proc_start = &usr_proc_p1_b_4;
    usr_proc_table[4].proc_start = &usr_proc_p1_b_5;
    usr_proc_table[5].proc_start = &usr_proc_p1_b_6;
    */

    usr_proc_table[0].proc_start = &usr_proc_p2_1;
    usr_proc_table[0].priority = MEDIUM;

    usr_proc_table[1].proc_start = &usr_proc_p2_2;
    usr_proc_table[1].priority = HIGH;

    usr_proc_table[2].proc_start = &usr_proc_p2_3;
    usr_proc_table[2].priority = LOW;

    usr_proc_table[3].proc_start = &usr_proc_p2_4;
    usr_proc_table[3].priority = LOW;

    usr_proc_table[4].proc_start = &usr_proc_p2_5;
    usr_proc_table[4].priority = LOW;

    usr_proc_table[5].proc_start = &usr_proc_p1_6; // always use p1 usr_proc for printing our test results
    usr_proc_table[5].priority = LOW;

    //printf("G005_test: START\r\n");
    //printf("G005_test: total 5 tests\r\n");
}

/******************************************************************************
 * Part 1 : Our user procs
 ******************************************************************************/

/**
 * Process prints five uppercase letters
 */
void usr_proc_p1_1() {
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
void usr_proc_p1_2() {
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
void usr_proc_p1_3() {
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
void usr_proc_p1_4() {
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
void usr_proc_p1_5() {
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
void usr_proc_p1_6() {
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
            // printf("G005_test: ");
            // printf("%d", passed);
            // printf("/5 OK\r\n");
            // printf("G005_test: ");
            // printf("%d", (5 - passed));
            // printf("/5 FAIL\r\n");
            // printf("G005_test: END\r\n");
            ranOnce = 1;
        }
        release_processor();
    }
}
/******************************************************************************
 * Part 2 : Our user procs
 ******************************************************************************/


void usr_proc_p2_1(void) {
    uint32_t target_pid = 0;
    uint32_t i = 0;
    msg_buf_t* msg_envelope = NULL;
    char msg[5];
    while (1) {
        if (i != 0 && i % 5 == 0) {
            target_pid = 2 + (i * 5 % 4); // send msg to p2 to p5 in round robin

            msg_envelope = (msg_buf_t*)request_memory_block();
            msg_envelope->msg_type = DEFAULT;
            strncpy(msg_envelope->msg_data, msg, 5);

            // printf("Sending  \"%s\" to p%d\r\n", msg, target_pid);
            //send_message(target_pid, msg_envelope);
            delayed_send(target_pid, msg_envelope, 100);
        }

        msg[i % 5] = 'A' + (i % 26);
        i++;

        release_processor();
    }
}

void usr_proc_p2_2(void) {
    uint32_t i = 0;
    int32_t sender_id = 0;
    msg_buf_t* msg_envelope = 0;

    while (1) {
        msg_envelope = (msg_buf_t*)receive_message(&sender_id);
        // printf("proc%s\r\n", "2");
        printf("Received \"%s\" from p%d in p2\r\n", (char*)msg_envelope->msg_data, (sender_id + 1));
        // uart1_put_string("Received : ");
        // uart1_put_string((char*)msg_envelope->msg_data);
        // uart1_put_string(" from ");
        // uart1_put_char('0' + (sender_id+1));
        // uart1_put_string("\r\n");

        for (i = 0; i < 0x0001; i++) {
            ; // nop to induce delay
        }

        release_memory_block(msg_envelope);
    }
}

void usr_proc_p2_3(void) {
    uint32_t i = 0;
    int32_t sender_id = 0;
    msg_buf_t* msg_envelope = 0;

    while (1) {
        msg_envelope = (msg_buf_t*)receive_message(&sender_id);
        // printf("proc3\r\n");
        printf("Received \"%s\" from p%d in p3\r\n", (char*)msg_envelope->msg_data, (sender_id + 1));
        // uart1_put_string("Received : ");
        // uart1_put_string((char*)msg_envelope->msg_data);
        // uart1_put_string(" from ");
        // uart1_put_char('0' + (sender_id+1));
        // uart1_put_string("\r\n");

        for (i = 0; i < 0x0001; i++) {
            ; // nop to induce delay
        }

        release_memory_block(msg_envelope);
    }
}

void usr_proc_p2_4(void) {
    uint32_t i = 0;
    int32_t sender_id = 0;
    msg_buf_t* msg_envelope = 0;

    while (1) {
        msg_envelope = (msg_buf_t*)receive_message(&sender_id);
        // printf("proc4\r\n");
        printf("Received \"%s\" from p%d in p4\r\n", (char*)msg_envelope->msg_data, (sender_id + 1));
        // uart1_put_string("Received : ");
        // uart1_put_string((char*)msg_envelope->msg_data);
        // uart1_put_string(" from ");
        // uart1_put_char('0' + (sender_id+1));
        // uart1_put_string("\r\n");

        for (i = 0; i < 0x0001; i++) {
            ; // nop to induce delay
        }

        release_memory_block(msg_envelope);
    }
}

void usr_proc_p2_5(void) {
    uint32_t i = 0;
    int32_t sender_id = 0;
    msg_buf_t* msg_envelope = 0;

    while (1) {
        msg_envelope = (msg_buf_t*)receive_message(&sender_id);
        // printf("proc5\r\n");
        printf("Received \"%s\" from p%d in p5\r\n", (char*)msg_envelope->msg_data, (sender_id + 1));
        // uart1_put_string("Received : ");
        // uart1_put_string((char*)msg_envelope->msg_data);
        // uart1_put_string(" from ");
        // uart1_put_char('0' + (sender_id+1));
        // uart1_put_string("\r\n");

        for (i = 0; i < 0x0001; i++) {
            ; // nop to induce delay
        }

        release_memory_block(msg_envelope);
    }
}

/******************************************************************************
 * Part 1 : Their user procs
 ******************************************************************************/

/**
 * @brief: a process that prints 2x5 lowercase letters
 */
void usr_proc_p1_a_1(void) {
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
void usr_proc_p1_a_2(void) {
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

void usr_proc_p1_a_3(void) {

    while (1) {
        uart1_put_string("proc3: \n\r");
        release_processor();
    }
}

void usr_proc_p1_a_4(void) {
    while (1) {
        uart1_put_string("proc4: \n\r");
        release_processor();
    }
}

void usr_proc_p1_a_5(void) {
    while (1) {
        uart1_put_string("proc5: \n\r");
        release_processor();
    }
}

void usr_proc_p1_a_6(void) {
    while (1) {
        uart1_put_string("proc6: \n\r");
        release_processor();
    }
}

/**
 * Prints five uppercase letters and request a memory block.
 */
void usr_proc_p1_b_1(void) {
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
void usr_proc_p1_b_2(void) {
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

void usr_proc_p1_b_3(void) {
    int i = 0;

    while (1) {
        if ( i < 2 ) {
            uart1_put_string("proc3: \n\r");
        }
        release_processor();
        i++;
    }
}

void usr_proc_p1_b_4(void) {
    int i = 0;

    while (1) {
        if ( i < 2 ) {
            uart1_put_string("proc4: \n\r");
        }
        release_processor();
        i++;
    }
}

void usr_proc_p1_b_5(void) {
    int i = 0;

    while (1) {
        if ( i < 2 )  {
            uart1_put_string("proc5: \n\r");
        }
        release_processor();
        i++;
    }
}

void usr_proc_p1_b_6(void) {
    int i = 0;

    while (1) {
        if ( i < 2 )  {
            uart1_put_string("proc6: \n\r");
        }
        release_processor();
        i++;
    }
}
