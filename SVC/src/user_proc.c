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

void display_error_on_crt(char* error_message, uint32_t n) {
    static const uint32_t max_error_length = 80;

    msg_buf_t* envelope;
    char error_buffer[max_error_length];

    if (strlen(error_message) + strlen("ERROR: \r\n") > max_error_length) {
        sprintf(error_buffer, "ERROR: display_error_on_crt got an error that's too long to show\r\n");
    } else {
        sprintf(error_buffer, "ERROR: %s\r\n", error_message);
    }

    while (n-- > 0) {
        *(error_message++) = '\0';
    }

    envelope = (msg_buf_t*)request_memory_block();
    envelope->msg_type = CRT_DISPLAY;
    strncpy(envelope->msg_data, error_buffer, strlen(error_buffer));
    send_message(PID_CRT, envelope); // -> crt_proc -> uart_i_proc -> frees envelope
}

void wall_clock_proc(void) {
    static const uint32_t buffer_size       = 15; // enough to store longest command "%WS hh:mm:ss\r\n\0"
    static const uint32_t error_buffer_size = 80; // more than enough for most error messages
    uint32_t i = 0;
    int32_t sender_id;
    msg_buf_t* envelope;
    char buffer[buffer_size];                        
    char error_buffer[error_buffer_size];   
    
    char* cmd = "%W";

    uint32_t running = 0;
    uint32_t currentTime = 0; // in sec

    // register the command with kcd
    envelope = (msg_buf_t*)request_memory_block();
    envelope->msg_type = KCD_REG;
    strncpy(envelope->msg_data, cmd, strlen(cmd));
    send_message(PID_KCD, envelope); // envelope is now considered freed memory

    // run clock
    while (1) {
        envelope = receive_message(&sender_id);

        // reset buffer before using it again
        for (i = 0; i < buffer_size; ++i) {
            buffer[i] = '\0';
        }

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
        } else {
            strncpy(buffer, envelope->msg_data, buffer_size);
            release_memory_block(envelope); // since we already copied out the data into our buffer

            if (strlen(buffer) < strlen("%W") || buffer[0] != '%' || buffer[1] != 'W') {
                // can also arrive here when the sender_pid is PID_CLOCK and runnning=0
                // in general, we ignore any non command messages
                continue;
            }

            switch (buffer[2]) {
                case 'R': {
                    if (strlen(buffer) != strlen("%WR\r\n")) {
                        sprintf(error_buffer, "wall_clock_proc WR recieved an invalid format length of %d", strlen(buffer));
                        display_error_on_crt(error_buffer, strlen(error_buffer));
                        continue;
                    }

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
                        sprintf(error_buffer, "wall_clock_proc WS recieved an invalid format length of %d", strlen(buffer));
                        display_error_on_crt(error_buffer, strlen(error_buffer));
                        continue;

                    } else if (buffer[3] != ' ') {
                        sprintf(error_buffer, "wall_clock_proc did not see a space after WS - buffer[3]=%c", buffer[3]);
                        display_error_on_crt(error_buffer, strlen(error_buffer));
                        continue;

                    } else if (buffer[6] != ':' || buffer[9] != ':') {
                        sprintf(error_buffer, "wall_clock_proc did not see 2 colons in WS - buffer[6]=%c buffer[9]=%c", buffer[6], buffer[9]);
                        display_error_on_crt(error_buffer, strlen(error_buffer));
                        continue;

                    } else if (!is_numeric_char(buffer[4]) || !is_numeric_char(buffer[5])) {
                        sprintf(error_buffer, "wall_clock_proc received an invalid hour - buffer[4]=%c buffer[5]=%c", buffer[4], buffer[5]);
                        display_error_on_crt(error_buffer, strlen(error_buffer));
                        continue;

                    } else if (!is_numeric_char(buffer[7]) || !is_numeric_char(buffer[8])) {
                        sprintf(error_buffer, "wall_clock_proc received an invalid minute - buffer[7]=%c buffer[8]=%c", buffer[7], buffer[8]);
                        display_error_on_crt(error_buffer, strlen(error_buffer));
                        continue;

                    } else if (!is_numeric_char(buffer[10]) || !is_numeric_char(buffer[11])) {
                        sprintf(error_buffer, "wall_clock_proc received an invalid second - buffer[10]=%c buffer[11]=%c", buffer[10], buffer[11]);
                        display_error_on_crt(error_buffer, strlen(error_buffer));
                        continue;
                    }

                    currentTime = 0;

                    // 0 1 2 3 4 5 6 7 8 9 A B
                    // % W S _ H H : M M : S S

                    // get hour
                    currentTime += substring_toi(&buffer[4], 2) * 3600;

                    // get minutes
                    currentTime += substring_toi(&buffer[7], 2) * 60;

                    // get seconds
                    currentTime += substring_toi(&buffer[10], 2);

                    if (running == 0) {
                        envelope = (msg_buf_t*)request_memory_block();
                        envelope->msg_type = DEFAULT;
                        send_message(PID_CLOCK, envelope); // show the HH:MM:SS immediately
                    }

                    running = 1;
                    break;
                }

                case 'T': {
                    if (strlen(buffer) != strlen("%WT\r\n")) {
                        sprintf(error_buffer, "wall_clock_proc WS recieved an invalid format length of %d", strlen(buffer));
                        display_error_on_crt(error_buffer, strlen(error_buffer));
                        continue;
                    }

                    running = 0;

                    // clear the clock
                    envelope = (msg_buf_t*)request_memory_block();
                    envelope->msg_type = CRT_DISPLAY;
                    sprintf(buffer, "\033[s\033[1;69H%11s\n\033[u", ' ');
                    strncpy(envelope->msg_data, buffer, strlen(buffer));
                    send_message(PID_CRT, envelope); // -> crt_proc -> uart_i_proc -> frees envelope

                    running = 0;
                    break;
                }

                default: {
                    sprintf(error_buffer, "wall_clock_proc received invalid command %c", buffer[2]);
                    display_error_on_crt(error_buffer, strlen(error_buffer))
                    break;
                }
            }
        }
    }
}

