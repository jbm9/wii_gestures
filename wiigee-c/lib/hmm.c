#include "hmm.h"

/* The dynamic arrays are row major format, so here are some convenience -------
 * wrappers for accessing the tables... ----------------------------------------
 */

//#pragma mark -
//#pragma mark accessors + mutators

void inline setEmitP(HmmStateRef hmm, uint state, uint obs, double value) {
	assert(state < hmm->numStates && obs < hmm->numObservations);
	hmm->p_emit[hmm->numObservations*state + obs] = value;
}

void inline setInitP(HmmStateRef hmm, uint state, double value) {
	assert(state < hmm->numStates);
	hmm->p_initial[state] = value;
}

void inline setChangeP(HmmStateRef hmm, uint from, uint to, double value) {
	assert(from < hmm->numStates && to < hmm->numStates);
	hmm->p_change[hmm->numStates*from + to] = value;
}

double inline getChangeP(HmmStateRef hmm, uint from, uint to) {
	assert(from < hmm->numStates);
	assert(to   < hmm->numStates);
	return hmm->p_change[hmm->numStates*from + to];
}

double inline getInitP(HmmStateRef hmm, uint state) {
	assert(state < hmm->numStates);
	return hmm->p_initial[state];
}

double inline getEmitP(HmmStateRef hmm, uint state, uint obs) {
	assert(state < hmm->numStates && obs < hmm->numObservations);
	return hmm->p_emit[hmm->numObservations*state + obs];
}

/* end of convenience wrappers ---------------------------------------------- */

//#pragma mark -
//#pragma mark allocation and destruction

void resetHmm(HmmStateRef hmm) {
	int i, j;
	
	/* assume we start in the first state, for now. */
	hmm->p_initial[0] = 1.0f;
	
	/* as noted in the wiigee source code; "static" and "quick & dirty" */
	/* FIXME: somebody should unravel this and code it more sensibly */
	int jumplimit = 2;
	for (i = 0; i < hmm->numStates; i++) {
		for (j = 0; j < hmm->numStates; j++) {
			
			if (i==hmm->numStates-1 && j==hmm->numStates-1) { // last row
				setChangeP(hmm, i, j, 1.0);
				
			} else if (i == hmm->numStates-2 && j == hmm->numStates-2) { // next to last row
				setChangeP(hmm, i, j, 0.5);
				
			} else if (i == hmm->numStates-2 && j == hmm->numStates-1) { // next to last row
				setChangeP(hmm, i, j, 0.5);
				
			} else if (i <= j && i > (j-jumplimit-1)) {
				setChangeP(hmm, i, j, 1.0/(double)(jumplimit+1));
				
			} else {
				setChangeP(hmm, i, j, 0.0);
			}	
		}
	}
	
	/* setup the emit probabilities */
	for (i = 0; i < hmm->numStates; i++) {
		for (j = 0; j < hmm->numObservations; j++) {
			setEmitP(hmm, i, j, 1.0 / (double)hmm->numObservations);
		}
	}
}

HmmState* hmm_new(uint numStates, uint numObservations) {	
	assert(numStates > 0 && numObservations > 0);
	
	HmmStateRef hmm = (HmmStateRef)malloc(sizeof(HmmState));
		
	hmm->numStates = numStates;
	hmm->numObservations = numObservations;
	
	/* malloc + 0.0 initialize everything */
	hmm->p_initial = (double*)calloc(sizeof(double), numStates);
	hmm->p_change  = (double*)calloc(sizeof(double), numStates * numStates);
	hmm->p_emit    = (double*)calloc(sizeof(double), numStates * numObservations);
	
	resetHmm(hmm);
	
	return hmm;
}

void hmm_free(HmmStateRef hmm) {
	assert(hmm != NULL);
	
	/* free up the internal arrays */
	free(hmm->p_initial);
	free(hmm->p_change);
	free(hmm->p_emit);
	
	/* ok: we're good to blow away the struct now */
	free(hmm);
	
	/* light is green, trap is clean. */
}

StateSequenceRef createStateSequence(uint* states, uint length) {
	assert(length > 0);
	
	uint i;
	
	StateSequenceRef sequence = (StateSequenceRef)malloc(sizeof(StateSequenceRef));
	
	sequence->length = length;
	sequence->states = malloc(sizeof(uint) * length);
	
	// XXX: could memcpy if we care about speed, which we don't.
	for (i = 0; i < length; i++) {
		sequence->states[i] = states[i];
	}
	
	return sequence;
}

void releaseStateSequence(StateSequenceRef sequence) {
	free(sequence->states);
	free(sequence);
}

//#pragma mark -
//#pragma mark utility

/* Utility funtion to dump out the state of the model */
void dumpModel(HmmState* hmm) {
	uint i, j;

	fprintf(stdout, "Initial Probabilites: [ ");
	for (i = 0; i < hmm->numStates - 1; i++) {
		fprintf(stdout, "%1.3f, ", hmm->p_initial[i]);
	}
	fprintf(stdout, "%1.3f ]\n", hmm->p_initial[hmm->numStates]);

	fprintf(stdout, "State Change Probabilites:\n");
	for (i = 0; i < hmm->numStates; i++) {
		fprintf(stdout, "%d:\t", i);
		for (j = 0; j < hmm->numStates; j++ ) {
			fprintf(stdout, "%1.3f\t", getChangeP(hmm, i, j));
		}
		fprintf(stdout, "\n");
	}

	fprintf(stdout, "Emission Probabilities:\n");
	for (i = 0; i < hmm->numStates; i++) {
		fprintf(stdout, "%d:\t", i);
		for (j = 0; j < hmm->numObservations; j++ ) {
			fprintf(stdout, "%1.3f\t", getEmitP(hmm, i,j));
		}
		fprintf(stdout, "\n");
	}
}

