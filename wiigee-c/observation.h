// vim:set ts=4 sw=4 ai et:

#ifndef _observation_h
#define _observation_h    1

#include "hmm.h"

typedef struct observation {
    int *sequence;
    int sequence_len;
} observation;

struct observation *observation_new();
void observation_append(struct observation *, int);
void observation_free();
StateSequence *observation_to_StateSequence(struct observation *);

#endif
