#ifndef HELPER_DEFN_FUNCS_H
#define HELPER_DEFN_FUNCS_H

#define COLS 80

/* Declare external variables - defined in helpers.c */
extern unsigned short* video_memory;
extern int attrib;
extern volatile int csr_x, csr_y;

/* Function declarations */
char * itoa(int value, char * str, int base);
void busy_wait(void);
void putc(unsigned char c);
int my_strlen(char* text);
void puts(char* text);
void putint(int value);

#endif /* HELPER_DEFN_FUNCS_H */