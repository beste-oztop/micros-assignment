#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "helpers.h"
#include "thread.h"
#include "heap.h"
#include "defs.h"

/* Function declarations */
/* Schedule threads using Rate-Monotonic Scheduling */
void schedule_rm(void);
void schedule_rm_org(void);
static int rm_cmp(const void *a, const void *b);

#endif /* SCHEDULER_H */