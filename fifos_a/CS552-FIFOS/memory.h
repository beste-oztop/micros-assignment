#include "defns.h"
#include "types.h"
#include "helpers.h"
#include "multiboot.h"

#ifndef _MEMORY_H
#define _MEMORY_H


/*Make sure we always display 32-bit (8 digit) hexadecimal strings*/
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

/* Memory types
    1 - Usable RAM
    2 - Reserved - Unusable
    3 - ACPI reclaimable Mem
    4 - ACPI NVS MEMORY
    5 - MEMORY 
*/
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



void printMemoryMap(multiboot_info_t* binfo){
    if(!(binfo->flags >> 6 & 0x1)){
        puts("Invalid Memory Map!");
    }

    /* Process to find Total memory first -> matches desired output in assignment specifications*/
    int i; 
    for(i=0; i < binfo->mmap_length; i+=sizeof(memory_map_t)){
        memory_map_t* mmt = (memory_map_t*)(binfo->mmap_addr + i);
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

    for(i = 0; i < binfo->mmap_length; i+= sizeof(memory_map_t)){
        char buff[33];
        memory_map_t* mmt = (memory_map_t*)(binfo->mmap_addr + i);

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

    /* Expected outputs from memory map -> based off GRUB 'displaymem' command.

    Address Range: [0x00000000:0x0009fbff] Status: Usable
    Address Range: [0x0009fc00:0x0009ffff] Status: Reserved
    Address Range: [0x000f0000:0x000fffff] Status: Reserved
    Address Range: [0x00100000:0x07ffdfff] Status: Usable
    Address Range: [0x07ffe000:0x07ffffff] Status: Reserved
    Address Range: [0xfffc0000:0xffffffff] Status: Reserved

    */
}

#endif