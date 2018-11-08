/*

   Copyright 2004 The University of New South Wales (UNSW) and National  
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
 * $Id: utility.cc 6487 2005-06-08 11:35:39Z alexn $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

/*
 * Strategy:
 *  utility functions
 *
 * Description:
 *
 * Strategy member variables used:
 *  dkd
 *  prevAttackMode
 *  lostBall
 *  ballUnderChinTilt
 *  isWirelessDown
 *
 * Local variables used:
 *  teammateSeenTime
 *  backoffTeammteBall
 *  backoffTeammate
 *
 *  panDirection
 *  prevPan
 *
 * Restrictions:
 *  none
 *
 * Comments:
 *  all the utility functions that involve
 *  teammates and opponents might have to
 *  be changed to use the vision model instead;
 *  currently, everything is based on the world 
 *  model
 *
 *  do not use utilOpponentInFace
 */

#include "Behaviours.h"

// num frames to back off once teammate and ball seen together
static int backoffTeammateBall = 0;

// num frames to back off once teammate seen
static int backoffTeammate = 0;

// pan direction for looking around for beacons
//static int panDirection = 1;         
static double prevPan = 0;        

// Ted: This function is called from beckhamGoalie.cc. Fix it when GPS and vision complete.
void Behaviours::setTeammateSeenTime() {
	//  commented out temporarily. we use GPS once its fixed

	/*
	// see if teammate in front     
	bool teammateSeen = false;
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
	if (i == PLAYER_NUMBER)
	continue;
	if (gps->teammate(i).posVar < get95CF(25)) {
	teammateSeenTime++;
	teammateSeen = true;
	break;
	}
	}  
	if (!teammateSeen) {
	teammateSeenTime = 0;
	} 
	// see if teammate in front 
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
	if (vTeammate[i].var < get95CF (25)) {
	teammateCounter = 15;
	teammateSeenTime++;
	break;
	}
	if (i == NUM_TEAM_MEMBER - 1) {
	teammateSeenTime = 0;
	}
	}
	*/
}

bool Behaviours::shouldBackOffTeammateWithBall() {
	return (backoffTeammate != 0);
}

void Behaviours::decrementBackOff() {
	DECREMENT(backoffTeammateBall);
	DECREMENT(backoffTeammate);
}

bool Behaviours::utilIs3rdClosest() {
	int closeRank = 0;

	// check if teamate in front with ball
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (gps->teammate(i).posVar > get95CF(75))
			continue;

		if ((uint32)i != PLAYER_NUMBER) {
			double tempDist = getDistanceBetween(gps->teammate(i, 'g').pos.x,
					gps->teammate(i, 'g').pos.y,
					gps->getBall(GLOBAL).x,
					gps->getBall(GLOBAL).y);

			// increment counter whenever a teammate is 
			// closer to the ball
			if (tempDist < gps->getBall(LOCAL).d)
				closeRank++;
		}
	}
	return (closeRank == 2);
}

bool Behaviours::utilBallInLeftHalf() {
	return gps->getBall(GLOBAL).x < FIELD_WIDTH / 2;
}

// Determine the desired kicking direction according to the robot's current position and
// its global heading.
void Behaviours::utilDetermineDKD() {
	if (utilBallOnLREdge() && gps->self().pos.y < FIELD_LENGTH * 3.0 / 4.0) {
		Vector r;
		r = Vector(vPOLAR, 1, M_PI / 2);
		r.rotate(-DEG2RAD(gps->self().h));
		dkd = RAD2DEG(r.getTheta());
	}
	else if (gps->self().pos.y > FIELD_LENGTH / 2) {
		dkd = gps->tGoal().pos.getHead();
	}
	else {
		dkd = utilNormaliseAngle(gps->oGoal().pos.getHead() + 180);
	}
}

