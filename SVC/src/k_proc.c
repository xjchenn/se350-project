#include "k_proc.h"
#include "uart.h"
#include "utils.h"
#include "process.h"
#include "message.h"
#include "memory.h"

typedef struct {
	uint32_t pid;
	char* cmd;
} kcd_cmd_t;

kcd_cmd_t commands[10];

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

void kcd_proc(void) {
	msg_buf_t* msg = NULL;
	
	while(1) {
		msg = receive_message(NULL);
        if(msg->msg_type == DEFAULT) {
            msg->msg_type = CRT_DISPLAY;
            send_message(PID_CRT, msg);
        } else if(msg->msg_type == KCD_REG) {
            
        } else {
            release_memory_block(msg);
        }
	}
}

// TODO move to own file with other message related functions
void timer_i_process(void) {
    return;
}
