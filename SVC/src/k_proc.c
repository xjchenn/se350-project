#include "k_proc.h"
#include "uart.h"
#include "utils.h"
#include "process.h"
#include "message.h"
#include "memory.h"
#include "string.h"

typedef struct {
	uint32_t pid;
	char* cmd;
} kcd_cmd_t;

kcd_cmd_t commands[10];
uint32_t num_of_cmds_reg = 0;
char msg_data[USER_DATA_BLOCK_SIZE - 4];

proc_image_t k_proc_table[NUM_K_PROCESSES];

void k_set_procs(void) {
    uint32_t i = 0;
    
    for (i = 0; i < NUM_K_PROCESSES; i++) {
        k_proc_table[i].stack_size = STACK_SIZE;
        k_proc_table[i].priority = LOWEST;
    }
    
    k_proc_table[0].pid = 0;
    k_proc_table[0].proc_start = &null_proc;
    
    k_proc_table[1].pid = 14;
    k_proc_table[1].proc_start = &timer_i_process;
    
    k_proc_table[2].pid = 15;
    k_proc_table[2].proc_start = &c_UART0_IRQHandler;
    
		k_proc_table[3].pid = 12;
    k_proc_table[3].priority = HIGHEST;
    k_proc_table[3].proc_start = &kcd_proc;
		
    k_proc_table[4].pid = 13;
    k_proc_table[4].priority = HIGHEST;
    k_proc_table[4].proc_start = &crt_proc;
		
		for(i = 0; i < 10; i++) {
				commands[i].cmd = NULL;
		}
}

void null_proc(void) {
    while (1) {
        release_processor();
    }
}

void crt_proc(void) {
    msg_buf_t* msg;
    
    while(1) {
        msg = receive_message(NULL);
        
        if(msg->msg_type == CRT_DISPLAY) {
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
	
	while(1) {
			msg = receive_message(&sender_id);
        if(msg->msg_type == DEFAULT) {
						msg_data_len = strlen(msg->msg_data);
						strncpy(msg_data, msg->msg_data, msg_data_len);
					
            msg->msg_type = CRT_DISPLAY;
            send_message(PID_CRT, msg);
						itr = msg_data;
						
						if(msg_data[0] == '%') {
								while(*itr != ' ' && *itr != '\0') {
										buffer[i++] = *itr++;
								}
								
								for(i = 0; i < num_of_cmds_reg; i++) {
									if(strcmp(commands[i].cmd, buffer)) {
											msg = request_memory_block();
											msg->msg_type = DEFAULT;
											strncpy(msg->msg_data, msg_data, msg_data_len);
											send_message(commands[i].pid, msg);
											break;
									}
								}
						}
	
						for(i = 0; i < strlen(buffer); i++) {
							buffer[i] = '\0';
						}
						
						reset_msg_data();
						
						continue;
        } else if(msg->msg_type == KCD_REG) {
            if(num_of_cmds_reg == 10) {
                release_memory_block(msg);
                continue;
			}
						
						commands[num_of_cmds_reg].pid = sender_id;
						msg_data_len = strlen(msg->msg_data);
						strncpy(commands[num_of_cmds_reg].cmd, msg->msg_data, msg_data_len);
						num_of_cmds_reg++;
        } else {
            release_memory_block(msg);
        }
	}
}

// TODO move to own file with other message related functions
void timer_i_process(void) {
    return;
}
