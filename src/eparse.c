/*
 * eparse.c
 * Elm Parser
 */

#include "eparse.h"
#include "east.h"
#include "mpc.h"

typedef struct elm_err_t {
    mpc_err_t *error;
} elm_err_t;

typedef struct elm_parser_t {
    //mpc_parser_t *Whitespace = mpc_new("whitespace");
    mpc_parser_t *Comment;
    mpc_parser_t *True;
    mpc_parser_t *False;
    mpc_parser_t *Number;
    mpc_parser_t *Float;
    mpc_parser_t *Char;
    mpc_parser_t *String;
    //mpc_parser_t *MultiStr = mpc_new("multistr");
    mpc_parser_t *Literal;
    mpc_parser_t *Decl;
    /*mpc_parser_t *ValueDecl = mpc_new("valuedecl");
    mpc_parser_t *TypeDecl  = mpc_new("typedecl");
    mpc_parser_t *Type   = mpc_new("type");
    mpc_parser_t *CustomType = mpc_new("customtype");
    mpc_parser_t *TypeAlias = mpc_new("typealias");*/
    mpc_parser_t *Var;
    mpc_parser_t *Value;
    mpc_parser_t *Expr;
    mpc_parser_t *List;
    mpc_parser_t *Tuple;
    mpc_parser_t *If;
    mpc_parser_t *Case;
    /*mpc_parser_t *Lexpr  = mpc_new("lexpr");
    mpc_parser_t *Prod   = mpc_new("product");
    mpc_parser_t *Value  = mpc_new("value");*/
    mpc_parser_t *Module;

} elm_parser_t;

static elm_err_t *elm_error(mpc_err_t *e) {
    elm_err_t *error = malloc(sizeof(elm_err_t));
    error->error = e;
    return error;
}

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

static int is_unreserved(mpc_val_t **xs) {
    if (strcmp(xs[0], "if") == 0)
        return 0;
    if (strcmp(xs[0], "then") == 0)
        return 0;
    if (strcmp(xs[0], "else") == 0)
        return 0;
    if (strcmp(xs[0], "case") == 0)
        return 0;
    if (strcmp(xs[0], "of") == 0)
        return 0;
    if (strcmp(xs[0], "->") == 0)
        return 0;
    if (strcmp(xs[0], "_") == 0)
        return 0;
    if (strcmp(xs[0], "=") == 0)
        return 0;
    return 1;
}

static mpc_parser_t *elm_variable(void) {
    return mpc_expect(mpc_apply(mpc_check(mpc_re("[a-z_][a-zA-Z_0-9]*"), free, is_unreserved, "keyword-variable-collision"), elm_ast_variable), "variable");
}

static mpc_parser_t *elm_value(mpc_parser_t *expr, mpc_parser_t *literal, mpc_parser_t *variable, mpc_parser_t *ifthen, mpc_parser_t *caseof) {
    return mpc_expect(mpc_or(5, ifthen, caseof, literal, variable, mpc_tok_parens(expr, free)), "value");
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
            mpc_sym("if"),
                mpc_tok(expr),
            mpc_sym("then"),
                mpc_tok(expr),
            mpc_sym("else"),
                mpc_tok(expr),
            free, free, free, free, free);
}

static mpc_parser_t *elm_case_branch(mpc_parser_t *pattern, mpc_parser_t *expr) {
    return mpc_and(3, elm_ast_branch, mpc_tok(pattern), mpc_sym("->"), mpc_tok_parens(expr, free), free, free);
}

static mpc_parser_t *elm_case(mpc_parser_t *expr, mpc_parser_t *pattern) {
    return mpc_and(4, elm_ast_case,
            mpc_sym("case"),
                mpc_tok(expr),
            mpc_sym("of"),
                mpc_many1(elm_ast_branches, mpc_tok(elm_case_branch(pattern, expr))),
            free, free, free);
}

static mpc_parser_t *elm_wildcard(void) {
    return mpc_expect(mpc_apply(mpc_char('_'), elm_ast_wildcard), "wildcard");
}

static mpc_parser_t *elm_pattern(mpc_parser_t *literal) {
    return mpc_or(2, literal, elm_wildcard());
}

static mpc_parser_t *elm_declaration(mpc_parser_t *var, mpc_parser_t *expr) {
    return mpc_and(3, elm_ast_decl, mpc_many1(elm_ast_func, mpc_tok(var)), mpc_sym("="), mpc_tok_parens(expr, free), free, free);
}

static mpc_parser_t *elm_module(mpc_parser_t *comment, mpc_parser_t *decl) {
    return mpc_whole(
            mpc_many1(elm_ast_module,
                mpc_or(2,
                    mpc_tok(comment),
                    mpc_tok(decl)
                )
            ), free);
}

