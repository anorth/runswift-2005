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
 * $Id: maverick.h 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * Maverick
 *
 * An interface with different behaviours separated into different files.
 *
 **/

#ifndef _Maverick_h_DEFINED
#define _Maverick_h_DEFINED

#include "Behaviours.h"

#error This should not be included

using namespace Behaviours;

namespace Maverick {
	using Behaviours::left;

	static const unsigned int FORWARD = 1;
	static const unsigned int GOALIE = 2;

	//////////////
	// MAVERICK //
	//////////////
	extern void initMaverickForward();
	extern void doMaverickForward();
	extern void setMaverickForwardValues();
	extern void chooseMaverickForwardStrategy();

	extern void maverickAttackBall();
	extern void chooseMaverickShootStrategy();
	double variableRequiredAccuracy(double tmx, double tmy, double reqGoalWidth);
	double minDistOppFromBall(double bx, double by);            

	extern void chooseBackOffStrategy(double tmx, double tmy);
	void calcBackOffPosition(double tmx, double tmy);

	extern bool is3rdClosest();
	extern void choose3rdPlayerStrategy();
	extern void choose3rdPlayerOffence(double tmx, double tmy);
	extern void choose3rdPlayerDefence(double tmx, double tmy);

	extern void hoverToBall(double ballDist, double ballHead);
	//extern void hoverToPositionHeading(double posx, double posy, double posh);  
	extern void hoverToTargetHeadingConsiderBall(double targetx,
			double targety,
			double targeth,
			double ballHead);
	extern void trackBallHeadParams();
	extern void gpsTrackBallHeadParams();

	extern bool is3rdClosestDebug;
	extern bool is3rdPositionDebug;
	extern bool is3rdUpDownDebug; 
	extern bool fireBallLights;   
	extern bool backOffEyes;
	extern bool thirdPlayerEyes;
	extern bool newShootStratEyes;
	extern double mavForward;
	extern double mavTurn;
	extern double mavLeft;

	// need unhacking
	extern int anotherCounter;


	//////////////////
	// FIRE PAWKICK //
	//////////////////
	static const double FIRE_PAWKICK_YOFFSET = BALL_RADIUS;
	static const double FIRE_PAWKICK_XOFFSET = 6.0;
	static const int FIRE_PAWKICK_RESET = 25;
	static const int FIRE_PAWKICK_SLOWDOWN = 20;
	static const double FIRE_PAWKICK_RANGE = 10;
	static const double FIRE_PAWKICK_ANGLE = 10;
	static const double FIRE_PAWKICK_APPROACH = 7.5;

	static const int FIRE_PAWKICK_LEFT = 0;
	static const int FIRE_PAWKICK_RIGHT = 1;
	static const int FIRE_PAWKICK_AUTO = 2;

	extern void initFirePawKick();
	extern void doFirePawKick();
	extern void fireApproach();
	extern void firePawKick(int paw, double offset = 6.0);
	extern void firePawKickAim();

	/////////////////////
	// MAVERICK GOALIE //
	///////////////////// 
	//temp
	extern double allowedAttackDist;
	extern double allowedAttackDem;

	extern double blockDistance;
	extern double blockSpeed;
	extern double blockAngle;
	extern int blockLength;


	extern void initMaverickGoalie();
	extern void doMaverickGoalie();
	extern void setMaverickGoalieValues();
	extern void chooseMaverickGoalieStrategy();

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

	extern bool maverickDebug;

	extern void lookAtBeaconAndGoToDefendPosition();
	extern void goalieAttackBall();
	extern void goalieAttackGpsBall();
	extern void goalieGoToBall();
	extern void goalieHoverToBall(bool usingVision);
	extern void aaGoalieHoldBall();
	extern void maverickGoalieGotBall();
	extern bool goalieAbleToDoARampShot();
	extern void findBestGap(double minAllowedAngle,
			double maxAllowedAngle,
			double *returnMinAngle,
			double *returnMaxAngle);
}


#endif // _Maverick_h_DEFINED
