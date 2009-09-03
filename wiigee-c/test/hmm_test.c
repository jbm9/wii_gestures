#include "hmm.h"

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
	HmmStateRef hmm = hmm_new(states, observations);
		
	/* test backward algorithm */
	uint test[5] = {0, 1, 1, 0, 0};
			
	StateSequenceRef sequence = createStateSequence(test, 5);
			
	/* test backward algorithm */
	double *results = backwardAlgorithm(hmm, sequence);
	
	fprintf(stdout, "Backward algorithm says:\n");
	for (i = 0; i < states; i++) {
		printf("%d: ", i);
		for (j = 0; j < sequence->length; j++) {
			printf("%1.4lf   ", results[i * sequence->length + j]);
		}
		printf("\n");
	}
		
	// calling code must free result:
	free(results);
	
	/* examine it! */
	dumpModel(hmm);
	
	/* kill it! */
	hmm_free(hmm);
	
	return 0;
}
