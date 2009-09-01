// vim:set ts=4 sw=4 ai et:

#ifndef _gesturemodel_h
#define _gesturemodel_h   1

#include "quantizer.h"
#include "hmm.h"

typedef struct gesturemodel {
    int states;                  // The number of states the hidden markov model consists of
    int observations;            // The number of observations for the hmm and k-mean
    int id;                      // The id representation of this model
    struct quantizer *quantizer; // The quantization component
    HmmState *markovmodel;       // The statistical model, hidden markov model
    double defaultprobability;   // The default probability of this gesturemodel, needed for the bayes classifier
} gesturemodel;

struct gesturemodel *gesturemodel_new(int id);
void gesturemodel_free(struct gesturemodel *this);

#endif
