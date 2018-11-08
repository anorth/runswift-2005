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
 * UNSW 2003 Robocup
 *
 * Last modification background information
 * $Id: pawKick.cc 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#include "Behaviours.h"

inline bool Behaviours::pkCanPawKick() {
	return vBall.vob->getPerpDistance() - NECK_TO_PAWTIP < 8;
}

void Behaviours::pkGoToBall(double offset) {
	Vector target(vCART,
			vBall.vob->getLateralOffset(),
			vBall.vob->getPerpDistance());
	Vector robOffset(vCART, offset, NECK_TO_PAWTIP);
	Vector vx = target - robOffset;

	// The ball is offset too much
	if (ABS(vx.x) > 10.0) {
		forward = 2.0;
		left = CLIP(vx.x, 10.0);
	}
	else if (ABS(vx.head) > 30) {
		//oriented incorrectly
		//turn 
		forward = 2.0;
		left = CLIP((vx.x), 2.0);
		turnCCW = CLIP(vx.head / 2, 30.0);
	}
	else {
		// can see the ball comfortably
		//use default forward paramater
		if (forward == 0.0)
			forward = 6.0;
		double divisor = 2;
		if (vx.y < 20)
			divisor = 3;
		else if (vx.y < 15)
			divisor = 4;
		left = CLIP(vx.x, 3.0) / divisor;
		double move = sqrt(SQUARE(forward) + SQUARE(left));
		double maxTurn = 9 - move;
		turnCCW = CLIP(vx.head / 2, maxTurn);
	}
}

inline void Behaviours::pkMakeZoidalWalk(double F, double L, double t) {
	PG = 40,
	hF = 70,
	hB = 110,
	hdF = 5,
	hdB = 35,
	ffO = 60,
	fsO = 5,
	bfO = -55,
	bsO = 10;
	walkType = ZoidalWalkWT, forward = F, left = L, turnCCW = t;
}

inline PawKickState Behaviours::pkChoosePawKick() {
	// && the dkd == ball's side (from hPan)
	if (vBall.vob->getPerpDistance() - NECK_TO_PAWTIP < 2
			&& vBall.vob->getFlatDistance() < 20) {
		forward = 0.5;
		left = wmBall.h < 0 ? -12.0 : 12.0;
		turnCCW = CLIP(wmtGoal.h, 3.0);    // change to dkd?
		return LATERAL_KICK;
	}
	if (pkCanPawKick()) {
		pkMakeZoidalWalk(MAX_OFFSET_FORWARD, 0, 0);
		if (tilty < -20) {
			tilty = -20;
		}
		return PAW_KICK;
	}
	if (forward == 6.0) {
		forward = 8.0;
	}
	pkMakeZoidalWalk(forward, left, turnCCW);
	return RUNUP;
}

PawKickState Behaviours::saPawKick(double paw) {
	pkGoToBall(paw);
	return pkChoosePawKick();
}

PawKickState Behaviours::saDribble() {
	static double paw = LEFT_PAW;
	static unsigned int lastFrame = 0;

	// change paw kick foot if necessary
	if (vBall.vob->getLateralOffset() < 2 * RIGHT_PAW) {
		paw = RIGHT_PAW;
		//wout << "right" << vBall.vob->getLateralOffset() <<"\n";
	}
	else if (vBall.vob->getLateralOffset() > 2 * LEFT_PAW) {
		paw = LEFT_PAW;
		// wout << "left --> " << vBall.vob->getLateralOffset() <<"\n";
	}
	else if (frame - lastFrame > 6) {
		// initialises the paw kick
		if (gps->getBall(LOCAL).head < 0) {
			paw = RIGHT_PAW;
		}
		else {
			paw = LEFT_PAW;
		}
		// wout << "frame reinit\n";

	}
	/*
	   if (frame%3==0) {
	   wout << "--> " << vBall.vob->getLateralOffset() <<"\n";
	   }
	   */
	lastFrame = frame;
	pkGoToBall(paw);
	return pkChoosePawKick();
}
