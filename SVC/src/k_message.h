#ifndef _K_MESSAGE_H_
#define _K_MESSAGE_H_

#include "utils.h"
#include "linkedlist.h"

#define KERNEL_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE - KERNEL_MSG_HEADER_SIZE)
#define USER_MSG_ADDR(MESSAGE) (void *)((uint32_t)MESSAGE + KERNEL_MSG_HEADER_SIZE)

#define MSG_PREVIEW_SIZE 16
#define MSG_BUFFER_SIZE 10

typedef struct {
    // hidden to the user (6 words)
    node_t msg_node;        // 3 words 
    uint32_t sender_pid;    // 1 word
    uint32_t receiver_pid;  // 1 word
    uint32_t expiry;        // 1 word

    // user gets pointer here
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

void* k_receive_message(int32_t* sender_id);
void* k_receive_message_i(int32_t* sender_id);

int32_t k_send_message(int32_t process_id, void* message_envelope);
int32_t k_send_message_i(int32_t process_id, void* message_envelope);

int32_t k_delayed_send(int32_t process_id, void* message_envelope, int32_t delay);

void k_print_msg_logs(void);

#endif
