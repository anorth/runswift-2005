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

#include "Behaviours.h"
#include "../share/BallDef.h"

static const double OFFEDGE_KICK_YOFFSET = BALL_RADIUS * 3.0;
static const double OFFEDGE_KICK_APPROACH = 7.5;

// this is how it works:
// PHASE 1 walk up beside the ball.
// PHASE 2 start side stepping, to nudge the ball out
//         add a gentle turn to prevent ball from being stuck
//         the turn param is tricky, if too much then walk up against
//         ramp and cant side step. none of the parametres are really
//         very well tuned, so future generations feel free to change it
//         also you can knock the ball pretty far if you turn up left param
// PHASE 3 keep side stepping/turning until either the ball pops out
//         or we time out.

void Behaviours::offEdgeKick(int paw, double offset) {
	double ballx = sin(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
	double bally = cos(DEG2RAD(vBall.vob->h)) * vBall.vob->d;

	// decide on a paw. once decided stick with it until ball rolls past the other paw
	// try to avoid problem with switching btwn left and right paws
	if (paw == OFFEDGE_KICK_AUTO) {
		if (Behaviours::offEdgePaw == OFFEDGE_KICK_LEFT
				&& ballx < -BALL_RADIUS * 1.5) {
			Behaviours::offEdgePaw = OFFEDGE_KICK_RIGHT;
		}
		else if (Behaviours::offEdgePaw == OFFEDGE_KICK_RIGHT
				&& ballx > BALL_RADIUS * 1.5) {
			Behaviours::offEdgePaw = OFFEDGE_KICK_LEFT;
		}
	}
	else {
		Behaviours::offEdgePaw = paw;
	}

	double target_x = (Behaviours::offEdgePaw == OFFEDGE_KICK_RIGHT)
		? ballx
		+ offset
		: ballx
		- offset;
	double target_y = bally - OFFEDGE_KICK_YOFFSET;
	double target_d = sqrt(target_x *target_x + target_y *target_y);
	double target_h = RAD2DEG(asin(target_x / target_d));

	// run at ball
	turnCCW = CLIP(target_h / 3, 30.0);
	left = 0;
	leds(2, 2, 2, 2, 2, 2);
	if (Behaviours::offEdgeCounter) {
		if (abs(bally) > 3.0 * BALL_RADIUS) {
			Behaviours::offEdgeCounter = 0;
		}
		else {
			left = (Behaviours::offEdgePaw == OFFEDGE_KICK_LEFT) ? 7.0 : -7.0;
			turnCCW = (Behaviours::offEdgePaw == OFFEDGE_KICK_LEFT) ? -5.0 : 5.0;
			walkType = CanterWalkWT;
			Behaviours::offEdgeCounter--;
		}
	}
	else if (abs(target_x) <= 4.0 && target_y < 0) {
		left = (Behaviours::offEdgePaw == OFFEDGE_KICK_LEFT) ? 7.0 : -7.0;
		turnCCW = (Behaviours::offEdgePaw == OFFEDGE_KICK_LEFT) ? -5.0 : 5.0;
		walkType = CanterWalkWT;
		Behaviours::offEdgeCounter = 18;
	}
	else if (abs(turnCCW) > 15) {
		walkType = CanterWalkWT;
		forward = (abs(turnCCW) < 20) ? 7.5 : 0.5;
	}
	else {
		walkType = ZoidalWalkWT;
		forward = MAX_OFFSET_FORWARD;
	}
}
