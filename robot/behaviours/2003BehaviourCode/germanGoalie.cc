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


//the goalie doesn't attack the ball if the teammate is gonna get it
//but it may accidently roll under its chin!!

//this needs to be fixed!!








/*
 * Last modification background information
 * $Id: GermanGoalie.cc 1016 2003-04-20 12:31:15Z echung $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#include "german.h"
#include "../share/BallDef.h"


static const double DEFAULT_POS_X = FIELD_WIDTH / 2;
static const double DEFAULT_POS_Y = WALL_THICKNESS + 23;
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

namespace German {
	double allowedAttackDist;
	double allowedAttackDem;

	double blockDistance;
	double blockSpeed;
	double blockAngle;
	int blockLength;
}



void German::initGermanGoalie() {
	lostBall = 0;
	seeBall = 0;
	lastSawBeacon = 0;

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


void German::doGermanGoalie() {
	setDefaultParams();
	setGermanGoalieValues();
	chooseGermanGoalieStrategy();


	if (lockMode == NoMode) {
		Beckham::clipBeckhamFireballPan();
	}
}

void German::setGermanGoalieValues() {
	setGermanGoalie();

	if (vision->vob[vobBall].cf < 3) {
		lostBall = 0;
	}
	else {
		lostBall++;
	}

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

	//Reset smartPositionCounter if we've seen a ball
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

void German::chooseGermanGoalieStrategy() {
	framesWithoutBeacons ++;
	//Reset framesWithoutBeacons if we've seen any beacons
	for (int i = vobBlueLeftBeacon; i < vobYellowRightBeacon; i ++) {
		if (vision->vob[i].cf > 0) {
			framesWithoutBeacons = 0;
		}
	}


	//#ifdef nowayitsdefinedForSure
	if (mode_ == mStartMatch) {
		saBallTracking();
	}
	else if (lockMode != NoMode) {
		// execute atomic action if in locked mode
		switch (lockMode) {
			case GotBall: //if you can see the ball somewhere else abort
				/*
				   if (vBall.cf >= 500) {
				//cout << " yes i can see a ball tilt pan " << tilty << " " << panx << endl;
				if (goalieHoldBallTimer>=25 && tilty >= 0 && panx == 0) {
				cout << "aborting carry ball cause i saw another ball somewhere else" << endl;
				gps->currentlyCarrying = false;
				setStandParams();
				setTrackBallHeadParams(); 
				lockMode = NoMode;
				goalieHoldBallTimer = 0;
				break;
				}
				}
				*/
				goalieGotBall();
				break;
			case ChestPush: aaChestPush();
					break;
			case SpinKick: aaSpinKick();
				       break;
			case SpinChestPush: aaSpinChestPush();
					    break;
			case GoalieKick: aaGoalieKick();
					 break;
			case LightningKick: aaLightningKick();
					    break;
			case LookAtBeacon: lookAtBeaconAndGoToDefendPosition();
					   break;                
			case GoalieHoldBall: aaGoalieHoldBall();
					     break;
		}
	}
	else {
		leds(1, 1, 1, 1, 1, 1);

		// ricky: if almost all i can see is my own goal, then I should turn around becuase I'm probably inside of goal
		int ownGoal;
		const int SIZE_THRESHOLD = 5;

		if (gps->targetGoal == vobBlueGoal) {
			ownGoal = vobYellowGoal;
		}
		else {
			ownGoal = vobBlueGoal;
		}
		// cout << "vision->vob[ownGoal].cf = " << vision->vob[ownGoal].cf << " width=" << vision->vob[ownGoal].width << " height=" << vision->vob[ownGoal].height << " vision->facingYellowGoal=" << vision->facingYellowGoal << " framesWithoutBeacons=" << framesWithoutBeacons << " getOutOfGoalCounter=" << getOutOfGoalCounter << endl;
		//If we can see our own goal real big or we've got our face in either goal or we're locked into a get-out-of-goal routine
		if ((vision->vob[ownGoal].cf
					> 0
					&& (vision->vob[ownGoal].width
						> WIDTH
						- SIZE_THRESHOLD
						|| (vision->vob[ownGoal].height > HEIGHT - SIZE_THRESHOLD
							&& vision->vob[ownGoal].width > WIDTH / 2)))
				|| getOutOfGoalCounter
				> 0) {
			attackBallMode = false;
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
			getOutOfGoalCounter ++;
			if (getOutOfGoalCounter > getOutOfGoalCounterThreshold) {
				//If we've been getting out of the goal for long enough, stop doing so. Note that we don't set the direction, if we
				//get back into here, we want to keep turning in the same direction so we don't keep to-ing and fro-ing. 
				getOutOfGoalCounter = 0;
			}
			return;
		}
		// end goal check



		// if ball is under chin
		if (Beckham::isBeckhamBallUnderChin()) {
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
				//if (gps->getBall(LOCAL)Var <= get95CF(65)) {
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
			} 
			//looking back after just actively localising
			else {
				headtype = lastHeadType;
				tilty = lastHeadTilt;
				panx = lastHeadPan;
			}

			checkLastBallPosTimer--;

			calculateDefendPositionHeading(&defendX, &defendY, &defendH);

			if (usingGpsLastPos && attackBallMode) {
				goalieAttackGpsBall();
			}
			else if (!hasReachedDefendPositionHeadingWhenLookingForBall(defendX,
						defendY,
						defendH)) {
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
					if (goalieBallWithinBlockDistance() && goalieAllowedToBlock()) {
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
					goalieAttackGpsBall();
				}
				else if (!hasReachedDefaultPosition()) {
					goToDefaultPositionHeading();
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

				return;
				/*
				   if (!hasReachedDefendPositionHeadingWhenLookingForBall(defendX, defendY, defendH)) {
				   goToDefendPositionHeading(defendX, defendY, defendH);
				   } else {
				   setStandParams();
				   }
				   */
			}

			attackBallMode = allowedToAttackBall();
			if (attackBallMode) {
				//findBallHeadParams();
				goalieHeadFindBall();
				if (!hasReachedDefendPositionHeadingWhenLookingForBall(defendX,
							defendY,
							defendH)) {
					goToDefendPositionHeading(defendX, defendY, defendH);
				}
				else {
					setStandParams();
				}

				return;
			}
			else {
				//findBallHeadParams();
				goalieHeadFindBall();
				static const int smartCounterLimit = 40;
				if (smartPositionCounter < smartCounterLimit) {
					leds(2, 1, 1, 2, 1, 1);
					if (!hasReachedSmartPosition()) {
						goToSmartPositionHeading();
					}
					else {
						setStandParams();
					}
					smartPositionCounter ++;    //this gets reset once we see a ball. 
					return;
				}
				else if (!hasReachedDefaultPosition()) {
					//We've been sitting at the 'smart' position for smartCounterLimit frames and haven't seen 
					//the ball, go to default position. 
					goToDefaultPositionHeading();
					return;
				}
				else {
					setStandParams();
					goalieHeadFindBall();
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

		if (goalieBallWithinBlockDistance()
				&& vBall.d > 30
				&& goalieAllowedToBlock()) {
			//cout << "I WANT TO BLOCK!" << endl;
			setBlockParams();
			setTrackBallHeadParams();
			goalieBlockTimer = blockLength;
			return;
		}

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
		if (lookAtBeaconCounter >= 24) {
			if (vBall.d >= 20) {
				lookAtBeaconCounter = 0;


				lastHeadType = headtype;
				lastHeadTilt = tilty;
				lastHeadPan = panx;

				lookAtBeaconTimer = 0;
				lockMode = LookAtBeacon;            
				setNewBeacons(FIELD_LENGTH * 0.75);
				lookAtBeaconAndGoToDefendPosition();

				checkLastBallPosTimer = 6;
			}
		}

		lookAtBeaconCounter++;
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

bool German::goalieAllowedToBlock() {
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

bool German::goalieBallWithinBlockDistance() {
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
	if (goalieIsInGoalieBox() && (vBall.d <= 60)) {
		return true;
	}
	return false;
}

bool German::goalieBallIsGoingToArriveAtYouSoon() {
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


bool German::goalieBallWithinDangerDistance() {
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


static const double attackDistance = 5;
bool German::goalieBallWithinAttackDistance() {
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

	if (goalieIsInGoalieBox() && (vBall.d <= 15) && !opponentIsCloserToBall()) {
		return true;
	}
	return false;
}

bool German::ballIsInCornerAreas() {
	static const double cornerXSize = WALL_THICKNESS + 80; //these params are VERY BIG Dangerous?
	static const double cornerYSize = WALL_THICKNESS + 50; //these params are VERY BIG Dangerous?

	if ((gps->getBall(GLOBAL).x <= cornerXSize)
			|| (gps->getBall(GLOBAL).x >= (FIELD_WIDTH - cornerXSize))
			&& (gps->getBall(GLOBAL).y <= cornerYSize)) {
		return true;
	}
	return false;
}

bool German::opponentIsCloserToBall() {
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

bool German::allowedToAttackBall() {
	static const double robotAllowedToAttackDistance = 10;
	static const double ballAllowedToAttackDistance = 50;

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

	//this section has the effect that
	//if you are far away from your goal, the ball has to be close to attack it.
	//the further you are, the closer the ball needs to be
	//the function sort of looks like a sqrt function.. use arcFunction to specify
	//how "arc"ed the function is
	static const double maxAttackableBallDist = 55;
	static const double maxAllowedDogDist = 60;
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
	if (teammateHasBall()) {
		//cout << "STOP attacking ball now because the teammate has the ball" << endl;
		return false;
	}

	return attackBallMode; //ie whatever it used to be
}

bool German::teammateHasBall() {
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


bool German::isFacingAwayFromOwnGoal(double posx, double posy, double posh) {
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


double German::goalieDistanceFromHome() {
	double distx = DEFAULT_POS_X - gps->self().pos.x;
	double disty = DEFAULT_POS_Y - gps->self().pos.y;
	return sqrt(SQUARE(distx) + SQUARE(disty));
}


	bool German::goalieIsInGoalieBox() {
		if (gps->self().pos.x >= (FIELD_WIDTH / 2.0) - (GOALBOX_WIDTH / 2.0)
				&& gps->self().pos.x <= (FIELD_WIDTH / 2.0) + (GOALBOX_WIDTH / 2.0)
				&& gps->self().pos.y <= (WALL_THICKNESS + GOALBOX_DEPTH)) {
			return true;
		}
		return false;
	}


	bool German::ballJumpedTooMuch() {
		double distance = sqrt(SQUARE(oldBallx - gps->getBall(LOCAL).x)
				+ SQUARE(oldBally - gps->getBall(LOCAL).y));
		if (distance >= 60) {
			return true;
		} 
		return false;
	}

void German::lookAtBeaconAndGoToDefendPosition() {
	activeGpsLocalise(false); 
	if (lookAtBeaconTimer == 8) {
		lockMode = NoMode;
	}
	lookAtBeaconTimer++;

	setStandParams();

	calculateDefendPositionHeading(&defendX, &defendY, &defendH);
	if (/*farFromDefendPosition() &&*/ !hasReachedDefendPositionHeadingWhenLookingForBall(defendX,
				defendY,
				defendH)) {
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


void German::calculateDefendPositionHeading(double *destx,
		double *desty,
		double *desth) {
	static const double pivotx = (double) FIELD_WIDTH / (double) 2;
	static const double pivoty = 10;
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

bool German::hasReachedDefaultPosition() {
	double diffx = DEFAULT_POS_X - gps->self().pos.x;
	double diffy = DEFAULT_POS_Y - gps->self().pos.y;

	if (ABS(diffx) <= POS_ERROR_MARGIN && ABS(diffy) <= POS_ERROR_MARGIN) {
		return true;
	}
	return false;
}

void German::goToDefaultPositionHeading() {
	saGoToTargetFacingHeading(DEFAULT_POS_X, DEFAULT_POS_Y, DEFAULT_POS_H);
}


/**
 * Note, this 'smart position' isn't actually all that clever. We rely on the fact that 
 * when the goalie loses the ball, chances are it was on the same side of the goal as the ball. 
 * Therefore, go to the point on the circle closest to where the ball was last seen (we assume that 
 * GPS ball doesn't move if we've not seen it). 
 */
bool German::hasReachedSmartPosition() {
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

void German::goToSmartPositionHeading() {
	//  goToDefaultPositionHeading(); return; //uncomment this to easily return to previous behaviour. 
	double destX, destY, destH;
	calculateDefendPositionHeading(&destX, &destY, &destH);
	saGoToTargetFacingHeading(destX, destY, destH);
}

void German::findSmartPosition(int *posX, int *posY, int *posH) {
	//yes this is a null function for now ...
}



void German::goToDefendPositionHeading(double defendX,
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

bool German::hasReachedDefendPositionHeading(double defendX,
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

bool German::hasReachedDefendPositionHeadingWhenLookingForBall(double defendX,
		double defendY,
		double defendH) {
	double diffx = defendX - gps->self().pos.x;
	double diffy = defendY - gps->self().pos.y;
	double diffh = defendH - gps->self().h;

	if (ABS(diffx) <= POS_ERROR_MARGIN_LARGER
			&& ABS(diffy) <= POS_ERROR_MARGIN_LARGER
			&& ABS(diffh) <= H_ERROR_MARGIN) {
		return true;
	}
	return false;
}

/////////////////////////////////////////////////
// GOALIE ATTACK BALL FUNCTIONS
/////////////////////////////////////////////////


void German::goalieAttackGpsBall() {
	/*
	// if cleared the ball, set attackBallMode to false
	if (tilty < -60.0 && ABS(panx) < 20.0 && vBall.vob->cam_dist < 10) {
	lockMode = GoalieHoldBall;
	aaGoalieHoldBall();
	}
	*/

	goalieHoverToBall(false);
}
void German::goalieAttackBall() {
	setTrackBallHeadParams();

	/*
	// if cleared the ball, set attackBallMode to false
	if (tilty < -60.0 && ABS(panx) < 20.0 && vBall.vob->cam_dist < 10) {
	lockMode = GoalieHoldBall;
	aaGoalieHoldBall();
	}
	*/

	goalieHoverToBall(true);
}

static const int holdLengthBeforeCarrying = 10;
void German::aaGoalieHoldBall() {
	setTrackBallHeadParams();

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
	goalieGotBall();
	//aaGotBall();
	return;
}

static double gapMin; //is originally local based, where the gap is set by find best gap function
static double gapMax; //but later becomes global based

//at the start it chooses a gap
//it then turns to that gap and shoots
void German::goalieGotBall() {
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
		//  cout << "clockwise" << endl;
		//} else {
		//  cout << "anit-clockwise" << endl;
		//}
	}

	//if (clockwise) {
	//  cout << "clockwise      ";
	//} else {
	//  cout << "anit-clockwise ";
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
	if (!goalieIsInGoalieBox() && goalieHoldBallTimer >= 65) {
		shoot = true;
		gps->currentlyCarrying = false;
		lockMode = GoalieKick;
		goalieHoldBallTimer = 0;
		aaGoalieKick();
		return;
	} 
	//if held too long inside goalie box shoot
	else if (goalieHoldBallTimer >= 85) {
		shoot = true;
		gps->currentlyCarrying = false;
		lockMode = GoalieKick;
		goalieHoldBallTimer = 0;
		aaGoalieKick();
		return;
	}

	if (shoot) {
		/*
		   gps->currentlyCarrying = false;
		   lockMode = GoalieKick;
		   goalieHoldBallTimer = 0;
		   aaGoalieKick();
		   return;
		   */
		turnCCW = 0;
		left = 0;
		forward = 0;
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

bool German::goalieAbleToDoARampShot() {
	//facing the ramp
	//no dogs between you and the ramp
	//dogs a reasonable distance out of the way of exit path of ramp
	return false;
}

void German::findBestGap(double minAllowedAngle,
		double maxAllowedAngle,
		double *returnMinAngle,
		double *returnMaxAngle) {
	static const int CLEARANCE_DISTANCE = 25; //the distance to either side of the dogs position that is considered "blocked"
	static const int CLEARANCE_ANGLE = 20; //the angle where, all gaps over this size are considered equally good

	//get all the dogs you want to avoid
	int countValidDogs = 0;
	sortee dogs[NUM_OPPONENTS];
	for (int i = 0; i < NUM_OPPONENTS; i++) {
		if (gps->getOppCovMax(i) < get95CF(50) && gps->getOppPos(i).d < 300) {
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
// GOALIE UTILITY FUNCTIONS
/////////////////////////////////////////////////

void German::setGpsTrackBallHeadParams() {
	headtype = ABS_XYZ;

	panx = -gps->getBall(LOCAL).x;
	tilty = BALL_DIAMETER; //BALL_DIAMETER/2.0;
	cranez = gps->getBall(LOCAL).y;
}

static double old_tilt = 0;
static double old_pan = 0;
#define FOV 10

void German::setTrackBallHeadParams() {
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


void German::goalieFullCircleFindBall() {
	//findBallHeadParams();
	goalieHeadFindBall();

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

void German::goalieSemiCircleFindBall() {
	//findBallHeadParams();
	goalieHeadFindBall();

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

void German::goalieHeadFindBall() {
	static const double lowtilt = -50;
	static const double hightilt = 5;

	int lowfirst = 0;

	// num frames in 1/2 cycle of head finding ball
	static const int HEAD_PERIOD = 30; 

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


void German::goalieHoverToBall(bool usingVision) {
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


/////////////////////////////////////////////////
// END GOALIE FUNCTIONS
/////////////////////////////////////////////////

