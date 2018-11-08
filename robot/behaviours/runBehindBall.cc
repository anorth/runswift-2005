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
 * $Id: runBehindBall.cc 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

/*
 * Strategy:
 *  saRunBehindBall
 *
 * Description:
 *  run behind ball in a faster way.
 *
 */

#include "Behaviours.h"
#ifdef COMPILE_ALL_CPP
// double b2my is actually the vBall.d
// Please read the documentation about this function in the notes directory

/* 
 ** radius  is the radius of the circular locus.
 ** aa      is the attack angle in global coordiante. aa must be larger than 180 less than 360 (or -180 to 0)
 ** turndir is either ANTICLOCKWISE or CLOCKWISE
 ** taOff   is the tangental offset of the circle from the ball. +ve mean the circle is higher than the ball.
 **         -ve means the circle is lower than the ball. 0 = on the circle.
 ** raOff   is the tangental offset of the circle from the ball. +ve mean the center is further away from
 **         the ball, -ve mean the ball is inside the circle. 0 = on the circle.
 **
 ** NOTE: At the moment the dog will keep circling, you need to decide what to do next if you want to use it.
 **       A good way is to detect the visual distance btw the ball and the dog, or the difference btw the
 **       attack angle and the dog's heading, etc...
 */

#include "UNSW2004.h"

static double sum_reqTurn = 0;
static double last_reqTurn = 0;
static bool bDebugEverything = true;
static double lastBx = 0;
static double lastBy = 0;

static unsigned long lastFrame = 0;

using namespace UNSW2004;

void Behaviours::resetSaRunBehindBall () {
  sum_reqTurn = 0;
  last_reqTurn = 0;
}

