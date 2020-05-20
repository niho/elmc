/*
** evm.c
** Elm Virtual Machine.
*/

#include <stdlib.h>
#include <string.h>

#include "evm.h"
#include "ecode.h"

elm_state_t *elm_newstate(void) {
    elm_state_t *E = malloc(sizeof(elm_state_t));
    E->pc = 0;
    memset(E->R, 0, sizeof(elmV_reg_t) * EVM_REGISTER_COUNT);
    return E;
}

void elm_close(elm_state_t *E) {
    free(E);
}

/*void elm_call(elm_state_t *E, elm_ast_t *a, const char *func) {
     
}*/

void elm_execute(elm_state_t *E, elmK_func_state_t *fs) {
    while(1) {
        Instruction i = ((Instruction *)(fs->code.p))[E->pc];
        OpCode o = GET_OPCODE(i);

        int Ax, A, Bx;

        switch(o) {
            case OP_NOP:
                break;

            case OP_LOADTRUE:
                A = GETARG_A(i);
                Bx = GETARG_Bx(i);
                E->R[A] = Bx;
                break;

            case OP_LOADFALSE:
                A = GETARG_A(i);
                Bx = GETARG_Bx(i);
                E->R[A] = Bx;
                break;

            case OP_LOADI:
                A = GETARG_A(i);
                Bx = GETARG_Bx(i);
                E->R[A] = Bx;
                break;

            case OP_LOADF:
                break;

            case OP_LOADC:
                break;

            case OP_LOADPTR:
                A = GETARG_A(i);
                Bx = GETARG_Bx(i);
                E->R[A] = Bx;
                break;

            case OP_CALL:
                break;

            case OP_CMP:

                break;

            case OP_BNE:
                break;

            case OP_RETURN:
                for(int r = 0; r < 8; r++) {
                    printf("$r%d %#018llx  ", r, E->R[r]);
                }
                printf("\n");
                printf("$pc %#010x\n", E->pc);
                break;
        }

        /* Increment program counter (pc) */
        E->pc++;

        /* If at end of program, break VM loop */
        if(E->pc >= fs->code.l)
            break;
    }
}

