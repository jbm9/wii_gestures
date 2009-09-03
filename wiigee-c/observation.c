// vim:set ts=4 sw=4 ai et:

#include <stdlib.h>

#include "observation.h"
#include "util.h"
#include "hmm.h"

struct observation *observation_new()
{
    struct observation *this = xalloc(sizeof(struct observation));
    this->sequence_len = 0;
    this->sequence = NULL;
    return this;
}

void observation_append(struct observation *this, int i)
{
    this->sequence_len++;
    this->sequence = xrealloc(this->sequence, sizeof(int) * this->sequence_len);
    this->sequence[this->sequence_len-1] = i;
}

void observation_free(struct observation *this)
{
    free(this->sequence);
    free(this);
}


// Convert a struct observation to a StateSequence, so we can interface with the hmm.c code
StateSequence *observation_to_StateSequence(struct observation *this)
{
    unsigned int states[this->sequence_len];

    for (int i = 0; i < this->sequence_len; i++)
        states[i] = this->sequence[i];

    return createStateSequence(states, this->sequence_len);
}
