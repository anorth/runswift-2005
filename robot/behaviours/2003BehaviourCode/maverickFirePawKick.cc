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
 * Team E Forward
 *
 **/

#include "maverick.h"
#include "../share/BallDef.h"

static const int vBallLost = 10;
static const int vBallAnywhere = 40;
static const int APPROACH = 0;
static const int ALIGN = 1;
static const int KICK = 2;

static int teamDForwardFBDir;
static int firePaw;
static int fireMode;
static int scounter;
static bool close;



void Maverick::initFirePawKick() {
	lockMode = NoMode;
	lostBall = 0;
	teamDForwardFBDir = ANTICLOCKWISE;
	firePaw = 0;
	fireMode = APPROACH;
	scounter = 0;
	close = false;
}

void Maverick::doFirePawKick() {
	setDefaultParams();
	setWalkParams();
	trackBallHeadParams();

	if (gps->canSee(vobBall)) {
		gps->r.behavioursVal = (int) vBall.d;
		lostBall = 0;
	}
	else {
		lostBall++;
		gps->r.behavioursVal = 500;
	}

	if (mode_ == mStartMatch) {
		// nothing
	}
	else if (lockMode != NoMode && lockMode != ResetMode) {
		switch (lockMode) {
			/*
			   case GrabBall:
			   aaGrabBall();
			   gps->r.ballDist = 0;
			   break;
			   case GotBall:
			   aaGotBall();
			   gps->r.ballDist = 0;
			   break;
			   */
			case ChestPush: aaChestPush();
					gps->r.ballDist = 0;
					break;
			case SpinKick: aaSpinKick();
				       gps->r.ballDist = 0;
				       break;
			case SpinChestPush: aaSpinChestPush();
					    gps->r.ballDist = 0;
					    break;
			case GoalieKick: aaGoalieKick();
					 gps->r.ballDist = 0;
					 break;
			case LightningKick: aaLightningKick();
					    gps->r.ballDist = 0;
					    break;
			case ProperVariableTurnKick: aaProperVariableTurnKick();
						     gps->r.ballDist = 0;
						     break;
		}
	}
	else {
		trackBallHeadParams();
		if (vBall.cf <= 0) {
			if (lostBall <= vBallLost) {
				gpsTrackBallHeadParams();
				return;
			}
			else {
				if (lostBall == vBallLost + 1) {
					setLocateBallParams();
				}
				locateBall(findPositionx, findPositiony); 
				return;
			}
		}
		else {
			/*
			   if (vBall.vob->d < BALL_RADIUS * 1.5 && abs(vBall.vob->h < 30)) {
			   chooseMaverickShootStrategy();
			   } else if (abs(gps->tGoal().pos.head) <= 60) {
			   leds (1,1,2,1,1,2);
			   firePawKick(FIRE_PAWKICK_AUTO);
			   } else if (utilBallOnLEdge() && utilIsOnLEdge() && gps->self().h < 120 && gps->self().h > 60 && gps->self().posVar < get95CF(75)) {
			   firePawKick(FIRE_PAWKICK_LEFT);
			   } else if (utilBallOnREdge() && utilIsOnREdge() && gps->self().h < 120 && gps->self().h > 60 && gps->self().posVar < get95CF(75)) {
			   firePawKick(FIRE_PAWKICK_RIGHT);
			   } else {
			   leds (2,1,2,2,1,2);
			   fireApproach();
			   }
			   */
			/*
			   if (utilBallOnLEdge() && utilIsOnLEdge() && gps->self().h < 120 && gps->self().h > 60 && gps->self().posVar < get95CF(75)) {
			   firePawKick(FIRE_PAWKICK_LEFT);
			   } else if (utilBallOnREdge() && utilIsOnREdge() && gps->self().h < 120 && gps->self().h > 60 && gps->self().posVar < get95CF(75)) {
			   firePawKick(FIRE_PAWKICK_RIGHT);
			   } else if (utilBallOnTEdge() && utilIsOnTEdge() && gps->self().h > 150 && gps->self().h < 210 && gps->self().posVar < get95CF(75)) {
			   firePawKick(FIRE_PAWKICK_RIGHT);
			   } else if (utilBallOnTEdge() && utilIsOnTEdge() && (gps->self().h < 30 || gps->self().h > 330) && gps->self().posVar < get95CF(75)) {
			   firePawKick(FIRE_PAWKICK_LEFT);
			   } else {
			   leds (2,1,2,2,1,2);
			   firePawKick(FIRE_PAWKICK_AUTO);
			   }
			   */
			//firePawKick(FIRE_PAWKICK_AUTO);
			if (fireMode != KICK)
				firePawKickAim();
			else
				firePawKick(FIRE_PAWKICK_AUTO);
		}
	}
}

