
.globl context_switch
.globl yield
.globl dispatcher
.globl dispatch_first_run




context_switch:
    pushfl
    pushal

    # do we need to push the segment registers, too?
    pushw %ds
    pushw %es
    pushw %fs
    pushw %gs

    # save the stack pointer
    movl %esp, %eax

    # get the new stack pointer with $edi
    movl (%edi), %esp

    # now pop the segment registers
    popw %gs
    popw %fs
    popw %es
    popw %ds

    popal
    popfl

    ret

yield:
    call dispatcher


dispatcher:
    pushfl
    pushal
    
    # do we need to push the segment registers, too?
    pushw %ds
    pushw %es
    pushw %fs
    pushw %gs

    # save the stack pointer
    movl %esp, %eax
    pushl %eax
    call save_prev_stack

    call save_curr_tid
    call schedule
    call save_next_tid
    call save_next_stack

    # get the new stack pointer with $edi
    movl %edi, %esp

    # now pop the segment registers
    popw %gs
    popw %fs
    popw %es
    popw %ds

    popal
    popfl

    ret

dispatch_first_run:
    call save_next_stack

    movl %edi, %esp

    # now pop the segment registers
    popw %gs
    popw %fs
    popw %es
    popw %ds

    popal
    popfl

    ret




