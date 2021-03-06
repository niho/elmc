/* 
 * east.h
 * Abstract Syntax Tree
 *
 */

#ifndef EAST_H
#define EAST_H

#include <stdio.h>

enum {
    ELM_AST_COMMENT,
    ELM_AST_BOOL,
    ELM_AST_NUMBER,
    ELM_AST_FLOAT,
    ELM_AST_CHAR,
    ELM_AST_STRING,
    ELM_AST_VAR,
    ELM_AST_EXPR,
    ELM_AST_LIST,
    ELM_AST_TUPLE,
    ELM_AST_IF,
    ELM_AST_CASE,
    ELM_AST_BRANCH,
    ELM_AST_WILDCARD,
    ELM_AST_DECL,
    ELM_AST_FUNC,
    ELM_AST_MODULE
};

typedef void elm_ast_val_t;

typedef struct elm_ast_t {
    int tag;
    union {
        elm_ast_val_t *contents;
        int i;
        float f;
    };
    int children_num;
    struct elm_ast_t** children;
} elm_ast_t;

elm_ast_t *elm_ast_new(int tag, elm_ast_val_t *contents);
elm_ast_t *elm_ast_add_child(elm_ast_t *r, elm_ast_t *a);

elm_ast_val_t *elm_ast_comment(elm_ast_val_t *x);
elm_ast_val_t *elm_ast_bool(elm_ast_val_t *x);
elm_ast_val_t *elm_ast_number(elm_ast_val_t *x);
elm_ast_val_t *elm_ast_float(elm_ast_val_t *x);
elm_ast_val_t *elm_ast_char(elm_ast_val_t *x);
elm_ast_val_t *elm_ast_string(elm_ast_val_t *x);
elm_ast_val_t *elm_ast_literal(elm_ast_val_t *x);
elm_ast_val_t *elm_ast_variable(elm_ast_val_t *x);
elm_ast_val_t *elm_ast_expr(int i, elm_ast_val_t **xs);
elm_ast_val_t *elm_ast_list(int i, elm_ast_val_t **xs);
elm_ast_val_t *elm_ast_list_cons(int i, elm_ast_val_t **xs);
elm_ast_val_t *elm_ast_tuple(int i, elm_ast_val_t **xs);
elm_ast_val_t *elm_ast_tuple_cons(int i, elm_ast_val_t **xs);
elm_ast_val_t *elm_ast_if(int i, elm_ast_val_t **xs);
elm_ast_val_t *elm_ast_case(int i, elm_ast_val_t **xs);
elm_ast_val_t *elm_ast_branches(int i, elm_ast_val_t **xs);
elm_ast_val_t *elm_ast_branch(int i, elm_ast_val_t **xs);
elm_ast_val_t *elm_ast_wildcard(elm_ast_val_t *x);
elm_ast_val_t *elm_ast_decl(int i, elm_ast_val_t **xs);
elm_ast_val_t *elm_ast_func(int i, elm_ast_val_t **xs);
elm_ast_val_t *elm_ast_module(int i, elm_ast_val_t **xs);

void elm_ast_print_depth(elm_ast_t *a, int d, FILE *fp);
void elm_ast_print_to(elm_ast_t *a, FILE *fp);
void elm_ast_print(elm_ast_t *a);

void elm_ast_delete(elm_ast_t *a);

typedef enum {
  ELM_AST_TRAV_ORDER_PRE,
  ELM_AST_TRAV_ORDER_POST
} elm_ast_trav_order_t;

typedef struct elm_ast_trav_t {
  elm_ast_t             *curr_node;
  struct elm_ast_trav_t *parent;
  int                    curr_child;
  elm_ast_trav_order_t   order;
} elm_ast_trav_t;

elm_ast_trav_t *elm_ast_traverse_start(elm_ast_t *ast, elm_ast_trav_order_t order);
elm_ast_t *elm_ast_traverse_next(elm_ast_trav_t **trav);
void elm_ast_traverse_free(elm_ast_trav_t **trav);

#endif

