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
 */

#include <numeric>

#include "KI2DWithVelocity.h"

/* See the big comment at the top of KI2DWithVelocity.h for why this file
 * does not look as you expected
 */

const Vector KI2DWithVelocity::INVALID_VEL(vCART, 0, 0);

KI2DWithVelocity::KI2DWithVelocity(GPS *g, double iniX, double iniY,
                                    double iniXVar, double iniYVar) :
        KalmanInfo2D(g, iniX, iniY, iniXVar, iniYVar),
        history(HISTORY_LENGTH, Vector(vCART, 0, 0)),
        lastx(0.0), lasty(0.0) , framesSinceObs(VERY_LARGE_INT)
{
}

void KI2DWithVelocity::shiftLastBallPos(double dForward, double dLeft, double dTurn) {
  // we need to inverse the movements to get the last correct ball local pos
  Vector pos(vCART, lastx - dForward, lasty + dLeft); // right is positive
  pos.rotate(-dTurn);
  lastx = pos.getX();
  lasty = pos.getY();
}


void KI2DWithVelocity::infoKalman(const SlimVisOb &vob, bool updateVelocity) {
    // (vCART, 0, 0) is a FLAG meaning THIS VECTOR NOT VALID,
    // possible because noise ensures we never actually get 0 in valid data, 
    Vector vel(vCART, 0, 0);
    
    updateVelocity = true;

    if (vob.cf > 0) {
            
        // vob.h is zero straight ahead, positive left
      double bx = - vob.d * sin(DEG2RAD(vob.h)); // x is positive right, 
                                                 //following the global ball position coordinates
        double by = vob.d * cos(DEG2RAD(vob.h));
        
        // Reset the position of the ball, if we haven't seen it for 1 sec
        // Should it be ball's global position?
        //if (framesSinceObs > 30) {      
        //    setVal(bx,by,30.0,0,0);            
        //}
        
        KalmanInfo2D::infoKalman(vob);        
        
//        cerr << "ball (" << vob.d << "," << vob.h << ") at local "
//            << bx << ", " << by << endl;
	  
	if (updateVelocity && framesSinceObs < 3) {
	  vel.setVector(vCART, (bx - lastx)/framesSinceObs, (by - lasty)/framesSinceObs);
	  if (vel.getR() > 13.3) { // ignore observations > 4 metres/s (13.3 cm/frame)
	    vel = INVALID_VEL;
	  }
        }
        lastx = bx;
        lasty = by;
        framesSinceObs = 0;
    }

//    cerr << "vel observation " << vel.getX() << ", " << vel.getY() << endl;
    history.pop_front();
    history.push_back(vel);

    ++framesSinceObs;


    // update velocity
    static const int NUM_INVALID_THRESHOLD = 5;
    localVel.setVector(vCART, 0, 0);
    int numInvalid = 0;
  
    list<Vector>::iterator itr;
    for (itr = history.begin(); itr != history.end(); itr++) { 
      if (itr->getX() == 0 && itr->getY() == 0) {
	numInvalid++;
      } else {
	localVel = localVel + *itr;
      }
    } 
  
    if (numInvalid >= HISTORY_LENGTH) {
      localVel = INVALID_VEL;
      lastValidVel = INVALID_VEL;
    } else if (numInvalid > NUM_INVALID_THRESHOLD) {
      localVel = lastValidVel;      
    } else { // numInvalid <= NUM_INVALID_THRESHOLD
      localVel.scale(1.0 / (HISTORY_LENGTH - numInvalid));
      if (numInvalid == NUM_INVALID_THRESHOLD)
	lastValidVel = localVel;
    }
    globalVel = localVel; // copy over
    globalVel.rotate(gps->self().h - 90);
}

void KI2DWithVelocity::setVal(double x, double y, double var,
                                double dx, double dy) {
    KalmanInfo2D::setVal(x, y, var);
    std::fill(history.begin(), history.end(), Vector(vCART, dx, dy));
}

const Vector& KI2DWithVelocity::getVelocity(int context) {    
  if (context == GLOBAL) {
    return globalVel;
  }            
  return localVel;    // local velocity
}

/* Grows the position variance and adds velocity to ball pos */
void KI2DWithVelocity::growVariance(double amount) {
  /*
  double x, y;
  x = velocity.getX();
  y = velocity.getY();
  if (fabs(x) > 2 || fabs(y) > 2)
    FFCOUT << "LOCAL:" << x << ' ' << y << endl;
  */
  Vector vel = getVelocity(GLOBAL);
  KalmanInfo2D::growVariance(amount, vel.getX(), vel.getY());
  /*
  x = vel.getX();
  y = vel.getY();
  if (fabs(x) > 2 || fabs(y) > 2)
    FFCOUT << "Global: " << x << ' ' << y << endl;
  */
}

#if 0

// maximum number of frames between two position observations constitute a
// velocity observation
static const int MAX_FRAME = 4;

// The ball decelerates this much if we can't see it
static const double DECELERATION = 0.90;

static const double IVSumDecayRate = 0.4;


