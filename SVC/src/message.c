#include "k_message.h"
#include "message.h"
#include "k_process.h"

#define KERNEL_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE - KERNEL_MSG_HEADER_SIZE)
#define USER_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE + KERNEL_MSG_HEADER_SIZE)

uint32_t k_init_message(void* message, uint32_t process_id) {
    message_t* msg = (message_t *)message;
    
    if(process_id < 1 || process_id >= NUM_PROCESSES) {
        return 1;
    }
    
    msg->msg_node.next = NULL;
    msg->msg_node.prev = NULL;
    msg->msg_node.value = message;
    msg->sender_pid = ((pcb_t *)(current_pcb_node->value))->pid;
    msg->receiver_pid = process_id;
    
    return 0;
}

uint32_t k_send_message(uint32_t process_id, void* message_envelope) {
    message_t* message = (message_t *)KERNEL_MSG_ADDR(message_envelope);
    pcb_t *receiver;
    
    k_init_message(message_envelope, process_id);
    
    receiver = pcbs[process_id];
    
    linkedlist_push_back(&receiver->msg_queue, &message->msg_node);
    
    if(receiver->state == MSG_BLOCKED) {
        k_pcb_msg_unblock(receiver);
        if(receiver->priority <= ((pcb_t *)current_pcb_node->value)->priority) {
            k_release_processor();
        }
    }
    
    return 0;
}

void* k_receive_message(int32_t* sender_id) {
    node_t* message_node;
    message_t* message;
    pcb_t* current_pcb;
    
    current_pcb = (pcb_t *)current_pcb_node->value;
    
    while(current_pcb->msg_queue.length == 0) {
        current_pcb->state = MSG_BLOCKED;
        k_release_processor();
    }
    
    message_node = linkedlist_pop_front(&current_pcb->msg_queue);
    message = (message_t *)message_node->value;
    *sender_id = message->sender_pid;
    
    return (void*)USER_MSG_ADDR(message);
}
