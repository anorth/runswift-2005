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
 * $Id: zannTheDefender.cc 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * This code is the sole property of Team Uther Lightbringer and the Silver Hand
 * Zann The Defender
 *
 * Originated from team C goalie 12/04/03
 *
 **/ 

//#define ZANN_THE_DEFENDER_OLD_LEDS
#define ZANN_THE_DEFENDER_ROLE_LEDS

#include "Behaviours.h"
#include "../share/BallDef.h"
#include "teamUtherCommon.h"


// num frames in 1/2 cycle of head, finding ball
//static const int goalieHEAD_PERIOD = 32;

//static const int GOAL_CENTRE_X = (FIELD_WIDTH / 2);
//static const int GOAL_CENTRE_Y = 0;

static const double DEFAULT_POS_X = FIELD_WIDTH / 2;
static const double DEFAULT_POS_Y = WALL_THICKNESS + 25;
static const double DEFAULT_POS_H = 90;

static const int NORMAL_TURN_AROUND_STEP = 10;
static const int FASTER_TURN_AROUND_STEP = 30;

static const int POS_ERROR_MARGIN = 10;
static const int H_ERROR_MARGIN = 8;
static const int POS_ERROR_MARGIN_LARGER = 20;
static const int H_ERROR_MARGIN_LARGER = 15;

// num frames spent in finding ball
static int goalieFindBallCounter = 0;

static int goalieLookAroundCounter = 150;
static int goalieLookAroundTimer = 0;

static bool goalieBlockStarted = false;
static int goalieBlockTimer = 0;

static bool attackBallMode;

static int lastHeadType;
static double lastHeadTilt;
static double lastHeadPan;
static double oldBallx;
static double oldBally;


static int lookAtBeaconTimer;      // duration of active localisation
static int lookAtBeaconCounter;     // frequency of active localisation

static int goalieHoldBallTimer;

static int checkLastBallPosTimer;

static bool haventGivenGpsAChance = true;

// num frames spent in finding ball
static int findBallCounter = 0;

static bool usingGpsLastPos = false;

// for zannGotBall
static bool clockwise = true;

static bool lostBallWhenReallyClose = false;

static const int vBallLost = 10;

static int ballSource = VISION_BALL;
static double ballX = 0;
static double ballY = 0;
static double ballH = 0;
static double ballD = 0;


namespace uther {
	double allowedAttackDist;
	double allowedAttackDem;

	double blockDistance;
	double blockSpeed;
	double blockAngle;
	int blockLength;
}

void uther::initZannTheDefender() {
	lostBall = 0;
	seeBall = 0;
	lastSawBeacon = 0;


	//ERIC TEMP REMOVED THIS
	//goalieResetFindBallCounter();

	isWirelessDown = true;

	lockMode = NoMode;

	dkd = 90;

	ballUnderChinTilt = -55;

	attackMode = GoToBall;
	prevAttackMode = GoToBall;

	grabTime = 0;
	grabInterval = 0;

	turnDir = -1;

	goalieLookAroundCounter = 150;
	goalieBlockTimer = 0;
	goalieBlockStarted = false;

	attackBallMode = false;

	lookAtBeaconCounter = 0;
	lookAtBeaconTimer = 0;

	lastHeadType = ABS_H;
	lastHeadTilt = 0;
	lastHeadPan = 0;

	goalieHoldBallTimer = 0;

	checkLastBallPosTimer = 0;   

	blockDistance = 54;
	blockSpeed = 1.8;
	blockAngle = 45;
	blockLength = 10;

	allowedAttackDist = 50;
	allowedAttackDem = 2;
}

void uther::doZannTheDefender() {
	// we are the goalie
	setGoalie(gps);

	setDefaultParams();
	zannTheDefenderValues();
	chooseZannTheDefenderStrategy();
}

void uther::zannTheDefenderValues() {
	lastSawBeacon++;
	for (int i = vobBlueLeftBeacon; i <= vobYellowRightBeacon; i++) {
		if (vision->vob[i].cf != 0) {
			lastSawBeacon = 0;
			break;
		}
	}

	DECREMENT(grabInterval);
	DECREMENT(grabTime);
	DECREMENT(goalieLookAroundTimer);

	//decrementDoNotLocalise();
	decrementBackOff();
	//decrementAttackLock();

	setTeammateSeenTime();

	if (lockMode != prevLockMode) {
		prevLockMode = lockMode;
	}

	utilDetermineDKD();

	// default to GoToBall; overridden by doAttackBall
	attackMode = GoToBall;

	// prevents ghost kicks
	if (*stepComplete && lockMode == ResetMode) {
		lockMode = NoMode;
	}

	if (vision->vob[vobBall].cf > 3) {
		seeBall = MIN(seeBall + 1, 30);
	}
	else {
		seeBall = 0;
	}

	// determine which ball values to use
	if (gps->canSee(vobBall)) {
		ballSource = VISION_BALL;
		ballX = vBall.x;
		ballY = vBall.y;
		ballH = vBall.h;
		ballD = vBall.d;
	}
	else if (gps->shareBallvar < LARGE_VAL) {
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
	}
	else {
		ballSource = GPS_BALL;
		ballX = gps->getBall(GLOBAL).x;
		ballY = gps->getBall(GLOBAL).y;
		ballH = gps->getBall(LOCAL).head;
		ballD = gps->getBall(LOCAL).d;
	}
}

double zannDefendX, zannDefendY, zannDefendH;

