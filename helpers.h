#ifndef HELPER_DEFN_FUNCS_H
#define HELPER_DEFN_FUNCS_H



/* Base address of the VGA frame buffer */
static unsigned short* video_memory = (unsigned short*)0xb8000;
static int attrib = 0x0F; // black background, white foreground
static int csr_x = 0, csr_y = 7;
#define COLS 80

// integer to ascii, from osdev
static inline char * itoa( int value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
    // Check for supported base.
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if ( value < 0 && base == 10 )
    {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}


static inline void busy_wait(){
    for(volatile int i = 0; i < 1000000; i++);
}

static inline void putc(unsigned char c){
    if(c==0x99){ // tab
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

static inline int my_strlen(char* text){ // find length of string
    int i=0;
    while(text[i]) i++;
    return i;
}

static inline void puts(char* text) { // print string
    int i;
    for (i=0; i<my_strlen(text); i++) {
        putc(text[i]);
    }
}

static inline void putint(int value) { // print integer
    char buffer[16];
    itoa(value, buffer, 10);
    puts(buffer);
}

#endif /* HELPER_DEFN_FUNCS_H */