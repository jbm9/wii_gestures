// vim:set ts=4 sw=4 ai et:

#include <math.h>       // cos and friends
#include <values.h>     // MAXDOUBLE
#include <string.h>     // memcpy

#include <stdlib.h>
#include <stdio.h>

#define NUM_CENTROIDS   14

typedef struct quantizer {
    double radius;
    int states;
    double map[NUM_CENTROIDS][3];
} quantizer;

typedef struct coordinate {
    double x, y, z;
} coordinate;

typedef struct gesture {
    double maxacc; // The maximal acceleration this gesture has got
    double minacc; // The minimal acceleration this gesture has got

    struct coordinate *data; // WiimoteAccelerationEvent
    int data_len;
} gesture;

////////////////////////////////////////////////////////////////////////////////

double getMaxAcc(struct coordinate *c, int data_len) {
    double max = 0;
    for (int i = 0; i < data_len; i++) {
        max = max > fabs(c[i].x) ? max : fabs(c[i].x);
        max = max > fabs(c[i].y) ? max : fabs(c[i].y);
        max = max > fabs(c[i].z) ? max : fabs(c[i].z);
    }
    return max;
}

double getMinAcc(struct coordinate *c, int data_len) {
    double min = MAXDOUBLE;
    for (int i = 0; i < data_len; i++) {
        min = min < fabs(c[i].x) ? min : fabs(c[i].x);
        min = min < fabs(c[i].y) ? min : fabs(c[i].y);
        min = min < fabs(c[i].z) ? min : fabs(c[i].z);
    }
    return min;
}

// public int[][] deriveGroups(Gesture gesture)
int *deriveGroups(struct quantizer *this, struct gesture *g) {
    int *groups = calloc(NUM_CENTROIDS * g->data_len, sizeof(int));
    double d[NUM_CENTROIDS * g->data_len];
    double curr[3];
    double vector[3];

    for (int i = 0; i < NUM_CENTROIDS; i++) { // lines

        double ref[3];

        ref[0] = this->map[i][0];
        ref[1] = this->map[i][1];
        ref[2] = this->map[i][2];

        for (int j = 0; j < g->data_len; j++) { // splits
            curr[0] = g->data[j].x;
            curr[1] = g->data[j].y;
            curr[2] = g->data[j].z;

            vector[0] = ref[0] - curr[0];
            vector[1] = ref[1] - curr[1];
            vector[2] = ref[2] - curr[2];

            d[i*NUM_CENTROIDS+j] = sqrt((vector[0] * vector[0])
                           + (vector[1] * vector[1])
                           + (vector[2] * vector[2]));
        }
    }

    // look, to which group a value belongs
    for (int j = 0; j < g->data_len; j++) {
        double smallest = MAXDOUBLE;
        int row = 0;

        for (int i = 0; i < NUM_CENTROIDS; i++) {
            if (d[i*NUM_CENTROIDS+j] < smallest) {
                smallest = d[i*NUM_CENTROIDS+j];
                row = i;
            }
            groups[i*NUM_CENTROIDS+j] = 0;
        }

        groups[row*NUM_CENTROIDS+j] = 1; // guppe gesetzt ("groups set")
    }

    return groups;
}

// public void trainCenteroids(Gesture gesture) {
void train(struct quantizer *this, struct gesture *gesture) {
    double pi = 3.14; // M_PI;
    this->radius = (getMaxAcc(gesture->data, gesture->data_len) + 
                   getMinAcc(gesture->data, gesture->data_len)) / 2;
    printf("Using radius %f\n", this->radius);

    double foo[NUM_CENTROIDS][3] = {
            { this->radius,                 0.0,                        0.0                      },
            { cos(pi/4)*this->radius,       0.0,                        sin(pi/4)*this->radius   },
            { 0.0,                          0.0,                        this->radius             },
            { cos(pi*3/4)*this->radius,     0.0,                        sin(pi*3/4)*this->radius },
            { -this->radius,                0.0,                        0.0                      },
            { cos(pi*5/4)*this->radius,     0.0,                        sin(pi*5/4)*this->radius },
            { 0.0,                          0.0,                        -this->radius            },
            { cos(pi*7/4)*this->radius,     0.0,                        sin(pi*7/4)*this->radius },
            { 0.0,                          this->radius,               0.0                      },
            { 0.0,                          cos(pi/4)*this->radius,     sin(pi/4)*this->radius   },
            { 0.0,                          cos(pi*3/4)*this->radius,   sin(pi*3/4)*this->radius },
            { 0.0,                          -this->radius,              0.0                      },
            { 0.0,                          cos(pi*5/4)*this->radius,   sin(pi*5/4)*this->radius },
            { 0.0,                          cos(pi*7/4)*this->radius,   sin(pi*7/4)*this->radius },
        };

    memcpy(&this->map, &foo, sizeof(foo)); // josh made me do it

    int *g     = calloc(NUM_CENTROIDS * gesture->data_len, 1);
    int *g_old = calloc(NUM_CENTROIDS * gesture->data_len, 1);

    do {
        // Derive new Groups...
        memcpy(g_old, g, sizeof(g));
        g = deriveGroups(this, gesture);

        // calculate new centeroids
        for (int i = 0; i < NUM_CENTROIDS; i++) {
            double countX = 0; // "zaehler"
            double countY = 0;
            double countZ = 0;
            int denominator = 0; // "nenner"
            for (int j = 0; j < gesture->data_len; j++) {
                if (g[i*NUM_CENTROIDS+j] == 1) {
                    countX += gesture->data[j].x;
                    countY += gesture->data[j].y;
                    countZ += gesture->data[j].z;
                    denominator++;
                }
            }

            if (denominator) {
                this->map[i][0] = countX / (double) denominator;
                this->map[i][1] = countY / (double) denominator;
                this->map[i][2] = countZ / (double) denominator;
            }
        }
    } while (memcmp(g, g_old, sizeof(g)) == 0);
}

int *getObservationSequence(struct quantizer *this, struct gesture *gesture, int *out_length)
{
    int *groups = deriveGroups(this, gesture);
    int *sequence = calloc(NUM_CENTROIDS*NUM_CENTROIDS, sizeof(int));
    int sequence_length = 0;

    for (int j = 0; j < NUM_CENTROIDS; j++) {  // spalten
        for (int i = 0; i < NUM_CENTROIDS; i++) { // zeilen
            if (groups[i*NUM_CENTROIDS+j] == 1) {
                // System.out.print(" "+ i);
                sequence[sequence_length++] = i;
                break;
            }
        }
    }

    while (sequence_length < this->states) {
        sequence[sequence_length] = sequence[sequence_length-1];
        sequence_length++;
    }

    *out_length = sequence_length;
    return sequence;
}


// public void printMap()
// private boolean equalarrays(int[][] one, int[][] two)

int main(int argc, char **argv)
{
    return 0;
}
