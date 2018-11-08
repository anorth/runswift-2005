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
 * Penalty Shooter
 *
 **/ 

#include "Behaviours.h"
#include "../share/BallDef.h"
#include "../share/Common.h"
#include "penaltyShooter.h"

// own goal coordinates
static const double OWN_GOAL_X = FIELD_WIDTH / 2.0;
static const double OWN_GOAL_Y = 0;

// target goal coordinates
static const double TARGET_GOAL_X = FIELD_WIDTH / 2.0;
static const double TARGET_GOAL_Y = FIELD_LENGTH;

static const double LEFT_GOALBOX_EDGE_X = TARGET_GOAL_X - GOALBOX_WIDTH / 2.0;
static const double LEFT_GOALBOX_EDGE_Y = TARGET_GOAL_Y
- WALL_THICKNESS
- GOALBOX_DEPTH;

static const double RIGHT_GOALBOX_EDGE_X = TARGET_GOAL_X
+ GOALBOX_WIDTH
/ 2.0;
static const double RIGHT_GOALBOX_EDGE_Y = TARGET_GOAL_Y
- WALL_THICKNESS
- GOALBOX_DEPTH;

// vision ball constants for findball trigger
static const int vBallLost = 10;
static const int vBallAnywhere = 40;

// active localise timer
static int activeLocaliseTimer = 0;
static int disallowActiveLocalise = 0;

//variables needed to lock looking back to old ball pos after doing active localise
static int checkLastBallPos = 6;
static int lastHeadType = ABS_XYZ;
static double lastHeadTilty = 0;
static double lastHeadPanx = 0;
static double lastHeadNullz = 0;

// which ball to use in current frame
static int ballSource = VISION_BALL;
static double ballX = 0;
static double ballY = 0;
static double ballH = 0;
static double ballD = 0;

// bernhard's visual ball smoothing for ball grabbing
static double fstVisBallDist = 0;
static double fstVisBallHead = 0;
static double sndVisBallDist = 0;
static double sndVisBallHead = 0;
static double thdVisBallDist = 0;
static double thdVisBallHead = 0;

void PenaltyShooter::initPenaltyShooter() {
	lockMode = NoMode;
	lostBall = 0;
	grabTime = 0;
}

void PenaltyShooter::doPenaltyShooter() {
	setDefaultParams();
	setWalkParams();
	setPenaltyShooterValues();
	choosePenaltyShooterStrategy();
}

void PenaltyShooter::setPenaltyShooterValues() {
	leds(1, 1, 1, 1, 1, 1);

	// determine which ball source to use
	if (gps->canSee(vobBall)) {
		ballSource = VISION_BALL;
		ballX = vBall.x;
		ballY = vBall.y;
		ballH = vBall.h;
		ballD = vBall.d;
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
	}
	else {
		ballSource = GPS_BALL;
		ballX = gps->getBall(GLOBAL).x;
		ballY = gps->getBall(GLOBAL).y;
		ballH = gps->getBall(LOCAL).head;
		ballD = gps->getBall(LOCAL).d;
	}

	DECREMENT(activeLocaliseTimer);
	DECREMENT(disallowActiveLocalise);
	DECREMENT(grabTime);
	DECREMENT(checkLastBallPos);

	attackMode = GoToBall;
	utilDetermineDKD();

	// bernhard's visual ball smoothing for ball grabbing
	if (gps->canSee(vobBall)) {
		thdVisBallDist = sndVisBallDist;
		thdVisBallHead = sndVisBallHead;
		sndVisBallDist = fstVisBallDist;
		sndVisBallHead = fstVisBallHead;
		fstVisBallDist = vBall.d;
		fstVisBallHead = vBall.h;
	}
}

