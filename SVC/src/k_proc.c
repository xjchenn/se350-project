#include "k_proc.h"
#include "uart.h"
#include "utils.h"
#include "process.h"
#include "message.h"
#include "memory.h"
#include "string.h"
#include "timer.h"
#include "wall_clock.h"
#include "printf.h"

typedef struct {
    uint32_t pid;
    char cmd[10];
} kcd_cmd_t;

kcd_cmd_t commands[10];
uint32_t num_of_cmds_reg = 0;
char msg_data[USER_DATA_BLOCK_SIZE - 4];

proc_image_t k_proc_table[NUM_K_PROCESSES];

void k_set_procs(void) {
    uint32_t i = 0;
    uint32_t j;

    for (i = 0; i < NUM_K_PROCESSES; i++) {
        k_proc_table[i].stack_size = STACK_SIZE;
        k_proc_table[i].priority = LOWEST;
    }

    k_proc_table[0].pid = PID_NULL;
    k_proc_table[0].proc_start = &null_proc;

    k_proc_table[1].pid = PID_TIMER_IPROC;
    k_proc_table[1].proc_start = &timer_i_process;

    k_proc_table[2].pid = 15;
    k_proc_table[2].proc_start = &c_UART0_IRQHandler;

    k_proc_table[3].pid = 12;
    k_proc_table[3].priority = HIGHEST;
    k_proc_table[3].proc_start = &kcd_proc;

    k_proc_table[4].pid = 13;
    k_proc_table[4].priority = HIGHEST;
    k_proc_table[4].proc_start = &crt_proc;

    k_proc_table[5].pid = 11;
    k_proc_table[5].priority = HIGHEST;
    k_proc_table[5].proc_start = &wall_clock_proc;

    for (i = 0; i < 10; i++) {
        for(j = 0; j < 10; j++) {
            commands[i].cmd[j] = '\0';
        }
    }
}

void null_proc(void) {
    while (1) {
        release_processor();
    }
}

void crt_proc(void) {
    msg_buf_t* msg;

    while (1) {
        msg = receive_message(NULL);
        if (msg->msg_type == CRT_DISPLAY) {
            send_message(PID_UART_IPROC, msg);
            read_interrupt();
        } else {
            release_memory_block(msg);
        }
    }
}

void reset_msg_data() {
    int i;
    
    for(i = 0; i < USER_DATA_BLOCK_SIZE - 4; i++) {
        msg_data[i] = '\0';
    }
}

void kcd_proc(void) {
    msg_buf_t* msg = NULL;
    int32_t sender_id;
    uint32_t msg_data_len;
    char* itr;
    uint32_t i = 0;
    char buffer[10];
    
    while (1) {
        msg = receive_message(&sender_id);

        if (msg->msg_type == DEFAULT) {
            msg_data_len = strlen(msg->msg_data);
            strncpy(msg_data, msg->msg_data, msg_data_len);
            
            //msg->msg_type = CRT_DISPLAY;
            //send_message(PID_CRT, msg); // -> crt_proc -> uart_i_proc -> frees msg
            
            itr = msg_data;
            if (msg_data[0] == '%') {
                while (*itr != ' ' && *itr != '\r') {
                    if (*itr == '\0') {
                        *itr++;
                    } else {
                        buffer[i++] = *itr++;
                    }
                }
                
                for (i = 0; i < num_of_cmds_reg; i++) {
                    if (!strcmp(commands[i].cmd, buffer)) {
                        msg = (msg_buf_t*)request_memory_block();
                        msg->msg_type = DEFAULT;
                        strncpy(msg->msg_data, msg_data, msg_data_len);
                        send_message(commands[i].pid, msg);
                        break;
                    }
                }
            }

            for(i = 0; i < 10; i++) {
                buffer[i] = '\0';
            }
            
            i = 0;
            
            reset_msg_data();
            continue;

            // since we're forwarding the msg, we should not free it in this case

        } else if(msg->msg_type == KCD_REG) {
            if(num_of_cmds_reg != 10) {
                commands[num_of_cmds_reg].pid = sender_id;
                msg_data_len = strlen(msg->msg_data);
                strncpy(commands[num_of_cmds_reg].cmd, msg->msg_data, msg_data_len);
                num_of_cmds_reg++;
            }

            k_release_memory_block_i(msg); // done reading msg, can now free it
        }
    }
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
    cmd = "%WR";
    envelope = (msg_buf_t*)request_memory_block();
    envelope->msg_type = KCD_REG;
    strncpy(envelope->msg_data, cmd, strlen(cmd));
    send_message(PID_KCD, envelope); // envelope is now considered freed memory
    
    cmd = "%WS";
    envelope = (msg_buf_t*)request_memory_block();
    envelope->msg_type = KCD_REG;
    strncpy(envelope->msg_data, cmd, strlen(cmd));
    send_message(PID_KCD, envelope); // envelope is now considered freed memory
    
    cmd = "%WT";
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
            sprintf(buffer, "%02d:%02d:%02d\r", (currentTime / 3600) % 100, (currentTime / 60) % 60, (currentTime % 60));
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
                    running = 1;
                    currentTime = 0;

                    envelope = (msg_buf_t*)request_memory_block();
                    envelope->msg_type = DEFAULT;
                    send_message(PID_CLOCK, envelope); // show the 00:00:00 immediately

                    break;
                }

                case 'S': {
                    running = 1;
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

                    envelope = (msg_buf_t*)request_memory_block();
                    envelope->msg_type = DEFAULT;
                    send_message(PID_CLOCK, envelope); // show the 00:00:00 immediately

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