/* Commented because not used, keep here for future reference.
void Behaviours::utilDetermineDKDaimAtCorner() {
	//--- right side of field ------------------------------------
	if (gps->self().pos.x > FIELD_WIDTH / 2) {
		double diffx = FIELD_WIDTH - gps->self().pos.x;
		double diffy = FIELD_LENGTH - gps->self().pos.y;
		double head = RAD2DEG(atan2(diffy, diffx));
		dkd = head;

		//--- left side of field -------------------------------------
	}
	
	// diffx = gps->self().pos.x because the corner is located at x = 0.	
	else {
		double diffx = gps->self().pos.x;
		double diffy = FIELD_LENGTH - gps->self().pos.y;
		double head = RAD2DEG(atan2(diffy, diffx));
		dkd = head;
	}
}
*/

/*
 * Return true if it is blocked by robots in front
 */
bool Behaviours::utilIsFrontObstructed() {
	//--- vision -------------------------------------------------
	for (int i = vobRedDog; i <= vobBlueDog4; i++) {
		if (vision->vob[i].cf > 0 && vision->vob[i].dist3 < 40) {
			if (ABS(vision->vob[i].head) < 15) {
				return true;
			}
		}
	}

	//--- gps ----------------------------------------------------
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (gps->teammate(i).posVar > get95CF(50) && gps->teammate(i).pos.d < 25) {
			if (gps->teammate(i).pos.head > 70 && gps->teammate(i).pos.head < 110) {
				return true;
			}
		}
	}

	for (int i = 0; i < NUM_OPPONENTS; i++) {
		if (gps->getOppPos(i).d < 40) {
			if (gps->getOppPos(i).head > 70 && gps->getOppPos(i).head < 110) {
				return true;
			}
		}
	}

	return false;
}

/*
 * Return true if opponent robot(s) is within the given distance
 */
bool Behaviours::utilOppWithinDist(double dist) {
	//--- vision -------------------------------------------------
	for (int i = gps->vobMinOpponent; i <= gps->vobMaxOpponent; i++) {
		if (vision->vob[i].cf > 0 && vision->vob[i].dist3 < dist) {
			return true;
		}
	}

	//--- gps ----------------------------------------------------
	for (int i = 0; i < NUM_OPPONENTS; i++) {
		if (gps->getOppPos(i).d < dist) {
			return true;
		}
	}

	return false;
}

bool Behaviours::utilNeedVisualBackoff() {
	// check if teammate in front of you with ball
	//cout << "teammate seen time" << teammateSeenTime << endl;
	for (int i = 0; i < NUM_TEAM_MEMBER /*&& teammateSeenTime > 4*/; i++) {
		if (vTeammate[i].var < get95CF(50)
				&& ABS(vTeammate[i].h) < 45
				&& vBall.cf > 0
				&& vBall.d < 50
				&& vTeammate[i].d < vBall.d + 30) {
			//cout << "teammate , ball " << vTeammate[i].d << " , " << vBall.d << " \t\tdifference " << (vTeammate[i].d - vBall.d) << endl;
			// do not lock into backoff mode for 20 frames if dog facing right way
			if (gps->self().h > 20 && gps->self().h < 160) {
				if (vTeammate[i].y < vBall.y)
					backoffTeammateBall = 24;
				else
					backoffTeammateBall = 1;
			}
			else {
				backoffTeammateBall = 20;
			}
			return true;
		}
	}

	// check if teammate in front of you regardless of  ball
	for (int i = 0; i < NUM_TEAM_MEMBER /*&& teammateSeenTime > 4*/; i++) {
		if (vTeammate[i].var < get95CF(50)
				&& ABS(vTeammate[i].h) < 45
				&& vTeammate[i].d < 30) {
			backoffTeammate = 20;
			backoffTeammateIndex = i;
			//forward = -4;
			//cout << "teammate dist head" << vTeammate[i].d << " , " << ABS(vTeammate[i].h) << endl;
			return true;
		}
	}

	if (backoffTeammateBall || backoffTeammate) {
		return true;
	}

	return false;
}

