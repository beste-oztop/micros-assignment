# MICROS Assignment - Project To-Do List

## Overview
This document tracks the development progress for the MICROS (Microsecond Time-aware System) assignment. The system implements rate-monotonic scheduling with timer-based preemption for thread management.

---

## Phase 1: Core Infrastructure Setup
**Status**: Partially Complete ✓

- [x] Basic heap structure defined in `heap.h`
- [x] Basic TCB structure defined in `helpers.h`
- [ ] Fix heap implementation - currently max-heap, needs to be **min-heap** (lower priority values = higher priority)
- [ ] Update TCB structure to include scheduling parameters:
  - Add `execution_time` (C)
  - Add `period` (T)
  - Add `priority` (calculated as inversely proportional to period)
  - Add `remaining_time` (C')
  - Add `job_count` (for tracking periodic instances)
  - Add `max_jobs` (maximum number of job executions)
  - Add `next_arrival_time`

---

## Phase 2: Thread Pool & Creation
**Status**: Not Started

- [ ] Update `thread_create()` signature to accept args:
  ```c
  int thread_create(void *stack, void *func, void *args);
  ```
- [ ] Define `struct sched_params_t` structure for C and T parameters
- [ ] Modify `thread_create()` to parse and store scheduling parameters from args
- [ ] Initialize thread pool (array of N TCBs) at boot time
- [ ] Implement thread pool management (mark busy/idle)
- [ ] Implement thread termination and return to idle state

---

## Phase 3: Heap-Based Priority Queue
**Status**: Not Started

- [ ] Implement `heap_insert()` - add thread to heap based on priority
- [ ] Implement `heap_remove()` - remove and return highest priority thread
- [ ] Implement `heapify()` - rebuild heap maintaining min-heap property
- [ ] Modify heap structure to store `[priority, TCB pointer]` instead of just integers
- [ ] Create ready queue management functions:
  - `add_to_ready_queue(tcb *thread)`
  - `get_next_thread()`
  - `remove_from_ready_queue(tcb *thread)`

---

## Phase 4: Basic Non-Preemptive Scheduling (Step 1)
**Status**: Not Started

- [ ] Implement basic `schedule_rm()` function
- [ ] Pick threads from heap in priority order
- [ ] Run each thread to completion (non-preemptive)
- [ ] Create simple test thread functions that print `<TID,C,T>` format
- [ ] Implement output formatting functions for tuple printing

---

## Phase 5: Stack Support (Step 2)
**Status**: Partially Complete

- [x] Stack allocation partially implemented in `thread_create()`
- [ ] Allocate stack memory for each thread (define stack size)
- [ ] Test threads with local variables and function calls
- [ ] Verify stack pointer setup is correct

---

## Phase 6: Cooperative Preemption (Step 3)
**Status**: Not Started

- [ ] Implement `yield()` function
- [ ] Create wait queue for yielded threads
- [ ] Save current thread context on yield
- [ ] Load next highest priority thread from ready queue
- [ ] Test: all threads run once in priority order then suspend

---

## Phase 7: Timer-Based Preemption Setup
**Status**: Not Started

- [ ] Setup Global Descriptor Table (GDT) if not already done
  - Null segment
  - Kernel code segment
  - Kernel data segment
- [ ] Implement `lgdt` instruction call
- [ ] Setup Interrupt Descriptor Table (IDT)
  - Create interrupt gate descriptors
  - Setup timer interrupt vector
- [ ] Implement `lidt` instruction call

---

## Phase 8: PIT and PIC Configuration
**Status**: Not Started

- [ ] Program 8253/4 Programmable Interval Timer (PIT)
  - Set timer frequency/period
  - Calculate divisor for desired time intervals
- [ ] Program 8259 Programmable Interrupt Controller (PIC)
  - Initialize PIC
  - Setup interrupt masks
  - Enable timer interrupt

---

## Phase 9: Context Switching
**Status**: Not Started

- [ ] Implement software context switching using `pusha`/`popa`
- [ ] Save/restore general purpose registers
- [ ] Save/restore segment selectors (ss, ds, es, fs, gs)
- [ ] Save/restore stack pointer (esp) and base pointer (ebp)
- [ ] Save/restore instruction pointer (eip)
- [ ] Save/restore flags register (eflags)

---

## Phase 10: Timer Interrupt Handler
**Status**: Not Started

- [ ] Implement timer interrupt handler in assembly (`interrupt_handler.S`)
- [ ] Call `rms_schedule()` on timeout
- [ ] Implement critical sections with `cli`/`sti`
- [ ] Test interrupt enabling/disabling

---

## Phase 11: Full Rate-Monotonic Scheduling
**Status**: Not Started

- [ ] Calculate thread priorities (inversely proportional to period)
- [ ] Track remaining execution time (C') for each thread
- [ ] Implement job completion logic
- [ ] Implement new job arrival logic (after period T)
- [ ] Preempt current thread if higher priority arrives
- [ ] Update heap when thread is preempted
- [ ] Set timer for next event (C' completion or job arrival)

---

## Phase 12: Time Tracking & Busy Waiting
**Status**: Not Started

- [ ] Implement time tracking mechanism
- [ ] Track CPU time consumed by each thread
- [ ] Implement busy waiting or CPU-consuming function
- [ ] Account for time used across preemptions

---

## Phase 13: Testing & Output
**Status**: Not Started

- [ ] Create test thread functions that:
  - Print `<TID,C,T>` when running
  - Run for 5-10 seconds total
  - Print `"Done <TID>"` when terminating
- [ ] Test with 3-7+ threads
- [ ] Implement configurable thread parameters
- [ ] Test with varying C and T values
- [ ] Verify rate-monotonic priority ordering

---

## Phase 14: Documentation
**Status**: Not Started

- [ ] Write comprehensive README with:
  - How to build (Makefile usage)
  - How to run (QEMU command)
  - How to test
  - Test case examples
  - Division of labor between team members
  - References and sources used
- [ ] Comment all code thoroughly
- [ ] Document assumptions made

---

## Bonus (Optional)
**Status**: Not Started

- [ ] Implement timeline output in seconds/microseconds
- [ ] Show execution traces with start/stop/preemption times
- [ ] Create visual timeline representation

---

## Build System
**Status**: Not Started

- [ ] Create/update Makefile
  - Compile all C files
  - Assemble all assembly files
  - Link into kernel binary
  - Clean targets
- [ ] Create `menu.lst` for GRUB if needed
- [ ] Document QEMU testing commands

---

## Current Status Notes

### ✅ Complete
- Basic heap structure defined
- Basic TCB structure exists
- `thread_create()` function exists

### ⚠️ Needs Work
- Heap implementation is max-heap, needs to be min-heap
- `schedule_rm()` is empty stub
- TCB missing scheduling parameters

### ❌ Not Started
- Timer/interrupt handling
- Context switching implementation
- PIT/PIC programming
- Full rate-monotonic scheduling logic

---

## Key Resources

- Intel Software Developer's Manual Volume 3 (Systems Programming)
- Intel Software Developer's Manual Volume 2 (Instruction Set Reference)
- [OSDev.org](https://www.osdev.org) - PIT and PIC programming guides
- Course webpage helper files from memos-2

---

## Testing Environment

- **OS**: Puppy Linux (recommended)
- **Emulator**: QEMU
- **Command**: `qemu-system-i386 -kernel micros -m XXX`
- **Architecture**: IA32 (32-bit x86)
- **Memory Model**: Flat memory layout (no paging required)

---

## Assignment Requirements Summary

- **Minimum threads**: 3 (recommended: 7+)
- **Thread execution time**: 5-10 seconds total per thread
- **Output format**: `<TID,C,T>` tuples
- **Termination message**: `"Done <TID>"`
- **Scheduling**: Rate-monotonic (priority inversely proportional to period)
- **Data structure**: Min-heap based priority queue
- **Team size**: Up to 2 people

---

## Notes

- All code runs in kernel protection domain
- 32-bit code and data
- Flat memory model up to 4GB logical space
- No paging required
- Do not share code on public websites/repositories
- Document all contributions from each team member
- Cite all external resources used

---

**Last Updated**: November 7, 2025
