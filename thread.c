/* thread.c: thread management functions */
#include "thread.h"


/* Global thread bookkeeping */
int thread_ids = 1;             /* next available TID (1-based internal) */
int curr_tid = -1;              /* current running thread TID (-1 = none) */
tcb *current_thread = NULL;     /* pointer to currently running TCB */
kbool_t done[MAX_THREADS] = {0}; // flags to indicate if thread is done
int counter[MAX_THREADS] = {0};  // keeps track of how many times each thread has run
uint32_t *f[MAX_THREADS] = {0};  // function pointers for each thread
uint32_t *stacks[MAX_THREADS] = {0};  // stack pointers for each thread
tcb *micros_threads[MAX_THREADS] = {0};  /*Array of pointers to TCBs*/
thread_heap_t* ready_queue = NULL; // global ready queue heap


/* we don't have access to malloc in kernel mode, so we can statically allocate the thread pool */
/* Statically allocate stacks (4KB each, aligned) */
uint8_t thread_stacks[MAX_THREADS][STACK_SIZE_PER_THREAD] __attribute__((aligned(16)));

// uint32_t thread_stacks[MAX_THREADS] = {0}; /* stack pointers for each thread */

/* Initialize the thread pool - must be called before using threads */
static tcb tcb_pool[MAX_THREADS];

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
    puts("exiting thread ID: ");
    putint(curr_tid);
    puts("\n");
    tcb* curr_thread = get_current_thread(curr_tid);
    // here we need to check if job of thread finished or not.
    // If finished, set state to 0, otherwise update remaining execution time
    if(curr_thread->is_periodic){
        curr_thread->jobs_done++;
        if(curr_thread->jobs_done >= curr_thread->max_jobs){
            curr_thread->state = THREAD_EXITED;  // Thread is done - don't reinsert
        }else{
            curr_thread->remaining_time = curr_thread->execution_time;
            curr_thread->next_arrival += curr_thread->period;
            curr_thread->state = THREAD_READY;
            // Reinsert into ready queue
            if (ready_queue) {
                if (heap_insert(ready_queue, curr_thread) != 0) {
                        puts("Failed to reinsert thread into ready queue!\n");
                }
            }
        }
    }

    // Schedule the next thread
    schedule_rm();  // This picks the next thread and sets current_thread

    if(current_thread){
        // Switch to the next thread
        // dispatch_first_run();  // or dispatcher() depending on your design
        dispatcher();
    } else {
        // No more threads - halt
        puts("All threads finished!\n");
        while(1) { __asm__ volatile ("hlt"); }
    }
    while(1) { __asm__ volatile ("hlt"); }
}


/* Initialize the thread pool - must be called before using threads */
void init_thread_pool(void) {
    if (!ready_queue) {
        ready_queue = heap_create(MAX_THREADS);
        if (!ready_queue) {
            puts("Failed to create ready queue!\n");
            return;
        }
    }
    for (int i = 0; i < MAX_THREADS; i++) {
        micros_threads[i] = &tcb_pool[i];
        micros_threads[i]->tid = i;
        micros_threads[i]->state = THREAD_IDLE;  // Start as idle
    }
}


/* Cleanup thread pool - call at shutdown */
void cleanup_thread_pool(void) {
    for (int i = 0; i < MAX_THREADS; i++) {
        micros_threads[i] = NULL;
    }
}


int get_tcb(){
    /*Return index for a tcb if the state is idle (0)

    we can assume threads are taken from the pool in order from lowest to highest TID until all threads have been assigned work*/
    for(int i = 0; i < MAX_THREADS; i++){
        // puts("Checking TCB ID: ");
        // putint(i);
        // puts("\n");
        // puts("State: ");
        // putint(micros_threads[i]->state);
        // puts("\n");
        if(micros_threads[i]->state == THREAD_IDLE){
            // puts("Allocating TCB ID: ");
            // putint(i);
            // puts("\n");
            return i;
        }
    }
    return -1;
}

// Get pointer to TCB by TID --> this is 0-based internally
tcb* get_current_thread(int tid){
    return micros_threads[tid];
}


/* TODO remove once done
we may not have access to malloc in kernel mode, so we can statically allocate the thread pool
stack creation:

for(int i = 0; i < MAX_THREADS; i++){
    done[i] = FALSE;
    fifos_threads[i] = (tcb *)0x500000 + (i * 0x1000);
    stacks[i] = (uint32_t *)0x400000 + (i * 0x1001);
    f[i] = (uint32_t *)thread;
    thread_create(stacks[i], f[i]);
}
    put the address of thread 1 in a function pointer, and this time pass a parameter, 1, to thread 1


*/




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
int thread_create_org(void *stack, void *func, void *args){
    /* args: struct_schedparams_t -> specifies the C and T values for the thread */
    // cast args to sched params
    struct_schedparams_t* sched_params = (struct_schedparams_t*) args;
    int C = sched_params->execution_time;
    int T = sched_params->period;

    // puts("Creating thread with execution_time=");
    // putint(sched_params->execution_time);
    // puts(", period=");
    // putint(sched_params->period);
    // puts(", max_jobs=");
    // putint(sched_params->max_jobs);
    // puts("\n");


    int new_tcb = -1;
    uint16_t ds = 0x10, es = 0x10, fs = 0x10, gs = 0x10;  // data segment selectors

    new_tcb = get_tcb(); // get a tcb from the thread pool

    /* Checking if there was an issue creating the thread...*/
    if(new_tcb == -1){
        puts("No TCB available!\n");  // if we run out of TCBs, means no more thread waiting to be created
        return -1;
    }

    *(((uint32_t *) stack) - 0) = (uint32_t) exit_thread;
    stack = (void *)(((uint32_t *) stack) - 1);
    //FIXME this function needs to be fixed to properly setup the stack for context switching
    /* Found new TCB*/
    micros_threads[new_tcb]->tid = new_tcb;  // TID is 0-based
    micros_threads[new_tcb]->bp = (uint32_t) stack;
    micros_threads[new_tcb]->entry = (uint32_t) func;  // entry point to function
    micros_threads[new_tcb]->flag = 0;
    micros_threads[new_tcb]->next = NULL;
    micros_threads[new_tcb]->state = THREAD_READY; // ready state
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
    /*FLG*/ *(((uint32_t *) stack) - 1) = 0 | (1 << 9);
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
            puts("Failed to insert thread into ready queue!\n");
        }
    }
    curr_tid = new_tcb;
    return new_tcb;  // return thread id (0-based)
}

