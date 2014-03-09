#ifndef _K_PROC_H_
#define _K_PROC_H_

#include "k_memory.h"

extern mem_blk_t* irq_message_block;

void k_set_procs(void);
void null_proc(void);
void crt_proc(void);
void kcd_proc(void);

#endif
