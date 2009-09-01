// vim:set ts=4 sw=4 ai et:

#include <stdio.h>
#include <float.h>
#include <malloc.h>

#include "gesture.h"
#include "util.h"

struct gesture *gesture_new()
{
    struct gesture *gesture = xalloc(sizeof(struct gesture));
    gesture->data_len = 0;
    gesture->data = NULL;
    gesture->minacc = DBL_MAX;
    gesture->maxacc = DBL_MIN;
    return gesture;
}

void gesture_append(struct gesture *gesture, double x, double y, double z)
{
    gesture->data_len++;
    gesture->data = xrealloc(gesture->data, sizeof(struct coordinate) * gesture->data_len);
    gesture->data[gesture->data_len-1].x = x;
    gesture->data[gesture->data_len-1].y = y;
    gesture->data[gesture->data_len-1].z = z;
}

void gesture_free(struct gesture *gesture)
{
    free(gesture->data);
    free(gesture);
}
