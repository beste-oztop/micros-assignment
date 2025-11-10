#include "helpers.h"

/* VGA text mode state - ONLY one copy in the entire kernel */
unsigned short* video_memory = (unsigned short*)0xb8000;
int attrib = 0x0F;
volatile int csr_x = 0;
volatile int csr_y = 7;

char * itoa(int value, char * str, int base)
{
    char * rc;
    char * ptr;
    char * low;
    if (base < 2 || base > 36)
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    if (value < 0 && base == 10)
    {
        *ptr++ = '-';
    }
    low = ptr;
    do
    {
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while (value);
    *ptr-- = '\0';
    while (low < ptr)
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

void busy_wait(){
    for(volatile long i = 0; i < 100000000; i++){
        __asm__ __volatile__ ("nop");
    }
}

void putc(unsigned char c){
    if(c==0x09){ // tab
        csr_x = (csr_x+8) & ~(8-1);
    }else if(c == '\r'){ // carriage return
        csr_x = 0;
    }else if(c == '\n'){ // new line
        csr_x = 0;
        csr_y++;
    }else if(c >= ' '){ // printable character
        *(video_memory + (csr_y*COLS + csr_x)) = (attrib << 8) | c;
        csr_x++;
    }
    if(csr_x >= COLS){ // end of line
        csr_x = 0;
        csr_y++;
    }
}

int my_strlen(char* text){
    int i=0;
    while(text[i]) i++;
    return i;
}

void puts(char* text) {
    int i;
    for (i=0; i<my_strlen(text); i++) {
        putc(text[i]);
    }
}

void putint(int value) {
    char buffer[16];
    itoa(value, buffer, 10);
    puts(buffer);
}


