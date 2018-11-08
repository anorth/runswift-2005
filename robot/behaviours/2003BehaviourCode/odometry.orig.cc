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
 * $Id$
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * Odometry Calibration tool
 *
 **/ 

#include "Behaviours.h"
#include "../share/BallDef.h"
#include "../share/Common.h"
#include "../share/CanterCalib.h"
#include "odometry.h"
#include "walkVals.h"

using Odometry::state;
using Odometry::stateType;

static const double origx = FIELD_WIDTH / 2.0;
static const double origy = FIELD_LENGTH / 4.0;
static const double origh = 90;

static const char *RESULT_FILE = "/MS/OPEN-R/odometry.log";

static const int stepsToDo = 12;
static const int framesInAStep = 8;
static const int timeToWalkFor = stepsToDo *framesInAStep + framesInAStep / 3; 
static const int timeToLocaliseFor = 100 + framesInAStep;

double startx = 0;
double starty = 0;
double starth = 0;

static int stateCounter = 0;
stateType state;

double randF = 0;
double randL = 0;
double randT = 0;

//counts how many frames hes been at the origin, because i dont let him change states
//until hes fairly sure that hes at the origin, ie hes been at the origin for a while
int inPosCount = 0;

static int randGenCount = 1;

ofstream *outFile;

static bool stateEyes = true;
#define widEyes  leds(2,1,1,2,1,1)
#define lawEyes  leds(2,2,1,2,2,1)
#define wbEyes   leds(1,1,2,1,1,2)
#define lawbEyes leds(1,2,2,1,2,2)
#define cpEyes   leds(2,1,2,2,1,2)
#define lacpEyes leds(2,2,2,2,2,2)

void Odometry::initOdometryTester() {
	lockMode = NoMode;        
	lostBall = 0;
	grabTime = 0;
	state = correctPosition;
	srand48(PLAYER_NUMBER);
}

void Odometry::doOdometryTester() {
	//outFile = new ofstream(RESULT_FILE);
	setDefaultParams();
	setWalkParams();
	setOdometryTesterValues();
	chooseOdometryTesterStrategy();
	//outFile->close();
	//cout << "from behavs randt " << randT << " turnCCW " << turnCCW << endl;
}

void Odometry::setOdometryTesterValues() {
	leds(1, 1, 1, 1, 1, 1); //reset the leds everyframe

	if (stateEyes) {
		switch (state) {
			case walkInDirection: widEyes;
					      break;
			case localiseAfterWalking: lawEyes;
						   break;
			case walkBack: wbEyes;
				       break;
			case localiseAfterWalkingBack: lawbEyes;
						       break;
			case correctPosition: cpEyes;
					      break;
			case localiseAfterCorrectingPosition: lacpEyes;
							      break;
			default: leds(1, 1, 1, 1, 1, 1);
				 break;
		}
	}
}

int randType = 1;

