/* thread.h: Thread Control Block (TCB) structure and related definitions */
#ifndef THREAD_H
#define THREAD_H

#include "defs.h"
#include "heap.h"
#include "helpers.h"
#include "scheduler.h"
// #include "dispatch.h"

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
int thread_func();


/* Shared globals: declare as extern in header, define in one .c file. */
extern int thread_ids;
extern int curr_tid;
extern tcb *current_thread;     // pointer to currently running thread
extern kbool_t done[MAX_THREADS];
extern int counter[MAX_THREADS];
extern uint32_t *f[MAX_THREADS];        /*Array of pointers to function addresses*/
// extern uint32_t *stacks[MAX_THREADS];  /* stack pointers */
extern tcb *micros_threads[MAX_THREADS]; /*Array of pointers to TCBs*/
extern thread_heap_t* ready_queue; /* Ready queue for threads */
extern uint8_t thread_stacks[MAX_THREADS][STACK_SIZE_PER_THREAD] __attribute__((aligned(16))); /* Statically allocated stacks */
// extern uint32_t thread_stacks[MAX_THREADS]; /* stack pointers for each thread */
#endif /* THREAD_H */