/******************************************************************************
* Stress Test A
* TODO add desc
*******************************************************************************/

void stress_test_proc_a(void) {
    int32_t sender_id;
    msg_buf_t* envelope;
    char* cmd = "%Z";
    char buffer[5]; // enough to store "%Z\r\n\0"

    uint32_t num = 0;

    envelope = (msg_buf_t*)request_memory_block();
    envelope->msg_type = KCD_REG;
    strncpy(envelope->msg_data, cmd, strlen(cmd));
    send_message(PID_KCD, envelope); // envelope is now considered freed memory

    while (1) {
        envelope = receive_message(&sender_id);
        strncpy(buffer, envelope->msg_data, 5);
        release_memory_block(envelope);

        if (buffer[0] == '%' && buffer[1] == 'Z') {
            break;
        }
    }

    while (1) {
        envelope = (msg_buf_t*)request_memory_block();
        envelope->msg_type = COUNT_REPORT;
        sprintf(envelope->msg_data, "%d", num++);
        send_message(PID_B, envelope);

        release_processor();
    }
}

/******************************************************************************
* Stress Test B
* TODO add desc
*******************************************************************************/

void stress_test_proc_b(void) {
    int32_t sender_id;
    msg_buf_t* envelope;

    while (1) {
        envelope = receive_message(&sender_id);
        send_message(PID_C, envelope);
    }
}

/******************************************************************************
* Stress Test C
* TODO add desc
*******************************************************************************/

void stress_test_proc_c(void) {
    int32_t sender_id;
    msg_buf_t* envelope;
    char buffer[10]; // enough to store 2^32 = 4294967296

    while (1) {
        // TODO remove when finished implementation
        release_processor();
    }

    // TODO create local queue

    while (1) {
        //TODO if (local queue empty) {
            envelope = receive_message(&sender_id);
        //} else {
            // TODO envelope = dequeue from local queue
        //}

        if (envelope->msg_type == COUNT_REPORT) {
            // TODO implement atoi in string.c
            if (atoi(envelope->msg_data) % 20 == 0) {
                envelope->msg_type = CRT_DISPLAY;
                send_message(PID_CRT, envelope);

                // add messages recieved in the next 10 sec to its on local queue
                // i.e. this is its own termination signal
                envelope = (msg_buf_t*)request_memory_block();
                envelope->msg_type = WAKEUP10;
                delayed_send(PID_C, envelope, 10 * 1000);
                
                while (1) {
                    envelope = receive_message(&sender_id);

                    if (envelope->msg_type == WAKEUP10) {
                        break; // got termination signal, stop enqueuing messages
                    } else {
                        // TODO put envelope on local queue
                    }
                }
            }
        }

        release_memory_block(envelope);
        release_processor();
    }
}
