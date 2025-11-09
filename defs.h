#ifndef DEFNS_H
#define DEFNS_H

#define FALSE 0
#define TRUE 1
#define NULL ((void*)0)  // Define NULL as a void pointer

// #define KERNEL_MODE  // to enable puts function from helpers.h


#define MAX_THREADS 10
#define SIM_TIME 100  // time slice in milliseconds
#define STACK_SIZE 4096  // 4KB per thread stack

/* Thread states */
#define THREAD_IDLE    0
#define THREAD_READY   1
#define THREAD_RUNNING 2
#define THREAD_WAITING 3
#define THREAD_EXITED  4


#endif /* DEFNS_H */