void Behaviours::saRunBehindBall(double radius,
		double aa,
		int turndir, double taOff, double raOff
) {
#ifndef OFFLINE
    UNSW2004::doUNSW2004TrackVisualBall();
#endif
	walkType = EllipseWalkWT;
	left = 0;
	forward = 7;

	double bx;
	double by;
	if ( vBall.cf > 2 ) {
	  lastBx = bx = vBall.x;                          // the ball position
	  lastBy = by = vBall.y;
	}
	else {
	  bx = lastBx;
	  by = lastBy;
	}
	double myx = gps->self().pos.x;                // My (the dog) position and its heading
	double myy = gps->self().pos.y;
	double myh = gps->self().h;
	double cx, cy;                                   // the center of the circular locus
	double lpx, lpy;                                 // the point where the dog leaves the locus (leaving point)
	double dX, dY;                                   // For the use of beta (angle between my and center)
	double AA;                                       // normalised attack angle

	if (turndir == CLOCKWISE) {
		AA = utilNormaliseAngle(aa);
	}
	else {
		AA = utilNormaliseAngle(aa + 180);
	}
	// The following code projects the circular locus.

	// Find the lp, given the ball position, taOff and the attack angle
	if (turndir == ANTICLOCKWISE) {
		saGetPointRelative(bx, by, AA, taOff, &lpx, &lpy);
	}
	else {
		saGetPointRelative(bx, by, AA, -taOff, &lpx, &lpy);
	}

	// Find the center of the circle, given the lp, the radius and attack angle
	saGetPointRelative(lpx, lpy, (AA - 90), (radius + raOff), &cx, &cy);

	double c2my = DISTANCE(cx, cy, myx, myy); // distance btw me and center
	double lp2my = DISTANCE(lpx, lpy, myx, myy); // distance btw me and lp
	double alpha;                                   // Angle between center and cloest tangent point
	double beta;                                    // Angle between global x-axis and center
	double errL = radius - c2my;      // Indicates inside or outside of the circle. Used as adjustment in vector field.
	double vectorArr;                               // The current vector in the whole vector field.
	double reqTurn;                                 // the PID controller error input.

	dX = cx - myx;
	dY = cy - myy;

	// Guard the division by zero error.
	if (c2my <= radius) {
		alpha = 0;
	}
	else {
	  alpha = RAD2DEG(asin(radius / c2my));
	}

	if (dX == 0) {
		beta = 90;
	}
	else {
		beta = RAD2DEG(atan2(dY,dX));
	}

	// The following 2 normalisation of arctan are needed for continuous range of values such that the
	// maths of finding the vector field and PID error input is a lot more straight and simplified.

	if (turndir == ANTICLOCKWISE) {
		// Get the 0 to 360 range of arctan used in ANTICLOCKWISE case. if 1st quadrant, dont do anything
	  /*		if (dX < 0 && dY >= 0)
			beta += 180;
		else if (dX < 0 && dY < 0)
			beta += 180;
		else if (dX >= 0 && dY < 0)
			beta += 360;
	  */
		vectorArr = beta - 90;           // Find the tangent vector perp to the c2my line first
		if (errL > 0)
			vectorArr -= errL / radius * 45.0; // linear discrepency
		else if (errL < 0)             // When outside of the circle, the vector is
			vectorArr = beta - alpha;    // the tangent closest to the circle.

	}
	else {
		// Get the -180 to 180 range of arctan used in CLOCKWISE case. if 1st and 4th quadrants, dont do anything
	  /*	if (dX < 0 && dY >= 0)
			beta += 180;
		else if (dX < 0 && dY < 0)
			beta += -180;
	  */
		vectorArr = beta + 90;
		if (errL > 0)
			vectorArr += errL / radius * 45.0; // linear discrepency
		else if (errL < 0)             // When outside of the circle, the vector is
			vectorArr = beta + alpha;    // the tangent closest to the circle.

	}
	// After the vector is found, the PID controller will handle the job.
	// At the moment, these values produce a good trajectory for this technique.
	// But it may need futher adjustment, lets wait and see.

	// Note:
	// When u tune, first tune the KP to improve the rise time, then the KD to improve
	// the overshoot, finally the KI for better steady state.
	// better see http://www.engin.umich.edu/group/ctm/PID/PID.html before u touch them

	const double KP = 0.5;//kp/100;//0.5   // Less Than 1
	const double KD = 0;//0.1
	const double KI = 0.1;//ki/100;//0.1  // Has to be very small

	double thisTurn = utilNormaliseAngle(vectorArr - myh);
	/*cout << "KI*sum_T  : " << KI * sum_reqTurn << "     KP*reqTurn: "
		<< KP * reqTurn << "     KD*error: " << KD * (reqTurn - last_reqTurn)
		<< endl << "Unjustied reqTurn: " << reqTurn;*/
	long  skippedFrames = frame-(lastFrame+1);
	if (skippedFrames > 0) {
          cout << "skipped " << skippedFrames << endl;
	  sum_reqTurn *= pow(0.9, (double)skippedFrames);
	}
	lastFrame = frame;
	if (abs(sum_reqTurn) > 30/KI) {
	  cout << "clipping sum" << endl;
	  sum_reqTurn = Cap(sum_reqTurn, 30/KI);
	}
	if (c2my > 3*radius) {
	  sum_reqTurn = 0;
	  last_reqTurn = thisTurn;
	}
	reqTurn = KP * thisTurn + KI * sum_reqTurn + KD * (thisTurn - last_reqTurn);
	/*cout << "      reqTurn: " << reqTurn << endl;
	cout << "----------\n";*/
	cout << "----------\n";

	sum_reqTurn += thisTurn;           // The integral part
	last_reqTurn = thisTurn;           // Used in the derivative part
	cout << "This turn: " << thisTurn << endl;
	//turnCCW      = vectorArr - myh; // Comment the next line, uncomment this line, to see the effect without PID
	turnCCW = reqTurn;           // the output of PID is assigned to the turnCCW
	turnCCW = Cap(turnCCW, 45);   // Cap the turnCCW to +-20, prevent jerking

	// Print the debugging info
	
    if (bDebugEverything){
	   cout << "My position      : x: " << myx << " y: " << myy << "\nBall position    : x: " 
	   << bx << " y: " << by << "\n";
	   //cout << "LP position  : x: " << lpx << " y: " << lpy << endl;
	   //cout << "Center position  : x: " << cx << " y: " << cy << endl;
	   cout << "c2my: " << c2my << "   lp2my: " << lp2my << endl;
	   cout << "beta: " << beta << "   alpha: " << alpha << "   myh: " << myh << endl;
    }
    cout << "SUM " << sum_reqTurn << "    KP " << KP << "   KI " << KI << endl;   
    cout << "Vector: " << vectorArr << "    turnCCCW: " << turnCCW << endl;
	
	return;
}
#endif //COMPILE_ALL_CPP
