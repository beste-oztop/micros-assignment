.global int_table
.section .data
.align 0x04

int_table:
    .long exception_handler_0 /* We should set up some generic functions to test in the C code */
    .long exception_handler_1
    .long exception_handler_2
    .long exception_handler_3
    .long exception_handler_4
    .long exception_handler_5
    .long exception_handler_6
    .long exception_handler_7
    .long exception_handler_8
    .long exception_handler_9
    .long exception_handler_10
    .long exception_handler_11
    .long exception_handler_12
    .long exception_handler_13
    .long exception_handler_14
    .long exception_handler_15  
    .long exception_handler_16
    .long exception_handler_17
    .long exception_handler_18
    .long exception_handler_19
    .long 0 /* Fill the rest with zeros for now */
    .long 0 /* 21 */
    .long 0 /* 22 */
    .long 0 /* 23 */
    .long 0 /* 24 */
    .long 0 /* 25 */
    .long 0 /* 26 */
    .long 0 /* 27 */
    .long 0 /* 28 */
    .long 0 /* 29 */
    .long 0 /* 30 */
    .long 0 /* 31 */
    .long timer         /* TIMER INTERRUPT: IRQ0 */ 