void Odometry::chooseOdometryTesterStrategy() {
	if (mode_ == mStartMatch) {
		setStandParams();
		StationaryLocalize();
	}
	else {
		const double locx = gps->self().pos.x;
		const double locy = gps->self().pos.y;
		const double loch = gps->self().h;

		if (state == walkInDirection) {
			if (stateCounter == 0) {
				randType = randFLT(&randF, &randL, &randT);
				cout << "r_flt," << randType << "," << randF << "," << randL << ","
					<< randT << ",";
				cout
					<< "c_flt,"
					<< randType
					<< ","
					<< Canter::forwardCalib(randF,
							randL,
							randT)
					<< ","
					<< Canter::leftCalib(randF,
							randL,
							randT)
					<< ","
					<< Canter::turnCCWCalib(randF,
							randL,
							randT)
					<< ",";
			} 

			if (stateCounter < timeToWalkFor) {
				setWalkParams();
				forward = randF;
				left = randL;
				turnCCW = randT;
			}
			else {
				setStandParams();
				state = localiseAfterWalking;
				stateCounter = 0;
				return;
			}
			stateCounter++;
		}
		else if (state == localiseAfterWalking) {
			if (stateCounter < framesInAStep) {
				setStandParams();
			}
			else if (stateCounter == framesInAStep) {
				cout << "o_xyh," << locx << "," << locy << "," << loch << ",";
				printInfo(randF,
						randL,
						randT,
						startx,
						starty,
						starth,
						locx,
						locy,
						loch,
						stepsToDo);
				setStandParams();
			}
			else if (stateCounter < timeToLocaliseFor) {
				gps->allowedToVisionUpdate = true;
				setStandParams();
				setNewBeacons(LARGE_VAL);
				activeGpsLocalise(true);
			}
			else if (stateCounter == timeToLocaliseFor) {
				//setStandParams();
				cout << "v_xyh," << locx << "," << locy << "," << loch << ",";
				printInfo(randF,
						randL,
						randT,
						startx,
						starty,
						starth,
						locx,
						locy,
						loch,
						stepsToDo);
				cout << endl;
				gps->allowedToVisionUpdate = false;
			}
			else {
				//setStandParams();
				startx = locx;
				starty = locy;
				starth = loch;
				cout << "s_xyh," << startx << "," << starty << "," << starth << ",";    
				state = walkBack;
				stateCounter = 0;
				return;
			}
			stateCounter++;
		}
		else if (state == walkBack) {
			if (stateCounter == 0) {
				randF = -1 * randF;
				randL = -1 * randL;
				randT = -1 * randT;
				cout << "r_flt," << randType << "," << randF << "," << randL << ","
					<< randT << ",";
				cout
					<< "c_flt,"
					<< randType
					<< ","
					<< Canter::forwardCalib(randF,
							randL,
							randT)
					<< ","
					<< Canter::leftCalib(randF,
							randL,
							randT)
					<< ","
					<< Canter::turnCCWCalib(randF,
							randL,
							randT)
					<< ",";
			}

			if (stateCounter < timeToWalkFor) {
				setWalkParams();
				forward = randF;
				left = randL;
				turnCCW = randT;
			}
			else {
				state = localiseAfterWalkingBack;
				stateCounter = 0;
				return;
			}
			stateCounter++;
		}
		else if (state == localiseAfterWalkingBack) {
			if (stateCounter < framesInAStep) {
				setStandParams();
			}
			else if (stateCounter == framesInAStep) {
				cout << "o_xyh," << locx << "," << locy << "," << loch << ",";
				printInfo(randF,
						randL,
						randT,
						startx,
						starty,
						starth,
						locx,
						locy,
						loch,
						stepsToDo);
				setStandParams();
			}
			else if (stateCounter < timeToLocaliseFor) {
				gps->allowedToVisionUpdate = true;
				setStandParams();
				setNewBeacons(LARGE_VAL);
				activeGpsLocalise(true);
			}
			else {
				setStandParams();
				cout << "v_xyh," << locx << "," << locy << "," << loch << ",";
				printInfo(randF,
						randL,
						randT,
						startx,
						starty,
						starth,
						locx,
						locy,
						loch,
						stepsToDo);
				cout << endl;
				state = correctPosition;
				stateCounter = 0;
				return;
			}
			stateCounter++;
		}
		else if (state == correctPosition) {
			if (abs(loch - origh) < 5
					&& abs(locx - origx) < 5
					&& abs(locy - origy) < 5) {
				if (inPosCount > 10) {
					setStandParams();
					state = localiseAfterCorrectingPosition;
					stateCounter = 0;
					return;
				}
				else {
					inPosCount++;
					setWalkParams();
					setNewBeacons(LARGE_VAL);
					activeGpsLocalise(true);        
					saGoToTargetFacingHeading(origx, origy, origh);
				}
			}
			else {
				inPosCount = 0;
				setWalkParams();
				setNewBeacons(LARGE_VAL);
				activeGpsLocalise(true);        
				saGoToTargetFacingHeading(origx, origy, origh);
			}
			stateCounter++;
		}
		else if (state == localiseAfterCorrectingPosition) {
			if (stateCounter < timeToLocaliseFor) {
				setStandParams();
				setNewBeacons(LARGE_VAL);
				activeGpsLocalise(true);
			}
			else if (stateCounter == timeToLocaliseFor) {
				//setStandParams();
				gps->allowedToVisionUpdate = false;
				//wait a frame after turning off gps before taking a measurement
			}
			else {
				//setStandParams();
				startx = locx;
				starty = locy;
				starth = loch;
				cout << "s_xyh," << startx << "," << starty << "," << starth << ",";
				state = walkInDirection;
				stateCounter = 0;
				return;
			}
			stateCounter++;
		}
		else {
			state = correctPosition;
			stateCounter = 0;
		}
	}
}

int Odometry::randFLT(double *fComp, double *lComp, double *tComp) {
	int numMoves = sizeof(sampleMoves) / sizeof(sampleMoves[0]);
	int fltSize = sizeof(sampleMoves[0]) / sizeof(sampleMoves[0][0]);
	static int arrayPosn = 0;

	if (fltSize != 3) {
		cout << "bad sampleMoves" << endl;
		*fComp = *lComp = *tComp = 0;
		return -1;
	}

	int sample = (int) (drand48() * numMoves);
	// int sample = arrayPosn;

	*fComp = sampleMoves[sample][0];
	*lComp = sampleMoves[sample][1];
	*tComp = sampleMoves[sample][2];

	arrayPosn = (arrayPosn + 1) % numMoves;

	return sample;
}

