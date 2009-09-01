/*
 * wiigee - accelerometerbased gesture recognition
 * Copyright (C) 2007, 2008 Benjamin Poppinga
 * 
 * Developed at University of Oldenburg
 * Contact: benjamin.poppinga@informatik.uni-oldenburg.de
 *
 * This file is part of wiigee.
 *
 * wiigee is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

package logic;

import logic.Gesture;
import java.util.HashMap;
import java.util.Vector;

/** 
 * This class implements a quantization component. In this case
 * a k-mean-algorithm is used. In this case the initial values of
 * the algorithm are ordered as two intersected circles, representing
 * an abstract globe with k=14 elements. As a special feature the radius
 * of this globe would be calculated dynamically before the training of
 * this component.
 * 
 * @author Benjamin 'BePo' Poppinga
 */
public class Quantizer {

	public boolean debug = false;
	
	/** This is the initial radius of this model. */
	private double radius;
	
	/** Number of states from the following Hidden Markov Model */
	private int states;
	
	/** The representation of the so called Centeroids */
	private HashMap<Integer, double[]> map = new HashMap<Integer, double[]>();

	/**
	 * Initialize a empty quantizer. The states variable is
	 * necessary since some algorithms need this value to calculate their
	 * values correctly.
	 * 
	 * @param states number of hidden markov model states
	 */
	public Quantizer(int states) {
		this.states=states;
	}
	
