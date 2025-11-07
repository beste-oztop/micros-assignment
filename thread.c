/* thread.c: thread management functions */
#include "thread.h"
#include "defs.h"

#ifdef KERNEL_MODE
#include "helpers.h"
#else
#include <stdlib.h>
#include <string.h>
#endif



int thread_ids = 0;             // next available TID (0-based internal)
int curr_tid = -1;              // current running thread TID (-1 = none)
tcb *current_thread = 0;        // pointer to currently running TCB
kbool_t done[MAX_THREADS] = {0};
int counter[MAX_THREADS] = {0};
uint32_t *f[MAX_THREADS] = {0};
uint32_t *stacks[MAX_THREADS] = {0};
tcb *micros_threads[MAX_THREADS] = {0};
thread_heap_t* ready_queue = NULL; // global ready queue heap


void exit_thread(void){
    /* Change the state of the current thread to exited
    Finished threads free their TCB slot for new thread_create() calls. */

    /*
    FIXME reference code from fifos delete once done
    current->state = 0;
    if(FIFOS == 1){
        int flag = schedule();
        if(flag==0)
            dispatch_first_run();
        else{
            __asm__ volatile ("hlt");
        }
    }
    else{
        __asm__ volatile ("hlt");
        // puts("danger");
        // dispatch_first_run();
    }*/

    tcb* curr_thread = get_current_thread(curr_tid);
    // here we need to check if job of thread finished or not.
    // If finished, set state to 0, otherwise update remaining execution time
}


/* Initialize the thread pool - must be called before using threads */
void init_thread_pool(void) {

    if (!ready_queue) {
        ready_queue = heap_create(MAX_THREADS);
        if (!ready_queue) {
            #ifdef KERNEL_MODE
                puts("Failed to create ready queue!\n");
            #else
                printf("Failed to create ready queue!\n");
            #endif
            return;
        }
    }
    for (int i = 0; i < MAX_THREADS; i++) {
        micros_threads[i] = (tcb*)malloc(sizeof(tcb));
        if (micros_threads[i]) {
            memset(micros_threads[i], 0, sizeof(tcb));
            micros_threads[i]->tid = i;
            micros_threads[i]->state = THREAD_IDLE;  // Start as idle
        }
    }
}


/* Cleanup thread pool - call at shutdown */
void cleanup_thread_pool(void) {
    for (int i = 0; i < MAX_THREADS; i++) {
        if (micros_threads[i]) {
            free(micros_threads[i]);
            micros_threads[i] = NULL;
        }
    }
}


int get_tcb(){
    /*Return index for a tcb if the state is idle (0)

    we can assume threads are taken from the pool in order from lowest to highest TID until all threads have been assigned work*/
    for(int i = 0; i < MAX_THREADS; i++){
        if(micros_threads[i] && micros_threads[i]->state == THREAD_IDLE){
            return i;
        }
    }
    return -1;
}


tcb* get_current_thread(int tid){
    return micros_threads[tid];
}

/* We can create pool of N threads statically
We can assume the thread pool exists at boot time, and no further threads need to be created dynamically

The pool itself might simply be an array of pointers to TCBs,
with some flag identifying whether or not a thread in the pool has been assigned work.

initially all threads in the pool are idle

work to specific threads can be assigned via thread_create()*/