/*

   int Odometry::randFLT(double *fComp, double *lComp, double *tComp) {

   if (randGenCount>9) {
   randGenCount = 1;
   }

   double maxT = 50;
   double maxF = 7;
   double maxL = 7;

   switch (randGenCount) {
   case 1:
   maxT = 0;
   maxF = 0;
   maxL = maxL;
   break;
   case 2:
   maxT = 0;
   maxF = maxF;
   maxL = 0;
   break;
   case 3:
   maxT = 0;
   maxF = maxF;
   maxL = maxL;
   break;
   case 4:
   maxT = maxT;
   maxF = 0;
   maxL = 0;
   break;
   case 5:
   maxT = maxT;
   maxF = 0;
   maxL = maxL;
   break;
   case 6:
   maxT = maxT;
   maxF = maxF;
   maxL = 0;
   break;
   case 7:
   maxT = maxT;
   maxF = maxF;
   maxL = maxL;
   break;
   case 8:
   maxT = maxT;
   maxF = maxF;
   maxL = maxL;
   break;
   case 9:
   maxT = maxT;
   maxF = maxF;
   maxL = maxL;
   break;
   default:
   maxT = maxT;
   maxF = maxF;
   maxL = maxL;
   break;
   }

   randGenCount++;

   double randt = drand48()*2*maxT - maxT;

   if (maxT==0) {
   if (maxF!=0) maxF = 7;
   if (maxL!=0) maxL = 7;
} else if (maxF==0 && maxL==0) {
	//do nothing
} else if (maxF==0 && maxL!=0) {
	if (abs(randt)<27) {
		maxL = 7;
	} else if (abs(randt) < 40) {
		maxL = 3;
	} else {
		maxL = 1;
	}
} else if (maxL==0 && maxF!=0) {
	if (abs(randt)<15) {
		maxF = 7;
	} if (abs(randt)<27) {
		maxF = 5;
	} else if (abs(randt)<40) {
		maxF = 2;
	} else {
		maxF = 0;
	}
} else {
	if (randt==0) {
		maxF = 7;
		maxL = 7;
	} else if (randt < 10 && randt >= 0) { 
		maxF = 6;
		maxL = 6;
	} 
	else if (randt < 20 && randt >= 0) { 
		maxF = 5;
		maxL = 5;
	}
	else if (randt < 30 && randt >= 0) { 
		maxF = 3;
		maxL = 3;
	}
	else if (randt < 40 && randt >= 0) { 
		maxF = 1;
		maxL = 1;
	} 
	else if (randt >= 0) {
		maxF = 0;
		maxL = 0;
	}

	else if (randt > -10 && randt < 0) { 
		maxF = 5;
		maxL = 5;
	}
	else if (randt > -20 && randt < 0) { 
		maxF = 3;
		maxL = 4;
	}
	else if (randt > -30 && randt < 0) { 
		maxF = 2;
		maxL = 2;
	}
	else if (randt > -40 && randt < 0) { 
		maxF = 1;
		maxL = 2;
	}   
	else {
		maxF = 0;
		maxL = 0;
	}
}

double randf = drand48()*2*maxF - maxF;
double randl = drand48()*2*maxL - maxL;

*fComp = randf;
*lComp = randl; 
*tComp = randt;
return (randGenCount-1);
}
*/

/** This routine tries to get an actual measurement of how far the dog has turned given the
 * commanded speed, the difference in angle from start and end, and the time taken.
 */

double Odometry::estimateTotalTurn(const double trnDeg,
		const double deltahDeg,
		double t) {
	double striationDist = trnDeg *t - deltahDeg;

	int striationNum = (int) round(striationDist / FULL_CIRCLE);
	return deltahDeg + FULL_CIRCLE * striationNum;
}

/** Convert from start and end point to instantaneous velocities.  This relies upon the fact that
 * if you start at a point, and walk with constant velocity (relative to yourself) and a constant
 * turn speed, you will walk around the circumference of a circle.  At each point, your actual
 * instantaneous velocity will be tangent to the circle.  The dog's heading will not be tangent to
 * the circle unless the dog is walking straight forward.  This routine takes two points and
 * headings lying on this circle, and prints out the instantaneous velocities required to get there.
 *
 * This code assumes that you you are taking @const{stepsToDo} steps around the circle.
 * Some positions have a 'Head' suffix.  These refer to the position of the dog's head.  Other
 * positions have a 'Body' suffix.  These refer to the position of the center of the dog's body.
 * There are similar suffixes for degrees, 'Deg', and radians, 'Rad'.
 * Most of the work is done in Body coordinates, as this aligns the center of rotation correctly.
 * The starting and ending locations are in field coordinates.  The end point is in global
 * coordinates, not relative to the start.
 */

