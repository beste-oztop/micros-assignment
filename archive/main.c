/* main.c: Main program for testing the heap implementation. Not part of the kernel*/
#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

int main(void) {
    heap_node_t *h = heap_create(0);
    if (!h) { fprintf(stderr, "heap allocation failed\n"); return 1; }

    int vals[] = {5, 3, 8, 1, 2, 9, 7};
    size_t n = sizeof(vals) / sizeof(vals[0]);

    for (size_t i = 0; i < n; ++i) {
        heap_push(h, vals[i]);
        printf("pushed %d, heap: ", vals[i]);
        heap_print(h);
    }

    printf("\npop sequence:\n");
    int v;
    while (heap_pop(h, &v) == 0) {
        printf("popped %d, heap: ", v);
        heap_print(h);
    }

    heap_destroy(h);
    return 0;
}
