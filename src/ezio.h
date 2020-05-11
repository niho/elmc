#ifndef ezio_h
#define ezio_h

#include <stddef.h>

#define EOZ          (-1)
#define zgetc(z)    (((z)->n--)>0 ? (*(z)->p++) : EOZ /*elmZ_fill(z))*/

typedef struct InputStream {
    size_t n;           /* bytes unread */
    const char *p;      /* current position in buffer */
    void *data;
} InputStream;

void elmZ_init(InputStream *z, void *data, size_t n);
void elmZ_read(InputStream *z, void *b, size_t n);

int elmZ_fill(InputStream *z);

#endif
