#include "dispatch.h"

void dispatcher(void){
    if(!current_thread){
    puts("No current thread to dispatch!\n");
    return;
    }


    puts("Dispatching to thread ID: ");
    putint(current_thread->tid);
    puts("\n");


    uint32_t new_sp = current_thread->sp;

    // Inline assembly to perform context switch as specified
    __asm__ __volatile__ (
        "pushfl\n\t"               // Save EFLAGS
        "pushal\n\t"               // Save general-purpose registers
        "pushw %%ds\n\t"           // Save segment registers
        "pushw %%es\n\t"
        "pushw %%fs\n\t"
        "pushw %%gs\n\t"
        "movl %%esp, %%eax\n\t"    // Save current ESP into EAX
        "pushl %%eax\n\t"          // Push it as argument
        "call save_prev_stack\n\t"
        "call save_curr_tid\n\t"
        "call schedule_rm\n\t"
        "call save_next_tid\n\t"
        "call save_next_stack\n\t"
        "movl %0, %%edi\n\t"       // Load new_sp into EDI
        "movl %%edi, %%esp\n\t"    // Set new ESP
        "popw %%gs\n\t"            // Restore segment registers
        "popw %%fs\n\t"
        "popw %%es\n\t"
        "popw %%ds\n\t"
        "popal\n\t"                // Restore general-purpose registers
        "popfl\n\t"                // Restore EFLAGS
        "retl\n\t"                 // Return to the thread's EIP
        :
        : "r" (new_sp)
        : "memory", "eax", "edi"
    );

    puts("Returned to dispatcher - this should never happen!\n");
}


void dispatch_first_run(void){

    if(!current_thread){
        puts("No current thread to dispatch!\n");
        return;
    }


    puts("Dispatching (first run) to thread ID: ");
    putint(current_thread->tid);
    puts("\n");


    uint32_t new_sp = current_thread->sp;

    __asm__ __volatile__ (
        "movl %0, %%edi\n\t"     /* place new stack pointer into %edi */
        // "call save_next_stack\n\t"
        "movl %%edi, %%esp\n\t"  /* switch to the new thread's stack */
        "popw %%gs\n\t"
        "popw %%fs\n\t"
        "popw %%es\n\t"
        "popw %%ds\n\t"
        "popal\n\t"
        "popfl\n\t"
        "sti\n\t"
        "retl\n\t"
        :
        : "r" (new_sp)  // input: new_sp in any general register
        : "memory", "edi"
    );
}


/* Voluntarily yield the CPU from the current thread */
void yield(void){
    dispatcher();
}


