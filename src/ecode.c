/*
** ecode.h
** Code generator for Elm.
*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "ecode.h"
#include "east.h"

#define R0  0x00
#define R1  0x01
#define R2  0x02
#define R3  0x03
#define R4  0x04
#define R5  0x05
#define R6  0x06
#define R7  0x07

#define elmK_asm_fprintf_seg(fp,s)      fprintf(fp, ".%s\n", s);
#define elmK_asm_fprintf_lbl(fp,l,a)    fprintf(fp, "%s<%#010x>:\n", l, a);
#define elmK_asm_fprintf_op(fp,o)       fprintf(fp, "    %-10s\n", o);
#define elmK_asm_fprintf_opAx(fp,o,a)   fprintf(fp, "    %-10s%#010x\n", o, a);
#define elmK_asm_fprintf_opABx(fp,o,a,b)   fprintf(fp, "    %-10sr%#01x, %#010x\n", o, a, b);

int elmK_code(elmK_func_state_t *fs, Instruction i) {
    fs->code.l++;
    fs->code.p = realloc(fs->code.p, sizeof(Instruction) * fs->code.l);
    ((Instruction *)(fs->code.p))[fs->code.l-1] = i;
    return fs->code.l;
}

int elmK_codeAx(elmK_func_state_t *fs, OpCode o, int a) {
    return elmK_code(fs, CREATE_Ax(o, a));
}

int elmK_codeABx(elmK_func_state_t *fs, OpCode o, int a, int b) {
    return elmK_code(fs, CREATE_ABx(o, a, b));
}

elmK_func_state_t *elmK_func_state(void) {
    elmK_func_state_t *fs = malloc(sizeof(elmK_func_state_t));
    fs->data.l = 0;
    fs->data.p = NULL;
    fs->code.l = 0;
    fs->code.p = NULL;
    fs->symbols = NULL;
    return fs;
}

void elmK_func_state_free(elmK_func_state_t *fs) {
    free(fs);
}

void elmK_sym(elmK_func_state_t *fs, const char *sym, int adr) {
    elmK_sym_t *s;

    s = malloc(sizeof(elmK_sym_t));
    s->sym = malloc(sizeof(char) * strlen(sym));
    strcpy(s->sym, sym);
    s->adr = adr;
    s->next = NULL;

    if(fs->symbols) {
        elmK_sym_t *sx = fs->symbols;
        while(1) {
            if(sx->next == NULL) {
                sx->next = s;
                break;
            } else {
                sx = sx->next;
            }
        }
    } else {
        fs->symbols = s;
    }
}

int elmK_sym_adr(elmK_func_state_t *fs, const char *sym) {
    elmK_sym_t *s = fs->symbols;
    while(s) {
        if(strcmp(s->sym, sym) == 0)
            return s->adr;
        s = s->next;
    }
    return -1;
}

void elmK_generate(elmK_func_state_t *fs, elm_ast_t *a) {
    /*elm_ast_trav_t *trav = elm_ast_traverse_start(a, ELM_AST_TRAV_ORDER_PRE);
    while(1) {
        elm_ast_t *node = elm_ast_traverse_next(&trav);
        if (!node)
            break;*/

        elm_ast_t *node = a;

        switch(node->tag) {
            case ELM_AST_COMMENT:
                break;

            case ELM_AST_BOOL:
                if (node->i)
                    elmK_codeABx(fs, OP_LOADTRUE, R0, 1);
                else
                    elmK_codeABx(fs, OP_LOADFALSE, R0, 0);
                break;

            case ELM_AST_NUMBER:
                elmK_codeABx(fs, OP_LOADI, R0, node->i);
                break;

            case ELM_AST_FLOAT:
                elmK_codeABx(fs, OP_LOADF, R0, (int)node->f);
                break;

            case ELM_AST_CHAR:
                elmK_codeABx(fs, OP_LOADC, R0, (uintptr_t)node->contents);
                break;

            case ELM_AST_STRING:
                elmK_codeABx(fs, OP_LOADPTR, R0, (uintptr_t)node->contents);
                break;

            case ELM_AST_VAR:
                elmK_codeABx(fs, OP_CALL, R0, (uintptr_t)node->contents);
                break;

            case ELM_AST_EXPR:
                assert(node->children_num > 0);
                if(node->children_num == 1) {
                    elmK_generate(fs, node->children[0]);
                } else {
                    assert(node->children[0]->tag == ELM_AST_VAR);
                    for(int i = 1; i < node->children_num; i++) {
                        elmK_generate(fs, node->children[i]);
                    }
                    elmK_codeAx(fs, OP_CALL,
                            elmK_sym_adr(fs, node->children[0]->contents));
                }
                break;

            case ELM_AST_LIST:
                break;

            case ELM_AST_TUPLE:
                break;

            case ELM_AST_IF:
                assert(node->children_num == 3);
                assert(node->children[0]->tag == ELM_AST_EXPR);
                assert(node->children[1]->tag == ELM_AST_EXPR);
                assert(node->children[2]->tag == ELM_AST_EXPR);
                elmK_generate(fs, node->children[0]);
                elmK_code(fs, OP_CMP);
                int pc = fs->code.l;
                elmK_codeAx(fs, OP_BNE, 0);
                elmK_generate(fs, node->children[1]);
                SETARG_Ax(((Instruction *)(fs->code.p))[pc], fs->code.l);
                elmK_generate(fs, node->children[2]);
                break;

            case ELM_AST_CASE:
                break;

            case ELM_AST_BRANCH:
                break;

            case ELM_AST_WILDCARD:
                break;

            case ELM_AST_DECL:
                assert(node->children[0]->tag == ELM_AST_FUNC);
                assert(node->children[1]->tag == ELM_AST_EXPR);
                elmK_sym(fs, node->children[0]->contents, fs->code.l);
                elmK_generate(fs, node->children[1]);
                elmK_codeAx(fs, OP_RETURN, 0);
                break;

            case ELM_AST_FUNC:
                break;

            case ELM_AST_MODULE:
                for(int i = 0; i < node->children_num; i++) {
                    elmK_generate(fs, node->children[i]);
                }
                break;
        }
    /*}
    elm_ast_traverse_free(&trav);*/
}

