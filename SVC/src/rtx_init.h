#ifndef _K_RTX_INIT_
#define _K_RTX_INIT_

#include "utils.h"

#define __SVC_0  __svc_indirect(0)

extern void k_rtx_init(void);
#define rtx_init() _rtx_init((uint32_t)k_rtx_init)
extern int _rtx_init(uint32_t p_func) __SVC_0;

#endif
