// vim:set ts=4 sw=4 ai et:

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

void *xalloc(int size)
{
    void *p = malloc(size);
    if (!p) {
        fprintf(stderr, "malloc() of %d failed\n", size);
        abort();
    }
    memset(p, 0, size);
    return p;
}
