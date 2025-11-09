#ifndef HEAP_H
#define HEAP_H


// #include "thread.h"
#include "defs.h"
#include "helpers.h"

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
