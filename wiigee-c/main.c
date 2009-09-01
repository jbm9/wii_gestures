// vim:set ts=4 sw=4 ai et:

#define _GNU_SOURCE /* define this before including stdio.h so we get getline() */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include "quantizer.h"
#include "util.h"

void read_input(struct gesture *gesture)
{
    char *line = NULL;
    size_t size = 0;

    while (getline(&line, &size, stdin) > 0) {
        double x, y, z;
        int num_found = sscanf(line, "%lf %lf %lf", &x, &y, &z);
        if (num_found != 3) {
            fprintf(stderr, "Parse error. Looking for 3 elements, but found %d: %s", num_found, line);
            exit(1);
        }

        gesture->data_len++;
        gesture->data = xrealloc(gesture->data, sizeof(struct coordinate) * gesture->data_len);
        gesture->data[gesture->data_len-1].x = x;
        gesture->data[gesture->data_len-1].y = y;
        gesture->data[gesture->data_len-1].z = z;
        debug("Input coordinate %f, %f, %f\n", x, y, z);
    }

    if (gesture->data_len < 1) {
        fprintf(stderr, "Couldn't read a single coordinate?\n");
        exit(1);
    }

    free(line);
}

void normalize_input(struct gesture *gesture)
{
    double minacc = DBL_MAX;
    double maxacc = DBL_MIN;

    for (int i = 0; i < gesture->data_len; i++) {
        maxacc = MAX(maxacc, fabs(gesture->data[i].x));
        maxacc = MAX(maxacc, fabs(gesture->data[i].y));
        maxacc = MAX(maxacc, fabs(gesture->data[i].z));

        minacc = MIN(minacc, fabs(gesture->data[i].x));
        minacc = MIN(minacc, fabs(gesture->data[i].y));
        minacc = MIN(minacc, fabs(gesture->data[i].z));
    }

    gesture->maxacc = maxacc;
    gesture->minacc = minacc;
    debug("minacc %f, maxacc %f\n", minacc, maxacc);
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
    read_input(&gesture);
    normalize_input(&gesture);

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
