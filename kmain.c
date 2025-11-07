#include "multiboot.h"
#include "helpers.h"


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

    
}