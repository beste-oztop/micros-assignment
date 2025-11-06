
#include "multiboot.h" /* this should include multiboot.h */
#include "types.h"
#include "defns.h"
#include "helpers.h"
#include "multiboot.h"
#include "preemption.h"

extern void setGdt(uint16_t limit, uint32_t base);
extern void dispatch_first_run();
extern void yield(void);
extern void setIdt();

extern void timer_interrupt_asm();
extern void timer_interrupt_asm2();
extern void other_interrupt_asm();
extern void divide_by_zero_fault_asm();
extern void single_step_trap_asm();
extern void nmi_interrupt_asm();
extern void breakpoint_trap_asm();
extern void overflow_trap_asm();
extern void bounds_check_fault_asm();
extern void invalid_opcode_fault_asm();
extern void no_device_fault_asm();
extern void double_fault_abort_asm();
extern void segment_overrun_fault_asm();
extern void invalid_tss_fault_asm();
extern void segment_not_present_asm();
extern void stack_fault_asm();
extern void general_protection_fault_asm();
extern void page_fault_asm();


void set_IDT_C() {
    idtr.base = (int)&idt[0];
    idtr.limit = (short)sizeof(idt_entry_t) * 48 - 1;

    for(int i = 0; i < 48; i++){
        isr_stub_table[i] = &other_interrupt_asm;
    }
    isr_stub_table[14] = &page_fault_asm;
    isr_stub_table[13] = &general_protection_fault_asm;
    isr_stub_table[12] = &stack_fault_asm;
    isr_stub_table[11] = &segment_not_present_asm;
    isr_stub_table[10] = &invalid_tss_fault_asm;
    isr_stub_table[9] = &segment_overrun_fault_asm;
    isr_stub_table[8] = &double_fault_abort_asm;
    isr_stub_table[7] = &no_device_fault_asm;
    isr_stub_table[6] = &invalid_opcode_fault_asm;
    isr_stub_table[5] = &bounds_check_fault_asm;
    isr_stub_table[4] = &overflow_trap_asm;
    isr_stub_table[3] = &breakpoint_trap_asm;
    isr_stub_table[2] = &nmi_interrupt_asm;
    isr_stub_table[1] = &single_step_trap_asm;
    isr_stub_table[0] = &divide_by_zero_fault_asm;
    isr_stub_table[32] = &timer_interrupt_asm;

    for(int i = 0; i < 48; i++){
        idt_set_descriptor(i, isr_stub_table[i], 0x8E);
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
    // just to test, trigger one exception
    // __asm__ volatile ("int $0x3");
}

void encodeGdtEntry(uint8_t *target, segment_descriptor_t source){
    // Check limit to ensure it can be encoded
    if (source.limit > 0xFFFFF){puts("GDT cannot encode limits larger than 0xFFFFF");}

    // Encode the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] = (source.limit >> 16) & 0x0F;

    // Encode the base 
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;

    // Encode the access byte
    target[5] = source.access_byte;

    // Encode the flags
    target[6] |= (source.flags << 4);
}



int schedule(void){
    if(current){
        /* thread id is 1-3, index into list is 0-2*/
        if(done[(current->tid)-1]){
            if(threads == 1) {
                __asm__ volatile ("cli; hlt");
                // busy_wait();
                return 1;
            }
            // We are done with the current target thread, so unlink it from its neighbors                
            finished = current; 
            finished->prev->next = current->next;
            current = current->next;
            current->prev = finished->prev;

            if(current->next == finished){// Down to last thread
                current->next = finished->next;
            }
            threads--;
            // puts("Threads ");
            // char buff[16];
            // itoa(threads, buff, 10);
            // puts(buff);
            // puts(" left\n");
        }
        else{
            current = current->next;
        }
        return 0;
    }
    return 0;
}

void save_prev_stack(int prev_stack){
    current->sp = prev_stack;
}

int curr_tid;
void save_curr_tid(){
    curr_tid = current->tid;
}

int next_tid;
void save_next_tid(){
    next_tid = current->tid;
}

void save_next_stack(){
    __asm__ volatile ("mov %0, %%edi"::"r"(current->sp));
}

int thread(){
    int id = thread_ids++;
    int i;
    char buff[16];
    itoa(id, buff, 10);
    while(1){
        for (i = 0; i < 10; i++){
            puts(buff);
            busy_wait();
        }
        putc('\n');
        if(FIFOS==1)
            yield();
        if(++counter[id-1] == 3)
            break;
    }
    puts("Done ");
    puts(buff);
    puts(" !\n");
    
    done[id-1] = TRUE;
    return 0;
}

void build_runqueue(){
    tcb *ptr, *pptr;
 
    pptr = fifos_threads[0];

    for (int t = 1; t < MAX_THREADS; t++){
        ptr = fifos_threads[t];
        pptr->next = ptr;
        ptr->prev = pptr;

        pptr = ptr;
    }

    pptr->next = fifos_threads[0];
    fifos_threads[0]->prev = pptr;
}


void exit_thread(void){
    // change the state
    current->state = 0;
    if(FIFOS == 1){
        int flag = schedule();
        if(flag==0)
            dispatch_first_run();
        else{
            __asm__ volatile ("hlt");
        }
    }
    else{
        __asm__ volatile ("hlt");
        // puts("danger");
        // dispatch_first_run();
    }
}

void timer_interrupt(){
    // stop interrupts
    PIC_sendEOI(0);
    timer_counter++;
    if(timer_counter % 10 == 0){
        puts("+");
        yield();
    }
}

int thread_create(void *stack, void *func){
    // puts("creating thread!\n");
    int new_tcb = -1;
    uint16_t ds = 0x10, es = 0x10, fs = 0x10, gs = 0x10;

    new_tcb = get_tcb();
    /* I don't think we should ever reach this point, but I have it anyways...*/
    if(new_tcb == -1){
        puts("No TCB available!\n");
        return -1;
    }

    /*Put exit_thread on top of thread stack so that user thread can "return". */
    /* This should be some sort of function that removes the thread from the runqueue and resets its state?*/
    *(((uint32_t *) stack) - 0) = (uint32_t) exit_thread;
    stack = (void *)(((uint32_t *) stack) - 1);

    /*Found new TCB*/
    fifos_threads[new_tcb]->tid = new_tcb + 1;
    fifos_threads[new_tcb]->bp = (uint32_t) stack;
    fifos_threads[new_tcb]->entry = (uint32_t) func;
    fifos_threads[new_tcb]->state = 1; /* Work has now been assigned to this tcb / thread */
    fifos_threads[new_tcb]->next = NULL;

    /* Fake new initial context */
    fifos_threads[new_tcb]->sp = (uint32_t) (((uint16_t *) stack) - 22);

    /*Fix the stack*/
    /*EIP*/ *(((uint32_t *) stack) - 0) = fifos_threads[new_tcb]->entry;
    if(FIFOS==1){
        /*FLG*/ *(((uint32_t *) stack) - 1) = 0; // | (1 << 9); // Set IF for preemption
    }
    else{
        /*FLG*/ *(((uint32_t *) stack) - 1) = 0 | (1 << 9); // Set IF for preemption
    }
    /*EAX*/ *(((uint32_t *) stack) - 2) = 0;
    /*ECX*/ *(((uint32_t *) stack) - 3) = 0;
    /*EDX*/ *(((uint32_t *) stack) - 4) = 0;

    /*EBX*/ *(((uint32_t *) stack) - 5) = 0; 
    /*ESP*/ *(((uint32_t *) stack) - 6) = (uint32_t)(((uint32_t *) stack) - 2);
    /*EBP*/ *(((uint32_t *) stack) - 7) = (uint32_t)(((uint32_t *) stack) - 2);
    /*ESI*/ *(((uint32_t *) stack) - 8) = 0;
    /*EDI*/ *(((uint32_t *) stack) - 9) = 0;

    /*DS*/ *(((uint16_t *) stack) - 19) = ds;
    /*ES*/ *(((uint16_t *) stack) - 20) = es;
    /*FS*/ *(((uint16_t *) stack) - 21) = fs;
    /*GS*/ *(((uint16_t *) stack) - 22) = gs;

    return new_tcb+1;
}




void kmain (multiboot_info_t* binfo) {
    // puts("start kmain\n");
    uint8_t gdt_entries[3][8] __attribute__((aligned(8)));

    encodeGdtEntry(gdt_entries[0], null_descriptor);
    encodeGdtEntry(gdt_entries[1], kernel_code);
    encodeGdtEntry(gdt_entries[2], kernel_data);

    gdt_t gdt;
    gdt.limit = (sizeof(gdt_entries)) - 1;
    gdt.base = (uint32_t)&gdt_entries;

    // puts("setting gdt\n");
   
    setGdt(gdt.limit, gdt.base);
    if(FIFOS == 2){
        // puts("gdt set\n");
        PIC_remap(0x20, 0x28);
        // puts("remapped\n");
        init_pit();
        // puts("pit initialized\n");
        set_IDT_C();
    }

    // puts("gdt set --> reloading segment registers\n");
    /* check if other seg registers are also changed by long jump */
    __asm__ volatile ("ljmp $0x08, $.reload_CS \n"
                      ".reload_CS: \n"
                      "mov $0x10, %%ax \n"
                      "mov %%ax, %%ds \n"
                      "mov %%ax, %%es \n"
                      "mov %%ax, %%fs \n"
                      "mov %%ax, %%gs \n"
                      "mov %%ax, %%ss \n"
                      :
                      :
                      : "ax");

    // puts("segment registers reloaded --> printing memory map\n");


    for(int i = 0; i < MAX_THREADS; i++){
        done[i] = FALSE;
        fifos_threads[i] = (tcb *)0x500000 + (i * 0x1000);
        stacks[i] = (uint32_t *)0x400000 + (i * 0x1001);
        f[i] = (uint32_t *)thread;
        thread_create(stacks[i], f[i]);
    }
    // put the address of thread 1 in a function pointer, and this time pass a parameter, 1, to thread 1


    build_runqueue();
    current = fifos_threads[0];
    dispatch_first_run();
}