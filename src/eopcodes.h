/*
** eopcodes.h
** Opcodes for Elm virtual machine.
*/

#ifndef EOPCODES_H
#define EOPCODES_H

#include <stdint.h>

/*==========================================================================
 * Instructions are 32-bit wide unsigned integers.
 * The first 7 bits contain the opcode.
 *
 * Instructions can have the following formats:
 *
 *        4 4 4 4 4 4 4 4 3 3 3 3 3 3 3 3 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1
 * Ax    |                        Ax(25)                   |    Op(7)    |
 * AB    |              Bx(17)             |      A(8)     |    Op(7)    |
 *
 =========================================================================*/

typedef uint32_t Instruction;

#define POS_OP      0
#define SIZE_OP     7

#define POS_A       7
#define SIZE_A      8

#define POS_Bx      15
#define SIZE_Bx     17

#define POS_Ax      7
#define SIZE_Ax     25

#define MASK1(n,p)  ((~((~(Instruction)0)<<(n)))<<(p))
#define MASK0(n,p)  (~MASK1(n,p))

#define GET_OPCODE(i)   ((OpCode)(((i)>>POS_OP) & 0x7F))
#define SET_OPCODE(i,o) ((i) = ((i)&MASK0(SIZE_OP,POS_OP)) | \
        ((((Instruction)o)<<POS_OP)&MASK1(SIZE_OP,POS_OP)))

#define getarg(i,p,s)       ((int)(((i)>>(p)) & MASK1(s,0)))
#define setarg(i,v,p,s)     ((i) = (((i)&MASK0(s,p)) | \
            ((((Instruction)(v))<<p)&MASK1(s,p))))

#define GETARG_A(i)         getarg(i, POS_A, SIZE_A)
#define SETARG_A(i,v)       setarg(i, v, POS_A, SIZE_A)

#define GETARG_Ax(i)        getarg(i, POS_Ax, SIZE_Ax)
#define SETARG_Ax(i,v)      setarg(i, v, POS_Ax, SIZE_Ax)

#define GETARG_Bx(i)        getarg(i, POS_Bx, SIZE_Bx)
#define SETARG_Bx(i,v)      setarg(i, v, POS_Bx, SIZE_Bx)

#define CREATE_Ax(o,a)      (((o)<<POS_OP) | ((a)<<POS_Ax))
#define CREATE_ABx(o,a,b)   (((o)<<POS_OP) | ((a)<<POS_A) | ((b)<<POS_Bx))

typedef enum {
    OP_NOP,
    OP_LOADTRUE,
    OP_LOADFALSE,
    OP_LOADI,
    OP_LOADF,
    OP_LOADC,
    OP_LOADPTR,
    OP_CALL,
    OP_CMP,
    OP_BNE,
    OP_RETURN
} OpCode;

#endif

