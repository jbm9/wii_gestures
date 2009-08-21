// vim:set ts=4 sw=4 ai et:

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "quantizer.h"

int main(int argc, char **argv)
{
    struct quantizer quantizer;
    struct gesture gesture;

    setvbuf(stdout, NULL, _IONBF, 0);

    memset(&quantizer, 0, sizeof(quantizer));
    quantizer.states = 8;

    memset(&gesture, 0, sizeof(gesture));
    gesture.data_len = 3;
    gesture.data = calloc(sizeof(struct coordinate), gesture.data_len);
    for (int i = 0; i < gesture.data_len; i++) {
        gesture.data[i].x = i+1;
        gesture.data[i].y = i+1;
        gesture.data[i].z = i+1;
    }

    trainCenteroids(&quantizer, &gesture);

    struct observation observation;
    memset(&observation, 0, sizeof(observation));
    getObservationSequence(&quantizer, &gesture, &observation);

    for (int i = 0; i < observation.sequence_len; i++) {
        printf("%4d: %d\n", i, observation.sequence[i]);
    }

    free(gesture.data);
    free(observation.sequence);

    //q.map[NUM_CENTROIDS][3];

    //this.quantizer.trainCenteroids(sum);

    // observations 14

    // getObservationSequence
    return 0;
}