void Odometry::printInfo(const double fwd,
		const double lft,
		const double trnDeg,
		const double startxHead,
		const double startyHead,
		const double starthDeg,
		const double endxHead,
		const double endyHead,
		const double endhDeg,
		const double time) {
	const double starthRad = DEG2RAD(starthDeg);
	const double startxBody = startxHead - TURN_CENTRE_TO_NECK *cos(starthRad);
	const double startyBody = startyHead - TURN_CENTRE_TO_NECK *sin(starthRad);

	const double endhRad = DEG2RAD(endhDeg);
	const double endxBody = endxHead - TURN_CENTRE_TO_NECK *cos(endhRad);
	const double endyBody = endyHead - TURN_CENTRE_TO_NECK *sin(endhRad);

	// initally tempx and tempy are the Body deltas in field coordinates
	double tempx = endxBody - startxBody;
	double tempy = endyBody - startyBody;

	const double deltahDeg = endhDeg - starthDeg;
	const double deltahRad = endhRad - starthRad;

	// let's rotate the deltas to assume we start pointing down the x axis.
	// this makes fwd = +x, lft = +y and trn right handed with x and y.
	const double deltax = tempx *cos(starthRad) + tempy *sin(starthRad);
	const double deltay = -tempx *sin(starthRad) + tempy *cos(starthRad);

	const double lineDist = sqrt(tempx *tempx + tempy *tempy);

	// these vars, prefixed 'cmd', are the calculated versions assuming the dog works perfectly
	const double cmdSpd = sqrt(fwd *fwd + lft *lft);
	const double cmdWalkDirRad = (lft == 0 && fwd == 0) ? 0 : atan2(lft, fwd);

	const double totDeltahDeg = estimateTotalTurn(trnDeg, deltahDeg, time);
	const double totDeltahRad = DEG2RAD(totDeltahDeg);

	// this is the initial estimate of the radius of the circle
	double radius = lineDist / ABS(2 * cos((M_PI - totDeltahRad) / 2));

	const double cmdRadius = (trnDeg == 0)
		? FIELD_DIAGONAL
		: (HALF_CIRCLE / ABS(trnDeg) * cmdSpd / M_PI);

	// this is a quick hack - if the radius is too large, then then just hack it so that the
	// distance around the circle will end up being the straight linear distance.
	/*  if (radius > cmdRadius*3)
	    radius = cmdRadius*3;
	    */
	const double circleDist = radius *ABS(totDeltahRad);

	double spd = circleDist / time;

	// we can't have a step longer than the body length

	if (spd > FULL_BODY_LENGTH) {
		spd = 0;
	}


	// secondHalfOfCircle is true if we have gone more than 180 deg around the circle, in
	// either direction.  It resets when get back to the start and start around the circle a
	// second time.
	// This will work best if we avoid turns that end up at 180 deg, 360 deg, 540 deg, etc.
	// That is every multiple of 180 deg is a problem, except 0
	// At those points we are succeptible to noise

	// const bool secondHalfOfCircle = fmod(ABS(totDeltahRad),2*M_PI) > M_PI;

	const double halfhRad = NormalizeAngle_PI(totDeltahRad) / 2;

	const double angleToDelta = (deltay == 0 && deltax == 0)
		? 0
		: atan2(deltay,
				deltax);

	// walkDirRad is the estimated version of cmdWalkDirRad
	double walkDirRad = NormalizeAngle_PI(angleToDelta - halfhRad);

	const bool secondHalfOfCircle = (ABS(ABS(walkDirRad - cmdWalkDirRad) - M_PI)
			< M_PI
			/ 4)
		&& (cmdWalkDirRad != 0);

	if (secondHalfOfCircle) {
		walkDirRad = NormalizeAngle_PI(walkDirRad + M_PI);
	}

	const double vx = spd *cos(walkDirRad);
	const double vy = spd *sin(walkDirRad);
	const double vhDeg = totDeltahDeg / time;

	cout << "dxyh, " << deltax << "," << deltay << "," << deltahDeg;
	cout << ", t, " << time;
	cout << ", totDh, " << totDeltahDeg;
	/*
	   cout << ", cmd rsh, " << cmdRadius << "," << cmdSpd << "," << cmdWalkDirRad;
	   cout << ", h/2 atd shc, " << halfhRad << "," << angleToDelta << "," << secondHalfOfCircle;
	   cout << ", rhc, " << radius << "," << lineDist << "," << circleDist;
	   */
	cout << ", sa, " << spd << "," << walkDirRad;
	cout << ", vxyh, " << vx << "," << vy << "," << vhDeg << ",";
	cout.flush();
}
