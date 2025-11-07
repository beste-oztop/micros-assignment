/* thread.h: Thread Control Block (TCB) structure and related definitions */
#ifndef THREAD_H
#define THREAD_H

#define MAX_THREADS 10


typedef signed char kbool_t;
typedef unsigned short int uint16_t;
typedef unsigned long int uint32_t;
typedef unsigned char priority_t;

typedef struct thread_control_block {
    int tid; // thread ID
    uint32_t bp; // base pointer
    uint32_t sp; // stack pointer
    uint32_t entry; // entry point
    int flag; // general purpose flag
    struct thread_control_block *next; // next thread in the queue
    struct thread_control_block *left_child; // left child in the tree
    struct thread_control_block *right_child; // right child in the tree
    struct thread_control_block *parent; // parent thread
    int state; // state of the thread (e.g., ready, running, exited)
} tcb;


/* Shared globals: declare as extern in header, define in one .c file. */
extern int thread_ids;
extern kbool_t done[MAX_THREADS];
extern int counter[MAX_THREADS] = {0};
extern uint32_t *f[MAX_THREADS];        /*Array of pointers to function addresses*/
extern uint32_t *stacks[MAX_THREADS] = {};  /* stack pointers */
extern tcb *micros_threads[MAX_THREADS]; /*Array of pointers to TCBs*/

#endif /* THREAD_H */
