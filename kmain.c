#include "multiboot.h"
#include "helpers.h"
#include "defs.h"
#include "thread.h"
#include "heap.h"
#include "scheduler.h"
// #define KERNEL_MODE  // to enable puts function from helpers.h



/* Args for thread creation, where MAX_THREADS is 3*/
/* <TIDi, Ci, Ti,>
<1, 2, 5>
<2, 2, 10>
<3, 3, 20>
*/

/* kmain.c: Kernel main function called by the bootloader (GRUB) */
void kmain(multiboot_info_t* mbd, unsigned long magic_num){

    puts("Hello, World!\n");

    /* Make sure the magic number matches for memory mapping*/
    if(magic_num != MULTIBOOT_BOOTLOADER_MAGIC) {
        puts("invalid magic number!");
    }


    /* Check bit 6 to see if we have a valid memory map */
    if(!(mbd->flags >> 6 & 0x1)) {
        puts("invalid memory map given by GRUB bootloader");
    }

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

        thread_create(thread_stacks[i], f[i], &schedparams);
    }

    puts("Ready queue looks like this:\n");
    heap_print(ready_queue);

    // FIXME: start scheduling using rate-monotonic scheduling
    schedule_rm();
    // FIXME: we need to implement the dispatch function to switch to the scheduled thread
    // dispatch_first_run();
}