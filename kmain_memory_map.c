#include "multiboot.h"
#include "helpers.h"
#include "defs.h"
#include "thread.h"
#include "heap.h"
#include "scheduler.h"
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


char* process_mem_type(unsigned long type){
    if(type == 1){
        return "Usable RAM";
    }
    else if(type == 2){
        return "Reserved - unusable";
    }
    else if(type == 3){
        return "ACPI reclaimable memory";
    }
    else if(type == 4){
        return "ACPI NVS Memory";
    }
    else if(type == 5){
        return "Bad Memory";
    }
    else {
        return "Unknown Memory Type";
    }
}

void put_memory(char *text){
    int num_pad = 0;
    if(my_strlen(text) < 8){
        num_pad = 8 - my_strlen(text);
    }
    for(int i = 0; i < num_pad; i++){
        putc('0');
    }
    puts(text);
}


int bytes_to_mb(int mem_b){
    return (mem_b >> 20) + 1;
}


/* kmain.c: Kernel main function called by the bootloader (GRUB) */
void kmain(multiboot_info_t* mbd, unsigned long magic_num){

    puts("Hello, World!\n");
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
    puts("GDT set up complete.\n");

    __asm__ volatile ("ljmp $0x08, $.reload_CS \n"  // long jump to reload CS -- 0x08 is the kernelcode segment
                      ".reload_CS: \n"
                      "mov $0x10, %%ax \n"
                      "mov %%ax, %%ds \n"
                      "mov %%ax, %%es \n"
                      "mov %%ax, %%fs \n"
                      "mov %%ax, %%gs \n"
                      "mov %%ax, %%ss \n"
                      :
                      :
                      : "ax");

    puts("Segment registers reloaded.\n");
    // print memory map ----------------------------------------------------------

    puts("Memory Map:\n");
    int total_free_mem = 0;
    /* Process to find Total memory first -> matches desired output in assignment specifications*/
    int i;
    for(i=0; i < mbd->mmap_length; i+=sizeof(memory_map_t)){
        memory_map_t* mmt = (memory_map_t*)(mbd->mmap_addr + i);
        /*Only add up type 1 memory*/
        if(mmt->type == 1){
            total_free_mem += mmt->length_low;
        }
    }
    total_free_mem = bytes_to_mb(total_free_mem);

    /*So we skip whatever is already printed on the screen for some reason.*/
    puts("\n\n\n\n\n\n\n");
    /* Rich said to only print free memory (type 1) (Piazza post @149)*/
    puts("MemOS: Welcome *** Total Free Memory is: ");
    char buff[33];
    itoa(total_free_mem, buff, 10);
    puts(buff);
    puts("MB\n");

    for(i = 0; i < mbd->mmap_length; i+= sizeof(memory_map_t)){
        char buff[33];
        memory_map_t* mmt = (memory_map_t*)(mbd->mmap_addr + i);

        /*Display: Address Range: [0xXXXXX:0xYYYYYY] Status: zzzzz*/
        puts("Address range [0x");
        itoa(mmt->base_addr_low, buff, 16);
        put_memory(buff);
        puts(":0x");
        unsigned long end_addr = mmt->base_addr_low + mmt->length_low - 1;
        itoa(end_addr, buff, 16);
        put_memory(buff);
        puts("] Status: ");
        puts(process_mem_type(mmt->type));
        putc('\n');
    }

    /* Check bit 6 to see if we have a valid memory map */
    if(!(mbd->flags >> 6 & 0x1)) {
        puts("invalid memory map given by GRUB bootloader");
    }

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

        thread_create(thread_stacks[i], f[i], &schedparams);
    }

    puts("Ready queue looks like this:\n");
    heap_print(ready_queue);

    // FIXME: start scheduling using rate-monotonic scheduling
    schedule_rm();
    // FIXME: we need to implement the dispatch function to switch to the scheduled thread
    // dispatch_first_run();
}