#include "helpers.h"


void exit_thread(void){
    /* Change the state of the current thread to exited */
}


int get_tcb(){
    /*Return index for a tcb if the state is idle (0)*/ 
    for(int i = 0; i < MAX_THREADS; i++){
        if(micros_threads[i]->state == 0){
            return i;
        }
    }
    return -1;
}


/* PCB: Process Control Block */
int thread_create(void *stack, void *func){
    // puts("creating thread!\n");
    int new_pcb = -1;
    uint16_t ds = 0x10, es = 0x10, fs = 0x10, gs = 0x10;

    new_pcb = get_tcb();

    /* Checking if there was an issue creating the thread...*/
    if(new_pcb == -1){
        puts("No PCB available!\n");
        return -1;
    }

    *(((uint32_t *) stack) - 0) = (uint32_t) exit_thread;
    stack = (void *)(((uint32_t *) stack) - 1);

    /* Found new PCB*/
    micros_threads[new_pcb]->tid = new_pcb;
    micros_threads[new_pcb]->bp = (uint32_t) stack;
    micros_threads[new_pcb]->entry = (uint32_t) func;
    micros_threads[new_pcb]->flag = 0;
    micros_threads[new_pcb]->next = NULL;
    micros_threads[new_pcb]->left_child = NULL;
    micros_threads[new_pcb]->right_child = NULL;
    micros_threads[new_pcb]->parent = NULL;
    micros_threads[new_pcb]->state = 1; // ready state

    /* Fake an initial context for the new thread*/
    micros_threads[new_pcb]->sp = (uint32_t) (((uint16_t *) stack) - 22);

    /* Now fix the stack pointer to fake a context switch*/
    /*EIP*/ *(((uint32_t *) stack) - 0) = micros_threads[new_pcb]->entry;
    /*FLG*/ *(((uint32_t *) stack) - 1) = 0 | (1 << 9); // Set IF for preemption
    /*EAX*/ *(((uint32_t *) stack) - 2) = 0;
    /*ECX*/ *(((uint32_t *) stack) - 3) = 0;
    /*EDX*/ *(((uint32_t *) stack) - 4) = 0;

    /*EBX*/ *(((uint32_t *) stack) - 5) = 0; 
    /*ESP*/ *(((uint32_t *) stack) - 6) = (uint32_t)(((uint32_t *) stack) - 2);
    /*EBP*/ *(((uint32_t *) stack) - 7) = (uint32_t)(((uint32_t *) stack) - 2);
    /*ESI*/ *(((uint32_t *) stack) - 8) = 0;
    /*EDI*/ *(((uint32_t *) stack) - 9) = 0;

    /*DS*/ *(((uint16_t *) stack) - 19) = ds; /* 16-bit segment selectors*/
    /*ES*/ *(((uint16_t *) stack) - 20) = es;
    /*FS*/ *(((uint16_t *) stack) - 21) = fs;
    /*GS*/ *(((uint16_t *) stack) - 22) = gs;

    return new_pcb+1;
}