void PenaltyShooter::choosePenaltyShooterStrategy() {
	if (mode_ == mStartMatch) {
		doPenaltyShooterTrackVisualBall();
	}
	else if (lockMode != NoMode && lockMode != ResetMode) {
		doPenaltyShooterTrackVisualBall();

		// execute atomic action if in locked mode
		switch (lockMode) {
		        case ChestPush: aaChestPush();
					break;

			case SpinKick: aaSpinKick();
				       break;

			case SafeSpinKick: aaSafeSpinKick();
					   break;

			case VisualSpinKick: aaVisualSpinKick();
					     break;

			case VisOppAvoidanceKick: aaVisOppAvoidanceKick();
						  break;

			case PenaltyShooterAimKick: doPenaltyShooterAimKick();
						    break;

			case SpinChestPush: aaSpinChestPush();
					    break;

			case GoalieKick: aaGoalieKick();
					 break;

			case LightningKick: aaLightningKick();
					    break;

			case ProperVariableTurnKick: aaProperVariableTurnKick();
			                 break;
		        default: aaLightningKick();
		                         break;
		}
	}
	else {
		if (gps->canSee(vobBall)) {
			doPenaltyShooterTrackVisualBall();
		}
		else if (lostBall <= vBallLost && abs(gps->getBall(LOCAL).head) < 90) {
			doPenaltyShooterTrackGpsBall();
		}
		else {
			headFindBall();
		}

		// if ball is under chin
		if (isPenaltyShooterBallUnderChin()) {
			choosePenaltyShooterShootStrategy();    
			return;
		}

		// normal strategy
		doPenaltyShooterNormal();
	}

	// needed for getBehindBall to work
	prevAttackMode = attackMode;

	// safeguard to ensure panx is clipped on close approach to ball
	// necessary in order to grab the ball reliably
	clipPenaltyShooterFireballPan();
}

/////////////////////////////////////////////////////////////////////////
// conditional functions
/////////////////////////////////////////////////////////////////////////

