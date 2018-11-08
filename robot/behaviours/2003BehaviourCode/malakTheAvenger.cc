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
 * $Id: malakTheAvenger.cc 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * This code is the sole property of Team Uther Lightbringer and the Silver Hand
 * Malak The Aveneger
 *
 **/ 

//#define MALAK_THE_AVENGER_LED_DEF

//#define MALAK_THE_AVENGER_ROLE_LED_DEF

#include "Behaviours.h"
#include "../share/BallDef.h"
#include "../share/Common.h"
#include "teamUtherCommon.h"

static const int vBallLost = 10;
static const int vBallAnywhere = 40;

static const int MALAK_LEFT_PAW = 0;
static const int MALAK_RIGHT_PAW = 1;

static int teamDForwardFBDir = ANTICLOCKWISE;
static int lastSeenOpponent = 0;

static int ballSource = VISION_BALL;

static int activeLocaliseTimer = 0;
static int disallowActiveLocalise = 0;
static int hasBallTimer = 0;

static int malakBackoffTeammateBall = 0;
static int malakBackoffTeammate = 0;
static int malakTeammateSeenTime = 0;

static int attackGpsBall = 0;
static double malakBackoffAngleMax = 5.0;

static double ballX = 0;
static double ballY = 0;
static double ballH = 0;
static double ballD = 0;

static double malakWirelessBallDistThreshold = 30;

static int malakTheAvengerActiveLocaliseCounter = 0;
static int malakTheAvengerActiveLocaliseTimer = 0;
static int malakTheAvengerDisallowActiveLocalise = 0;

static bool malakTheAvengerMoveSideways = false;
static int malakTheAvengerMoveSidewaysTimer = 0;
static bool malakTheAvengerMoveToLeft = false;

static int malakTheAvengerClearKickCounter = 0;
static int malakTheAvengerClearPushCounter = 0;
static int malakTheAvengerTurnKickTimer = 0;
static int malakTheAvengerSpinKickTimer = 0;

/////////////////////////////////
// turnkick variables

static int malakTurnKickCounter = 0;
static const int turnKick90LiftHead = 8;
static const int turnKick90MaxCounter = 20;
static const int turnKick180LiftHead = 24;
static const int turnKick180MaxCounter = 36;
static const double turnKickLiftTilt = -10;

/////////////////////////////////
// stuck variables

static const double STUCK_DIST_TO_BALL_THRESHOLD = 10;
static const double STUCK_NUM_OF_FRAMES_WITHIN_THRESHOLD_LIMIT = 60;
static const double STUCK_FORWARD_THRESHOLD = 5;

static double stuckPrevDistToBall = LARGE_VAL;
static int stuckNumOfFramesWithinThreshold = 0;

static bool stuckBallNotMoving = false;
static bool stuckTrully = false;

namespace uther {
	bool prevHaveBall; // true if attacker in previous frame
	bool prevSupport;  // true if support in previous frame
}

void uther::initMalakTheAvenger() {
	lockMode = NoMode;
	lostBall = 0;

	grabTime = 0;
	attackLock = 0;
	useZoidWalk = 1;

	prevHaveBall = false;
	prevSupport = false;

	malakTheAvengerTurnKickTimer = 0;
	malakTheAvengerSpinKickTimer = 0;

	malakTheAvengerKickOffTimer = 0;
	utherKickingOff = true;
}

void uther::doMalakTheAvenger() {
	setDefaultParams();
	setWalkParams();
	setMalakTheAvengerValues();
	chooseMalakTheAvengerStrategy();
}

void uther::setMalakTheAvengerValues() {
	if (gps->canSee(vobBall)) {
		malakWirelessBallDistThreshold = 30;
	}

	// timeout kickoff after 3 secs
	if (malakTheAvengerKickOffTimer < 75) {
		malakTheAvengerKickOffTimer++;
	}
	if (malakTheAvengerKickOffTimer == 75) {
		utherKickingOff = false;
	}

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].var < get95CF(25)) {
			teammateSeenTime++;
			break;
		}                
		if (i == NUM_TEAM_MEMBER - 1) {
			teammateSeenTime = 0;
		}
	}

	if (lockMode != prevLockMode) {
		prevLockMode = lockMode;
	}

	attackMode = GoToBall;

	DECREMENT(attackLock);
	DECREMENT(attackGpsBall);
	DECREMENT(grabTime);
	DECREMENT(activeLocaliseTimer);
	DECREMENT(disallowActiveLocalise);
	DECREMENT(malakTheAvengerActiveLocaliseTimer);
	DECREMENT(malakTheAvengerDisallowActiveLocalise);
	DECREMENT(malakTheAvengerTurnKickTimer);
	DECREMENT(malakTheAvengerSpinKickTimer);
	DECREMENT(malakBackoffTeammateBall);
	DECREMENT(malakBackoffTeammate);

	decrementBackOff();

	utilDetermineDKDwithRobotAvoidance();

	//utilDetermineDKD();

	// determine which ball values to use
	if (gps->canSee(vobBall)) {
		ballSource = VISION_BALL;
		ballX = vBall.x;
		ballY = vBall.y;
		ballH = vBall.h;
		ballD = vBall.d;
		//cout << "vision ball" << endl;
	}
	else if (gps->shareBallvar <LARGE_VAL && lostBall> vBallLost) {
		ballSource = WIRELESS_BALL;
		ballX = gps->shareBallx;
		ballY = gps->shareBally;
		ballH = NormalizeAngle_180(RAD2DEG(getHeadingBetween(gps->self().pos.x,
						gps->self().pos.y,
						ballX,
						ballY))
				- gps->self().h);
		ballD = getDistanceBetween(ballX,
				ballY,
				gps->self().pos.x,
				gps->self().pos.y);
#ifdef MALAK_THE_AVENGER_LED_DEF
		leds(0, 0, 2, 0, 0, 2);
#endif // MALAK_THE_AVENGER_LED_DEF
		//cout << "wireless ball" << endl;
	}
	else {
		ballSource = GPS_BALL;
		ballX = gps->getBall(GLOBAL).x;
		ballY = gps->getBall(GLOBAL).y;
		ballH = gps->getBall(LOCAL).head;
		ballD = gps->getBall(LOCAL).d;
		//cout << "gps ball" << endl;
	}

	// determine whether you're stuck
	if (gps->canSee(vobBall) && vBall.d > 20) {
		if (ABS(vBall.d - stuckPrevDistToBall) < STUCK_DIST_TO_BALL_THRESHOLD) {
			stuckNumOfFramesWithinThreshold++;
		}
		else {
			stuckNumOfFramesWithinThreshold = 0;
		}

		if (stuckNumOfFramesWithinThreshold
				>= STUCK_NUM_OF_FRAMES_WITHIN_THRESHOLD_LIMIT) {
			stuckBallNotMoving = true;
		}
		else {
			stuckBallNotMoving = false;
		}                

		stuckPrevDistToBall = vBall.d;
	}
	else {
		stuckBallNotMoving = false;
	}              

	// see if teammate in front
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].var < get95CF(25)) {
			malakTeammateSeenTime++;
			break;
		}

		if (i == NUM_TEAM_MEMBER - 1)
			malakTeammateSeenTime = 0;
	}
}