KI2DWithVelocity::KI2DWithVelocity(GPS *g,
		double iniX,
		double iniY,
		double iniXVar,
		double iniYVar,
		double iniVX,
		double iniVY,
		double iniVXVar,
		double iniVYVar) : KalmanInfo2D(g,
			iniX,
			iniY,
			iniXVar,
			iniYVar)
{
	velVal(0, 0) = iniVX;
	velVal(1, 0) = iniVY;
	velNorm2Global();
	velNorm2Local();

	velCov(0, 0) = iniVXVar;
	velCov(0, 1) = 0.0;
	velCov(1, 0) = 0.0;
	velCov(1, 1) = iniVYVar;

	lastDist = 0.0;
	lastHead = 0.0;
	lastPos(0, 0) = 0.0;
	lastPos(1, 0) = 0.0;
	lastPos(2, 0) = 0.0;
	framesSinceObs = VERY_LARGE_INT;

	ivLength = 0.0;
}

const Vector & KI2DWithVelocity::getVelocity(int context) {
	if (context == GLOBAL) {
		if (!velGlobalCur)
			velNorm2Global();

		return gVelVal;
	}
	else {
		// context == LOCAL

		if (!velLocalCur)
			velNorm2Local();

		return lVelVal;
	}
}

void KI2DWithVelocity::infoKalman(const SlimVisOb &vob, bool updateVelocity) {

    /* Do position update */
	KalmanInfo2D::infoKalman(vob);

	const WMObj &self = gps->self(); // position of robot
    const MMatrix3 &rcov = gps->selfCov();

    // we only perform velocity updates if we get two consecutive frames
	if (updateVelocity && framesSinceObs <= 1) {
		// global position of last observation
		MVec2 lastObs;
		MMatrix2 lastObsCov;
        
		// global position of this observation
		MVec2 curObs;
		MMatrix2 curObsCov;

		MVec3 curPos;
		curPos(0, 0) = self.pos.x;
		curPos(1, 0) = self.pos.y;
		curPos(2, 0) = self.h;
		
		(gps->vbPull) (0, 0) = 0.0;
		(gps->vbPull) (1, 0) = 0.0;
		(gps->vbPull) (2, 0) = 0.0;
        
        // Get the last observation as global coords and covariance from
        // to out current position. Current position rather than last position
        // so that if our GPS self position moves this does not create ball
        // velocity. The lastDist/Head are local so the calculations are local
        // as well, but the actual velocity is then stored in global coords
        // so it can update (global) gps ball properly
		getGlobalObservation(curPos,
				rcov,
				lastDist,
				lastHead,
				lastDistVar,
				lastHeadVar,
				lastObs,
				lastObsCov);

        // get the current observation in global coords
		getGlobalObservation(curPos,
				rcov,
				vob.d,
				vob.h,
				vob.var,
				vob.angleVar,
				curObs,
                curObsCov);

        // measurement of velocity is the difference betweeen the position
        // observations, covariance is the sum of the two.
		MVec2 meas = curObs;
		meas -= lastObs;

		if (getNumSDVel(meas(0, 0), meas(1, 0)) > 1.0) {
            // Probability filtering: if this observation is outside 1 s.d.
            // then ignore it and increase variance.
			velCov *= 2.0;
		} else {
			MMatrix2 ccov = curObsCov;
			ccov += lastObsCov;
			//ccov ^= framesSinceObs;

			// perform kalman filtering
			MVec2 iv = meas;
			iv -= velVal;

			double ivl = sqrt(SQUARE(iv(0, 0)) + SQUARE(iv(1, 0)));
			ivLength *= 1.0 - IVSumDecayRate;
			ivLength += IVSumDecayRate * ivl;

			MMatrix2 m1;
			m1 = velCov;
			m1 += ccov;
			MMatrix2 m1Inv;
			m1Inv.isInverse(m1);
			MMatrix2 k;
			k.isMult(velCov, m1Inv);

			MVec2 diff;
			diff.isMult(k, iv);
			velVal += diff;

			MMatrix2 m3(1);
			m3 -= k;
			velCov.preMult(m3);

			velGlobalCur = false;
			velLocalCur = false;
		}
	}

	lastDist = vob.d;
	lastHead = vob.h;
	lastDistVar = vob.var;
	lastHeadVar = vob.angleVar;
	lastPos(0, 0) = self.pos.x;
	lastPos(1, 0) = self.pos.y;
	lastPos(2, 0) = self.h;
	framesSinceObs = 0;

}


/* Grows the position and velocity variance. Ball velocity is decayed
 * if visual is false.
 */