/* TCB: Thread Control Block

This function binds a thread in the pool to specific stack and function addresses,
while passing args to target function. Thread executes using specified stack.


if a func returns, that TCB slot can be used later for some other new func.
does subsequent mean, we should immediately use released TCB? I think our get_tcb function already handles this.

if max num of threads is reached, we need to deal with that case as well.*/
int thread_create(void *stack, void *func, void *args){
    /* args: struct_schedparams_t -> specifies the C and T values for the thread */

    #ifdef KERNEL_MODE
        puts("creating thread!\n");
    #else
        printf("creating thread!\n");
    #endif

    // cast args to sched params
    struct_schedparams_t* sched_params = (struct_schedparams_t*) args;
    int C = sched_params->execution_time;
    int T = sched_params->period;

    int new_tcb = -1;
    uint16_t ds = 0x10, es = 0x10, fs = 0x10, gs = 0x10;  // data segment selectors

    new_tcb = get_tcb(); // get a tcb from the thread pool

    /* Checking if there was an issue creating the thread...*/
    if(new_tcb == -1){
        #ifdef KERNEL_MODE
            puts("No TCB available!\n");  // if we run out of TCBs, means no more thread waiting to be created
        #else
            printf("No TCB available!\n");
        #endif
        return -1;
    }

    *(((uint32_t *) stack) - 0) = (uint32_t) exit_thread;
    stack = (void *)(((uint32_t *) stack) - 1);

    /* Found new TCB*/
    micros_threads[new_tcb]->tid = new_tcb;
    micros_threads[new_tcb]->bp = (uint32_t) stack;
    micros_threads[new_tcb]->entry = (uint32_t) func;  // entry point to function
    micros_threads[new_tcb]->flag = 0;
    micros_threads[new_tcb]->next = NULL;
    micros_threads[new_tcb]->left_child = NULL;
    micros_threads[new_tcb]->right_child = NULL;
    micros_threads[new_tcb]->parent = NULL;
    micros_threads[new_tcb]->state = 1; // ready state
    micros_threads[new_tcb]->execution_time = C;
    micros_threads[new_tcb]->remaining_time = C;
    micros_threads[new_tcb]->period = T;
    micros_threads[new_tcb]->priority = (T > 0) ? T : 0xFFFFFFFF; // lower T = higher priority
    micros_threads[new_tcb]->next_arrival = 0;
    micros_threads[new_tcb]->max_jobs = sched_params->max_jobs;
    micros_threads[new_tcb]->jobs_done = 0;
    micros_threads[new_tcb]->is_periodic = (T > 0) ? TRUE : FALSE;

    /* Fake an initial context for the new thread*/
    micros_threads[new_tcb]->sp = (uint32_t) (((uint16_t *) stack) - 22);

    /* Now fix the stack pointer to fake a context switch*/
    /*EIP*/ *(((uint32_t *) stack) - 0) = micros_threads[new_tcb]->entry;
    /*FLG*/ *(((uint32_t *) stack) - 1) = 0 | (1 << 9); // Set IF for preemption
    /*EAX*/ *(((uint32_t *) stack) - 2) = 0;
    /*ECX*/ *(((uint32_t *) stack) - 3) = 0;
    /*EDX*/ *(((uint32_t *) stack) - 4) = 0;

    /*EBX*/ *(((uint32_t *) stack) - 5) = 0;
    /*ESP*/ *(((uint32_t *) stack) - 6) = (uint32_t)(((uint32_t *) stack) - 2);
    /*EBP*/ *(((uint32_t *) stack) - 7) = (uint32_t)(((uint32_t *) stack) - 2);
    /*ESI*/ *(((uint32_t *) stack) - 8) = 0;
    /*EDI*/ *(((uint32_t *) stack) - 9) = 0;

    /*DS*/ *(((uint16_t *) stack) - 19) = ds; /* 16-bit segment selectors*/
    /*ES*/ *(((uint16_t *) stack) - 20) = es;
    /*FS*/ *(((uint16_t *) stack) - 21) = fs;
    /*GS*/ *(((uint16_t *) stack) - 22) = gs;

    // TODO : store created thread in ready queue. Priority should be determined based on T value (rate-monotonic scheduling)
    if (ready_queue) {
        if (heap_insert(ready_queue, micros_threads[new_tcb]) != 0) {
            #ifdef KERNEL_MODE
                puts("Failed to insert thread into ready queue!\n");
            #else
                printf("Failed to insert thread into ready queue!\n");
            #endif
        }
    }
    curr_tid = new_tcb;
    return new_tcb + 1;  // return thread id (1-based)
}
