#include "hmm.h"


/* The dynamic arrays are row major format, so here are some convenience -------
 * wrappers for accessing the tables... ----------------------------------------
 */

#pragma mark -
#pragma mark accessors + mutators

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

#pragma mark -
#pragma mark allocation and destruction

HmmState* createHmm(uint numStates, uint numObservations) {
	int i, j;
	
	HmmStateRef hmm = (HmmStateRef)malloc(sizeof(HmmState));
	
	assert(numStates > 0 && numObservations > 0);
	
	hmm->numStates = numStates;
	hmm->numObservations = numObservations;
	
	/* malloc + 0.0 initialize everything */
	hmm->p_initial = (double*)calloc(sizeof(double), numStates);
	hmm->p_change  = (double*)calloc(sizeof(double), numStates * numStates);
	hmm->p_emit    = (double*)calloc(sizeof(double), numStates * numObservations);
	
	/* assume we start in the first state, for now. */
	hmm->p_initial[0] = 1.0f;
	
	/* as noted in the wiigee source code; "static" and "quick & dirty" */
	/* FIXME: somebody should unravel this and code it more sensibly */
	int jumplimit = 2;
	for (i = 0; i < hmm->numStates; i++) {
		for (j = 0; j < hmm->numStates; j++) {
			
			if (i==numStates-1 && j==numStates-1) { // last row
				setChangeP(hmm, i, j, 1.0);
				
			} else if (i == numStates-2 && j == numStates-2) { // next to last row
				setChangeP(hmm, i, j, 0.5);
				
			} else if (i == numStates-2 && j == numStates-1) { // next to last row
				setChangeP(hmm, i, j, 0.5);
				
			} else if (i <= j && i > (j-jumplimit-1)) {
				setChangeP(hmm, i, j, 1.0/(double)(jumplimit+1));
				
			} else {
				setChangeP(hmm, i, j, 0.0);
			}	
		}
	}
	
	/* setup the emit probabilities */
	for (i = 0; i < numStates; i++) {
		for (j = 0; j < numObservations; j++) {
			setEmitP(hmm, i, j, 1.0 / (double)numObservations);
		}
	}
	
	return hmm;
}

void releaseHmm(HmmStateRef hmm) {
	assert(hmm != NULL);
	
	/* free up the internal arrays */
	free(hmm->p_initial);
	free(hmm->p_change);
	free(hmm->p_emit);
	
	/* ok: we're good to blow away the struct now */
	free(hmm);
	
	/* light is green, trap is clean. */
}

#pragma mark -
#pragma mark utility

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
	
	fprintf(stdout, "Emmision Probabilites:\n");
	for (i = 0; i < hmm->numStates; i++) {
		fprintf(stdout, "%d:\t", i);
		for (j = 0; j < hmm->numObservations; j++ ) {
			fprintf(stdout, "%1.3f\t", getEmitP(hmm, i,j));
		}
		fprintf(stdout, "\n");
	}
}

#pragma mark -
#pragma mark "logic"

/*
 * Backward Algorithm.  Calling code is responsible for freeing 'results'
 *
 * Translated as directly as possible from the Wiigee Java.  No references
 * given; documentation will evolve.
 */
double* backwardAlgorithm(HmmStateRef hmm, uint* sequence, uint length) { 	
	int i, j, t;
	
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
                                           getEmitP(hmm, j, sequence[t + 1]); 
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
double* forwardAlgorithm(HmmStateRef hmm, uint* sequence, uint length) {
	assert(hmm && sequence && length > 0);
	
	uint i, j, k;
	
	double* results = (double*)calloc(sizeof(double), hmm->numStates * length);
	
	// P0 for each state = Pinitial[state] * EmissionP(state, first element in the sequence) 
	for (i = 0; i < hmm->numStates; i++) {
		// by advancing numStates with each iteration, we land on the first
		// entry for each state:
		results[i*hmm->numStates] = getInitP(hmm, i) * getEmitP(hmm, i, sequence[0]);
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
			
			results[j*hmm->numStates+i] = sum * getEmitP(hmm, j, sequence[i]);
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
double getProbability(HmmStateRef hmm, uint *sequence, uint length) {
	uint i;
	double prob = 0.0;
	
	// call the forward algorithm, which returns a list of probs
	double *results = forwardAlgorithm(hmm, sequence, length);
	
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

int main(int argc, char const* argv[]) {
	uint i, j;
	
	uint states = 5;
	uint observations = 10;
	
	if (argc == 3) {
		states       = atoi(argv[1]);
		observations = atoi(argv[2]); 
	}
	
	fprintf(stderr, "Testing HMM with %d states and %d observations.\n", states, observations);
	
	/* test hmm with 10 states and 10 observations */
	HmmStateRef hmm = createHmm(states, observations);
		
	/* test backward algorithm */
	uint test[5] = {0, 1, 1, 0, 0};
			
	/* test backward algorithm */
	double *results = backwardAlgorithm(hmm, test, 5);
	
	fprintf(stdout, "Backward algorithm says:\n");
	for (i = 0; i < states; i++) {
		printf("%d: ", i);
		for (j = 0; j < 5; j++) {
			printf("%1.4lf   ", results[i * 5 + j]);
		}
		printf("\n");
	}
		
	// calling code must free result:
	free(results);
	
	/* examine it! */
	dumpModel(hmm);
	
	/* kill it! */
	releaseHmm(hmm);
	
	return 1;
}
