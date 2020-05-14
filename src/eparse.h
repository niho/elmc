/*
 * eparse.h
 * Elm Parser
 */

#ifndef EPARSE_H
#define EPARSE_H

#include <stdio.h>

struct elm_err_t;
struct elm_ast_t;

typedef union {
  struct elm_err_t *error;
  struct elm_ast_t *output;
} elm_result_t;

int elm_parse(const char *filename, const char *string, elm_result_t *r);
int elm_parse_file(const char *filename, FILE *file, elm_result_t *r);
int elm_parse_pipe(const char *filename, FILE *pipe, elm_result_t *r);
int elm_parse_contents(const char *filename, elm_result_t *r);

void elm_err_print(struct elm_err_t *e);
void elm_err_print_to(struct elm_err_t *e, FILE *fp);
void elm_err_delete(struct elm_err_t *e);

#endif