int thread_create(void *stack, void *func, void *args){
    struct_schedparams_t* sched_params = (struct_schedparams_t*) args;
    int C = sched_params->execution_time;
    int T = sched_params->period;

    int new_tcb = -1;
    uint16_t ds = 0x10, es = 0x10, fs = 0x10, gs = 0x10;

    new_tcb = get_tcb();
    if(new_tcb == -1){
        puts("No TCB available!\n");
        return -1;
    }

    /* Found new TCB*/
    micros_threads[new_tcb]->tid = new_tcb;
    micros_threads[new_tcb]->bp = (uint32_t) stack;
    micros_threads[new_tcb]->entry = (uint32_t) func;
    micros_threads[new_tcb]->flag = 0;
    micros_threads[new_tcb]->next = NULL;
    micros_threads[new_tcb]->state = THREAD_READY;
    micros_threads[new_tcb]->execution_time = C;
    micros_threads[new_tcb]->remaining_time = C;
    micros_threads[new_tcb]->period = T;
    micros_threads[new_tcb]->priority = (T > 0) ? T : 0xFFFFFFFF;
    micros_threads[new_tcb]->next_arrival = 0;
    micros_threads[new_tcb]->max_jobs = sched_params->max_jobs;
    micros_threads[new_tcb]->jobs_done = 0;
    micros_threads[new_tcb]->is_periodic = (T > 0) ? TRUE : FALSE;

    /* Set up the stack frame - careful with pointer arithmetic! */
    uint32_t *sp = (uint32_t *)stack;

    // Stack layout (addresses decrease as we go down):
    // sp[0]   = exit_thread (return address)
    // sp[-1]  = EIP
    // sp[-2]  = EFLAGS
    // sp[-3]  = EAX
    // sp[-4]  = ECX
    // sp[-5]  = EDX
    // sp[-6]  = EBX
    // sp[-7]  = ESP (saved - points to exit_thread)
    // sp[-8]  = EBP
    // sp[-9]  = ESI
    // sp[-10] = EDI
    // After this, we need 4 words (16-bit each) for DS/ES/FS/GS
    // sp[-11] will contain DS and ES (2 bytes each, packed into 32-bit word)
    // sp[-12] will contain FS and GS (2 bytes each, packed into 32-bit word)

    sp[0]  = (uint32_t) exit_thread;    // Return address
    sp[-1] = (uint32_t) func;            // EIP - thread entry point
    sp[-2] = 0x200;                      // EFLAGS - IF=1 (interrupts enabled)
    sp[-3] = 0;                          // EAX
    sp[-4] = 0;                          // ECX
    sp[-5] = 0;                          // EDX
    sp[-6] = 0;                          // EBX
    sp[-7] = (uint32_t)(&sp[0]);        // ESP - points to return address
    sp[-8] = (uint32_t)(&sp[0]);        // EBP
    sp[-9] = 0;                          // ESI
    sp[-10] = 0;                         // EDI

    // Segment registers - packed as 16-bit values
    // dispatch_first_run does: popw %gs, popw %fs, popw %es, popw %ds
    // So we need them in reverse order in memory (stack grows down)
    uint16_t *sp16 = (uint16_t *)&sp[-11];
    sp16[1] = ds;  // Will be popped last (DS)
    sp16[0] = es;  // (ES)
    sp16 = (uint16_t *)&sp[-12];
    sp16[1] = fs;  // (FS)
    sp16[0] = gs;  // Will be popped first (GS)

    // Stack pointer points to GS (first thing to be popped)
    micros_threads[new_tcb]->sp = (uint32_t)&sp[-12];

    if (ready_queue) {
        if (heap_insert(ready_queue, micros_threads[new_tcb]) != 0) {
            puts("Failed to insert thread into ready queue!\n");
        }
    }

    curr_tid = new_tcb;
    return new_tcb;
}

int thread_func(){
     __asm__ volatile ("cli");  // Clear interrupt flag
    // int id = curr_tid;  //  GIVES ERROR qemu does not work
    int id = thread_ids;  //  id is 0 based  // FIXME this does not seem right
    int i;
    char buff[16];
    itoa(id, buff, 10);

    puts("Thread ");
    putint(id);
    puts(" started!");
    while(1){
        for (i = 0; i < 10; i++){
            // puts(buff);
            busy_wait();
        }
        // putc('\n');
        if(1){ // only if preemptive scheduling is enabled
            // yield();
        }
        if(++counter[id] == 3)  // run 3 times, counter is global array  -> this needs to come from args
            break;
    }

    // thread finished executing
    puts("Done ");
    puts(buff);
    puts(" !\n");
    done[id] = TRUE;

    // Re-enable interrupts at the end
    __asm__ volatile ("sti");  // Set interrupt flag

    return 0;
}


