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
 * $$
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * A subclass of KalmanInfo2D that also tracks velocity of an object
 *
 **/

#ifndef KI2D_WITH_VELOCITY_h_DEFINED
#define KI2D_WITH_VELOCITY_h_DEFINED

#include "KalmanInfo2D.h"

enum {
    HISTORY_LENGTH = 10 // how long a history to average over
};


class KI2DWithVelocity : public KalmanInfo2D {

    /* This does not do what you think it does.
     * In 2005 we discovered that a Kalman filter is not suitable for tracking
     * ball velocity. We need a constant-variance type filter giving a 
     * fairly constant "reaction time" to observations and not becoming
     * too sensitive to observations after a long period of little information.
     *
     * Thus the body of this class has been replaced with a simple running
     * average over a fixed number of observations. This emulates a filter
     * with fixed variance (and fixed variance observations) but without
     * doing all the matrix math of the Kalman filter.
     *
     * This still uses a KalmanInfo2D in the usual way for tracking position.
     */

    public:

        /* Initialises the velocity (to zero) and underlying KI2D to
         * posn (iniX, iniY), var (iniXVar, iniYVar)
         */
        KI2DWithVelocity(GPS *g,
                        double iniX,
                        double iniY,
                        double iniXVar,
                        double iniYVar);


		virtual ~KI2DWithVelocity() { }
        
        // performs a information form kalman update on the data of the vob for
        // position and then if appropriate performs a normal kalman filter
        // update on velocity
		void infoKalman(const SlimVisOb &vob, bool updateVelocity);

		// shift the velocity history according to the dog's self movement
		void shiftLastBallPos(double, double, double);
	

        // sets the ball's position, velocity and variance to the given
        // values with zero velocity. Used for ball-out placement etc
        void setVal(double x, double y, double var, double dx=0, double dy=0);

		// returns a constant reference to the normal values of velocity
		// argument should be GLOBAL or LOCAL (wrt the dog)
		// note - vector cannot be altered
		const Vector &getVelocity(int context);
        
		// performs a prediction update on the filter
		// just enlarges the variance by the given amount in each dimension.
        // If visual is false then velocity is reduced
		void growVariance(double amount);
        
    private:
        list<Vector> history;    // HISTORY_LENGTH most recent observations
        double lastx, lasty;    // ball local pos
        int framesSinceObs;     // count frames between observations
	static const Vector INVALID_VEL; // flag for invalid velocity
        Vector localVel;        // local velocity for returning
	Vector globalVel;       // global velocity for returning
	Vector lastValidVel;    // the last velocity that we seen
   
#if 0
	public:

        // constructor that simply invokes KalmanInfo2D constructor and
        // initialises other data
        KI2DWithVelocity(GPS *g,
				double iniX,
				double iniY,
				double iniXVar,
				double iniYVar,
				double iniVX,
				double iniVY,
				double iniVXVar,
				double iniVIVar);

		virtual ~KI2DWithVelocity() {
		}

        // performs a information form kalman update on the data of the vob for
        // position and then if appropriate performs a normal kalman filter
        // update on velocity
		void infoKalman(const SlimVisOb &vob, bool updateVelocity);

		// returns a constant reference to the normal values of velocity
		// argument should be GLOBAL or LOCAL (wrt the dog)
		// note - vector cannot be altered
		const Vector &getVelocity(int context);

		// returns a constant reference to the covariance of the velocity
		// or copies of matrices in other contexts
		const MMatrix2 &getVelCov();
		MMatrix2 getVCovLocal(); // local coordinates 
		MMatrix2 getVCovBall();  // coordinates wrt ball y->robot

		// returns the decaying sum of the innovation vectors
		double getIVLength();

		double getNumSDVel(double vx, double vy);

		// performs a prediction update on the filter
		// just enlarges the variance by the given amount in each dimension.
        // If visual is false then velocity is reduced
		void growVariance(double amount, double velAmount, bool visual);

		// enters data for this object in the given char array
		// returns next index that can be used
		virtual int serialize(unsigned char *add, int base);

		// resets this objects data to that of the serialized object in
		// the given array
		virtual int unserialize(unsigned char *add, int base);

	private:

		Vector gVelVal;     // x and y velocity vector in global coordinates

		Vector lVelVal;     // x and y velocity vector in local coordinates

		MVec2 velVal;           // x and y velocity vector coordinates used for calculations (global)

		MMatrix2 velCov;        // velocity covariance matrix

		bool velGlobalCur;    // true if global vector is up to date with filter

		bool velLocalCur;   // true if local vector is up to date with filter

		double ivLength;         // decaying sum of the innovation vector length

		// information for the last sight of the ball
		double lastDist;   // last distance to the ball
		double lastHead;   // last heading to the ball
		double lastDistVar;  // last distance variance
		double lastHeadVar;  // last heading variance
		MVec3 lastPos;    // our position when we last saw the ball

		// this is the number of frames since the ball was last seen
		// it is incremented by growVariance so this must be used in conjunction
		// with vision updates for velocity to be working
		int framesSinceObs;

        // converts the normal data for use in the kalman filter to the global
        // vector
        inline void KI2DWithVelocity::velNorm2Global() {
            gVelVal.setVector(vCART, velVal(0, 0), velVal(1, 0));
            velGlobalCur = true;
        }

        // converts the normal data for use in the kalman filter to the local
        // vector
        inline void KI2DWithVelocity::velNorm2Local() {
            lVelVal.setVector(vCART, velVal(0, 0), velVal(1, 0));
            lVelVal.rotate(90 - (gps->self()).h);
            velLocalCur = true;
        }
#endif
};

// return the global position of an observation
inline void getGlobalPos(double dist,
		double head,
		const MVec3 &position,
		MVec2 &ret) {
	ret(0, 0) = position(0, 0) + dist * cos(RAD2DEG(position(2, 0) - head));
	ret(1, 0) = position(1, 0) + dist * sin(RAD2DEG(position(2, 0) - head));
}


#endif // KI2D_WITH_VELOCITY_h_DEFINED
