// vim:set ts=4 sw=4 ai et:

#define NUM_CENTROIDS   14

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

typedef struct quantizer {
    double radius;
    int states;
    double map[NUM_CENTROIDS][3];
} quantizer;

typedef struct coordinate {
    double x, y, z;
} coordinate;

typedef struct gesture {
    double maxacc; // "The maximal acceleration this gesture has got" -- XXX not used!?
    double minacc; // "The minimal acceleration this gesture has got" -- XXX not used!?

    struct coordinate *data; // WiimoteAccelerationEvent
    int data_len;
} gesture;

typedef struct observation {
    int *sequence;
    int sequence_len;
} observation;

void trainCenteroids        (struct quantizer *, struct gesture *);
void getObservationSequence (struct quantizer *, struct gesture *, struct observation *);
