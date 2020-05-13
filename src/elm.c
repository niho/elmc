/*
** Elm standalone interpreter.
** Copyright (C) 2020 Niklas Holmgren. See Copyright Notice in elm.h
*/

#define elm_c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ezio.h"
#include "mpc.h"
#include "east.h"

#ifndef ELM_PROGNAME
#define ELM_PROGNAME    "elmc"
#endif

#ifndef ELM_VERSION
#define ELM_VERSION     "0.19.0"
#endif

#ifndef ELMC_VERSION
#define ELMC_VERSION    "0.0.1"
#endif

static const char *progname = ELM_PROGNAME;


static void print_usage(void) {
    printf("usage: %s [options] [script]\n"
            "Available options are:\n"
            "  -v        show version information\n"
            "  --        stop handling options\n"
            "  -         stop handling options and execute stdin\n",
            progname);
}

static void print_version(void) {
    printf("%s (%s)", ELM_VERSION, ELMC_VERSION);
    printf("\n");
}

/*static void elmL_loadfile(InputStream *stream, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("File not found: %s\n", filename);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    char *buffer = (char *) malloc(sizeof(char) * size);
    if (buffer == NULL) {
        printf("Memory error");
        exit(1);
    }

    size_t result = fread(buffer, 1, size, fp);
    if (result != size) {
        printf("Reading error");
        exit(1);
    }

    fclose(fp);

    elmZ_init(stream, buffer, size);
}*/


/*
 * eparse.h/c
 * Elm Parser
 */

static int is_newline(mpc_val_t **xs) {
    char **vs = (char**)xs;
    if (vs[0][0] == '\n') {
        return 0;
    }
    return 1;
}

static mpc_parser_t *elm_singleline_comment(void) {
    return mpc_expect(
            mpc_and(2, mpcf_snd_free,
                    mpc_string("--"),
                    mpc_stripl(mpc_many(mpcf_strfold,
                        mpc_check(mpc_any(), free, is_newline, "end-of-line"))),
                    free),
            "comment");
}

static mpc_parser_t *elm_multiline_comment(void) {
    /*return mpc_expect(mpc_and(3, mpcf_snd_free,
                mpc_string("{-"),
                mpc_many1(mpcf_strfold, mpc_any()),
                mpc_string("-}"), free, free), "comment");*/
    return mpc_expect(mpc_tok_between(
                mpc_many(mpcf_strfold, mpc_any()), free, "{-", "-}"
            ), "multiline comment");
}

static mpc_parser_t *elm_comment(void) {
    return mpc_expect(mpc_or(2,
            elm_multiline_comment(),
            elm_singleline_comment()), "comment");
}

static mpc_parser_t *elm_hex(void) {
    return mpc_expect(mpc_and(2, mpcf_snd_free, mpc_string("0x"), mpc_hex(), free), "hexadecimal");
}

static mpc_parser_t *elm_number(void) {
    return mpc_expect(mpc_or(2, elm_hex(), mpc_int()), "number");
}

static mpc_parser_t *elm_real(void) {
    /* [+-]?\d+\.\d+([eE][+-]?[0-9]+)? */

    mpc_parser_t *p0, *p1, *p2, *p30, *p31, *p32, *p3;

    p0 = mpc_maybe_lift(mpc_oneof("+-"), mpcf_ctor_str);
    p1 = mpc_digits();
    p2 = mpc_and(2, mpcf_strfold, mpc_char('.'), mpc_digits(), free);
    p30 = mpc_oneof("eE");
    p31 = mpc_maybe_lift(mpc_oneof("+-"), mpcf_ctor_str);
    p32 = mpc_digits();
    p3 = mpc_maybe_lift(mpc_and(3, mpcf_strfold, p30, p31, p32, free, free), mpcf_ctor_str);

    return mpc_expect(mpc_and(4, mpcf_strfold, p0, p1, p2, p3, free, free, free), "real");

}

static mpc_parser_t *elm_float(void) {
    return mpc_expect(mpc_apply(elm_real(), mpcf_float), "float");
}

static int is_keyword(mpc_val_t **xs) {
    if (strcmp(xs[0], "if") == 0)
        return 0;
    if (strcmp(xs[0], "then") == 0)
        return 0;
    if (strcmp(xs[0], "else") == 0)
        return 0;
    return 1;
}

static mpc_parser_t *elm_variable(void) {
    return mpc_check(mpc_re("[a-z_][a-zA-Z_0-9]*"), free, is_keyword, "keyword-variable-collision");
}

static mpc_parser_t *elm_value(mpc_parser_t *expr, mpc_parser_t *literal, mpc_parser_t *variable, mpc_parser_t *ifthen) {
    return mpc_expect(mpc_or(4, ifthen, literal, variable, mpc_tok_parens(expr, free)), "value");
}

static mpc_parser_t *elm_expr(mpc_parser_t *value) {
    return mpc_expect(mpc_many1(elm_ast_expr, mpc_tok(value)), "expression");
}

static mpc_parser_t *elm_list(mpc_parser_t *expr) {
    return mpc_expect(
            mpc_or(2,
                mpc_tok_squares(mpc_many(elm_ast_list, expr), free),
                mpc_tok_squares(mpc_and(2, elm_ast_list_cons, expr, mpc_many(elm_ast_list, mpc_and(2, mpcf_snd_free, mpc_tok(mpc_char(',')), expr, free)), free), free))
            , "list");
}

