#include "k_message.h"
#include "message.h"
#include "k_process.h"

#define KERNEL_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE - KERNEL_MSG_HEADER_SIZE)
#define USER_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE + KERNEL_MSG_HEADER_SIZE)

msg_hist_t sent_msg_buffer[MSG_BUFFER_SIZE];
msg_hist_t received_msg_buffer[MSG_BUFFER_SIZE];

uint32_t sent_msg_buffer_size = 0;
uint32_t received_msg_buffer_size = 0;

uint32_t front_sent_msg_idx = 0;
uint32_t end_sent_msg_idx = 0;

uint32_t front_received_msg_idx = 0;
uint32_t end_received_msg_idx = 0;


uint32_t k_init_message(void* message, uint32_t process_id) {
    message_t* msg = (message_t *)message;
    
    msg->msg_node.next = NULL;
    msg->msg_node.prev = NULL;
    msg->msg_node.value = message;
    msg->sender_pid = ((pcb_t *)(current_pcb_node->value))->pid;
    msg->receiver_pid = process_id;
    
    return 0;
}

uint32_t copy_message_to_history(const message_t* message, msg_hist_t* history) {
    history->sender = message->sender_pid;
    history->receiver = message->receiver_pid;
    history->msg_type = message->msg_type;
    //strncpy here
    
    return 0;
}

void buffer_increment(uint32_t* ptr, uint32_t max_size) {
    if(*ptr == (max_size - 1)) {
        ptr = 0;
    } else {
        ptr++;
    }
}

uint32_t track_msg(msg_hist_t* buffer, uint32_t* front, uint32_t* end, uint32_t* size, const message_t* msg) {
    if(size == 0) {
        copy_message_to_history(msg, &buffer[*end++]);
        buffer_increment(end, MSG_BUFFER_SIZE);
        *size++;
        return 0;
    }
    
    buffer_increment(end, MSG_BUFFER_SIZE);
    copy_message_to_history(msg, &buffer[*end++]);
    
    if(*size < 10) {
        *size++;
    } else {
        buffer_increment(front, MSG_BUFFER_SIZE);
    }
    
    return 0;
}

uint32_t track_sent_msg(const message_t* msg) {
    return track_msg(sent_msg_buffer, &front_sent_msg_idx, &end_sent_msg_idx, &sent_msg_buffer_size, msg);
}

uint32_t track_received_msg(const message_t* msg) {
    return track_msg(received_msg_buffer, &front_received_msg_idx, &end_received_msg_idx, &received_msg_buffer_size, msg);
}

uint32_t k_send_message(uint32_t process_id, void* message_envelope) {
    message_t* message = (message_t *)KERNEL_MSG_ADDR(message_envelope);
    pcb_t *receiver;
    
    if(process_id < 1 || process_id >= NUM_PROCESSES) {
        return 1;
    }
    
    k_init_message(message_envelope, process_id);
    
    receiver = pcbs[process_id];
    
    linkedlist_push_back(&receiver->msg_queue, &message->msg_node);
    
    track_sent_msg(message);
    
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
    
    track_received_msg(message);
    
    return (void*)USER_MSG_ADDR(message);
}