void uther::chooseMalakTheAvengerStrategy() {
	if (mode_ == mStartMatch) {
		malakTheAvengerTrackBall();
	}
	else if (lockMode != NoMode && lockMode != ResetMode) {
		// execute atomic action if in locked mode
		malakTheAvengerTrackBall();
		switch (lockMode) {
			case ChestPush: aaChestPush();
					setHaveBall(gps); 
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
					leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF                
					break;
			case SpinKick: aaSpinKick();
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
				       leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF                
				       break;
			case SafeSpinKick: aaSafeSpinKick();
					   malakTheAvengerSpinKickTimer = 60;
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
					   leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF                
					   break;
					   //case ActiveLocalise:
					   //aaActiveLocalise();
					   //break;
			case SpinChestPush: aaSpinChestPush();
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
					    leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF                
					    break;
			case GoalieKick: aaGoalieKick();
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
					 leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF                
					 break;
			case LightningKick: aaLightningKick();
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
					    leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF                
					    break;
					    //case VariableTurnKick:
					    //aaVariableTurnKick();
					    //setHaveBall(gps);  
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
					    //leds(0,2,0,0,2,0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF               
					    //break;
			case ProperVariableTurnKick: aaProperVariableTurnKick();
						     setHaveBall(gps); 
						     malakTheAvengerTurnKickTimer = 60;
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
						     leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF
						     break;
			case MalakTurnKick90: malakTurnKick90();
					      setHaveBall(gps);
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
					      leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF               
					      break;
			case MalakTurnKick180: malakTurnKick180();
					       setHaveBall(gps);   
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
					       leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF              
					       break;
			case MalakSpinKick: malakSpinKick();   
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
					    leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF              
					    break;
			case MalakClearKick: malakClearKick();
					     setHaveBall(gps);  
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
					     leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF              
					     break;
			case MalakClearPush: malakClearPush();
					     setHaveBall(gps);  
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
					     leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF
					     break;
		}
	}
	else {
		// need this to see if we can grab ball
		if (ballSource == VISION_BALL)
			malakTheAvengerTrackBall();
		else {
			//ERIC TEMP REMOVED THIS
			zannSetGpsTrackBallHeadParams();
			//setGpsTrackBallHeadParams();
		}

		// if ball under chin
		if (tilty < -55.0 && ABS(panx) < 15.0 && vBall.vob->cam_dist < 5) {
			chooseMalakTheAvengerShootStrategy();
			return;
		}

		// determine whether to assume wing striker role
		if (assumeWingStrikerRole(gps, ballX, ballY)) {
			prevHaveBall = false;

			leds(1, 1, 2, 1, 1, 2);

			malakWingForward();
			return;
		}

		// determine whether to assume attacker role
		if (malakTheAvengerHaveBall()) {
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
			leds(0, 2, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF 

#ifdef MALAK_THE_AVENGER_LED_DEF
			leds(2, 0, 0, 2, 0, 0);
#endif // MALAK_THE_AVENGER_LED_DEF

			prevHaveBall = true;

			malakNormalForward();

			// send have ball signal to others only if you're not stuck                            
			// this call to setHaveBall has been placed before the call
			// to malakNormalForward(), because we need the walk params
			// to be set

			if (!stuckTrully) {
				leds(2, 1, 1, 2, 1, 1);
				setHaveBall(gps);
			}                

			return;
		}
		else if (gps->canSee(vobBall)
				&& !malakTheAvengerGPSTeammateWithinDistOfBall(60)) {
			malakNormalForward();
		}
		else {
			prevHaveBall = false;

			bool goalieAdv = goalieAdvancing(gps);
			bool forwardAdv = forwardAdvancing(gps);
			int friendHasBall = friendHaveBall(gps);

			if (goalieAdv) {
				// goalie is going for the ball, we want to support him and not get in his way

				malakGoalieAdvancing();
				return;
			}
			else if (friendHasBall == TU_GOALIE) {
				malakGoalieHasBall();
				return;
			}
			else if (friendHasBall == TU_FORWARD) {
				leds(1, 2, 1, 1, 2, 1);

				setSupport(gps);
				malakSupportForward();
				return;
			}

			// make sure you return from all of the above cases
			malakNormalForward();
		}
	}
}

/////////////////////////////////////////////////
// START MALAK THE AVENGER FORWARD FUNCTIONS
/////////////////////////////////////////////////

bool uther::malakTheAvengerHaveBall() {
	int friendHasBall = friendHaveBall(gps);
	bool gAdvancing = goalieAdvancing(gps);
	if (friendHasBall == TU_GOALIE || gAdvancing) {
		return false;
	}

	//close to ball and can't see teammate
	if (vBall.d < 60 && !malakTheAvengerCanSeeTeammateWithinDist(60)) {
		return true;
	}

	//dir roughly correct and within 1m of ball
	if (utilIsUp() && vBall.d < 80) {
		return true;
	}

	//normal stuffs
	//if (lostBall < 4 && vBall.d < 50 && (!friendHasBall || mostSuitableToAttack(gps, ballX, ballY))) {
	//if (lostBall < 4 && vBall.d < 50 && !friendHasBall) {
	//    return true;
	//}
	return false;
}

bool uther::malakTheAvengerSupport() {
	bool friendSupport = friendIsSupport(gps);
	if (forwardPosRel(gps) != TU_GREATEST && (!friendSupport || prevSupport)) {
		return true;
	}
	return false;
}

bool uther::malakTheAvengerStriker() {
	bool friendSupport = friendIsSupport(gps);
	if (friendSupport) {
		return true;
	}
	return false;
}

void uther::malakNormalForward() {
	// if already started to actively localise, continue to localise
	if (activeLocaliseTimer) {
		activeGpsLocalise(false);

		walkType = ZoidalWalkWT;
		forward = MAX_OFFSET_FORWARD;
		left = 0;
		turnCCW = 0;

		if (activeLocaliseTimer == 1) {
			disallowActiveLocalise = 53;
		}
	}
	else if (lostBall > vBallLost && ballSource != WIRELESS_BALL) {
		// if you've lost the ball, figure out which way to spin first

		// first time in series
		if (lostBall == vBallLost + 1) {
			double h = NormalizeAngle_180(gps->getBall(LOCAL).head);
			/*
			   if (h > 0) {
			   teamDForwardFBDir = ANTICLOCKWISE;
			   } else {
			   teamDForwardFBDir= CLOCKWISE;
			   }
			   */
			teamDForwardFBDir = (gps->self().pos.x > FIELD_WIDTH / 2)
				? ANTICLOCKWISE
				: CLOCKWISE;
			setDirection(teamDForwardFBDir); //ERIC
		}

		//ERIC TEMP REMOVED THIS
		//teamDForwardFindBall();
		locateBall(); //ERIC
	}
	else {
		// choose appropriate approach ball behaviour
		chooseMalakTheAvengerAttackBall();
	}

	// now that we have the walk parameters set,
	// decide whether we're stuck

	if (stuckBallNotMoving && forward > STUCK_FORWARD_THRESHOLD) {
		stuckTrully = true;
	}
	else {
		stuckTrully = false;
	}
}


bool uther::malakTheAvengerCanSeeTeammate() {
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].cf > 0) {
			return true;
		}
	}
	return false;
}


bool uther::malakTheAvengerCanSeeTeammateWithinDist(double dist) {
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].cf > 0 && vTeammate[i].d < dist) {
			return true;
		}
	}
	return false;
}


