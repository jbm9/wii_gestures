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
    this->hmm          = hmm_new(this->states, this->observations);

    return this;
}

void gesturemodel_free(struct gesturemodel *this)
{
    quantizer_free(this->quantizer);
    hmm_free(this->hmm);
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
    StateSequence *seqs[trainsequence_len];
    for (int i = 0; i < trainsequence_len; i++) {
        struct observation *observation = quantizer_getObservationSequence(this->quantizer, &trainsequence[i]);
        seqs[i] = observation_to_StateSequence(observation);
        observation_free(observation);
    }

    // train the markov model with this derived discrete sequences
    hmm_train(this->hmm, seqs, trainsequence_len);

    // set the default probability
    setDefaultProbability(this, trainsequence, trainsequence_len);

    gesture_free(sum);
    for (int i = 0; i < trainsequence_len; i++)
        releaseStateSequence(seqs[i]);
}

double matches(struct gesturemodel *this, struct gesture *gesture)
{
    struct observation *observation = quantizer_getObservationSequence(this->quantizer, gesture);
    StateSequenceRef sequence = observation_to_StateSequence(observation);

    double out = getProbability(this->hmm, sequence);

    observation_free(observation);
    releaseStateSequence(sequence);
    return out;
}

void setDefaultProbability(struct gesturemodel *this, struct gesture *trainsequence, int trainsequence_len)
{
    double prob = 0;

    for (int i = 0; i < trainsequence_len; i++)
        prob += matches(this, &trainsequence[i]);

    this->defaultprobability = prob / trainsequence_len;
}
