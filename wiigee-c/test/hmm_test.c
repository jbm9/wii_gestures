#include "hmm.h"
#include <math.h>

void test_Moss_Q520() {
  // http://www.indiana.edu/~iulg/moss/hmmcalculations.pdf
  HmmStateRef hmm = hmm_new(2, 2); // 2 states, 2 obs
  setInitP(hmm, 0, 0.85);
  setInitP(hmm, 1, 0.15);

  setChangeP(hmm, 0, 0, 0.3);
  setChangeP(hmm, 0, 1, 0.7);
  setChangeP(hmm, 1, 0, 0.1);
  setChangeP(hmm, 1, 1, 0.9);

  setEmitP(hmm, 0, 0, 0.4);
  setEmitP(hmm, 0, 1, 0.6);
  setEmitP(hmm, 1, 0, 0.5);
  setEmitP(hmm, 1, 1, 0.5);

  int ABBA[] = { 0, 1, 1, 0 };
  StateSequenceRef seq = createStateSequence(ABBA, 4);

  double *f = forwardAlgorithm(hmm, seq);

  double expected[] = { 
    0.34, 
    0.066,
0.02118,
0.00625,

0.075,
0.155,
0.09285,
0.04919 };

  for(int i = 0; i < 2*4; i++) {
    // printf(" %f, ", f[i]);
    if ( fabs( (expected[i]-f[i]) / expected[i] ) > 0.05 ) {
      printf("ERROR: (alpha) More than 5%% relative error at offset %d (expected=%f, got=%f)\n", i, expected[i], f[i]);
    }
  }
  printf("\n");
  free(f);


  double *b = backwardAlgorithm(hmm, seq);

  double expected_backwards[] = {
    0.13315,
    0.2561,
    0.47,
    1.0,

    0.12729,
    0.2487,
    0.49,
    1.0,
  };

  for(int i = 0; i < 2*4; i++) {
    // printf(" %f, ", b[i]);
    if ( fabs( (expected_backwards[i]-b[i]) / expected_backwards[i] ) > 0.05 ) {
      printf("ERROR: (beta) More than 5%% relative error at offset %d (expected=%f, got=%f)\n", i, expected_backwards[i], b[i]);
    }
  }
  printf("\n");
  free(b);

  double prob = getProbability(hmm, seq);
  double expected_prob = 0.05544;


  if ( fabs( (expected_prob-prob) / expected_prob ) > 0.05 ) {
    printf("ERROR: (P) More than 5%% relative error (expected=%f, got=%f)\n", expected_prob, prob);
  }


  double gamma = 0.0;
  double expected_gamma = 0.0;

  expected_gamma = 0.28271;
  gamma = hmm_gamma(hmm, seq, 1, 0, 0);
  if ( fabs( (expected_prob-prob) / expected_prob ) > 0.05 ) {
    printf("ERROR: (g) More than 5%% relative error (expected=%f, got=%f)\n", expected_prob, prob);
  }

  expected_gamma = 0.53383;
  gamma = hmm_gamma(hmm, seq, 1, 0, 1);
  if ( fabs( (expected_prob-prob) / expected_prob ) > 0.05 ) {
    printf("ERROR: (g) More than 5%% relative error (expected=%f, got=%f)\n", expected_prob, prob);
  }


  expected_gamma = 0.04584;
  gamma = hmm_gamma(hmm, seq, 3, 0, 0);
  if ( fabs( (expected_prob-prob) / expected_prob ) > 0.05 ) {
    printf("ERROR: (g) More than 5%% relative error (expected=%f, got=%f)\n", expected_prob, prob);
  }


  expected_gamma = 0.75365;
  gamma = hmm_gamma(hmm, seq, 3, 1, 1);
  if ( fabs( (expected_prob-prob) / expected_prob ) > 0.05 ) {
    printf("ERROR: (g) More than 5%% relative error (expected=%f, got=%f)\n", expected_prob, prob);
  }



  double expected_delta = 0.0;
  double delta =  0.0;

  expected_delta = 0.81654;
  delta = hmm_delta(hmm, seq, 1, 0);
  if ( fabs( (expected_prob-prob) / expected_prob ) > 0.05 ) {
    printf("ERROR: (d) More than 5%% relative error (expected=%f, got=%f)\n", expected_prob, prob);
  }

  expected_delta = 0.11273;
  delta = hmm_delta(hmm, seq, 4, 0);
  if ( fabs( (expected_prob-prob) / expected_prob ) > 0.05 ) {
    printf("ERROR: (d) More than 5%% relative error (expected=%f, got=%f)\n", expected_prob, prob);
  }

  expected_delta = 0.18366;
  delta = hmm_delta(hmm, seq, 1, 1);
  if ( fabs( (expected_prob-prob) / expected_prob ) > 0.05 ) {
    printf("ERROR: (d) More than 5%% relative error (expected=%f, got=%f)\n", expected_prob, prob);
  }

  expected_delta = 0.88727;
  delta = hmm_delta(hmm, seq, 4, 1);
  if ( fabs( (expected_prob-prob) / expected_prob ) > 0.05 ) {
    printf("ERROR: (d) More than 5%% relative error (expected=%f, got=%f)\n", expected_prob, prob);
  }

}

int test_round_trip() {
  uint states = 2;
  uint observations = 2;

  uint seq_len = 10;

  uint seq_1[] = { 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, };
  uint seq_2[] = { 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, };
  uint seq_3[] = { 1, 0, 1, 0, 1,  0, 1, 0, 1, 0, };

  int n_hmms = 2;

  HmmStateRef *hmms = malloc(sizeof(HmmStateRef) * n_hmms);
  for(int i = 0; i < n_hmms; i++)
    hmms[i] = hmm_new(states, observations);

  StateSequenceRef seq;

  for(int i = 0; i < n_hmms; i++) {
    printf("===== HMM %d ======\n", i);

    dumpModel(hmms[i]);
  }

  printf("------------ TRAIN 0 --------------\n");

  seq = createStateSequence(seq_1, seq_len);
  hmm_train(hmms[0], &seq, 1);
  releaseStateSequence(seq);

  for(int i = 0; i < n_hmms; i++) {
    printf("===== HMM %d ======\n", i);

    dumpModel(hmms[i]);
  }

  printf("------------ TRAIN 1 --------------\n");


  seq = createStateSequence(seq_2, seq_len);
  hmm_train(hmms[1], &seq, 1);
  releaseStateSequence(seq);

  for(int i = 0; i < n_hmms; i++) {
    printf("===== HMM %d ======\n", i);

    dumpModel(hmms[i]);
  }
  printf("------------ GOGOGO --------------\n");

  float p;

  for(int i = 0; i < n_hmms; i++) {
    printf("===== HMM %d ======\n", i);

    dumpModel(hmms[i]);

    printf("\n\n");


    seq = createStateSequence(seq_1, seq_len);
    p = getProbability(hmms[i], seq);
    printf("seq_1 : P(%d) = %f\n", i, p);
    releaseStateSequence(seq);

    seq = createStateSequence(seq_2, seq_len);
    p = getProbability(hmms[i], seq);
    printf("seq_2 : P(%d) = %f\n", i, p);
    releaseStateSequence(seq);

    seq = createStateSequence(seq_3, seq_len);
    p = getProbability(hmms[i], seq);
    printf("seq_3 : P(%d) = %f\n", i, p);
    releaseStateSequence(seq);
  }
}

int main(int argc, char const* argv[]) {
  test_Moss_Q520();
  test_round_trip();
  return 0;


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
