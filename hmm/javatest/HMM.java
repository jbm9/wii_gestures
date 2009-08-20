
import java.text.*;
import java.util.Vector;

/**
 * This is a Hidden Markov Model implementation which internally provides
 * the basic algorithms for training and recognition (forward and backward
 * algorithm). Since a regular Hidden Markov Model doesn't provide a possibility
 * to train multiple sequences, this implementation has been optimized for this
 * purposes using some state-of-the-art technologies described in several papers.
 *
 * @author Benjamin 'BePo' Poppinga
 *
 */

public class HMM {
	/** The number of states */
	private int numStates;

	/** The number of observations */
	private int sigmaSize;

	/** The initial probabilities for each state: p[state] */
	public double pi[];

	/** The state change probability to switch from state A to
	 * state B: a[stateA][stateB] */
	public double a[][];

	/** The probability to emit symbol S in state A: b[stateA][symbolS] */
	public double b[][];

	/**
	 * Initialize the Hidden Markov Model in a left-to-right version.
	 * 
	 * @param numStates Number of states
	 * @param sigmaSize Number of observations
	 */
	public HMM(int numStates, int sigmaSize) {
		this.numStates = numStates;
		this.sigmaSize = sigmaSize;
		pi = new double[numStates];
		a = new double[numStates][numStates];
		b = new double[numStates][sigmaSize];
		this.reset();
	}
	
	/**
	 * Reset the Hidden Markov Model to the initial left-to-right values.
	 *
	 */
	private void reset() {
		int jumplimit = 2;
		
		// set startup probability
		pi[0] = 1;
		for(int i=1; i<numStates; i++) {
			pi[i] = 0;
		}
		
		// set state change probabilities in the left-to-right version
		// NOTE: i now that this is dirty and very static. :)
		for(int i=0; i<numStates; i++) {
			for(int j=0; j<numStates; j++) {
				if(i==numStates-1 && j==numStates-1) { // last row
					a[i][j] = 1.0;
				} else if(i==numStates-2 && j==numStates-2) { // next to last row
					a[i][j] = 0.5;
				} else if(i==numStates-2 && j==numStates-1) { // next to last row
					a[i][j] = 0.5;
				} else if(i<=j && i>j-jumplimit-1) {
					a[i][j] = 1.0/(jumplimit+1);
				} else {
					a[i][j] = 0.0;
				}
			}
		}
		
		
		// emission probability
		for(int i=0; i<numStates; i++) {
			for(int j=0; j<sigmaSize; j++) {
				b[i][j] = 1.0/(double)sigmaSize;
			}
		}
	}

	/**
	 * Trains the Hidden Markov Model with multiple sequences.
	 * This method is normally not known to basic hidden markov
	 * models, because they usually use the Baum-Welch-Algorithm.
	 * This method is NOT the traditional Baum-Welch-Algorithm.
	 * 
	 * If you want to know in detail how it works please consider
	 * my Individuelles Projekt paper on the wiigee Homepage. Also
	 * there exist some english literature on the world wide web.
	 * Try to search for some papers by Rabiner or have a look at
	 * Vesa-Matti Mäntylä - "Discrete Hidden Markov Models with
	 * application to isolated user-dependent hand gesture recognition". 
	 * 
	 */
	public void train(Vector<int[]> trainsequence) {

		double[][] a_new = new double[a.length][a.length];
		double[][] b_new = new double[b.length][b[0].length];
		
		// re calculate state change probability a
		for(int i=0; i<a.length; i++) {
			for(int j=0; j<a[i].length; j++) {	
				double zaehler=0;
				double nenner=0;
			
				for(int k=0; k<trainsequence.size(); k++) {
					this.reset();
					int[] sequence = trainsequence.elementAt(k);
					
					double[][] fwd = this.forwardProc(sequence);
					double[][] bwd = this.backwardProc(sequence);
					double prob = this.getProbability(sequence);
		
					double zaehler_innersum=0;
					double nenner_innersum=0;
					
					
					for(int t=0; t<sequence.length-1; t++) {
						zaehler_innersum+=fwd[i][t]*a[i][j]*b[j][sequence[t+1]]*bwd[j][t+1];
						nenner_innersum+=fwd[i][t]*bwd[i][t];
					}
					zaehler+=(1/prob)*zaehler_innersum;
					nenner+=(1/prob)*nenner_innersum;
				} // k
		
				a_new[i][j] = zaehler/nenner;
			} // j
		} // i
		
		// re calculate emission probability b
		for(int i=0; i<b.length; i++) { // zustaende
			for(int j=0; j<b[i].length; j++) {	// symbole
				double zaehler=0;
				double nenner=0;
			
				for(int k=0; k<trainsequence.size(); k++) {
					this.reset();
					int[] sequence = trainsequence.elementAt(k);
					
					double[][] fwd = this.forwardProc(sequence);
					double[][] bwd = this.backwardProc(sequence);
					double prob = this.getProbability(sequence);
		
					double zaehler_innersum=0;
					double nenner_innersum=0;
					
					
					for(int t=0; t<sequence.length-1; t++) {
						if(sequence[t]==j) {
							zaehler_innersum+=fwd[i][t]*bwd[i][t];
						}
						nenner_innersum+=fwd[i][t]*bwd[i][t];
					}
					zaehler+=(1/prob)*zaehler_innersum;
					nenner+=(1/prob)*nenner_innersum;
				} // k
		
				b_new[i][j] = zaehler/nenner;
			} // j
		} // i
	
		this.a=a_new;
		this.b=b_new;
	}
	