bool uther::malakTheAvengerGPSTeammateWithinDistOfBall(double dist) {
	double ballX = gps->getBall(GLOBAL).x;
	double ballY = gps->getBall(GLOBAL).y;
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (i == PLAYER_NUMBER - 1)
			continue;
		if (ABS(gps->teammate(i, 'g').pos.x - ballX) <= dist
				&& ABS(gps->teammate(i, 'g').pos.y
					- ballY) <= dist)
			return true;
	}
	return false;
}

void uther::malakGoalieAdvancing() {
	bool forwardAdv = forwardAdvancing(gps);
	double desiredX, desiredY, desiredH;
	bool advSide = (ballX > FIELD_WIDTH / 2)
		&& (lateralPosRel(gps) == TU_GREATEST)
		|| (ballX < FIELD_WIDTH / 2)
		&& (lateralPosRel(gps) == TU_LEAST);
	if (forwardAdv || !advSide) {
		// we are the secondary forward
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
		leds(2, 0, 0, 2, 0, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF 
		desiredX = FIELD_WIDTH / 2;
		desiredY = WALL_THICKNESS + GOALBOX_DEPTH + 10;
	}
	else {
		// we are the primary forward
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
		leds(2, 0, 0, 0, 0, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF 
		desiredX = (ballX > FIELD_WIDTH / 2)
			? ((FIELD_WIDTH + GOALBOX_WIDTH) / 2 + 10)
			: ((FIELD_WIDTH - GOALBOX_WIDTH) / 2 - 10);
		desiredY = WALL_THICKNESS + GOALBOX_DEPTH / 2;
		setAdvancing(gps);
	}
	const Vector &pos = gps->self().pos;
	desiredH = RAD2DEG(atan2(ballY - pos.y, ballX - pos.x));
	saGoToTargetFacingHeading(desiredX, desiredY, desiredH);

	if (gps->canSee(vobBall)) {
		malakTheAvengerTrackBall();
	}
	else {
		trackCalcBall();
	}

	// return now to prevent normal forward behaviour
	return;
}

void uther::malakGoalieHasBall() {
	bool forwardAdv = forwardAdvancing(gps);
	double desiredX, desiredY, desiredH;
	bool advSide = (ballX > FIELD_WIDTH / 2)
		&& (lateralPosRel(gps) == TU_GREATEST)
		|| (ballX < FIELD_WIDTH / 2)
		&& (lateralPosRel(gps) == TU_LEAST);
	if (forwardAdv || !advSide) {
		// we are the secondary forward
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
		leds(2, 0, 0, 2, 0, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF 
		desiredX = FIELD_WIDTH / 2;
		desiredY = WALL_THICKNESS + GOALBOX_DEPTH + 10;
	}
	else {
		// we are the primary forward
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
		leds(2, 0, 0, 0, 0, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF
		desiredX = (ballX > FIELD_WIDTH / 2)
			? ((FIELD_WIDTH + GOALBOX_WIDTH) / 2 + 10)
			: ((FIELD_WIDTH - GOALBOX_WIDTH) / 2 - 10);
		desiredY = FIELD_LENGTH / 2;
		setAdvancing(gps);
	}
	const Vector &pos = gps->self().pos;
	desiredH = RAD2DEG(atan2(ballY - pos.y, ballX - pos.x));
	saGoToTargetFacingHeading(desiredX, desiredY, desiredH);

	if (gps->canSee(vobBall)) {
		malakTheAvengerTrackBall();
	}
	else {
		trackCalcBall();
	}

	// return now to prevent normal forward behaviour
	return;
}

void uther::malakSupportForward() {
#ifdef MALAK_THE_AVENGER_ROLE_LED_DEF
	leds(0, 0, 0, 0, 2, 0);
#endif //  MALAK_THE_AVENGER_ROLE_LED_DEF

	// once you have started to actively localise, continue for 8 frames
	if (malakTheAvengerActiveLocaliseTimer) {
		activeGpsLocalise(false);
		setStandParams();

		if (malakTheAvengerActiveLocaliseTimer == 1) {
			malakTheAvengerDisallowActiveLocalise = 30;
		}
		return;
	}

	const Vector &pos = gps->self().pos;

	/*
	   double dist2ball = sqrt(SQUARE(ballX - pos.x) +
	   SQUARE(ballY - pos.y));                    
	// if too close to ball and can see ball, back-off to get out of attacker's way
	if (dist2ball < 40 && ballSource == VISION_BALL) {
	malakTheAvengerTrackBall();
	walkType = CanterWalkWT;
	forward = -5;
	left = 0;
	turnCCW = 0;
	return;
	}
	*/

	double desiredX, desiredY;
	double dist = 120;
	double angle;

	if (ballY > pos.y) {
		// ball more upfield than you, get behind ball more
		angle = 60;
	}
	else {
		// you're more upfield than the ball, move more to the side
		angle = 20;
	}

	if (malakTheAvengerBallInMiddleStrip()) {
		// ball in middle strip
		if (malakTheAvengerIsInMiddleStrip()) {
			// malak in middle strip

			if (pos.x > ballX) {
				// move right

#ifdef MALAK_THE_AVENGER_LED_DEF
				leds(0, 0, 0, 0, 2, 0);
#endif // MALAK_THE_AVENGER_LED_DEF 

				angle = FULL_CIRCLE - angle;
			}
			else {
				// move left

#ifdef MALAK_THE_AVENGER_LED_DEF
				leds(0, 2, 0, 0, 0, 0);
#endif // MALAK_THE_AVENGER_LED_DEF

				angle += HALF_CIRCLE;
				setLeftSupport(gps);
			}
		}
		else if (pos.x > FIELD_WIDTH / 2) {
			// malak in right strip, move right

#ifdef MALAK_THE_AVENGER
			leds(0, 0, 0, 0, 2, 0);
#endif // MALAK_THE_AVENGER_LED_DEF

			angle = FULL_CIRCLE - angle;
		}
		else {
			// malak in left strip, move left

#ifdef MALAK_THE_AVENGER_LED_DEF
			leds(0, 2, 0, 0, 0, 0);
#endif // MALAK_THE_AVENGER_LED_DEF

			angle += HALF_CIRCLE;
			setLeftSupport(gps);
		}
	}
	else if (ballX < FIELD_WIDTH / 2.0) {
		// ball in left strip, move right

#ifdef MALAK_THE_AVENGER_LED_DEF
		leds(0, 0, 0, 0, 2, 0);
#endif // MALAK_THE_AVENGER_LED_DEF

		angle = FULL_CIRCLE - angle;
	}
	else {
#ifdef MALAK_THE_AVENGER_LED_DEF
		leds(0, 2, 0, 0, 0, 0);
#endif // MALAK_THE_AVENGER_LED_DEF

		// ball in right strip, move left
		angle += HALF_CIRCLE;
		setLeftSupport(gps);
	}

	desiredX = ballX + dist * cos(DEG2RAD(angle));
	desiredY = ballY + dist * sin(DEG2RAD(angle));

	if (desiredY < FIELD_LENGTH / 4) {
		desiredY = FIELD_LENGTH / 4;

#ifdef MALAK_THE_AVENGER_LED_DEF
		leds(0, 2, 0, 0, 2, 0);
#endif // MALAK_THE_AVENGER_LED_DEF
	}

	double desiredH = NormalizeAngle_0_360(180 + angle);
	if (desiredY < FIELD_LENGTH / 4) {
		desiredH = (desiredX < ballX) ? 0 : HALF_CIRCLE;
	}            

	CurObject point;
	point.x = desiredX;
	point.y = desiredY;
	point.calcHD(gps->self().pos.x, gps->self().pos.y, gps->self().h);

	saGoToTargetFacingHeading(point, ballH);

	if (gps->canSee(vobBall)) {
		malakTheAvengerTrackBall();
	}
	else {
		trackCalcBall();
	}

	// trigger for active localise
	if (malakTheAvengerActiveLocaliseCounter >= 24
			&& !malakTheAvengerDisallowActiveLocalise) {
		malakTheAvengerActiveLocaliseCounter = 0;

		setNewBeacons(LARGE_VAL, 50);
		activeGpsLocalise(false);
		malakTheAvengerActiveLocaliseTimer = 8;
	}

	malakTheAvengerActiveLocaliseCounter++;
}

void uther::malakStrikerForward() {
	// once you have started to actively localise, continue for 8 frames
	if (malakTheAvengerActiveLocaliseTimer) {
		activeGpsLocalise(false);
		setStandParams();
		walkType = ZoidalWalkWT;
		forward = MAX_OFFSET_FORWARD;
		left = 0;
		turnCCW = 0;

		if (malakTheAvengerActiveLocaliseTimer == 1) {
			malakTheAvengerDisallowActiveLocalise = 30;
		}
		return;
	}

	const double STRIKER_ANGLE = 30;
	const double STRIKER_DIST = 150;

	const double SUPPORT_ANGLE = 45;
	const double SUPPORT_DIST = 100;

	double desiredX;
	double desiredY;

	double angle;
	double dist;

	const Vector &pos = gps->self().pos;

	if (!malakTheAvengerBallInMiddleStrip()) {
		if (ballX > FIELD_WIDTH / 2.0) {
			// ball in right strip, move to left
			angle = HALF_CIRCLE - STRIKER_ANGLE;
			dist = STRIKER_DIST;
		}
		else {
			// ball in right strip, move to left
			angle = STRIKER_ANGLE;
			dist = STRIKER_DIST;
		}
	}
	else {
		if (friendIsLeftSupport(gps)) {
			// move to right
			angle = FULL_CIRCLE - SUPPORT_ANGLE;
			dist = SUPPORT_DIST;
		}
		else {
			// move to left
			angle = HALF_CIRCLE + SUPPORT_ANGLE;
			dist = SUPPORT_DIST;
		}
	}

	desiredX = ballX + dist * cos(DEG2RAD(angle));
	desiredY = ballY + dist * sin(DEG2RAD(angle));

	if (desiredY > FIELD_LENGTH * 3 / 4) {
		desiredY = FIELD_LENGTH * 3 / 4;
	}

	if (desiredY < FIELD_LENGTH / 4) {
		desiredY = FIELD_LENGTH / 4;
	}

	double desiredH = NormalizeAngle_0_360(180 + angle);
	saGoToTargetFacingHeading(desiredX, desiredY, desiredH, 7);

	if (gps->canSee(vobBall)) {
		malakTheAvengerTrackBall();
	}
	else {
		trackCalcBall();
	}        

	// trigger for active localise
	if (malakTheAvengerActiveLocaliseCounter >= 24
			&& !malakTheAvengerDisallowActiveLocalise) {
		malakTheAvengerActiveLocaliseCounter = 0;

		setNewBeacons(LARGE_VAL, 50);
		activeGpsLocalise(false);
		malakTheAvengerActiveLocaliseTimer = 8;
	}

	malakTheAvengerActiveLocaliseCounter++;
}

void uther::malakWingForward() {
	// once you have started to actively localise, continue for 8 frames
	if (malakTheAvengerActiveLocaliseTimer) {
		activeGpsLocalise(false);
		walkType = ZoidalWalkWT;
		forward = MAX_OFFSET_FORWARD;
		left = 0;
		turnCCW = 0;

		if (malakTheAvengerActiveLocaliseTimer == 1) {
			malakTheAvengerDisallowActiveLocalise = 30;
		}
		return;
	}

	const double STRIKER_ANGLE = 30;
	const double STRIKER_DIST = 150;

	double desiredX;
	double desiredY;

	double angle;
	double dist;

	const Vector &pos = gps->self().pos;

	if (ballX > FIELD_WIDTH / 2.0) {
		// ball in right strip, move to left
		angle = HALF_CIRCLE - STRIKER_ANGLE;
		dist = STRIKER_DIST;
	}
	else {
		// ball in left strip, move to right
		angle = STRIKER_ANGLE;
		dist = STRIKER_DIST;
	}

	desiredX = ballX + dist * cos(DEG2RAD(angle));
	desiredY = ballY + dist * sin(DEG2RAD(angle));

	if (desiredY > FIELD_LENGTH * 3 / 4) {
		desiredY = FIELD_LENGTH * 3 / 4;
	}

	if (desiredY < FIELD_LENGTH / 4) {
		desiredY = FIELD_LENGTH / 4;
	}
	/*
	   double moveX = desiredX - pos.x;
	   double moveY = desiredY - pos.y;
	   double dist2move = sqrt(SQUARE(moveX) + SQUARE(moveY));
	   if (dist2move > 50) {
	   double desiredH = RAD2DEG(atan2(moveY, moveX));
	   double deltaH = NormalizeAngle_180(desiredH - gps->self().h);
	   walkType = ZoidalWalkWT;
	   forward = MAX_OFFSET_FORWARD;
	   left = 0;
	   turnCCW = CLIP(deltaH/2.0, 40.0);
	   if (ABS(turnCCW) > 15) {
	   forward = 0.5;
	   walkType = CanterWalkWT;
	   }
	   } else {
	   double desiredH = NormalizeAngle_0_360(180 + angle);
	   saGoToTargetFacingHeading(desiredX, desiredY, desiredH);
	   }
	   */    

	CurObject point;
	point.x = desiredX;
	point.y = desiredY;
	point.calcHD(gps->self().pos.x, gps->self().pos.y, gps->self().h);
	saGoToTargetFacingHeading(point, ballH);

	if (gps->canSee(vobBall)) {
		malakTheAvengerTrackBall();
	}
	else {
		trackCalcBall();
	}

	// trigger for active localise
	if (malakTheAvengerActiveLocaliseCounter >= 24
			&& turnCCW <= 15
			&& !malakTheAvengerDisallowActiveLocalise) {
		malakTheAvengerActiveLocaliseCounter = 0;

		setNewBeacons(LARGE_VAL, 50);
		activeGpsLocalise(false);
		malakTheAvengerActiveLocaliseTimer = 8;
	}

	malakTheAvengerActiveLocaliseCounter++;
}

double uther::malakTheAvengerGetSmallestAngleBetweenBallTeammate() {
	double headingToGpsBall = gps->getBall(LOCAL).head;
	double smallestAngleDiffSoFar = LARGE_VAL;
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].var > MAX_VAR_USE_VISION)
			continue;
		double currAngleDiff = ABS(headingToGpsBall - vTeammate[i].h);
		if (smallestAngleDiffSoFar > currAngleDiff)
			smallestAngleDiffSoFar = currAngleDiff;
	}
	return smallestAngleDiffSoFar;
}

bool uther::malakTheAvengerBallInMiddleStrip() {
	if (ballX > FIELD_WIDTH / 3.0 && ballX < FIELD_WIDTH * 2.0 / 3.0) {
		return true;
	}
	return false;
}        

bool uther::malakTheAvengerIsInMiddleStrip() {
	if (gps->self().pos.x > FIELD_WIDTH / 3.0
			&& gps->self().pos.x < FIELD_WIDTH * 2.0 / 3.0) {
		return true;
	}
	return false;
}          


bool uther::malakNeedVisualBackoff() {
	// check if teammate in front of you with ball
	for (int i = 0; i <NUM_TEAM_MEMBER && malakTeammateSeenTime> 4; i++) {
		if (vTeammate[i].var < get95CF(50)
				&& ABS(vTeammate[i].h) < 45
				&& vBall.cf > 0
				&& vBall.d < 50
				&& vTeammate[i].d < vBall.d + 30) {
			// do not lock into backoff mode for 20 frames if dog facing right way
			if (gps->self().h > 20 && gps->self().h < 160) {
				if (vTeammate[i].y < vBall.y)
					malakBackoffTeammateBall = 24;
				else
					malakBackoffTeammateBall = 1;
			}
			else {
				malakBackoffTeammateBall = 20;
			}
			return true;
		}
	}


	// check if teammate in front of you regardless of  ball
	for (int i = 0; i <NUM_TEAM_MEMBER && malakTeammateSeenTime> 4; i++) {
		if (vTeammate[i].var < get95CF(50)
				&& ABS(vTeammate[i].h) < 45
				&& vTeammate[i].d < 30) {
			malakBackoffTeammate = 20;
			//malakBackoffTeammateIndex = i;
			return true;
		}
	}

	if (malakBackoffTeammateBall || malakBackoffTeammate) {
		return true;
	}

	return false;
}             


bool uther::malakTeammateInFace() {
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].var < get95CF(50)
				&& vTeammate[i].d < 40
				&& ABS(vTeammate[i].h) < 45) {
			return true;
		}
	}
	return false;
}


