#ifndef _K_MESSAGE_H_
#define _K_MESSAGE_H_

#include "utils.h"
#include "linkedlist.h"

#define MSG_PREVIEW_SIZE 16
#define MSG_BUFFER_SIZE 10

typedef struct {
    node_t msg_node;
    uint32_t sender_pid;
    uint32_t receiver_pid;
    
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

uint32_t k_send_message(uint32_t process_id, void* message_envelope);
void* k_receive_message(int32_t* sender_id);
uint32_t delayed_send(uint32_t process_id, void* message_envelope, uint32_t delay);

#endif