void uther::chooseZannTheDefenderStrategy() {
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
	if (friendHaveBall(gps)) {
		leds(2, 0, 0, 2, 0, 0);
	}
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS  

	if (mode_ == mStartMatch) {
		saBallTracking();
	}
	else if (lockMode != NoMode) {
		// execute atomic action if in locked mode
		switch (lockMode) {
			/*
			   case GrabBall:
			   aaGrabBall();
			   setHaveBall(gps);
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
leds(0,2,0,0,2,0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS      
break;
case HoldBall:
aaHoldBall();
setHaveBall(gps); 
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
leds(0,2,0,0,2,0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS  
break;
			//case TurnKick:
			//aaTurnKick();
			//setHaveBall(gps); 
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
			//leds(0,2,0,0,2,0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS  
			//break;

*/
			case GotBall: zannGotBall();
				      setHaveBall(gps); 
				      break;
			case ChestPush: aaChestPush();
					setHaveBall(gps); 
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
					leds(0, 2, 0, 0, 2, 0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS  
					break;
			case SpinKick: aaSpinKick();
				       setHaveBall(gps); 
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
				       leds(0, 2, 0, 0, 2, 0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS  
				       break;
				       //case ActiveLocalise:
				       //aaActiveLocalise();
				       //break;
			case SpinChestPush: aaSpinChestPush();
					    setHaveBall(gps); 
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
					    leds(0, 2, 0, 0, 2, 0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS  
					    break;
			case GoalieKick: aaGoalieKick();
					 setHaveBall(gps); 
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
					 leds(0, 2, 0, 0, 2, 0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS  
					 break;
			case LightningKick: aaLightningKick();
					    setHaveBall(gps);
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
					    leds(0, 2, 0, 0, 2, 0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS   
					    break;
					    //case VariableTurnKick:
					    //aaVariableTurnKick();
					    //setHaveBall(gps); 
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
					    //leds(0,2,0,0,2,0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS  
					    //break;
					    //ERIC TEMP REMOVED THIS
					    //case GoalieLookAround:
					    //goalieLookAround();
					    //break;
			case LookAtBeacon: zannLookAtBeaconAndGoToDefendPosition();
					   break;                
			case GoalieHoldBall: zannaaGoalieHoldBall();
					     setHaveBall(gps); 
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
					     leds(0, 2, 0, 0, 2, 0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS  
					     break;
		}
	}
	else {
		// ricky: if almost all i can see is my own goal, then I should turn around becuase I'm probably inside of goal
		int ownGoal;
		const int SIZE_THRESHOLD = 5;
		if (gps->targetGoal == vobBlueGoal)
			ownGoal = vobYellowGoal;
		else
			ownGoal = vobBlueGoal;

		if (vision->vob[ownGoal].cf > 0) {
			if ((vision->vob[ownGoal].width > WIDTH - SIZE_THRESHOLD)
					|| ((vision->vob[ownGoal].height > HEIGHT - SIZE_THRESHOLD)
						&& (vision->vob[ownGoal].width > WIDTH / 2))) {
				attackBallMode = false;
#ifdef ZANN_THE_DEFENDER_OLD_LEDS
				leds(2, 1, 1, 1, 1, 1);
#endif // ZANN_THE_DEFENDER_OLD_LEDS
				if (vision->vob[ownGoal].head >= 0) {
					turnCCW = -30;
				}
				else {
					turnCCW = 30;
				}
			}
		}
		// end goal check

		//if (attackBallMode && vBall.cf > 3 && 
		//    !zannGoalieBallWithinDangerDistance()) {
		//    attackBallMode = false;
		//}        

		// right after it has looked at beacon, we want
		// to look at where it last saw the ball
		//
		// Or if we are giving the gps location of the ball a chance
		if (checkLastBallPosTimer > 0) {
			//if looking at gps ball
			if (usingGpsLastPos) {
#ifdef ZANN_THE_DEFENDER_OLD_LEDS
				if (lostBallWhenReallyClose) {
					leds(2, 2, 2, 2, 2, 2);
				}
				else {
					leds(1, 1, 2, 1, 1, 1);
				}
#endif // ZANN_THE_DEFENDER_OLD_LEDS
				//if (gps->getBall(LOCAL)Var <= get95CF(65)) {
				zannSetGpsTrackBallHeadParams(); 
				//try to block against phantom balls
				//yes this is very much a hack... 
				if (zannBallJumpedTooMuch()) {
					headtype = lastHeadType;
					tilty = lastHeadTilt;
					panx = lastHeadPan;
				}
				lastHeadType = headtype;
				lastHeadTilt = tilty;
				lastHeadPan = panx;
				oldBallx = gps->getBall(LOCAL).x;
				oldBally = gps->getBall(LOCAL).y;
				//} 
				//look one last time and thats it
				//else {
				//    headtype = lastHeadType;
				//    tilty = lastHeadTilt;
				//    panx = lastHeadPan;
				//    checkLastBallPosTimer = 0;
				//}   

				if (attackBallMode) {
					zannGoalieAttackGpsBall();
				}
				else {
					setStandParams();
				}
			} 
			//looking back after just actively localising
			else {
#ifdef ZANN_THE_DEFENDER_OLD_LEDS
				leds(1, 1, 2, 1, 1, 2);
#endif // ZANN_THE_DEFENDER_OLD_LEDS
				headtype = lastHeadType;
				tilty = lastHeadTilt;
				panx = lastHeadPan;
				setStandParams();
			}

			checkLastBallPosTimer--;


			/*
			   if (farAwayFromPosition() && !zannHasReachedDefendPositionHeading(defendX, defendY, defendH)) {
			   zannGoToDefendPositionHeading(defendX, defendY, defendH);
			   } else {
			   setStandParams();
			   }
			   */            

			//if you can now see the ball break out of the 
			//checkLastBallPosTimer lock
			if (vBall.cf >= 3) {
				checkLastBallPosTimer = 0;
				//do nothing which will hence go through and do the rest of the code and set something else to do
			}
			else {
				return;
			}
		}
		if (checkLastBallPosTimer == 0) {
			usingGpsLastPos = false;
			lostBallWhenReallyClose = false;
		}

		//might want to implement something that aborts the block if the block condition is no longer true
		if (goalieBlockTimer > 0) {
			setBlockParams();
			zannSetTrackBallHeadParams();
			goalieBlockTimer--;

			//if this is the end of the block timer, decide if you want to renew it
			if (goalieBlockTimer == 0) {
				//check that you wouldn't rather attack
				if (!zannGoalieBallWithinAttackDistance()) {
					//check block condition
					if (zannGoalieBallWithinBlockDistance()
							&& zannGoalieAllowedToBlock()) {
						//cout << "RENEWING THE BLOCK TO BLOCK FOR LONGER" << endl;
						goalieBlockTimer = blockLength;
					}
				}
			}        
			return;
		}

		// if it has lost ball
		if (vBall.cf < 3) {
			//give gps ball a chance before doing a find ball
			if (haventGivenGpsAChance) {
				if (attackBallMode) {
					zannGoalieAttackGpsBall();
				}
				else {
					zannGoToDefaultPositionHeading();
				}
				zannSetGpsTrackBallHeadParams(); //this needs to be called before the lastHead* lines
				usingGpsLastPos = true;
				lastHeadType = headtype;
				lastHeadTilt = tilty;
				lastHeadPan = panx;
				if (panx <= 20 && panx >= -20 && tilty <= -40) {
					lostBallWhenReallyClose = true;
					checkLastBallPosTimer = 20; //HMmm making this larger may have no effect because if you see above, theres a variance based override
				}
				else {
					checkLastBallPosTimer = 12;
				}
				haventGivenGpsAChance = false;                
				oldBallx = gps->getBall(LOCAL).x;
				oldBally = gps->getBall(LOCAL).y;

				return;
				/*
				   if (!zannHasReachedDefendPositionHeadingWhenLookingForBall(defendX, defendY, defendH)) {
				   zannGoToDefendPositionHeading(defendX, defendY, defendH);
				   } else {
				   setStandParams();
				   }
				   */
			}
			/*
			   else {
			   setStandParams();
			   }
			   */

			// use wireless ball
			if (ballSource == WIRELESS_BALL) {
				leds(2, 2, 2, 2, 2, 2);                    
				zannUseWirelessBall();
				return;
			}

			attackBallMode = zannAllowedToAttackBall();
			if (attackBallMode) {
#ifdef ZANN_THE_DEFENDER_OLD_LEDS
				leds(2, 2, 1, 2, 2, 1); 
#endif // ZANN_THE_DEFENDER_OLD_LEDS
				//findBallHeadParams();
				zannGoalieHeadFindBall();
				if (!zannHasReachedDefendPositionHeadingWhenLookingForBall(zannDefendX,
							zannDefendY,
							zannDefendH)) {
					zannGoToDefendPositionHeading(zannDefendX, zannDefendY, zannDefendH);
				}
				else {
					setStandParams();
				}

				return;
			}
			else {
#ifdef ZANN_THE_DEFENDER_OLD_LEDS
				leds(1, 2, 1, 1, 2, 1); 
#endif
				//findBallHeadParams();
				zannGoalieHeadFindBall();
				if (!zannHasReachedDefaultPosition()) {
					zannGoToDefaultPositionHeading();
					return;
				}
				else {
					//zannGoalieSemiCircleFindBall();
					setStandParams();
					//findBallHeadParams();
					zannGoalieHeadFindBall();
					return;
				}
			}
		} 
		//else you have seen the ball
		else {
			findBallCounter = 0;
			haventGivenGpsAChance = true; //reset the allowed to look at gps ball
		}

		// if you get up to here, the ball is not lost
		attackBallMode = zannAllowedToAttackBall();
		if (attackBallMode) {
#ifdef ZANN_THE_DEFENDER_OLD_LEDS
			leds(2, 1, 1, 2, 1, 1);
#endif // ZANN_THE_DEFENDER_OLD_LEDS
			zannGoalieAttackBall();
			setAdvancing(gps);
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
			leds(0, 2, 0, 0, 0, 0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS  
			return;
		}

#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
		if (zannBallIsInBox()) {
			leds(0, 0, 2, 0, 0, 2);
		}
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS 

		if (((!friendHaveBall(gps)) && zannGoalieBallWithinAttackDistance())
				|| zannBallIsInBox()) {
#ifdef ZANN_THE_DEFENDER_OLD_LEDS
			leds(2, 1, 1, 2, 1, 1); 
#endif // ZANN_THE_DEFENDER_OLD_LEDS    
			zannGoalieAttackBall();
			attackBallMode = true;
			setAdvancing(gps);
#ifdef ZANN_THE_DEFENDER_ROLE_LEDS
			leds(0, 2, 0, 0, 0, 0);
#endif //  ZANN_THE_DEFENDER_ROLE_LEDS  
			return;
		}

		if (zannGoalieBallWithinBlockDistance()
				&& vBall.d > 30
				&& zannGoalieAllowedToBlock()) {
			//cout << "I WANT TO BLOCK!" << endl;
			setBlockParams();
			zannSetTrackBallHeadParams();
			goalieBlockTimer = blockLength;
			return;
		}

		//try to intercept it.. ie go towards the ball but dont lock the attack mode
		if (zannGoalieIsInGoalieBox() && zannGoalieBallIsGoingToArriveAtYouSoon()) {
#ifdef ZANN_THE_DEFENDER_OLD_LEDS
			leds(1, 1, 1, 2, 2, 2);
#endif // ZANN_THE_DEFENDER_OLD_LEDS
			zannGoalieAttackBall();
			zannSetTrackBallHeadParams();
			return;
		}

		zannCalculateDefendPositionHeading(&zannDefendX,
				&zannDefendY,
				&zannDefendH);

		if (!zannHasReachedDefendPositionHeading(zannDefendX,
					zannDefendY,
					zannDefendH)) {
			zannGoToDefendPositionHeading(zannDefendX, zannDefendY, zannDefendH);
		}
		else {
			setStandParams();
		}

		// periodically look at beacons
#ifdef ZANN_THE_DEFENDER_OLD_LEDS
		leds(2, 2, 2, 1, 1, 1);
#endif // ZANN_THE_DEFENDER_OLD_LEDS
		zannSetTrackBallHeadParams();


		if (lookAtBeaconCounter >= 24) {
			if (vBall.d >= 20) {
				lookAtBeaconCounter = 0;


				lastHeadType = headtype;
				lastHeadTilt = tilty;
				lastHeadPan = panx;

				lookAtBeaconTimer = 0;
				lockMode = LookAtBeacon;            
				setNewBeacons(FIELD_LENGTH * 0.75);
				zannLookAtBeaconAndGoToDefendPosition();

				checkLastBallPosTimer = 6;
			}
		}

		lookAtBeaconCounter++;
	}                
	//#endif

}

////////////////////////////////
// Uses team c goalie functions
////////////////////////////////
void uther::zannLookAtBeaconAndGoToDefendPosition() {
#ifdef ZANN_THE_DEFENDER_OLD_LEDS
	leds(1, 2, 1, 2, 1, 2);
#endif // ZANN_THE_DEFENDER_OLD_LEDS
	activeGpsLocalise(false);
	if (lookAtBeaconTimer == 8) {
		lockMode = NoMode;
	}
	lookAtBeaconTimer++;

	setStandParams();
	/*
	   if (farFromDefendPosition() && !zannHasReachedDefendPositionHeading(defendX, defendY, defendH)) {
	   zannGoToDefendPositionHeading(defendX, defendY, defendH);
	   } else {
	   setStandParams();
	   }
	   */
}

void uther::zannGoalieFullCircleFindBall() {
	//findBallHeadParams();
	zannGoalieHeadFindBall();

	walkType = CanterWalkWT;
	forward = 0;
	left = 0;

	//if close to the goal wall
	if (gps->self().pos.y < 30) {
		//facing away from origin
		if (gps->self().h <= 45) {
			turnDir = ANTICLOCKWISE;
		} 
		//else facing towards the origin
		else if (gps->self().h >= 135) {
			turnDir = CLOCKWISE;
		}
		//might make this random later
		else {
			turnDir = CLOCKWISE;
		}
	} 
	//might make this random later
	else {
		turnDir = CLOCKWISE;
	}

	turnCCW = turnDir * 10;

	PG = 40;
	hF = 70;
	hB = 110;
	hdF = 5;
	hdB = 30;
	ffO = 55;
	fsO = 15;
	bfO = -55;
	bsO = 15;
}

void uther::zannGoalieSemiCircleFindBall() {
	//findBallHeadParams();
	zannGoalieHeadFindBall();

	walkType = CanterWalkWT;
	forward = 0;
	left = 0;

	if (vision->vob[gps->ownGoal].cf > 0) {
		if (turnDir == CLOCKWISE) {
			turnDir = ANTICLOCKWISE;
		}
		else {
			turnDir = CLOCKWISE;
		}
		turnCCW = turnDir * FASTER_TURN_AROUND_STEP;
	}
	else {
		turnCCW = turnDir * NORMAL_TURN_AROUND_STEP;
	}    

	PG = 40;
	hF = 70;
	hB = 110;
	hdF = 5;
	hdB = 30;
	ffO = 55;
	fsO = 15;
	bfO = -55;
	bsO = 15;
}

void uther::zannGoalieHeadFindBall() {
	static const double lowtilt = -50;
	static const double hightilt = 5;

	int lowfirst = 0;

	// num frames in 1/2 cycle of head finding ball
	static const int HEAD_PERIOD = 20; //ERIC CHANGE THIS BACK TO 30 *******************************************

	/*
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 ************************
	 */

	// if haven't started finding ball yet ...
	if (!findBallCounter) {
		// 0 = false, 1 = true
		lowfirst = (lastBallTilt > (hightilt + lowtilt / 2)) ? 0 : 1;

		lookdir = ((gps->getBall(LOCAL).head <= 0 && lowfirst == 1)
				|| (gps->getBall(LOCAL).head > 0 && lowfirst == 0))
			? CLOCKWISE
			: ANTICLOCKWISE;
	}

	//every head period alternate between low first and high first
	if ((findBallCounter / HEAD_PERIOD) % 2 == lowfirst) {
		// look down first
		panx = -90 + (180 / HEAD_PERIOD) * ((findBallCounter) % HEAD_PERIOD);
		tilty = lowtilt;
	}
	else {
		// look up first
		panx = 90 - (180 / HEAD_PERIOD) * ((findBallCounter) % HEAD_PERIOD);
		tilty = hightilt;
	}

	panx *= lookdir;
	findBallCounter++;
	headtype = ABS_H;

	/*
	// leg motion, trigered after an initial stationary scanning
	if (findBallCounter >= 4 * HEAD_PERIOD) {
	findBallCounter = 2 * HEAD_PERIOD;
	}
	*/
}

bool uther::zannGoalieAllowedToBlock() {
	/*
	//bool headingAtYou = false;
	//bool headingQuickly = false;

	//calculate if the ball is travelling in a direction that is sort of heading your way
	Vector ball = gps->getBall(LOCAL);
	Vector vel = gps->getVBall(LOCAL);

	ball.setAngleNorm(90);
	vel.setAngleNorm(90);
	ball.rotate(180);
	if (abs(ball.head-vel.head) <= blockAngle) {
	headingAtYou = true;
	} else {
	headingAtYou = false;
	}
	//same calculation but using dot product
	double dx = gps->getBall(LOCAL).x * (- gps->getVBall(LOCAL).x);
	double dy = gps->getBall(LOCAL).y * (- gps->getVBall(LOCAL).y);
	double length = sqrt((SQUARE(gps->getBall(LOCAL).x) + SQUARE(gps->getBall(LOCAL).y))
	 *(SQUARE(gps->getVBall(LOCAL).x)+SQUARE(gps->getVBall(LOCAL).y))     );
	 double dotProduct = (dx+dy)/length;

	 if (dotProduct >= cos(DEG2RAD(blockAngle))) {
	 headingAtYou = true;
	 } else {
	 headingAtYou = false;
	 }
	 */

	// block distance from centre of chest to tip of toe abt 12 cm, plus ball radius 3 cm
	// so we want to block anything that 'll eventually reach our position +/- 15 cm then
	// use that angle

	Vector ball = gps->getBall(LOCAL);
	Vector vel = gps->getVBall(LOCAL);

	// if heading away then ignore
	if (vel.y > 0) {
		return false;
	}

	double m = vel.y / vel.x;
	double b = ball.y - ball.x *m;
	double x = -b / m;
	//the range where you actually have a chance of stopping the ball by sticking out your arms
	//otherwise its better to not stick out your arms and walk to a better location
	static const int HORIZONTAL_BLOCKING_RANGE = 15;
	//the range where you going to block the ball anyway so theres no real need to stick out your arms
	//and waste walking time
	static const int HORIZONTAL_REDUNDANT_BLOCKING_RANGE = 4;

	if (x <-HORIZONTAL_BLOCKING_RANGE
			|| x> HORIZONTAL_BLOCKING_RANGE
			|| x <HORIZONTAL_REDUNDANT_BLOCKING_RANGE
			&& x> - HORIZONTAL_REDUNDANT_BLOCKING_RANGE) {
		return false;
	}

	// if contact within the next second and a bit
	static const int CONTACT_RANGE = 28;
	if (ball.d <= vel.d * CONTACT_RANGE) {
		return true;
	}
	else {
		return false;
	}
}

bool uther::zannGoalieBallWithinBlockDistance() {
	// dimensions of goalie box: 50cm  x 100cm     
	if (gps->getBall(GLOBAL).x
			>= (FIELD_WIDTH / 2.0)
			- (GOALBOX_WIDTH / 2.0 + blockDistance)
			&& gps->getBall(GLOBAL).x
			<= (FIELD_WIDTH / 2.0)
			+ (GOALBOX_WIDTH / 2.0 + blockDistance)
			&& gps->getBall(GLOBAL).y
			<= (WALL_THICKNESS + GOALBOX_DEPTH + blockDistance)) {
		return true;
	}
	if (zannGoalieIsInGoalieBox() && (vBall.d <= 60)) {
		return true;
	}
	return false;
}

static const double attackDistance = 15;

bool uther::zannGoalieBallWithinAttackDistance() {
	// dimensions of goalie box: 50cm  x 100cm     
	if (gps->getBall(GLOBAL).x
			>= (FIELD_WIDTH / 2.0)
			- (GOALBOX_WIDTH / 2.0 + attackDistance)
			&& gps->getBall(GLOBAL).x
			<= (FIELD_WIDTH / 2.0)
			+ (GOALBOX_WIDTH / 2.0 + attackDistance)
			&& gps->getBall(GLOBAL).y
			<= (WALL_THICKNESS + GOALBOX_DEPTH + attackDistance)) {
		return true;
	}

	if (zannGoalieIsInGoalieBox() && (vBall.d <= 45)) {
		return true;
	}
	return false;
}

bool uther::zannBallIsInCornerAreas() {
	static const double cornerXSize = 60;
	static const double cornerYSize = 30;

	if ((gps->getBall(GLOBAL).x <= cornerXSize)
			|| (gps->getBall(GLOBAL).x >= (FIELD_WIDTH - cornerXSize))
			&& (gps->getBall(GLOBAL).y <= cornerYSize)) {
		return true;
	}
	return false;
}

bool uther::zannOpponentIsCloserToBall() {
	//double ownDistance = sqrt(SQUARE(gps->ball().x) + SQUARE(gps->ball().y));
	double ownDistance = vBall.d;
	for (int i = 0; i < NUM_OPPONENTS; i++) {
		double oppDist = sqrt(SQUARE(gps->getOppPos(i, GLOBAL).x
					- gps->getBall(GLOBAL).x)
				+ SQUARE(gps->getOppPos(i, GLOBAL).y
					- gps->getBall(GLOBAL).y));
		if (oppDist <= ownDistance) {
			return true;
		}
	}
	return false;
}

bool uther::zannGoalieBallWithinDangerDistance() {
	static const double dangerDistance = 35;

	// dimensions of goalie box: 50cm  x 100cm     
	if (gps->getBall(GLOBAL).x
			>= (FIELD_WIDTH / 2.0)
			- (GOALBOX_WIDTH / 2.0 + dangerDistance)
			&& gps->getBall(GLOBAL).x
			<= (FIELD_WIDTH / 2.0)
			+ (GOALBOX_WIDTH / 2.0 + dangerDistance)
			&& gps->getBall(GLOBAL).y
			<= (WALL_THICKNESS + GOALBOX_DEPTH + dangerDistance)) {
		return true;
	}
	/*
	   if (zannGoalieIsInGoalieBox() && (vBall.d<=60)) {
	   return true;
	   }
	   */
	return false;
}    

bool uther::zannAllowedToAttackBall() {
	static const double robotAllowedToAttackDistance = 55;
	static const double ballAllowedToAttackDistance = 50;

	if ((gps->self().pos.x
				<= (FIELD_WIDTH / 2.0)
				- (GOALBOX_WIDTH / 2.0 + robotAllowedToAttackDistance)
				|| gps->self().pos.x
				>= (FIELD_WIDTH / 2.0)
				+ (GOALBOX_WIDTH / 2.0 + robotAllowedToAttackDistance)
				|| gps->self().pos.y
				>= (WALL_THICKNESS + GOALBOX_DEPTH + robotAllowedToAttackDistance))) {
		return false;
	}
	/*
	   if (gps->getBall(GLOBAL).x <= (FIELD_WIDTH / 2.0) - (GOALBOX_WIDTH/2.0 + ballAllowedToAttackDistance) &&
	   gps->getBall(GLOBAL).x >= (FIELD_WIDTH / 2.0) + (GOALBOX_WIDTH/2.0 + ballAllowedToAttackDistance) &&
	   gps->getBall(GLOBAL).y >= (WALL_THICKNESS + GOALBOX_DEPTH + ballAllowedToAttackDistance)) {
	   return false;
	   }
	   */
	//if the ball is not close to your goal face

	//this section has the effect that
	//if you are far away from your goal, the ball has to be close to attack it.
	//the further you are, the closer the ball needs to be
	//the function sort of looks like a sqrt function.. use arcFunction to specify
	//how "arc"ed the function is
	static const double maxAttackableBallDist = 55;
	static const double maxAllowedDogDist = 110;
	static const double arcFunction = 3; //the higher the number the more the function arcs
	double requiredBallDist;
	if (!(gps->getBall(GLOBAL).x
				>= (FIELD_WIDTH / 2.0)
				- (GOALBOX_WIDTH / 2.0 + attackDistance)
				&& gps->getBall(GLOBAL).x
				<= (FIELD_WIDTH / 2.0)
				+ (GOALBOX_WIDTH / 2.0 + attackDistance)
				&& gps->getBall(GLOBAL).y
				<= (WALL_THICKNESS + GOALBOX_DEPTH + attackDistance))
			&& (vBall.d >= 20)) {
		requiredBallDist = maxAttackableBallDist
			- ((maxAttackableBallDist * pow(zannGoalieDistanceFromHome(),
							arcFunction))
					/ pow(maxAllowedDogDist,
						arcFunction));
		if (vBall.d > requiredBallDist) {
			return false;
		}
	}

	/*
	   if (gps->getBall(GLOBAL).x <= (FIELD_WIDTH / 2.0) - (GOALBOX_WIDTH/2.0 + ballAllowedToAttackDistance) &&
	   gps->getBall(GLOBAL).x >= (FIELD_WIDTH / 2.0) + (GOALBOX_WIDTH/2.0 + ballAllowedToAttackDistance) &&
	   gps->getBall(GLOBAL).y >= (WALL_THICKNESS + GOALBOX_DEPTH + ballAllowedToAttackDistance)) {
	   return false;
	   }
	   */
	return attackBallMode; //ie whatever it used to be
}


double uther::zannGoalieDistanceFromHome() {
	double distx = DEFAULT_POS_X - gps->self().pos.x;
	double disty = DEFAULT_POS_Y - gps->self().pos.y;
	return sqrt(SQUARE(distx) + SQUARE(disty));
}


	bool uther::zannGoalieIsInGoalieBox() {
		if (gps->self().pos.x >= (FIELD_WIDTH / 2.0) - (GOALBOX_WIDTH / 2.0)
				&& gps->self().pos.x <= (FIELD_WIDTH / 2.0) + (GOALBOX_WIDTH / 2.0)
				&& gps->self().pos.y <= (WALL_THICKNESS + GOALBOX_DEPTH)) {
			return true;
		}
		return false;
	}

bool uther::zannGoalieBallIsGoingToArriveAtYouSoon() {
	Vector ball = gps->getBall(LOCAL);
	Vector vel = gps->getVBall(LOCAL);

	// if heading away then ignore
	if (vel.y > 0) {
		return false;
	}

	double m = vel.y / vel.x;
	double b = ball.y - ball.x *m;
	double x = -b / m;

	static const int HORIZONTAL_DANGER_RANGE = 30;

	if (x <-HORIZONTAL_DANGER_RANGE || x> HORIZONTAL_DANGER_RANGE) {
		return false;
	}

	static const int CONTACT_RANGE = 75;
	if (ball.d <= vel.d * CONTACT_RANGE) {
		return true;
	}
	else {
		return false;
	}
}    

void uther::zannCalculateDefendPositionHeading(double *destx,
		double *desty,
		double *desth) {
	static const double pivotx = (double) FIELD_WIDTH / (double) 2;
	static const double pivoty = 5;
	static const double pivotr = 35; //radius of the circle

	double ballx = gps->getBall(GLOBAL).x;
	double bally = gps->getBall(GLOBAL).y;

	//find intercept between line through ball and pivot point
	//and circle around pivot point
	//
	//logic is a weighted average
	//(destx,desty) = (pivotx,pivoty) + (pivotr/d) * [ (ballx,bally) - (pivotx,pivoty) ]

	double d = sqrt(SQUARE(ballx - pivotx) + SQUARE(bally - pivoty));

	//right now dx and dy are a vector from pivot to ball
	double dx = ballx - pivotx;
	double dy = bally - pivoty;
	double dh = RAD2DEG(atan2(dy, dx));

	double ratio = pivotr / d;

	//dx and dy are now the vector scaled by the ratio
	dx = ratio * dx;
	dy = ratio * dy;

	//dx and dy are now the intersection of the line and circle
	dx = pivotx + dx;
	dy = pivoty + dy;

	*destx = dx;
	*desty = dy;
	*desth = dh;
}

bool uther::zannHasReachedDefaultPosition() {
	double diffx = DEFAULT_POS_X - gps->self().pos.x;
	double diffy = DEFAULT_POS_Y - gps->self().pos.y;

	if (ABS(diffx) <= POS_ERROR_MARGIN && ABS(diffy) <= POS_ERROR_MARGIN) {
		return true;
	}
	return false;
}

void uther::zannGoToDefaultPositionHeading() {
	saGoToTargetFacingHeading(DEFAULT_POS_X, DEFAULT_POS_Y, DEFAULT_POS_H);
}

void uther::zannGoToDefendPositionHeading(double defendX,
		double defendY,
		double defendH) {
	double diffx = defendX - gps->self().pos.x;
	double diffy = defendY - gps->self().pos.y;
	double dist = sqrt(diffx *diffx + diffy *diffy);
	if (dist < 20) {
		saGoToTargetFacingHeading(defendX, defendY, defendH, 4);
	}
	else {
		saGoToTargetFacingHeading(defendX, defendY, defendH);
	}
}

//static bool farFromDefendPosFlag = false;

bool uther::zannHasReachedDefendPositionHeading(double defendX,
		double defendY,
		double defendH) {
	double diffx = defendX - gps->self().pos.x;
	double diffy = defendY - gps->self().pos.y;
	double diffh = defendH - gps->self().h;

	if (ABS(diffx) <= POS_ERROR_MARGIN
			&& ABS(diffy) <= POS_ERROR_MARGIN
			&& ABS(diffh) <= H_ERROR_MARGIN) {
		wireless = IND_LED2_OFF;
		//farFromDefendPosFlag = false;
		return true;
	}
	wireless = IND_LED2_ON;
	return false;
}

bool uther::zannHasReachedDefendPositionHeadingWhenLookingForBall(double defendX,
		double defendY,
		double defendH) {
	double diffx = defendX - gps->self().pos.x;
	double diffy = defendY - gps->self().pos.y;
	double diffh = defendH - gps->self().h;

	if (ABS(diffx) <= POS_ERROR_MARGIN_LARGER
			&& ABS(diffy) <= POS_ERROR_MARGIN_LARGER
			&& ABS(diffh) <= H_ERROR_MARGIN) {
		wireless = IND_LED2_OFF;
		return true;
	}
	wireless = IND_LED2_ON;
	return false;
}

	bool uther::zannBallJumpedTooMuch() {
		double distance = sqrt(SQUARE(oldBallx - gps->getBall(LOCAL).x)
				+ SQUARE(oldBally - gps->getBall(LOCAL).y));
		if (distance >= 60) {
			return true;
		} 
		return false;
	}

void uther::zannSetGpsTrackBallHeadParams() {
	headtype = ABS_XYZ;

	panx = -gps->getBall(LOCAL).x;
	tilty = BALL_DIAMETER; //BALL_DIAMETER/2.0;
	cranez = gps->getBall(LOCAL).y;
}

static double old_tilt = 0;
static double old_pan = 0;
#define ZFOV 10

void uther::zannSetTrackBallHeadParams() {
	if (vision->vob[vobBall].cf > 0) {
		tilty = vision->vob[vobBall].elev;
		panx = vision->vob[vobBall].head;

		if (abs(panx - hPan) > ZFOV) {
			double factor = 1.0 + ABS(panx - hPan) / 60;
			panx = hPan + (panx - hPan) * factor;
		}

		double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, z6;



		x1 = vBall.d * tan(radians(vBall.imgHead));
		//track top
		y1 = vBall.d * tan(radians(PointToElevation(vision->vob[0].misc)));
		//track middle
		//y1 = vBall.d * tan(radians(vBall.imgElev));
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

		double turn = radians(turnCCW / 8);
		x5 = x4 * cos(turn) - z4 * sin(turn);
		y5 = y4;
		z5 = x4 * sin(turn) + z4 * cos(turn);

		if (z5 < NECK_LENGTH)
			z5 = NECK_LENGTH;

		headtype = ABS_H;
		double dist = sqrt(z5 *z5 + y5 *y5);
		tilty = degrees(atan(y5 / z5) - asin(NECK_LENGTH / dist));

		old_tilt = tilty;
		old_pan = panx;
	}
	else {
		tilty = old_tilt;
		panx = old_pan;
	}

	/*
	   double faceLength = 7.0;
	   double neckLength = 5.5;
	   double x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,x5,z5;
	   x1 = vision->vob[vobBall].d * tan(radians(vision->vob[vobBall].imgHead));
	   y1 = vision->vob[vobBall].d * tan(radians(vision->vob[vobBall].imgElev));
	   z1 = vision->vob[vobBall].d;
	   x2 = x1;
	   y2 = y1 + neckLength;
	   z2 = z1 + faceLength;
	   x3 = x2*cos(radians(-hPan))-z2*sin(radians(-hPan));
	   y3 = y2;
	   z3 = x2*sin(radians(-hPan))+z2*cos(radians(-hPan));
	   x4 = x3;
	   y4 = z3*sin(radians(hTilt))+y3*cos(radians(hTilt));
	   z4 = z3*cos(radians(hTilt))-y3*sin(radians(hTilt));
	   if (z4 < neckLength) z4 = neckLength;
	   headtype = ABS_H;
	   double dist = sqrt(z4*z4 + y4*y4);
	   tilty = degrees(atan(y4/z4)-asin(neckLength/dist));
	   x5 = x4;
	//z5 = z4*cos(radians(-tilt))-y4*sin(radians(-tilt));
	z5 = dist;
	panx = degrees(atan(x5/z5));
	// need this bit?
	// tilt = hTilt+(tilt-hTilt)*0.6;
	// pan  = hPan+(pan-hPan)*0.8;

*/
}

void uther::zannGoalieAttackGpsBall() {
	// if cleared the ball, set attackBallMode to false
	if (tilty < -60.0 && ABS(panx) < 20.0 && vBall.vob->cam_dist < 10) {
		lockMode = GoalieHoldBall;
		zannaaGoalieHoldBall();
	}

	zannGoalieHoverToBall(false);
}
void uther::zannGoalieAttackBall() {
	zannSetTrackBallHeadParams();

	// if cleared the ball, set attackBallMode to false
	if (tilty < -60.0 && ABS(panx) < 20.0 && vBall.vob->cam_dist < 10) {
		lockMode = GoalieHoldBall;
		zannaaGoalieHoldBall();
	}

	zannGoalieHoverToBall(true);
}

/*void uther::zannGoalieGoToBall() {
// need a lot of turn to get align with ball
if (ABS(vBall.h) > 30) {
forward = Cap(vBall.d, 2.0) * cos(DEG2RAD(vBall.h));
left    = Cap(vBall.d, 2.0) * sin(DEG2RAD(vBall.h));
turnCCW = Cap(vBall.h, 30.0);
} else {
if (vBall.d > 30.0) {
forward = Cap(vBall.d, 8.0) * cos(DEG2RAD(vBall.h));
left    = Cap(vBall.d, 6.0) * sin(DEG2RAD(vBall.h));
} else {
forward = Cap(vBall.d, 6.0) * cos(DEG2RAD(vBall.h));
left    = Cap(vBall.d, 5.0) * sin(DEG2RAD(vBall.h));
}

double move = sqrt(SQUARE(forward) + SQUARE(left));
double maxTurn = 9 - move;

turnCCW = Cap(vBall.h/2.0, maxTurn);
}
}*/

void uther::zannGoalieHoverToBall(bool usingVision) {
	double ballDist;
	double ballHead;
	if (usingVision) {
		ballDist = vBall.d;
		ballHead = vBall.h;
	}
	else {
		ballDist = gps->getBall(LOCAL).d;
		ballHead = gps->getBall(LOCAL).head;
	}
	bool onLeft = utilIsUp()
		&& utilBallOnLEdge()
		&& utilIsOnLEdge()
		&& gps->self().posVar < get95CF(75);
	bool onRight = utilIsUp()
		&& utilBallOnREdge()
		&& utilIsOnREdge()
		&& gps->self().posVar < get95CF(75);

	double maxCanterForwardSpeed = 8;
	double maxCanterLeftSpeed = 6;

	double relH = ballHead;
	if ((onLeft || onRight)
			&& (NormalizeAngle_0_360(gps->self().h) < HALF_CIRCLE)) {
		relH = QUARTER_CIRCLE - gps->self().h;
		if (ABS(relH - ballHead) > 10)
			relH = ballHead;
	}

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

	if (ballDist < 20) {
		// The following values were taken directly
		// from DotsAndSpot's saGoToBall; they seem
		// to work very well upon closely approaching
		// the ball.
		walkType = CanterWalkWT;
		forward = 7.5;
		left = 0;
		turnCCW = CLIP(vBall.h / 3.0, 40.0);
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
}
static const int holdLengthBeforeCarrying = 10;
void uther::zannaaGoalieHoldBall() {
	zannSetTrackBallHeadParams();

	if (goalieHoldBallTimer < holdLengthBeforeCarrying) {
		setHoldParams();
		goalieHoldBallTimer++;
		return;
	}


	attackBallMode = false;
	lockMode = GotBall;
	gps->currentlyCarrying = true;
	setCarryParams();
	tilty = 5;
	panx = 0;
	zannGotBall();
	//aaGotBall();
	return;
}

static double gapMin; //is originally local based, where the gap is set by find best gap function
static double gapMax; //but later becomes global based

//at the start it chooses a gap
//it then turns to that gap and shoots
void uther::zannGotBall() {
	bool shoot = false;
	static const int anglesOnSidesWhereNotAllowedToShoot = 10; //if this is 10 you can only shoot between global 10 and 170
	double minAllowedTurn = -gps->self().h
		+ anglesOnSidesWhereNotAllowedToShoot; //convert that range to local coordinates
	double maxAllowedTurn = 180
		- gps->self().h
		- anglesOnSidesWhereNotAllowedToShoot;


	// init gap
	if (goalieHoldBallTimer <= holdLengthBeforeCarrying) {
		zannFindBestGap(minAllowedTurn, maxAllowedTurn, &gapMin, &gapMax);
		//cout << "********************************" << endl;
		//cout << "min and max gap local " << gapMin << "," << gapMax << endl;
		//****************************
		// ALARMS!!! Fix This Properly
		//****************************
		//if the gap is really small or there is no gap (negative case) do a chest push
		if ((gapMax - gapMin) <= 5) {
			gps->currentlyCarrying = false;
			lockMode = SpinChestPush;
			goalieHoldBallTimer = 0;
			aaSpinChestPush(); 
			return;
		}
		//should do that only if everyone is really close
		//if not everyone is closing in on you
		//use vision to find a gap
		//
		//or you can rerun this find gap function
		//until you do find a gap

		//pick a direction to turn
		if (gapMin >= 0 && gapMax >= 0) {
			clockwise = false;
		} else if (gapMax <= 0 && gapMin <= 0) {
			clockwise = true;
		}
		else {
			//else your in the gap
			//cout << "im already in the gap " << endl;
			if (abs(gapMax) > abs(gapMin)) {
				//go towards the middle
				clockwise = false;
			}
			else {
				clockwise = true;
			}
		}

		//convert definition of the gap into global coordinates
		gapMin += gps->self().h;
		gapMax += gps->self().h;
		//cout << "min and max gap global " << gapMin << "," << gapMax << endl;
		//cout << "my heading is " << gps->self().h << " so i should turn ";
		//if (clockwise) {
		//    cout << "clockwise" << endl;
		//} else {
		//    cout << "anit-clockwise" << endl;
		//}
	}

	//if (clockwise) {
	//    cout << "clockwise      ";
	//} else {
	//    cout << "anit-clockwise ";
	//}    
	//cout << "current heading " << gps->self().h << endl;

	// overturn case 
	if ((gps->self().h <anglesOnSidesWhereNotAllowedToShoot
				|| gps->self().h> 270)
			&& clockwise) {
		clockwise = false;
	}
	// overturn case 
	else if (gps->self().h > (180 - anglesOnSidesWhereNotAllowedToShoot)
			&& gps->self().h < 270
			&& !clockwise) {
		clockwise = true;
	} 

	static const int beatGapBy = 5;
	//shoot in the gaps
	if (!clockwise && (gps->self().h > (gapMin + beatGapBy))) {
		shoot = true;
	}
	else if (clockwise && (gps->self().h < (gapMax - beatGapBy))) {
		shoot = true;
	}

	gps->currentlyCarrying = true;
	setCarryParams();
	tilty = 5;
	panx = 0;

	goalieHoldBallTimer++;

	//***************
	//    ALARM
	//***************
	//you need to uncomment out these time outs when playing a proper game
	//
	//if held too long outside of goalie box shoot
	//if (!zannGoalieIsInGoalieBox() && goalieHoldBallTimer >= 65) {    
	//    shoot = true;
	//} 
	//if held too long inside goalie box shoot
	//else if (goalieHoldBallTimer >= 85) {    
	//    shoot = true;
	//}

	if (shoot) {
		gps->currentlyCarrying = false;
		lockMode = GoalieKick;
		goalieHoldBallTimer = 0;
		aaGoalieKick();
		return;
	}
	else {
		if (clockwise) {
			turnCCW = -30.0;
			left = 5.0;
		}
		else {
			turnCCW = 30.0;
			left = -5.0;
		}
	}
}

//this is the old version
//just keeps on turning until it is not blocked
/*
   void uther::zannGotBall() {

   bool shoot = false;
// init turn direction
if (goalieHoldBallTimer <= holdLengthBeforeCarrying) {
if (gps->self().h < 90 || gps->self().h > 270)
clockwise = false;
else
clockwise = true;
}
gps->currentlyCarrying = true;
setCarryParams();
tilty = 5;
panx = 0;

goalieHoldBallTimer++;

static const int angleThreshold = 25;

//if held too long outside of goalie box shoot
//if (!zannGoalieIsInGoalieBox() && goalieHoldBallTimer >= 65) {    
//    shoot = true;
//} 
//if held too long inside goalie box shoot
//else if (goalieHoldBallTimer >= 85) {    
//    shoot = true;
//}
// overturn case 
if (gps->self().h < angleThreshold && clockwise) {
clockwise = false;
//shoot = true;
}
// overturn case 
else if (gps->self().h > (180-angleThreshold) && gps->self().h < 270 && !clockwise) {
clockwise = true;
//shoot = true;
} 
//deliberately do a ramp shot if conditions are right
//else if (zannGoalieAbleToDoARampShot()) { 
//    shoot = true;
//}
//shoot in opponent gaps
else if ((gps->self().h>=angleThreshold) && (gps->self().h<=(180-angleThreshold))) {

bool blocked = false;
//static const int OBSTACLE_RANGE = 25;
static const int CLEARANCE_DISTANCE = 25;

double clearance[NUM_OPPONENTS];
for (int i=0; i<NUM_OPPONENTS; i++) {
double clearanceRange = RAD2DEG(atan2(CLEARANCE_DISTANCE,gps->getOppPos(i).d));
clearance[i] = clearanceRange;
if (abs(gps->getOppPos(i).head) < clearanceRange) {
blocked = true;
}
}
shoot = !blocked;
#ifdef ericsShootingComments
if (shoot) {
cout << "SHOOTING!!!!!!!!!!!" << endl;
} else {
cout << "******** NOT SHOOTING !!!!!!!!!" << endl;
}
cout << "my heading " << gps->self().h << endl;
for (int i=0; i<NUM_OPPONENTS; i++) {
cout << "   opp " << i << " head " << gps->getOppPos(i).head << "\t dist " << gps->getOppPos(i).d <<
"\t clear by " << clearance[i] << endl;
}
cout << endl;
#endif  
}

if (shoot) {
	gps->currentlyCarrying = false;
	lockMode = GoalieKick;
	goalieHoldBallTimer = 0;
	aaGoalieKick();
} else {
	if (clockwise) {
		turnCCW = -30.0;
		left = 5.0;
	} else {
		turnCCW = 30.0;
		left = -5.0;
	}
}

#ifdef rickyStuff
// overturn case
if (gps->self().h < 15 && clockwise) {
	shoot = true;
}
// overturn case 
else if (gps->self().h > 165 && gps->self().h < 270 && !clockwise) {
	shoot = true;
} 
// if hold too long
else if (goalieHoldBallTimer >= 90) {    
	shoot = true;
} 
// check whether we have clear path to launch
else { 
	bool blocked = false;
	const int OBSTACLE_RANGE = 20;
	int start, end;
	// we are blue
	if (gps->targetGoal == vobYellowGoal) { 
		start = vobRedDog; end = vobRedDog4;
	} else {
		start = vobBlueDog; end = vobBlueDog4;
	}
	//go through your opponent dogs
	for (int i = start; i <= end; i++) {
		if (vision->vob[i].cf > 0 && vision->vob[i].head > -OBSTACLE_RANGE && vision->vob[i].head < OBSTACLE_RANGE) {
			blocked = true;
			break;
		}
	}
	shoot = !blocked;
}
#endif
}
*/

/*bool uther::zannGoalieAbleToDoARampShot() {
//facing the ramp
//no dogs between you and the ramp
//dogs a reasonable distance out of the way of exit path of ramp
return false;
}*/

void uther::zannFindBestGap(double minAllowedAngle,
		double maxAllowedAngle,
		double *returnMinAngle,
		double *returnMaxAngle) {
	static const int CLEARANCE_DISTANCE = 25; //the distance to either side of the dogs position that is considered "blocked"
	static const int CLEARANCE_ANGLE = 20; //the angle where, all gaps over this size are considered equally good

	//get all the dogs you want to avoid
	int countValidDogs = 0;
	sortee dogs[NUM_OPPONENTS];
	for (int i = 0; i < NUM_OPPONENTS; i++) {
		if (gps->getOppPos(i).d < 300) {
			dogs[countValidDogs].id = i;
			dogs[countValidDogs].weight = gps->getOppPos(i).head;
			countValidDogs++;
		}
	}
	quickSort(dogs, countValidDogs); //sort dogs based on their headings

	//calculate all the clearance angles
	double clearanceAngles[countValidDogs];
	for (int i = 0; i < countValidDogs; i++) {
		clearanceAngles[i] = RAD2DEG(atan2(CLEARANCE_DISTANCE,
					gps->getOppPos(int(dogs[i].id)).d));
	}

	//calculate all the gaps
	sortee gaps[countValidDogs + 1];

	gaps[0].id = minAllowedAngle; 
	gaps[0].weight = (dogs[0].weight - clearanceAngles[0]) - minAllowedAngle;

	double tempAngle = (dogs[countValidDogs - 1].weight
			+ clearanceAngles[countValidDogs - 1]);
	gaps[countValidDogs].id = tempAngle;
	gaps[countValidDogs].weight = maxAllowedAngle - tempAngle;

	for (int i = 1; i < countValidDogs; i++) {
		double angle1 = (dogs[i].weight - clearanceAngles[i]); //larger one
		double angle2 = (dogs[i - 1].weight + clearanceAngles[i - 1]); //smaller one
		gaps[i].id = angle2; 
		gaps[i].weight = angle1 - angle2; //this could be negative which means its blocked and not a gap
	}
	quickSort(gaps, (countValidDogs + 1)); //sort gaps so that we can extract the largest

	//find closest gap thats larger than the required threshold
	double closestAngle;
	double closestSoFar = 180;
	int bestIndex = -1;
	for (int i = 0; i < (countValidDogs + 1); i++) {
		if (gaps[i].weight > CLEARANCE_ANGLE) {
			double gapCenter = gaps[i].id + (gaps[i].weight / 2.0);
			//if im already in the gap, return this gap
			if (gaps[i].id <= 0 && (gaps[i].id + gaps[i].weight) >= 0) {
				bestIndex = i;
				break;
			} 

			if (gapCenter < 0) {
				closestAngle = gaps[i].id + gaps[i].weight;
			}
			else {
				closestAngle = gaps[i].id;
			}

			if (abs(closestAngle) < closestSoFar) {
				closestSoFar = abs(closestAngle);
				bestIndex = i;
			}
		}
	}

	//if none of the gaps cleared the threshhold, return the largest gap.
	//this could be negative
	if (bestIndex < 0) {
		bestIndex = countValidDogs;
	}
	*returnMinAngle = gaps[bestIndex].id;
	*returnMaxAngle = gaps[bestIndex].id + gaps[bestIndex].weight;
}


/////////////////////////////////////////////////
// START ZANN THE DEFENDER FUNCTIONS
/////////////////////////////////////////////////

	bool uther::zannBallIsInBox() {
		if (gps->getBall(GLOBAL).x >= (FIELD_WIDTH / 2.0) - (GOALBOX_WIDTH / 2.0)
				&& gps->getBall(GLOBAL).x <= (FIELD_WIDTH / 2.0) + (GOALBOX_WIDTH / 2.0)
				&& gps->getBall(GLOBAL).y <= (WALL_THICKNESS + GOALBOX_DEPTH)) {
			return true;
		}
		return false;
	}

void uther::zannUseWirelessBall() {
	// precondition: 
	// ballSource == WIRELESS_BALL, 
	// ballX, ballY & ballH all set correctly

	//zannTrackCalcBall();
	zannGoalieHeadFindBall();
	zannCalculateDefendPositionHeadingWireless(&zannDefendX,
			&zannDefendY,
			&zannDefendH);

	if (!zannHasReachedDefendPositionHeading(zannDefendX,
				zannDefendY,
				zannDefendH)) {
		zannGoToDefendPositionHeading(zannDefendX, zannDefendY, zannDefendH);
	}
	else {
		setStandParams();
	}
}

void uther::zannCalculateDefendPositionHeadingWireless(double *destx,
		double *desty,
		double *desth) {
	static const double pivotx = (double) FIELD_WIDTH / (double) 2;
	static const double pivoty = 5;
	static const double pivotr = 35; //radius of the circle

	// wireless ball
	double ballx = ballX;
	double bally = ballY;

	//find intercept between line through ball and pivot point
	//and circle around pivot point
	//
	//logic is a weighted average
	//(destx,desty) = (pivotx,pivoty) + (pivotr/d) * [ (ballx,bally) - (pivotx,pivoty) ]
	//

	double d = sqrt(SQUARE(ballx - pivotx) + SQUARE(bally - pivoty));

	//right now dx and dy are a vector from pivot to ball
	double dx = ballx - pivotx;
	double dy = bally - pivoty;
	double dh = RAD2DEG(atan2(dy, dx));

	double ratio = pivotr / d;

	//dx and dy are now the vector scaled by the ratio
	dx = ratio * dx;
	dy = ratio * dy;

	//dx and dy are now the intersection of the line and circle
	dx = pivotx + dx;
	dy = pivoty + dy;

	*destx = dx;
	*desty = dy;
	*desth = dh;
}

void uther::zannTrackCalcBall() {
	Vector v;
	v.setAngleNorm(HALF_CIRCLE);
	v.setVector(vCART, ballX - gps->self().pos.x, ballY - gps->self().pos.y);
	v.rotate(90 - (gps->self()).h);

	if (v.y > 0) {
		headtype = ABS_XYZ;
		panx = -v.x;
		tilty = 2 * BALL_RADIUS; //v.y;
		cranez = v.y; //BALL_DIAMETER/2;
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
