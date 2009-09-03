// vim:set ts=4 sw=4 ai et:

#ifndef _quantizer_h
#define _quantizer_h    1

#define MAP_SIZE   14

#include "gesture.h"
#include "observation.h"

typedef struct quantizer {
    double radius;
    int states;
    double map[MAP_SIZE][3];
} quantizer;

struct quantizer *quantizer_new(int);
void quantizer_free         (struct quantizer *);
void quantizer_trainCenteroids        (struct quantizer *, struct gesture *);
struct observation *quantizer_getObservationSequence (struct quantizer *, struct gesture *);

#endif
