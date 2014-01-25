#ifndef _USR_PROC_H_
#define _USR_PROC_H_

void set_procs();

/* Have 5 regular user processes */
void usr_proc_1();
void usr_proc_2();
void usr_proc_3();
void usr_proc_4();
void usr_proc_5();
void usr_proc_6();

/* Have 1 NULL process */
void null_proc();

#endif