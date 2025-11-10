#include "dispatch.h"

extern int curr_tid;
extern tcb *current_thread;
extern tcb *micros_threads[MAX_THREADS];

void dispatch(void){
    if(!current_thread){
        #ifdef KERNEL_MODE
            puts("No current thread to dispatch!\n");
        #else
            printf("No current thread to dispatch!\n");
        #endif
        return;
    }

    #ifdef KERNEL_MODE
        puts("Dispatching to thread ID: ");
        putint(current_thread->tid);
        puts("\n");
    #endif

    uint32_t new_sp = current_thread->sp;    


    // Inline assembly to perform context switch
    __asm__ __volatile__ (
        "movl %%esp, %0\n\t"   // Save old ESP
        "popl %%gs\n\t"       // Restore segment registers
        "popl %%fs\n\t"
        "popl %%es\n\t"
        "popl %%ds\n\t"
        "popal\n\t"       // Restore general-purpose registers
        "popfl\n\t"      // Restore EFLAGS
        "retl\n\t"        // Return to the thread's EIP
        :
        : "r" (new_sp)
        : "memory"
    );

}


void dispatch_first_run(void){

    if(!current_thread){
        #ifdef KERNEL_MODE
            puts("No current thread to dispatch!\n");
        #else
            printf("No current thread to dispatch!\n");
        #endif
        return;
    }

    #ifdef KERNEL_MODE
        puts("Dispatching (first run) to thread ID: ");
        putint(current_thread->tid);
        puts("\n");
    #endif

    dispatch();
}


/* Voluntarily yield the CPU from the current thread */
void yield(void){
    save_context_and_schedule();
}

void save_context_and_schedule(void){
    tcb *old_thread = current_thread;

    if(!old_thread){
        #ifdef KERNEL_MODE
            puts("No current thread to save context from!\n");
        #else
            printf("No current thread to save context from!\n");
        #endif
        return;

        // Just schedule
        schedule_rm();
        if(current_thread){
            dispatch_first_run();
        }
    }

    // There is an existing thread, need to context switch

    __asm__ __volatile__ (
        "pushfl\n\t"      // Save EFLAGS
        "pushal\n\t"      // Save general-purpose registers
        "pushw %%ds\n\t"   // Save segment registers
        "pushw %%es\n\t"
        "pushw %%fs\n\t"
        "pushw %%gs\n\t"
        "movl %%esp, %0\n\t"   // Save ESP to old_thread->sp TCB
        : "=m" (old_thread->sp)
        :
        : "memory"
    );


    // Now schedule
    schedule_rm();

    if (current_thread == old_thread) {
        // Same thread, just restore context
        __asm__ __volatile__ (
            "movl %0, %%esp\n\t"   // Restore ESP from old_thread->sp
            "popw %%gs\n\t"       // Restore segment registers
            "popw %%fs\n\t"
            "popw %%es\n\t"
            "popw %%ds\n\t"
            "popal\n\t"       // Restore general-purpose registers
            "popfl\n\t"      // Restore EFLAGS
            :
            : "m" (old_thread->sp)
            : "memory"
        );
    } else {
        // Different thread, dispatch to new thread
        dispatch();
    }
        
}



void run_scheduler(void){

    #ifdef KERNEL_MODE
        puts("Running scheduler...\n");
    #endif

    schedule_rm();

    if(current_thread){
        dispatch_first_run();
    } else {
        #ifdef KERNEL_MODE
            puts("No thread scheduled to run!\n");
        #else
            printf("No thread scheduled to run!\n");
        #endif
    }

}