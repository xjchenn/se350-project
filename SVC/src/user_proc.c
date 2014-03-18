#include "user_proc.h"
#include "rtx.h"

proc_image_t u_proc_table[NUM_USER_PROCESSES];

void set_user_procs(void) {
    u_proc_table[0].pid        = PID_CLOCK;
    u_proc_table[0].proc_start = &wall_clock_proc;
    u_proc_table[0].priority   = HIGHEST;

    u_proc_table[1].pid        = PID_A;
    u_proc_table[1].proc_start = &stress_test_proc_a;
    u_proc_table[1].priority   = MEDIUM;

    u_proc_table[2].pid        = PID_B;
    u_proc_table[2].proc_start = &stress_test_proc_b;
    u_proc_table[2].priority   = MEDIUM;

    u_proc_table[3].pid        = PID_C;
    u_proc_table[3].proc_start = &stress_test_proc_c;
    u_proc_table[3].priority   = MEDIUM;
}

/******************************************************************************
* Wall Clock P11
*******************************************************************************/

int is_numeric_char(char c) {
    return (c >= '0' && c <= '9') ? 1 : 0;
}

void wall_clock_proc(void) {    
    int32_t sender_id;
    msg_buf_t* envelope;
    char* cmd;
    char buffer[15];            // enough to store longest command "%WS hh:mm:ss\r\n\0"
    int32_t time_value;

    uint32_t running = 0;
    uint32_t currentTime = 0;   // in sec

    // register the command with kcd
    cmd = "%W";
    envelope = (msg_buf_t*)request_memory_block();
    envelope->msg_type = KCD_REG;
    strncpy(envelope->msg_data, cmd, strlen(cmd));
    send_message(PID_KCD, envelope); // envelope is now considered freed memory

    // run clock
    while (1) {
        envelope = receive_message(&sender_id);

        // guaranteed that there's at least 1 free memory block
        if (sender_id == PID_CLOCK && running == 1) {

            // even if clock is not running, we need to take back the block that we freed in the previous iteration so
            // we're guaranteed that we always have a block available
            envelope->msg_type = DEFAULT;
            delayed_send(PID_CLOCK, envelope, CLOCK_INTERVAL); // clock don't need a message

            // print time to crt
            envelope = (msg_buf_t*)request_memory_block();
            envelope->msg_type = CRT_DISPLAY;
            sprintf(buffer, "\033[s\033[1;69H%02d:%02d:%02d\n\033[u", (currentTime / 3600) % 24, (currentTime / 60) % 60, (currentTime % 60)); // 69 x-offset = (80 col width - 11 char for HH:MM:SS)
            strncpy(envelope->msg_data, buffer, strlen(buffer));
            send_message(PID_CRT, envelope); // -> crt_proc -> uart_i_proc -> frees envelope

            // received a wall clock update
            currentTime++;
            currentTime = currentTime % (60 * 60 * 24);

        } else if (sender_id == PID_KCD) {
            strncpy(buffer, envelope->msg_data, 15);
            release_memory_block(envelope); // since we already copied out the data into our buffer

            if (buffer[0] != '%' || buffer[1] != 'W') {
                DEBUG_PRINT("wall_clock_proc received invalid message from kcd");
            }

            switch (buffer[2]) {
                case 'R': {
                    currentTime = 0;

                    if (running == 0) {
                        envelope = (msg_buf_t*)request_memory_block();
                        envelope->msg_type = DEFAULT;
                        send_message(PID_CLOCK, envelope); // show the 00:00:00 immediately
                    }

                    running = 1;
                    break;
                }

                case 'S': {
                    if (strlen(buffer) != strlen("%WS HH:MM:SS\r\n")) {
                        println("wall_clock_proc recieved invalid format");
                        goto INPUT_ERROR;

                    } else if (buffer[3] != ' ') {
                        println("wall_clock_proc did a space after WS - buffer[3]=%c", buffer[3]);
                        goto INPUT_ERROR;

                    } else if (buffer[6] != ':' || buffer[9] != ':') {
                        println("wall_clock_proc did not see 2 colons in WS - buffer[6]=%c buffer[9]=%c", buffer[6], buffer[9]);
                        goto INPUT_ERROR;

                    } else if (!is_numeric_char(buffer[4]) || !is_numeric_char(buffer[5])) {
                        println("wall_clock_proc received an invalid hour %c%c", buffer[4], buffer[5]);
                        goto INPUT_ERROR;

                    } else if (!is_numeric_char(buffer[7]) || !is_numeric_char(buffer[8])) {
                        println("wall_clock_proc received an invalid minute %c%c", buffer[7], buffer[8]);
                        goto INPUT_ERROR;

                    } else if (!is_numeric_char(buffer[10]) || !is_numeric_char(buffer[11])) {
                        println("wall_clock_proc received an invalid second %c%c", buffer[10], buffer[11]);
                        goto INPUT_ERROR;

                    }

                    currentTime = 0;

                    // 0 1 2 3 4 5 6 7 8 9 A B
                    // % W S _ H H : M M : S S

                    // get hour
                    time_value = 0;
                    time_value += (buffer[4] - '0') * 10;
                    time_value += (buffer[5] - '0');
                    currentTime += time_value * 3600;

                    // get minutes
                    time_value = 0;
                    time_value += (buffer[7] - '0') * 10;
                    time_value += (buffer[8] - '0');
                    currentTime += time_value * 60;

                    // get seconds
                    time_value = 0;
                    time_value += (buffer[10] - '0') * 10;
                    time_value += (buffer[11] - '0');
                    currentTime += time_value;

                    if (running == 0) {
                        envelope = (msg_buf_t*)request_memory_block();
                        envelope->msg_type = DEFAULT;
                        send_message(PID_CLOCK, envelope); // show the HH:MM:SS immediately
                    }

                    running = 1;
                    break;

                    INPUT_ERROR:
                    continue;
                }

                case 'T': {
                    running = 0;

                    // clear the clock
                    envelope = (msg_buf_t*)request_memory_block();
                    envelope->msg_type = CRT_DISPLAY;
                    sprintf(buffer, "\033[s\033[1;69H%11s\n\033[u", ' ');
                    strncpy(envelope->msg_data, buffer, strlen(buffer));
                    send_message(PID_CRT, envelope); // -> crt_proc -> uart_i_proc -> frees envelope

                    break;
                }

                default: {
                    DEBUG_PRINT("wall_clock_proc received invalid command");
                    break;
                }
            }

        } else {
            release_memory_block(envelope); // clock got turned off
        }
    }
}

/******************************************************************************
* Stress Test P7
* TODO add desc
*******************************************************************************/

void stress_test_proc_a(void) {
    while (1) {
        release_processor();
    }
}

/******************************************************************************
* Stress Test P8
* TODO add desc
*******************************************************************************/

void stress_test_proc_b(void) {
    while (1) {
        release_processor();
    }
}

/******************************************************************************
* Stress Test P9
* TODO add desc
*******************************************************************************/

void stress_test_proc_c(void) {
    while (1) {
        release_processor();
    }
}
