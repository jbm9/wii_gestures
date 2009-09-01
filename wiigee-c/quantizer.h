// vim:set ts=4 sw=4 ai et:

#define MAP_SIZE   14

typedef struct quantizer {
    double radius;
    int states;
    double map[MAP_SIZE][3];
} quantizer;

typedef struct coordinate {
    double x, y, z;
} coordinate;

typedef struct gesture {
    double minacc, maxacc;   // Min and max acceleration
    struct coordinate *data; // WiimoteAccelerationEvent
    int data_len;
} gesture;

typedef struct observation {
    int *sequence;
    int sequence_len;
} observation;

void trainCenteroids        (struct quantizer *, struct gesture *);
void getObservationSequence (struct quantizer *, struct gesture *, struct observation *);
