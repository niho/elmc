/*
** evm.h
** Elm Virtual Machine.
*/

#ifndef EVM_H
#define EVM_H

#include <stdint.h>

struct elmK_func_state_t;

typedef uint64_t elmV_reg_t;

#define EVM_REGISTER_COUNT      8

typedef struct elm_state_t {
    int pc;
    elmV_reg_t R[EVM_REGISTER_COUNT];
} elm_state_t;

elm_state_t *elm_newstate(void);
void elm_close(elm_state_t *E);

void elm_execute(elm_state_t *E, struct elmK_func_state_t *fs);

#endif

