#include "k_message.h"
#include "message.h"
#include "k_process.h"
#include "timer.h"

#define KERNEL_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE - KERNEL_MSG_HEADER_SIZE)
#define USER_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE + KERNEL_MSG_HEADER_SIZE)

extern linkedlist_t timeout_queue;
extern volatile uint32_t g_timer_count;

uint32_t k_init_message(void* message, uint32_t process_id) {
    message_t* msg = (message_t *)message;
    
    msg->msg_node.next = NULL;
    msg->msg_node.prev = NULL;
    msg->msg_node.value = msg;
    msg->sender_pid = ((pcb_t *)(current_pcb_node->value))->pid;
    msg->receiver_pid = process_id;
    msg->expiry = 0;
    
    return 0;
}

uint32_t k_send_message(uint32_t process_id, void* message_envelope) {
    message_t* message = (message_t *)KERNEL_MSG_ADDR(message_envelope);
    pcb_t *receiver;

    
    if(process_id < 1 || process_id >= NUM_PROCESSES) {
        return 1;
    }
    
    k_init_message(message, process_id);
    
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
    if(sender_id != NULL) {
        *sender_id = message->sender_pid;
    }
    
    return (void*)USER_MSG_ADDR(message);
}
/* 
 * Non-blocking version of receive_message
 * 
 * returns NULL if no messages available
 */
void* k_receive_message_i(int32_t* sender_id) {
    node_t* message_node;
    message_t* message;
    pcb_t* current_pcb;
    
    current_pcb = (pcb_t *)current_pcb_node->value;
    
    if(current_pcb->msg_queue.length == 0) {
        return NULL;
    }
    
    message_node = linkedlist_pop_front(&current_pcb->msg_queue);
    message = (message_t *)message_node->value;
		if(sender_id != NULL) {
			*sender_id = message->sender_pid;
		}
    
    return (void*)USER_MSG_ADDR(message);
}

uint32_t k_send_message_i(uint32_t process_id, void* message_envelope) {
    message_t* message = (message_t *)KERNEL_MSG_ADDR(message_envelope);
    pcb_t *receiver;
    
    if(process_id < 1 || process_id >= NUM_PROCESSES) {
        return 1;
    }
    
    k_init_message(message, process_id);
    
    receiver = pcbs[process_id];
    
    linkedlist_push_back(&receiver->msg_queue, &message->msg_node);
    
    if(receiver->state == MSG_BLOCKED) {
        k_pcb_msg_unblock(receiver);
    }
	return 0;
}
int32_t k_delayed_send(int32_t process_id, void* message_envelope, int32_t delay) {
    message_t* message = (message_t *)KERNEL_MSG_ADDR(message_envelope);
    node_t* iter;
    node_t* new_node = NULL;
    int i = 0;

    if(process_id < 1 || process_id >= NUM_PROCESSES || delay < 0) {
        return 1;
    }
    // If no delay, send message right away
    if (delay == 0) {
        return k_send_message(process_id, message_envelope);
    }

    // Initialize the message
    k_init_message(message, process_id);
    // Add the timer_count to keep track of expiry without linear search
    message->expiry = delay + g_timer_count;

    iter = timeout_queue.first;

    while(iter != NULL) {
        // If this is true, we found a place to insert into the queue
        if (((message_t *)iter->value)->expiry > delay) {
            new_node = &message->msg_node;
            new_node->prev = iter->prev;
            new_node->next = iter;
            if (iter->prev == NULL) {
                timeout_queue.first = new_node;
            } else {
                iter->prev->next = new_node;
            }
            iter->prev = new_node;
            timeout_queue.length++;
            return 0;
        }
        iter = iter->next;
    }
    // If we get here, then the delay time is greater than all that are in the queue
    linkedlist_push_back(&timeout_queue, &message->msg_node);
    return 0;
}
