// vim:set ts=4 sw=4 ai et:

#include "gesturemodel.h"
#include "util.h"

struct gesturemodel *gesturemodel_new(int id)
{
    struct gesturemodel *this = xalloc(sizeof(struct gesturemodel));

    this->id           = id;
    this->states       = 8;     // n=8 states empirical value
    this->observations = 14;    // k=14 observations empirical value
    this->quantizer    = quantizer_new(this->states);
    this->markovmodel = hmm_new(this->states, this->observations);

    return this;
}

void gesturemodel_free(struct gesturemodel *this)
{
    quantizer_free(this->quantizer);
    hmm_free(this->markovmodel);
    free(this);
}

// void train(Vector<Gesture> trainsequence)
void train(struct gesture *trainsequence, int trainsequence_len)
{
    //double maxacc = 0;
    //double minacc = 0;
    //struct gesture gesture;
}
