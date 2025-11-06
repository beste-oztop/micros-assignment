.bss
.comm idt 0x8000 # 2048 bytes for the IDT

idt_ptr:
    .short 0x7FF # 2047 bytes long, 8N-1 as the manual states
    .long idt

gdtr: .word 0 
      .long 0 

.text

.globl _start
.globl setGdt
.globl setIdt
.globl timer_interrupt_asm
.globl timer_interrupt_asm2
.globl other_interrupt_asm
.globl divide_by_zero_fault_asm
.globl single_step_trap_asm
.globl nmi_interrupt_asm
.globl breakpoint_trap_asm
.globl overflow_trap_asm
.globl bounds_check_fault_asm
.globl invalid_opcode_fault_asm
.globl no_device_fault_asm
.globl double_fault_abort_asm
.globl segment_overrun_fault_asm
.globl invalid_tss_fault_asm
.globl segment_not_present_asm
.globl stack_fault_asm
.globl page_fault_asm
.globl general_protection_fault_asm
.globl infinite_loop

_start:
    jmp real_start
    # Multiboot header – Must be in 1st page of memory for GRUB
    .align 4
    .long 0x1BADB002 # Multiboot magic number
    .long 0x00000003 # Align modules to 4KB, req. mem size
    # See 'info multiboot' for further info
    .long 0xE4524FFB # Checksum
real_start:
    # Disable interrupts
    cli

    # Setup a proper stack for C
    sub $16384, %esp # Reserve 8KB of stack space, I guess no more is needed?
    # 16384 is not a magic number, it is the stack size set in multiboot.h
    # Prepare the boot information to pass to kmain
    push %ebx # The address of this structure to be passed to the kernel is in %ebx
    # the first argument of the function is the address of the multiboot structure
    # The cdecl (which stands for C declaration) is a calling convention 
    # for the programming language C and is used by many C compilers for the x86 architecture.[1] 
    # In cdecl, subroutine arguments are passed on the stack.
    # https://en.m.wikipedia.org/wiki/X86_calling_conventions
    
    call kmain
    hlt

setGdt:
    movl  4(%esp), %eax    # MOV AX, [esp + 4] -> move 4 bytes at [esp+4] into %eax
    movw  %ax, gdtr        # MOV [gdtr], AX -> store %ax into gdtr (word, so use movw)
    movl  8(%esp), %eax    # MOV EAX, [ESP + 8] -> move 4 bytes at [esp+8] into %eax
    movl  %eax, gdtr+2     # MOV [gdtr + 2], EAX -> store %eax into gdtr+2 (long, so use movl)
    lgdt  gdtr             # LGDT [gdtr] -> load GDTR with address of gdtr
    ret                    # RET


other_interrupt_asm:
    pushal
    cld # osdev tells us to do this
    call other_interrupt
    popal
    iret

divide_by_zero_fault_asm:
    pushal
    cld # osdev tells us to do this
    call divide_by_zero_fault
    popal
    iret

single_step_trap_asm:
    pushal
    cld # osdev tells us to do this
    call single_step_trap
    popal
    iret

nmi_interrupt_asm:
    pushal
    cld # osdev tells us to do this
    call nmi_interrupt
    popal
    iret

breakpoint_trap_asm:
    pushal
    cld # osdev tells us to do this
    call breakpoint_trap
    popal
    iret

overflow_trap_asm:
    pushal
    cld # osdev tells us to do this
    call overflow_trap
    popal
    iret

bounds_check_fault_asm:
    pushal
    cld # osdev tells us to do this
    call bounds_check_fault
    popal
    iret

invalid_opcode_fault_asm:
    pushal
    cld # osdev tells us to do this
    call invalid_opcode_fault
    popal
    iret

no_device_fault_asm:
    pushal
    cld # osdev tells us to do this
    call no_device_fault
    popal
    iret

double_fault_abort_asm:
    pushal
    cld # osdev tells us to do this
    call double_fault_abort
    popal
    iret

segment_overrun_fault_asm:
    pushal
    cld # osdev tells us to do this
    call segment_overrun_fault
    popal
    iret

invalid_tss_fault_asm:
    pushal
    cld # osdev tells us to do this
    call invalid_tss_fault
    popal
    iret

segment_not_present_asm:
    pushal
    cld # osdev tells us to do this
    call no_segment_fault
    popal
    iret

stack_fault_asm:
    pushal
    cld # osdev tells us to do this
    call stack_fault
    popal
    iret

page_fault_asm:
    pushal
    cld # osdev tells us to do this
    call page_fault
    popal
    iret

general_protection_fault_asm:
    pushal
    cld # osdev tells us to do this
    call general_protection_fault
    popal
    iret

timer_interrupt_asm2:
    cli
    pushal
    cld # osdev tells us to do this
    call timer_interrupt
    popal
    sti
    iret


timer_interrupt_asm:
    cli
    pushal
    cld # osdev tells us to do this
    call PIC_sendEOI_0
    popal
    # call timer_interrupt
    # popal
    # # sti

    # mov registers inside memory somewhere
    mov %eax, 0x123122
    mov %ebx, 0x123126

    # popl %ebx # error code
    popl %eax # eip
    popl %ebx # cs
    popl %ebx # eflags

    pushl %eax
    mov 0x123122, %eax
    mov 0x123126, %ebx

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

    sti
    ret

