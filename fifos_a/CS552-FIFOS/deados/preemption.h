#include "defns.h"
#include "helpers.h"

void PIC_sendEOI(char irq)
{
	if(irq >= 8)
		outb(PIC2_COMMAND,PIC_EOI);
	
	outb(PIC1_COMMAND,PIC_EOI);
}

void PIC_sendEOI_0(){
    outb(PIC1_COMMAND, PIC_EOI);
    //puts("S");
}


void other_interrupt() {
    puts("Exception occurred\n");
    PIC_sendEOI(0);
    //__asm__ volatile ("hlt");
}

void divide_by_zero_fault() {
    puts("Divide by zero fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void single_step_trap() {
    puts("Single step trap\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void nmi_interrupt() {
    puts("NMI trap\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void breakpoint_trap() {
    puts("Breakpoint trap\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void overflow_trap() {
    puts("Overflow trap\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void bounds_check_fault() {
    puts("Bounds check fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void invalid_opcode_fault() {
    puts("Invalid opcode fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void no_device_fault() {
    puts("No device fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void double_fault_abort() {
    puts("Double fault abort\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void segment_overrun_fault() {
    puts("Segment overrun fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void invalid_tss_fault() {
    puts("Invalid TSS fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void no_segment_fault() {
    puts("No segment fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void stack_fault() {
    puts("Stack fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void general_protection_fault() {
    puts("General protection fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void page_fault() {
    puts("Page fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void intel_reserved_fault() {
    puts("Intel reserved fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void floating_point_fault() {
    puts("Floating point fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void alignment_check_fault() {
    puts("Alignment check fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}

void machine_check_fault() {
    puts("Machine check fault\n");
    PIC_sendEOI(0);
    __asm__ volatile ("hlt");
}




/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(int offset1, int offset2)
{
	char a1, a2;
	
	a1 = inb(PIC1_DATA);                        // save masks
	a2 = inb(PIC2_DATA);
	
	outb(PIC1_COMMAND, INITIALIZE);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_COMMAND, INITIALIZE);
	io_wait();
	outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset 0x20
	io_wait();
	outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
	
	outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode) 0D
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();
	
	outb(PIC1_DATA, a1);   // restore saved masks.
	outb(PIC2_DATA, a2);
}


// PIT chip uses the following I/O ports
/*
I/O port     Usage
0x40         Channel 0 data port (read/write)
0x41         Channel 1 data port (read/write)
0x42         Channel 2 data port (read/write)
0x43         Mode/Command register (write only, a read is ignored)
*/
void init_pit(){
    outb(0x43, 0x34); // Channel 0, access mode lobyte/hibyte, rate generator
    // PIT frequency is 1193182 Hz
    // Let's say frequency we want is 100 Hz
    // 1193182 / 100 = 11931
    // 11931 = 0x2E9B
    int pit_frequency = 1193182;
    int required_frequency = 10;
    int divisor = pit_frequency / required_frequency;
    outb(0x40, divisor & 0xFF); // Low byte // WHY?
    outb(0x40, (divisor >> 8) & 0xFF); // High byte

    __asm__ volatile("sti");
}

void timer_handler(void){
    static int count = 0;
    count++;
    // puts("Timer interrupt\n");
    PIC_sendEOI(0);
}

void idt_set_descriptor(char vector, void* isr, char flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low        = (int)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
    descriptor->attributes     = flags;
    descriptor->isr_high       = (int)isr >> 16;
    descriptor->reserved       = 0;
}


void irq_handler(int irq){
    switch(irq){
        case 0:
            timer_handler();
            break;
        default:
            break;
    }
}

