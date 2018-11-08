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
 * $Id:
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

/*
 * Strategy:
 *  aaBeckhamChestPass
 */

#include "beckham.h"

void Beckham::aaBeckhamChestPass() {

	// find friend with highest y-value
	int friendMostUpFieldIndex = -1;
	double friendMostUpFieldValue = -1;

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		// ignore yourself
		if (i == PLAYER_NUMBER)
			continue;

		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;

		// ignored expired friend or goalie
		if (tm.counter <= 0 || info.amGoalie != 0)
			continue;

		if (tm.pos.y > friendMostUpFieldValue) {
			friendMostUpFieldIndex = i;
			friendMostUpFieldValue = tm.pos.y;
		}
	}

	// if no friends at all, chest push immediately
	if (friendMostUpFieldIndex == -1) {
		lockMode = ChestPush;
		aaChestPush();
		return;
	}

	Vector &friendToPassTo = gps->teammate(friendMostUpFieldIndex + 1).pos;

	if (ABS(friendToPassTo.getHead()) <= 20) {
		lockMode = ChestPush;
		aaChestPush();
		return;
	}
	else {
		// spin with ball under snout
		walkType = NormalWalkWT;

		fsO = 25;
		ffO = 80;

		forward = 0;
		left = (friendToPassTo.getHead() > 0) ? -3 : 3;
		turnCCW = CLIP(friendToPassTo.getHead(), 35.0);

		panx = 0;
		tilty = -50;        
		return;
	}
}
