#include "multiboot.h"
#include "helpers.h"
#include "defs.h"
#include "thread.h"
#include "heap.h"
// #include "scheduler.h"
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

    // create pool of threads
    init_thread_pool();

    for(int i = 0; i < MAX_THREADS; i++){
        done[i] = FALSE;

        f[i] = (uint32_t *)thread_func;
        // TODO read from predefined args
        struct_schedparams_t schedparams = { .execution_time = 2 + i, .period = 5 * (i + 1), .max_jobs = 3 };

        thread_create(thread_stacks[i], f[i], &schedparams);
    }

    // start scheduling using rate-monotonic scheduling
    // schedule_rm();
}