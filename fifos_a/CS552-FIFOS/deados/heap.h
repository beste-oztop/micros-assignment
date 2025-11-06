#include "defns.h"
#include "helpers.h"

node *heap_malloc(){
    heap_memory++;
    return heap_memory;
}

void swap(){
    node *current = top_node;
    while(current->right != NULL){
        if(current->t->deadline > current->right->t->deadline){
            tcb *temp = current->t;
            current->t = current->right->t;
            current->right->t = temp;
        }
        current = current->right;
    }
}

// Heap will be based on tcb.deadline
// Earliest deadline will be at the top
void add_node(tcb *thread){
    node* new_node = heap_malloc();
    new_node->t = thread;
    new_node->left = NULL;
    node* temp = top_node;
    top_node = new_node;
    new_node->right = temp;
    swap();
}



tcb* pop(){
    if(top_node == NULL){
        return NULL;
    }
    if(top_node->t->deadline <= current_time){
        tcb* temp = top_node->t;
        top_node = top_node->right;
        temp->deadline = temp->period + temp->deadline;
        puts("miss");
        char buff[16];
        itoa(temp->tid, buff, 10);
        puts(buff);
        puts(" ");
        add_node(temp);
        return pop();
    }
    if(top_node->right == NULL){
        node *current = top_node;
        top_node = NULL;
        // puts("NULL ");
        // char buff[16];
        // itoa(current->t->tid, buff, 10);
        // puts(buff);
        // puts("\n");
        return current->t;
    }
    else{
        node *current = top_node;
        // puts("Pop ");
        // char buff[16];
        // itoa(current->t->tid, buff, 10);
        // puts(buff);
        // puts("\n");
        top_node = top_node->right;
        return current->t;
    }
}

tcb* pop_old(){
    if(top_node->left == NULL && top_node->right == NULL){
        node* current = top_node;
        top_node = NULL;
        return current->t;
    }
    else if(top_node->left == NULL){
        node *current = top_node;
        top_node = top_node->right;
        return current->t;
    }
    else if(top_node->right == NULL){
        node *current = top_node;
        top_node = top_node->left;
        return current->t;
    }
    else if(top_node->left->t->deadline < top_node->right->t->deadline){
        node *current = top_node;
        top_node = top_node->left;
        return current->t;
    }
    else{
        node *current = top_node;
        top_node = top_node->right;
        return current->t;

    }
}


tcb* peek(){
    if(top_node == NULL){
        return NULL;
    }
    return top_node->t;
}



