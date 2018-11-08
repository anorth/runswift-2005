/*

   Copyright 2003 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
   redistribute it and/or modify it under the terms of the GNU General  
   Public License as published by the Free Software Foundation; either  
   version 2 of the License, or (at your option) any later version as  
   modified below.  As the original licensors, we add the following  
   conditions to that license:

   In paragraph 2.b), the phrase "distribute or publish" should be  
   interpreted to include entry into a competition, and hence the source  
   of any derived work entered into a competition must be made available  
   to all parties involved in that competition under the terms of this  
   license.

   In addition, if the authors of a derived work publish any conference  
   proceedings, journal articles or other academic papers describing that  
   derived work, then appropriate academic citations to the original work  
   must be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/*
 * Last modification background information
 * $Id: KalmanInfo2D.h 6497 2005-06-09 06:43:54Z alexn $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * A structure that stores data relevant for the information form Kalman filter
 * for tracking a two dimensional value. Originally developed for tracking
 * opponent robots. The information form of the Kalman filter stores variables
 * in a way that innovations are uncorrelated and updates are additive,
 * therefore we are able to use it for a decentralised sensor network such as
 * a team of aibos
 *
 **/

#ifndef KALMAN_INFO2D_h_DEFINED
#define KALMAN_INFO2D_h_DEFINED


#include <iostream>

using std::cout;
using std::endl;

#include "../share/Common.h"
#include "../share/minimalMatrix.h"
#include "Vector.h"
#include "gps.h"

class GPS;
struct SlimVisOb;
struct IFKUpdateData;


class KalmanInfo2D {
	public:

		KalmanInfo2D() {
		}

		// constructor that simply sets values and variances, assumes x & y
		// initially independent
		// note - g is a pointer to the gps being currently used by the dog
		KalmanInfo2D(GPS *g,
				double iniX,
				double iniY,
				double iniXVar,
				double iniYVar);

		virtual ~KalmanInfo2D() {
		}

		// returns a constant reference to the normal values
		// argument should be GLOBAL or LOCAL (wrt the dog)
		// note - vector cannot be altered
		const Vector &getPos(int context);

		// returns a constant reference to the normal covariance matrix
		// note - matrix cannot be altered
		const MMatrix2 &getCov();

		// returns the value of the given vector in this probability distribution
		double getProb(double x, double y);
		double getProb(const SlimVisOb &vob);
		double getNumSD(double x, double y);
		double getNumSD(const SlimVisOb &vob);

        double getHeadDiff(const SlimVisOb &vob);

		// sets the given argument with update information, then resets internal values
		void flushUpdate(IFKUpdateData &data);

		// returns a constant reference to the information update values
		// note - vector cannot be altered
		//      - only x and y values will be reliable
		const MVec2 &getUpdatePos() const;

		// returns a constant reference to the information update covariance matrix
		// note - matrix cannot be altered
		const MMatrix2 &getUpdateCov() const;

		// resets the update values - should be done once update values and
		// covariance is sent to rest of network
		void resetUpdates();

		// adds the given values and covariance information to current information
		void addUpdates(const MVec2 &upVal, const MMatrix2 &upCov);
		void addUpdates(const IFKUpdateData &data);

		// performs a information form kalman update on the data of the vob
		// weight provides a way of applying the observation to a certain
		// "degree", ie divide covariance matrix by this value
		void infoKalman(const SlimVisOb &vob, double weight = 1.0);

        // Reset's the position and covariance to the given values (global).
        // Used for ball-out, offline etc
        void setVal(double x, double y, double var);

		// performs a prediction update on the filter
		// just enlarges the variance by the given amount in each dimension
		void growVariance(double amount, double addx = 0.0, double addy = 0.0);

		// enters data for this object in the given char array
		// returns next index that can be used
		virtual int serialize(unsigned char *add, int base);

		// resets this objects data to that of the serialized object in
		// the given array
		virtual int unserialize(unsigned char *add, int base);

	protected:

		GPS *gps;           // the current gps being used by the dogs, used to
		// determine expected measurements etc

		MVec2 val;          // x and y coordinates


	private:

		Vector globalVal; // x and y coordinates in global coordinates

		Vector localVal;  // x and y in local coordinates

		MMatrix2 cov;        // covariance matrix

		MVec2 infoVal;       // x and y coordinates in information form

		MMatrix2 infoCov;    // covariance matrix in information form

		MVec2 updateVal;     // information filter updates since last transmission

		MMatrix2 updateCov;  // information filter covariance updates since last transmission

		bool valuesCurrent; // true if values (val & cov) are up to date with the information
		// form values

		bool globalCurrent; // true if global vector is up to date with filter

		bool localCurrent;  // true if local vector is up to date with filter

        // sets the information form data such that it is consistent with the
        // normal data
        inline void normal2Info() {
            infoCov.isInverse(cov);
            infoVal.isMult(infoCov, val);
            valuesCurrent = true;
        }


        // sets the normal data such that it is consistent with the information
        // form data
        inline void info2Normal() {
            cov.isInverse(infoCov);
            val.isMult(cov, infoVal);

            valuesCurrent = true;
            globalCurrent = false;
            localCurrent = false;
        }

		// sets the global data such that it is consistent with the filter
        inline void normal2Global() {
            globalVal.setVector(vCART, val(0, 0), val(1, 0));
            globalCurrent = true;
        }

		// sets the local data such that it is consistent with the filter
        inline void normal2Local() {
            double dx = val(0, 0) - gps->self().pos.x;
            double dy = val(1, 0) - gps->self().pos.y;
            localVal.setVector(vCART, dx, dy);
            localVal.rotate(90 - (gps->self()).h);
            localCurrent = true;
        }

};


// Returns the data of an observation, in a global context
//    - pos - self position
//    - cov - covariance of self position
//    - dist - distance of observation
//    - head - heading of observation
//    - distVar - variance of distance measurement
//    - headVar - variance of heading measurement
//    - obsPos - the global position of the observation (return)
//    - obsCov - the covariance of the global position of the observation (return)
void getGlobalObservation(const MVec3 &pos,
		const MMatrix3 &cov,
		double dist,
		double head,
		double distVar,
		double headVar,
		MVec2 &obsPos,
		MMatrix2 &obsCov);


// apply the given observation to the array of KalmanInfo2D objects in the
// simplest way, find the best fitting object and update that information
// form filter
void applyKalmanUpdateSimple(KalmanInfo2D **array,
		int num,
		const SlimVisOb &vob);


// apply the given observation to the array of KalmanInfo2D objects in the
// weighted average way, apply the observation to each filter relative to
// the probability that the observation
void applyKalmanUpdateWA(KalmanInfo2D **array, int num, const SlimVisOb &vob);


// apply the given observation to the array of KalmanInfo2D objects in the
// hybrid way, which is a combination of best fit and weighted average ways.
// a fraction of the pulling power is reserved for the best fit and the rest
// is spread around the other entities evenly 
void applyKalmanUpdateHybrid(KalmanInfo2D **array,
		int num,
		const SlimVisOb &vob);


void applyKalmanMultiUpdatesHybrid(KalmanInfo2D **array,
		int num,
		const SlimVisOb vob[],
		int vobMinInd,
		int vobMaxInd);

#endif // KALMAN_INFO2D_h_DEFINED
