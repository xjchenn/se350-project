#include "k_message.h"
#include "message.h"
#include "k_process.h"
#include "timer.h"
#include "string.h"
#include "printf.h"

extern linkedlist_t timeout_queue;
extern volatile uint32_t g_timer_count;

msg_hist_t sent_msg_buffer[MSG_BUFFER_SIZE];
msg_hist_t received_msg_buffer[MSG_BUFFER_SIZE];

uint32_t sent_msg_buffer_size = 0;
uint32_t received_msg_buffer_size = 0;

uint32_t front_sent_msg_idx = 0;
uint32_t end_sent_msg_idx = 0;

uint32_t front_received_msg_idx = 0;
uint32_t end_received_msg_idx = 0;

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

uint32_t copy_message_to_history(const message_t* message, msg_hist_t* history) {
    history->sender = message->sender_pid;
    history->receiver = message->receiver_pid;
    history->msg_type = message->msg_type;
    strncpy(history->msg_preview, message->msg_data, MSG_PREVIEW_SIZE);

    return 0;
}

void buffer_increment(uint32_t* ptr, uint32_t max_size) {
    if (*ptr == (max_size - 1)) {
        *ptr = 0;
    } else {
        (*ptr)++;
    }
}

uint32_t track_msg(msg_hist_t* buffer, uint32_t* front, uint32_t* end, uint32_t* size, const message_t* msg) {
    if (*size == 0) {
        copy_message_to_history(msg, &buffer[*end]);
        buffer_increment(end, MSG_BUFFER_SIZE);
        (*size)++;
        return 0;
    }

    copy_message_to_history(msg, &buffer[*end]);
    buffer_increment(end, MSG_BUFFER_SIZE);

    if (*size < 10) {
        (*size)++;
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

/**
 * preempting send
 */
int32_t k_send_message(int32_t process_id, void* message_envelope) {
    message_t* message = (message_t*)KERNEL_MSG_ADDR(message_envelope);
    pcb_t* receiver;

    __disable_irq();

    if (process_id < 1 || process_id >= NUM_PROCESSES) {
        __enable_irq();
        return 1;
    }

    k_init_message(message, process_id);

    receiver = pcbs[process_id];

    linkedlist_push_back(&receiver->msg_queue, &message->msg_node);

    track_sent_msg(message);

    if (receiver->state == MSG_BLOCKED) {

        k_pcb_msg_unblock(receiver);
        if (receiver->priority <= ((pcb_t*)current_pcb_node->value)->priority) {
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

    track_sent_msg(message);

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

    current_pcb = (pcb_t*)current_pcb_node->value;

    while (current_pcb->msg_queue.length == 0) {
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

    track_received_msg(message);
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

    track_received_msg(message);
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

        if (message->expiry < current_message->expiry) {
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

void print_log(msg_hist_t buffer[], uint32_t front_idx) {
    uint32_t i;
    uint32_t idx;
    msg_hist_t msg;

    for (i = 0, idx = front_idx; i < MSG_BUFFER_SIZE; i++, buffer_increment(&idx, MSG_BUFFER_SIZE)) {
        msg = buffer[i];
        println("[%d] Sender:%d Receiver:%d Type:%d Msg:\"%s\"", i, msg.sender, msg.receiver, msg.msg_type, msg.msg_preview);
    }
}

void k_print_msg_logs() {
    println("SENT MESSAGES\r\n");
    print_log(sent_msg_buffer, front_sent_msg_idx);

    println("");

    println("RECEIVED MESSAGES\r\n");
    print_log(received_msg_buffer, front_received_msg_idx);
}
