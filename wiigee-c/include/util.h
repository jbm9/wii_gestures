// vim:set ts=4 sw=4 ai et:

#ifndef _util_h
#define _util_h 1

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

void *xalloc(int);
void *xrealloc(void *, int);
int row_col(int, int, int, int);
int debug(const char *, ...);
int die(const char *, ...);

#endif
