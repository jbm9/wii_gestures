// vim:set ts=4 sw=4 ai et:

#ifndef _quantizer_h
#define _quantizer_h    1

#define MAP_SIZE   14

#include "gesture.h"

typedef struct quantizer {
    double radius;
    int states;
    double map[MAP_SIZE][3];
} quantizer;

typedef struct observation {
    int *sequence;
    int sequence_len;
} observation;

struct quantizer *quantizer_new(int);
void quantizer_free         (struct quantizer *);
void trainCenteroids        (struct quantizer *, struct gesture *);
struct observation *getObservationSequence (struct quantizer *, struct gesture *);
void observation_free();

#endif