bool Behaviours::utilNeedGpsBackoff() {
	for (int i = 1; i <= NUM_TEAM_MEMBER; i++) {
		//for all people infront of you using gps and further than the ball {
		if ((uint32)i != PLAYER_NUMBER
				&& gps->tmObj(i - 1).counter > 0
				&& gps->teammate(i).posVar <= get95CF(50)
				&& gps->getBallMaxVar() < get95CF(50)
				&& abs(gps->teammate(i).pos.head) < 45
				&& gps->teammate(i).pos.d < gps->getBall(LOCAL).d + 20) {
			// do not lock into backoff mode for 20 
			// frames if dog facing right way
			if (!utilIsDown()) {
				backoffTeammateBall = 1;
			}
			else {
				backoffTeammateBall = 20;
			}
			return true;
		}
	}

	// if teamate in front regardless of ball for 4 consec frames
	for (int i = 1; i <= NUM_TEAM_MEMBER /*&& teammateSeenTime > 4*/; i++) {
		if ((uint32)i != PLAYER_NUMBER
				&& gps->tmObj(i - 1).counter > 0
				&& gps->teammate(i).posVar <= get95CF(50)
				&& abs(gps->teammate(i).pos.head) < 45
				&& gps->teammate(i).pos.d < 60) {
			// step back if see teammate but not ball
			// lock visual backoff for 20 frames
			backoffTeammate = 20;
			return true;
		}
	}

	// if teammate not in front, return true if still locked in 
	// visual backoff mode
	if (backoffTeammateBall || backoffTeammate) {
		return true;
	}

	return false;
}

//these oldVis... values used to be needed when there was locking, but are not anymore..
static double oldVisTeammateX = 0;
static double oldVisTeammateY = 0;
static double oldVisTeammateHead = 0;

//returns your local heading to that teammate, and the teammates global x and y pos
bool Behaviours::utilNeedVisualBackoffWithReturn(double *headingToTM, double *retx, double *rety) {
	// check if teammate in front of you with ball
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].var < get95CF(200)
			&& vTeammate[i].d <100
			&& vBall.cf> 0
			&& vBall.d < 100) {
				oldVisTeammateX = vTeammate[i].x;
				oldVisTeammateY = vTeammate[i].y;
				oldVisTeammateHead = vTeammate[i].h;
				*retx = oldVisTeammateX;
				*rety = oldVisTeammateY;
				*headingToTM = oldVisTeammateHead;

				return true;
		}
	}

	// check if teammate in front of you regardless of  ball
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].var < get95CF(200)
			&& ABS(vTeammate[i].h) < 45
			&& vTeammate[i].d < 100) {
				oldVisTeammateX = vTeammate[i].x;
				oldVisTeammateY = vTeammate[i].y;
				oldVisTeammateHead = vTeammate[i].h;
				*retx = oldVisTeammateX;
				*rety = oldVisTeammateY;
				*headingToTM = oldVisTeammateHead;

				return true;
		}
	}

	oldVisTeammateX = 0;
	oldVisTeammateY = 0;
	oldVisTeammateHead = 0;
	*retx = oldVisTeammateX;
	*rety = oldVisTeammateY;
	*headingToTM = oldVisTeammateHead;
	return false;
}

