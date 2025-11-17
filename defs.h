#ifndef DEFNS_H
#define DEFNS_H

#define FALSE 0
#define TRUE 1
#define NULL ((void*)0)  // Define NULL as a void pointer

#define KERNEL_MODE  // to enable puts function from helpers.h


#define MAX_THREADS 4  // maximum number of threads
#define SIM_TIME 100000  // time slice in milliseconds
#define STACK_SIZE_PER_THREAD 4096  // 4KB per thread stack

/* Thread states */
#define THREAD_IDLE    0
#define THREAD_READY   1
#define THREAD_RUNNING 2
#define THREAD_WAITING 3
#define THREAD_EXITED  4



typedef signed char kbool_t;
typedef unsigned short int uint16_t;
typedef unsigned long int uint32_t;
typedef unsigned char priority_t;
typedef unsigned char uint8_t;


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
// typedef uint8 uint8_t;
// typedef uint16 uint16_t;
// typedef uint32 uint32_t;
typedef uint64 uint64_t;
#endif


typedef struct thread_control_block tcb;

struct thread_control_block {
    int tid; // thread ID
    uint32_t bp; // base pointer
    uint32_t sp; // stack pointer
    uint32_t entry; // entry point
    int flag; // general purpose flag
    int state; // state of the thread (e.g., ready, running, exited) 0 - Idle, 1 - Busy

    uint32_t execution_time;    // C: total execution time per job
    uint32_t remaining_time;    // C': remaining time for current job
    uint32_t period;            // T: period for periodic tasks
    priority_t priority;        // derived from period (smaller period = higher priority)

    // for periodic jobs
    uint32_t next_arrival;      // next job arrival time (for periodic tasks)
    int max_jobs;               // maximum number of periodic instances (-1 = infinite, 0 = one-shot)
    int jobs_done;              // number of completed job instances
    kbool_t is_periodic;        // true if this is a periodic task

    struct thread_control_block *next; // next thread in the queue
    struct thread_control_block *prev; // previous thread in the queue
    // struct thread_control_block *left_child; // left child in the tree
    // struct thread_control_block *right_child; // right child in the tree
    // struct thread_control_block *parent; // parent thread
};

// in the heap we should have [thread priority, pointer to thread control block (TCB)]
typedef struct heap_node
{
    priority_t priority;   // The key for comparison (lower = higher priority)
    tcb* tcb;           // Pointer to the thread control block
} heap_node_t  __attribute__((packed));

// this heap needs to be a min-heap since lower values have higher priority
typedef struct Heap {
    heap_node_t *data; // Array of heap nodes
    size_t size;  // Current number of threads in the heap
    size_t capacity;  // Maximum capacity of the heap
} thread_heap_t;

struct segment_descriptor {
    uint32_t base; /* 32 bits, broken up into sections of 16/8/8*/
    uint32_t limit; /* limit is only 20 bits, broken up into a 16 bit section and a 4 bit section*/
    uint8_t access_byte;
    uint8_t flags; /* Flags is only 4 bits */
}__attribute__((packed));

typedef struct segment_descriptor gdt_t;

/* Segments for the flat mode GDT setup in 32-bit */
static gdt_t null_desc = {.base = 0, .limit = 0x00000000, .access_byte = 0x00, .flags = 0x0};
static gdt_t kernel_code = {.base = 0, .limit = 0xFFFFF, .access_byte = 0x9A, .flags = 0xC};
static gdt_t kernel_data = {.base = 0, .limit = 0xFFFFF, .access_byte = 0x92, .flags = 0xC};



#endif /* DEFNS_H */