/**
 * @brief:  rtx.c kernel API implementations, this is only a skeleton.
 * @author: Yiqing Huang
 * @date:   2014/01/12
 */

#include "uart_polling.h"

/****************************************************************************
 ************************READ BEFORE YOU PROCEED FURTHER*********************
 ****************************************************************************
  To better strcture your code, you may want to split these functions
  into different files. For example, memory related kernel APIs in one file
  and process related API in another file.
*/

int k_release_processor(void)
{
    uart0_put_string("k_release_processor: entering\r\n");
	return 0;
}


