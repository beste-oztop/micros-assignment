#include "heap.h"
#include <stdlib.h>
#include <stdio.h>

static void swap_int(int *a, int *b) { int t = *a; *a = *b; *b = t; }

Heap* heap_create(size_t capacity) {
    Heap *h = malloc(sizeof(Heap));
    if (!h) return NULL;
    if (capacity == 0) capacity = 8;
    h->data = malloc(sizeof(int) * capacity);
    if (!h->data) { free(h); return NULL; }
    h->size = 0;
    h->capacity = capacity;
    return h;
}

void heap_destroy(Heap* h) {
    if (!h) return;
    free(h->data);
    free(h);
}

static int heap_resize(Heap* h, size_t newcap) {
    int *tmp = realloc(h->data, sizeof(int) * newcap);
    if (!tmp) return -1;
    h->data = tmp;
    h->capacity = newcap;
    return 0;
}

/* MIN-HEAP sift_up: bubble smaller elements up so the root is the minimum */
static void sift_up(Heap* h, size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (h->data[parent] <= h->data[idx]) break; // min-heap: parent <= child
        swap_int(&h->data[parent], &h->data[idx]);
        idx = parent;
    }
}

static void sift_down(Heap* h, size_t idx) {
    for (;;) {
        size_t left = idx * 2 + 1;
        size_t right = left + 1;
        size_t smallest = idx;
        if (left < h->size && h->data[left] < h->data[smallest]) smallest = left;
        if (right < h->size && h->data[right] < h->data[smallest]) smallest = right;
        if (smallest == idx) break;
        swap_int(&h->data[smallest], &h->data[idx]);
        idx = smallest;
    }
}

int heap_push(Heap* h, int value) {
    if (!h) return -1;
    if (h->size >= h->capacity) {
        if (heap_resize(h, h->capacity * 2) != 0) return -1;
    }
    h->data[h->size++] = value;
    sift_up(h, h->size - 1);
    return 0;
}

int heap_peek(Heap* h, int *out) {
    if (!h || h->size == 0) return -1;
    if (out) *out = h->data[0];
    return 0;
}

int heap_pop(Heap* h, int *out) {
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

size_t heap_size(const Heap* h) { return h ? h->size : 0; }

void heap_print(const Heap* h) {
    if (!h) return;
    printf("[");
    for (size_t i = 0; i < h->size; ++i) {
        printf("%d", h->data[i]);
        if (i + 1 < h->size) printf(", ");
    }
    printf("]\n");
}
