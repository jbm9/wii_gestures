// vim:set ts=4 sw=4 ai et:

#ifndef _gesture_h
#define _gesture_h    1

typedef struct coordinate {
    double x, y, z;
} coordinate;

typedef struct gesture {
    double minacc, maxacc;   // Min and max acceleration
    struct coordinate *data; // WiimoteAccelerationEvent
    int data_len;
} gesture;

struct gesture *gesture_new();
void gesture_append(struct gesture *, double, double, double);
void gesture_free(struct gesture *);

#endif
