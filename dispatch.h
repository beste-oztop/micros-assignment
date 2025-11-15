#include "thread.h"
#include "helpers.h"
#include "defs.h"
#include "heap.h"
#include "scheduler.h"
#ifndef DISPATCH_H
#define DISPATCH_H

/* External runtime/thread state */
extern int curr_tid;
extern tcb *current_thread;  // this is a global pointer to the currently running thread
extern tcb *micros_threads[MAX_THREADS];

/* Prototypes for dispatching / scheduling */
void dispatcher(void);
void dispatch_first_run(void);
void yield(void);

#endif // DISPATCH_H
