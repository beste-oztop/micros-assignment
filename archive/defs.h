#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

#define MAX_THREADS 64
typedef struct ThreadControlBlock {
    uint32_t tid;
    uint32_t state;
    uint32_t sp;
    uint32_t bp;
    uint32_t entry;
    struct ThreadControlBlock *next;
    struct ThreadControlBlock *left_child;
    struct ThreadControlBlock *right_child;
    struct ThreadControlBlock *parent;
} TCB;

extern TCB *micros_threads[MAX_THREADS];

#endif /* DEFS_H */
