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
 * $Id: locateBall.cc 6552 2005-06-12 04:27:30Z alexn $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/ 

/*
 * Strategy:
 *  locateBall
 *
 * Description:
 *
 * Strategy member variables used:
 *
 * Local variables used:
 *
 * Restrictions:
 * first time you call this locate ball, you need to call setLocateBallParams
 * you need to maintain the variable lostBall outside of this file (ie in the strategy)
 * which counts how many frames its been since you last saw the ball
 *
 * Comments:
 */

#include "Behaviours.h"

//predeclaration
//static void headFindBall(); 

/*
//these you can change to whatever you want
static const int vBallLost = 0;
static const int vBallAnywhere = 80;
static const int timeSpinFindBall = 165;
*/

// num frames in 1/2 cycle of head finding ball
static const int HEAD_PERIOD = 22;

//these you can change to whatever you want
static const int lBallLookGps = 12;                      //upto this many frames of losing the ball, look at gps ball
static const int lBallFindOnSpot = lBallLookGps + HEAD_PERIOD*2 + 10;   //upto this many frames of losing the ball, stay on the spot and do head find ball
static const int lBallSlowFind = lBallFindOnSpot + HEAD_PERIOD*2 + 40;  //upto this many frames of losing the ball, start spinning and do head find ball
static const int lBallFastFind = lBallSlowFind + 110;               //upto this many frames of losing the ball, put your head to the side and spin fast to find ball
//afterwoods, spin and move to a spot whilst doing head find ball



static int FBDir = ANTICLOCKWISE;

//first time you call locateBall you should call this, otherwise the dog will always spin anticlockwise
void Behaviours::setLocateBallParams() {
	if (PLAYER == 15 || PLAYER == 19) 
	  {
		FBDir = (gps->self().pos.x > FIELD_WIDTH / 2) ? ANTICLOCKWISE : CLOCKWISE;
	}
	else {
		double h = NormalizeAngle_180(gps->getBall(LOCAL).head);
		if (h > 0) {
			FBDir = ANTICLOCKWISE;
		}
		else {
			FBDir = CLOCKWISE;
		}
	}
}

void Behaviours::setDirection(int dir) {
	if (dir == ANTICLOCKWISE) {
		FBDir = ANTICLOCKWISE;
	}
	else {
		FBDir = CLOCKWISE;
	}
}


/* 3 stages to locate the ball, depending on how long the ball has been lost. */
void Behaviours::locateBall() {

    /* 1st stage: Use only the head to find ball. */
	if (lostBall <= lBallFindOnSpot) {
		headFindBall(-55);
		forward = 0;
		left = 0;
		turnCCW = 0;
		standTall = true;

    /* 2nd stage: spin on the spot with head moving to find the ball. */
	} else if (lostBall <= lBallSlowFind) {

		lookdir = FBDir; //set head spin direction
		headFindBall(-50);

		double dir = (FBDir == ANTICLOCKWISE) ? 1 : -1;
		forward = 0;
		left = 0;
		turnCCW = dir * 25;

    /* 3rd stage: spin on the spot to find the ball. */
	} else {
		spinFindBall();
	}
}


/* Slightly different locateBall - if after 1 fast revolution and still can't
 * find the ball, it moves towards the poit (findPositionx, findPositiony).
 */
void Behaviours::locateBall(double findPositionx, double findPositiony) {

	if (lostBall <= lBallFindOnSpot) {
		headFindBall(-55);
		forward = 0;
		left = 0;
		turnCCW = 0;
		standTall = true;

	} else if (lostBall <= lBallSlowFind) {
		lookdir = FBDir; //set head spin direction
		headFindBall(-50);
		double dir = (FBDir == ANTICLOCKWISE) ? 1 : -1;
		forward = 0;
		left = 0;
		turnCCW = dir * 25;

	} else if (lostBall <= lBallFastFind) {
		spinFindBall();

	} else {

		//this needs to be much smarter!!

        /* A hack for calling function to use:
         * eg. in doBeckhamNormalForward, -1,-1 is used if there is any other
         * teammate. */
		if (PLAYER == 15 || (findPositionx == -1 && findPositiony == -1)) {
			if (PLAYER_NUMBER == 1) {
				findPositionx = 100;
				findPositiony = 200;
			}
			else if (PLAYER_NUMBER == 2) {
				findPositionx = 160;
				findPositiony = 240;
			}
			else if (PLAYER_NUMBER == 3) {
				findPositionx = 145;
				findPositiony = 220;
			}
			else {
				findPositionx = 145;
				findPositiony = 220;
			}

		} else if (findPositionx == -2 && findPositiony == -2) {
			findPositionx = FIELD_WIDTH / 2.0;
			findPositiony = FIELD_LENGTH / 2.0;
		}    

		Vector target(vCART,
                        findPositionx + 8.0 * cos(DEG2RAD(gps->self().getHackedH())),
                        findPositiony + 8.0 * sin(DEG2RAD(gps->self().getHackedH()))
                     );

		Vector robot(vCART, (double) gps->self().pos.x, (double) gps->self().pos.y);

		//vector to destination point
		Vector vx(robot);
		vx.sub(target);

		double pointAngle2 = utilNormaliseAngle(
                                vx.getHackedTheta() - gps->self().getHackedH()
                             );

		lookdir = FBDir; //set head spin direction
		headFindBall();

		double dir = (FBDir == ANTICLOCKWISE) ? 1 : -1;
		turnCCW = dir * 20;

		forward = -5.0 * cos(DEG2RAD(pointAngle2));
		left = -5.0 * sin(DEG2RAD(pointAngle2));

		if (vx.d < 20) {
			forward /= 4.0;
			left /= 4.0;
		}
	}
}

