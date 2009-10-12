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

/* This initializes our HMM to be a L-R HMM.  See Rabiner 1990 p266.
*/
void resetHmm(HmmStateRef hmm) {
	int i, j;

  setInitP(hmm, 0, 1.0); // must start in first state in L-R HMM

  // Set up an upper-diagonal transition matrix, with each row containing an
  // equal probability of jumping to any state to the right of the state
  // corresponding to the current row.  See the above reference to understand
  // this; it's pretty straightforward with a diagram.  -jbm

  for (i = 0; i < hmm->numStates; i++) {
    if (i > 0) setInitP(hmm, i, 0.0); // L-R constraint: p_i(0) = 0.0, i!=0

    for (j = 0; j < i; j++)
			setChangeP(hmm, i, j, 0.0); // L-R constraint

    for (j = i; j < hmm->numStates; j++) // equal opportunity
			setChangeP(hmm, i, j, 1.0/(hmm->numStates-i));
  }

	/* setup the emit probabilities */
	for (i = 0; i < hmm->numStates; i++) {
		for (j = 0; j < hmm->numObservations; j++) {
      setEmitP(hmm, i, j, 1.0f/(hmm->numObservations));
		}
	}
  return;
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

  fprintf(stdout, "nStates=%d, nObs=%d\n", hmm->numStates, hmm->numObservations);

	fprintf(stdout, "Initial Probabilites: [ ");
	for (i = 0; i < hmm->numStates - 1; i++) {
		fprintf(stdout, "%1.3f, ", hmm->p_initial[i]);
	}
	fprintf(stdout, "%1.3f ]\n", hmm->p_initial[i]);

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
		fprintf(stdout, "state%d:\t", i);
		for (j = 0; j < hmm->numObservations; j++ ) {
			fprintf(stdout, "%1.3f\t", getEmitP(hmm, i,j));
		}
		fprintf(stdout, "\n");
	}
}

//#pragma mark -
//#pragma mark "logic"

void hmm_train_bw(HmmStateRef hmm, StateSequenceRef Y) {
  int T = Y->length;

  double *alpha = forwardAlgorithm(hmm, Y);
  double *beta = backwardAlgorithm(hmm, Y);

  double P_Y = getProbability(hmm, Y);

  double *pi_star = (double*)malloc(sizeof(double) * hmm->numStates);
  for(int j = 0; j < hmm->numStates; j++) {
    pi_star[j] = (alpha[j*T] * beta[j*T])/P_Y;
  }
}

/*
 * param sequences: an array of 'num' sequences with which to train the model.
 *
 * Rabiner 1990 p273
 */
