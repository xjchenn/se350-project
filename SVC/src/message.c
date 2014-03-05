#include "k_message.h"
#include "message.h"
#include "k_process.h"

#define KERNEL_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE - KERNEL_MSG_HEADER_SIZE)
#define USER_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE + KERNEL_MSG_HEADER_SIZE)

uint32_t k_init_message(void* message) {
    message_t* msg = (message_t *)message;
    
    msg->msg_node.next = NULL;
    msg->msg_node.prev = NULL;
    msg->msg_node.value = message;
    msg->sender_pid = ((pcb_t *)(current_pcb_node->value))->pid;
    
    return 0;
}

uint32_t k_send_message(uint32_t process_id, void* message_envelope) {
    message_t* message = (message_t *)KERNEL_MSG_ADDR(message_envelope);
    pcb_t *receiver;
    
    k_init_message(message_envelope);
    message->receiver_pid = process_id;
    
    if(process_id < 1 || process_id >= NUM_PROCESSES) {
        return 1;
    }
    
    receiver = pcbs[process_id];
    
    return 0;
}
