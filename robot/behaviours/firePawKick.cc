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
#include "../vision/InfoManager.h"
#include "../share/BallDef.h"
#include <cmath>

static const double FIRE_PAWKICK_YOFFSET = BALL_RADIUS;
static const double FIRE_PAWKICK_OFFSET = 7.5;
static const double FIRE_PAWKICK_APPROACH = 7.5;

// firePaw is zero initally, so without paw assignment it is assigned as left paw.
static int firePaw;

// approaches ball and paw kicks with which ever paw nearer the ball
// try to align ball with designated paw
// switch paw if one rolls past opposite paw
// note this kicks straight
void Behaviours::firePawKick(int paw, double offset) {
    using namespace Behaviours;
	double ballx = sin(DEG2RAD(vBall.vob->head)) * vBall.vob->dist2;
	double bally = cos(DEG2RAD(vBall.vob->head)) * vBall.vob->dist2;

	// The 2003 offset is 6 and it is not too bad.

	// Decide on a paw. once decided stick with it until ball rolls past the other paw
	// try to avoid problem with switching btwn left and right paws.
	if (paw == FIRE_PAWKICK_AUTO) {
		if (firePaw == FIRE_PAWKICK_LEFT && ballx < -offset / 2) {
			firePaw = FIRE_PAWKICK_RIGHT;
		}
		else if (firePaw == FIRE_PAWKICK_RIGHT && ballx > offset / 2) {
			firePaw = FIRE_PAWKICK_LEFT;
		}
	}
	else {
		firePaw = paw;
	}

	double target_x = (firePaw == FIRE_PAWKICK_RIGHT) ? ballx + offset : ballx - offset;
	double target_y = bally - FIRE_PAWKICK_YOFFSET;
	double target_d = sqrt(target_x *target_x + target_y *target_y);
	double target_h = RAD2DEG(asin(target_x / target_d));

	// Run at ball
	turnCCW = CLIP(target_h / 3, 30.0);
	Behaviours::left = 0;
	
	// Once the ball is lined up and well within range for the forward, the robot should
	// commit to charge at the ball at full speed.	
	if (abs(target_x) <= 1.0 && target_d < BALL_RADIUS * 1.5) {
		walkType = NormalWalkWT;
		turnCCW = CLIP(turnCCW, 10.0);
		forward = 7.0;   // Fastest speed we currently have.
	 /*
		if (abs(turnCCW) <= 8) {
			// forward = MAX_OFFSET_FORWARD_SMALL_TURN;
			forward = 7.0;
		}
		else {
			forward = MAX_OFFSET_FORWARD_LARGE_TURN;
		}
	  */				
	}
	// If need to turn at a larger degrees.
	else if (abs(turnCCW) > 13.75) {
		walkType = NormalWalkWT;
             // forward = (abs(turnCCW) < 20) ? FIRE_PAWKICK_APPROACH : 0.5;
	        forward = (abs(turnCCW) < 20) ? 7.0 : 0.5; // 7.0 is the fastest speed we currently have.
	}
	// The ball is not lined up and not turning at a very large degress, so go a full speed.
	else {
		walkType = NormalWalkWT;
		forward = 7;  // The current fastest speed.
	}
}

void firePawKickElliptical(int paw, double offset) {
    using namespace Behaviours;
	double ballx = sin(DEG2RAD(vBall.vob->head)) * vBall.vob->dist2;
	//double bally = cos(DEG2RAD(vBall.vob->head)) * vBall.vob->dist2;

    double h = gInfoManager.getBallHeading();
    double d = gInfoManager.getBallDistance();
	if (paw == FIRE_PAWKICK_AUTO) {
		if (firePaw == FIRE_PAWKICK_LEFT && ballx < -offset / 2) {
			firePaw = FIRE_PAWKICK_RIGHT;
		}
		else if (firePaw == FIRE_PAWKICK_RIGHT && ballx > offset / 2) {
			firePaw = FIRE_PAWKICK_LEFT;
		}
	}
            
    double offHeading = RAD2DEG( asin(offset / d) );
    printf( "h = %3.1f , off = %3.1f " , h,offHeading);
    if (paw == FIRE_PAWKICK_RIGHT )
        h += offHeading;
    else
        h -= offHeading;
            
    setWalkParams();//Normal Walk Fast
    if (d > 30 || h < 40){
        walkType = EllipseWalkWT;
    }
    turnCCW = CLIP(h, 40.0);
    printf(" turn = %3.1f" , turnCCW);
            
}