//these oldGps... values used to be needed when there was locking, but are not anymore..
static double oldGpsTeammateX = 0;
static double oldGpsTeammateY = 0;
static double oldGpsTeammateHead = 0;
//returns your local heading to that teammate, and the teammates global x and y pos
bool Behaviours::utilNeedGpsBackoffWithReturn(double *headingToTM, double *retx, double *rety) {
	for (int i = 1; i <= NUM_TEAM_MEMBER; i++) {
		if (gps->tmObj(i - 1).counter > 0
			&& gps->teammate(i).posVar < get95CF(50)
			&& ABS(gps->teammate(i).pos.head) < 45
			&& gps->getBallMaxVar() < get95CF(50)
			&& gps->getBall(LOCAL).d < 50
			&& gps->teammate(i).pos.d < gps->getBall(LOCAL).d + 20) {
				oldGpsTeammateX = gps->teammate(i).pos.x;
				oldGpsTeammateY = gps->teammate(i).pos.y;
				oldGpsTeammateHead = gps->teammate(i).pos.head;
				*retx = oldGpsTeammateX;
				*rety = oldGpsTeammateY;
				*headingToTM = oldGpsTeammateHead;      

				return true;
		}
	}

	// if teamate in front regardless of ball for 4 consec frames
	for (int i = 1; i <= NUM_TEAM_MEMBER; i++) {
		if (gps->tmObj(i - 1).counter > 0
			&& gps->teammate(i).posVar < get95CF(50)
			&& ABS(gps->teammate(i).pos.head) < 45
			&& gps->teammate(i).pos.d < 60) {
				oldGpsTeammateX = gps->teammate(i).pos.x;
				oldGpsTeammateY = gps->teammate(i).pos.y;
				oldGpsTeammateHead = gps->teammate(i).pos.head;
				*retx = oldGpsTeammateX;
				*rety = oldGpsTeammateY;
				*headingToTM = oldGpsTeammateHead;      

				return true;
		}
	}

	oldGpsTeammateX = 0;
	oldGpsTeammateY = 0;
	oldGpsTeammateHead = 0;
	*retx = oldGpsTeammateX;
	*rety = oldGpsTeammateY;
	*headingToTM = oldGpsTeammateHead;  
	return false;
}

bool Behaviours::utilIsBallUnderChin() {
	if (tilty < -60 && abs(panx) < 20 && gps->getBall(LOCAL).d < 17.0) {
		// gps->getBall(LOCAL).d < 20 && lostBall < 6) {
		return true;
	}
	
	return false;
}

/*
 * This function can no longer be used because
 * the new gps no longer provides opponent information
 * in the necessary format
 */
/*
   bool Behaviours::utilOpponentInFace() {
   // check if opponent in front with ball
   for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
     if (wmOpponent[i].var < get95CF (50) && wmOpponent[i].d < 25
         && ABS (wmOpponent[i].h) < 45) {
           return true;
     }
   }
   return false;
}
*/

bool Behaviours::utilTeammateInFace() {
	// check if teammate in front with ball
	// version that uses gps teammate 
	/*
		for (int i = 1; i <= NUM_TEAM_MEMBER; i++) {
		if (i == PLAYER_NUMBER)
		  continue;
		if (gps->teammate(i).posVar < get95CF(50) &&
		    ABS (gps->teammate(i).pos.getHackedHead()) < 45 &&
		    gps->teammate(i).pos.d < 35) {
		    	backoffTeammate = 20;                
			return true;
		    }
		    cout << "teammate dist: " << i << " " << vTeammate[i-1].d << endl;
		    if (vTeammate[i-1].var < get95CF(50) && ABS (vTeammate[i-1].h) < 45 &&
		      // never, ever insert this part of code without other action since
		      // if the teammate is not seen then vteammate will give you completely
		      // wrong information
		    vTeammate[i-1].d < 60 && teammateSeenTime > 4) {
		    backoffTeammate = 20;    
		    return true;
		    }
		   }
	*/
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].var < get95CF(50) && vTeammate[i].d < 40
		    && ABS(vTeammate[i].h) < 80) {
			return true;
		}
	}
	return false;
}

