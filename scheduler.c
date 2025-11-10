#include "scheduler.h"
#include "thread.h"
#include "heap.h"
// #include <stdio.h>
// #include <stdlib.h>

/* Rate-Monotonic Scheduling Implementation */
extern tcb *micros_threads[MAX_THREADS]; /*Array of pointers to TCBs*/
extern thread_heap_t* ready_queue;  /*Use the global ready queue*/
extern int curr_tid; /*Current thread ID*/
extern tcb *current_thread; /*Current thread pointer*/



/* comparator: higher priority = smaller period. Tie-breaker: smaller id. */
static int rm_cmp(const void *a, const void *b){
        uint32_t thread_period_a = ((tcb *)a)->period;
        uint32_t thread_period_b = ((tcb *)b)->period;

        if (thread_period_a < thread_period_b) {
                return -1; // a has higher priority
        } else if (thread_period_a > thread_period_b) {
                return 1; // b has higher priority
        } else {
                // Tie-breaker: smaller TID has higher priority
                int tid_a = ((tcb *)a)->tid;
                int tid_b = ((tcb *)b)->tid;
                return (tid_a < tid_b) ? -1 : (tid_a > tid_b) ? 1 : 0;
        }
}

/* Schedule threads using Rate-Monotonic Scheduling
*/
void schedule_rm(void){

        #ifdef KERNEL_MODE
                puts("Scheduling using Rate-Monotonic Scheduling...\n");
                busy_wait();
        #else
                printf("Scheduling using Rate-Monotonic Scheduling...\n");
        #endif
        if (!ready_queue) {
                #ifdef KERNEL_MODE
                        puts("Error: ready_queue not initialized!\n");
                #else
                        printf("Error: ready_queue not initialized!\n");
                #endif
                return;
        }

       // Find the highest priority thread in the ready queue 
       tcb *curr = (curr_tid>=0 && curr_tid<MAX_THREADS) ? micros_threads[curr_tid] : NULL;

       // If current thread is running, check if it should continue or be preempted
       tcb *best_candidate = NULL;
        if (curr && curr->state == THREAD_READY && curr->remaining_time > 0) {
                 best_candidate = curr;
        }

        // Look at the ready queue, find highest priority ready thread
        heap_node_t node;
        tcb *candidates[MAX_THREADS];
        int candidate_count = 0;
        while(heap_remove(ready_queue, &node) == 0) {
                // Debug info
                #ifdef KERNEL_MODE
                        puts("Examining thread ID: ");
                        putint(node.tcb->tid);
                        puts(" with period: ");
                        putint(node.tcb->period);
                        puts("\n");
                #endif
                tcb *thread = node.tcb;
                if (!thread) continue; // safety check

                // Skip exited threads
                if (thread->state == THREAD_EXITED) continue;
                // Skip threads that have completed their jobs
                if (thread->is_periodic && thread->jobs_done >= thread->max_jobs) continue;
                // Skip threads that do not have remaining time
                if (thread->remaining_time <= 0) continue;

                candidates[candidate_count++] = thread;
                // Keep track of the best candidate
                if (!best_candidate){
                        best_candidate = thread;
                } else {
                       // Compare priorities
                        if (rm_cmp(thread, best_candidate) < 0) {
                                best_candidate = thread;
                        }
                }

        }
       
        for (int i = 0; i < candidate_count; i++) {
                // Reinsert candidates back into the ready queue
                if (heap_insert(ready_queue, candidates[i]) != 0) {
                        #ifdef KERNEL_MODE
                                puts("Failed to reinsert thread into ready queue!\n");
                        #else
                                printf("Failed to reinsert thread into ready queue!\n");
                        #endif
                }
        }

        // If current thread is still best, reinsert it
        if (best_candidate && best_candidate == curr) {
                if (heap_insert(ready_queue, curr) != 0) {
                        #ifdef KERNEL_MODE
                                puts("Failed to reinsert current thread into ready queue!\n");
                        #else
                                printf("Failed to reinsert current thread into ready queue!\n");
                        #endif
                }
        }

        // Schedule the best candidate
        if (best_candidate) {
                // Preempt current thread if different
                if (curr != best_candidate) {
                        // Reinsert current thread to the ready queue if still runnable
                        if (curr && curr->state == THREAD_READY && curr->remaining_time > 0) {
                                if (heap_insert(ready_queue, curr) != 0) {
                                        #ifdef KERNEL_MODE
                                                puts("Failed to reinsert current thread into ready queue!\n");
                                        #else
                                                printf("Failed to reinsert current thread into ready queue!\n");
                                        #endif
                                }
                        }
                       #ifdef KERNEL_MODE
                                puts("Switching to higher priority thread.\n");
                                puts("Preempting thread ID: ");
                                putint(curr_tid);
                                puts("--> Switching to thread ID: ");
                                putint(best_candidate->tid);
                                puts("\n");
                        #else
                                printf("Switching to higher priority thread.\n");
                        #endif
                }

                // Set the best candidate as running
                // Print debug info
                #ifdef KERNEL_MODE
                        puts("Scheduled thread ID: ");
                        putint(best_candidate->tid);
                        puts(" with period: ");
                        putint(best_candidate->period);
                        puts("\n");
                #endif
                best_candidate->state = THREAD_RUNNING;
                curr_tid = best_candidate->tid;
                current_thread = best_candidate;
        } else {
                // No runnable threads found
                curr_tid = -1;
                current_thread = NULL;
        }

}        