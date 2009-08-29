// vim:set ts=4 sw=4 ai et:

#include <math.h>       // cos and friends
#include <float.h>      // DBL_MAX
#include <string.h>     // memcpy
#include <assert.h>

#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "quantizer.h"

double getMaxAcc(struct coordinate *c, int data_len)
{
    double max = 0;
    for (int i = 0; i < data_len; i++) {
        max = MAX(max, fabs(c[i].x));
        max = MAX(max, fabs(c[i].y));
        max = MAX(max, fabs(c[i].z));
    }
    return max;
}

double getMinAcc(struct coordinate *c, int data_len)
{
    double min = DBL_MAX;
    for (int i = 0; i < data_len; i++) {
        min = MIN(min, fabs(c[i].x));
        min = MIN(min, fabs(c[i].y));
        min = MIN(min, fabs(c[i].z));
    }
    return min;
}

void initialize_centroids(struct quantizer *this, struct gesture *gesture)
{
    double pi = 3.14; // M_PI;
    this->radius = (getMaxAcc(gesture->data, gesture->data_len) + 
                    getMinAcc(gesture->data, gesture->data_len)) / 2;

    printf("Using radius %f\n", this->radius);

    #if NUM_CENTROIDS != 14
    #error "If you change NUM_CENTROIDS, you must update initialize_centroids()"
    #endif

    int i = 0;

    #define _add(x, y, z) do { \
        this->map[i][0] = x; \
        this->map[i][1] = y; \
        this->map[i][2] = z; \
        i++; \
    } while (0);

    _add( this->radius,               0.0,                         0.0                      );
    _add( cos(pi/4)*this->radius,     0.0,                         sin(pi/4)*this->radius   );
    _add( 0.0,                        0.0,                         this->radius             );
    _add( cos(pi*3/4)*this->radius,   0.0,                         sin(pi*3/4)*this->radius );
    _add( -this->radius,              0.0,                         0.0                      );
    _add( cos(pi*5/4)*this->radius,   0.0,                         sin(pi*5/4)*this->radius );
    _add( 0.0,                        0.0,                         -this->radius            );
    _add( cos(pi*7/4)*this->radius,   0.0,                         sin(pi*7/4)*this->radius );
    _add( 0.0,                        this->radius,                0.0                      );
    _add( 0.0,                        cos(pi/4)*this->radius,      sin(pi/4)*this->radius   );
    _add( 0.0,                        cos(pi*3/4)*this->radius,    sin(pi*3/4)*this->radius );
    _add( 0.0,                        -this->radius,               0.0                      );
    _add( 0.0,                        cos(pi*5/4)*this->radius,    sin(pi*5/4)*this->radius );
    _add( 0.0,                        cos(pi*7/4)*this->radius,    sin(pi*7/4)*this->radius );

    printf("Initial centroids:\n");
    for (int i = 0; i < NUM_CENTROIDS; i++) {
        printf("   %2d:  %9.5f  %9.5f  %9.5f\n",
            i,
            this->map[i][0],
            this->map[i][1],
            this->map[i][2]);
    }
    printf("\n");
}

int *deriveGroups(struct quantizer *this, struct gesture *gesture)
{
    int *groups = xalloc(NUM_CENTROIDS * MAX(gesture->data_len, NUM_CENTROIDS) * sizeof(int));
    double d[NUM_CENTROIDS * gesture->data_len];
    double curr[3];
    double vector[3];

    for (int i = 0; i < NUM_CENTROIDS; i++) { // lines

        double ref[3];

        ref[0] = this->map[i][0];
        ref[1] = this->map[i][1];
        ref[2] = this->map[i][2];

        for (int j = 0; j < gesture->data_len; j++) { // splits
            curr[0] = gesture->data[j].x;
            curr[1] = gesture->data[j].y;
            curr[2] = gesture->data[j].z;

            vector[0] = ref[0] - curr[0];
            vector[1] = ref[1] - curr[1];
            vector[2] = ref[2] - curr[2];

            d[i*gesture->data_len+j] = sqrt((vector[0] * vector[0])
                                    + (vector[1] * vector[1])
                                    + (vector[2] * vector[2]));
        }
    }

    // look, to which group a value belongs
    for (int j = 0; j < gesture->data_len; j++) {
        double smallest = DBL_MAX;
        int row = 0;

        for (int i = 0; i < NUM_CENTROIDS; i++) {
            if (d[i*gesture->data_len+j] < smallest) {
                smallest = d[i*gesture->data_len+j];
                row = i;
            }
            groups[i*gesture->data_len+j] = 0;
        }

        groups[row*gesture->data_len+j] = 1; // guppe gesetzt ("groups set")
    }

    return groups;
}

void trainCenteroids(struct quantizer *this, struct gesture *gesture)
{
    int size   = NUM_CENTROIDS * gesture->data_len;
    int *g     = xalloc(size);
    int *g_old = xalloc(size);

    initialize_centroids(this, gesture);

    do {
        // Derive new Groups...
        memcpy(g_old, g, size);
        free(g);
        g = deriveGroups(this, gesture);

        // calculate new centeroids
        for (int i = 0; i < NUM_CENTROIDS; i++) {
            double countX = 0; // "zaehler"
            double countY = 0;
            double countZ = 0;
            int denominator = 0; // "nenner"

            for (int j = 0; j < gesture->data_len; j++) {
                if (g[i*gesture->data_len+j] == 1) {
                    countX += gesture->data[j].x;
                    countY += gesture->data[j].y;
                    countZ += gesture->data[j].z;
                    denominator++;
                }
            }

            if (denominator) {
                this->map[i][0] = countX / denominator;
                this->map[i][1] = countY / denominator;
                this->map[i][2] = countZ / denominator;
            }
        }
    } while (memcmp(g, g_old, size) == 0);

    free(g);
    free(g_old);
}

void getObservationSequence(struct quantizer *this, struct gesture *gesture, struct observation *observation)
{
    int *groups = deriveGroups(this, gesture);

    observation->sequence_len = 0;
    observation->sequence = xalloc(MAX(NUM_CENTROIDS, this->states)
                                 * MAX(NUM_CENTROIDS, this->states)
                                 * sizeof(int));

    for (int j = 0; j < NUM_CENTROIDS; j++) {  // spalten
        for (int i = 0; i < NUM_CENTROIDS; i++) { // zeilen
            if (groups[j*NUM_CENTROIDS+i] == 1) {
                // System.out.print(" "+ i);
                observation->sequence[observation->sequence_len++] = i;
                break;
            }
        }
    }

    while (observation->sequence_len < this->states) {
        observation->sequence[observation->sequence_len] = observation->sequence[observation->sequence_len-1];
        observation->sequence_len++;
    }

    free(groups);
}