static elm_parser_t *elm_parser_init(void) {
    elm_parser_t *p = malloc(sizeof(elm_parser_t));

    //mpc_parser_t *Whitespace = mpc_new("whitespace");
    p->Comment  = mpc_new("comment");
    p->True     = mpc_new("true");
    p->False    = mpc_new("false");
    p->Number   = mpc_new("number");
    p->Float    = mpc_new("float");
    p->Char     = mpc_new("char");
    p->String   = mpc_new("string");
    //p->MultiStr = mpc_new("multistr");
    p->Literal  = mpc_new("literal");
    p->Decl     = mpc_new("declaration");
    /*p->ValueDecl = mpc_new("valuedecl");
    p->TypeDecl  = mpc_new("typedecl");
    p->Type   = mpc_new("type");
    p->CustomType = mpc_new("customtype");
    p->TypeAlias = mpc_new("typealias");*/
    p->Var      = mpc_new("variable");
    p->Value    = mpc_new("value");
    p->Expr     = mpc_new("expression");
    p->List     = mpc_new("list");
    p->Tuple    = mpc_new("tuple");
    p->If       = mpc_new("if");
    p->Case     = mpc_new("case");
    /*p->Lexpr  = mpc_new("lexpr");
    p->Prod     = mpc_new("product");
    p->Value    = mpc_new("value");*/
    p->Module   = mpc_new("module");

    mpc_define(p->Comment, mpc_apply(elm_comment(), elm_ast_comment));

    mpc_define(p->True, mpc_apply(mpc_sym("True"), elm_ast_bool));
    mpc_define(p->False, mpc_apply(mpc_sym("False"), elm_ast_bool));

    mpc_define(p->Number, mpc_apply(elm_number(), elm_ast_number));
    mpc_define(p->Float, mpc_apply(elm_float(), elm_ast_float));

    mpc_define(p->Char, mpc_apply(mpc_char_lit(), elm_ast_char));
    mpc_define(p->String, mpc_apply(mpc_string_lit(), elm_ast_string));

    mpc_define(p->Literal, mpc_apply(mpc_or(7, p->True, p->False, mpc_or(2, p->Float, p->Number), p->Char, p->String, p->List, p->Tuple), elm_ast_literal));
    mpc_define(p->Var, elm_variable());

    mpc_define(p->Value, elm_value(p->Expr, p->Literal, p->Var, p->If, p->Case));
    mpc_define(p->Expr, elm_expr(p->Value));

    mpc_define(p->List, elm_list(p->Expr));
    mpc_define(p->Tuple, elm_tuple(p->Expr));

    mpc_define(p->If, elm_if(p->Expr));
    mpc_define(p->Case, elm_case(p->Expr, elm_pattern(mpc_or(6, p->True, p->False, p->Float, p->Number, p->Char, p->String))));

    mpc_define(p->Decl, elm_declaration(p->Var, p->Expr));

    mpc_define(p->Module, elm_module(p->Comment, p->Decl));
    
    return p;
}

static void elm_parser_cleanup(elm_parser_t *p) {
    mpc_cleanup(17,
            p->Comment,
            p->True,
            p->False,
            p->Number,
            p->Float,
            p->Char,
            p->String,
            p->Literal,
            p->Var,
            p->Value,
            p->Expr,
            p->List,
            p->Tuple,
            p->If,
            p->Case,
            p->Decl,
            p->Module);
}

int elm_parse(const char *filename, const char *string, elm_result_t *r) {
    elm_parser_t *p = elm_parser_init();
    mpc_result_t result;
    if (mpc_parse(filename, string, p->Module, &result)) {
        r->output = (elm_ast_t *)result.output;
        elm_parser_cleanup(p);
        return 1;
    } else {
        r->error = elm_error(result.error);  
        elm_parser_cleanup(p);
        return 0;
    }
}

int elm_parse_file(const char *filename, FILE *file, elm_result_t *r) {
    elm_parser_t *p = elm_parser_init();
    mpc_result_t result;
    if (mpc_parse_file(filename, file, p->Module, &result)) {
        r->output = (elm_ast_t *)result.output;
        elm_parser_cleanup(p);
        return 1;
    } else {
        r->error = elm_error(result.error);  
        elm_parser_cleanup(p);
        return 0;
    }
}

int elm_parse_pipe(const char *filename, FILE *pipe, elm_result_t *r) {
    elm_parser_t *p = elm_parser_init();
    mpc_result_t result;
    if (mpc_parse_pipe(filename, pipe, p->Module, &result)) {
        r->output = (elm_ast_t *)result.output;
        elm_parser_cleanup(p);
        return 1;
    } else {
        r->error = elm_error(result.error);  
        elm_parser_cleanup(p);
        return 0;
    }
}

int elm_parse_contents(const char *filename, elm_result_t *r) {
    elm_parser_t *p = elm_parser_init();
    mpc_result_t result;
    if (mpc_parse_contents(filename, p->Module, &result)) {
        r->output = (elm_ast_t *)result.output;
        elm_parser_cleanup(p);
        return 1;
    } else {
        r->error = elm_error(result.error);
        elm_parser_cleanup(p);
        return 0;
    }
}

void elm_err_print(elm_err_t *e) {
    mpc_err_print(e->error);
}

void elm_err_print_to(elm_err_t *e, FILE *fp) {
    mpc_err_print_to(e->error, fp);
}

void elm_err_delete(elm_err_t *e) {
    mpc_err_delete(e->error);
    free(e);
}

