/*
** Elm standalone compiler.
** Copyright (C) 2020 Niklas Holmgren. See Copyright Notice in elm.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ezio.h"
#include "east.h"
#include "eparse.h"

#ifndef ELM_PROGNAME
#define ELM_PROGNAME    "elmc"
#endif

#ifndef ELM_VERSION
#define ELM_VERSION     "0.19.0"
#endif

#ifndef ELMC_VERSION
#define ELMC_VERSION    "1.0.0"
#endif

static const char *progname = ELM_PROGNAME;


static void print_usage(void) {
    printf("usage: %s [options] [filenames]\n"
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

static int handle_script(char **argv) {
    const char *fname = argv[0];

    elm_result_t r;

    if (strcmp(argv[0], "-") == 0) {
        if (elm_parse_pipe("<stdin>", stdin, &r)) {
            elm_ast_print(r.output);
            elm_ast_delete(r.output);
            return 0;
        } else {
            elm_err_print(r.error);
            elm_err_delete(r.error);
            return 1;
        }
    } else {
        if (elm_parse_contents(fname, &r)) {
            elm_ast_print(r.output);
            elm_ast_delete(r.output);
            return 0;
        } else {
            elm_err_print(r.error);
            elm_err_delete(r.error);
            return 1;
        }
    }
}

int main (int argc, char **argv) {
    if (argc <= 1) {
        print_usage();
    } else if (strcmp(argv[1], "-v") == 0) {
        print_version();
    } else {
        return handle_script(&argv[1]);
    } 
    return 0;
}

