.globl _start
.globl idt

.bss /* Uninitialized global data */

.comm stack 0x1000 /* Reserve 4KB for stack area in bss */
.comm idt 0x800 /* Reserve 2KB for IDT in bss */
.comm int_table 0x800 /* Reserve 2KB for interrupt table */

.data
.align 0x4


gdt: /* Global Descriptor Table */
    .long 0 /* Null descriptor */
    .long 0 /* 8 bytes*/

    /* Kernel Code Segment Descriptor */
    .long 0x0000FFFF /* Limit Low, Base Low */
    .long 0x00CF9A00 /* Base:0, Access, Limit:0xFFFFFFFF, Ring:0, Type: A(Readable) */

    /* Kernel Data Segment Descriptor */
    .long 0x0000FFFF /* Limit Low, Base Low */
    .long 0x00CF9200 /* Base:0, Access, Limit:0xFFFFFFFF, Ring:0, Type: 2(Writable) */

gdt_ptr:
    .short 0x7FF /* Length of GDT in bytes, 256(?2048??) bytes. We have 3 descriptors total of 24 bytes */
    .long gdt /* 4 byte Linear (base) address of GDT */

idt_ptr:
    .short 0x7FF /* Length of IDT in bytes, 2048 bits */
    .long idt /* Linear (base) address of IDT */
    .text
    .globl _start

_start:
    /* From memos-2 */
    jmp real_start
	.align 4 # multiboot header - must be in 1st page of memory  for GRUB
	# multiboot stuff
	.long 0x1BADB002 # multiboot magic number
	.long 0x00000003 # aligns modules to 4KB
	.long 0xE4524FFB # multiboot checksum

real_start:
    /* Load the GDT */
    lgdt gdt_ptr

    /* Perform a far jump to flush the prefetch queue and load CS */
    ljmp $0x08, $0x1f


1:   /* Set up Data Segment Registers */
    mov $0x10, %ax /* 2nd descriptor from GDT: Kernel DS */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Initialize the stack pointer to 4KB */
    movl stack + 0x1000, %esp /* Point ESP to the top of the stack since stack grows downwards */

    /* Save multiboot parameters before C code call */
    pushl %ebx /* Multiboot info pointer */

    /* Set up IDT */
    /* We can do this part in the C code as well (and actually, we should!! */
    movl $idt, %edi
    movl $0x30, %ecx /* 0x30 = 48 entries, each 8 bytes - set counter for 48 idt entries used*/
    movl $0x080000, %edx /* Interrupt gate descriptor for 32-bit interrupt - ? used to index kernel code segment ?*/
    movl $int_table, %esi /* Base address for the interrupt table */

1:
    lodsl
    movw %ax, %dx /* Lower 16 bits of int_table */
    movw $0xEE00, %ax /* DPL = 3, Present = 1, Type = 0xE (32-bit interrupt gate) ??? */
    movw %edx, (%edi) /* Store bottom 32 bits */
    addl $4, %edi
    stosl /* Store top 32 bits */
    loop 1b


    /* Load the IDT */
    lidt idt_ptr

    /* Call the main kernel function */
    /* TODO Prepare arguments for kmain */
    call kmain /* Call the C function 'kmain' */
    cli /* Clear interrupts */
    hlt /* Halt the CPU */

loop:
    jmp loop /* Infinite loop to prevent falling through */