void KI2DWithVelocity::growVariance(double amount, double velAmount,
                                    bool visual) {
	KalmanInfo2D::growVariance(amount, velVal(0, 0), velVal(1, 0));

	MMatrix2 velMult(0);
    if (! visual) { // decelerate if we can't see it (reliably)
        velMult(0, 0) = DECELERATION;
        velMult(1, 1) = DECELERATION;
    }

	velVal.preMult(velMult);
	//velCov.preMult(velMult);

	velCov(0, 0) += velAmount;
	velCov(1, 1) += velAmount;

	// if the ball has gone off the field then set velocity to zero
    // [and decrease velocity variance (we know it will be stationary)]
	if ((val(0, 0) < 0)
			|| (val(0, 0) > FIELD_WIDTH)
			|| (val(1, 0) < 0)
			|| (val(1, 0) > FIELD_LENGTH)) {
		velVal(0, 0) = 0.0;
		velVal(1, 0) = 0.0;
        //velCov *= 0.5;
	}

	framesSinceObs++;

	velGlobalCur = false;
	velLocalCur = false;
}


const MMatrix2 & KI2DWithVelocity::getVelCov() {
	return velCov;
}

MMatrix2 KI2DWithVelocity::getVCovLocal() {
	// get the covariance matrix in terms of local coordinates
	// angle to get the coords wrt robot to global coords
	double angleRad = DEG2RAD(gps->self().h - 90);
	double cs = cos(angleRad);
	double sn = sin(angleRad);
	MMatrix2 rotate(0);
	rotate(0, 0) = cs;
	rotate(0, 1) = -sn;
	rotate(1, 0) = sn;
	rotate(0, 1) = cs;
	MMatrix2 rotateInv(0);
	rotateInv(0, 0) = cs;
	rotateInv(0, 1) = sn;
	rotateInv(1, 0) = -sn;
	rotateInv(0, 1) = cs;

	MMatrix2 covWRTB;
	covWRTB.isMult(velCov, rotate);
	covWRTB.preMult(rotateInv);

	return covWRTB;
}


MMatrix2 KI2DWithVelocity::getVCovBall() {
	// get the covariance matrix in terms of the ball, y axis towards the robot
	// angle to get the coords wrt ball to global coords
	double angleRad = atan2(velVal(0, 0) - gps->self().pos.x,
			velVal(1, 0) - gps->self().pos.y);
	double cs = cos(angleRad);
	double sn = sin(angleRad);
	MMatrix2 rotate(0);
	rotate(0, 0) = cs;
	rotate(0, 1) = -sn;
	rotate(1, 0) = sn;
	rotate(0, 1) = cs;
	MMatrix2 rotateInv(0);
	rotateInv(0, 0) = cs;
	rotateInv(0, 1) = sn;
	rotateInv(1, 0) = -sn;
	rotateInv(0, 1) = cs;

	MMatrix2 covWRTB;
	covWRTB.isMult(velCov, rotate);
	covWRTB.preMult(rotateInv);

	return covWRTB;
}

double KI2DWithVelocity::getIVLength() {
	return ivLength;
}

double KI2DWithVelocity::getNumSDVel(double vx, double vy) {
	MVec2 diff;
	diff(0, 0) = vx - velVal(0, 0);
	diff(1, 0) = vy - velVal(1, 0);

	MRowVec2 diffTrans;
	diffTrans.isTranspose(diff);

	MMatrix2 velCovInv;
	velCovInv.isInverse(velCov);
	MRowVec2 val1;
	val1.isMult(diffTrans, velCovInv);
	MMatrix1 val2;
	val2.isMult(val1, diff);
	return val2(0, 0);
}

int KI2DWithVelocity::serialize(unsigned char *add, int base) {
	int ret = KalmanInfo2D::serialize(add, base);
	ret = velVal.serialize(add, ret);
	ret = velCov.serialize(add, ret);

	memcpy(add + ret, &framesSinceObs, sizeof(int));
	ret += (int)sizeof(int);

	float temp = (float) lastDist;
	memcpy(add + ret, &temp, sizeof(float));
	ret += (int)sizeof(float);
	temp = (float) lastHead;
	memcpy(add + ret, &temp, sizeof(float));
	ret += (int)sizeof(float);
	temp = (float) lastDistVar;
	memcpy(add + ret, &temp, sizeof(float));
	ret += (int)sizeof(float);
	temp = (float) lastHeadVar;
	memcpy(add + ret, &temp, sizeof(float));
	ret += (int)sizeof(float);

	ret = lastPos.serialize(add, ret);

	return ret;
}


int KI2DWithVelocity::unserialize(unsigned char *add, int base) {
	int ret = KalmanInfo2D::unserialize(add, base);
	ret = velVal.unserialize(add, ret);
	ret = velCov.unserialize(add, ret);

	memcpy(&framesSinceObs, add + ret, sizeof(int));
	ret += (int)sizeof(int);

	float temp;
	memcpy(&temp, add + ret, sizeof(float));
	lastDist = (double) temp;
	ret += (int)sizeof(float);
	memcpy(&temp, add + ret, sizeof(float));
	lastHead = (double) temp;
	ret += (int)sizeof(float);
	memcpy(&temp, add + ret, sizeof(float));
	lastDistVar = (double) temp;
	ret += (int)sizeof(float);
	memcpy(&temp, add + ret, sizeof(float));
	lastHeadVar = (double) temp;
	ret += (int)sizeof(float);

	ret = lastPos.unserialize(add, ret);

	velGlobalCur = false;
	velLocalCur = false;

	return ret;
}
#endif