	/**
	 * Trains this Quantizer with a specific gesture. This means that the
	 * positions of the centeroids would adapt to this training gesture.
	 * In our case this would happen with a summarized virtual gesture,
	 * containing all the other gestures.
	 * 
	 * @param gesture the summarized virtual gesture
	 */
	public void trainCenteroids(Gesture gesture) {
		Vector<double[]> data = gesture.getData();
		double pi = Math.PI;
		this.radius=(gesture.getMaxAcceleration()+gesture.getMinAcceleration())/2;
		if (debug) System.out.println("Using radius: "+this.radius);
		
		// x , z , y
			if (this.map.isEmpty()) {
					this.map.put(0, new double[] {this.radius, 0.0, 0.0});
					this.map.put(1, new double[] {Math.cos(pi/4)*this.radius, 0.0, Math.sin(pi/4)*this.radius});
					this.map.put(2, new double[] {0.0, 0.0, this.radius});
					this.map.put(3, new double[] {Math.cos(pi*3/4)*this.radius, 0.0, Math.sin(pi*3/4)*this.radius});
					this.map.put(4, new double[] {-this.radius, 0.0, 0.0});
					this.map.put(5, new double[] {Math.cos(pi*5/4)*this.radius, 0.0, Math.sin(pi*5/4)*this.radius});
					this.map.put(6, new double[] {0.0, 0.0, -this.radius});
					this.map.put(7, new double[] {Math.cos(pi*7/4)*this.radius, 0.0, Math.sin(pi*7/4)*this.radius});
					
					
					this.map.put(8, new double[] {0.0, this.radius, 0.0});
					this.map.put(9, new double[] {0.0, Math.cos(pi/4)*this.radius, Math.sin(pi/4)*this.radius});
					// this.map.put(2, new double[] {0.0, 0.0, this.radius}); // doppelt
					this.map.put(10, new double[] {0.0, Math.cos(pi*3/4)*this.radius, Math.sin(pi*3/4)*this.radius});
					this.map.put(11, new double[] {0.0, -this.radius, 0.0});
					this.map.put(12, new double[] {0.0, Math.cos(pi*5/4)*this.radius, Math.sin(pi*5/4)*this.radius});
					// this.map.put(6, new double[] {0.0, 0.0, -this.radius}); // doppelt
					this.map.put(13, new double[] {0.0, Math.cos(pi*7/4)*this.radius, Math.sin(pi*7/4)*this.radius});
			}

			if (debug) {
				System.out.printf("Initial centroids:\n");
				for (int i = 0; i < this.map.size(); i++) {
				    System.out.printf("   %2d:  %9.5f  %9.5f  %9.5f\n",
					i,
					this.map.get(i)[0],
					this.map.get(i)[1],
					this.map.get(i)[2]);
				}
				System.out.printf("\n");
			}

			int[][] g_alt = new int[this.map.size()][data.size()];
			int[][] g = new int[this.map.size()][data.size()];

			do {
				// Derive new Groups...
				if (debug) System.out.printf("Still in the loop\n");
				g_alt = this.copyarray(g);
				g = this.deriveGroups(gesture);

				// calculate new centeroids
				for (int i = 0; i < this.map.size(); i++) {
					if (debug) System.out.printf("iter i %d\n", i);
					double zaehlerX = 0;
					double zaehlerY = 0;
					double zaehlerZ = 0;
					int nenner = 0;
					for (int j = 0; j < data.size(); j++) {
						if (g[i][j] == 1) {
							zaehlerX += data.elementAt(j)[0];
							zaehlerY += data.elementAt(j)[1];
							zaehlerZ += data.elementAt(j)[2];
							nenner++;
						}
					}
					if (nenner > 1) { // nur wenn der nenner>0 oder >1??? ist muss was
						// geaendert werden
						if (debug) System.out.println("Setze neuen Centeroid!");
						double[] newcenteroid = { (zaehlerX / (double) nenner),
								(zaehlerY / (double) nenner),
								(zaehlerZ / (double) nenner) };
						this.map.put(i, newcenteroid);
						//System.out.println("Centeroid: "+i+": "+newcenteroid[0]+":"+newcenteroid[1]);
						if (debug) System.out.printf("Centeroid: %d: %5.1f  %5.1f  %5.1f\n",
								i, newcenteroid[0], newcenteroid[1], newcenteroid[2]);
					}
				} // new centeroids

				if (debug) {
					System.out.printf("old g:\n");
					for (int row = 0; row < this.map.size(); row++) {
					    for (int col = 0; col < data.size(); col++) {
						System.out.printf("%d ", g_alt[row][col]);
					    }
					    System.out.printf("\n");
					}
					System.out.printf("new g:\n");
					for (int row = 0; row < this.map.size(); row++) {
					    for (int col = 0; col < data.size(); col++) {
						System.out.printf("%d ", g[row][col]);
					    }
					    System.out.printf("\n");
					}
					System.out.printf("\n");
				}


			} while (!equalarrays(g_alt, g));
			
			// Debug: Printout groups
			
			if (debug) {
				System.out.println("Final g output:");
				int n = this.states;
				for (int i = 0; i < n; i++) {
					for (int j = 0; j < data.size(); j++) { 
						System.out.print(g[i][j] + "|"); 
					}
					System.out.println(""); 
				}
				System.out.println("Final map value:");
				for (int i = 0; i < this.map.size(); i++) {
				    System.out.printf("   %2d:  %5.1f  %5.1f  %5.1f\n",
					i,
					this.map.get(i)[0],
					this.map.get(i)[1],
					this.map.get(i)[2]);
				}
			}
			if (debug) System.out.printf("trainCenteroids returning\n");
	}