void Behaviours::utilDetermineRole() {
	// calculate each players distance to the ball.
	int closest = -1;
	double closestDist = VERY_LARGE_INT;
	double tempDist;

	// find the closest robot to the ball not including ourselves
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
	     tempDist = getDistanceBetween(gps->teammate(i, 'g').pos.x,
	     gps->teammate(i, 'g').pos.y, gps->getBall(GLOBAL).x,
	     gps->getBall(GLOBAL).y);

	     if (tempDist < closestDist) {
	     	closest = i;
		closestDist = tempDist;
	     }
	 }

	 closestRobotToBall = closest;

	 if (closest < 0 || gps->getBall(LOCAL).d < closestDist) {
	 	role = AGGRESSOR;
	 }
	 else {
	 	role = ASSISTANT;
	 }
}

bool Behaviours::utilIsInPenaltyArea() {
	return gps->self().pos.y > FIELD_LENGTH - WALL_THICKNESS - 50;
}

bool Behaviours::utilIsInOwnHalf() {
	return gps->self().pos.y < FIELD_LENGTH / 2;
}

bool Behaviours::utilIsOnLREdge(double dist) {
	return utilIsOnLEdge(dist) || utilIsOnREdge(dist);
}

bool Behaviours::utilIsOnLEdge(double dist) {
	return (gps->self().pos.x < WALL_THICKNESS + dist);
}

bool Behaviours::utilIsOnREdge(double dist) {
	return (gps->self().pos.x > FIELD_WIDTH - WALL_THICKNESS - dist);
}

bool Behaviours::utilIsOnTEdge(double dist) {
	return (gps->self().pos.y > (FIELD_LENGTH - WALL_THICKNESS - dist));
}

bool Behaviours::utilIsOnBEdge(double dist) {
	return (gps->self().pos.y < (WALL_THICKNESS + dist));
}

bool Behaviours::utilBallOnLREdge(double dist) {
	return utilBallOnLEdge(dist) || utilBallOnREdge(dist);
}

bool Behaviours::utilBallOnLEdge(double dist) {
	return gps->getBall(GLOBAL).x < WALL_THICKNESS + dist;
}

bool Behaviours::utilBallOnREdge(double dist) {
	return gps->getBall(GLOBAL).x > FIELD_WIDTH - WALL_THICKNESS - dist;
}

bool Behaviours::utilBallOnTEdge(double dist) {
	return gps->getBall(GLOBAL).y > (FIELD_LENGTH - WALL_THICKNESS - dist);
}

bool Behaviours::utilBallInTLCorner() {
	return gps->getBall(GLOBAL).y > (FIELD_LENGTH - WALL_THICKNESS - 40)
		&& gps->getBall(GLOBAL).x < WALL_THICKNESS + 40;
}    

bool Behaviours::utilBallInTRCorner() {
	return gps->getBall(GLOBAL).y > (FIELD_LENGTH - WALL_THICKNESS - 40)
		&& gps->getBall(GLOBAL).x > (FIELD_WIDTH - WALL_THICKNESS - 40);
}    

bool Behaviours::utilBallOnBEdge(double dist) {
	return gps->getBall(GLOBAL).y < (WALL_THICKNESS + dist);
}

bool Behaviours::utilIsInFrontTargetGoal() {
	return ABS(gps->self().pos.x - FIELD_WIDTH / 2) < GOAL_WIDTH / 2 + 10;
}

bool Behaviours::utilBallInOwnHalf() {
	return gps->getBall(GLOBAL).y < FIELD_LENGTH / 2;
}

bool Behaviours::utilIsUp() {
	return ABS(dkd) < 60;
}

bool Behaviours::utilIsDown() {
	return ((prevAttackMode == GetBehindBall)
		 ? (ABS(dkd) > 110) : (ABS(dkd) > 120));
}

void Behaviours::utilLookAroundParams(int t) {
	headtype = ABS_H;
	tilty = t;

	if (prevPan > 80.0) {
		panDirection = -1;
	}

	if (prevPan < -80.0) {
		panDirection = +1;
	}

	prevPan += (panDirection * 5.0);
	panx = prevPan;
}

void Behaviours::setPanDirection(int dir) {
	panDirection = dir;
}

void Behaviours::setPrevPan(double pan) {
	prevPan = pan;
}

