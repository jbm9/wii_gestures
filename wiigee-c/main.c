// vim:set ts=4 sw=4 ai et:

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "quantizer.h"
#include "util.h"

int main(int argc, char **argv)
{
    struct quantizer quantizer;
    struct gesture gesture;
    struct observation observation;

    // Initialize our quantizer object
    quantizer.states = 8;

    // Initialize our gesture object
    gesture.data_len = 20;
    gesture.data = xalloc(sizeof(struct coordinate) * gesture.data_len);
    for (int i = 0; i < gesture.data_len; i++) {
        gesture.data[i].x = i+1;
        gesture.data[i].y = i+1;
        gesture.data[i].z = i+1;
    }

    // Train
    trainCenteroids(&quantizer, &gesture);

    // Get out observation object back
    getObservationSequence(&quantizer, &gesture, &observation);

    // Dump it
    for (int i = 0; i < observation.sequence_len; i++) {
        printf("%4d: %d\n", i, observation.sequence[i]);
    }

    free(gesture.data);
    free(observation.sequence);
    return 0;
}
