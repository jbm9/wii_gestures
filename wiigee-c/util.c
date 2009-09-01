// vim:set ts=4 sw=4 ai et:

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "util.h"

int debug(const char *format, ...)
{
    va_list ap;
    int out = 0;

    va_start(ap, format);
    if (getenv("DEBUG"))
        out = vfprintf(stderr, format, ap);
    va_end(ap);
    return out;
}

int die(const char *format, ...)
{
    va_list ap;
    int out;

    va_start(ap, format);
    out = vfprintf(stderr, format, ap);
    va_end(ap);
    abort();
}

void *xalloc(int size)
{
    void *p = malloc(size);
    if (!p)
        die("malloc() of %d failed\n", size);
    memset(p, 0, size);
    return p;
}

void *xrealloc(void *p, int size)
{
    p = realloc(p, size);
    if (!p)
        die("realloc() of %d failed\n", size);
    return p;
}

/*
 * A utility function for treating a contiguous block of memory as a
 * two-dimensional array.
 *
 * Inputs:  num_rols -- The total number of rows in the two-dimensional array
 *          num_cols -- The total number of cols in the two-dimensional array
 *          row      -- The row we wish to access
 *          col      -- The col we wish to access
 *
 * Outputs: None
 *
 * Returns: The appropriate offset to access row,col
 */
int row_col(int num_rows, int num_cols, int row, int col)
{
    if (! (row < num_rows))
        die("row %d out of bounds for num_rows %d\n", row, num_rows);

    if (! (col < num_cols))
        die("col %d out of bounds for num_cols %d\n", col, num_cols);

    assert(row < num_rows);
    assert(col < num_cols);
    return row * num_cols + col;
}