	/**
	 * This methods looks up a Gesture to a group matrix, used
	 * by the k-mean-algorithm (traincenteroid method) above.
	 * 
	 * @param gesture the gesture
	 */
	public int[][] deriveGroups(Gesture gesture) {
		Vector<double[]> data = gesture.data;
		int[][] groups = new int[this.map.size()][data.size()];

		if (debug) System.out.printf("\nderiveGroups\n");

		// Calculate cartesian distance
		double[][] d = new double[this.map.size()][data.size()];
		double[] curr = new double[3];
		double[] vector = new double[3];
		for (int i = 0; i < this.map.size(); i++) { // zeilen
			double[] ref = this.map.get(i);
			if (debug) System.out.print("|");
			for (int j = 0; j < data.size(); j++) { // spalten

				curr[0] = data.elementAt(j)[0];
				curr[1] = data.elementAt(j)[1];
				curr[2] = data.elementAt(j)[2];

				vector[0] = ref[0] - curr[0];
				vector[1] = ref[1] - curr[1];
				vector[2] = ref[2] - curr[2];
				d[i][j] = Math.sqrt((vector[0] * vector[0])
						+ (vector[1] * vector[1]) + (vector[2] * vector[2]));
				//System.out.print(d[i][j] + "|");
				if (debug) System.out.printf("%5.0f |", d[i][j]);
			}
			if (debug) System.out.println("");
		}
		if (debug) System.out.println("");

		// look, to which group a value belongs
		for (int j = 0; j < data.size(); j++) {
			double smallest = Double.MAX_VALUE;
			int row = 0;
			for (int i = 0; i < this.map.size(); i++) {
				if (d[i][j] < smallest) {
					smallest = d[i][j];
					row = i;
				}
				groups[i][j] = 0;
			}
			groups[row][j] = 1; // guppe gesetzt
		}

		// Debug output
		for (int i = 0; i < groups.length; i++) { // zeilen
			for (int j = 0; j < groups[i].length; j++) {
				if (debug) System.out.print(groups[i][j] + "|");
			}
			if (debug) System.out.println("");
		}

		return groups;

	}


	/**
	 * With this method you can transform a gesture to a discrete
	 * symbol sequence with values between 0 and granularity (number of
	 * observations).
	 * 
	 * @param gesture Gesture to get the observationsequence to.
	 */
	public int[] getObservationSequence(Gesture gesture) {
		if (debug) System.out.println("getObservationSequence starting");
		int[][] groups = this.deriveGroups(gesture);
		Vector<Integer> sequence = new Vector<Integer>();

		if (debug) System.out.println("Visible symbol sequence:");

		for (int j = 0; j < groups[0].length; j++) { // spalten
			for (int i = 0; i < groups.length; i++) { // zeilen
				if (groups[i][j] == 1) {
					if (debug) System.out.println(i);
					sequence.add(i);
					break;
				} else {
					if (debug) System.out.printf("skipping groups[%d][%d] == %d)\n", i, j, groups[i][j]);
				}
			}
		}
		
		// die sequenz darf nicht zu kurz sein... mindestens so lang
		// wie die anzahl der zust??nde. weil sonst die formeln nicht klappen.
		// english: this is very dirty! it have to be here because if not
		// too short sequences would cause an error. i've to think about a
		// better resolution than copying the old value a few time.
		if (sequence.size()<this.states)
			if (debug) System.out.printf("padding out to %d\n", this.states);

		while(sequence.size()<this.states) {
			sequence.add(sequence.elementAt(sequence.size()-1));
			if (debug) System.out.println(sequence.elementAt(sequence.size()-1));
		}
		
		int[] out = new int[sequence.size()];
		for(int i=0; i<sequence.size(); i++) {
			out[i] = sequence.elementAt(i);
		}
		
		if (debug) System.out.printf("returning\n\n\n");
		return out;
	}

	/**
	 * Prints out the current centeroids-map.
	 * Its for debug or technical interests.
	 */
	public void printMap() {
		System.out.println("Centeroids:");
		for (int i = 0; i < this.map.size(); i++) {
			System.out.println(i + ". :" + this.map.get(i)[0] + ":"
					+ this.map.get(i)[1] + ":" + this.map.get(i)[2]);
		}
	}
	
	/**
	 * Function to deepcopy an array.
	 */
	private int[][] copyarray(int[][] alt) {
		int[][] neu = new int[alt.length][alt[0].length];
		for (int i = 0; i < alt.length; i++) {
			for (int j = 0; j < alt[i].length; j++) {
				neu[i][j] = alt[i][j];
			}
		}
		return neu;
	}

	/**
	 * Function to look if the two arrays containing
	 * the same values.
	 */
	private boolean equalarrays(int[][] one, int[][] two) {
		for (int i = 0; i < one.length; i++) {
			for (int j = 0; j < one[i].length; j++) {
				if (!(one[i][j] == two[i][j])) {
					return false;
				}
			}
		}
		return true;
	}
}