bool PenaltyShooter::isPenaltyShooterBallUnderChin() {
	if (!gps->canSee(vobBall)) {
		return false;
	}

	// calculate vision ball local coordinates
	double ballx = sin(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
	double bally = cos(DEG2RAD(vBall.vob->h)) * vBall.vob->d;

	// set pan to zero in preparation for ball grabbing when ball is close
	clipPenaltyShooterFireballPan();

	// check for ball under chin condition
	if (ABS(ballx) <= 3.0 && bally <= 1.5 * BALL_RADIUS && hTilt <= -40) {
		return true;
	}
	return false;
}

void PenaltyShooter::clipPenaltyShooterFireballPan() {
	if (!gps->canSee(vobBall)) {
		return;
	}

	// calculate vision ball local coordinates
	double ballx = sin(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
	double bally = cos(DEG2RAD(vBall.vob->h)) * vBall.vob->d;

	if ((ABS(ballx) <= 3.0 && bally <= 2.0 * BALL_RADIUS)
			|| (hTilt <= -30 && ABS(ballx) <= 3.0 && bally <= 3.0 * BALL_RADIUS)) {
		panx = 0;
	}
}

/////////////////////////////////////////////////////////////////////////
// normal functions
/////////////////////////////////////////////////////////////////////////

void PenaltyShooter::doPenaltyShooterNormal() {
	// if locked into active localise, continue to localise
	if (activeLocaliseTimer) {
		activeGpsLocalise(false);

		doPenaltyShooterHoverToBall(gps->getBall(LOCAL).d,
				gps->getBall(LOCAL).head);

		if (activeLocaliseTimer == 1) {
			disallowActiveLocalise = 40;
			if (lostBall > vBallLost) {
				lostBall = 0;
			}
		}                    

		return;
	}

	// if lost ball, perform find ball
	if (lostBall > vBallLost && ballSource == GPS_BALL) {
		// if you've lost the ball, figure out which way to spin first

		// first time in series
		if (lostBall == vBallLost + 1) {
			setLocateBallParams();
		}

		// nathan must make eric fix this dodgy hack
		locateBall(145, 220);
		return;
	}

	// if we can start to active localise
	if (ballSource == VISION_BALL
			&& vBall.d > 80
			&& ABS(vBall.h) < 5
			&& !disallowActiveLocalise) {
		// so far normal forward sets the lastballpos timer but doesn't use it
		if (gps->canSee(vobBall)) {
			doPenaltyShooterTrackVisualBall();
			checkLastBallPos = 6;
			lastHeadType = headtype;
			lastHeadTilty = tilty;
			lastHeadPanx = panx;
			lastHeadNullz = cranez;
		}

		setNewBeacons(LARGE_VAL, 50);            
		activeGpsLocalise(false);
		activeLocaliseTimer = 8;
		doPenaltyShooterHoverToBall(gps->getBall(LOCAL).d,
				gps->getBall(LOCAL).head);
		return;
	}

	if (ballSource == GPS_BALL) {
		doPenaltyShooterHoverToBall(gps->getBall(LOCAL).d,
				gps->getBall(LOCAL).head);
		doPenaltyShooterTrackGpsBall();
		return;
	}

	// choose appropriate approach ball behaviour
	doPenaltyShooterAttackBall();
}

void PenaltyShooter::doPenaltyShooterAttackBall() {
	double reqAccuracy = requiredAccuracy(25);

	// local coords
	double minGoalHead = gps->tGoal().pos.head - reqAccuracy;
	double maxGoalHead = gps->tGoal().pos.head + reqAccuracy;

	//////////////////////////////////////////////////////////////////////////////////////////
	// PAWKICK ON EDGE CASES
	//////////////////////////////////////////////////////////////////////////////////////////

	// pawkick on left edge
	if (ballSource == VISION_BALL
			&& utilBallOnLEdge()
			&& utilIsOnLEdge()
			&& gps->self().h <120
			&& gps->self().h> 60
			&& gps->self().posVar < get95CF(75)
			&& abs(vBall.h) < 30) {
		firePawKick(FIRE_PAWKICK_LEFT);
		return;
	} 

	// pawkick on right edge
	if (ballSource == VISION_BALL
			&& utilBallOnREdge()
			&& utilIsOnREdge()
			&& gps->self().h <120
			&& gps->self().h> 60
			&& gps->self().posVar < get95CF(75)
			&& abs(vBall.h) < 30) {
		firePawKick(FIRE_PAWKICK_RIGHT);
		return;
	} 

	//////////////////////////////////////////////////////////////////////////////////////////
	// GET BEHIND BALL CASES
	//////////////////////////////////////////////////////////////////////////////////////////

	// if the ball is on one of the side edges and you are not facing the right way
	if (ballSource == VISION_BALL
			&& vBall.d < 50
			&& utilBallOnLREdge()
			&& gps->self().h >= 195
			&& gps->self().h <= 345) {
		saFarGetBehindBall(prevAttackMode != GetBehindBall);

		// this is a hack required because the dog gets behind the ball already 
		// and still trys to get to the ball resulting in it walkin backwards
		if (forward >= 0) {
			return;
		}
	}

	if (ballSource == VISION_BALL
			&& vBall.d <40
			&& utilBallOnBEdge()
			&& gps->self().h> 200
			&& gps->self().h < 340) {
		double awayFromOwnGoal = RAD2DEG(atan2(vBall.y - 0,
					vBall.x - (FIELD_WIDTH / 2.0)));
		saFarGetBehindBall(prevAttackMode != GetBehindBall, awayFromOwnGoal);

		//this is a hack required because the dog gets behind the ball already 
		//and still trys to get to the ball resulting in it walkin backwards
		if (forward >= 0) {
			return;
		}
	} 

	//////////////////////////////////////////////////////////////////////////////////////////
	// DEFAULT CASE
	//////////////////////////////////////////////////////////////////////////////////////////

	// bernhard's visual ball smoothing for ball-grabbing

	double weightedVisBallDist = fstVisBallDist*5.0
		/ 10.0
		+ sndVisBallDist*3.0
		/ 10.0
		+ thdVisBallDist*2.0
		/ 10.0;
	double weightedVisBallHead = fstVisBallHead*5.0
		/ 10.0
		+ sndVisBallHead*3.0
		/ 10.0
		+ thdVisBallHead*2.0
		/ 10.0;

	doPenaltyShooterHoverToBall(weightedVisBallDist, weightedVisBallHead);
}

void PenaltyShooter::choosePenaltyShooterShootStrategy() {
	double reqAccuracy = requiredAccuracy(25);

	double minGoalHead = gps->tGoal().pos.head - reqAccuracy;
	double maxGoalHead = gps->tGoal().pos.head + reqAccuracy;

	double min = MIN(abs(minGoalHead), abs(maxGoalHead));
	double max = MAX(abs(minGoalHead), abs(maxGoalHead));

	grabTime = 65;

	// on top, bottom or side edges
	if (isPenaltyShooterOnAnEdge()) {
		lockMode = SpinChestPush;
		aaSpinChestPush();
		return;
	}

	// in the attacking half
	if (gps->self().pos.y > FIELD_LENGTH / 2) {
		// better to be safe than sorry
		if (gps->self().pos.y >= (FIELD_LENGTH - WALL_THICKNESS - 20)) {
			lockMode = SpinChestPush;
			aaSpinChestPush();
			return;
		}

		// if already lined up with goal
		if (maxGoalHead >= 0 && minGoalHead <= 0) {
			lockMode = PenaltyShooterAimKick;
			doPenaltyShooterAimKick();
			return;
		}

		//////////////////////////////////////////////////////////////
		// past this point both goal posts must be on the same side
		//////////////////////////////////////////////////////////////

		// out of turn kick range & not on edges, do spin front kick
		if (max <= 110 && !isPenaltyShooterOnAnEdge()) {
			lockMode = PenaltyShooterAimKick;
			doPenaltyShooterAimKick();
			return;
		}

		/*
		// turn kick go for goal
		double dir;
		if (min <= 90 && max >= 90) {
		dir = (gps->tGoal().pos.head > 0 ? 90 : -90);
		} else if (min <= 180 && max >= 180) {
		dir = (gps->tGoal().pos.head > 0 ? 180 : -180);
		} else {
		if (abs(gps->tGoal().pos.head) < 135) {
		dir = (gps->tGoal().pos.head > 0) ? 90 : -90;
		} else {
		dir = (gps->tGoal().pos.head > 0) ? 180 : -180;
		}
		}
		lockMode = ProperVariableTurnKick;
		setProperVariableTurnKick(dir);
		aaProperVariableTurnKick();
		*/

		lockMode = SpinChestPush;
		aaSpinChestPush();
		return;
	}
	else {
		// defend half    

		reqAccuracy = requiredAccuracy(3.0 * FIELD_WIDTH / 2.0);

		// local coords

		minGoalHead = gps->tGoal().pos.head - reqAccuracy;
		maxGoalHead = gps->tGoal().pos.head + reqAccuracy;
		min = MIN(abs(minGoalHead), abs(maxGoalHead));
		max = MAX(abs(minGoalHead), abs(maxGoalHead));        

		if (maxGoalHead >= 0 && minGoalHead <= 0) {
			lockMode = PenaltyShooterAimKick;
			doPenaltyShooterAimKick();
			return;
		} 

		//////////////////////////////////////////////////////////////////////
		// past this point both goal posts must be on the same side
		// front kick it forward to the target half
		//////////////////////////////////////////////////////////////////////

		lockMode = PenaltyShooterAimKick;
		doPenaltyShooterAimKick();

		return;
	}
}

/////////////////////////////////////////////////////////////////////////
// action functions
/////////////////////////////////////////////////////////////////////////

static double old_tilt = 0;
static double old_pan = 0;

void PenaltyShooter::doPenaltyShooterTrackVisualBall() {
	if (gps->canSee(vobBall)) {
		double ballx = sin(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
		double bally = cos(DEG2RAD(vBall.vob->h)) * vBall.vob->d;

		headtype = ABS_XYZ;

		panx = ballx;
		tilty = BALL_DIAMETER;
		cranez = bally;

		old_tilt = tilty;
		old_pan = panx;
	}
	else {
		tilty = old_tilt;
		panx = old_pan;
	}
}        

void PenaltyShooter::doPenaltyShooterTrackGpsBall() {
	headtype = ABS_XYZ;

	panx = -gps->getBall(LOCAL).x;
	tilty = BALL_DIAMETER;
	cranez = gps->getBall(LOCAL).y;
}        

void PenaltyShooter::doPenaltyShooterHoverToBall(double ballDist,
		double ballHead) {
	bool onLeft = utilBallOnLEdge()
		&& utilIsOnLEdge()
		&& gps->self().posVar < get95CF(75);
	bool onRight = utilBallOnREdge()
		&& utilIsOnREdge()
		&& gps->self().posVar < get95CF(75);
	bool onTop = utilBallOnTEdge()
		&& utilIsOnTEdge()
		&& gps->self().posVar < get95CF(75);
	bool onBottom = utilBallOnBEdge()
		&& utilIsOnBEdge()
		&& gps->self().posVar < get95CF(75);

	static const double maxCanterForwardSpeed = 8;
	static const double maxCanterLeftSpeed = 6;

	double relH = ballHead;

	if (onLeft || onRight) {
		if (NormalizeAngle_0_360(gps->self().h) < HALF_CIRCLE) {
			relH = QUARTER_CIRCLE - gps->self().h;
			if (ABS(relH - ballHead) > 10) {
				relH = ballHead;
			}
		}
		else {
			relH = 3 * QUARTER_CIRCLE - gps->self().h;
			if (ABS(relH - ballHead) > 10) {
				relH = ballHead;
			}
		}
	}
	else if (onTop || onBottom) {
		if (NormalizeAngle_0_360(gps->self().h) < QUARTER_CIRCLE
				|| NormalizeAngle_0_360(gps->self().h) > 3 * QUARTER_CIRCLE) {
			relH = NormalizeAngle_180(gps->self().h);
			if (ABS(relH - ballHead) > 10) {
				relH = ballHead;
			}
		}
		else {
			relH = HALF_CIRCLE - gps->self().h;
			if (ABS(relH - ballHead) > 10) {
				relH = ballHead;
			}
		}
	}

	turnCCW = CLIP(relH / 2.0, 40.0);

	if (ABS(turnCCW) <= 13.75 && ballDist > 30) {
		setWalkParams();
		walkType = ZoidalWalkWT;
		left = 0;

		if (abs(turnCCW) <= 8) {
			forward = MAX_OFFSET_FORWARD_SMALL_TURN;
		}
		else {
			forward = MAX_OFFSET_FORWARD_LARGE_TURN;
		}
	}
	else if (ABS(relH) > 18) {
		setOmniParams();

		walkType = CanterWalkWT;

		double maxF = maxCanterForwardSpeed;
		double maxL = maxCanterLeftSpeed;

		if (turnCCW < 10 && turnCCW >= 0) {
			maxF = 6;
			maxL = 6;
		} 

		// counter clockwise case turning faster
		else if (turnCCW < 20 && turnCCW >= 0) {
			maxF = 5;
			maxL = 5;
		}

		// turn too fast anticlockwise case
		else if (turnCCW >= 0) {
			maxF = 3;
			maxL = 2;
		}

		// slow clockwise turn case
		else if (turnCCW > -10 && turnCCW < 0) {
			maxF = 5;
			maxL = 5;
		}

		// all other clockwise cases
		else {
			maxF = 1;
			maxL = 3;
		}

		// calculate how much you should go forward and left whilst turning
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
	}

	// otherwise use zoidal walk to run to the ball
	else {
		setWalkParams();
		walkType = ZoidalWalkWT;
		forward = MAX_OFFSET_FORWARD;
		left = 0;
	}

	// the rest are guards
	if (forward <0 || ABS(turnCCW)> 15) {
		walkType = CanterWalkWT;
		forward = CLIP(forward, maxCanterForwardSpeed);
	}

	if (ballDist < 20) {
		walkType = CanterWalkWT;
		forward = 4.5;
		PG = 30;
		left = 0;
		turnCCW = CLIP(ballHead / 2.0, 30.0);
		if (ABS(turnCCW) > 20 && ballDist < 20) {
			forward = 0.5;
		}
	}

	if (walkType == CanterWalkWT) {
		if (forward > maxCanterForwardSpeed) {
			forward = maxCanterForwardSpeed;
		}

		if (turnCCW < 10 && turnCCW >= 0) {
			forward = CLIP(forward, 7.5);
			left = CLIP(left, 6.0);
		} 

		// counter clockwise case turning faster
		else if (turnCCW < 20 && turnCCW >= 0) {
			forward = CLIP(forward, 5.7);
			left = CLIP(left, 5.0);
		} 

		// turn too fast anticlockwise case
		else if (turnCCW >= 0) {
			forward = CLIP(forward, 5.0);
			left = CLIP(left, 5.0);
		}

		// slow clockwise turn case
		else if (turnCCW > -10 && turnCCW < 0) {
			forward = CLIP(forward, 5.0);
			left = CLIP(left, 5.0);
		} 
		// all other clockwise cases
		else {
			forward = CLIP(forward, 4.3);
			left = CLIP(left, 5.0);
		}
	}
	else {
		if (forward > MAX_OFFSET_FORWARD) {
			forward = MAX_OFFSET_FORWARD;
		}
	}
}    

bool PenaltyShooter::isPenaltyShooterOnLEdge() {
	return (gps->self().pos.x < WALL_THICKNESS + 25);
}    

bool PenaltyShooter::isPenaltyShooterOnREdge() {
	return (gps->self().pos.x > FIELD_WIDTH - (WALL_THICKNESS + 25));
}

bool PenaltyShooter::isPenaltyShooterOnBEdge() {
	return (gps->self().pos.y < WALL_THICKNESS + 25);
}

bool PenaltyShooter::isPenaltyShooterOnTEdge() {
	return (gps->self().pos.y > FIELD_LENGTH - (WALL_THICKNESS + 25));
}

	bool PenaltyShooter::isPenaltyShooterOnAnEdge() {
		return (isPenaltyShooterOnLEdge()
				|| isPenaltyShooterOnREdge()
				|| isPenaltyShooterOnBEdge()
				|| isPenaltyShooterOnTEdge());
	}
