/*
 * ML Wednesday hack-a-thon! 5.Aug.09
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef unsigned int uint;

#pragma mark -
#pragma mark structures

typedef struct _hmmState {
		
	/* the number of states */
	uint numStates;
	
	/* the number of observations / codebook entries */
	uint numObservations;
	
	/* array of size numStates containing the initial probabilities */
	double *p_initial;
	
	/* array of size numStates * numStates containing the p of moving from 
	 * state A to state B */
	double *p_change;
	
	/* array of size numStates * numObservations containing the p of state
	 * x emmiting obserbation y */
	double *p_emit;
		
} HmmState;
typedef HmmState* HmmStateRef;

typedef struct _stateSequence {
	
	/* the length of this sequence */
	uint  length;
	
	/* the array of states in the sequence */
	uint* states;
	
} StateSequence;
typedef StateSequence* StateSequenceRef;

#pragma mark -
#pragma mark methods to allocate and destory the above structures

/* Allocate a new HMM state struct for the given data sizes */
HmmStateRef createHmm(uint numStates, uint numObservations);

/* Dealloc an HMM state struct */
void releaseHmm(HmmStateRef hmm);

/* Allocate a new state sequence, initialized using the passed data.
 * Keeps it's own internal copy. */
StateSequenceRef createStateSequence(uint* states, uint length);

void releaseStateSequence(StateSequenceRef);

#pragma mark -
#pragma mark utility methods for dealing with the model

/* Utility funtion to dump out the state of the model */
void dumpModel(HmmStateRef hmm);

/* these are a bunch of convenience wrappers for mutating/accessing the
 * probability tables; this is needed since they are laid out in 1D,
 * row-major format. */

void inline   setEmitP   (HmmStateRef hmm, uint state, uint obs, double value);
void inline   setInitP   (HmmStateRef hmm, uint state, double value);
void inline   setChangeP (HmmStateRef hmm, uint from, uint to, double value);
double inline getChangeP (HmmStateRef hmm, uint from, uint to);
double inline getInitP   (HmmStateRef hmm, uint state);
double inline getEmitP   (HmmStateRef hmm, uint state, uint obs);
 