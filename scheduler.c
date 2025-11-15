#include "scheduler.h"
#include "thread.h"
#include "heap.h"
// #include <stdio.h>
// #include <stdlib.h>

/* Rate-Monotonic Scheduling Implementation */
// extern tcb *micros_threads[MAX_THREADS]; /*Array of pointers to TCBs*/
// extern thread_heap_t* ready_queue;  /*Use the global ready queue*/
// extern int curr_tid; /*Current thread ID*/
// extern tcb *current_thread; /*Current thread pointer*/



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
void schedule_rm_org(void){
         __asm__ volatile ("cli");  // Clear interrupt flag

        puts("Scheduling using Rate-Monotonic Scheduling...  / ");
        if (!ready_queue) {
                puts("Error: ready_queue not initialized!\n");
                return;
        }

        puts("Current ready queue state:\n");
        heap_print(ready_queue);

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

                puts("Examining thread ID: ");
                putint(node.tcb->tid);
                puts(" with period: ");
                putint(node.tcb->period);
                puts(" / ");
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
                        puts("Failed to reinsert thread into ready queue!\n");
                }
        }
//             while(1){
//         __asm__ volatile ("hlt");
//     }
        // Schedule the best candidate
        if (best_candidate) {
                // Preempt current thread if different
                // if (curr != best_candidate) {
                //         // Reinsert current thread to the ready queue if still runnable
                //         if (curr && curr->state == THREAD_READY && curr->remaining_time > 0) {
                //                 if (heap_insert(ready_queue, curr) != 0) {
                //                         #ifdef KERNEL_MODE
                //                                 puts("Failed to reinsert current thread into ready queue!\n");
                //                         #else
                //                                 printf("Failed to reinsert current thread into ready queue!\n");
                //                         #endif
                //                 }
                //         }
                //        #ifdef KERNEL_MODE
                //                 puts("Switching to higher priority thread.\n");
                //                 puts("Preempting thread ID: ");
                //                 putint(curr_tid);
                //                 puts("--> Switching to thread ID: ");
                //                 putint(best_candidate->tid);
                //                 puts("\n");
                //         #else
                //                 printf("Switching to higher priority thread.\n");
                //         #endif
                // }

                // Set the best candidate as running
                // Print debug info
                #ifdef KERNEL_MODE
                        puts("Scheduled thread ID: ");
                        putint(best_candidate->tid);
                        puts(" with period: ");
                        putint(best_candidate->period);
                        puts("\n");
                #endif
                // FIXME: Things get messy after this point
                best_candidate->state = THREAD_RUNNING;
                curr_tid = best_candidate->tid;
                current_thread = best_candidate;
        } else {
                // No runnable threads found
                #ifdef KERNEL_MODE
                        puts("No runnable threads found. CPU idle.\n");
                #endif
                curr_tid = -1;
                current_thread = NULL;
        }

        #ifdef KERNEL_MODE
                puts("Ready queue state after scheduling:\n");
                heap_print(ready_queue);
                puts("Scheduling complete.\n");
        #endif

        // Re-enable interrupts at the end
        __asm__ volatile ("sti");  // Set interrupt flag
}


/* Schedule threads using Rate-Monotonic Scheduling */
void schedule_rm(void){
    __asm__ volatile ("cli");  // Disable interrupts

    puts("RM Scheduler called\n");

    if (!ready_queue) {
        puts("Error: ready_queue not initialized!\n");
        __asm__ volatile ("sti");
        return;
    }

    // Temporarily store all threads we examine
    heap_node_t node;
    tcb *candidates[MAX_THREADS];
    int candidate_count = 0;
    tcb *best_thread = NULL;

    // Remove all threads from heap and find the best one
    while (heap_remove(ready_queue, &node) == 0) {
        tcb *thread = node.tcb;

        if (!thread) continue;

        // Skip exited threads
        if (thread->state == THREAD_EXITED) {
            puts("Skipping exited thread ");
            putint(thread->tid);
            puts("\n");
            continue;
        }

        // Skip threads that completed all jobs
        if (thread->is_periodic && thread->jobs_done >= thread->max_jobs) {
            puts("Thread ");
            putint(thread->tid);
            puts(" completed all jobs\n");
            thread->state = THREAD_EXITED;
            continue;
        }

        // Store this candidate
        candidates[candidate_count++] = thread;

        // Keep track of best (first valid thread from min-heap is best)
        if (!best_thread) {
            best_thread = thread;
        }
    }

    // Put all candidates EXCEPT the best one back into the ready queue
    for (int i = 0; i < candidate_count; i++) {
        if (candidates[i] != best_thread) {
            if (heap_insert(ready_queue, candidates[i]) != 0) {
                puts("Failed to reinsert thread!\n");
            }
        }
    }

    if (best_thread) {
        // Set as current thread
        best_thread->state = THREAD_RUNNING;
        current_thread = best_thread;
        curr_tid = best_thread->tid;

        puts("Scheduled thread ID: ");
        putint(curr_tid);
        puts(" (period=");
        putint(best_thread->period);
        puts(")\n");
    } else {
        // No runnable threads
        puts("No runnable threads found!\n");
        current_thread = NULL;
        curr_tid = -1;
    }

    __asm__ volatile ("sti");  // Re-enable interrupts
}