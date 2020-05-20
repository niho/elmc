/*
** ecode.h
** Code generator for Elm.
*/

#ifndef ECODE_H
#define ECODE_H

#include <stdio.h>
#include "eopcodes.h"

struct elm_ast_t;
struct elmK_sym_t;

typedef struct elmK_segment_t {
    int l;
    void *p;
} elmK_segment_t;

typedef struct elmK_sym_t {
    char *sym;
    int adr;
    struct elmK_sym_t *next;
} elmK_sym_t;

typedef struct elmK_func_state_t {
    elmK_segment_t data;
    elmK_segment_t code;
    elmK_sym_t *symbols;
} elmK_func_state_t;

typedef struct elmK_module_state_t {
    elmK_sym_t *symbols;
    elmK_func_state_t *functions;
} elmK_module_state_t;

elmK_func_state_t *elmK_func_state(void);
void elmK_func_state_free(elmK_func_state_t *fs);

void elmK_generate(elmK_func_state_t *fs, struct elm_ast_t *a);

int elmK_code(elmK_func_state_t *fs, Instruction i);
int elmK_codeA(elmK_func_state_t *fs, OpCode o, int a);
int elmK_codeAB(elmK_func_state_t *fs, OpCode o, int a, int b);

void elmK_print_asm(elmK_func_state_t *fs);
void elmK_print_asm_to(elmK_func_state_t *fs, FILE *fp);

#endif

