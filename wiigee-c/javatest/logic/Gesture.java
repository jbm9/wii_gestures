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

import java.util.Vector;

/**
 * This class represents ONE movement trajectory in a
 * concrete instance.
 * 
 * @author Benjamin 'BePo' Poppinga
 */

public class Gesture implements Cloneable {

	/** The maximal acceleration this gesture has got. */
	private double maxacc;
	
	/** The minimal acceleration this gesture has got. */
	private double minacc;
	
	/** The complete trajectory as WiimoteAccelerationEvents
	 * as a vector. It's a vector because we don't want to
	 * loose the chronology of the stored events.
	 */
	Vector<double[]> data;

	/**
	 * Create an empty Gesture.
	 */
	public Gesture() {
		this.maxacc=Double.MIN_VALUE;
		this.data = new Vector<double[]>();
	}

	/** 
	 * Make a deep copy of another Gesture object.
	 * 
	 * @param original Another Gesture object
	 */
	/*
	public Gesture(Gesture original) {
		this.data = new Vector<WiimoteAccelerationEvent>();
		Vector origin = original.getData();
		for (int i = 0; i < origin.size(); i++) {
			this.add((WiimoteAccelerationEvent) origin.get(i));
		}
		this.maxacc=original.getMaxAcceleration();
		this.minacc=original.getMinAcceleration();
	}
	*/


	/**
	 * Adds a new acceleration event to this gesture.
	 * 
	 * @param event The WiimoteAccelerationEvent to add.
	 */
	public void add(double[] event) {
		this.data.add(event);
	}

	/**
	 * Returns the last acceleration added to this gesture.
	 * 
	 * @return the last acceleration event added.
	 */
	public double[] getLastData() {
		return (double[]) this.data.get(this.data.size() - 1);
	}

	/**
	 * Returns the whole chronological sequence of accelerations as
	 * a vector.
	 * 
	 * @return chronological sequence of accelerations.
	 */
	public Vector<double[]> getData() {
		return this.data;
	}
	
	public int getCountOfData() {
		return this.data.size();
	}
	
	public void setMaxAcceleration(double maxacc) {
		this.maxacc=maxacc;
	}

	public double getMaxAcceleration() {
		return this.maxacc;
	}
	
	public void setMinAcceleration(double minacc) {
		this.minacc=minacc;
	}
	
	public double getMinAcceleration() {
		return this.minacc;
	}
}
