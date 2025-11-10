.globl _start
.globl idt
.global set_gdt
.bss /* Uninitialized global data */

.comm stack 0x1000 /* Reserve 4KB for stack area in bss */
.comm idt 0x800 /* Reserve 2KB for IDT in bss */
.comm int_table 0x800 /* Reserve 2KB for interrupt table */

.data
.align 16
.skip 16384  # reserve 16KB for the stack
stack_addr:

.text


gdtr: /* Global Descriptor Table Register - osdev */
	.word 0      /* Limit (2 bytes) */
	.long 0      /* Base (4 bytes) */

_start:
	jmp real_start
	# multiboot header - must be in 1st page of memory  for GRUB
	.align 4
	# multiboot stuff
	.long 0x1BADB002 # multiboot magic number
	.long 0x00000003 # aligns modules to 4KB
	.long 0xE4524FFB # multiboot checksum

real_start:
	# init the stack
	mov $stack_addr, %esp
	# pass arguments by pushing them onto the stack
    # last push becomes first argument
	push %eax   # multiboot magic number
	push %ebx   # multiboot mbd ptr
    # call the kernel main function
	call kmain
	hlt


set_gdt:
    movl  4(%esp), %eax    # MOV AX, [esp + 4] -> move 4 bytes at [esp+4] into %eax
    movw  %ax, gdtr        # MOV [gdtr], AX -> store %ax into gdtr (word, so use movw) -- store lower 2 bytes (limit)
    movl  8(%esp), %eax    # MOV EAX, [ESP + 8] -> move 4 bytes at [esp+8] into %eax
    movl  %eax, gdtr+2     # MOV [gdtr + 2], EAX -> store %eax into gdtr+2 (long, so use movl)  -- store  32-bit base address
    lgdt  gdtr             # LGDT[ gdtr] -> load GDTR with address of gdtr
    ret                    # RET

