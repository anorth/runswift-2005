/*

   Copyright 2004 The University of New South Wales (UNSW) and National  
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

//the goalie doesn't attack the ball if the teammate is gonna get it
//but it may accidently roll under its chin!!
//this needs to be fixed!!

/*
 * Last modification background information
 * $Id: beckhamGoalie.cc 1016 2003-04-20 12:31:15Z echung $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 * 
 **/

#include "beckham.h"
#include "../share/BallDef.h"

static const bool ledSilence = true;
static const bool rossLedSilence1 = false;
static const bool rossLedSilence2 = true;

static const bool goalieRoleEyes = false;
#define CHECK_LAST_EYES leds(2,1,1,2,1,1) // (GPS)
#define WIRELESS_EYES   leds(1,1,2,1,1,2)
#define VISION_EYES     leds(1,2,1,1,2,1)
#define NO_IDEA_EYS     leds(2,1,2,1,1,1)

static const double DEFAULT_POS_X = FIELD_WIDTH / 2;
static const double DEFAULT_POS_Y = WALL_THICKNESS + 10;
static const double DEFAULT_POS_H = 90;

static const double SMART_POSL_X = FIELD_WIDTH / 2 - 15;
static const double SMART_POSL_Y = WALL_THICKNESS + 10;
static const double SMART_POSL_H = 110;

static const double SMART_POSR_X = FIELD_WIDTH / 2 + 15;
static const double SMART_POSR_Y = WALL_THICKNESS + 10;
static const double SMART_POSR_H = 70;

static const int NORMAL_TURN_AROUND_STEP = 10;
static const int FASTER_TURN_AROUND_STEP = 30;

static const int POS_ERROR_MARGIN = 10;
static const int H_ERROR_MARGIN = 8;
static const int POS_ERROR_MARGIN_LARGER = 20;
static const int H_ERROR_MARGIN_LARGER = 15;

static const int BAR_LED_INDEX = NUM_OF_INDICATORS - 3;

static bool goalieBlockStarted = false;
static int goalieBlockTimer = 0;

static int goalieCanBlockCounter = 0;
static const int goalieBCLimit = 1; 

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

// for goalieGotBall
static bool clockwise = true;

static bool lostBallWhenReallyClose = false;

// for just after the goalie loses the ball
static int smartPositionCounter = 0;

// the direction we've decided on for turning out of goal (to stop to-and-fro-ing)
static double getOutOfGoalDirection = 0;
static int getOutOfGoalCounter = 0;
static int framesWithoutBeacons = 0;
static const int framesWithoutBeaconsThreshold = 50;
static const int getOutOfGoalCounterThreshold = 60;

namespace Beckham {
	double allowedAttackDist;
	double allowedAttackDem;

	double blockDistance;
	double blockSpeed;
	double blockAngle;
	int blockLength;
}