// Make sure a is: -180 < a <= 180.
double Behaviours::utilNormaliseAngle(double a) {
	while (a > 180) {
		a -= 360;
	}
	while (a <= -180) {
		a += 360;
	}

	return a;
}

double Behaviours::saHeadingBetween(double fromX, double fromY, double toX, double toY) {
	double relx = toX - fromX;
	double rely = toY - fromY;
	if (relx == 0 && rely == 0) {
		return (0.0);
	}
	else {
		return (degrees(atan2(rely, relx)));
	}
}

void Behaviours::saGetPointRelative(double baseX, double baseY, double offsetHead,
				    double offsetDist, double *resX, double *resY) {
	*resX = baseX + offsetDist * cos(radians(offsetHead));
	*resY = baseY + offsetDist * sin(radians(offsetHead));
}

void Behaviours::saFindAngleDist(double x, double y, double *head, double *dist) {
	double dX = x - gps->self().pos.x;
	double dY = y - gps->self().pos.y;
	*dist = sqrt(SQUARE(dX) + SQUARE(dY));
	*head = RAD2DEG(atan2(dY, dX)) - gps->self().h;
	*head = NormalizeAngle_180(*head);

	/* THIS IS STUFFED!
		*head = asin (ABS (gps->self().pos.y - y) / *dist) * 180 / pi;
		if (y >= gps->self().pos.y && x <= gps->self().pos.x) {
		*head = 180 - *head;
		 } else {
		 if (x >= gps->self().pos.x) {
		 *head *= -1;
		 } else {
		 *head -= 180;
		 }
		 }
		 *head -= gps->self().getHackedH();
		 *head = utilNormaliseAngle(*head);
		 */
}

double Behaviours::Cap(double value, double limit) {
	if (value > limit) {
		return limit;
	}
	else if (value < -limit) {
		return -limit;
	}
	else {
		return value;
	}
}

void Behaviours::saGoToTargetFacingHeading(const double &targetx, const double &targety,
		                                   const double &targeth, double maxSpeed) {
	// variables relative to self localisation
	double relx = targetx - (gps->self().pos.x);
	double rely = targety - (gps->self().pos.y);
	double reld = sqrt(SQUARE(relx) + SQUARE(rely));
	double relTheta = NormalizeAngle_180(RAD2DEG(atan2(rely, relx))
			- (gps->self().h));
	double relh = NormalizeAngle_180(targeth - (gps->self().h));

	forward = CLIP(reld, maxSpeed) * cos(DEG2RAD(relTheta));
	left = CLIP(reld, maxSpeed) * sin(DEG2RAD(relTheta));

	//if(reld < 100) {
	//double move = sqrt(SQUARE(forward) + SQUARE(left));
	double maxTurn = 15;//(move > 4) ? 5 : (9-move);
	turnCCW = CLIP(relh, maxTurn);
	//}
}

static const double TARGET_GOAL_X = FIELD_WIDTH / 2.0;
static const double TARGET_GOAL_Y = FIELD_LENGTH;
static const double LEFT_GOALBOX_EDGE_X = TARGET_GOAL_X - GOALBOX_WIDTH / 2.0;
static const double LEFT_GOALBOX_EDGE_Y = TARGET_GOAL_Y - WALL_THICKNESS - GOALBOX_DEPTH;

static const double RIGHT_GOALBOX_EDGE_X = TARGET_GOAL_X + GOALBOX_WIDTH / 2.0;
static const double RIGHT_GOALBOX_EDGE_Y = TARGET_GOAL_Y - WALL_THICKNESS - GOALBOX_DEPTH;

bool Behaviours::utilBallInTargetGoalBox(double ballx, double bally) {
	return (bally >= LEFT_GOALBOX_EDGE_Y
		&& ballx >= LEFT_GOALBOX_EDGE_X
		&& ballx <= RIGHT_GOALBOX_EDGE_X);
}    

