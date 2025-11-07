#include "heap.h"
#include <stdlib.h>
#include <stdio.h>

// static void swap_int(int *a, int *b) { int t = *a; *a = *b; *b = t; }

/* Swap two heap nodes */
static void swap_node(heap_node_t *a, heap_node_t *b) {
    heap_node_t temp = *a;
    *a = *b;
    *b = temp;
}

thread_heap_t* heap_create(size_t capacity) {
    thread_heap_t *h = malloc(sizeof(thread_heap_t));
    if (!h) return NULL;
    if (capacity == 0) capacity = 8; // Default capacity
    h->data = malloc(sizeof(heap_node_t) * capacity);
    if (!h->data) {  // Allocation failed
        free(h);
        return NULL;
    }
    h->size = 0;
    h->capacity = capacity;
    return h;
}

void heap_destroy(thread_heap_t* h) {
    if (!h) return;
    free(h->data); // Free the array (doesn't free the threads themselves) frees the pointers to tcb
    free(h);
}

static int heap_resize(thread_heap_t* h, size_t newcap) {
    if (!h) return -1;
    heap_node_t *tmp = realloc(h->data, sizeof(heap_node_t) * newcap);
    if (!tmp) return -1;
    h->data = tmp;
    h->capacity = newcap;
    return 0;
}

/* MIN-HEAP sift_up: bubble smaller elements up so the root is the minimum */
static void sift_up(thread_heap_t* h, size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (h->data[parent].priority <= h->data[idx].priority) break; // min-heap: parent <= child
        swap_node(&h->data[parent], &h->data[idx]);
        idx = parent;
    }
}

static void sift_down(thread_heap_t* h, size_t idx) {
    for (;;) {
        size_t left = idx * 2 + 1;
        size_t right = left + 1;
        size_t smallest = idx;
        if (left < h->size && h->data[left].priority < h->data[smallest].priority) smallest = left;
        if (right < h->size && h->data[right].priority < h->data[smallest].priority) smallest = right;
        if (smallest == idx) break;
        swap_node(&h->data[smallest], &h->data[idx]);
        idx = smallest;
    }
}

int heap_insert(thread_heap_t* h, tcb* thread) {
    if (!h || !thread) return -1;

    // Resize if needed
    if (h->size >= h->capacity) {
        if (heap_resize(h, h->capacity * 2) != 0)
            return -1;  // Allocation failed
    }

    // Create a new heap node with thread's priority and pointer
    h->data[h->size].priority = thread->priority;
    h->data[h->size].tcb = thread;

    // Bubble up to maintain heap property
    sift_up(h, h->size);
    h->size++;

    return 0;
}

int heap_peek(thread_heap_t* h, heap_node_t *out) {
    if (!h || h->size == 0) return -1;
    if (out) *out = h->data[0];
    return 0;
}

int heap_remove(thread_heap_t* h, heap_node_t *out) {
    if (!h || h->size == 0) return -1;
    if (out) *out = h->data[0];
    h->data[0] = h->data[h->size - 1];
    h->size--;
    if (h->size > 0) sift_down(h, 0);
    if (h->size > 0 && h->size <= h->capacity / 4 && h->capacity > 8) {
        heap_resize(h, h->capacity / 2);
    }
    return 0;
}

size_t heap_size(const thread_heap_t* h) { return h ? h->size : 0; }

int heap_is_empty(const thread_heap_t* h) {
    return !h || h->size == 0;
}

#ifdef KERNEL_MODE
void heap_print(const thread_heap_t* h) {
    if (!h) return;
    puts("[");
    for (size_t i = 0; i < h->size; ++i) {
        char buffer[12];  // Enough for a 32-bit int + null terminator
        itoa(h->data[i].priority, buffer, 10);  // Convert to decimal string
        puts(buffer);
        if (i + 1 < h->size) puts(", ");
    }
    puts("]\n");
}
#else
void heap_print(const thread_heap_t* h) {
    if (!h) return;
    printf("[");
    for (size_t i = 0; i < h->size; ++i) {
        printf("%d", h->data[i].priority);
        if (i + 1 < h->size) printf(", ");
    }
    printf("]\n");
}
#endif
