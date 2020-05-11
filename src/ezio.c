#define ezio_c

#include "ezio.h"

void elmZ_init(InputStream *z, void *data, size_t n) {
    z->n = n;
    z->p = data;
    z->data = data;
}

void elmZ_read(InputStream *z, void *b, size_t n) {
    for(size_t i = 0; i < n; i++) {
        ((char *)b)[i] = z->p[i];
    }
    z->n = z->n + n;
    z->p = z->p + n;
}

int elmZ_fill(InputStream *z) {
    if (z->n-- == 0) {
        return EOZ;
    }
    return (*(z->p++));
}
