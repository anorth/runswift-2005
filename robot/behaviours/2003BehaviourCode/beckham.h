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
 * Beckham Forward & Goalie
 *
 **/

#ifndef _Beckham_h_DEFINED
#define _Beckham_h_DEFINED

#include "Behaviours.h"

using namespace Behaviours;

namespace Beckham {
	// this is needed to get rid of ambiguity 
	// because std namespace also has a variable called left
	using Behaviours::left;

	static const unsigned int FORWARD = 1;
	static const unsigned int GOALIE = 2;

	// upper limit to how far away the ball to be from you
	static const double LOSTBALL_DIST = 500;  

	struct interpBehav {
		unsigned int ballDist : 26;  // distance to ball if ball visible
		unsigned int amStriker : 1;  // set if you are playing the striker role
		unsigned int amGoalie : 1;   // set if you are the goalie
		unsigned int haveBall : 1;   // set if you have ball and want support
		unsigned int kickedOff : 1;  // set if you have just kicked off
		unsigned int defending : 1;  // set if you are you are in cover defence modes
		unsigned int backingOff : 1; // set if you are currently backing off (either getbehind or support pos)

		interpBehav() {
			ballDist = (unsigned int) abs(LOSTBALL_DIST);
			amStriker = 0;
			amGoalie = 0;
			haveBall = 0;
			kickedOff = 0;
			defending = 0;
			backingOff = 0;
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


	/////////////////////
	// Beckham Forward //
	/////////////////////

	extern void initBeckhamForward();
	extern void doBeckhamForward();
	extern void beckhamStartMatchMode();
	extern void setBeckhamForwardKickOffValues();
	extern void beckhamForwardDetermineBallSource();
	extern void setBeckhamForwardValues();
	extern void chooseBeckhamForwardStrategy();
        extern void beckhamExecuteLockModeAction();
	extern int chooseBeckhamKickOffSide();
	extern bool isBeckhamKickOffAttacker();
	extern bool isBeckhamKickOffReceiver();
	extern bool isBeckhamKickOffNoOpponentsInFront();

	extern bool isBeckhamBallUnderChin();
	extern void clipBeckhamFireballPan();
	extern int numForwardsCommunicating();
	extern int numTeammatesCommunicating();
	extern bool isBeckhamToAssumeStrikerRole(double ptx, double pty);
	extern bool isBeckhamToAssumeVisualStrikerRole(double ptx, double pty);
	extern bool isBeckhamClosestToStrikerPoint(double ptx, double pty);
	extern void getBeckhamStrikerPoint(double *ptx, double *pty);
	extern void getBeckhamStrikerPointOffense(double *ptx, double *pty);
	extern void getBeckhamStrikerPointDefense(double *ptx, double *pty);
	extern bool isBeckhamToAssumeSupportRole();
	extern bool isBeckhamToBackOff(double *headToTeammate,
			double *teammatex,
			double *teammatey);
	extern bool beckhamNeedVisualBackoffWithReturn(double *headingToTM,
			double *retx,
			double *rety);
	extern bool isBeckhamToSideBackOff();
	extern bool isBeckhamHaveBall();
	extern bool canBeckhamSeeTeammateWithinDist(double dist);
	extern bool isBeckhamStuck();
	extern bool isBeckhamBallNotMoving();

	extern void doBeckhamTrackVisualBall();
	extern void doBeckhamTrackGpsBall();

	extern void doBeckhamStrikerForward(double ptx, double pty);
	extern void doBeckhamBackOffForward(double headToTeammate,
			double teammatex,
			double teammatey);
	extern void doBeckhamBackoffGetBehindBall(double towardsTargetGoal,
			double headToTeammate,
			double angYouToBallToGoal,
			double angTeammateToBallToGoal,
			bool stepBack);
	extern void boGetBehindBall(double sd,
			double aa,
			int turndir,
			double ballX,
			double ballY,
			double ballH,
			bool stepBack = false);
	extern void doBeckhamSupportForward(double ptx, double pty);
	extern double absAngleBetweenTwoPointsFromPivotPoint(double pointax,
			double pointay,
			double pointbx,
			double pointby,
			double pivotx,
			double pivoty);

	//these are mini helper functions that handle debugging etc
	//there was beginning to be too much copy and paste debugging stuff
	//so it got pulled out into seperate functions
	extern void boMaybeNormal(int matchedTmPlayerNumber,
			double teammate,
			double desDir,
			double headToTeammate,
			double you);
	extern void boDoNormal();
	extern void boDoSupport();
	extern void boDoGetBehind(double desDir,
			double headToTeammate,
			double you,
			double teammate,
			bool stepBack);

	extern int matchVisualTeammateToGpsTeammate(double headToTeammate,
			double teammatex,
			double teammatey);                            

	extern void doBeckhamNormalForward();

	extern void chooseBeckhamShootStrategy();
	extern void doBeckhamAttackBall();
	extern void doBeckhamHoverToBall(double ballD, double ballH);
	extern void doBeckhamHoverToSpot(double spotD, double spotH);
	extern bool getDistHeadProjectedBall(double &dist, double &head);
	extern void aaBeckhamChestPass();
	extern double minDistOppFromBall(double bx, double by);
	extern bool isOnLeftOfLine(double fromx,
			double fromy,
			double tox,
			double toy);
	extern double posnTowardsOwnGoalFromPointWithCrop(double pointx,
			double pointy,
			double distance,
			double *posx,
			double *posy);

	extern void getBeckhamHailMaryPoint(double *ptx, double *pty);
	extern void doFastGoToPointHeading(double x, double y, double globalH);
	extern void doBeckhamHailMaryReceive();

	extern int getNumOpponentsOnSide();
	extern int getNumTeammatesOnSide(bool countGoalie = false);

        extern int findHowManyBlockingRobot();
	extern double getBeckhamDKD();

	// methods for defence
	extern bool beckhamDefenceRequired();
	extern bool areClosest();
	extern bool areFurthestX();
	extern int numBirds();
	extern void setDefending();
	extern bool isDefenceProblem(double offset,
			double angle,
			bool mustHaveBall = false,
			bool includeYou = true);
	extern void doBeckhamDefend();

	extern bool ballInOwnBox(double errorMargin);

	extern void beckhamGeneralHead();

	extern int getMinVarOppNum();
	extern bool oppInTargetGoal(int oppIndex);

	// Stealth Dog
	extern void doStealthDog(double leftAngle, double rightAngle);
	extern void doStealthDog(double leftAngle, double rightAngle, double head);
	extern bool needStealthDog(double &leftAngle, double &rightAngle);
	extern bool needStealthDog(double &leftAngle,
			double &rightAngle,
			double locx,
			double locy,
			double locD);

	// sets gps wireless communication to portray that this player is the goalie
	inline static void setBeckhamGoalie() {
		interpBehav info = (interpBehav) gps->self().behavioursVal; 
		info.amGoalie = 1;
		gps->sharedBehaviour() = info.convert();
	}

	// sends 'have ball' signal to beckham teammates
	inline static void setBeckhamHaveBall() {
		interpBehav info = (interpBehav) gps->self().behavioursVal; 
		info.haveBall = 1;
		gps->sharedBehaviour() = info.convert();
	}

	// sends 'have kicked off' signal to beckham teammates
	inline static void setBeckhamHaveKickedOff() {
		interpBehav info = (interpBehav) gps->self().behavioursVal; 
		info.kickedOff = 1;
		gps->sharedBehaviour() = info.convert();
	}            

	// sends 'have ball' signal to beckham teammates
	inline static void setBeckhamStriker() {
		interpBehav info = (interpBehav) gps->self().behavioursVal; 
		info.amStriker = 1;
		gps->sharedBehaviour() = info.convert();
	}

	// sends 'cover defending' signal to beckham teammates
	inline static void setBeckhamDefending() {
		interpBehav info = (interpBehav) gps->self().behavioursVal; 
		info.defending = 1;
		gps->sharedBehaviour() = info.convert();
	}

	// sends 'am backing off' signal to beckham teammates
	inline static void setBeckhamBackingOff() {
		interpBehav info = (interpBehav) gps->self().behavioursVal; 
		info.backingOff = 1;
		gps->sharedBehaviour() = info.convert();
	}

	// sends 'have ball' signal to beckham teammates
	inline static void setBeckhamBallDist(double dist) {
		interpBehav info = (interpBehav) gps->self().behavioursVal; 
		info.ballDist = (unsigned int) abs(dist);
		gps->sharedBehaviour() = info.convert();
	}

	// returns true iff 'have ball' signal received from another beckham forward
	inline static bool isBeckhamTeammateHaveBall() {
		for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
			const WMObj &tm = gps->tmObj(i);
			interpBehav info = (interpBehav) tm.behavioursVal;
			if (info.haveBall != 0 && tm.counter > 0) {
				return true;
			}
		}
		return false;
	}

