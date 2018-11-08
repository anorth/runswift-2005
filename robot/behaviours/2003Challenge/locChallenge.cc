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
 * UNSW 2003 Robocup (Ross Edwards)
 *
 * Last modification background information
 * $Id: locChallenge.cc 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#include "locChallenge.h"

static const double locChallengeDistSlow = 15;
static const double locChallengeDistLimit = 5;
static const double locChallengeAngLimit = 3;
static const double locChallengePosVar = SQUARE(10);
static const double locChallengeHVar = SQUARE(6);

struct locPoint {
	double x;
	double y;
	double h;
};

static const int locChallengeNumPoints = 5;
static const locPoint points[locChallengeNumPoints] = {
	{100,100,100}, {150,200,200}, {100,300,15}, {145,400,270}, {220,240,315}
};

static int locChallengeIndex;
static int locChallengeCounter;

void locChallenge::initLocChallenge() {
	locChallengeIndex = 0;
	locChallengeCounter = 0;
}

void locChallenge::doLocChallenge() {
	const double &desiredX = points[locChallengeIndex].x;
	const double &desiredY = points[locChallengeIndex].y;
	const double &desiredH = points[locChallengeIndex].h;

	setNewBeacons(LARGE_VAL);
	activeGpsLocalise(true);

	if (locChallengeCounter == 0) {
		double distDiff = sqrt(SQUARE(gps->self().pos.x - desiredX)
				+ SQUARE(gps->self().pos.y - desiredY));
		double angDiff = ABS(gps->self().h - desiredH);
		double posVar = gps->self().posVar;
		double hVar = gps->self().hVar;

		if (distDiff < locChallengeDistLimit && angDiff < locChallengeAngLimit) {
			if (posVar < locChallengePosVar && hVar < locChallengeHVar) {
				setStandParams();
				locChallengeCounter = 1;
				cout << "got to {" << desiredX << "," << desiredY << "," << desiredH
					<< "}" << endl;
			} // else just stay still and localise
		}
		else if (distDiff < locChallengeDistSlow) {
			saGoToTargetFacingHeading(desiredX, desiredY, desiredH, 2.0);
		}
		else {
			saGoToTargetFacingHeading(desiredX, desiredY, desiredH, 6.0);
		}
	}
	else {
		switch ((locChallengeCounter / 5) % 4) {
			case 1: tail_h = IND_TAIL_RIGHT;
				break;

			case 3: tail_h = IND_TAIL_LEFT;
				break;

			default: tail_h = IND_TAIL_H_CENTRED;
				 break;
		}

		locChallengeCounter++;

		if (locChallengeCounter > 125) {
			locChallengeCounter = 0;
			locChallengeIndex++;
			locChallengeIndex %= locChallengeNumPoints;
		}
	}
}
