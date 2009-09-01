// vim:set ts=4 sw=4 ai et:

#include <math.h>       // cos and friends
#include <float.h>      // DBL_MAX
#include <string.h>     // memcpy
#include <assert.h>

#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "quantizer.h"

void initialize_centroids(struct quantizer *this, struct gesture *gesture)
{
    double pi = 3.14; // M_PI;
    this->radius = (gesture->minacc + gesture->maxacc) / 2;

    debug("Using radius: %f\n", this->radius);

    #if MAP_SIZE != 14
    #error "If you change MAP_SIZE, you must update initialize_centroids()"
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

    debug("Initial centroids:\n");
    for (int i = 0; i < MAP_SIZE; i++) {
        debug("   %2d:  %9.5f  %9.5f  %9.5f\n",
            i,
            this->map[i][0],
            this->map[i][1],
            this->map[i][2]);
    }
    debug("\n");
}

int *deriveGroups(struct quantizer *this, struct gesture *gesture)
{
    int groups[MAP_SIZE * gesture->data_len];

    double d[MAP_SIZE * gesture->data_len];

    debug("\nderiveGroups\n");

    // Calculate cartesian distance
    for (int i = 0; i < MAP_SIZE; i++) { // lines
        double ref[3];
        ref[0] = this->map[i][0];
        ref[1] = this->map[i][1];
        ref[2] = this->map[i][2];

        debug("|");
        for (int j = 0; j < gesture->data_len; j++) { // splits
            double curr[3];
            curr[0] = gesture->data[j].x;
            curr[1] = gesture->data[j].y;
            curr[2] = gesture->data[j].z;

            double vector[3];
            vector[0] = ref[0] - curr[0];
            vector[1] = ref[1] - curr[1];
            vector[2] = ref[2] - curr[2];

            double newd = sqrt((vector[0] * vector[0])
                             + (vector[1] * vector[1])
                             + (vector[2] * vector[2]));
            d[ row_col(MAP_SIZE, gesture->data_len, i, j) ] = newd;
            debug("%5.0f |", newd);
        }
        debug("\n");
    }
    debug("\n");


    // look, to which group a value belongs
    for (int j = 0; j < gesture->data_len; j++) {
        double smallest = DBL_MAX;
        int row = 0;

        for (int i = 0; i < MAP_SIZE; i++) {
            if (d[i*gesture->data_len+j] < smallest) {
                smallest = d[i*gesture->data_len+j];
                row = i;
            }
            groups[i*gesture->data_len+j] = 0;
        }

        groups[row*gesture->data_len+j] = 1; // guppe gesetzt ("groups set")
    }

    int *ret = xalloc(sizeof(groups));
    return memcpy(ret, groups, sizeof(groups));
}

void trainCenteroids(struct quantizer *this, struct gesture *gesture)
{
    int size   = MAP_SIZE * gesture->data_len * sizeof(int);
    int *g     = xalloc(size);
    int *g_old = xalloc(size);

    initialize_centroids(this, gesture);

    do {
        // Derive new Groups...

        debug("Still in the loop\n");
        memcpy(g_old, g, size);
        free(g);
        g = deriveGroups(this, gesture);

        // calculate new centeroids
        for (int i = 0; i < MAP_SIZE; i++) {
            debug("iter i %d\n", i);
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
                debug("Centeroid: %d: %5.1f  %5.1f  %5.1f\n",
                        i, this->map[i][0], this->map[i][1], this->map[i][2]);
            }
        }

        debug("old g:\n");
        for (int row = 0; row < MAP_SIZE; row++) {
            for (int col = 0; col < gesture->data_len; col++) {
                debug("%d ", g_old[ row_col(MAP_SIZE, gesture->data_len, row, col) ]);
            }
            debug("\n");
        }
        debug("new g:\n");
        for (int row = 0; row < MAP_SIZE; row++) {
            for (int col = 0; col < gesture->data_len; col++) {
                debug("%d ", g[ row_col(MAP_SIZE, gesture->data_len, row, col) ]);
            }
            debug("\n");
        }
        debug("\n");


    } while (memcmp(g, g_old, size) != 0);

    debug("Final g output:\n");
    //for (int i = 0; i < this->states; i++) {
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < gesture->data_len; j++) {
            debug("%d|", g[ row_col(MAP_SIZE, gesture->data_len, i, j) ]);
        }
        debug("\n");
    }

    debug("Final map value:\n");
    // double map[MAP_SIZE][3];
    for (int i = 0; i < MAP_SIZE; i++) {
        debug("   %2d:  %5.1f  %5.1f  %5.1f\n",
            i,
            this->map[i][0],
            this->map[i][1],
            this->map[i][2]);
    }

    free(g);
    free(g_old);
    debug("trainCenteroids returning\n");
}

void getObservationSequence(struct quantizer *this, struct gesture *gesture, struct observation *observation)
{
    debug("getObservationSequence starting\n");
    int *groups = deriveGroups(this, gesture);

    observation->sequence_len = 0;
    observation->sequence = NULL;

    debug("Visible symbol sequence:\n");

    for (int j = 0; j < gesture->data_len; j++) {  // spalten
        for (int i = 0; i < MAP_SIZE; i++) { // zeilen
            if (groups[ row_col(MAP_SIZE, gesture->data_len, i, j) ] == 1) {
                debug("%d\n", i);
                observation->sequence_len++;
                observation->sequence = xrealloc(observation->sequence, sizeof(int) * observation->sequence_len);
                observation->sequence[observation->sequence_len-1] = i;
                break;
            } else {
                debug("skipping groups[%d][%d] == %d)\n", i, j, groups[row_col(MAP_SIZE, gesture->data_len, i, j) ]);
            }
        }
    }

    if (observation->sequence_len < this->states)
        debug("padding out to %d\n", this->states);

    while (observation->sequence_len < this->states) {
        observation->sequence_len++;
        observation->sequence = xrealloc(observation->sequence, sizeof(int) * observation->sequence_len);
        observation->sequence[observation->sequence_len-1] = observation->sequence[observation->sequence_len-2];
        debug("%d\n", observation->sequence[observation->sequence_len-1]);
    }

    debug("returning\n\n\n");
    free(groups);
}