//#pragma mark -
//#pragma mark "logic"

/*
 * param sequences: an array of 'num' sequences with which to train the model.
 */
void hmm_train(HmmStateRef hmm, StateSequenceRef* sequences, uint num) {
	assert(hmm && sequences && num > 0);
	
	uint i, j, k, t;
	StateSequenceRef sequence;
	double numer, denom;
	double numer_inner, denom_inner;
	double *forward, *backward;
	double prob;
	
	double *change_new = (double*)malloc(sizeof(double) * hmm->numStates * hmm->numStates);
	double *emit_new   = (double*)malloc(sizeof(double) * hmm->numStates * hmm->numObservations);

	// recalculate the state change probabilities:
	for (i = 0; i < hmm->numStates; i++) {
		for (j = 0; j < hmm->numStates; j++) {
			numer = denom = 0.0;
			
			for (k = 0; k < num; k++) {
				resetHmm(hmm); // of this, I am suspect
				
				// grab the k'th sequence:
				sequence = sequences[k];
				
				// run the forward and backward algorithms on this sequence
				forward  = forwardAlgorithm(hmm, sequence);
				backward = backwardAlgorithm(hmm, sequence);
				prob     = getProbability(hmm, sequence);
				
				numer_inner = denom_inner = 0.0;
				for (t = 0; t < sequence->length - 1; t++) {
					
					numer_inner += (forward[i * sequence->length + t] *
						getChangeP(hmm, i, j) *
						getEmitP(hmm, j, sequence->states[t+1]) *
						backward[j * sequence->length + (t+1)]);
								
					denom_inner += (forward[i * sequence->length + t] *
						backward[i * sequence->length + t]);
				}
				
				numer += (1.0/prob) * numer_inner;
				denom += (1.0/prob) * denom_inner;
				
				// they are my responsibility:
				free(forward);
				free(backward);
			}
			
			change_new[i * hmm->numStates + j] = numer / denom;
		}
	}

	// recalculate the emissions probabilites:

	/* dump the old */
	free(hmm->p_change);
	free(hmm->p_emit);
	
	/* lovingly embrace the new */
	hmm->p_change = change_new;
	hmm->p_emit   = emit_new;
}

/*
 * Backward Algorithm.  Calling code is responsible for freeing 'results'
 *
 * Translated as directly as possible from the Wiigee Java.  No references
 * given; documentation will evolve as I understand this better.
 */
double* backwardAlgorithm(HmmStateRef hmm, StateSequenceRef sequence) { 	
	int i, j, t;
	
	uint length = sequence->length;
	
	// a conceptually 2D table of numStates rows and length of sequence cols:
	double* results = (double*)calloc(sizeof(double), hmm->numStates * length);
	
	// initialize the last element for each state to 1.0:
	for (i = 0; i < hmm->numStates; i++) {
		results[length * i + (length - 1)] = 1.0;
	}
	
	// work our way backwards:
	for (t = length - 2; t >= 0; t--) {
		for (i = 0; i < hmm->numStates; i++) {
			results[i * length + t] = 0.0;
			for (j = 0; j < hmm->numStates; j++) {
				results[i * length + t] += results[j * length + (t+1)] *
				                           getChangeP(hmm, i, j) *
                                           getEmitP(hmm, j, sequence->states[t + 1]); 
			}
		}
	}
	
	return results;
}

/*
 * Forward Algorithm.  Translated from the WiiGee function of the similar name.
 * Can't say I really grok this, alas.
 *
 * hmm      - the HMM, fucking obviously, sheesh
 * sequence - the observation sequence
 * length   - the length of the observation sequence
 * returns  - a 2D array of probabilities; caller will free it
 */
double* forwardAlgorithm(HmmStateRef hmm, StateSequenceRef sequence) {
	assert(hmm && sequence);
	
	uint i, j, k;
	uint length = sequence->length;
	
	double* results = (double*)calloc(sizeof(double), hmm->numStates * length);
	
	// P0 for each state = Pinitial[state] * EmissionP(state, first element in the sequence) 
	for (i = 0; i < hmm->numStates; i++) {
		// by advancing numStates with each iteration, we land on the first
		// entry for each state:
		results[i*hmm->numStates] = getInitP(hmm, i) * 
		                            getEmitP(hmm, i, sequence->states[0]);
	}
	
	// over all the states in the sequence:
	for (i = 1; i < length; i++) {
		
		// over all the states in the model:
		for (j = 0; j < hmm->numStates; j++) {
			
			double sum = 0.0;
			
			// over all the states in the model:
			for (k = 0; k < hmm->numStates; k++) {
				
				// XXX: this is admittedly nasty:
				sum += results[(k*hmm->numStates)+(i-1)] * getChangeP(hmm, k, j);
			}
			
			results[j*hmm->numStates+i] = sum * getEmitP(hmm, j, sequence->states[i]);
		}
 	}
	
	return results;
}

/*
 * Returns the probability that the given sequence of states belongs
 * to this HMM.
 *
 * sequence: a sequence of states
 * length:   the length of said sequence
 */
double getProbability(HmmStateRef hmm, StateSequenceRef sequence) {
	uint i;
	double prob = 0.0;
	uint length = sequence->length;
	
	// call the forward algorithm, which returns a list of probs
	double *results = forwardAlgorithm(hmm, sequence);
	
	// NOTE: ^^ this here array is of length hmm->numStates * sequence length
	
	// add up the returned probabilities:
	for (i = 0; i < hmm->numStates * length; i++) {
		
		// we care about what's in the last entry of the table for each
		// state:
		prob += results[i*hmm->numStates + (length-1)];
	}
	
	/* clean up after "forward" */
	free(results);
	
	return prob;
}
