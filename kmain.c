#include "multiboot.h"
#include "helpers.h"
#include "defs.h"
#include "thread.h"
#include "heap.h"
#include "scheduler.h"
#include "dispatch.h"
// #define KERNEL_MODE  // to enable puts function from helpers.h

extern void set_gdt(uint16_t limit, uint32_t base); // this is defined in boot.s


/* Args for thread creation, where MAX_THREADS is 3*/
/* <TIDi, Ci, Ti,>
<1, 2, 5>
<2, 2, 10>
<3, 3, 20>
*/

void encodeGdtEntry(uint8_t *target, gdt_t source)
{   // from osdev
    // Check the limit to make sure that it can be encoded
    if (source.limit > 0xFFFFF) {puts("GDT cannot encode limits larger than 0xFFFFF");}

    // Encode the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] = (source.limit >> 16) & 0x0F;

    // Encode the base
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;

    // Encode the access byte
    target[5] = source.access_byte;

    // Encode the flags
    target[6] |= (source.flags << 4);
}


void save_prev_stack(int prev_stack){
    current_thread->sp = prev_stack;
}

void save_next_stack(){
    /* Save the current stack pointer  %0 is placeholder, ::{argument}, r means use any general purpose register to hold this value */
    __asm__ volatile ("mov %0, %%edi"::"r"(current_thread->sp));
    puts("Switching to stack pointer: ");
    putint(current_thread->sp);
    puts("\n");

}

void save_curr_tid(){
    curr_tid = current_thread->tid;
}

int next_tid;
void save_next_tid(){
    next_tid = current_thread->tid;
    #ifdef KERNEL_MODE
        puts("Switching to thread ID: ");
        putint(next_tid);
        puts("\n");
    #endif
}


/* kmain.c: Kernel main function called by the bootloader (GRUB) */
void kmain(multiboot_info_t* mbd, unsigned long magic_num){

    // puts("Hello, World!\n");
    /* Make sure the magic number matches for memory mapping*/
    if(magic_num != MULTIBOOT_BOOTLOADER_MAGIC) {
        puts("invalid magic number!");
    }

    // set up GDT
    uint8_t gdt_entries[3][8] __attribute__((aligned(8)));
    encodeGdtEntry(gdt_entries[0], null_desc);
    encodeGdtEntry(gdt_entries[1], kernel_code);
    encodeGdtEntry(gdt_entries[2], kernel_data);

    set_gdt(sizeof(gdt_entries) - 1, (uint32_t)&gdt_entries);
    // puts("GDT set up complete.\n");

    __asm__ volatile ("ljmp $0x08, $.reload_CS \n"  // long jump to reload CS -- 0x08 is the kernelcode segment
                      ".reload_CS: \n"
                      "mov $0x10, %%ax \n"
                      "mov %%ax, %%ds \n"
                      "mov %%ax, %%es \n"
                      "mov %%ax, %%fs \n"
                      "mov %%ax, %%gs \n"
                      "mov %%ax, %%ss \n"
                      :  /* no output operands */
                      :  /* no input operands */
                      : "ax");  /* clobbered registers, compiler should not assume on ax value*/

    // puts("Segment registers reloaded.\n");
    // thread management ---------------------------------------------------------

    // Initiailize the thread pool
    init_thread_pool();

    /* Predefined Ci, Ti and max_jobs for [MAX_THREADS] threads */
    static const uint32_t exec_time_predef[MAX_THREADS] = { 2, 2, 3 };   /* .execution_time*/
    static const uint32_t period_predef[MAX_THREADS]    = { 10, 5, 20 };  /* .period */
    static const uint32_t max_jobs_predef[MAX_THREADS]  = { 3, 3, 3 };   /* .max_jobs */


    for(int i = 0; i < MAX_THREADS; i++){
        done[i] = FALSE;

        f[i] = (uint32_t *)thread_func;

        // Read from predefined args
        struct_schedparams_t schedparams = { .execution_time = exec_time_predef[i], .period = period_predef[i], .max_jobs = max_jobs_predef[i] };
        void *stack_top = (void*)(&thread_stacks[i][STACK_SIZE_PER_THREAD-1]);
        thread_create(stack_top, f[i], &schedparams);
    }

    // DEBUG
    // puts("Ready queue looks like this:\n");
    // heap_print(ready_queue);

    // Set the first thread to run
    current_thread = micros_threads[0];
    // puts("Starting first thread with address: ");
    // putint(current_thread);
    // puts("\n");
    curr_tid = 0;
    // dispatch_first_run();
    // TODO create the runque with rate-monotonic scheduling. After that each thread should be scheduled according to RM
    // We don't need to call RM everytime here, since it's not preemptive scheduling. Just need to create the runqueue once.
    // FIXME: start scheduling using rate-monotonic scheduling
    dispatch_first_run();
    //halt the CPU if we ever return here
    while(1){
        __asm__ volatile ("hlt");
    }
}