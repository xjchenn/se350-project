#include "k_message.h"
#include "message.h"
#include "k_process.h"
#include "timer.h"

extern linkedlist_t timeout_queue;
extern volatile uint32_t g_timer_count;

/**
 * assumes input params are correct
 * @param  message      not null
 * @param  process_id   >= 0 and <= NUM_PROCESSES
 * @return              0 if successful
 */
int32_t k_init_message(void* message, int32_t process_id) {
    message_t* msg = (message_t*)message;

    msg->msg_node.next = NULL;
    msg->msg_node.prev = NULL;
    msg->msg_node.value = msg;
    msg->sender_pid = ((pcb_t*)(current_pcb_node->value))->pid;
    msg->receiver_pid = process_id;
    msg->expiry = 0;

    return 0;
}

/**
 * preempting send
 */
int32_t k_send_message(int32_t process_id, void* message_envelope) {
    message_t* message = (message_t *)KERNEL_MSG_ADDR(message_envelope);
    pcb_t *receiver;
	
		__disable_irq();
    
    if(process_id < 1 || process_id >= NUM_PROCESSES) {
				__enable_irq();
        return 1;
    }

    k_init_message(message, process_id);

    receiver = pcbs[process_id];

    linkedlist_push_back(&receiver->msg_queue, &message->msg_node);

    if (receiver->state == MSG_BLOCKED) {
        k_pcb_msg_unblock(receiver);
        if(receiver->priority <= ((pcb_t *)current_pcb_node->value)->priority) {
						__enable_irq();
            k_release_processor();
						__disable_irq();
        }	
    }
		__enable_irq();
    return 0;
}

/**
 * non preempting send
 */
int32_t k_send_message_i(int32_t process_id, void* message_envelope) {
    message_t* message = (message_t*)KERNEL_MSG_ADDR(message_envelope);
    pcb_t* receiver;

    if (process_id < 1 || process_id >= NUM_PROCESSES) {
        return -1;
    }

    k_init_message(message, process_id);

    receiver = pcbs[process_id];

    linkedlist_push_back(&receiver->msg_queue, &message->msg_node);

    if (receiver->state == MSG_BLOCKED) {
        k_pcb_msg_unblock(receiver);
    }

    return 0;
}

/**
 * blocking receive
 */
void* k_receive_message(int32_t* sender_id) {
    node_t* message_node;
    message_t* message;

    pcb_t* current_pcb;
		
		__disable_irq();
	
    current_pcb = (pcb_t *)current_pcb_node->value;
    
    while(current_pcb->msg_queue.length == 0) {
        current_pcb->state = MSG_BLOCKED;
				__enable_irq();
        k_release_processor();
				__disable_irq();
    }

    message_node = linkedlist_pop_front(&current_pcb->msg_queue);
    message = (message_t*)message_node->value;

    if (sender_id != NULL) {
        *sender_id = message->sender_pid;
    }
		__enable_irq();
    return (void*)USER_MSG_ADDR(message);
}

/**
 * non blocking receive
 */
void* k_receive_message_i(int32_t* sender_id) {
    node_t* message_node;
    message_t* message;
    pcb_t* current_pcb = (pcb_t*)current_pcb_node->value;

    if (current_pcb->msg_queue.length == 0) {
        return NULL;
    }

    message_node = linkedlist_pop_front(&current_pcb->msg_queue);
    message = (message_t*)message_node->value;

    if (sender_id != NULL) {
        *sender_id = message->sender_pid;
    }

    return (void*)USER_MSG_ADDR(message);
}

int32_t k_delayed_send(int32_t process_id, void* message_envelope, int32_t delay) {
    message_t* message = (message_t*)KERNEL_MSG_ADDR(message_envelope);
    message_t* current_message;
    node_t* queue_iter;
    node_t* new_node;

    if (process_id < 1 || process_id >= NUM_PROCESSES || delay < 0) {
        return -1;
    }

    // If no delay, send message right away
    if (delay == 0) {
        return k_send_message(process_id, message_envelope);
    }

    k_init_message(message, process_id);
    message->expiry = delay + g_timer_count; // Add the timer_count to keep track of expiry without linear search

    // insert new message into sorted queue (in desc. order of expiry time)
    queue_iter = timeout_queue.first;
    while (queue_iter != NULL) {
        current_message = (message_t*)queue_iter->value;

        if (current_message->expiry > delay) {
            // insert new_node before queue_iter
            new_node = &message->msg_node;
            new_node->prev = queue_iter->prev;
            new_node->next = queue_iter;

            // queue_iter is first node in queue
            if (queue_iter->prev == NULL) {
                timeout_queue.first = new_node;
            } else {
                (queue_iter->prev)->next = new_node;
            }

            // queue_iter is last node in queue
            if (queue_iter->next == NULL) {
                timeout_queue.last = new_node;
            }

            queue_iter->prev = new_node;
            timeout_queue.length++;
            return 0;
        }

        queue_iter = queue_iter->next;
    }

    // If we get here, then the delay time is greater than all that are in the queue
    linkedlist_push_back(&timeout_queue, &message->msg_node);
    return 0;
}
