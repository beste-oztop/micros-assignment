#include "scheduler.h"
// #include <stdio.h>
// #include <stdlib.h>

/* Rate-Monotonic Scheduling Implementation */
extern tcb *micros_threads[MAX_THREADS]; /*Array of pointers to TCBs*/
// extern thread_heap_t* ready_queue;  // FIXME: ADD THIS - use the global ready queue


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
        // REMOVE THIS LINE: thread_heap_t *ready_queue = heap_create(rm_cmp);
        thread_heap_t *ready_queue = heap_create(rm_cmp);  // FIXME: instead of creating new ready queue, use global one
        if (!ready_queue) {
                #ifdef KERNEL_MODE
                        puts("Error: ready_queue not initialized!\n");
                #else
                        printf("Error: ready_queue not initialized!\n");
                #endif
                return;
    }

        /* per-thread deadlines kept externally because struct does not have a 'deadline' field */
        uint32_t deadlines[MAX_THREADS];

        /* initialize per-thread bookkeeping */
        for (int i = 0; i < MAX_THREADS; ++i){
                if (!micros_threads[i]) { // skip NULL entries
                        #ifdef KERNEL_MODE
                                puts("Skipping null entries!\n");
                        #else
                                printf("Skipping null entries!\n");
                        #endif
                        continue;
                }
                micros_threads[i]->remaining_time = 0;           /* no job at time 0 unless next_arrival == 0 */
                if (micros_threads[i]->next_arrival <= 0) {
                        /* normalize next_arrival: if negative or zero, set to 0 */
                        micros_threads[i]->next_arrival = 0;
                }
                deadlines[i] = micros_threads[i]->next_arrival + micros_threads[i]->period; /* first deadline */
        }
        tcb *running = NULL;
        int missed_deadlines = 0;

        for (int t = 0; t < SIM_TIME; ++t){
                /* release jobs whose release time == t */
                for (int i = 0; i < MAX_THREADS; ++i){
                        if (micros_threads[i]->next_arrival == t){
                                /* release a new job */
                                micros_threads[i]->remaining_time = micros_threads[i]->execution_time;
                                deadlines[i] = t + micros_threads[i]->period;
                                /* push into ready queue */
                                heap_push(ready_queue, micros_threads[i]);
                                /* schedule next release */
                                micros_threads[i]->next_arrival += micros_threads[i]->period;
                        }
                }

                /* check for deadline misses (jobs whose deadline <= t and still have remaining) */
                for (int i = 0; i < MAX_THREADS; ++i){
                        if (micros_threads[i]->remaining_time > 0 && deadlines[i] <= t){
                                /* record miss and drop the job (common policy) */
                                missed_deadlines++;
                                micros_threads[i]->remaining_time = 0;
                                /* if it was in the heap, remove it lazily: heap_pop will skip zero-remaining entries below */
                        }
                }

                /* select highest-priority ready task */
                /* ensure peeked item is still runnable (remaining_time > 0) */
                tcb *next = NULL;
                while ((heap_peek(ready_queue, &next)) != NULL){
                        if (next->remaining_time > 0) break;
                        /* stale entry (job finished or deadline missed), remove it */
                        heap_pop(ready_queue);
                        next = NULL;
                }

                /* preempt if needed */
                if (next != NULL){
                        if (running != next){
                                /* preempt running (if any) by pushing it back to ready if it still has remaining */
                                if (running && running->remaining_time > 0){
                                        heap_push(ready_queue, running);
                                }
                                /* take new task from heap */
                                heap_pop(ready_queue); /* remove next from heap */
                                running = next;
                        } else {
                                /* running remains the same; it's already removed from heap previously */
                        }
                } else {
                        /* no ready job: idle */
                        if (running && running->remaining_time == 0) running = NULL;
                        /* running may continue if it still has remaining and wasn't preempted;
                             but by construction running should always be next highest priority and removed from heap.
                             If running == NULL, CPU is idle this tick.
                        */
                }

                /* execute one time unit on running task */
                if (running && running->remaining_time > 0){
                        running->remaining_time -= 1;
                        /* if finished, running will be cleared next loop iteration */
                        if (running->remaining_time == 0){
                                /* job completed; running becomes NULL to let heap pick next job next tick */
                                running = NULL;
                        }
                } else {
                        /* idle tick */
                }
        }

        /* cleanup */
        heap_destroy(ready_queue);

        /* report missed deadlines (adapt printing mechanism to your environment) */
        // printf("RM schedule finished. Missed deadlines: %d\n", missed_deadlines);
}