void hmm_train(HmmStateRef hmm, StateSequenceRef* sequences, uint num) {
	assert(hmm && sequences && num > 0);
	
	uint i, j, k, n;
	StateSequenceRef sequence;
	double numer, denom;
	double numer_inner, denom_inner;
	double *forward, *backward;
	double prob;
	
	double *change_new = (double*)malloc(sizeof(double) * hmm->numStates * hmm->numStates);
	double *emit_new   = (double*)malloc(sizeof(double) * hmm->numStates * hmm->numObservations);

	// recalculate the state change probabilities:

    numer = denom = 0.0;

    // recompute transition matrix
    for (i = 0; i < hmm->numStates; i++) {
      for (j = 0; j < hmm->numStates; j++) {
        numer = denom = 0.0;
        for (k = 0; k < num; k++) {
          //resetHmm(hmm); // of this, I am suspect
				
          // grab the k'th sequence:
          sequence = sequences[k];
				
          // run the forward and backward algorithms on this sequence
          forward  = forwardAlgorithm(hmm, sequence);
          backward = backwardAlgorithm(hmm, sequence);
          prob     = getProbability(hmm, sequence);
          int N = sequence->length;

          numer_inner = denom_inner = 0.0;
          for (n = 0; n < N - 1; n++) {
					
            numer_inner += (forward[i * N + n] *
                getChangeP(hmm, i, j) *
                getEmitP(hmm, j, sequence->states[n+1]) *
                backward[j * N + (n+1)]);
								
            denom_inner += (forward[i * N + n] *
                backward[i * N + n]);
          }
				
          numer += (1.0/prob) * numer_inner;
          denom += (1.0/prob) * denom_inner;

          // they are my responsibility:
          free(forward);
          free(backward);
        }
        change_new[i * hmm->numStates + j] = numer / denom;
      }

      // And scale it to unity per-origin state
      double total = 0.0;
      for(j = 0; j < hmm->numStates; j++)
        total += change_new[i * hmm->numStates + j];
      for(j = 0; j < hmm->numStates; j++)
        change_new[i * hmm->numStates + j] /= total;

    }

    // recompute emission matrix
    numer_inner = denom_inner = 0.0;
    numer = denom = 0.0;
    for (int l = 0; l < hmm->numObservations; l++) {
      for (j = 0; j < hmm->numStates; j++) {
        numer = denom = 0.0;
        for (k = 0; k < hmm->numObservations; k++) {
          forward  = forwardAlgorithm(hmm, sequence);
          backward = backwardAlgorithm(hmm, sequence);
          prob     = getProbability(hmm, sequence);

          int N = sequence->length;

          numer_inner = denom_inner = 0.0;

          for (n = 0; n < N; n++) {
            denom_inner += forward[j * N + n] * backward[j * N + n];
            if (sequence->states[n] == l)
              numer_inner += forward[j * N + n] * backward[j * N + n];
          }

        printf("   emit: j=%d -> l=%d, k=%d = %f/%f\n", j, l, k, numer_inner, denom_inner);
          numer += (1.0/prob) * numer_inner;
          denom += (1.0/prob) * denom_inner;

          // they are my responsibility:
          free(forward);
          free(backward);
        }
        printf("   emit: j=%d -> l=%d = %f/%f\n", j, l, numer, denom);
        emit_new[hmm->numObservations*j + l] = numer / denom;
      }
    }

    for(j = 0; j < hmm->numStates; j++) {
      double total = 0.0;
      for (int l = 0; l < hmm->numObservations; l++)
        total += emit_new[hmm->numObservations*j + l];

      for (int l = 0; l < hmm->numObservations; l++)
        emit_new[hmm->numObservations*j + l] /= total;
    }

  printf("\nBEFORE TRAIN\n");
  dumpModel(hmm);

	/* dump the old */
	free(hmm->p_change);
	free(hmm->p_emit);
	
	/* lovingly embrace the new */
	hmm->p_change = change_new;
	hmm->p_emit   = emit_new;

  printf("\nAFTER TRAIN\n");
  dumpModel(hmm);

  printf("\n\n");
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
		results[i*length] = getInitP(hmm, i) * 
		                            getEmitP(hmm, i, sequence->states[0]);
	}
	
	// over all the symbols in the sequence:
	for (i = 1; i < length; i++) {
		
		// over all the states in the model:
		for (j = 0; j < hmm->numStates; j++) {
			
			double sum = 0.0;
			
			// over all the states in the model:
			for (k = 0; k < hmm->numStates; k++) {
				
				// XXX: this is admittedly nasty:
				sum += results[(k*length)+(i-1)] * getChangeP(hmm, k, j);
			}
			
			results[j*length+i] = sum * getEmitP(hmm, j, sequence->states[i]);
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
		prob += results[i*sequence->length + (length-1)];
	}
	
	/* clean up after "forward" */
	free(results);
	
	return prob;
}

double hmm_gamma(HmmStateRef hmm, StateSequenceRef Y, int j, int state1, int state2) {
  double *alpha = forwardAlgorithm(hmm, Y);
  double *beta = backwardAlgorithm(hmm, Y);
  double P_Y = getProbability(hmm, Y);

  int T = Y->length;

  // gamma(Y,j,s,t) = alpha(Y,j,s)*go(s,t)*out(t, A_(j+1))*beta(Y,j+1,t) / P(Y)
  // gamma(Y,j,s,t) = alpha(Y,j,i)*a(i,j)*b(j, Y[+1])*beta(y,j+1,t) / P(Y)

  double gamma = alpha[state1*T+j] * getChangeP(hmm, state1, state2) * getEmitP(hmm, state2, Y->states[j]) * beta[state2*T+j] / P_Y;

  free(beta);
  free(alpha);

  return gamma;
}

double hmm_delta(HmmStateRef hmm, StateSequenceRef Y, int j, int s) {
  // This is the probability of an analyzed word in A(y) that the jth state is s.

  double sum = 0.0;

  for(int u = 0; u < hmm->numStates; u++) {
    sum += hmm_gamma(hmm, Y, j, s, u);
  }

  return sum;
}


