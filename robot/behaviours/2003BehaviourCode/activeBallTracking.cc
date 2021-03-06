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
 * $Id: activeBallTracking.cc 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

/*
 * Strategy: 
 *  saActiveBallTracking
 *
 * Description: 
 *  tracks the ball and looks at nearest beacon 
 *  occasionally also.
 *
 * Strategy member variables used:
 *  lostBall      - number of frames ball unseen
 *  lastSawBeacon - number of frames last saw beacon
 */

#include "Behaviours.h"


void Behaviours::saActiveBallTracking() {
	//if (lostBall == 0 && gps->getBall(LOCAL).d > 30 && lastSawBeacon > 10) {
	if (lostBall == 0 && gps->getBall(LOCAL).d > 60 && lookAroundCounter == 0) {
		//saLookAtNearestBeacon();
		lookAroundCounter = 25;
	}

	if (lookAroundCounter) {
		setNewBeacons(LARGE_VAL);            
		activeGpsLocalise(true);
		//        utilLookAroundParams(15);
	}
	else {
		saBallTracking();
	}
}
