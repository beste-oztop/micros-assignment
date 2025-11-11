.globl yield
.globl dispatcher
.globl dispatch_first_run

yield:
    call dispatcher

dispatcher:
    pushfl
    pushal
    
    # Push segment registers
    pushw %ds
    pushw %es
    pushw %fs
    pushw %gs

    # Save current stack pointer
    movl %esp, %eax
    pushl %eax
    call save_prev_stack


    call save_curr_tid
    call schedule_rm
    call save_next_tid
    call save_next_stack

    # Load the new stack pointer into %esp
    movl %edi, %esp

    # Pop the segment registers
    popw %gs
    popw %fs
    popw %es
    popw %ds

    popal
    popfl

    ret

dispatch_first_run:
    # BEfore calling this function, %edi should have the stack pointer of the first thread to run
    call save_next_stack

    movl %edi, %esp

    # Pop the segment registers
    popw %gs
    popw %fs
    popw %es
    popw %ds

    popal
    popfl

    sti
    ret