static mpc_parser_t *elm_tuple(mpc_parser_t *expr) {
    return mpc_expect(
            mpc_or(2,
                mpc_tok_parens(mpc_many(elm_ast_tuple, expr), free),
                mpc_tok_parens(mpc_and(2, elm_ast_tuple_cons, expr, mpc_many(elm_ast_tuple, mpc_and(2, mpcf_snd_free, mpc_tok(mpc_char(',')), expr, free)), free), free))
            , "tuple");
}

static mpc_parser_t *elm_if(mpc_parser_t *expr) {
    return mpc_and(6, elm_ast_if,
            mpc_tok(mpc_string("if")),
                expr,
            mpc_tok(mpc_string("then")),
                expr,
            mpc_tok(mpc_string("else")),
                expr,
            free, free, free, free, free);
}

/*
 * main.c
 */

static int handle_script(char **argv) {
    const char *fname = argv[0];

    mpc_result_t r;
    //mpc_parser_t *Whitespace = mpc_new("whitespace");
    mpc_parser_t *Comment = mpc_new("comment");
    mpc_parser_t *True   = mpc_new("true");
    mpc_parser_t *False  = mpc_new("false");
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Float  = mpc_new("float");
    mpc_parser_t *Char   = mpc_new("char");
    mpc_parser_t *String = mpc_new("string");
    //mpc_parser_t *MultiStr = mpc_new("multistr");
    mpc_parser_t *Literal = mpc_new("literal");
    /*mpc_parser_t *Decl   = mpc_new("declaration");
    mpc_parser_t *ValueDecl = mpc_new("valuedecl");
    mpc_parser_t *TypeDecl  = mpc_new("typedecl");
    mpc_parser_t *Type   = mpc_new("type");
    mpc_parser_t *CustomType = mpc_new("customtype");
    mpc_parser_t *TypeAlias = mpc_new("typealias");*/
    mpc_parser_t *Var    = mpc_new("variable");
    mpc_parser_t *Value  = mpc_new("value");
    mpc_parser_t *Expr   = mpc_new("expression");
    mpc_parser_t *List   = mpc_new("list");
    mpc_parser_t *Tuple  = mpc_new("tuple");
    mpc_parser_t *If     = mpc_new("if");
    /*mpc_parser_t *Lexpr  = mpc_new("lexpr");
    mpc_parser_t *Prod   = mpc_new("product");
    mpc_parser_t *Value  = mpc_new("value");
    mpc_parser_t *Module = mpc_new("module");*/
    mpc_parser_t *Elm    = mpc_new("elm");

    mpc_define(Comment, mpc_apply(mpc_tok(elm_comment()), elm_ast_comment));

    mpc_define(True, mpc_apply(mpc_sym("True"), elm_ast_bool));
    mpc_define(False, mpc_apply(mpc_sym("False"), elm_ast_bool));

    mpc_define(Number, mpc_apply(mpc_tok(elm_number()), elm_ast_number));
    mpc_define(Float, mpc_apply(mpc_tok(elm_float()), elm_ast_float));

    mpc_define(Char, mpc_apply(mpc_tok(mpc_char_lit()), elm_ast_char));
    mpc_define(String, mpc_apply(mpc_tok(mpc_string_lit()), elm_ast_string));

    mpc_define(Literal, mpc_apply(mpc_or(7, True, False, mpc_or(2, Float, Number), Char, String, List, Tuple), elm_ast_literal));
    mpc_define(Var, mpc_apply(mpc_tok(elm_variable()), elm_ast_variable));

    mpc_define(Value, mpc_tok(elm_value(Expr, Literal, Var, If)));
    mpc_define(Expr, mpc_tok(elm_expr(Value)));

    mpc_define(List, mpc_tok(elm_list(Expr)));
    mpc_define(Tuple, mpc_tok(elm_tuple(Expr)));

    mpc_define(If, elm_if(Expr));

    mpc_define(Elm,
        mpc_whole(
            mpc_many1(elm_ast_module,
                mpc_or(2,
                    Comment,
                    Expr
                )
            ), free));
    
    /*mpca_lang_contents(MPCA_LANG_DEFAULT | MPCA_LANG_WHITESPACE_SENSITIVE,
        "./src/elm.grammar"
        , Whitespace, Comment, True, False, Number, Float, Char, String, Literal, Var, Value, Expr, Type, CustomType, TypeAlias, ValueDecl, TypeDecl, Decl, Elm, NULL); */

    if (strcmp(argv[0], "-") == 0) {
        if (mpc_parse_pipe("<stdin>", stdin, Elm, &r)) {
            elm_ast_print(r.output);
            elm_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
    } else {
        if (mpc_parse_contents(fname, Elm, &r)) {
            elm_ast_print(r.output);
            elm_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
    }

    //mpc_cleanup(5, Number, True, False, Comment, Elm);
    //mpc_cleanup(9, Comment, Decl, ValueDecl, TypeDecl, Type, Expr, Prod, Value, Elm);

    return 0;
}

int main (int argc, char **argv) {
    if (argc <= 1) {
        print_usage();
    } else if (strcmp(argv[1], "-v") == 0) {
        print_version();
    } else {
        handle_script(&argv[1]);
    } 
    return 0;
}


