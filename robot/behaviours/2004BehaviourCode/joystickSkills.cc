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
 * $Id
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * Beckham Forward
 *
 **/ 

#include "joystickSkills.h"
#include "Behaviours.h"
#include "../share/BallDef.h"
#include "../share/Common.h"
#include "UNSW2004.h"

#ifdef COMPILE_ALL_CPP

namespace JoystickSkills {
	JoystickState jsState = nothingJS;
}

bool ballTrk = false;
double sign(double x);

static const int vBallLost = 10;
static const bool ballTrkDebug = false;
static double ballH = 0;
static double ballD = 0;
static int inRange = 0; //used as a boolean to say whether or not the
//ball is in the auto kickable range..
//implemented as a int so i can do hyteresis
static int blockCount = 0;
static int kickTimeout = 0;
static const int stateTimeout = (int) 3.5 * 25; //frames

void JoystickSkills::jsMaintenance() {
	DECREMENT(blockCount);
	DECREMENT(kickTimeout);

	//absolute timeout on a kick so that
	//the dog doesn't fumble+chase the ball forever
	if (jsState != nothingJS && jsState != blockJS && kickTimeout <= 0) {
		jsState = nothingJS;
		cmdStance = NORMAL_STANCE;
	} 
	if (jsState == nothingJS) {
		kickTimeout = 0;
	}

	if (vBall.cf > 3) {
		ballH = vBall.h;
		ballD = vBall.d;
	}
	else {
		//dont change ballH ballD
	}

	//hysterisis on losing sight of the ball
	if (ballD < 33 && abs(ballH) < 45) {
		inRange = 5;
	}
	else if (ballD >= 40 || abs(ballH) >= 50) {
		inRange--;
		inRange = max(0, inRange);
	}
	//this statement must go after the inRange checks
	if (lostBall > vBallLost) {
		inRange = 0;
	}
}

bool JoystickSkills::isBallKickable() {
	return inRange;
}

void JoystickSkills::setBallTrk(bool b) {
	ballTrk = b;
	return;
}

void JoystickSkills::jsGeneralHead() {
	if (ballTrk) {
		if (ballTrkDebug)
			cout << "@@ ballTrk ON";
		if (gps->canSee(vobBall)) {
			if (ballTrkDebug)
				cout << " - TrackVisualBall" << endl;
			UNSW2004::doUNSW2004TrackVisualBall();
		}
		else if (lostBall <= vBallLost && abs(gps->getBall(LOCAL).head) < 90) {
			if (ballTrkDebug)
				cout << " - TrackGPSBall" << endl;
			UNSW2004::doUNSW2004TrackGpsBall();
		}
		else if (lostBall <= (int) (3 * 25)) {
			remoteHeadFindBall(-10, -45);
		}
		else {
			if (ballTrkDebug)
				cout << " - LostBall" << endl;
		}
	}
	else {
		if (ballTrkDebug)
			cout << "@ ballTrk OFF" << endl;
		headtype = REL_H;
		panx = cmdPan;
		tilty = cmdTilt;
		cranez = cmdCrane;
	}
}

void JoystickSkills::lightningJK() {
	if (isBallKickable()) {
		if (kickTimeout <= 0) {
			kickTimeout = stateTimeout;
		}
		jsState = lightningJS;
		jsGeneralHead();
		if (UNSW2004::isUNSW2004BallUnderChin()) {
			jsState = nothingJS;
			lockMode = LightningKick;
			aaLightningKick();
		}
		else {
			UNSW2004::hoverToBall(ballD, ballH);
		}
	}
	else {
		jsState = nothingJS;
		cmdStance = NORMAL_STANCE;
	}
	return;
}

void JoystickSkills::chestPushJK() {
	if (isBallKickable()) {
		if (kickTimeout <= 0) {
			kickTimeout = stateTimeout;
		}
		jsState = chestPushJS;
		jsGeneralHead();
		if (UNSW2004::isUNSW2004BallUnderChin()) {
			jsState = nothingJS;
			lockMode = ChestPush;
			aaChestPush();
		}
		else {
			UNSW2004::hoverToBall(ballD, ballH);
		}
	}
	else {
		jsState = nothingJS;
		cmdStance = NORMAL_STANCE;
	}
	return;
}

void JoystickSkills::tk90LeftJK() {
	if (isBallKickable()) {
		if (kickTimeout <= 0) {
			kickTimeout = stateTimeout;
		}
		jsState = tk90LeftJS;
		jsGeneralHead();
		if (UNSW2004::isUNSW2004BallUnderChin()) {
			jsState = nothingJS;
			lockMode = ProperVariableTurnKick;
			setProperVariableTurnKick(90);
			aaProperVariableTurnKick();
		}
		else {
			UNSW2004::hoverToBall(ballD, ballH);
		}
	}
	else {
		jsState = nothingJS;
		cmdStance = NORMAL_STANCE;
	}
	return;
}

void JoystickSkills::tk90RightJK() {
	if (isBallKickable()) {
		if (kickTimeout <= 0) {
			kickTimeout = stateTimeout;
		}
		jsState = tk90RightJS;
		jsGeneralHead();
		if (UNSW2004::isUNSW2004BallUnderChin()) {
			jsState = nothingJS;
			lockMode = ProperVariableTurnKick;
			setProperVariableTurnKick(-90);
			aaProperVariableTurnKick();
		}
		else {
			UNSW2004::hoverToBall(ballD, ballH);
		}
	}
	else {
		jsState = nothingJS;
		cmdStance = NORMAL_STANCE;
	}
	return;
}