void uther::trackCalcBall() {
	Vector v;
	v.setAngleNorm(HALF_CIRCLE);
	v.setVector(vCART, ballX - gps->self().pos.x, ballY - gps->self().pos.y);
	v.rotate(90 - (gps->self()).h);

	if (v.y > 0) {
		headtype = ABS_XYZ;
		panx = -v.x;
		tilty = 2 * BALL_RADIUS;
		cranez = v.y;
	}
	else {
		headtype = ABS_H;
		tilty = -90;
		if (v.x > 0) {
			panx = -90;
		}
		else {
			panx = 90;
		}
	}
}  

static double old_tilt = 0;
static double old_pan = 0;

void uther::malakTheAvengerTrackBall() {
	if (gps->canSee(vobBall)) {
		double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, z5;

		x1 = vBall.d * tan(radians(vision->vob[vobBall].imgHead));
		y1 = vBall.d * tan(radians(PointToElevation(vision->vob[vobBall].misc)));

		// if middle of ball is higher than top of ball (due to a 
		// misbound), track top of ball

		if (vision->vob[vobBall].imgElev
				> PointToElevation(vision->vob[vobBall].misc)) {
			y1 = vBall.d * tan(radians(PointToElevation(vision->vob[vobBall].misc)));
		}

		z1 = vBall.d;

		x2 = x1;
		y2 = y1 + NECK_LENGTH;
		z2 = z1 + FACE_LENGTH;

		x3 = x2 * cos(radians(-hPan)) - z2 * sin(radians(-hPan));
		y3 = y2;
		z3 = x2 * sin(radians(-hPan)) + z2 * cos(radians(-hPan));

		x4 = x3;
		y4 = z3 * sin(radians(hTilt)) + y3 * cos(radians(hTilt));
		z4 = z3 * cos(radians(hTilt)) - y3 * sin(radians(hTilt));

		if (z4 < NECK_LENGTH)
			z4 = NECK_LENGTH;

		tilty = degrees(atan2(y4, z4)
				- asin(NECK_LENGTH / sqrt(z4 * z4 + y4 * y4)));

		x5 = x4;

		z5 = z4 * cos(radians(-tilty)) - y4 * sin(radians(-tilty));

		panx = degrees(atan2(x5, z5));
		headtype = ABS_H;

		old_tilt = tilty;
		old_pan = panx;
	}
	else {
		tilty = old_tilt;
		panx = old_pan;
	}
}      


