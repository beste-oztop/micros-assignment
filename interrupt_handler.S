.global int_table
.global IRQ_CTX_SAVE
.global IRQ_CTX_RESTORE
.section .data
.section .text
.align 0x04


/* * Macros to save and restore CPU context during IRQ handling
    * NOT SUPER SURE ABOUT THESE!!!
 */
.macro IRQ_CTX_SAVE
    pushl %ds
    pushl %es
    pusha
    pushfl
.endm

.macro IRQ_CTX_RESTORE
    popfl
    popa
    popl %es
    popl %ds
.endm



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
    .long timer         /* TIMER INTERRUPT: IRQ0 at 0x20 */ 
    .long unhandled_interrupt0 /* IRQ1 */
    .long unhandled_interrupt0 /* IRQ2 */
    .long unhandled_interrupt0 /* IRQ3 */
    .long unhandled_interrupt0 /* IRQ4 */
    .long unhandled_interrupt0 /* IRQ5 */
    .long unhandled_interrupt0 /* IRQ6 */       
    .long unhandled_interrupt0 /* IRQ7 */
    .long unhandled_interrupt0 /* IRQ8 */
    .long unhandled_interrupt0 /* IRQ9 */
    .long unhandled_interrupt0 /* IRQ10 */
    .long unhandled_interrupt0 /* IRQ11 */
    .long unhandled_interrupt0 /* IRQ12 */
    .long unhandled_interrupt0 /* IRQ13 */
    .long unhandled_interrupt0 /* IRQ14 */
    .long unhandled_interrupt0 /* IRQ15 */

exception_handler_0:
    pushl $0  /* Push error code 0 onto stack */
    IRQ_CTX_SAVE /* MACRO DEFN to save context */
    pushl $0  /* Push interrupt number */
    call handle_exception
    IRQ_CTX_RESTORE /* MACRO DEFN to restore context */
    addl $8, %esp /* Clean up stack (error code + int number) */
    iret

exception_handler_1:
    pushl $0
    IRQ_CTX_SAVE
    pushl $1
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret    

exception_handler_2:
    pushl $0
    IRQ_CTX_SAVE
    pushl $2
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_3:
    pushl $0
    IRQ_CTX_SAVE
    pushl $3
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_4:
    pushl $0
    IRQ_CTX_SAVE
    pushl $4
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_5:
    pushl $0
    IRQ_CTX_SAVE
    pushl $5
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_6:
    pushl $0
    IRQ_CTX_SAVE
    pushl $6
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_7:
    pushl $0
    IRQ_CTX_SAVE
    pushl $7
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_8:
    pushl $0
    IRQ_CTX_SAVE
    pushl $8
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_9:
    pushl $0
    IRQ_CTX_SAVE
    pushl $9
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_10:
    pushl $0
    IRQ_CTX_SAVE
    pushl $10
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_11:
    pushl $0
    IRQ_CTX_SAVE
    pushl $11
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_12:
    pushl $0
    IRQ_CTX_SAVE
    pushl $12
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_13:
    pushl $0
    IRQ_CTX_SAVE
    pushl $13
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_14:
    pushl $0
    IRQ_CTX_SAVE
    pushl $14
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_15:
    pushl $0
    IRQ_CTX_SAVE
    pushl $15
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_16:
    pushl $0
    IRQ_CTX_SAVE
    pushl $16
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_17:
    pushl $0
    IRQ_CTX_SAVE
    pushl $17
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_18:
    pushl $0
    IRQ_CTX_SAVE
    pushl $18
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
    iret

exception_handler_19:
    pushl $0
    IRQ_CTX_SAVE
    pushl $19
    call handle_exception
    IRQ_CTX_RESTORE
    addl $8, %esp
ret