/* Spin the body on the spot to find ball. 
 * Use the global FBDir variable to determin direction. */
void Behaviours::spinFindBall() {
	headtype = ABS_XYZ;
	tilty = 0;
	cranez = 0;
	if (FBDir == CLOCKWISE) {
		panx = -50;
		turnCCW = -50;
	}
	else {
		panx = 50;
		turnCCW = 50;
	}
}

/* Spin the body on the spot to find ball. 
 * Use the input variable to determin direction. */
void Behaviours::spinFindBall(int dir) {
	headtype = ABS_XYZ;
	tilty = 0;
	cranez = 0;
	if (dir == CLOCKWISE) {
		panx = -50;
		turnCCW = -50;
	}
	else {
		panx = 50;
		turnCCW = 50;
	}
}

void Behaviours::improvedSpinFindBall(int dir) {
	setOmniParams();
	headtype = ABS_XYZ;
	tilty = 0;
	cranez = 0;
	if (dir == CLOCKWISE) {
		panx = -50;
		turnCCW = -40;
		left = 5;
	}
	else {
		panx = 50;
		turnCCW = 40;
		left = -5;
	}
	forward = 0;
	FBDir = dir;
	lostBall = lBallSlowFind + 1;
}

void Behaviours::headFindBall(double lowtilt) {
	static const double hightilt = -5;

	int lowfirst = 0;

	// if haven't started finding ball yet ...
	if (lostBall <= lBallLookGps + 1) {
		// 0 = false, 1 = true
		//lowfirst = (lastBallTilt > (hightilt + lowtilt / 2)) ? 0 : 1;
		lowfirst = 1; //this is changed so that it always looks low first because lastBallTilt wasn't being properly maintained

		lookdir = ((gps->getBall(LOCAL).head >= 0 && lowfirst == 1)
				|| (gps->getBall(LOCAL).head < 0 && lowfirst == 0))
			? CLOCKWISE
			: ANTICLOCKWISE;
	}

	//every head period alternate between low first and high first
	if ((lostBall / (HEAD_PERIOD)) % 2 == lowfirst) {
		// look down first
		panx = -90 + (180 / (HEAD_PERIOD)) * (lostBall % (HEAD_PERIOD));
		tilty = lowtilt + 30;
		cranez = -15;
		// cout << "Look down in headFindBall\n";
	}
	else {
		// look up first
		panx = 90 - (180 / (HEAD_PERIOD)) * (lostBall % (HEAD_PERIOD));
		tilty = hightilt;
		cranez = 5;
		// cout << "Look up in headfindball\n";
	}
	if (lookdir == CLOCKWISE) {
		panx *= 1;
	}
	else {
		panx *= -1;
	}
	headtype = ABS_H;
}


static int locateBallKickCount = 0;
void Behaviours::setLocateBallKickParams() {
	locateBallKickCount = 0;
	setLocateBallParams();
}
void Behaviours::aaLocateBallKick() {
	if (locateBallKickCount < 23) {
		headtype = ABS_H;
		tilty = -15;
		cranez = 0;
		panx = 0;
		if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
			turnCCW = -50;
		}
		else {
			turnCCW = 50;
		}
		locateBallKickCount++;
	}
	else {
		lockMode = ResetMode;
		lostBall = lBallSlowFind + 1; //so it jumps into the cmu find ball part of locate ball
	}
}
