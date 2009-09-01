// vim:set ts=4 sw=4 ai et:

#include <malloc.h>

#include "observation.h"
#include "util.h"

struct observation *observation_new()
{
    struct observation *observation = xalloc(sizeof(struct observation));
    observation->sequence_len = 0;
    observation->sequence = NULL;
    return observation;
}

void observation_append(struct observation *observation, int i)
{
    observation->sequence_len++;
    observation->sequence = xrealloc(observation->sequence, sizeof(int) * observation->sequence_len);
    observation->sequence[observation->sequence_len-1] = i;
}

void observation_free(struct observation *observation)
{
    free(observation->sequence);
    free(observation);
}


