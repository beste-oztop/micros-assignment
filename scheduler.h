#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "helpers.h"
#include "thread.h"
#include "heap.h"
#include "defs.h"

/* Function declarations */
void schedule_rm(void);
static int rm_cmp(const void *a, const void *b);

#endif /* SCHEDULER_H */