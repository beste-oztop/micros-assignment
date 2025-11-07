#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include "thread.h"
// this heap needs to be a min-heap since lower values have higher priority
// in the heap we should have [thread priority, pointer to thread control block (TCB)]
typedef struct Heap {
    int *data;
    size_t size;
    size_t capacity;
    priority_t priority;
    tcb* tcb;
} Heap;


/*
We need to implement: heapify(), heap_insert() and heap_remove() functions.
Just to be consistent with the naming convention.
*/

/* Create a new heap. If capacity == 0 a default is used. Returns NULL on alloc failure. */
Heap* heap_create(size_t capacity);

/* Free heap resources. Safe to call with NULL. */
void heap_destroy(Heap* h);

/* Push a value onto the heap. Returns 0 on success, -1 on failure. */
int heap_push(Heap* h, int value);

/* Peek at the min element (root). Returns 0 on success, -1 if empty. */
int heap_peek(Heap* h, int *out);

/* Pop the min element. Returns 0 on success, -1 if empty. */
int heap_pop(Heap* h, int *out);

/* Number of elements in the heap. */
size_t heap_size(const Heap* h);

/* Print heap array (for debugging). */
void heap_print(const Heap* h);

#endif /* HEAP_H */
