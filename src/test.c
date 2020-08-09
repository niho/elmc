/*
** Test suite for the elmc compiler.
** Copyright (C) 2020 Niklas Holmgren. See Copyright Notice in elm.h
*/

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "estack.h"


static void stack_push_pop_test() {
    elmS_stack_t *S = elmS_stack_new();
    assert(S != NULL);

    const char *hello = "hello";
    const char *world = "world";
    elmS_stack_push(S, &world);
    elmS_stack_push(S, &hello);

    assert(S->size == 2);

    assert(strcmp(elmS_stack_pop(S), "hello"));
    assert(strcmp(elmS_stack_pop(S), "world"));
    assert(elmS_stack_pop(S) == NULL);

    assert(S->size == 0);
}

int main() {
    printf("Performing tests:\n");
    stack_push_pop_test(); puts(".");
    printf("All tests completed.\n");
}

