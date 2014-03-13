#include "printf.h"
#include "uart_polling.h"
#include "wall_clock.h"
#include "message.h"
#include "string.h"
#include "rtx.h"

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
            sprintf(buffer, "%02d:%02d:%02d\r", (currentTime / 3600) % 24, (currentTime / 60) % 60, (currentTime % 60));
            strncpy(envelope->msg_data, buffer, strlen(buffer));
            send_message(PID_CRT, envelope); // -> crt_proc -> uart_i_proc -> frees envelope

            // received a wall clock update
            currentTime++;
            
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
                        send_message(PID_CLOCK, envelope); // show the 00:00:00 immediately
                    }
                    running = 1;
                    break;
                }

                case 'T': {
                    running = 0;
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

