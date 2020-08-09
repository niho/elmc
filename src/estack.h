#ifndef ESTACK_H
#define ESTACK_H

struct elmS_stack_frame_t;

typedef struct elmS_stack_frame_t {
    void *item;
    struct elmS_stack_frame_t *next;
} elmS_stack_frame_t;

typedef struct elmS_stack_t {
    elmS_stack_frame_t *head;
    unsigned int size;
} elmS_stack_t;

elmS_stack_t *elmS_stack_new();
void elmS_stack_delete(elmS_stack_t *stack);

void elmS_stack_push(elmS_stack_t *stack, void *item);
void *elmS_stack_pop(elmS_stack_t *stack);

#endif