void uther::malakTurnKick90() {
	malakTheAvengerTrackBall();

	//setTurnKickParams();

	if (*stepComplete || malakTurnKickCounter) {
		malakTurnKickCounter++;
	}


	// after 4 frames, raise head by a little
	if (malakTurnKickCounter >= turnKick90LiftHead) {
		panx = 0;
		tilty = turnKickLiftTilt;
	}

	/*if (turnKickCounter <= 4) {
	  forward = -0.5;
	  }*/         


	// exit turn kick upon completion or when ball pops out
	if ((malakTurnKickCounter >= turnKick90MaxCounter && *stepComplete) /*|| gps->getBall(LOCAL).d > 40*/) {
		malakTurnKickCounter = 0;
		lockMode = ResetMode;

		// don't grab ball for next 50 frames
		grabInterval = 50;

		malakTheAvengerTrackBall();
	}
}  

void uther::malakTurnKick180() {
	malakTheAvengerTrackBall();

	//setTurnKickParams();

	if (*stepComplete || malakTurnKickCounter) {
		malakTurnKickCounter++;
	}


	// after 4 frames, raise head by a little
	if (malakTurnKickCounter >= turnKick180LiftHead) {
		panx = 0;
		tilty = turnKickLiftTilt;
	}

	/*if (turnKickCounter <= 4) {
	  forward = -0.5;
	  }*/         


	// exit turn kick upon completion or when ball pops out
	if ((malakTurnKickCounter >= turnKick180MaxCounter && *stepComplete) /*|| gps->getBall(LOCAL).d > 40*/) {
		malakTurnKickCounter = 0;
		lockMode = ResetMode;

		// don't grab ball for next 50 frames
		grabInterval = 50;

		malakTheAvengerTrackBall();
	}
}  

void uther::malakTheAvengerOutsideEdgeKickApproach() {
	// if ball right beside it, perform outside edge turn kick
	if ((panx <-45 || panx> 45) && tilty < -20) {
		setWalkParams();
		walkType = CanterWalkWT;
		//turnCCW = 0;
		//forward = 6;
		left = (vBall.h > 0 ? 7 : -7);
		malakTheAvengerMoveSideways = true;
		malakTheAvengerMoveSidewaysTimer = 0;
		malakTheAvengerMoveToLeft = (vBall.h > 0 ? true : false);
		return;
	}

	leds(2, 2, 2, 2, 2, 2);
	// go to ball offset position
	const double BODY_WIDTH_OFFSET = 12;

	double desiredAngle;

	/*
	   if (vBall.h > 0) {
	   desiredAngle = -RAD2DEG(BODY_WIDTH_OFFSET/vBall.d) + vBall.h;
	   } else {
	   desiredAngle = RAD2DEG(BODY_WIDTH_OFFSET/vBall.d) + vBall.h;
	   }
	   */

	// for testing, make it always left
	desiredAngle = vBall.h + RAD2DEG(BODY_WIDTH_OFFSET / vBall.d);

	setWalkParams();
	walkType = ZoidalWalkWT;
	left = 0;
	forward = MAX_OFFSET_FORWARD;
	turnCCW = CLIP(desiredAngle, 10.0);
}

