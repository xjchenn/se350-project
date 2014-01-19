/* @brief: rtx.h User API prototype, this is an example only
 * @author: Yiqing Huang
 * @date: 2014/01/07
 */
#include "utils.h"

#ifndef RTX_H_
#define RTX_H_

#define __SVC_0  __svc_indirect(0)

extern int k_release_processor(void);
#define release_processor() _release_processor((U32)k_release_processor)
extern int __SVC_0 _release_processor(U32 p_func);

#endif // !RTX_H_
