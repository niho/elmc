/*
 * east.c
 * Abstract Syntax Tree
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "east.h"


static void elm_ast_delete_no_children(elm_ast_t *a);


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

elm_ast_val_t *elm_ast_literal(elm_ast_val_t *x) {
    return elm_ast_expr(1, &x);
}

elm_ast_val_t *elm_ast_variable(elm_ast_val_t *x) {
    return elm_ast_new(ELM_AST_VAR, x);
}

elm_ast_val_t *elm_ast_expr(int i, elm_ast_val_t **xs) {
    elm_ast_t *node = elm_ast_new(ELM_AST_EXPR, NULL);
    for (int x = 0; x < i; x++) {
        elm_ast_add_child(node, xs[x]);
    }
    return node;
}

elm_ast_val_t *elm_ast_list(int i, elm_ast_val_t **xs) {
    elm_ast_t *node = elm_ast_new(ELM_AST_LIST, NULL);
    for (int x = 0; x < i; x++) {
        elm_ast_add_child(node, xs[x]);
    }
    return node;
}

elm_ast_val_t *elm_ast_list_cons(int i, elm_ast_val_t **xs) {
    elm_ast_t *node = elm_ast_new(ELM_AST_LIST, NULL);
    for (int x = 0; x < i; x++) {
        elm_ast_t *nx = (elm_ast_t *)xs[x];
        if (nx->tag == ELM_AST_LIST) {
            for (int y = 0; y < nx->children_num; y++) {
                elm_ast_add_child(node, nx->children[y]);
            }
            elm_ast_delete_no_children(nx);
        } else {
            elm_ast_add_child(node, xs[x]);
        }
    }
    return node;
}

elm_ast_val_t *elm_ast_tuple(int i, elm_ast_val_t **xs) {
    elm_ast_t *node = elm_ast_new(ELM_AST_TUPLE, NULL);
    for (int x = 0; x < i; x++) {
        elm_ast_add_child(node, xs[x]);
    }
    return node;
}

elm_ast_val_t *elm_ast_tuple_cons(int i, elm_ast_val_t **xs) {
    elm_ast_t *node = elm_ast_new(ELM_AST_TUPLE, NULL);
    for (int x = 0; x < i; x++) {
        elm_ast_t *nx = (elm_ast_t *)xs[x];
        if (nx->tag == ELM_AST_TUPLE) {
            for (int y = 0; y < nx->children_num; y++) {
                elm_ast_add_child(node, nx->children[y]);
            }
            elm_ast_delete_no_children(nx);
        } else {
            elm_ast_add_child(node, xs[x]);
        }
    }
    return node;
}

elm_ast_val_t *elm_ast_if(int i, elm_ast_val_t **xs) {
    assert(i == 6);
    elm_ast_t *node = elm_ast_new(ELM_AST_IF, NULL);
    elm_ast_add_child(node, xs[1]);
    elm_ast_add_child(node, xs[3]);
    elm_ast_add_child(node, xs[5]);
    free(xs[0]);
    free(xs[2]);
    free(xs[4]);
    return node;
}

elm_ast_val_t *elm_ast_case(int i, elm_ast_val_t **xs) {
    assert(i == 4);
    //assert(((elm_ast_t *)(xs[1]))->tag == ELM_AST_EXPR);
    assert(((elm_ast_t *)(xs[3]))->tag == ELM_AST_CASE);
    
    elm_ast_t *node = elm_ast_new(ELM_AST_CASE, NULL);
    elm_ast_add_child(node, xs[1]);
    
    elm_ast_t *branches = (elm_ast_t *)xs[3];
    for (int x = 0; x < branches->children_num; x++) {
        elm_ast_add_child(node, branches->children[x]);
    } 
    
    free(xs[0]);
    free(xs[2]);
    elm_ast_delete_no_children(xs[3]);
    
    return node;
}

elm_ast_val_t *elm_ast_branches(int i, elm_ast_val_t **xs) {
    elm_ast_t *node = elm_ast_new(ELM_AST_CASE, NULL);
    for (int x = 0; x < i; x++) {
        elm_ast_add_child(node, xs[x]);
    }
    return node;
}

elm_ast_val_t *elm_ast_branch(int i, elm_ast_val_t **xs) {
    assert(i == 3);
    assert(((elm_ast_t *)(xs[i - 1]))->tag == ELM_AST_EXPR);

    elm_ast_t *node = elm_ast_new(ELM_AST_BRANCH, NULL);
    elm_ast_add_child(node, xs[0]);
    elm_ast_add_child(node, xs[2]);

    free(xs[1]);
    
    return node;
}

elm_ast_val_t *elm_ast_wildcard(elm_ast_val_t *x) {
    elm_ast_t *node = elm_ast_new(ELM_AST_WILDCARD, NULL);
    free(x);
    return node;
}

elm_ast_val_t *elm_ast_decl(int i, elm_ast_val_t **xs) {
    assert(i == 3);
    assert(((elm_ast_t *)(xs[0]))->tag == ELM_AST_FUNC);
    assert(((elm_ast_t *)(xs[2]))->tag == ELM_AST_EXPR);

    elm_ast_t *node = elm_ast_new(ELM_AST_DECL, NULL);
    elm_ast_add_child(node, xs[0]);
    elm_ast_add_child(node, xs[2]);

    free(xs[1]);

    return node;
}

elm_ast_val_t *elm_ast_func(int i, elm_ast_val_t **xs) {
    elm_ast_t *node = elm_ast_new(ELM_AST_FUNC, NULL);

    assert(((elm_ast_t *)(xs[0]))->tag == ELM_AST_VAR);
    assert(((elm_ast_t *)(xs[0]))->contents != NULL);

    node->contents = malloc(sizeof(char) * strlen(((elm_ast_t *)(xs[0]))->contents));
    strcpy(node->contents, ((elm_ast_t *)(xs[0]))->contents);
    elm_ast_delete(xs[0]);

    if (i > 1) {
        for (int x = 1; x < i; x++) {
            elm_ast_add_child(node, xs[x]);
        }
    }

    return node;
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
            if (a->i)
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
        case ELM_AST_VAR:
            fprintf(fp, "variable: <%s>\n", (char*)a->contents);
            break;
        case ELM_AST_EXPR:
            fprintf(fp, "expr:\n");
            for(int i = 0; i < a->children_num; i++) {
                elm_ast_print_depth(a->children[i], d+1, fp);
            }
            break;
        case ELM_AST_LIST:
            fprintf(fp, "list:\n");
            for(int i = 0; i < a->children_num; i++) {
                elm_ast_print_depth(a->children[i], d+1, fp);
            }
            break;
        case ELM_AST_TUPLE:
            fprintf(fp, "tuple:\n");
            for(int i = 0; i < a->children_num; i++) {
                elm_ast_print_depth(a->children[i], d+1, fp);
            }
            break;
        case ELM_AST_IF:
            fprintf(fp, "if:\n");
            for(int i = 0; i < a->children_num; i++) {
                elm_ast_print_depth(a->children[i], d+1, fp);
            }
            break;
        case ELM_AST_CASE:
            fprintf(fp, "case:\n");
            for(int i = 0; i < a->children_num; i++) {
                elm_ast_print_depth(a->children[i], d+1, fp);
            }
            break;
        case ELM_AST_BRANCH:
            fprintf(fp, "branch:\n");
            for(int i = 0; i < a->children_num; i++) {
                elm_ast_print_depth(a->children[i], d+1, fp);
            }
            break;
        case ELM_AST_WILDCARD:
            fprintf(fp, "wildcard\n");
            break;
        case ELM_AST_DECL:
            fprintf(fp, "declaration:\n");
            for(int i = 0; i < a->children_num; i++) {
                elm_ast_print_depth(a->children[i], d+1, fp);
            }
            break;
        case ELM_AST_FUNC:
            fprintf(fp, "function: <%s>\n", a->contents);
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
            fprintf(fp, "unknown ast node (tag: %d)\n", a->tag);
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

static void elm_ast_delete_no_children(elm_ast_t *a) {
    free(a->children);
    free(a->contents);
    free(a);
}

elm_ast_trav_t *elm_ast_traverse_start(elm_ast_t *ast, elm_ast_trav_order_t order) {
    elm_ast_trav_t *trav;
    trav = malloc(sizeof(elm_ast_trav_t));
    trav->curr_node = ast;
    trav->parent = NULL;
    trav->curr_child = 0;
    trav->order = order;
    return trav;
}

elm_ast_t *elm_ast_traverse_next(elm_ast_trav_t **trav) {
    elm_ast_trav_t *n_trav, *to_free;
    elm_ast_t *ret = NULL;
    int cchild;

    if(*trav == NULL) return NULL;

    switch((*trav)->order) {
        case ELM_AST_TRAV_ORDER_PRE:
            ret = (*trav)->curr_node;
            
            while(*trav != NULL &&
                    (*trav)->curr_child >= (*trav)->curr_node->children_num)
            {
                to_free = *trav;
                *trav = (*trav)->parent;
                free(to_free);
            }
            
            if(*trav == NULL) {
                break;
            }

            n_trav = malloc(sizeof(elm_ast_trav_t));

            cchild = (*trav)->curr_child;
            n_trav->curr_node = (*trav)->curr_node->children[cchild];
            n_trav->parent = *trav;
            n_trav->curr_child = 0;
            n_trav->order = (*trav)->order;

            (*trav)->curr_child++;
            *trav = n_trav;

            break;
        case ELM_AST_TRAV_ORDER_POST:
            break;
    }

    return ret;
}

void elm_ast_traverse_free(elm_ast_trav_t **trav) {
    elm_ast_trav_t *n_trav;

    while(*trav != NULL) {
        n_trav = (*trav)->parent;
        free(*trav);
        *trav = n_trav;
    }
}

