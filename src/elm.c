/*
** Elm standalone interpreter.
** Copyright (C) 2020 Niklas Holmgren. See Copyright Notice in elm.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ezio.h"
#include "east.h"
#include "eparse.h"
#include "ecode.h"
#include "evm.h"

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

            elmK_func_state_t *fs = elmK_func_state();
            elmK_generate(fs, r.output);
            elmK_print_asm(fs);

            elm_ast_delete(r.output);

            elm_state_t *E = elm_newstate();
            elm_execute(E, fs);
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


