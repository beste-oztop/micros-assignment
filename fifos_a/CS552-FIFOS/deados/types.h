#ifndef TYPES_H
#define TYPES_H

#define FALSE 0
#define TRUE 1

#define NULL -1

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
typedef unsigned long long int uint64;

typedef signed char sint8, s8;
typedef signed short int sint16, s16;
typedef signed long int sint32, s32;
typedef signed long long int sint64, s64;

#ifndef _SIZE_T
typedef int size_t;
#define _SIZE_T 1
#endif

typedef signed char bool;

typedef unsigned long uint;
typedef signed long sint;

#ifndef _STDINT_
#define _STDINT_
typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t;
typedef uint64 uint64_t;
#endif

/* segment descriptor breakdown in bits 

    Limit: (0-15) + (48-51)
    Base: (16-31) + (32-39) + (56-63)
    Access Byte: (40-47)
    Flags: (52-55)

*/

struct segment_descriptor {
    uint32_t base; /* 32 bits, broken up into sections of 16/8/8*/
    uint32_t limit; /* limit is only 20 bits, broken up into a 16 bit section and a 4 bit section*/
    uint8_t access_byte;
    uint8_t flags; /* Flags is only 4 bits */
}__attribute__((packed));
typedef struct segment_descriptor segment_descriptor_t;

struct gdt{
    uint16_t limit;
    uint32_t base;
}__attribute__((packed));
typedef struct gdt gdt_t;

/* I think this is like a runqueue and a mock TCB*/
typedef struct runqueue {
    int (*task)();
    int tid;
    struct runqueue *next;
    struct runqueue *prev;
} rq;

// typedef struct task_control_block{
//     int (*task)();
//     int tid;
//     int state;
//     int eip;
//     int esp;
//     struct tcb *next;
//     struct tcb *prev;
// } tcb;

typedef struct task_control_block{
    int tid;
    uint32_t bp; /*Stack pointer? or Point to top of stack?? -> based of Rich's example code*/
    uint32_t entry; /* starting entry point -> EIP */
    /* 0 - Idle, 1 - Busy*/
    int state; /*Same as Rich's flag ( or should be )*/
    int budget;
    int period;
    int prev_deadline;
    int deadline;
    int run_time;
    
    struct task_control_block *next; 
    uint32_t sp; /*Stack pointer? Bottom of stack??*/

    /* Does each thread need to keep track of its next / prev ??? */
    // struct tcb *next;
    struct task_control_block *prev;
} tcb;

typedef struct {
	short    isr_low;      // The lower 16 bits of the ISR's address
	short    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	char     reserved;     // Set to zero
	char     attributes;   // Type and attributes; see the IDT page
	short    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

typedef struct {
	short	limit;
	int	base;
} __attribute__((packed)) idtr_t;


#endif