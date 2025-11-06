# Group Info

- Alp Eren Yilmaz (BUID: U13910952)
- Andrew Lawson (BUID: U31011815)

We finished FIFOS-1 and FIFOS-2, and we have an implementation for DEADOS that partially works with some inputs.
To change between FIFOS1 and 2, please see line 6 in defns.h, you can set it to 1 to change to FIFOS-1.

In DEADOS we used the timer interrupt, and our time units is every tick in the timer interrupt. We did not use the rdtsc instruction that way, and all budgets, deadlines, and periods are defined in the same time unit, can be seen in line 432 in kentry.c. As far as we know our DEADOS implementation works when all the deadlines can be met, but sometimes in overload situation we encountered some bugs that caused it to not finish execution of every thread.

Our resources was the course slides, the Software Developer's Manual, the course textbook, and osdev.org.







