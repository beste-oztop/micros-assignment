#include "types.h"

#ifndef _DEFNS_H
#define _DEFNS_H

#define FIFOS 2

#define MAX_THREADS 3

int thread_ids = 1;

static bool done[MAX_THREADS] = {0};
int counter[MAX_THREADS] = {0};

uint32_t *f[MAX_THREADS]; /*Array of pointers to function addresses*/
/* Currently how I set up the addresses for the stacks, each has ~4kb of space*/
uint32_t *stacks[MAX_THREADS] = {};
static tcb *fifos_threads[MAX_THREADS] = {0};

/* Segments for the flat mode GDT setup in 32-bit */
static segment_descriptor_t null_descriptor = {.base = 0, .limit = 0x00000000, .access_byte = 0x00, .flags = 0x0};
static segment_descriptor_t kernel_code = {.base = 0, .limit = 0xFFFFF, .access_byte = 0x9A, .flags = 0xC};
static segment_descriptor_t kernel_data = {.base = 0, .limit = 0xFFFFF, .access_byte = 0x92, .flags = 0xC};
static int total_free_mem = 0;
/* Assembly function for setting up GDT */


/* Base address of the VGA frame buffer */
static unsigned short *videoram = (unsigned short *) 0xB8000;
static int attrib = 0x0F; /* black background, white foreground */
static int csr_x = 0, csr_y = 0; /* x and Y position of cursor on screen*/
#define COLS 80


tcb *current; // rq *current; // Current thread in runqueue
tcb *finished;// rq *finished; // A thread that's finished execution
tcb *burner_thread;
int threads = MAX_THREADS;

static void* isr_stub_table[48];

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance
static idtr_t idtr; // Create an IDTR


int timer_counter = 0;

// timer interrupt is at IRQ0
// ISR returns must use iret instead of ret
// test the IDT
// I need to map IRQs to interrupt numbers beyond 31
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define PIC_EOI		0x20		/* End-of-interrupt command code */

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
#define INITIALIZE 0x11

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */


typedef struct node{
   tcb *t;
   struct node *left;
   struct node *right;
} node;

node* heap_memory = (node*)0x800000;
node* top_node = NULL;

int current_thread_id;
int current_time;

// all threads here finished their budget
// we will only check their current deadline
tcb* waiting[MAX_THREADS] = {0};
int waiting_size;

#endif