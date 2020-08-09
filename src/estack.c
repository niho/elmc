#include "estack.h"
#include <stdlib.h>

elmS_stack_t *elmS_stack_new() {
    elmS_stack_t *stack = malloc(sizeof(elmS_stack_t));
    stack->size = 0;
    stack->head = NULL;
    return stack;
}

void elmS_stack_delete(elmS_stack_t *stack) {
    while(1) {
        if(elmS_stack_pop(stack) == NULL)
            break;
    }
    stack->head = NULL;
    stack->size = 0;
    free(stack);
}

void elmS_stack_push(elmS_stack_t *stack, void *item) {
    elmS_stack_frame_t *frame = malloc(sizeof(elmS_stack_frame_t));
    frame->item = item;
    /* Start of critical section */
    frame->next = stack->head;
    stack->head = frame;
    stack->size++;
    /* End of critical section */
}

void *elmS_stack_pop(elmS_stack_t *stack) {
    if(stack->head == NULL)
        return NULL;

    elmS_stack_frame_t *frame = stack->head;
    /* Start of critical section */
    stack->head = frame->next;
    stack->size--;
    /* End of critical section */

    void *item = frame->item;
    frame->item = NULL;
    frame->next = NULL;
    free(frame);

    return item;
}