/////////////////////////////////////////////////
// START FIREPAWKICK FUNCTIONS
/////////////////////////////////////////////////

// approaches ball and paw kicks with which ever paw nearer the ball
// try to align ball with designated paw
// switch paw if one rolls past opposite paw
// note this kicks straight
void Maverick::firePawKick(int paw, double offset) {
	double ballx = sin(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
	double bally = cos(DEG2RAD(vBall.vob->h)) * vBall.vob->d;

	// decide on a paw. once decided stick with it until ball rolls past the other paw
	// try to avoid problem with switching btwn left and right paws
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

	double target_x = (firePaw == FIRE_PAWKICK_RIGHT)
		? ballx
		+ offset
		: ballx
		- offset;
	double target_y = bally - FIRE_PAWKICK_YOFFSET;
	double target_d = sqrt(target_x *target_x + target_y *target_y);
	double target_h = RAD2DEG(asin(target_x / target_d));

	// run at ball
	turnCCW = CLIP(target_h / 3, 30.0);
	left = 0;
	if (abs(target_x) <= 1.0 && target_d < BALL_RADIUS * 1.5) {
		walkType = ZoidalWalkWT;
		forward = MAX_OFFSET_FORWARD;
		turnCCW = CLIP(turnCCW, 10.0);
		//        walkType = CanterWalkWT;
		//        forward = FIRE_PAWKICK_APPROACH;
	}
	else if (abs(turnCCW) > 15) {
		walkType = CanterWalkWT;
		forward = (abs(turnCCW) < 20) ? FIRE_PAWKICK_APPROACH : 0.5;
	}
	else {
		walkType = ZoidalWalkWT;
		forward = MAX_OFFSET_FORWARD;
		//        walkType = CanterWalkWT;
		//        forward = FIRE_PAWKICK_APPROACH;
	}
}

void Maverick::firePawKickAim() {
	// first, find the point we want to get to
	double ballh = vBall.vob->h;
	double goalh = gps->tGoal().pos.head;
	double ballx = sin(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
	double bally = cos(DEG2RAD(vBall.vob->h)) * vBall.vob->d;

	firePaw = (ballh > goalh) ? FIRE_PAWKICK_RIGHT : FIRE_PAWKICK_LEFT;
	double target_x = (firePaw == FIRE_PAWKICK_RIGHT)
		? ballx
		+ FIRE_PAWKICK_XOFFSET
		: ballx
		- FIRE_PAWKICK_XOFFSET;  
	double target_y = bally - FIRE_PAWKICK_YOFFSET;
	double target_d = sqrt(target_x *target_x + target_y *target_y);
	//    double target_h = RAD2DEG(asin(target_x / target_d));
	/*    
	      if (scounter >= 0) {
	      leds(2,2,2,2,2,2);
	      walkType = ZoidalWalkWT;
	      turnCCW = (firePaw == FIRE_PAWKICK_RIGHT) ? -15 : 15;
	      forward = MAX_OFFSET_FORWARD;
	//setStandParams();
	scounter--;
	} else if (abs(target_x) <= 2.0 && target_d < BALL_RADIUS * 3.0) {
	leds(2,2,2,2,2,2);
	scounter = 24;
	walkType = CanterWalkWT;
	turnCCW = (firePaw == FIRE_PAWKICK_RIGHT) ? -50 : 50;
	forward = 1.5;
	} else {
	// run at ball
	turnCCW = CLIP(target_h/3, 30.0);
	if (abs(turnCCW) > 10 || target_d < 15) {
	if (target_d < 15) {
	PG = 20;
	walkType = CanterWalkWT;
	CLIP(turnCCW, 10.0);
	forward = 3.0;
	} else {
	walkType = CanterWalkWT;
	forward = (abs(turnCCW) < 20) ? FIRE_PAWKICK_APPROACH : 0.5;
	}
	} else {
	walkType = ZoidalWalkWT;
	forward = MAX_OFFSET_FORWARD;
	}
	}        
	*/

	double xvec = gps->getBall(GLOBAL).x - FIELD_WIDTH / 2.0;
	double yvec = gps->getBall(GLOBAL).y - FIELD_LENGTH;

	double vecd = sqrt(SQUARE(yvec) + SQUARE(xvec));
	double g_targetx = FIELD_WIDTH / 2.0 + xvec*(vecd + 25) / vecd;
	double g_targety = FIELD_LENGTH + yvec*(vecd + 25) / vecd;

	double lx = g_targetx - gps->self().pos.x;
	double ly = g_targety - gps->self().pos.y;

	Vector l_target(vCART, lx, ly);
	l_target.rotate(90 - gps->self().h);

	double target_h = l_target.head;

	if (fireMode == KICK) {
		cout << "kick" << endl;
		leds(2, 2, 2, 2, 2, 2);
		firePawKick(firePaw);
	}
	else if (fireMode == ALIGN) {
		cout << "align" << endl;
		leds(2, 1, 2, 2, 1, 2);
		if (abs(ballh) < 15) {
			fireMode = KICK;
			firePaw = (ballh > goalh) ? FIRE_PAWKICK_LEFT : FIRE_PAWKICK_RIGHT;
			firePawKick(firePaw);
			scounter = 24;
		}
		else {
			turnCCW = CLIP(ballh / 3, 40.0);
			walkType = CanterWalkWT;
		}
	}
	else if (abs(ballh - goalh) < 15 && vBall.vob->d < 30) {
		cout << "approach" << endl;
		fireMode = ALIGN;
		if (abs(ballh) < 15) {
			fireMode = KICK;
			firePaw = (ballh > goalh) ? FIRE_PAWKICK_LEFT : FIRE_PAWKICK_RIGHT;
			firePawKick(firePaw);
			scounter = 24;
		}
		else {
			turnCCW = CLIP(ballh / 3, 40.0);
			walkType = CanterWalkWT;
		}
	}
	else {
		cout << "approach" << endl;
		// readjust
		fireMode = APPROACH;
		scounter = 0;
		turnCCW = CLIP(target_h / 3, 30.0);
		if (abs(turnCCW) > 15) {
			leds(2, 1, 1, 2, 1, 1);
			walkType = CanterWalkWT;
			forward = (abs(turnCCW) < 20) ? FIRE_PAWKICK_APPROACH : 0.5;
		}
		else {
			leds(1, 1, 2, 1, 1, 2);
			walkType = ZoidalWalkWT;
			forward = MAX_OFFSET_FORWARD;
		}
	}
}

// approach ball similar to a hold
void Maverick::fireApproach() {
	turnCCW = CLIP(vBall.h / 3, 40.0);
	if (abs(turnCCW) > 15 || vBall.d < 20) {
		walkType = CanterWalkWT;
		forward = (abs(turnCCW) < 20) ? FIRE_PAWKICK_APPROACH : 0.5;
	}
	else {
		walkType = ZoidalWalkWT;
		forward = MAX_OFFSET_FORWARD;
	}
}


/////////////////////////////////////////////////
// END MAVERICK FORWARD FUNCTIONS
/////////////////////////////////////////////////