void uther::malakTheAvengerPawKick(int pawType) {
	leds(2, 2, 2, 2, 2, 2);
	// go to ball offset position
	const double BODY_WIDTH_OFFSET = 5;

	double desiredAngle;
	if (pawType == MALAK_LEFT_PAW) {
		desiredAngle = vBall.h - RAD2DEG(BODY_WIDTH_OFFSET / vBall.d);
	}
	else {
		desiredAngle = vBall.h + RAD2DEG(BODY_WIDTH_OFFSET / vBall.d);
	}

	setWalkParams();
	walkType = ZoidalWalkWT;
	left = 0;
	forward = MAX_OFFSET_FORWARD;
	turnCCW = CLIP(desiredAngle / 2, 40.0);
	if (ABS(turnCCW) > 15) {
		forward = 0.5;
	}
}

void uther::malakTheAvengerDribble() {
	/*
	   if (vBall.d < 40 || ABS(gps->tGoal().pos.head) <= 10 || utilIsInOwnHalf()) {
// go to ball offset position for paw-kick
const double BODY_WIDTH_OFFSET = 4;
double desiredAngle;
if (vBall.h > 0) {
desiredAngle = -RAD2DEG(BODY_WIDTH_OFFSET/vBall.d) + vBall.h;
} else {
desiredAngle = RAD2DEG(BODY_WIDTH_OFFSET/vBall.d) + vBall.h;
}
setWalkParams();
walkType = ZoidalWalkWT;
left = 0;
forward = MAX_OFFSET_FORWARD;
turnCCW = CLIP(desiredAngle, 10.0);
} else {
leds(2,2,2,2,2,2);
if (gps->tGoal().pos.head < -10) {
Vector offset (vCART, -10, -10);
offset.rotate(gps->self().h - QUARTER_CIRCLE);
malakTheAvengerGetNearBall(offset.x, offset.y);
} else {
Vector offset (vCART, 10, -10);
offset.rotate(gps->self().h - QUARTER_CIRCLE);
malakTheAvengerGetNearBall(offset.x, offset.y);
}
}
*/    
// go to ball offset position for paw-kick
const double BODY_WIDTH_OFFSET = 4;

double desiredAngle;
if (vBall.h > 0) {
	desiredAngle = -RAD2DEG(BODY_WIDTH_OFFSET / vBall.d) + vBall.h;
}
else {
	desiredAngle = RAD2DEG(BODY_WIDTH_OFFSET / vBall.d) + vBall.h;
}

setWalkParams();
walkType = ZoidalWalkWT;
left = 0;
forward = MAX_OFFSET_FORWARD;
turnCCW = CLIP(desiredAngle, 10.0);
}



void uther::malakSpinKick() {
	// break out of spin kick if timer expires
	if (!grabTime) {
		gps->currentlySpinKicking = false;
		lockMode = GoalieKick;
		aaGoalieKick();
		return;
	}

	double accuracyRequirement = requiredAccuracy(20);
	accuracyRequirement = MAX(accuracyRequirement, 5);
	if (ABS(NormalizeAngle_180(dkd - (gps->self().h))) <= accuracyRequirement) {
		gps->currentlySpinKicking = false;
		lockMode = GoalieKick;
		aaGoalieKick();
		return;
	}
	else {
		setSpinKickParams();
		return;
	}
}



void uther::malakSetSpinKickParams() {
	gps->currentlySpinKicking = true;
	walkType = NormalWalkWT;
	fsO = 25;
	ffO = 80;
	forward = 1;
	turnCCW = CLIP(NormalizeAngle_180(dkd - (gps->self().h)), 40.0);
	left = (NormalizeAngle_180(dkd - (gps->self().h)) > 0) ? -3 : 3;
	headtype = ABS_H;
	panx = 0;
	tilty = -50;
}



/*
 * localise first then spin kick
 */
void uther::malakClearKick() {
	if (malakTheAvengerClearKickCounter) {
		DECREMENT(malakTheAvengerClearKickCounter);
		setHoldParams();
		saBallTracking();
	}
	else if (activeLocaliseTimer) {
		activeGpsLocalise(true);
		setHoldParams();
	}
	else {
		lockMode = SafeSpinKick;
		aaSafeSpinKick();
	}
}



/*
 * localise first then spin chest push
 */
void uther::malakClearPush() {
	if (malakTheAvengerClearPushCounter) {
		DECREMENT(malakTheAvengerClearPushCounter);
		setHoldParams();
		saBallTracking();
	}
	else if (activeLocaliseTimer) {
		activeGpsLocalise(true);
		setHoldParams();
	}
	else {
		lockMode = SpinChestPush;
		aaSpinChestPush();
	}
}



