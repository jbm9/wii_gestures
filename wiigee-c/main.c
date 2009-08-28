// vim:set ts=4 sw=4 ai et:

#define _GNU_SOURCE /* define this before including stdio.h so we get getline() */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "quantizer.h"
#include "util.h"

void readinput(struct gesture *gesture)
{
    char *line = NULL;
    size_t size = 0;

    while (getline(&line, &size, stdin) > 0) {
        int x, y, z;
        int found;
        
        found = sscanf(line, "%d %d %d", &x, &y, &z);
        if (found != 3) {
            fprintf(stderr, "Parse error. Looking for 3 elements, but found %d: %s", found, line);
            exit(1);
        }

        gesture->data_len++;
        gesture->data = xrealloc(gesture->data, sizeof(struct coordinate) * gesture->data_len);
        gesture->data[gesture->data_len-1].x = x;
        gesture->data[gesture->data_len-1].y = y;
        gesture->data[gesture->data_len-1].z = z;
    }

    if (gesture->data_len < 1) {
        fprintf(stderr, "Couldn't read a single coordinate?\n");
        exit(1);
    }

    free(line);
}

int main(int argc, char **argv)
{
    struct quantizer quantizer;
    struct gesture gesture;
    struct observation observation;

    // Initialize our quantizer object
    quantizer.states = 8;

    // Initialize our gesture object
    gesture.data_len = 0;
    gesture.data = NULL;
    readinput(&gesture);

    for (int i = 0; i < gesture.data_len; i++) {
        printf("Input coordinate %f, %f, %f\n",
            gesture.data[i].x, gesture.data[i].y, gesture.data[i].z);
    }

    // Train
    trainCenteroids(&quantizer, &gesture);

    // Get out observation object back
    getObservationSequence(&quantizer, &gesture, &observation);

    // Dump it
    for (int i = 0; i < observation.sequence_len; i++) {
        printf("%d\n", observation.sequence[i]);
    }

    free(gesture.data);
    free(observation.sequence);
    return 0;
}
