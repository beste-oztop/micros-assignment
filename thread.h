/* thread.h: Thread Control Block (TCB) structure and related definitions */
#ifndef THREAD_H
#define THREAD_H

#include "defs.h"
#include "heap.h"

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

    uint32_t execution_time;    // C: total execution time per job
    uint32_t remaining_time;    // C': remaining time for current job
    uint32_t period;            // T: period for periodic tasks
    priority_t priority;        // derived from period (smaller period = higher priority)

    // for periodic jobs
    uint32_t next_arrival;      // next job arrival time (for periodic tasks)
    int max_jobs;               // maximum number of periodic instances (-1 = infinite, 0 = one-shot)
    int jobs_done;              // number of completed job instances
    kbool_t is_periodic;        // true if this is a periodic task

    struct thread_control_block *next; // next thread in the queue
    struct thread_control_block *left_child; // left child in the tree
    struct thread_control_block *right_child; // right child in the tree
    struct thread_control_block *parent; // parent thread
    struct thread_control_block *prev; // previous thread in the queue
    int state; // state of the thread (e.g., ready, running, exited) 0 - Idle, 1 - Busy
} tcb;

typedef struct struct_schedparams
{
    uint32_t execution_time; // C
    uint32_t period;  // T, rate-monotonic scheduling -> priorities inversely proportional to period
    int max_jobs;               // how many periodic instances to run (-1 = infinite)
} struct_schedparams_t;

/* Function declarations */
int thread_create(void *stack, void *func, void *args);
void exit_thread(void);
int get_tcb(void);
void init_thread_pool(void);      // Initialize thread pool
void cleanup_thread_pool(void);   // Cleanup thread pool
tcb* get_current_thread(int tid);


/* Shared globals: declare as extern in header, define in one .c file. */
extern int thread_ids;
extern int curr_tid;
extern tcb *current_thread;     // pointer to currently running thread
extern kbool_t done[MAX_THREADS];
extern int counter[MAX_THREADS];
extern uint32_t *f[MAX_THREADS];        /*Array of pointers to function addresses*/
extern uint32_t *stacks[MAX_THREADS];  /* stack pointers */
extern tcb *micros_threads[MAX_THREADS]; /*Array of pointers to TCBs*/
extern thread_heap_t* ready_queue; /* Ready queue for threads */


#endif /* THREAD_H */