void uther::chooseMalakTheAvengerAttackBall() {
	// used to smooth out walk by preventing constant changes in walk type
	if (attackLock) {
		useZoidWalk = 0;
	}
	else {
		useZoidWalk = 1;
	}

	/*
	 * side-kick experiment
	 *
	 if (malakTheAvengerMoveSideways) {
	 malakTheAvengerMoveSidewaysTimer++;
	 if (malakTheAvengerMoveSidewaysTimer < 5) {
	 setWalkParams();
	 walkType = CanterWalkWT;
	 forward = 0;
	 turnCCW = 0;
	 left = (malakTheAvengerMoveToLeft ? 7 : -7);
	 } else {
	 setOmniParams();
	 walkType = CanterWalkWT;
	 forward = 1;
	 left = 1;
	 turnCCW = (malakTheAvengerMoveToLeft ? 25 : -25);
	 }

	 if (malakTheAvengerMoveSidewaysTimer > 10) {
	 malakTheAvengerMoveSidewaysTimer = 0;
	 malakTheAvengerMoveSideways = false;
	 }                        

	 } else {                    
	 malakTheAvengerOutsideEdgeKickApproach();
	 }
	 */

	if (malakNeedVisualBackoff()) {
		leds(2, 2, 2, 1, 1, 1); 
		if (malakBackoffTeammateBall) {
			malakTheAvengerTrackBall();

			/* If really close to teammate then step back. */
			if (malakTeammateInFace()) {
				leds(2, 1, 2, 1, 2, 1);
				forward = -8;
			}

			/* Else get behind ball. */
			else {
				if (ballX < WALL_THICKNESS + 35 && gps->self().h < 0) {
					left = 12;
				}
				else if (ballX > FIELD_WIDTH - (WALL_THICKNESS + 35)
						&& gps->self().h < 0) {
					left = -12;
				}
				else {
					// error condition is the angle to the target goal
					left = CLIP(((-gps->tGoal().pos.head / 60) * 12), 12.0);
				}
				turnCCW = CLIP(ballH / 2, malakBackoffAngleMax);
			}
		} 
		return;
	} 

	if (ballSource == VISION_BALL
			&& vBall.d > 15
			&& vBall.d <50
			&& utilBallOnLREdge()
			&& gps->self().h> 210
			&& gps->self().h < 330) {
		// perform get behind ball if not facing up the field,
		// ball on left/right edge or close to own goal

		saMalakFarGetBehindBall(prevAttackMode != GetBehindBall);
	}
	else if (ballSource == VISION_BALL
			&& vBall.d < 50
			&& utilBallOnLEdge()
			&& utilIsOnLEdge()
			&& gps->self().h <120
			&& gps->self().h> 60
			&& gps->self().posVar < get95CF(75)) {
		// paw kick on left field edge
		malakTheAvengerPawKick(MALAK_LEFT_PAW);
	}
	else if (ballSource == VISION_BALL
			&& vBall.d < 50 & utilBallOnREdge()
			&& utilIsOnREdge()
			&& gps->self().h <120
			&& gps->self().h> 60
			&& gps->self().posVar < get95CF(75)) {
		// paw kick on right field edge
		malakTheAvengerPawKick(MALAK_RIGHT_PAW);
	}
	else if (ballSource == VISION_BALL
			&& utilIsInOwnHalf()
			&& gps->self().h > 35
			&& gps->self().h < 145
			&& gps->self().posVar < get95CF(75)) {
		if (utilBallInLeftHalf()) {
			malakTheAvengerPawKick(MALAK_LEFT_PAW);
		}
		else {
			malakTheAvengerPawKick(MALAK_RIGHT_PAW);
		}
		/*        
			  } else if (ballSource == VISION_BALL && !utilIsInOwnHalf() &&
			  ABS(gps->tGoal().pos.head) < 45) {

			  if (utilBallInLeftHalf()) {
			  malakTheAvengerPawKick(MALAK_LEFT_PAW);
			  } else {
			  malakTheAvengerPawKick(MALAK_RIGHT_PAW);
			  }                
			  */
}
else {
	// can we do saBallTracking and reset head parameters
	// at the end? i.e. do we need to actively localise or
	// perform find ball with head?
	bool adjustHead = true;

	if (ballSource == WIRELESS_BALL) {
		// if close enough to share ball, perform find ball
		if (ballD < malakWirelessBallDistThreshold) {
			teamDForwardFBDir = (gps->self().pos.x > FIELD_WIDTH / 2)
				? ANTICLOCKWISE
				: CLOCKWISE;
			spinFindBall();

			malakWirelessBallDistThreshold = 85;

			return;
		}
		else {
			malakWirelessBallDistThreshold = 30;

			// track share ball
			trackCalcBall();
			adjustHead = false;
		}
	}

	const Vector &opp = gps->getClosestOppPos(LOCAL);

	// if we can active localise ...
	if (ballSource == VISION_BALL
			&& vBall.d > 80
			&& ABS(vBall.h) < 5
			&& opp.d > 40
			&& !disallowActiveLocalise) {
		setNewBeacons(LARGE_VAL, 50);
		activeGpsLocalise(false);
		activeLocaliseTimer = 8;
		adjustHead = false;
	}

	////////////////////////////////////////////////////////////////////////

	double maxCanterForwardSpeed = 8;
	double maxCanterLeftSpeed = 6;

	double relH = ballH;

	turnCCW = CLIP(relH / 2.0, 40.0);

	// if the ball is at a high angle, walk forward and left appropriately whilst turning
	if (abs(relH) > 17) {
		setOmniParams();
		walkType = CanterWalkWT;

		double maxF = maxCanterForwardSpeed;
		double maxL = maxCanterLeftSpeed;

		//calculate the max forward and left you can do whilst turning
		//counter clockwise case turning slow
		if (turnCCW < 10 && turnCCW >= 0) {
			maxF = 6.5;
			maxL = 6.5;
		} 
		//counter clockwise case turning faster
		else if (turnCCW < 20 && turnCCW >= 0) {
			maxF = 5;
			maxL = 5;
		} 
		//turn too fast anticlockwise case
		else if (turnCCW >= 0) {
			maxF = 3;
			maxL = 2;
		}

		//slow clockwise turn case
		else if (turnCCW > -10 && turnCCW < 0) {
			maxF = 5;
			maxL = 5;
		} 
		//all other clockwise cases
		else {
			maxF = 1;
			maxL = 3;
		} 

		//calculate how much you should go forward and left whilst turning
		double fComp = cos(DEG2RAD(relH));
		double lComp = sin(DEG2RAD(relH));
		double scale;
		if (maxF * ABS(fComp) >= maxL * ABS(lComp)) {
			scale = 1.0 / ABS(fComp);
		}
		else {
			scale = 1.0 / ABS(lComp);
		}

		fComp = scale * fComp;
		lComp = scale * lComp;

		forward = maxF * fComp;
		left = maxL * lComp;
		//cout << "forward, left, turn " << forward << "," << left << "," << turnCCW;
	} 
	//otherwise use zoidal walk to run to the ball
	else {
		setWalkParams();
		walkType = ZoidalWalkWT;
		left = 0;
		forward = MAX_OFFSET_FORWARD;
	}

	//the rest is guards to make sure your not doing crazy things with the walk
	if (forward <0 || ABS(turnCCW)> 15) {
		walkType = CanterWalkWT;
	}

	// on close approach to ball, slow down accordingly
	if (ballD < 15) {
		walkType = CanterWalkWT;
		forward = 7.5;
		left = 0;
		turnCCW = CLIP(ballH / 2.0, 40.0);
		if (ABS(turnCCW) > 15) {
			forward = 0.5;
		}
	}

	if (walkType == CanterWalkWT) {
		if (forward > maxCanterForwardSpeed) {
			forward = maxCanterForwardSpeed;
		}
	}
	else {
		if (forward > MAX_OFFSET_FORWARD) {
			forward = MAX_OFFSET_FORWARD;
		}
	}

	if (walkType == CanterWalkWT && attackLock == 0) {
		attackLock = 20;
	}
	attackMode = GoToBall;

	// saBallTracking caters for the offset created by the body's turn
	if (adjustHead) {
		malakTheAvengerTrackBall();
	}
}
}        

void uther::malakTheAvengerGetNearBall(double xoffset, double yoffset) {
	// calculate the vectors
	Vector target(vCART,
			vBall.x
			+ 6.0 * cos(DEG2RAD(gps->self().getHackedH())) + xoffset,
			vBall.y
			+ 6.0 * sin(DEG2RAD(gps->self().getHackedH())) + yoffset);
	Vector robot(vCART, (double) gps->self().pos.x, (double) gps->self().pos.y);

	// vector to destination point
	Vector vx(robot);
	vx.sub(target);

#define pointAngle3 utilNormaliseAngle( (vx.getHackedTheta() - gps->self().getHackedH()) )

	setWalkParams();
	walkType = ZoidalWalkWT;
	turnCCW = Cap(vBall.h / 2.0, 20);
	forward = -MAX_OFFSET_FORWARD * cos(DEG2RAD(pointAngle3));
	left = -7.2 * sin(DEG2RAD(pointAngle3));
}

