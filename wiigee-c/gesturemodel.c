// vim:set ts=4 sw=4 ai et:

#include "gesturemodel.h"
#include "observation.h"
#include "quantizer.h"
#include "hmm.h"
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
void gesturemodel_train(struct gesturemodel *this, struct gesture *trainsequence, int trainsequence_len)
{
    struct gesture *sum = gesture_new();
    double minacc = 0;
    double maxacc = 0;

    // summarize all vectors from the different gestures in the trainsequence into a single gesture

    for (int i = 0; i < trainsequence_len; i++) {
        minacc += trainsequence[i].minacc;
        maxacc += trainsequence[i].maxacc;

        for (int j = 0; j < trainsequence[i].data_len; j++) {
            // transfer every single accelerationevent of each gesture to the new gesture sum
            gesture_append(sum,
                           trainsequence[i].data[j].x,
                           trainsequence[i].data[j].y,
                           trainsequence[i].data[j].z);
        }
    }

    // average the max and min accelerations
    sum->minacc = minacc / trainsequence_len;
    sum->maxacc = maxacc / trainsequence_len;

    // train the centeroids of the quantizer with this master gesture sum
    quantizer_trainCenteroids(this->quantizer, sum);

    // convert gesture vector to a sequence of discrete values
    struct observation *seqs = observation_new();

    for (int i = 0; i < trainsequence_len; i++) {
        struct observation *observation = quantizer_getObservationSequence(this->quantizer, &trainsequence[i]);

        for (int j = 0; j < observation->sequence_len; j++) {
            observation_append(seqs, observation->sequence[j]);
        }
    }

}
