#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "utils.h"

#define __SVC_0  __svc_indirect(0)


#define DEFAULT         0
#define KCD_REG         1
#define CRT_DISPLAY     2
#define COUNT_REPORT    3
#define WAKEUP10        4

typedef struct {
    uint32_t msg_type;
    char msg_data[1];
} msg_buf_t;

void* k_receive_message_i(int32_t*);

extern int32_t k_send_message(int32_t, void*);
#define send_message(process_id, message_envelope) _send_message((int32_t)k_send_message, process_id, message_envelope)
extern int32_t __SVC_0 _send_message(int32_t p_func, int32_t process_id, void* message_envelope);

extern void* k_receive_message(int32_t*);
#define receive_message(sender_id) _receive_message((void *)k_receive_message, sender_id)
extern void* _receive_message(void* p_func, int32_t* sender_id) __SVC_0;

extern int32_t k_delayed_send(int32_t, void*, int32_t);
#define delayed_send(process_id, message_envelope, delay) _delayed_send((int32_t)k_delayed_send, process_id, message_envelope, delay)
extern int32_t __SVC_0 _delayed_send(int32_t p_func, int32_t process_id, void* message_envelope, int32_t delay);

#endif
