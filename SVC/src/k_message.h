#ifndef _K_MESSAGE_H_
#define _K_MESSAGE_H_

#include "utils.h"
#include "linkedlist.h"

#define KERNEL_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE - KERNEL_MSG_HEADER_SIZE)
#define USER_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE + KERNEL_MSG_HEADER_SIZE)

#define MSG_PREVIEW_SIZE 16
#define MSG_BUFFER_SIZE 10

typedef struct {
    node_t msg_node;
    uint32_t sender_pid;
    uint32_t receiver_pid;
    uint32_t expiry;

    uint32_t msg_type;
    char msg_data[1];
} message_t;

typedef struct {
    uint32_t sender;
    uint32_t receiver;
    uint32_t msg_type;
    //Add timestamp here
    char msg_preview[16];
} msg_hist_t;

extern msg_hist_t sent_msg_buffer[MSG_BUFFER_SIZE];
extern msg_hist_t received_msg_buffer[MSG_BUFFER_SIZE];
extern uint32_t sent_msg_buffer_size;
extern uint32_t received_msg_buffer_size;

void* k_receive_message(int32_t* sender_id);
void* k_receive_message_i(int32_t* sender_id);

int32_t k_send_message(int32_t process_id, void* message_envelope);
int32_t k_send_message_i(int32_t process_id, void* message_envelope);

int32_t k_delayed_send(int32_t process_id, void* message_envelope, int32_t delay);

void k_print_logs(msg_hist_t*, uint32_t);

#endif
