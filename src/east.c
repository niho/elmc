/*
 * east.c
 * Abstract Syntax Tree
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "east.h"


elm_ast_t *elm_ast_new(int tag, elm_ast_val_t *contents) {
    elm_ast_t *node = malloc(sizeof(elm_ast_t));
    node->tag = tag;
    node->contents = contents;
    node->children_num = 0;
    node->children = NULL;
    return node;
}

elm_ast_t *elm_ast_add_child(elm_ast_t *r, elm_ast_t *a) {
    r->children_num++;
    r->children = realloc(r->children, sizeof(elm_ast_t*) * r->children_num);
    r->children[r->children_num-1] = a;
    return r;
}

elm_ast_val_t *elm_ast_comment(elm_ast_val_t *x) {
    return elm_ast_new(ELM_AST_COMMENT, x); 
}

elm_ast_val_t *elm_ast_bool(elm_ast_val_t *x) {
    elm_ast_val_t *value = strcmp((char*)x, "True") == 0 ? x : NULL;
    return elm_ast_new(ELM_AST_BOOL, value);
}

elm_ast_val_t *elm_ast_number(elm_ast_val_t *x) {
    return elm_ast_new(ELM_AST_NUMBER, x);
}

elm_ast_val_t *elm_ast_float(elm_ast_val_t *x) {
    return elm_ast_new(ELM_AST_FLOAT, x);
}

elm_ast_val_t *elm_ast_char(elm_ast_val_t *x) {
    return elm_ast_new(ELM_AST_CHAR, x);
}

elm_ast_val_t *elm_ast_string(elm_ast_val_t *x) {
    return elm_ast_new(ELM_AST_STRING, x);
}

elm_ast_val_t *elm_ast_expr(int i, elm_ast_val_t **xs) {
    elm_ast_t *node = elm_ast_new(ELM_AST_EXPR, NULL);
    for (int x = 0; x < i; x++) {
        elm_ast_add_child(node, xs[x]);
    }
    return node;
}

elm_ast_val_t *elm_ast_literal(elm_ast_val_t *x) {
    return elm_ast_expr(1, &x);
}

elm_ast_val_t *elm_ast_module(int i, elm_ast_val_t **xs) {
    elm_ast_t *node = elm_ast_new(ELM_AST_MODULE, NULL);
    for (int x = 0; x < i; x++) {
        elm_ast_add_child(node, xs[x]);
    }
    return node;
}

void elm_ast_print_depth(elm_ast_t *a, int d, FILE *fp) {
    if (a == NULL) {
        fprintf(fp, "NULL\n");
        return;
    }

    for (int i = 0; i < d; i++) { fprintf(fp, "  "); }

    switch(a->tag) {
        case ELM_AST_COMMENT:
            fprintf(fp, "comment: <%s>\n", (char*)a->contents);
            break;
        case ELM_AST_BOOL:
            if (a->value)
                fprintf(fp, "bool: <True>\n");
            else
                fprintf(fp, "bool: <False>\n");
            break;
        case ELM_AST_NUMBER:
            fprintf(fp, "number: <%d>\n", *(int*)a->contents);
            break;
        case ELM_AST_FLOAT:
            fprintf(fp, "float: <%f>\n", *(float*)a->contents);
            break;
        case ELM_AST_CHAR:
            fprintf(fp, "char: <%s>\n", (char*)a->contents);
            break;
        case ELM_AST_STRING:
            fprintf(fp, "string: <%s>\n", (char*)a->contents);
            break;
        case ELM_AST_IDENT:
            fprintf(fp, "ident: <%s>\n", (char*)a->contents);
            break;
        case ELM_AST_EXPR:
            fprintf(fp, "expr:\n");
            for(int i = 0; i < a->children_num; i++) {
                elm_ast_print_depth(a->children[i], d+1, fp);
            }
            break;
        case ELM_AST_MODULE:
            fprintf(fp, "module:\n");
            for(int i = 0; i < a->children_num; i++) {
                elm_ast_print_depth(a->children[i], d+1, fp);
            }
            break;
        default:
            fprintf(fp, "unknown ast node (tag: %d)", a->tag);
            break;
    }
}

void elm_ast_print_to(elm_ast_t *a, FILE *fp) {
    elm_ast_print_depth(a, 0, fp);
}

void elm_ast_print(elm_ast_t *a) {
    elm_ast_print_to(a, stdout);
}

void elm_ast_delete(elm_ast_t *a) {
    if (a == NULL) { return; }

    for (int i = 0; i < a->children_num; i++) {
        elm_ast_delete(a->children[i]);
    }

    free(a->children);
    free(a->contents);
    free(a);
}