	// returns true iff 'have kicked off' signal received from another forward
	inline static bool isBeckhamTeammateHaveKickedOff() {
		for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
			const WMObj &tm = gps->tmObj(i);
			interpBehav info = (interpBehav) tm.behavioursVal;
			
			// tm.counter must be > 0 otherwise invalid teammate.
			if (info.kickedOff != 0 && tm.counter > 0) { 
				return true;
			}
		}
		return false;
	}            

	////////////////////
	// Beckham Goalie //
	////////////////////

	extern double allowedAttackDist;
	extern double allowedAttackDem;

	extern double blockDistance;
	extern double blockSpeed;
	extern double blockAngle;
	extern int blockLength;


	extern void initBeckhamGoalie();
	extern void doBeckhamGoalie();
	extern void setBeckhamGoalieValues();
	extern void chooseBeckhamGoalieStrategy();

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
	extern void calculateSearchDefendPositionHeading(double *defendX,
			double *defendY,
			double *defendH,
			int source = GPS_BALL);
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
	extern bool goalieAllowedToBlock(bool initial);
	extern double goalieDistanceFromHome();
	extern bool teammateHasBall();
	extern bool isFacingAwayFromOwnGoal(double posx, double posy, double posh);

	extern bool opponentIsCloserToBall();
	extern bool ballIsInCornerAreas();

	extern void lookAtBeaconAndGoToDefendPosition();
	extern void goalieAttackBall();
	extern void goalieAttackGpsBall();
	extern void goalieGoToBall();
	extern void goalieHoverToBall(bool usingVision, bool usePrediction);
	extern void aaGoalieHoldBall();
	extern void goalieGotBall();
	extern bool goalieAbleToDoARampShot();
	extern void findBestGap(double minAllowedAngle,
			double maxAllowedAngle,
			double *returnMinAngle,
			double *returnMaxAngle);
}

#endif // _Beckham_h_DEFINED
