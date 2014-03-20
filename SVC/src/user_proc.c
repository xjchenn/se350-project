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

    u_proc_table[4].pid        = PID_SET_PRIO;
    u_proc_table[4].proc_start = &priority_change_proc;
    u_proc_table[4].priority   = HIGHEST;
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
    const uint32_t buffer_size       = 15; // enough to store longest command "%WS hh:mm:ss\r\n\0"
    const uint32_t error_buffer_size = 80; // more than enough for most error messages
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
                    DEBUG_PRINT("wall_clock_proc received invalid command");
                    break;
                }
            }
            
            continue;
        }
    }
}

void priority_change_proc(void) {
    int32_t process_id;
    int32_t new_priority;
		int32_t msg_len;
    int32_t i = 0;
    char* cmd = "%C";
    msg_buf_t* envelope;
    const uint32_t buffer_size       = 10; // this can store "%C pid(2 chars) priority(1 char)\r\n"
    const uint32_t error_buffer_size = 80; // more than enough for most error messages
    char buffer[buffer_size];                        
    char error_buffer[error_buffer_size];

    // register the command with kcd
    envelope = (msg_buf_t*)request_memory_block();
    envelope->msg_type = KCD_REG;
    msg_len = strlen(cmd);
    strncpy(envelope->msg_data, cmd, msg_len);
    send_message(PID_KCD, envelope); // envelope is now considered freed memory

    while(1) {

        envelope = receive_message(NULL);

        // reset buffer before using it again
        for (i = 0; i < buffer_size; ++i) {
            buffer[i] = '\0';
        }
        strncpy(buffer, envelope->msg_data, buffer_size);
        release_memory_block(envelope); // since we already copied out the data into our buffer
        println(buffer);
        if (strlen(buffer) < strlen("%C") || buffer[0] != '%' || buffer[1] != 'C') {
            // in general, we ignore any non command messages
            continue;
        }
        if (strlen(buffer) == strlen("%C 00 0\r\n")) {
            if (!is_numeric_char(buffer[3]) || !is_numeric_char(buffer[4]) || !is_numeric_char(buffer[6])) {
                sprintf(error_buffer, "priority_change_proc received an input - buffer[3]=%cbuffer[4]=%c buffer[6]=%c", buffer[3], buffer[4], buffer[6]);
                display_error_on_crt(error_buffer, strlen(error_buffer));
                continue;

            } else {
                process_id = substring_toi(&buffer[3], 2);
                new_priority = substring_toi(&buffer[6], 1);

            }
        } else if (strlen(buffer) == strlen("%C 0 0\r\n")) {
            if (!is_numeric_char(buffer[3]) || !is_numeric_char(buffer[5])) {
                sprintf(error_buffer, "priority_change_proc received an input - buffer[3]=%c buffer[5]=%c", buffer[3], buffer[5]);
                display_error_on_crt(error_buffer, strlen(error_buffer));
                continue;
            } else {
                process_id = substring_toi(&buffer[3], 1);
                new_priority = substring_toi(&buffer[5], 1);
            }
        } else {
            sprintf(error_buffer, "priority_change_proc received invalid input");
            display_error_on_crt(error_buffer, strlen(error_buffer));
            continue;
        }


        if (process_id < 1 || process_id > 11) {
            sprintf(error_buffer, "process with id=%d not available or has restricted access", process_id);
            display_error_on_crt(error_buffer, strlen(error_buffer));
            continue;
        }

        if (new_priority < 1 || new_priority > 3) {
            sprintf(error_buffer, "process priority level %d not available", new_priority);
            display_error_on_crt(error_buffer, strlen(error_buffer));
            continue;   
        }
        sprintf(error_buffer, "test information: pid: %d, new priority: %d", process_id, new_priority);
        display_error_on_crt(error_buffer, strlen(error_buffer));
        set_process_priority(process_id, new_priority);
        continue;
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
