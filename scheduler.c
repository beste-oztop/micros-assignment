#include "scheduler.h"
#include "thread.h"
#include "heap.h"
// #include <stdio.h>
// #include <stdlib.h>

/* Rate-Monotonic Scheduling Implementation */
extern tcb *micros_threads[MAX_THREADS]; /*Array of pointers to TCBs*/
extern thread_heap_t* ready_queue;  /*Use the global ready queue*/


/*
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

*/


/* comparator: higher priority = smaller period. Tie-breaker: smaller id. */
static int rm_cmp(const void *a, const void *b){
        const tcb *t1 = (const tcb *)a;
        const tcb *t2 = (const tcb *)b;
        if (t1->period != t2->period) return (t1->period < t2->period) ? -1 : 1;
        return (t1->tid < t2->tid) ? -1 : (t1->tid > t2->tid);
}

/*
Any thread that later becomes idle is not put back in the ready queue.
*/
void schedule_rm(void){
        if (!ready_queue) {
                #ifdef KERNEL_MODE
                        puts("Error: ready_queue not initialized!\n");
                #else
                        printf("Error: ready_queue not initialized!\n");
                #endif
                return;
        }
        /* Remove and schedule the highest-priority ready thread (smallest period).
           Preempt the currently running thread only if the newly selected thread
           has a strictly smaller period (higher priority). Exited threads are skipped. */
        heap_node_t node;

        /* Keep trying until we either schedule a runnable thread or the queue is empty */
        while (heap_remove(ready_queue, &node) == 0) {
                tcb *next = node.tcb;
                if (!next) continue;
                if (next->state == THREAD_EXITED) {
                        /* Skip exited threads */
                        // puts("Finished job:");
                        // putint(next->tid);
                        continue;
                }

                /* Current thread (may be NULL / -1) */
                extern int curr_tid;
                extern tcb *current_thread;
                tcb *curr = (curr_tid >= 0) ? micros_threads[curr_tid] : NULL;

                /* If the candidate is the same as current, just keep running it */
                if (curr && curr == next) {
                        current_thread = curr;
                        curr->state = 1; /* mark as running/ready-busy */
                        // puts("Continuing thread:");
                        // putint(curr->tid);
                        return;
                }

                /* Determine periods; treat 0 as very large (non-periodic / lowest priority) */
                uint32_t next_period = (next->period == 0) ? 0xFFFFFFFFu : next->period;
                uint32_t curr_period = (curr && curr->period == 0) ? 0xFFFFFFFFu :
                                                           (curr ? curr->period : 0xFFFFFFFFu);

                if (curr && curr->state != THREAD_EXITED) {
                        /* Compare priorities: smaller period => higher priority */
                        if (next_period < curr_period) {
                                /* Preempt current: reinsert current if it still has remaining work */
                                // puts("\n"); //newline;
                                // puts("Preempting thread:");
                                // putint(curr->tid);
                                // puts(" -> Running:");
                                // putint(next->tid);

                                curr->state = THREAD_READY; /* mark ready */
                                if (curr->remaining_time > 0) {
                                        heap_insert(ready_queue, curr);
                                }
                                /* Dispatch next */
                                next->state = 1; /* running */
                                curr_tid = next->tid;
                                current_thread = next;
                                return;
                        } else {
                                /* Current continues to run; put the candidate back into the ready queue */
                                heap_insert(ready_queue, next);
                                // puts("\n"); //newline;
                                // puts("Kept running thread:");
                                // putint(curr->tid);
                                // puts(" (candidate returned):");
                                // putint(next->tid);
                                current_thread = curr;
                                return;
                        }
                } else {
                        /* No current running thread: dispatch next */
                        //  puts("\n"); //newline;
                        // puts("Dispatching thread:");
                        // putint(next->tid);
                        next->state = 1; /* running */
                        curr_tid = next->tid;
                        current_thread = next;
                        return;
                }
        }

        /* Nothing runnable in queue */
        return;
 
}