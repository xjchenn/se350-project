#ifndef _USER_PROC_H_
#define _USER_PROC_H_

void set_user_procs(void);

void wall_clock_proc(void);     // p11
void priority_change_proc(void);// p10
void stress_test_proc_a(void);  // p7
void stress_test_proc_b(void);  // p8
void stress_test_proc_c(void);  // p9
void timer_test_proc(void);

#endif
