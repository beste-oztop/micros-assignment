#include "types.h"
#include "defns.h"

#ifndef _HELPERS_H
#define _HELPERS_H

void increase_time(){
    current_time++;
    //puts("++\n");
}

/*Calculate length of string*/
int my_strlen(const char *text){
    int len = 0;
    int idx = 0;
    while(text[idx]){
        len++;
        idx++;
    }
    return len;
}

void putc (unsigned char c) {
    if (c == 0x09) { /* Tab (move to next multiple of 8) */
        csr_x = (csr_x + 8) & ~(8 - 1);
    } else if (c == '\r') { /* Carriage Return */
        csr_x = 0;
    } else if (c == '\n') { /* Line Feed (unix-like) */
        csr_x = 0; csr_y++;
    } else if(c >= ' ') { /* Printable characters */
        /* Put the character w/attributes */
        *(videoram + (csr_y * COLS + csr_x)) = c | (attrib << 8);
        csr_x++;
    }
    if(csr_x >= COLS){ csr_x = 0; csr_y++;} /* wrap around */
}


void puts (char *text) {
    int len = my_strlen(text);
    for(int i = 0; i < len; i++){
        putc(text[i]);
    }
}


/* Based on itoa function from OSDev -> https://wiki.osdev.org/Printing_To_Screen */
char * itoa(unsigned long value, char * str, int base){
    char * rc; /*Hold address of string to return*/
    char * ptr; /*Pointer to string*/
    char * low; /*Used for reversing string at end of algorithm */

    /*Checking for valid base*/
    if(base < 2 || base > 36){
        *str = '\0';
        return str;
    }
    /*Set up pointers, rc and ptr both point to the passed in buff (destination)*/
    rc = ptr = str;
    
    if(value < 0 && base == 10){
        *ptr++ = '-';
    }
    /*Set low to current position of pointer*/
    low = ptr;
    do 
    {   
        /*
            value % base: compute remainder of value/base to get index of character 
            Use offset 35 + index (handles base 2 through 36)
            *ptr++ -> place character and increment ptr 
        */
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while(value);

    /*Set null terminator -> then decrement to point at character right before*/
    *ptr-- = '\0';
    /*Reverse the string*/
    while (low < ptr){
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc; 
}

int bytes_to_mb(int mem_b){
    return (mem_b >> 20) + 1;
}


void busy_wait(){
    for(int i = 0; i < 80000000; i++){
        ;
    }
}

int get_tcb(){
    /*Return index for a tcb if the state is idle (0)*/
    for(int i = 0; i < MAX_THREADS; i++){
        if(fifos_threads[i]->state == 0){
            return i;
        }
    }
    return -1;
}

static inline void outb(short port, char val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    /* There's an outb %al, $imm8 encoding, for compile-time constant port numbers that fit in 8b. (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static inline char inb(short port)
{
    char ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

void pic_disable(void) {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}


#endif