	/**
	 * Traditional Forward Algorithm.
	 * 
	 * @param o the observationsequence O
	 * @return Array[State][Time] 
	 * 
	 */
	private double[][] forwardProc(int[] o) {
		double[][] f = new double[numStates][o.length];
		for (int l = 0; l < f.length; l++) {
			f[l][0] = pi[l] * b[l][o[0]];
		}
		
		//o.length returns length of the first array dimension:
		for (int i = 1; i < o.length; i++) {
			for (int k = 0; k < f.length; k++) {
				double sum = 0;
				for (int l = 0; l < numStates; l++) {
					sum += f[l][i-1] * a[l][k];
				}
				f[k][i] = sum * b[k][o[i]];
			}
		}
		return f;
	}
	
	/**
	 * Returns the probability that a observation sequence O belongs
	 * to this Hidden Markov Model without using the bayes classifier.
	 * Internally the well known forward algorithm is used.
	 * 
	 * @param o observation sequence
	 * @return probability that sequence o belongs to this hmm
	 */
	public double getProbability(int[] o) {
		double prob = 0.0;
		double[][] forward = this.forwardProc(o);
		//	add probabilities
		for (int i = 0; i < forward.length; i++) { // for every state
			prob += forward[i][forward[i].length - 1];
		}
		return prob;
	}
	
	/**
	 * Backward algorithm.
	 * 
	 * @param o observation sequence o
	 * @return Array[State][Time]
	 */
	public double[][] backwardProc(int[] o) {
		int T = o.length;
		double[][] bwd = new double[numStates][T];
		/* Basisfall */
		for (int i = 0; i < numStates; i++)
			bwd[i][T - 1] = 1;
		/* Induktion */
		for (int t = T - 2; t >= 0; t--) {
			for (int i = 0; i < numStates; i++) {
				bwd[i][t] = 0;
				for (int j = 0; j < numStates; j++)
					bwd[i][t] += (bwd[j][t + 1] * a[i][j] * b[j][o[t + 1]]);
			}
		}
		return bwd;
	}

	

	/** 
	 * Prints everything about this model, including
	 * all values. For debug purposes or if you want
	 * to comprehend what happend to the model.
	 * 
	 */
	public void print() {
		DecimalFormat fmt = new DecimalFormat();
		fmt.setMinimumFractionDigits(5);
		fmt.setMaximumFractionDigits(5);
		for (int i = 0; i < numStates; i++)
			System.out.println("pi(" + i + ") = " + fmt.format(pi[i]));
		System.out.println();
		for (int i = 0; i < numStates; i++) {
			for (int j = 0; j < numStates; j++)
				System.out.print("a(" + i + "," + j + ") = "
						+ fmt.format(a[i][j]) + " ");
			System.out.println();
		}
		System.out.println();
		for (int i = 0; i < numStates; i++) {
			for (int k = 0; k < sigmaSize; k++)
				System.out.print("b(" + i + "," + k + ") = "
						+ fmt.format(b[i][k]) + " ");
			System.out.println();
		}
	}

	public double[][] getA() {
		return this.a;
	}
	
	public void setA(double[][] a) {
		this.a = a;
	}
	
	public double[][] getB() {
		return this.b;
	}
	
	public void setB(double[][] b) {
		this.b=b;
	}
	
	// testing some shit and whatnot:
	public static void main(String args[]) {
		int states = 5;
		int observations = 10;
		
		if (args.length == 2) {
			states = Integer.parseInt(args[0]);
			observations = Integer.parseInt(args[1]);
		}
		
		HMM hmm = new HMM(states, observations);
		hmm.print();
		
		int test[] = new int[5];
		test[0] = 0;
		test[1] = 1;
		test[2] = 1;
		test[3] = 0;
		test[4] = 0;
		
		System.out.println("oracle speaketh: "+hmm.getProbability(test));
	}
}
