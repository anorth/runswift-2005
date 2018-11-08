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
 * $Id: gotoBall.cc 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

/*
 * Strategy:
 *  saGoToBall
 *
 * Description:
 *  goes to ball
 *
 * Strategy member variables used:
 *
 * Local variables used:
 *  MAX_OFFSET_FORWARD
 *
 * Restrictions:
 *  none
 *
 * Comments:
 *  none
 */

#include "Behaviours.h"

void Behaviours::saGoToBall(double slowDownDist) {
	double forwardSpeed = 7.5;

	if (useZoidWalk == 1 && gps->getBall(LOCAL).d > slowDownDist) {
		walkType = ZoidalWalkWT;
		forwardSpeed = MAX_OFFSET_FORWARD;
		PG = 40;
	}

	forward = forwardSpeed;
	left = 0;
	turnCCW = CLIP(gps->getBall(LOCAL).head / 2, 40.0);

	if (ABS(turnCCW) > 15) {
		forward = 0.5;
		walkType = CanterWalkWT;
	}

	if (forward <slowDownDist || ABS(turnCCW)> 20) {
		walkType = CanterWalkWT;
	}

	/*
	// copied straight from Challenge.cc
	// need a lot of turn to get align with ball
	if (ABS(gps->getBall(LOCAL).getHackedHead()) > 30) {
	forward = Cap(gps->getBall(LOCAL).d,2.0) * cos(DEG2RAD(gps->getBall(LOCAL).getHackedHead()));
	left    = Cap(gps->getBall(LOCAL).d,2.0) * sin(DEG2RAD(gps->getBall(LOCAL).getHackedHead()));
	turnCCW = Cap(gps->getBall(LOCAL).getHackedHead(), 30.0);
	} else {
// can see the ball comfortably
if (gps->getBall(LOCAL).d > 30.0) {
//          too fast for match              
//          forward = Cap(gps->getBall(LOCAL).d,8.0) * cos(DEG2RAD(gps->getBall(LOCAL).getHackedHead()));
//          left    = Cap(gps->getBall(LOCAL).d,6.0) * sin(DEG2RAD(gps->getBall(LOCAL).getHackedHead()));
//            
forward = Cap(gps->getBall(LOCAL).d,6.0) * cos(DEG2RAD(gps->getBall(LOCAL).getHackedHead()));
left    = Cap(gps->getBall(LOCAL).d,4.0) * sin(DEG2RAD(gps->getBall(LOCAL).getHackedHead()));

} else {
forward = Cap(gps->getBall(LOCAL).d,4.0) * cos(DEG2RAD(gps->getBall(LOCAL).getHackedHead()));
left    = Cap(gps->getBall(LOCAL).d,3.0) * sin(DEG2RAD(gps->getBall(LOCAL).getHackedHead()));
}
double move = sqrt(SQUARE(forward) + SQUARE(left));
double maxTurn = 9 - move;
turnCCW = Cap(gps->getBall(LOCAL).getHackedHead()/2.0, maxTurn);
}
*/
}
