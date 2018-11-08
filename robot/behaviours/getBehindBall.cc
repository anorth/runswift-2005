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
 * $Id: getBehindBall.cc 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

/*
 * Strategy:
 *  saFarGetBehindBall
 *  saGetBehindBall
 *
 * Description:
 *  gets behind ball in a defensive, 
 *  conservative way.
 *
 * Strategy member variables used:
 *  dkd - 
 *      used in determining the 
 *      direction in which to circle ball
 *  attackMode -
 *      set to GetBehindBall
 *
 * Local variables used:
 *  getBehindBallDir
 *
 * Restrictions:
 *  none
 *
 * Comments:
 *  Previously used WMObject for target and 
 *  attackPos, but now uses CurObject instead.
 */

#include "Behaviours.h"

// direction in which to rotate to get behind ball
static int getBehindBallDir = 0;
static const int bottomBODist = 50;
static const int BACKOFF_BALL_BOTTOM_EDGE_DISTANCE = bottomBODist
+ WALL_THICKNESS;

void Behaviours::saFarGetBehindBall(bool chooseTurnDir, double aa) {
	double saftyDist = 30;

	// convert dkd from dog-relative to absolute
	double rdkd = NormalizeAngle_180(dkd + gps->self().getHackedH());

	if (aa != INVALID_FLAG) {
		rdkd = aa; //global heading that you want to attack the ball at
	}

	if (chooseTurnDir) {
		//left right edge cases
		if (utilBallOnLEdge()) {
			saGetBehindBall(saftyDist, rdkd, getBehindBallDir = CLOCKWISE);
		}
		else if (utilBallOnREdge()) {
			saGetBehindBall(saftyDist, rdkd, getBehindBallDir = ANTICLOCKWISE);
		} 

		//top edge cases
		else if (utilBallOnTEdge() && gps->getBall(GLOBAL).x < FIELD_WIDTH / 2.0) {
			saGetBehindBall(saftyDist, rdkd, getBehindBallDir = CLOCKWISE);
		}
		else if (utilBallOnTEdge() && gps->getBall(GLOBAL).x >= FIELD_WIDTH / 2.0) {
			saGetBehindBall(saftyDist, rdkd, getBehindBallDir = ANTICLOCKWISE);
		} 

		//bottom edge cases
		else if (gps->getBall(GLOBAL).y < BACKOFF_BALL_BOTTOM_EDGE_DISTANCE
				&& gps->getBall(GLOBAL).x < FIELD_WIDTH / 2.0) {
			saGetBehindBall(saftyDist, rdkd, getBehindBallDir = CLOCKWISE);
		}
		else if (gps->getBall(GLOBAL).y < BACKOFF_BALL_BOTTOM_EDGE_DISTANCE
				&& gps->getBall(GLOBAL).x >= FIELD_WIDTH / 2.0) {
			saGetBehindBall(saftyDist, rdkd, getBehindBallDir = ANTICLOCKWISE);
		}

		// otherwise get behind ball based on dkd angle
		else {
			saGetBehindBall(saftyDist, rdkd, getBehindBallDir = ((dkd < 0) ? CLOCKWISE : ANTICLOCKWISE));
		}
	}
	else {
		saGetBehindBall(saftyDist, rdkd, getBehindBallDir);
	}
	attackMode = GetBehindBall;
}

void Behaviours::saGetBehindBall(double sd, double aa, int turndir, double ballX, double ballY, double ballH) {
	CurObject target;      // circling point
	CurObject attackPos;   // attacking point (lined up with attack angle)


	//default the pivot point to vision ball
	//unless otherwise specified
	//(default values for ballX ballY ballH are the invalid flag)
	double pivotx = vBall.x;
	double pivoty = vBall.y;
	double pivoth = vBall.h;
	if (ballX != INVALID_FLAG || ballH != INVALID_FLAG || ballH != INVALID_FLAG) {
		pivotx = ballX;
		pivoty = ballY;
		pivoth = ballH;
	}

	double ballToMe = saHeadingBetween(pivotx, pivoty, gps->self().pos.x, gps->self().pos.y);

	saGetPointRelative(pivotx, pivoty, utilNormaliseAngle(aa + 180.0), sd, &attackPos.x, &attackPos.y);
	saFindAngleDist(attackPos.x, attackPos.y, &attackPos.h, &attackPos.d);

	if (turndir == CLOCKWISE) {
		saGetPointRelative(pivotx, pivoty, utilNormaliseAngle(ballToMe - 70.0), sd, &target.x, &target.y);
	}
	else {
		saGetPointRelative(pivotx, pivoty, utilNormaliseAngle(ballToMe + 70.0), sd, &target.x, &target.y);
	}
	saFindAngleDist(target.x, target.y, &target.h, &target.d);

	if (target.d < attackPos.d) {
		/*
		   if (ABS(pivoth) < 30) {
		   forward = CLIP (target.d, 8.0) * cos (DEG2RAD (target.h));
		   left    = CLIP (target.d, 6.0) * sin (DEG2RAD (target.h));
		   turnCCW = CLIP (pivoth / 2.0, 20.0);
		   } else {
		   forward = CLIP (target.d, 3.0) * cos (DEG2RAD (target.h));
		   left    = CLIP (target.d, 3.0) * sin (DEG2RAD (target.h));
		   turnCCW = CLIP (pivoth, 30.0);
		   }
		   */
		turnCCW = CLIP(pivoth / 2.0, 20.0);

		forward = CLIP(attackPos.d, 6.5) * cos(DEG2RAD(target.h));
		double desLeft = attackPos.d *sin(DEG2RAD(target.h));

		if (ABS(forward) < 3.0) {
			if (desLeft >= 0) {
				left = MIN(6.0, desLeft);
			}
			else {
				left = MAX(-6.0, desLeft);
			}
		}
		else {
			if (desLeft >= 0) {
				left = MIN(5.0, desLeft);
			}
			else {
				left = MAX(-5.0, desLeft);
			}
		}
	}
	else {
		turnCCW = CLIP(pivoth / 2.0, 20.0);

		forward = CLIP(attackPos.d, 6.5) * cos(DEG2RAD(attackPos.h));
		double desLeft = attackPos.d *sin(DEG2RAD(attackPos.h));

		if (ABS(forward) < 3.0) {
			if (desLeft >= 0) {
				left = MIN(6.0, desLeft);
			}
			else {
				left = MAX(-6.0, desLeft);
			} 
			//left = CLIP (attackPos.d, 7.2) * sin (DEG2RAD (attackPos.h));

		}
		else {
			if (desLeft >= 0) {
				left = MIN(5.0, desLeft);
			}
			else {
				left = MAX(-5.0, desLeft);
			} 
			//left = CLIP (attackPos.d, 6.0) * sin (DEG2RAD (attackPos.h));

		}
	}
}
