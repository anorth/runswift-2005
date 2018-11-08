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
 * $Id
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * german Forward & Goalie
 *
 **/

#ifndef _German_h_DEFINED
#define _German_h_DEFINED

#include "Behaviours.h"
#include "beckham.h"

using namespace Behaviours;

namespace German {
	// this is needed to get rid of ambiguity 
	// because std namespace also has a variable called left
	using Behaviours::left;

	static const unsigned int FORWARD = 1;
	static const unsigned int GOALIE = 2;

	// upper limit to how far away the ball to be from you
	static const double LOSTBALL_DIST = 500;  

	struct interpBehav {
		unsigned int ballDist : 29; // distance to ball if ball visible
		unsigned int amStriker : 1; // set if you are playing the striker role
		unsigned int amGoalie : 1;  // set if you are the goalie
		unsigned int haveBall : 1;  // set if you have ball and want support

		interpBehav() {
			ballDist = (unsigned int) abs(LOSTBALL_DIST);
			amStriker = 0;
			amGoalie = 0;
			haveBall = 0;
		}

		interpBehav(int i) {
			memcpy(this, &i, sizeof(int));
		}

		int convert() {
			int i;
			memcpy(&i, this, sizeof(interpBehav));
			return i;
		}
	};

	// sets gps wireless communication to portray that this player is the goalie
	inline static void setGermanGoalie() {
		interpBehav info = (interpBehav) gps->self().behavioursVal; 
		info.amGoalie = 1;
		gps->sharedBehaviour() = info.convert();
	}


	////////////////////
	// German Goalie //
	////////////////////

	extern double allowedAttackDist;
	extern double allowedAttackDem;

	extern double blockDistance;
	extern double blockSpeed;
	extern double blockAngle;
	extern int blockLength;


	extern void initGermanGoalie();
	extern void doGermanGoalie();
	extern void setGermanGoalieValues();
	extern void chooseGermanGoalieStrategy();

	extern bool allowedToAttackBall();
	extern void goalieFullCircleFindBall();
	extern void goalieSemiCircleFindBall();
	extern void goalieHeadFindBall();
	extern bool hasReachedDefaultPosition();
	extern void goToDefaultPositionHeading();

	extern bool hasReachedSmartPosition();
	extern void goToSmartPositionHeading();
	extern void findSmartPosition(int *posX, int *posY, int *posH);

	extern bool goalieIsInGoalieBox();
	extern bool goalieBallWithinAttackDistance();
	extern bool goalieBallWithinDangerDistance();
	extern bool goalieBallWithinBlockDistance();
	extern bool goalieBallIsGoingToArriveAtYouSoon();

	extern void calculateDefendPositionHeading(double *defendX,
			double *defendY,
			double *defendH);
	extern void goToDefendPositionHeading(double defendX,
			double defendY,
			double defendH);
	extern bool hasReachedDefendPositionHeading(double defendX,
			double defendY,
			double defendH);
	extern bool hasReachedDefendPositionHeadingWhenLookingForBall(double defendX,
			double defendY,
			double defendH);

	extern void setTrackBallHeadParams();
	extern void setGpsTrackBallHeadParams();
	extern bool ballJumpedTooMuch();
	extern bool goalieAllowedToBlock();
	extern double goalieDistanceFromHome();
	extern bool teammateHasBall();
	extern bool isFacingAwayFromOwnGoal(double posx, double posy, double posh);

	extern bool opponentIsCloserToBall();
	extern bool ballIsInCornerAreas();

	extern void lookAtBeaconAndGoToDefendPosition();
	extern void goalieAttackBall();
	extern void goalieAttackGpsBall();
	extern void goalieGoToBall();
	extern void goalieHoverToBall(bool usingVision);
	extern void aaGoalieHoldBall();
	extern void goalieGotBall();
	extern bool goalieAbleToDoARampShot();
	extern void findBestGap(double minAllowedAngle,
			double maxAllowedAngle,
			double *returnMinAngle,
			double *returnMaxAngle);
}

#endif // _German_h_DEFINED
