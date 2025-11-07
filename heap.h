#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include "thread.h"
#include "defs.h"


#ifdef KERNEL_MODE
#include "helpers.h"
#endif

// in the heap we should have [thread priority, pointer to thread control block (TCB)]
typedef struct heap_node
{
    priority_t priority;   // The key for comparison (lower = higher priority)
    tcb* tcb;           // Pointer to the thread control block
} heap_node_t;

// this heap needs to be a min-heap since lower values have higher priority
typedef struct Heap {
    heap_node_t *data; // Array of heap nodes
    size_t size;  // Current number of threads in the heap
    size_t capacity;  // Maximum capacity of the heap
} thread_heap_t;


/*
We need to implement: heapify(), heap_insert() and heap_remove() functions.
Just to be consistent with the HW's naming convention.

Each time new thread enters the system or existing thread is preempted, it's added to the heap
with heap_insert() based on its priority.
*/

/* Create a new heap. If capacity == 0 a default is used. Returns NULL on alloc failure. */
thread_heap_t* heap_create(size_t capacity);

/* Free heap resources. Safe to call with NULL. */
void heap_destroy(thread_heap_t* h);

/* Insert a thread into the heap based on its priority. Returns 0 on success, -1 on failure. */
int heap_insert(thread_heap_t* h, tcb* thread); // priority is derived from thread->priority

/* Peek at the min element (root). Returns 0 on success, -1 if empty. */
int heap_peek(thread_heap_t* h, heap_node_t *out);

/* Remove and return the highest priority thread. Returns NULL if empty. */
int heap_remove(thread_heap_t* h, heap_node_t *out);

/* Number of elements in the heap. */
size_t heap_size(const thread_heap_t* h);

/* Check if heap is empty. Returns 1 if empty, 0 otherwise. */
int heap_is_empty(const thread_heap_t* h);

/* Print heap array (for debugging). */
void heap_print(const thread_heap_t* h);

#endif /* HEAP_H */