void elmK_print_asm(elmK_func_state_t *fs) {
   elmK_print_asm_to(fs, stdout); 
}

void elmK_print_asm_to(elmK_func_state_t *fs, FILE *fp) {

    elmK_asm_fprintf_seg(fp, "symbols");

    elmK_sym_t *s = fs->symbols;
    while(s) {
        elmK_asm_fprintf_lbl(fp, s->sym, s->adr);
        s = s->next;
    }

    elmK_asm_fprintf_seg(fp, "data");

    for (int x = 0; x < fs->data.l; x++) {
        fprintf(fp, "%#010x\n", ((int *)(fs->data.p))[x]);
    }

    elmK_asm_fprintf_seg(fp, "code");

    for (int ip = 0; ip < fs->code.l; ip++) {

        Instruction i = ((Instruction *)(fs->code.p))[ip];
        OpCode o = GET_OPCODE(i);
        int Ax = GETARG_Ax(i);
        int A = GETARG_A(i);
        int Bx = GETARG_Bx(i);

        switch(o) {
            case OP_NOP:
                elmK_asm_fprintf_op(fp, "nop");
                break;
            case OP_LOADTRUE:
                elmK_asm_fprintf_opAx(fp, "loadtrue", Ax);
                break;
            case OP_LOADFALSE:
                elmK_asm_fprintf_opAx(fp, "loadfalse", Ax);
                break;
            case OP_LOADI:
                elmK_asm_fprintf_opABx(fp, "loadi", A, Bx);
                break;
            case OP_LOADF:
                elmK_asm_fprintf_opABx(fp, "loadf", A, Bx);
                break;
            case OP_LOADC:
                elmK_asm_fprintf_opABx(fp, "loadc", A, Bx);
                break;
            case OP_LOADPTR:
                elmK_asm_fprintf_opABx(fp, "loadptr", A, Bx);
                break;
            case OP_CALL:
                elmK_asm_fprintf_opAx(fp, "call", Ax);
                break;
            case OP_CMP:
                elmK_asm_fprintf_op(fp, "cmp");
                break;
            case OP_BNE:
                elmK_asm_fprintf_opAx(fp, "bne", Ax);
                break;
            case OP_RETURN:
                elmK_asm_fprintf_op(fp, "return");
                break;
        }
    }
}