void JoystickSkills::tk180LeftJK() {
	if (isBallKickable()) {
		if (kickTimeout <= 0) {
			kickTimeout = stateTimeout;
		}
		jsState = tk180LeftJS;
		jsGeneralHead();
		if (UNSW2004::isUNSW2004BallUnderChin()) {
			jsState = nothingJS;
			lockMode = ProperVariableTurnKick;
			setProperVariableTurnKick(180);
			aaProperVariableTurnKick();
		}
		else {
			UNSW2004::hoverToBall(ballD, ballH);
		}
	}
	else {
		jsState = nothingJS;
		cmdStance = NORMAL_STANCE;
	}
	return;
}

void JoystickSkills::tk180RightJK() {
	if (isBallKickable()) {
		if (kickTimeout <= 0) {
			kickTimeout = stateTimeout;
		}
		jsState = tk180RightJS;
		jsGeneralHead();
		if (UNSW2004::isUNSW2004BallUnderChin()) {
			jsState = nothingJS;
			lockMode = ProperVariableTurnKick;
			setProperVariableTurnKick(-180);
			aaProperVariableTurnKick();
		}
		else {
			UNSW2004::hoverToBall(ballD, ballH);
		}
	}
	else {
		jsState = nothingJS;
		cmdStance = NORMAL_STANCE;
	}
	return;
}

void JoystickSkills::blockJK() {
	if (jsState != blockJS) {
		jsState = blockJS;
		blockCount = 18;
		setBlockParams();
	}
	else if (jsState == blockJS && blockCount > 0) {
		setBlockParams();
	}
	else {
		jsState = nothingJS;
		cmdStance = NORMAL_STANCE;
	}
	return;
}






// num frames in 1/2 cycle of head finding ball
static const int HEAD_PERIOD = 18;
static const int lBallLookGps = 10;   

void JoystickSkills::remoteHeadFindBall(double hightilt, double lowtilt) {
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
	if ((lostBall / HEAD_PERIOD) % 2 == lowfirst) {
		// look down first
		panx = -90 + (180 / HEAD_PERIOD) * (lostBall % HEAD_PERIOD);
		tilty = lowtilt;
	}
	else {
		// look up first
		panx = 90 - (180 / HEAD_PERIOD) * (lostBall % HEAD_PERIOD);
		tilty = hightilt;
	}
	if (lookdir == CLOCKWISE) {
		panx *= 1;
	}
	else {
		panx *= -1;
	}
	panx = panx / 2.0; 
	headtype = ABS_H;
}




static const double maxCanterForwardSpeed = 8;
static const double maxCanterLeftSpeed = 6;

void JoystickSkills::hoverCrop(double f,
		double l,
		double t,
		double *fr,
		double *lr,
		double *tr,
		int *wt) {
	*tr = t;

	if (abs(t) <= 15 && f > 0 && abs(l) < 3) {
		*wt = ZoidalWalkWT;
		*lr = 0;
		*fr = f;

		//this forces it to max out
		/*
		   if (abs(t) <= 8) {  
		 *fr = MAX_OFFSET_FORWARD_SMALL_TURN;
		 } else {
		 *fr = MAX_OFFSET_FORWARD_LARGE_TURN;
		 }
		 */
	}
	else {
		*wt = CanterWalkWT;

		double maxF = maxCanterForwardSpeed;
		double maxL = maxCanterLeftSpeed;

		if (t < 10 && t >= 0) {
			maxF = 5.5;
			maxL = 5.5;
		} 
		// counter clockwise case turning faster
		else if (t < 20 && t >= 0) {
			maxF = 4.5;
			maxL = 4.5;
		}
		// turn too fast anticlockwise case
		else if (t >= 0) {
			maxF = 3;
			maxL = 2;
		}
		// slow clockwise turn case
		else if (t > -10 && t < 0) {
			maxF = 5;
			maxL = 5;
		}
		// all other clockwise cases
		else {
			maxF = 1;
			maxL = 3;
		}

		// calculate how much you should go forward and left whilst turning
		double fComp = f;
		double lComp = l;

		if (f == 0 && l == 0) {
			*fr = 0;
			*lr = 0;
		}
		else if (f == 0) {
			*fr = 0;
			*lr = sign(l) * MIN(abs(l), maxL);
		}
		else if (l == 0) {
			*fr = sign(f) * MIN(abs(f), maxL);
			*lr = 0;
		}
		else if (abs(f) > maxF || abs(l) > maxL) {
			//scale it back so that you walk in the same direction
			//but with values under the threshold
			double scale;
			if (maxF * ABS(fComp) >= maxL * ABS(lComp)) {
				scale = 1.0 / ABS(fComp);
			}
			else {
				scale = 1.0 / ABS(lComp);
			}

			fComp = scale * fComp;
			lComp = scale * lComp;

			*fr = maxF * fComp;
			*lr = maxL * lComp;
		}
		else {
			*fr = f;
			*lr = l;
		}
	}
}

double sign(double x) {
	if (x >= 0) {
		return +1;
	}
	else {
		return -1;
	}
}


#endif //COMPILE_ALL_CPP