void uther::chooseMalakTheAvengerShootStrategy() {
	// perform turn-kick for kicking off
	if (utherKickingOff && gps->self().h > 20 && gps->self().h < 160) {
		/*    
		      lockMode = ProperVariableTurnKick;
		      setProperVariableTurnKick(90);
		      aaProperVariableTurnKick();
		      */
		lockMode = LightningKick;
		aaLightningKick();        
		utherKickingOff = false;
		return;
	}

	// if you turn-kicked in the last 60 frames, and you
	// find yourself in possession of the ball again,
	// do a chest-push

	if (malakTheAvengerTurnKickTimer > 0 || malakTheAvengerSpinKickTimer > 0) {
		lockMode = ChestPush;
		aaChestPush();
		return;
	}

	grabTime = 65;

	double reqAccuracy = requiredAccuracy(25);

	//local coords
	double minGoalHead = gps->tGoal().pos.head - reqAccuracy;
	double maxGoalHead = gps->tGoal().pos.head + reqAccuracy;

	double min = MIN(abs(minGoalHead), abs(maxGoalHead));
	double max = MAX(abs(minGoalHead), abs(maxGoalHead));

	double gh = gps->tGoal().pos.head;
	double sh = gps->self().h;

	//--- opponent(s) near --------------------------------------
	/*if (utilOppWithinDist(100)) {
	  if (utilIsInPenaltyArea() && utilIsInFrontTargetGoal()) {
	// in front of target goal
	if (ABS(gh) <= 10) {
	lockMode = LightningKick;
	aaLightningKick();
	} else if (ABS(gh) <= 60) {
	lockMode = SpinChestPush;
	aaSpinChestPush();
	} else {
	lockMode = ProperVariableTurnKick;
	if (gh >= 0 && gh <= 170) {
	setProperVariableTurnKick(90);
	} else if (gh <= 0 && gh >= -170) {
	setProperVariableTurnKick(-90);
	} else if (gh >= 170) {
	setProperVariableTurnKick(180);
	} else if (gh <= -170) {
	setProperVariableTurnKick(-180);
	}
	aaProperVariableTurnKick();
	}
	} else if (gps->self().pos.y > FIELD_LENGTH/2) {
	// in target half
	if (ABS(gh) <= 10) {
	lockMode = LightningKick;
	aaLightningKick();
	} else if (ABS(gh) <= 60) {
	//lockMode = SpinKick;
	//aaSpinKick();
	lockMode = SafeSpinKick;
	aaSafeSpinKick();
	} else if (ABS(gh) <= 160) {
	if (gh > 0) {
	setProperVariableTurnKick(90);
	} else {
	setProperVariableTurnKick(-90);
	}
	lockMode = ProperVariableTurnKick;
	aaProperVariableTurnKick();
	} else {
	if (gh > 0) {
	setProperVariableTurnKick(180);
	} else {
	setProperVariableTurnKick(-180);
	}
	lockMode = ProperVariableTurnKick;
	aaProperVariableTurnKick();
	}
	} else {
	// in own half
	if (sh >= 45 && sh <= 135) {
	lockMode = LightningKick;
	aaLightningKick();
	} else if (sh >= 135 && sh <= 260) {
	lockMode = ProperVariableTurnKick;
	setProperVariableTurnKick(-90);
	aaProperVariableTurnKick();
	} else if (sh >= 260 && sh <= 280) {
	double sx = gps->self().pos.x;
	if (sx < FIELD_WIDTH/2) {
	setProperVariableTurnKick(-180);
	} else {
	setProperVariableTurnKick(180);
	}
	lockMode = ProperVariableTurnKick;
	aaProperVariableTurnKick();
	} else {
	lockMode = ProperVariableTurnKick;
	setProperVariableTurnKick(90);
	aaProperVariableTurnKick();
	}
}
*/
/*
   const Vector& opp = gps->getClosestOppPos(LOCAL);                 
   const Vector& pos = gps->self().pos;
   double h = gps->self().h;
   const Vector& goal = gps->tGoal().pos;
   bool oppInFront = (opp.d < closestOppNearDistance &&
   ABS(opp.head) < opponentIsInFront);
   if (pos.y < FIELD_LENGTH/2) {
// defensive half
if (h > 225 && h < 315) {
//we are facing the wrong direction
lockMode = MalakTurnKick180;
turnDir = (h > 270) ? 1 : -1;
malakTurnKick180();
} else if (h < 150 && h > 30 && !oppInFront) {
// we are facing the right direction and aren't blocked by an opponent
lockMode = LightningKick;
aaLightningKick();
} else {
// we are facing across the field or have an opponent blocking us
lockMode = MalakTurnKick90;
turnDir = (h < 90 || h > 270) ? 1 : -1;
malakTurnKick90();
}
} else {
// offensive half
if (pos.y > chestPushRange && pos.x > FIELD_WIDTH/3 &&
pos.x < 2*FIELD_WIDTH/3) {
lockMode = SpinChestPush;
aaSpinChestPush();
} else if (ABS(goal.head) > 135) {
//we are facing the wrong direction
lockMode = MalakTurnKick180;
turnDir = (goal.head > 0) ? 1 : -1;
malakTurnKick180();
} else if (h < 150 && h > 30 && !oppInFront) {
// we are facing the right direction and aren't blocked by an opponent
lockMode = SpinKick;
aaSpinKick();
} else {
// we are facing across the field or have an opponent blocking us
lockMode = MalakTurnKick90;
turnDir = (goal.head > 0) ? 1 : -1;
malakTurnKick90();
}
}
*/

//--- in the clear ------------------------------------------
//} else {

//-- in front of goal -------
if (utilIsInPenaltyArea() && utilIsInFrontTargetGoal()) {
	if (ABS(gh) <= 10) {
		// line up with goal
		lockMode = LightningKick;
		aaLightningKick();
	}
	else {
		// not line up with goal
		malakTheAvengerClearPushCounter = 3; 
		activeLocaliseTimer = 15;
		setNewBeacons(LARGE_VAL, 50);
		lockMode = MalakClearPush;
		setHoldParams();
	}

	//-- corner area near goal --

}
else if (utilIsInPenaltyArea()) {
	//in corner
	//if (gps->self().pos.x > FIELD_WIDTH - 50 || 
	//    gps->self().pos.x < 50) {
	//    utilDetermineDKDaimAtCorner();
	//    lockMode = MalakSpinKick;
	//    malakSpinKick();
	//not in corner
	//} else {

	malakTheAvengerClearPushCounter = 3; 
	activeLocaliseTimer = 15;
	setNewBeacons(LARGE_VAL, 50);
	lockMode = MalakClearPush;
	setHoldParams();
	//}

	//-- not in front of goal ---
}
else {
	//-- need small turn to line up with goal
	if (ABS(gh) <= 90) {
		malakTheAvengerClearKickCounter = 3; 
		activeLocaliseTimer = 20;
		setNewBeacons(LARGE_VAL, 50);
		lockMode = MalakClearKick;
		setHoldParams();

		//-- big angle from goal
	}
	else {
		// target half
		if (gps->self().pos.y > FIELD_LENGTH / 2) {
			if (ABS(gh) <= 160) {
				if (gh > 0) {
					setProperVariableTurnKick(90);
				}
				else {
					setProperVariableTurnKick(-90);
				}
				lockMode = ProperVariableTurnKick;
				aaProperVariableTurnKick();
			}
			else {
				if (gh > 0) {
					setProperVariableTurnKick(180);
				}
				else {
					setProperVariableTurnKick(-180);
				}
				lockMode = ProperVariableTurnKick;
				aaProperVariableTurnKick();
			}

			// in own half

		}
		else {
			if (sh >= 45 && sh <= 135) {
				lockMode = LightningKick;
				aaLightningKick();
			}
			else if (sh >= 135 && sh <= 260) {
				lockMode = ProperVariableTurnKick;
				setProperVariableTurnKick(-90);
				aaProperVariableTurnKick();
			}
			else if (sh >= 250 && sh <= 290) {
				double sx = gps->self().pos.x;
				if (sx < FIELD_WIDTH / 2) {
					setProperVariableTurnKick(-180);
				}
				else {
					setProperVariableTurnKick(180);
				}
				lockMode = ProperVariableTurnKick;
				aaProperVariableTurnKick();
			}
			else {
				lockMode = ProperVariableTurnKick;
				setProperVariableTurnKick(90);
				aaProperVariableTurnKick();
			}
		}
	}
}
//}

}        

/////////////////////////////////////////////////
// END MALAK THE AVENGER FORWARD FUNCTIONS
/////////////////////////////////////////////////