void Beckham::initBeckhamGoalie() {
	lostBall = 0;
	seeBall = 0;
	lastSawBeacon = 0;

        // Ted: removed goalieResetFindBallCounter()

        // Ted: Why? The goalie still need to receive information from teammates?
	isWirelessDown = true;
	lockMode = NoMode;

        // Fixed dkd for goalie.
	dkd = 90;
	ballUnderChinTilt = -55;
	attackMode = GoToBall;
	prevAttackMode = GoToBall;
	grabTime = 0;
	grabInterval = 0;
	turnDir = -1;

	//goalieLookAroundCounter = 150;
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


void Beckham::doBeckhamGoalie() {
	setDefaultParams();
	
	// Core functions are setBeckhamGoalieValues() and chooseBeckhamGoalieStrategy().
	setBeckhamGoalieValues();
	setWalkParams();
	chooseBeckhamGoalieStrategy();

	if (lockMode == NoMode) {
		clipBeckhamFireballPan();
	}
}

void Beckham::setBeckhamGoalieValues() {
	setBeckhamGoalie();
	lastSawBeacon++;
	for (int i = vobBlueLeftBeacon; i <= vobYellowRightBeacon; i++) {
		if (vision->vob[i].cf != 0) {
			lastSawBeacon = 0;
			break;
		}
	}

	DECREMENT(grabInterval);
	DECREMENT(grabTime);
	//DECREMENT (goalieLookAroundTimer);

	//decrementDoNotLocalise();
	decrementBackOff();
	//decrementAttackLock();

	setTeammateSeenTime();

	if (lockMode != prevLockMode) {
		prevLockMode = lockMode;
	}

	// Reset smartPositionCounter if we've seen a ball
	if (vBall.cf >= 3) {
		smartPositionCounter = 0;
	}
	//Reset getOutOfGoalDirection if we've seen any beacons
	for (int i = vobBlueLeftBeacon; i < vobYellowRightBeacon; i ++) {
		if (vision->vob[i].cf > 0) {
			getOutOfGoalDirection = 0;
			getOutOfGoalCounter = 0;
		}
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
}

static double defendX, defendY, defendH;

void Beckham::chooseBeckhamGoalieStrategy() {

        // Ted: Didn't we already do this in setBeckhamValues() - the lastSawBeacon variable ?
	// If so, we should remove it.
	framesWithoutBeacons ++;
	//Reset framesWithoutBeacons if we've seen any beacons
	for (int i = vobBlueLeftBeacon; i < vobYellowRightBeacon; i ++) {
		if (vision->vob[i].cf > 0) {
			framesWithoutBeacons = 0;
		}
	}

	if (lockMode != NoMode) {
		// execute atomic action if in locked mode
		switch (lockMode) {
			case GotBall: //if you can see the ball somewhere else abort
				if (vBall.cf >= 500 && vBall.d > 10) {
					//if (goalieHoldBallTimer>=25 && tilty >= 0 && panx == 0) {
					setStandParams();
					setTrackBallHeadParams(); 
					lockMode = NoMode;
					goalieHoldBallTimer = 0;
					break;
					//}
				}
				goalieGotBall();
				break;
		        case ChestPush: lockMode = NoMode;
			                //aaChestPush();
					break;
			case SpinKick: aaSpinKick();
				       break;
			case SpinChestPush: aaSpinChestPush();
					    break;
		        case GoalieKick: lockMode = NoMode;
			                 //aaGoalieKick();
					 break;
		        case LightningKick: lockMode = NoMode;
			                    //aaLightningKick();
					    break;
			case LookAtBeacon: lookAtBeaconAndGoToDefendPosition();
					   break;                
			case GoalieHoldBall: aaGoalieHoldBall();
					     break;
		}
	}
	else {
		leds(1, 1, 1, 1, 1, 1);

		// hack to get the goally back in right position if it was heading into goal.
		// not as bad a hack as it was before anymore, because of infra red distance
		// when turning, set head tilt to something high, so that nothing can be seen
		// when face in goal, this'll make the robot spin because infra red will return
		// soemthing
		// when no longer facing goal, infra red will give 90, the highest value it can
		// return, and so we know we are outside goal
		// START HACK ****************************************************************************
		// START HACK HACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACK
		int ownGoal, targetGoal;
		const int SIZE_THRESHOLD = 5;

		if (gps->targetGoal == vobBlueGoal) {
			ownGoal = vobYellowGoal;
			targetGoal = vobBlueGoal;
		}
		else {
			ownGoal = vobBlueGoal;
			targetGoal = vobYellowGoal;
		}
		
		// cout << "vision->vob[ownGoal].cf = " << vision->vob[ownGoal].cf << " width=" << vision->vob[ownGoal].width << " height=" << vision->vob[ownGoal].height << " vision->facingYellowGoal=" << vision->facingYellowGoal << " framesWithoutBeacons=" << framesWithoutBeacons << " getOutOfGoalCounter=" << getOutOfGoalCounter << endl;
		//If we can see our own goal real big or we've got our face in either goal or we're locked into a get-out-of-goal routine
		if ((vision->vob[ownGoal].cf > 0 && (vision->vob[ownGoal].width > WIDTH - SIZE_THRESHOLD
		     || (vision->vob[ownGoal].height > HEIGHT - SIZE_THRESHOLD && vision->vob[ownGoal].width > WIDTH / 2)))
		     || getOutOfGoalCounter > 0) {

			attackBallMode = false;
			setWalkParams();
			walkType = CanterWalkWT;
			if (getOutOfGoalDirection == 0) {
				forward = 0;
				left = 0;
				if (vision->vob[ownGoal].head >= 0) {
					turnCCW = -40;
				}
				else {
					turnCCW = 40;
				}
				getOutOfGoalDirection = turnCCW;
			}
			else {
				turnCCW = getOutOfGoalDirection;
			}
			headtype = ABS_H;
			tilty = 40;
			panx = 0;
			getOutOfGoalCounter ++;

			static const double MAX_INFRA_RED = 90.0;
			if (getOutOfGoalCounter > getOutOfGoalCounterThreshold
			    || framesWithoutBeacons == 0
			    || vision->vob[targetGoal].cf > 0
			    || sensors->sensorVal[ssINFRARED_FAR] / 10000.0 >= MAX_INFRA_RED) {
				// If we've been getting out of the goal for long enough, stop doing so. Note that we don't set the direction, if we
				// get back into here, we want to keep turning in the same direction so we don't keep to-ing and fro-ing.
				// also escape the turn, if it sees beacons, or if it sees the opposite goal
				// at which point it falls through to the rest of the decision tree
				getOutOfGoalCounter = 0;
			}
			else {
				return;
			}
		}
		// END HACK ****************************************************************************
		// END HACK HACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACK

		// if ball is under chin
		if (isBeckhamBallUnderChin()) {
			goalieCanBlockCounter = 0;
			lockMode = GoalieHoldBall;
			aaGoalieHoldBall();
			return;
		}

		// right after it has looked at beacon, we want
		// to look at where it last saw the ball
		//
		// Or if we are giving the gps location of the ball a chance
		if (checkLastBallPosTimer > 0) {
			//if looking at gps ball
			if (usingGpsLastPos) {
				//if (gps->ball('l').posVar <= get95CF(65)) {
				setGpsTrackBallHeadParams(); 
				//try to block against phantom balls
				//yes this is very much a hack... 
				if (ballJumpedTooMuch()) {
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
				calculateSearchDefendPositionHeading(&defendX, &defendY, &defendH);
			} 
			//looking back after just actively localising
			else {
				headtype = lastHeadType;
				tilty = lastHeadTilt;
				panx = lastHeadPan;
				calculateDefendPositionHeading(&defendX, &defendY, &defendH);
			}

			checkLastBallPosTimer--;

			if (usingGpsLastPos && attackBallMode) {
				goalieAttackGpsBall();
			}
			else if (!hasReachedDefendPositionHeadingWhenLookingForBall(defendX, defendY, defendH)) {
				if (!ledSilence)
					leds(1, 2, 1, 1, 2, 1);
				goToDefendPositionHeading(defendX, defendY, defendH);
			}
			else {
				setStandParams();
			}        

			//if you can now see the ball break out of the 
			//checkLastBallPosTimer lock
			if (vBall.cf >= 3) {
				checkLastBallPosTimer = 0;
				//do nothing which will hence go through and do the rest of the code and set something else to do
			}
			else {
				if (goalieRoleEyes)
					CHECK_LAST_EYES;
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
			setTrackBallHeadParams();
			goalieBlockTimer--;

			//if this is the end of the block timer, decide if you want to renew it
			if (goalieBlockTimer == 0) {
				//check that you wouldn't rather attack
				if (!goalieBallWithinAttackDistance()) {
					//check block condition					
					if (goalieBallWithinBlockDistance() && goalieAllowedToBlock(false)) {
						//cout << "RENEWING THE BLOCK TO BLOCK FOR LONGER" << endl;
						goalieBlockTimer = blockLength;
					}
				}
			}        
			return;
		}

		// if it has lost ball
		if (vBall.cf < 3) {
			goalieCanBlockCounter = 0;

			//give gps ball a chance before doing a find ball
			if (haventGivenGpsAChance) {
				calculateSearchDefendPositionHeading(&defendX, &defendY, &defendH);
				if (attackBallMode) {
					goalieAttackGpsBall();
				}
				else if (!hasReachedDefendPositionHeadingWhenLookingForBall(defendX, defendY, defendH)) {
					goToDefendPositionHeading(defendX, defendY, defendH);
				}
				else {
					setStandParams();
				}
				setGpsTrackBallHeadParams(); //this needs to be called before the lastHead* lines
				usingGpsLastPos = true;
				lastHeadType = headtype;
				lastHeadTilt = tilty;
				lastHeadPan = panx;
				if (panx <= 20 && panx >= -20 && tilty <= -35) {
					lostBallWhenReallyClose = true;
					checkLastBallPosTimer = 14; //HMmm making this larger may have no effect because if you see above, theres a variance based override
				}
				else {
					checkLastBallPosTimer = 12;
				}
				haventGivenGpsAChance = false;                
				oldBallx = gps->getBall(LOCAL).x;
				oldBally = gps->getBall(LOCAL).y;

				if (goalieRoleEyes)
					CHECK_LAST_EYES;

				return;
				/*
				   if (!hasReachedDefendPositionHeadingWhenLookingForBall(defendX, defendY, defendH)) {
				   goToDefendPositionHeading(defendX, defendY, defendH);
				   } else {
				   setStandParams();
				   }
				   */
			}

			// orginally factored in ... maintained for consistency
			attackBallMode = allowedToAttackBall();

			/*if (attackBallMode) {
			//findBallHeadParams();
			//goalieHeadFindBall();
			headFindBall();
			if (!hasReachedDefendPositionHeadingWhenLookingForBall(defendX, defendY, defendH)) {
			goToDefendPositionHeading(defendX, defendY, defendH);
			} else {
			setStandParams();
			}
			return;
			}*/
			
			if (gps->shareBallvar < get95CF(100)) {
				// search for ball and position defensively
				headFindBall();
				calculateSearchDefendPositionHeading(&defendX,
						&defendY,
						&defendH,
						WIRELESS_BALL);
				if (!hasReachedDefendPositionHeadingWhenLookingForBall(defendX,
							defendY,
							defendH)) {
					goToDefendPositionHeading(defendX, defendY, defendH);
				}
				else {
					setStandParams();
				}

				if (goalieRoleEyes)
					WIRELESS_EYES;

				return;
			}
			else {
				//findBallHeadParams();
				//goalieHeadFindBall();
				headFindBall();
				if (goalieRoleEyes)
					NO_IDEA_EYS;
				/*static const int smartCounterLimit = 40;
				  if (smartPositionCounter < smartCounterLimit) {
				  if (!ledSilence) leds(2,1,1,2,1,1);
				  if (!hasReachedSmartPosition()) {
				  goToSmartPositionHeading();
				  } else {
				  setStandParams();
				  }
				  smartPositionCounter ++;    //this gets reset once we see a ball. 
				  return;
				  } else*/ if (!hasReachedDefaultPosition()) {
					  //We've been sitting at the 'smart' position for smartCounterLimit frames and haven't seen 
					  //the ball, go to default position. 
					  goToDefaultPositionHeading();
					  return;
				  }
				  else {
					  setStandParams();
					  //goalieHeadFindBall();
					  headFindBall();
					  return;
				  }
			}
		}
		//else you have seen the ball
		else {
			findBallCounter = 0;
			haventGivenGpsAChance = true; //reset the allowed to look at gps ball
		}

		if (goalieRoleEyes)
			VISION_EYES;

		if (goalieBallWithinBlockDistance()
				&& vBall.d > 30
				&& goalieAllowedToBlock(true)) {
			//cout << "I WANT TO BLOCK!" << endl;
			setBlockParams();
			setTrackBallHeadParams();
			goalieBlockTimer = blockLength;
			return;
		}

		// if you get up to here, the ball is not lost
		attackBallMode = allowedToAttackBall();
		if (attackBallMode) {
			goalieAttackBall();
			return;
		}

		if (goalieBallWithinAttackDistance()) {
			//if teammates gonna get it then let him get it and dont go out of your box
			if (teammateHasBall()) {
				//cout << "not going to START attacking because teammate has ball " << endl;
				//do nothing ie, skip this if
			}
			else {
				goalieAttackBall();
				attackBallMode = true;
				return;
			}
		}

		/*if (goalieBallWithinBlockDistance() && vBall.d>30 && goalieAllowedToBlock(true)) {
		//cout << "I WANT TO BLOCK!" << endl;
		setBlockParams();
		setTrackBallHeadParams();
		goalieBlockTimer = blockLength;
		return;
		}*/

		//try to intercept it.. ie go towards the ball but dont lock the attack mode
		if (goalieIsInGoalieBox() && goalieBallIsGoingToArriveAtYouSoon()) {
			goalieAttackBall();
			setTrackBallHeadParams();
			return;
		}

		calculateDefendPositionHeading(&defendX, &defendY, &defendH);

		if (!hasReachedDefendPositionHeading(defendX, defendY, defendH)) {
			goToDefendPositionHeading(defendX, defendY, defendH);
		}
		else {
			setStandParams();
		}

		setTrackBallHeadParams();

		// periodically look at beacons
		const Vector &vel = gps->getVBall(LOCAL);
		double confidence = gps->getVBIVLength() / vel.d;
		if (vBall.cf > 3 && confidence < 2.0 && (vel.head> 100 || vel.head < -100)) {
			if (lookAtBeaconCounter)
				lookAtBeaconCounter--;
		}
		else
			lookAtBeaconCounter++;

		if (lookAtBeaconCounter >= 24) {
			if (vBall.d >= 20) {
				lookAtBeaconCounter = 0;
				lastHeadType = headtype;
				lastHeadTilt = tilty;
				lastHeadPan = panx;
				lookAtBeaconTimer = 0;
				lockMode = LookAtBeacon;            
				//setNewBeacons(FIELD_LENGTH*0.75);
				smartSetBeacon(FIELD_LENGTH * 0.75);
				lookAtBeaconAndGoToDefendPosition();
				checkLastBallPosTimer = 6;
			}
		}
	}                
	//#endif
}


/////////////////////////////////////////////////
// START GOALIE FUNCTIONS
/////////////////////////////////////////////////

/////////////////////////////////////////////////
// GOALIE BLOCKING FUNCTIONS
/////////////////////////////////////////////////

//TO DO
//might want to add stuff like which way your facing
//use heading and distance to vision ball rather than gps

//MIGHT ONLY WANT TO BLOCK IF YOUR ACTUALLY FACING THE BALL
//IE IF THE BALL IS HEADING TOWARDS YOU BUT 90 TOWARDS YOU DONT BLOCK CAUSE YOU MIGHT HIT IT IN

//right now sometimes
//goalie doesn't need to turn kick (unless he is out of the box?) he has enough time to grab the ball

bool Beckham::goalieAllowedToBlock(bool initial) {
	if (!rossLedSilence1) {
		leds(2, 0, 0, 0, 0, 0);
	}
	// block distance from centre of chest to tip of toe abt 12 cm, plus ball radius 3 cm
	// so we want to block anything that 'll eventually reach our position +/- 15 cm then
	// use that angle

	// Ross change
	//Vector ball = gps->ball('l').pos;
	//Vector vel = gps->vBall().pos;
	const Vector &ball = gps->getBall(LOCAL);
	const Vector &vel = gps->getVBall(LOCAL);

	MMatrix2 vcovLocal = gps->getVCLocal();
	MMatrix2 vcovBall = gps->getVCBall();

	// if heading away (or very small) then ignore
	if (vel.y > -0.2) {
		goalieCanBlockCounter = 0;
		return false;
	}

	// if not sure about velocity then ignore
	double confidence = gps->getVBIVLength() / vel.d;
	if (confidence > 2.0 || (confidence > 1.2 && goalieCanBlockCounter == 0)) {
		goalieCanBlockCounter = 0;
		return false;
	}

	// number of frames till ball gets to robot
	double numFrames = -ball.y / vel.y;
	// x value when ball gets to you
	double xval = ball.x + numFrames *vel.x;
	// standard deviation on that x value
	double xSD = numFrames *sqrt(ABS(vcovLocal(0, 0)));

	// take into account one half standard deviation
	// double m = vel.y / vel.x;
	// double b = ball.y - ball.x * m;
	// double x = -b/m;
	// the range where you actually have a chance of stopping the ball by sticking out your arms
	// otherwise its better to not stick out your arms and walk to a better location
	static const int HORIZONTAL_BLOCKING_RANGE = 15;
	//the range where you going to block the ball anyway so theres no real need to stick out your arms
	//and waste walking time
	static const int HORIZONTAL_REDUNDANT_BLOCKING_RANGE = 4;

	if ((xval + xSD / 8.0) < -HORIZONTAL_BLOCKING_RANGE
			|| (xval - xSD / 8.0) > HORIZONTAL_BLOCKING_RANGE) {
		if (!rossLedSilence1) {
			leds(0, 0, 0, 2, 0, 0);
		}
		goalieCanBlockCounter = 0;
		return false;
	}
	if ((xval + xSD / 12.0) < HORIZONTAL_REDUNDANT_BLOCKING_RANGE
			&& (xval - xSD / 12.0) > -HORIZONTAL_REDUNDANT_BLOCKING_RANGE) {
		if (!rossLedSilence1) {
			leds(0, 0, 2, 0, 0, 2);
		}
		goalieCanBlockCounter = 0;
		return false;
	}

	// 1/2 standard deviation towards robot
	double towardsSD = sqrt(ABS(vcovBall(1, 1)));
	// if contact within the next second and a bit, take into account one standard deviation
	static const int CONTACT_RANGE = 20;
	
	if (ball.d <= (vel.d + towardsSD / 6.0) * CONTACT_RANGE) {
		goalieCanBlockCounter++;
		if (!initial || goalieCanBlockCounter > goalieBCLimit) {
			if (!rossLedSilence1) {
				if (xval > -HORIZONTAL_BLOCKING_RANGE / 2.0)
					leds(0, 0, 0, 0, 2, 0);
				if (xval < HORIZONTAL_BLOCKING_RANGE / 2.0)
					leds(0, 2, 0, 0, 0, 0);
			}
			return true;
		}
		else
			return false;
	}
	else {
		if (!rossLedSilence1) {
			leds(0, 2, 2, 0, 2, 2);
		}
		goalieCanBlockCounter = 0;
		return false;
	}
}

bool Beckham::goalieBallWithinBlockDistance() {
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
	if (goalieIsInGoalieBox() && (vBall.d <= 80)) {
		return true;
	}
	return false;
}

bool Beckham::goalieBallIsGoingToArriveAtYouSoon() {
	const Vector &ball = gps->getBall(LOCAL);
	const Vector &vel = gps->getVBall(LOCAL);

	// if heading away then ignore
	if (vel.y >= 0) {
		return false;
	}

	if (abs(vel.x) > 1E-5) //stop divide by zero
	{
		double m, b, x;
		m = vel.y / vel.x;
		b = ball.y - ball.x * m;
		x = -b / m;
		static const int HORIZONTAL_DANGER_RANGE = 30;
		if (x < -HORIZONTAL_DANGER_RANGE || x > HORIZONTAL_DANGER_RANGE)
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


bool Beckham::goalieBallWithinDangerDistance() {
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
	   if (goalieIsInGoalieBox() && (vBall.d <= 60)) {
	   return true;
	   }
	   */
	return false;
}    


/////////////////////////////////////////////////
// GOALIE SHOULD ATTACK FUNCTIONS
/////////////////////////////////////////////////


static const double attackDistance = 10;
bool Beckham::goalieBallWithinAttackDistance() {
	/*
	   cout << "**************************" << endl;
	   double xcomp = gps->getBall(GLOBAL).x;
	   if (xcomp <= FIELD_WIDTH/2.0) {
	   cout << "ball from corner x comp " << xcomp - WALL_THICKNESS << endl;
	   } else {
	   cout << "ball from corner x comp " << FIELD_WIDTH - xcomp - WALL_THICKNESS << endl;
	   }
	   cout << "ball from corner y comp " << FIELD_LENGTH - WALL_THICKNESS - gps->getBall(GLOBAL).y << endl;
	   cout << "goalie in box is " << goalieIsInGoalieBox() << endl;
	   */

	//stay in position and let them bring the ball to you
	//hopefully drawing a foul
	//rather than you coming out and leaving goal unattended
	if (goalieIsInGoalieBox()
			&& ballIsInCornerAreas()
			&& opponentIsCloserToBall()) {
		//leds(2,2,2,2,2,2);
		return false;
	}

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

	if (goalieIsInGoalieBox() && (vBall.d <= 35) && !opponentIsCloserToBall()) {
		return true;
	}
	return false;
}

bool Beckham::ballIsInCornerAreas() {
	static const double cornerXSize = WALL_THICKNESS + 80; //these params are VERY BIG Dangerous?
	static const double cornerYSize = WALL_THICKNESS + 50; //these params are VERY BIG Dangerous?

	if ((gps->getBall(GLOBAL).x <= cornerXSize)
			|| (gps->getBall(GLOBAL).x >= (FIELD_WIDTH - cornerXSize))
			&& (gps->getBall(GLOBAL).y <= cornerYSize)) {
		return true;
	}
	return false;
}

bool Beckham::opponentIsCloserToBall() {
	double ownDistance = vBall.d;
	for (int i = 0; i < NUM_OPPONENTS; i++) {
		if (gps->getOppCovMax(i) < get95CF(50)) {
			double oppDist = sqrt(SQUARE(gps->getOppPos(i, GLOBAL).x
						- gps->getBall(GLOBAL).x)
					+ SQUARE(gps->getOppPos(i, GLOBAL).y
						- gps->getBall(GLOBAL).y));
			if (oppDist <= ownDistance + 40) {
				return true;
			}
		}
	}
	return false;
}

bool Beckham::allowedToAttackBall() {
	static const double robotAllowedToAttackDistance = 45;
	static const double ballAllowedToAttackDistance = 50;

	if (teammateHasBall()) {
		//cout << "STOP attacking ball now because the teammate has the ball" << endl;
		return false;
	}

	// if I'm twice as close as any opponent
	// and closer than any teammate
	/*
	   bool isClosest = true;
	   for (int i = 0; i < 4; i++) {
	   const Vector& opp  = gps->getOppPos(i, GLOBAL);
	   if ((sqrt(SQUARE(opp.x - vBall.x) + SQUARE(opp.y - vBall.y)) < vBall.d * 2.0 && gps->getOppCovMax(i) < get95CF(50))
	   || 
	   ((i != PLAYER_NUMBER-1) && sqrt(SQUARE(gps->teammate(i).pos.x - vBall.x) + SQUARE(gps->teammate(i).pos.y - vBall.y)) < vBall.d)) {
	   isClosest = false;
	   break;
	   }
	   }
	   if (isClosest) return true;
	   */

	//hard limit.. if you've gone too far out go back
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

	//special case to handle scrums along bottom edge
	//try to get back into the box to draw the foul
	int oppNum = 0;
	const Vector &opp = gps->getClosestOppPos(LOCAL, &oppNum);
	if (gps->self().pos.y <= (WALL_THICKNESS + 50)
			&& !goalieIsInGoalieBox()
			&& gps->getOppCovMax(oppNum) < get95CF(50)
			&& opp.d < 40
			&& abs(opp.head) < 45) {
		//cout << "STOP attacking ball now because your in a corner scrum" << endl;
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
        //
	//this section has the effect that
	//if you are far away from your goal, the ball has to be close to attack it.
	//the further you are, the closer the ball needs to be
	//the function sort of looks like a sqrt function.. use arcFunction to specify
	//how "arc"ed the function is
	static const double maxAttackableBallDist = 55;
	static const double maxAllowedDogDist = 100;
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
			- ((maxAttackableBallDist * pow(goalieDistanceFromHome(),
							arcFunction))
					/ pow(maxAllowedDogDist,
						arcFunction));
		if (vBall.d > requiredBallDist)
			return false;
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

bool Beckham::teammateHasBall() {
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);

		interpBehav info = (interpBehav) tm.behavioursVal; 

		if ((i != (PLAYER_NUMBER - 1))
				&& (info.ballDist < (unsigned int) 30)
				&& (tm.counter > 0)
				&& isFacingAwayFromOwnGoal(tm.pos.x,
					tm.pos.y,
					tm.h)) {
			return true;
			//cout << "player " << i+i << " is " << tm.ballDist << " cms away from the ball" << endl;
			//cout << "he is at " << tm.pos.x << " , " << tm.pos.y << " facing " << tm.h << endl;
		}
	}
	return false;
}

bool Beckham::isFacingAwayFromOwnGoal(double posx, double posy, double posh) {
	double awayFromGoal = RAD2DEG(atan2(posy - 0, posx - (FIELD_WIDTH / 2.0)));

	//calculate if the ball is travelling in a direction that is sort of heading your way
	Vector toTeammate(vPOLAR, 1, awayFromGoal);
	Vector teammatesHeading(vPOLAR, 1, posh);

	toTeammate.setAngleNorm(270);
	teammatesHeading.setAngleNorm(270);

	if (abs(toTeammate.theta - teammatesHeading.theta) <= 40) {
		return true;
	}
	else {
		return false;
	}
}

/////////////////////////////////////////////////
// GOALIE MISC FUNCTIONS
/////////////////////////////////////////////////

	double Beckham::goalieDistanceFromHome() {
		double distx = DEFAULT_POS_X - gps->self().pos.x;
		double disty = DEFAULT_POS_Y - gps->self().pos.y;
		return sqrt(SQUARE(distx) + SQUARE(disty));
	}


	bool Beckham::goalieIsInGoalieBox() {
		if (gps->self().pos.x >= (FIELD_WIDTH / 2.0) - (GOALBOX_WIDTH / 2.0)
				&& gps->self().pos.x <= (FIELD_WIDTH / 2.0) + (GOALBOX_WIDTH / 2.0)
				&& gps->self().pos.y <= (WALL_THICKNESS + GOALBOX_DEPTH)) {
			return true;
		}
		return false;
	}


	bool Beckham::ballJumpedTooMuch() {
		double distance = sqrt(SQUARE(oldBallx - gps->getBall(LOCAL).x)
				+ SQUARE(oldBally - gps->getBall(LOCAL).y));
		if (distance >= 60) {
			return true;
		} 
		return false;
	}

	void Beckham::lookAtBeaconAndGoToDefendPosition() {
		smartActiveGpsLocalise(); 
		if (lookAtBeaconTimer == 8) {
			lockMode = NoMode;
		}
		lookAtBeaconTimer++;
		setStandParams();
		calculateDefendPositionHeading(&defendX, &defendY, &defendH);
		if (/*farFromDefendPosition() &&*/
		    !hasReachedDefendPositionHeadingWhenLookingForBall(defendX, defendY, defendH)) {
		if (!ledSilence)
			leds(1, 1, 2, 1, 1, 2);
		goToDefendPositionHeading(defendX, defendY, defendH);
	}
	else {
		setStandParams();
	}
}


/////////////////////////////////////////////////
// GOALIE POSITIONING FUNCTIONS
/////////////////////////////////////////////////


void Beckham::calculateDefendPositionHeading(double *destx, double *desty, double *desth) {
	static const double pivotx = (double) FIELD_WIDTH / (double) 2;
	static const double pivoty = 10;
	static const double pivotr = 35; //radius of the circle

	double ballx = gps->getBall(GLOBAL).x;
	double bally = gps->getBall(GLOBAL).y;

	// find intercept between line through ball and pivot point
	// and circle around pivot point
	//
	// now been changed to weight toward the edges

	// perform translation to make pivot point (0,0)
	double relx = ballx - pivotx;
	double rely = bally - pivoty;

	// cout << "(" << relx << "," << rely << ") ";

	// determine angle compared to straight up the field (should be [-90,90] as y>=0)
	// note - this is opposite to normal heading (i couldn't be bothered changing it)
	double angle = NormalizeAngle_180(90.0 - RAD2DEG(atan2(rely, relx)));
	//cout << "orig=" << angle << " ";
	static const double weightLimit = 30.0;
	if (abs(angle) > weightLimit) {
		double sign = (angle < 0) ? -1 : 1;
		double val = abs(angle) - weightLimit;
		// redistribute to weight towards the ends
		double weightConjugate = 90.0 - weightLimit;
		double newval = weightConjugate *sqrt(val / weightConjugate);
		angle = sign * (newval + weightLimit);
	}
	
	// cout << "after=" << angle << " ";
	// cout << "(" << - pivotr*sin(DEG2RAD(angle)) << "," << pivotr*cos(DEG2RAD(angle)) << ") " << endl;

	*destx = pivotx + pivotr * sin(DEG2RAD(angle));
	*desty = pivoty + pivotr * cos(DEG2RAD(angle));
	double newLocx = ballx - *destx;
	double newLocy = bally - *desty;
	*desth = RAD2DEG(atan2(newLocy, newLocx));
}

void Beckham::calculateSearchDefendPositionHeading(double *defendX, double *defendY, double *defendH,
		int source) {
	static const double pivotx = (double) FIELD_WIDTH / (double) 2;
	static const double pivoty = 10;
	static const double pivotMajorAxis = 35;
	static const double pivotMinorAxis = 5;

	double ballx;
	double bally;
	// expect the source of the ball to be wireless or gps
	// if it is vision the above function should be called
	if (source == WIRELESS_BALL) {
		ballx = gps->shareBallx;
		bally = gps->shareBally;
	}
	else {
		ballx = gps->getBall(GLOBAL).x;
		bally = gps->getBall(GLOBAL).y;
	}

	// find intercept between line through ball and pivot point and ellipse
	// centred at pivot point and of the pivot axis

	// perform translation to make pivot point (0,0)
	double relx = ballx - pivotx;
	double rely = bally - pivoty;

	// perform scaling to convert ellipse into unit circle
	double scalex = relx / pivotMajorAxis;
	double scaley = rely / pivotMinorAxis;

	// find intersection of line (0,0)->(x,y) and unit circle
	double scaleLength = sqrt(SQUARE(scalex) + SQUARE(scaley));
	double intx = scalex / scaleLength;
	double inty = scaley / scaleLength;

	// here we assume bally >= pivoty
	if (inty < 0) {
		cout << "Error in calculateSearchDefendPositionHeading: ball in goal?"
			<< endl;
		inty = 0;
	}
	double inth = RAD2DEG(atan2(inty, intx));
	double tanAngle = (intx > 0) ? 90 - inth : inth - 90;

	// reverse scaling
	double unscalex = intx *pivotMajorAxis;
	double unscaley = inty *pivotMinorAxis;

	double opposite = tan(DEG2RAD(tanAngle));
	opposite *= pivotMinorAxis / pivotMajorAxis;
	double unscaleTanAngle = RAD2DEG(atan(opposite));
	*defendH = (intx > 0) ? 90 - unscaleTanAngle : 90 + unscaleTanAngle;

	// reverse translation
	*defendX = unscalex + pivotx;
	*defendY = unscaley + pivoty;
}

bool Beckham::hasReachedDefaultPosition() {
	double diffx = DEFAULT_POS_X - gps->self().pos.x;
	double diffy = DEFAULT_POS_Y - gps->self().pos.y;

	if (ABS(diffx) <= POS_ERROR_MARGIN && ABS(diffy) <= POS_ERROR_MARGIN) {
		return true;
	}
	return false;
}

void Beckham::goToDefaultPositionHeading() {
	saGoToTargetFacingHeading(DEFAULT_POS_X, DEFAULT_POS_Y, DEFAULT_POS_H);
}


/**
 * Note, this 'smart position' isn't actually all that clever. We rely on the fact that 
 * when the goalie loses the ball, chances are it was on the same side of the goal as the ball. 
 * Therefore, go to the point on the circle closest to where the ball was last seen (we assume that 
 * GPS ball doesn't move if we've not seen it). 
 */
bool Beckham::hasReachedSmartPosition() {
	//  return hasReachedDefaultPosition(); //uncomment this to easily return to previous behaviour. 
	double destX, destY, destH;
	calculateDefendPositionHeading(&destX, &destY, &destH);
	double diffx = destX - gps->self().pos.x;
	double diffy = destY - gps->self().pos.y;

	if (ABS(diffx) <= POS_ERROR_MARGIN && ABS(diffy) <= POS_ERROR_MARGIN) {
		return true;
	}
	return false;
}

void Beckham::goToSmartPositionHeading() {
	//  goToDefaultPositionHeading(); return; //uncomment this to easily return to previous behaviour. 
	double destX, destY, destH;
	calculateDefendPositionHeading(&destX, &destY, &destH);
	saGoToTargetFacingHeading(destX, destY, destH);
}

void Beckham::findSmartPosition(int *posX, int *posY, int *posH) {
	//yes this is a null function for now ...
}

void Beckham::goToDefendPositionHeading(double defendX,
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

bool Beckham::hasReachedDefendPositionHeading(double defendX,
		double defendY,
		double defendH) {
	double diffx = defendX - gps->self().pos.x;
	double diffy = defendY - gps->self().pos.y;
	double diffh = defendH - gps->self().h;

	if (ABS(diffx) <= POS_ERROR_MARGIN
			&& ABS(diffy) <= POS_ERROR_MARGIN
			&& ABS(diffh) <= H_ERROR_MARGIN) {
		//farFromDefendPosFlag = false;
		return true;
	}
	return false;
}

bool Beckham::hasReachedDefendPositionHeadingWhenLookingForBall(double defendX,
		double defendY,
		double defendH) {
	double diffx = defendX - gps->self().pos.x;
	double diffy = defendY - gps->self().pos.y;
	double diffh = defendH - gps->self().h;

	if (ABS(diffx) <= POS_ERROR_MARGIN &&  //POS_ERROR_MARGIN_LARGER && 
			ABS(diffy) <= POS_ERROR_MARGIN &&  //POS_ERROR_MARGIN_LARGER &&
			ABS(diffh) <= H_ERROR_MARGIN) {
		return true;
	}
	return false;
}

/////////////////////////////////////////////////
// GOALIE ATTACK BALL FUNCTIONS
/////////////////////////////////////////////////


void Beckham::goalieAttackGpsBall() {
	/*
	// if cleared the ball, set attackBallMode to false
	if (tilty < -60.0 && ABS(panx) < 20.0 && vBall.vob->cam_dist < 10) {
	lockMode = GoalieHoldBall;
	aaGoalieHoldBall();
	}
	*/

	goalieHoverToBall(false, false);
}
void Beckham::goalieAttackBall() {
	setTrackBallHeadParams();

	/*
	// if cleared the ball, set attackBallMode to false
	if (tilty < -60.0 && ABS(panx) < 20.0 && vBall.vob->cam_dist < 10) {
	lockMode = GoalieHoldBall;
	aaGoalieHoldBall();
	}
	*/

	goalieHoverToBall(true, true);
	if (!rossLedSilence2) {
		leds(0, 2, 0, 0, 0, 0);
	}
}

static const int holdLengthBeforeCarrying = 10;

void Beckham::aaGoalieHoldBall() {
	setTrackBallHeadParams();

	if (goalieHoldBallTimer < holdLengthBeforeCarrying) {
		setHoldParams();
		goalieHoldBallTimer++;
		return;
	}

	attackBallMode = false;
	lockMode = GotBall;
	setCarryParams();
	tilty = 5;
	panx = 0;
	goalieGotBall();
	//aaGotBall();
	return;
}

static double gapMin; //is originally local based, where the gap is set by find best gap function
static double gapMax; //but later becomes global based

//at the start it chooses a gap
//it then turns to that gap and shoots
void Beckham::goalieGotBall() {
	bool shoot = false;
	static const int anglesOnSidesWhereNotAllowedToShoot = 10; //if this is 10 you can only shoot between global 10 and 170
	double minAllowedTurn = -gps->self().h
		+ anglesOnSidesWhereNotAllowedToShoot; //convert that range to local coordinates
	double maxAllowedTurn = 180
		- gps->self().h
		- anglesOnSidesWhereNotAllowedToShoot;

	// init gap
	if (goalieHoldBallTimer <= holdLengthBeforeCarrying) {
		findBestGap(minAllowedTurn, maxAllowedTurn, &gapMin, &gapMax);
		//cout << "********************************" << endl;
		//cout << "min and max gap local " << gapMin << "," << gapMax << endl;
		//****************************
		// ALARMS!!! Fix This Properly
		//****************************
		//if the gap is really small or there is no gap (negative case) do a chest push
		if ((gapMax - gapMin) <= 5) {
			cout << "gap was too small!!!!" << endl;
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
	}

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

	setCarryParams();
	tilty = 5;
	panx = 0;

	goalieHoldBallTimer++;

	/*
	//if held too long outside of goalie box shoot
	if (!goalieIsInGoalieBox() && goalieHoldBallTimer >= 65) {    
	shoot = true;
	} 
	//if held too long inside goalie box shoot
	else if (goalieHoldBallTimer >= 85) {    
	shoot = true;
	}
	*/

	if (shoot) {
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

bool Beckham::goalieAbleToDoARampShot() {
	//facing the ramp
	//no dogs between you and the ramp
	//dogs a reasonable distance out of the way of exit path of ramp
	return false;
}

void Beckham::findBestGap(double minAllowedAngle,
		double maxAllowedAngle,
		double *returnMinAngle,
		double *returnMaxAngle) {
	static const int CLEARANCE_DISTANCE = 25; //the distance to either side of the dogs position that is considered "blocked"
	static const int CLEARANCE_ANGLE = 20; //the angle where, all gaps over this size are considered equally good

	//get all the dogs you want to avoid
	int countValidDogs = 0;
	//int countValidOpponents = 0;
	sortee dogs[NUM_OPPONENTS/* + NUM_TEAM_MEMBER*/];
	for (int i = 0; i < NUM_OPPONENTS; i++) {
		if (gps->getOppCovMax(i) < get95CF(50) && gps->getOppPos(i).d < 300) {
			dogs[countValidDogs].id = i;
			dogs[countValidDogs].weight = gps->getOppPos(i).head;
			//dogs[countValidDogs].opponent = true;
			countValidDogs++;
			//countValidOpponents++;
		}
	}

	/*
	   for (int i = NUM_OPPONENTS; i < NUM_TEAM_MEMBER + NUM_OPPONENTS; i++) {
	   WMObj tm = gps->teammate(i+1-NUM_OPPONENTS);
	   if (tm.posVar < get95CF(50) && tm.pos.d < 300) {
	   dogs[countValidDogs].id = i;
	   dogs[countValidDogs].weight = tm.pos.head;
	   dogs[countValidDogs].opponent = false;
	   countValidDogs++;
	   }
	   }    
	   */

	//if theres no obstacles just return some values that makes it shoot downfield
	if (countValidDogs == 0) {
		//return local coordinates of a gap that makes it shoot downfield
		*returnMinAngle = 45 - gps->self().h;
		*returnMaxAngle = 135 - gps->self().h;
		return;
	}    

	quickSort(dogs, countValidDogs); //sort dogs based on their headings

	//calculate all the clearance angles
	double clearanceAngles[countValidDogs];
	for (int i = 0; i < countValidDogs; i++) {
		/*
		   if (dogs[i].opponent) {
		   clearanceAngles[i] = RAD2DEG(atan2(CLEARANCE_DISTANCE,
		   gps->getOppPos(int(dogs[i].id)).d));
		   } else {
		   clearanceAngles[i] = RAD2DEG(atan2(CLEARANCE_DISTANCE,
		   gps->teammate(int(dogs[i].id)+1).pos.d));
		   }
		   */
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
// GOALIE UTILITY FUNCTIONS
/////////////////////////////////////////////////

void Beckham::setGpsTrackBallHeadParams() {
	headtype = ABS_XYZ;

	panx = -gps->getBall(LOCAL).x;
	tilty = BALL_DIAMETER; //BALL_DIAMETER/2.0;
	cranez = gps->getBall(LOCAL).y;
}

static double old_tilt = 0;
static double old_pan = 0;
#define FOV 10

void Beckham::setTrackBallHeadParams() {
	if (vision->vob[vobBall].cf > 0) {
		tilty = vision->vob[vobBall].elev;
		panx = vision->vob[vobBall].head;

		if (abs(panx - hPan) > FOV) {
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
}


void Beckham::goalieFullCircleFindBall() {
	//findBallHeadParams();
	//goalieHeadFindBall();
	headFindBall();

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
	hdB = 25;
	ffO = 55;
	fsO = 15;
	bfO = -55;
	bsO = 15;
}

void Beckham::goalieSemiCircleFindBall() {
	//findBallHeadParams();
	//goalieHeadFindBall();
	headFindBall();

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
	hdB = 25;
	ffO = 55;
	fsO = 15;
	bfO = -55;
	bsO = 15;
}

void Beckham::goalieHeadFindBall() {
	static const double lowtilt = -50;
	static const double hightilt = 5;

	int lowfirst = 0;

	// num frames in 1/2 cycle of head finding ball
	static const int HEAD_PERIOD = 18; 

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
	if (lookdir == CLOCKWISE) {
		panx *= 1;
	}
	else {
		panx *= -1;
	}
	findBallCounter++;
	headtype = ABS_H;
	cout << "counter.. " << findBallCounter << " panx:" << panx << " tilty:"
		<< tilty << endl;

	/*
	// leg motion, trigered after an initial stationary scanning
	if (findBallCounter >= 4 * HEAD_PERIOD) {
	findBallCounter = 2 * HEAD_PERIOD;
	}
	*/
}


void Beckham::goalieHoverToBall(bool usingVision, bool usePrediction) {
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

	const Vector &ballVel = gps->getVBall(LOCAL);

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

	if (usePrediction && (ballDist > 15.0) && ballVel.d > 0.5) {
		// don't use prediction if ball is close or slow

		if (!rossLedSilence2)
			leds(0, 0, 0, 0, 2, 0);

		setOmniParams();
		walkType = CanterWalkWT;

		double maxF = maxCanterForwardSpeed;
		double maxL = maxCanterLeftSpeed;

		//calculate the max forward and left you can do whilst turning
		//counter clockwise case turning slow
		if (turnCCW < 10 && turnCCW >= 0) {
			maxF = 5.5;
			maxL = 5.5;
		} 
		//counter clockwise case turning faster
		else if (turnCCW < 20 && turnCCW >= 0) {
			maxF = 4.5;
			maxL = 4.5;
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

		double ballX = ballDist *cos(DEG2RAD(90 + ballHead));
		double ballY = ballDist *sin(DEG2RAD(90 + ballHead));

		// keep maximum speed 6.0 so hopefully we can maintain PG 30
		double maxSpeed = 6.0;
		// speed at cm/frame - guess
		double maxSpeedCMPFrame = 0.75; //6.0/6.0;  
		// want to try to use smaller PG value if possible
		//double maxlinSpeed = 6.9 - abs(turnCCW/6.0);
		// if small PG slows us down too much then forget it
		//if (maxlinSpeed < 3.0)
		//    maxlinSpeed = 20.0;

		/*double mindiff = LARGE_VAL;
		  double minHS = -1;
		  int halfSecs = 0;
		  for(; halfSecs < 6 ; halfSecs++) {
		  int numFrames = (int)(halfSecs*12.5);
		  double x = ballX + numFrames*ballVel.x;
		  double y = ballY + numFrames*ballVel.y;

		// difference between how far ball will be and how far we think we can move
		double dist = sqrt(SQUARE(x) + SQUARE(y));
		double diff = dist-numFrames*maxSpeedCMPFrame;
		if (diff < 0) {
		minHS = halfSecs;
		break;
		} else if (diff < mindiff) {
		mindiff = diff;
		minHS = halfSecs;
		}
		}*/

		double solution = 0.0;
		// solve the parabola representing the timestep where our possible posistions intersect the
		// balls predicted locus
		double a = SQUARE(ballVel.x)
			+ SQUARE(ballVel.y)
			- SQUARE(maxSpeedCMPFrame);
		double b = 2 * (ballX *ballVel.x + ballY *ballVel.y);
		double c = SQUARE(ballX) + SQUARE(ballY);
		double discrim = SQUARE(b) - 4 * a *c;

		if (abs(a) < 0.0001) {
			// avoid prediction
			solution = 0;
		}
		else if (discrim < 0.01) {
			// no solution, find closest usng derivative of parabola equals 0, or single solution (same)
			solution = -b / (2 * a);
		}
		else {
			double discSqrt = sqrt(discrim);
			// try to take minimum positive solution
			if (discSqrt > -b)
				solution = (-b + discSqrt) / (2 * a);
			else
				solution = (-b - discSqrt) / (2 * a);
		} 

		if (solution < 0)
			solution = 0.0;
		if (solution > 75)
			solution = 75.0;

		// go towards desired point
		//double x = ballX + minHS*12.5*ballVel.x;
		//double y = ballY + minHS*12.5*ballVel.y;
		double x = ballX + solution *ballVel.x;
		double y = ballY + solution *ballVel.y;
		double dist = sqrt(SQUARE(x) + SQUARE(y));
		double xfrac = x / dist;
		double yfrac = y / dist;
		double mul = (maxSpeed > dist) ? dist : maxSpeed;
		left = -mul * xfrac;
		forward = mul * yfrac;
		if (abs(forward) > maxF) {
			double factor = maxF / abs(forward);
			forward *= factor;
			left *= factor;
		}
		if (abs(left) > maxL) {
			double factor = maxL / abs(left);
			forward *= factor;
			left *= factor;
		}

		//streamsize prec = cout.precision();
		//cout.precision(5);
		//cout << "b={" << ballDist << "," << ballHead << "}/{" << ballX << "," << ballY << "}";
		//cout << " vel={" << ballVel.x << "," << ballVel.y << "}" << endl;
		//cout << "HS=" << minHS << " p={" << x << "," << y << "} [" << turnCCW << "," << forward << "," << left << "]" << endl << endl;
		//cout.precision(prec);
		return;
	}

	// if the ball is at a high angle, walk forward and left appropriately whilst turning
	if (abs(relH) > 17) {
		setOmniParams();
		walkType = CanterWalkWT;

		double maxF = maxCanterForwardSpeed;
		double maxL = maxCanterLeftSpeed;

		//calculate the max forward and left you can do whilst turning
		//counter clockwise case turning slow
		if (turnCCW < 10 && turnCCW >= 0) {
			maxF = 5.5;
			maxL = 5.5;
		} 
		//counter clockwise case turning faster
		else if (turnCCW < 20 && turnCCW >= 0) {
			maxF = 4.5;
			maxL = 4.5;
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

/////////////////////////////////////////////////
// END GOALIE FUNCTIONS
/////////////////////////////////////////////////
