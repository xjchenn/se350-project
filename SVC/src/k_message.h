#ifndef _K_MESSAGE_H_
#define _K_MESSAGE_H_

#include "utils.h"
#include "linkedlist.h"

#define KERNEL_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE - KERNEL_MSG_HEADER_SIZE)
#define USER_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE + KERNEL_MSG_HEADER_SIZE)

typedef struct {
    node_t msg_node;
    uint32_t sender_pid;
    uint32_t receiver_pid;
    uint32_t expiry;

    uint32_t msg_type;
    char msg_data[1];
} message_t;

void* k_receive_message(int32_t* sender_id);
void* k_receive_message_i(int32_t* sender_id);

int32_t k_send_message(int32_t process_id, void* message_envelope);
int32_t k_send_message_i(int32_t process_id, void* message_envelope);

int32_t k_delayed_send(int32_t process_id, void* message_envelope, int32_t delay);

#endif
