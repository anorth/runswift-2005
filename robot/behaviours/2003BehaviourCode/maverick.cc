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

// POSSIBLE PROBLEM
// THOSE FUNKY ATTACKMODE PREVATTACK MODE THINGS COULD BE SCREWING UP THE GET BEHIND BALL

// all those get behind balls, im only checking if the ball is closer than something not further than something
// is it possibly that the ball would be very close and im still trying to do a get behind ball?

//in all those get behind balls, that if forward is negative -> hoverToBall hack is probably bad
//because it stops you from going through the rest of the decision tree


/*
 * Last modification background information
 * $Id$
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Team E Forward
 *
 **/

#include "maverick.h"
#include "ballChallenge.h"
#include "../share/BallDef.h"

//using ballChallenge::GetNearBall;

static const double OWNGOALX = FIELD_WIDTH / 2.0;
static const double OWNGOALY = 0;
static const double TARGETGOALX = FIELD_WIDTH / 2.0;
static const double TARGETGOALY = FIELD_LENGTH;

static const double rightGoalBoxEdgex = FIELD_WIDTH
/ 2.0
+ GOALBOX_WIDTH
/ 2.0;
static const double rightGoalBoxEdgey = FIELD_LENGTH
- WALL_THICKNESS
- GOALBOX_DEPTH;
static const double leftGoalBoxEdgex = FIELD_WIDTH
/ 2.0
- GOALBOX_WIDTH
/ 2.0;
static const double leftGoalBoxEdgey = FIELD_LENGTH
- WALL_THICKNESS
- GOALBOX_DEPTH;

static const int BO_ATTACK = 0;
static const int BO_GETBEHIND = 1;
static const int BO_BACKOFF = 2;
static const int BO_NONE = 3;

static int findBallKount = 0;
static int lastTCCW = 0;

static int findBallDirectionOverrideDelay = 0;

static int lastBackOffStrategy = BO_NONE;

static const int vBallLost = 10;
static const int vBallAnywhere = 40;

static const int LOSTBALL_DIST = 500;

static int framesSinceBO = 0;

static int teamDForwardFBDir = ANTICLOCKWISE;
static int lastSeenOpponent = 0;

static int ballSource = VISION_BALL;

static bool allowedToCharge = false;
static bool chargeBall = false;
static bool avoidOpponent = false;

static int activeLocaliseTimer = 0;
static int disallowActiveLocalise = 0;
static int hasBallTimer = 0;

static int attackGpsBall = 0;

static int kickOffTimeout = 0;

static int lock3rdClosest = 0;
static int grablock = 0;

namespace Maverick {
	double mavForward;
	double mavTurn;
	double mavLeft;
	bool is3rdClosestDebug;
	bool is3rdPositionDebug;
	bool is3rdUpDownDebug;
	bool fireBallLights;
	bool backOffEyes;
	bool thirdPlayerEyes;
	bool newShootStratEyes;
	bool maverickDebug;
	int anotherCounter;
}

static double ballX = 0;
static double ballY = 0;
static double ballH = 0;
static double ballD = 0;

static const int wirelessBallMinThreshold = 100;
static double maverickWirelessBallDistThreshold = wirelessBallMinThreshold;
static double storeWirelessBallDistThreshold = 0;

static bool adjustHead = true;

static bool ericSilence = true;
#define eout cout

void Maverick::initMaverickForward() {
	lockMode = NoMode;
	lostBall = 0;

	grabTime = 0;
	attackLock = 0;
	useZoidWalk = 1;
	mavForward = MAX_OFFSET_FORWARD;
	mavTurn = 15;

	is3rdClosestDebug = false;
	is3rdPositionDebug = false;
	is3rdUpDownDebug = false;

	fireBallLights = false;
	backOffEyes = false;
	thirdPlayerEyes = false;
	newShootStratEyes = true;
	maverickDebug = false;

	anotherCounter = 0;
}

void Maverick::doMaverickForward() {
	setDefaultParams();
	setMaverickForwardValues();
	chooseMaverickForwardStrategy();
	//eout << gps->r.ballDist << endl;
}

void Maverick::setMaverickForwardValues() {
	if (gps->canSee(vobBall)) {
		storeWirelessBallDistThreshold = 0;
	}

	//you need to say that your a forward
	//so that when you share world model packets with your friends
	//they know that the packet came from a forward.
	//this is needed because information about the ball which comes from a forward is treated differently than that 
	//which comes from a goalie
	gps->r.playerType = FORWARD;

	if (gps->canSee(vobBall)) {
		gps->r.ballDist = (int) vBall.d;
	}
	else {
		gps->r.ballDist = LOSTBALL_DIST;
	}

	//timeout the kickoff after 10 sec
	if (kickOffTimeout < 250) {
		kickOffTimeout++;
	}
	if (kickOffTimeout == 250) {
		kickingOff = false;
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

	static const int CHARGE_THRESHOLD = 12;
	static const int BLOCK_THRESHOLD = 15;

	allowedToCharge = false;

	//cout << "****************** " << lastSeenOpponent << endl;
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		/*
		   cout << "can see  " << vOpponent[i].cf << endl;
		   cout << "oppHead  " << vOpponent[i].h << endl;
		   cout << "ballHead " << vBall.h << endl;
		   cout << "oppDist  " << vOpponent[i].d << endl;
		   cout << "balldist " << vBall.d << endl;
		   cout << endl;
		   */

		// should be using variance - Ross
		if (vOpponent[i].cf
				> 0
				&& abs(vOpponent[i].h - vBall.h)
				< BLOCK_THRESHOLD
				&& abs(vOpponent[i].d
					- vBall.d)
				< BLOCK_THRESHOLD) {
			lastSeenOpponent = 0;
			break;
		}
	}

	if (lastSeenOpponent < CHARGE_THRESHOLD) {
		lastSeenOpponent++;
		allowedToCharge = true;
		//cout << "&&&&&&&&&&&&&&&&&&&&&& CHARGE!! " << endl;
	}

	chargeBall = false;

	attackMode = GoToBall;

	DECREMENT(attackLock);
	DECREMENT(attackGpsBall);
	DECREMENT(grabTime);
	DECREMENT(activeLocaliseTimer);
	DECREMENT(disallowActiveLocalise);
	DECREMENT(grabInterval);
	DECREMENT(lock3rdClosest);
	DECREMENT(findBallKount);
	DECREMENT(findBallDirectionOverrideDelay);

	decrementBackOff();
	utilDetermineDKD();

	// determine which ball values to use

	if (vBall.cf
			<3
			&& lostBall
			> vBallLost
			&& gps->shareBallvar
			< LARGE_VAL
			&& gps->tmObj(gps->sbRobotNum
				- 1).playerType
			== FORWARD
			&& gps->tmObj(gps->sbRobotNum
				- 1).ballDist
			<= (unsigned int)
			35
			&& gps->teammate(gps->sbRobotNum).posVar
			<= get95CF(100) 
			/*&& gps->wmTeammate[gps->sbRobotNum-1].counter > 0*/) {
		ballSource = TEAMMATE_HAS_BALL;

		ballX = gps->teammate(gps->sbRobotNum, 'g').pos.x;
		ballY = gps->teammate(gps->sbRobotNum, 'g').pos.y;
		ballH = gps->teammate(gps->sbRobotNum).pos.head;
		ballD = gps->teammate(gps->sbRobotNum).pos.d;

		maverickWirelessBallDistThreshold = MAX(wirelessBallMinThreshold,
				2 * sqrt(gps->teammate(gps->sbRobotNum).posVar));

		if (maverickDebug) {
			cout << "****************************" << endl;
			cout << "team mate has ball" << endl;
		}

		//if (!ericSilence) leds(1,1,2,1,1,2);
		anotherCounter++;
	} 
	//this will probably trigger on goalie sending the wireless ball
	else if (vBall.cf <3
			&& lostBall> vBallLost
			&& gps->shareBallvar < LARGE_VAL
			&& gps->teammate(gps->sbRobotNum).posVar <= get95CF(100)) {
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
		maverickWirelessBallDistThreshold = MAX(wirelessBallMinThreshold,
				2 * sqrt(gps->teammate(gps->sbRobotNum).posVar));

		if (maverickDebug) {
			cout << "****************************" << endl;
			cout << "using wireless ball" << endl;
		}

		//if (!ericSilence) leds(1,2,1,1,2,1);
		anotherCounter++;
	}
	else {
		ballSource = VISION_BALL;
		if (vBall.cf < 3 && lostBall < vBallLost) {
			ballX = gps->getBall(GLOBAL).x;
			ballY = gps->getBall(GLOBAL).y;
			ballH = gps->getBall(LOCAL).head;
			ballD = gps->getBall(LOCAL).d;

			if (maverickDebug) {
				cout << "****************************" << endl;
				cout << "using gps ball" << endl;
			}
			anotherCounter++;
		}
		else {
			ballX = vBall.x;
			ballY = vBall.y;
			ballH = vBall.h;
			ballD = vBall.d;

			if (maverickDebug) {
				cout << "****************************" << endl;
				cout << "using vision ball" << endl;
			}

			if (vBall.cf < 3)
				anotherCounter++;
			else
				anotherCounter = 0;
		}
		//if (!ericSilence) leds(2,1,1,2,1,1);
		maverickWirelessBallDistThreshold = wirelessBallMinThreshold; //?? does this make sense for gps ball?
	}

	if (maverickWirelessBallDistThreshold < storeWirelessBallDistThreshold) {
		maverickWirelessBallDistThreshold = storeWirelessBallDistThreshold;
	}

	/*
	   if (ballSource == WIRELESS_BALL) {
//cout << "*** wireless ball     " << ballX << " , " << ballY << " \t\t95% range " << maverickWirelessBallDistThreshold << " \t\thead dist " << ballH << " , " << ballD << endl;
cout << "**** wireless *****" << endl;
cout << "myself  " << gps->self().pos.x << ",\t\t" << gps->self().pos.y << ",\t\t" << gps->self().h << endl;
cout << "ball    " << ballX << ",\t\t" << ballY << ",\t\t" << ballD << ",\t\t" << ballH << endl;

} else if (ballSource == TEAMMATE_HAS_BALL) {
//cout << "### teammate location " << ballX << " , " << ballY << " \t\t95% range " << maverickWirelessBallDistThreshold << " \t\thead dist " << ballH << " , " << ballD << endl;
cout << "!!!! teammate !!!!!" << endl;
cout << "myself  " << gps->self().pos.x << ",\t\t" << gps->self().pos.y << ",\t\t" << gps->self().h << endl;
cout << "ball    " << ballX << ",\t\t" << ballY << ",\t\t" << ballD << ",\t\t" << ballH << endl;
}
*/
if (anotherCounter > 10000) {
	anotherCounter = 0;
}
}

/////////////////////////////////////////////////
// MAIN DECISION TREE
/////////////////////////////////////////////////

void Maverick::chooseMaverickForwardStrategy() {
	setWalkParams();

	const Vector &opp = gps->getClosestOppPos(LOCAL);

	wireless = IND_LED2_OFF;

	if (mode_ == mStartMatch) {
		//saBallTracking();
		trackBallHeadParams();
	}
	else if (lockMode != NoMode && lockMode != ResetMode) {
		// execute atomic action if in locked mode
		//saBallTracking();
		trackBallHeadParams();
		switch (lockMode) {
			case ChestPush: aaChestPush();
					gps->r.ballDist = 0;
					break;
			case SpinKick: aaSpinKick();
				       gps->r.ballDist = 0;
				       break;
			case SafeSpinKick: aaSafeSpinKick();
					   gps->r.ballDist = 0;
					   break;
			case SpinChestPush: aaSpinChestPush();
					    gps->r.ballDist = 0;
					    break;
			case GoalieKick: aaGoalieKick();
					 gps->r.ballDist = 0;
					 break;
			case LightningKick: aaLightningKick();
					    gps->r.ballDist = 0;
					    break;
			case ProperVariableTurnKick: aaProperVariableTurnKick();
						     gps->r.ballDist = 0;
						     break;
		}
	}
	else {
		//leds(2,2,2,2,2,2);
		if (backOffEyes)
			barLed = indLEDOFF;
		barLed = indLEDOFF;

		//if (!ericSilence) leds(1,1,1,1,1,1);
		//saBallTracking();
		gps->currentlyCarrying = false;
		trackBallHeadParams();

		//used to test the 3rd player
		//by making the other players just stand still and look at the ball

		//start improved findball
#if 0

		if (PLAYER_NUMBER!=3) {

			/*
			   if (vBall.cf >= 3) {    
			   double h = NormalizeAngle_0_360(RAD2DEG(atan2(vBall.y-gps->self().pos.y, vBall.x-gps->self().pos.x)));
			   hoverToTargetHeadingConsiderBall(rightGoalBoxEdgex, rightGoalBoxEdgey, h, vBall.h);
			   } else {
			   double h = NormalizeAngle_0_360(RAD2DEG(atan2(rightGoalBoxEdgey-gps->self().pos.y, rightGoalBoxEdgex-gps->self().pos.x)));
			   hoverToTargetHeadingConsiderBall(rightGoalBoxEdgex, rightGoalBoxEdgey, h, 0);
			   }
			   */

			if (findBallKount > 0) {
				improvedSpinFindBall(lastTCCW < 0 ? ANTICLOCKWISE : CLOCKWISE);
				/*
				   setOmniParams();
				   leds(2,2,2,2,2,2);
				   turnCCW = lastTCCW;
				   if (lastTCCW < 0) {
				   left = 3;
				   } else {
				   left = -3;
				   }
				   forward = 0;
				   */
				//left = 0;            
				//locateBall(145,220);
				return;
			}

			firePawKick(FIRE_PAWKICK_AUTO, 13);

			if (activeLocaliseTimer) {
				activeGpsLocalise(false);
				if (activeLocaliseTimer == 1) {
					disallowActiveLocalise = 70;
				}
			} else if (!disallowActiveLocalise) {
				setNewBeacons(LARGE_VAL, 50);
				activeGpsLocalise(false);
				activeLocaliseTimer = 10;
			} else 

				if (findBallDirectionOverrideDelay <= 0) {
					setLocateBallParams();
				}
			if (vBall.cf < 3 && lostBall > 20) {
				headFindBall();
				locateBall();
			}  else if (vBall.cf < 3 && lostBall <= 20) {
				gpsTrackBallHeadParams();
				locateBall();
			}
			else {
				trackBallHeadParams();
			}

			if (vBall.cf>=3 && vBall.d < 18 && abs(vBall.h)>70 && findBallDirectionOverrideDelay <= 0) {
				cout << "***********************" << endl;
				findBallKount = 30;//this should be just long enough to break vision contact with the ball. Will NOT see the ball during this time.
				findBallDirectionOverrideDelay = 100;//this should be just long enough to realistically turn the dog back to the ball. Will see the ball during this time. 
				leds(2,2,2,2,2,2);
				lastTCCW = (vBall.h > 0 ? 50 : -50);
				improvedSpinFindBall(lastTCCW < 0 ? ANTICLOCKWISE : CLOCKWISE);
				/*
				   if (vBall.h > 0) {
				   setOmniParams();
				   turnCCW = -50;
				   forward = 0;
				   left = 3;
				   lastTCCW = -50;
				   }
				   else {
				   turnCCW = 50;
				   forward = 0;
				   left = -3;
				   lastTCCW = 50;
				   }
				   */
			}

			if (vBall.cf >=3) {
				cout << "ball d, h, tilty, panx " << vBall.d << " , " << vBall.h << " , " << tilty << " , " << panx << endl;
			}

			return;
		}
#endif
		//end improved findball



		if (activeLocaliseTimer) {
			//if (!ericSilence) leds(2, 1, 2, 2, 1, 2);
			activeGpsLocalise(false);

			hoverToBall(gps->getBall(LOCAL).d, gps->getBall(LOCAL).head);

			if (maverickDebug) {
				cout << "active localise" << endl;
			}
			if (activeLocaliseTimer == 1) {
				if (ballSource == WIRELESS_BALL) {
					disallowActiveLocalise = 30;
				}
				else {
					disallowActiveLocalise = 40;
				}
			}
			return;
		}

		if (is3rdClosest()
				&& (ballSource != VISION_BALL || vBall.cf > 3 || lostBall < vBallLost) /*|| lock3rdClosest > 50*/) {
			//tried to put get visual backoff before third player, but then it visual backoffs too much
			/*
			   if (utilNeedVisualBackoff() || utilNeedGpsBackoff()) {
			   if ((lostBall > vBallLost || vBall.d > 80) && !disallowActiveLocalise) {
			   setNewBeacons(LARGE_VAL, 50);
			   activeGpsLocalise(false);
			   activeLocaliseTimer = 10;
			   } else if (vBall.cf > 3) {
			   trackBallHeadParams();
			   } else if (lostBall <= vBallLost) {
			   gpsTrackBallHeadParams();
			   } else {
			   headFindBall(-30);
			   }  

			   if (ballSource == TEAMMATE_HAS_BALL || ballSource == WIRELESS_BALL) {
			   calcBackOffPosition(ballX, ballY);
			   } else {
			//if you haven't seen the ball then this spot is gonna be pretty random!
			calcBackOffPosition(gps->getBall(GLOBAL).x, gps->getBall(GLOBAL).y);
			}
			return;
			}
			*/
			gps->r.amThirdPlayer = 1;
			if (thirdPlayerEyes)
				leds(2, 2, 2, 2, 2, 2);
			choose3rdPlayerStrategy();
			if (maverickDebug) {
				cout << "3rd player" << endl;
			}
			return;
		}
		gps->r.amThirdPlayer = 0;    

		if (utilNeedVisualBackoff() || utilNeedGpsBackoff()) {
			framesSinceBO = 0;

			//do i need an active localise whilst in visual backoff state?

			if ((lostBall > vBallLost || vBall.d > 80) && !disallowActiveLocalise) {
				setNewBeacons(LARGE_VAL, 50);
				activeGpsLocalise(false);
				activeLocaliseTimer = 10;
			}
			else if (vBall.cf > 3) {
				trackBallHeadParams();
			}
			else if (lostBall <= vBallLost) {
				gpsTrackBallHeadParams();
			}
			else {
				headFindBall(-30);
			}  

			if (ballSource == TEAMMATE_HAS_BALL || ballSource == WIRELESS_BALL) {
				chooseBackOffStrategy(ballX, ballY);
			}
			else {
				//if you haven't seen the ball then this spot is gonna be pretty random!
				chooseBackOffStrategy(gps->getBall(GLOBAL).x, gps->getBall(GLOBAL).y);
			}

			/*
			//reset this thing.. because if you cant see the ball, the ball counter will keep going up
			//whilst your doing backoff, and if you go into the find ball routine, the counter will be 
			//up to a point that your not really up to
			if (lostBall > vBallLost + 1) {
			lostBall = vBallLost + 1;
			}
			*/

			if (maverickDebug) {
				cout << "back off" << endl;
			}

			return;
		}

		//when i execute the backoff strat, i remember the last thing i told it for 5 frames then i reset it.
		if (framesSinceBO < 5) {
			framesSinceBO++;
		}
		else {
			lastBackOffStrategy = BO_NONE;
		}

		if (ballSource == VISION_BALL && vBall.cf < 3) {
			if (lostBall <= vBallLost) {
				//if (!ericSilence) leds(1,2,1,1,2,1);
				gpsTrackBallHeadParams();
				hoverToBall(gps->getBall(LOCAL).d, gps->getBall(LOCAL).head);
				if (maverickDebug) {
					cout << "goto gps ball" << endl;
				}

				return;
			}
			else {
				//if (!ericSilence) leds(1,2,1,1,2,1);
				// first time in series
				if (lostBall == vBallLost + 1 && findBallDirectionOverrideDelay <= 0) {
					setLocateBallParams();
				}
				if (findBallDirectionOverrideDelay > 0) {
					locateBall();
				}
				else {
					locateBall(findPositionx, findPositiony);
				}
				if (maverickDebug) {
					cout << "vision findball" << endl;
				}

				return;
			}
			return;
		}

		if (ballSource == WIRELESS_BALL || ballSource == TEAMMATE_HAS_BALL) {
			if (ballD < maverickWirelessBallDistThreshold) {
				//if (!ericSilence) leds(1,2,1,1,1,1);
				//if (!ericSilence) leds(2,0,0,1,0,0);

				//spinFindBall(ANTICLOCKWISE);


				//if first time you fell into this state
				if (ballSource == WIRELESS_BALL) {
					if (storeWirelessBallDistThreshold == 0
							&& findBallDirectionOverrideDelay <= 0) {
						setLocateBallParams(); //this should only happen once ...
						//cout << "Setting ball params for wireless find ball\n";
						storeWirelessBallDistThreshold = maverickWirelessBallDistThreshold
							+ 50;
						lostBall = vBallLost + 2;
					}
					if (maverickDebug) {
						cout << "find ball within wireless position" << endl;
					}

					if (findBallDirectionOverrideDelay > 0) {
						locateBall();
					}
					else {
						locateBall(ballX, ballY);
					}
				}
				else {
					if (storeWirelessBallDistThreshold == 0) {
						storeWirelessBallDistThreshold = maverickWirelessBallDistThreshold
							+ 50;
					}
					if (maverickDebug) {
						cout << "spin head around whilst backing off" << endl;
					}

					calcBackOffPosition(ballX, ballY); //ballX and ballY are really the teammates coordinates
					headFindBall();
				}

				//maverickWirelessBallDistThreshold = maverickWirelessBallDistThreshold + 50;
				return;
			}
			else if (/*and ball not under y our chin*/ !disallowActiveLocalise) {
				//if (!ericSilence) leds(1,0,0,2,0,0);
				setNewBeacons(LARGE_VAL, 50);
				activeGpsLocalise(false);
				activeLocaliseTimer = 10;
				hoverToBall(ballD, ballH);
				if (maverickDebug) {
					cout << "active localise" << endl;
				}

				return;
			}
			else {
				//if (!ericSilence) leds(1,0,0,2,0,0);
				//maverickWirelessBallDistThreshold = wirelessBallMinThreshold;
				storeWirelessBallDistThreshold = 0;
				//if (!ericSilence) leds(1,1,1,1,2,1);

				/*
				   headFindBall();
				   hoverToBall(ballD, ballH);
				   return;
				   */

				if (ballSource == TEAMMATE_HAS_BALL) {
					headFindBall();
					calcBackOffPosition(ballX, ballY); //ballX and ballY are really the teammates coordinates

					if (maverickDebug) {
						cout << "spin head around whilst backing off" << endl;
					}
				}
				else {
					headFindBall();
					hoverToBall(ballD, ballH);

					if (maverickDebug) {
						cout << "spin head whilst moving to team mate" << endl;
					}

					return;
				}
			}
			return;
		}

		//you probably want to make it that your still allowed to actively localise when your tracking wireless ball
		if (/*and ball not under your chin*/ ballSource == VISION_BALL
				&& vBall.d > 70
				&& ABS(vBall.h) < 5 /*&& opp.d > 20*/
				&& !disallowActiveLocalise) {
			setNewBeacons(LARGE_VAL, 50);
			activeGpsLocalise(false);
			activeLocaliseTimer = 8;

			walkType = ZoidalWalkWT;
			forward = MAX_OFFSET_FORWARD;
			left = 0;
			turnCCW = 0;

			if (maverickDebug) {
				cout << "active localise" << endl;
			}

			return;
		}

		if (maverickDebug) {
			cout << "maverick attack ball" << endl;
		}
		maverickAttackBall();
	}
}





/////////////////////////////////////////////////
// START MAVERICK ATTACK FUNCTIONS
/////////////////////////////////////////////////

void Maverick::maverickAttackBall() {
	//saBallTracking();
	trackBallHeadParams();



	const Vector &opp = gps->getClosestOppPos(LOCAL);
	double reqAccuracy = requiredAccuracy(25);

	//local coords
	double minGoalHead = gps->tGoal().pos.head - reqAccuracy;
	double maxGoalHead = gps->tGoal().pos.head + reqAccuracy;

	//////////////////////////////////////////////////////////////////////////////////////////
	// GOT BALL TRIGGER
	//////////////////////////////////////////////////////////////////////////////////////////

	// if ball under chin needs work
#ifdef FIREBALL
	double ballx = sin(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
	double bally = cos(DEG2RAD(vBall.vob->h)) * vBall.vob->d;

	// NEEDS WORK,
	// basically, I want the head not pan when it is on the ball
	// so when the ball is within a certain distance, then just dont pan
	// since the robot will still turn to adjust to the ball appropriately
	//    if (abs(ballx) <= 3.0 && bally <= 1.5 * BALL_RADIUS && hTilt <= -45) { // 24/30 <- gives the best kicks
	//    if (abs(ballx) <= 3.0 && bally <= 1.5 * BALL_RADIUS) { // 27/30
	//    if (abs(ballx) <= 3.0 && bally <= 1.0 * BALL_RADIUS) { // 13/15

	// this one scored 29/30, it isnt perfect with all the kicks, but it's a compromise
	if ((abs(ballx) <= 3.0 && bally <= 2.0 * BALL_RADIUS)
			|| (hTilt <= -30 && abs(ballx) <= 3.0 && bally <= 3.0 * BALL_RADIUS)) {
		panx = 0;
	}
	if (abs(ballx) <= 3.0 && bally <= 1.5 * BALL_RADIUS && hTilt <= -40) {
		chooseMaverickShootStrategy();
		return;
	}
#else
	if (tilty < -60.0 && ABS(panx) < 15.0) {
		chooseMaverickShootStrategy();
		return;
	}
#endif

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
		if (fireBallLights)
			leds(1, 2, 1, 1, 2, 1);
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
		if (fireBallLights)
			leds(1, 2, 1, 1, 2, 1);
		firePawKick(FIRE_PAWKICK_RIGHT);
		return;
	} 

	// pawkick top edge right half
	if (ballSource == VISION_BALL
			&& utilBallOnTEdge()
			&& utilIsOnTEdge()
			&& gps->self().h > 150
			&& gps->self().h < 180
			&& gps->self().posVar < get95CF(75)
			&& abs(vBall.h) < 30
			&& gps->self().pos.x > FIELD_WIDTH / 2) {
		if (!ericSilence)
			leds(2, 1, 1, 1, 1, 2);
		if (fireBallLights)
			leds(1, 2, 1, 1, 2, 1);
		firePawKick(FIRE_PAWKICK_AUTO);
		return;
	}
	// pawkick top edge left half
	if (ballSource == VISION_BALL
			&& utilBallOnTEdge()
			&& utilIsOnTEdge()
			&& (gps->self().h > 0 && gps->self().h < 30)
			&& gps->self().posVar < get95CF(75)
			&& abs(vBall.h) < 30
			&& gps->self().pos.x < FIELD_WIDTH / 2) {
		if (fireBallLights)
			leds(1, 2, 1, 1, 2, 1);
		firePawKick(FIRE_PAWKICK_AUTO);
		return;
	}

	// pawkick bottom left half
	if (ballSource == VISION_BALL
			&& utilBallOnBEdge()
			&& utilIsOnBEdge()
			&& gps->self().h > 150
			&& gps->self().h < 180
			&& gps->self().posVar < get95CF(75)
			&& abs(vBall.h) < 30
			&& gps->self().pos.x < FIELD_WIDTH / 2) {
		if (fireBallLights)
			leds(1, 2, 1, 1, 2, 1);
		firePawKick(FIRE_PAWKICK_AUTO);
		return;
	}
	// pawkick bottom right half
	if (ballSource == VISION_BALL
			&& utilBallOnBEdge()
			&& utilIsOnBEdge()
			&& (gps->self().h > 0 && gps->self().h < 30)
			&& gps->self().posVar < get95CF(75)
			&& abs(vBall.h) < 30
			&& gps->self().pos.x > FIELD_WIDTH / 2) {
		if (fireBallLights)
			leds(1, 2, 1, 1, 2, 1);
		firePawKick(FIRE_PAWKICK_AUTO);
		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// GET BEHIND BALL CASES
	//////////////////////////////////////////////////////////////////////////////////////////

	//if the ball is on one of the side edges and your not facing the right way and theres oppponents near
	if (ballSource == VISION_BALL /*&& vBall.d > 15 */
			&& vBall.d < 50
			&& utilBallOnLREdge()
			&& opp.d < 40
			&& abs(opp.head) < 70
			&& (gps->self().h >= 110 || gps->self().h <= 70)) {
		saFarGetBehindBall(prevAttackMode != GetBehindBall, 90);

		//this is a hack required because the dog gets behind the ball already 
		//and still trys to get to the ball resulting in it walkin backwards
		if (forward >= 0) {
			return;
			//else fall through and execute the rest of the decision tree
			//hoverToBall(vBall.d, vBall.h);
		}
	} 
	//if the ball is on one of the side edges and your not facing the right way
	if (ballSource == VISION_BALL /*&& vBall.d > 15 */
			&& vBall.d < 50
			&& utilBallOnLREdge()
			&& (gps->self().h >= 180 || gps->self().h <= 0)) {
		saFarGetBehindBall(prevAttackMode != GetBehindBall);

		//this is a hack required because the dog gets behind the ball already 
		//and still trys to get to the ball resulting in it walkin backwards
		if (forward >= 0) {
			return;
			//hoverToBall(vBall.d, vBall.h);
		}
	} 

	/*
	//If the ball is on the top edge, get behind ball
	if ( ballSource == VISION_BALL && vBall.d < 50 && utilBallOnTEdge()) {
	double towardsFromTargetGoal = RAD2DEG(atan2(FIELD_LENGTH-vBall.y, (FIELD_WIDTH/2.0)-vBall.x));

	saFarGetBehindBall(prevAttackMode != GetBehindBall, towardsFromTargetGoal);

	//this is a hack required because the dog gets behind the ball already 
	//and still trys to get to the ball resulting in it walkin backwards
	if (forward >= 0) {
	return;
	//hoverToBall(vBall.d, vBall.h);
	}
	}
	*/

	//if the ball is in a compromising position (or on bottom edge), get behind ball so that you face away from your own goal
	/*
	   if (
	   ( (ballSource == VISION_BALL && vBall.d < 50 && abs(vBall.h) < 45 && opp.d < 50 && abs(opp.head) < 70) //if ball in compromising position
	   || (ballSource == VISION_BALL && vBall.d < 50 && utilBallOnBEdge()) // if the ball is on the bottom edge
	   ) 
	   &&
	   ( (prevAttackMode != GetBehindBall && gps->self().h > 200 && gps->self().h < 340) //you have to be facing your own goal to start the get behind ball defence
	   || (prevAttackMode == GetBehindBall) //if you have already started you can keep going
	   )   
	   ) {
	   */        
	if ((ballSource == VISION_BALL && vBall.d < 30 && utilBallOnBEdge())
			&& (gps->self().h > 200 && gps->self().h < 340)) {
		//double awayFromGoal = RAD2DEG(atan2(vBall.x-(FIELD_WIDTH/2.0) , vBall.y-0));
		double awayFromOwnGoal = RAD2DEG(atan2(vBall.y - 0,
					vBall.x - (FIELD_WIDTH / 2.0)));
		if (!ericSilence)
			cout << "away from goal is " << awayFromOwnGoal << endl;
		saFarGetBehindBall(prevAttackMode != GetBehindBall, awayFromOwnGoal);

		//this is a hack required because the dog gets behind the ball already 
		//and still trys to get to the ball resulting in it walkin backwards
		if (forward >= 0) {
			return;
			//hoverToBall(vBall.d, vBall.h);
		}
	} 

	//////////////////////////////////////////////////////////////////////////////////////////
	// AVOID SCRUM CASES
	//////////////////////////////////////////////////////////////////////////////////////////

#if 0
	//if opponent is in the way walk around him
	if (ballSource == VISION_BALL && opp.d < 50 && (opp.d + 6) < vBall.d &&  abs(opp.head) < abs(RAD2DEG(atan2(30,opp.d))) ) {
		/*
		   cout << "*************************" << endl;
		   cout << "opp d  " << opp.d << endl;
		   cout << "ball d " << vBall.d << endl;
		   cout << "opp head " << abs(opp.head) << endl;
		   cout << "req head " << abs(RAD2DEG(atan2(30,opp.d))) << endl;
		   */

		double xoffset = RAD2DEG(atan2(100,opp.d)); //default positive
		//check cases to make it negative
		//if facing own goal
		if (gps->self().h>180) {
			//go around him goal side
			if (gps->oGoal().pos.head <0) {
				xoffset *= -1.0;
			}
		} else {
			if (opp.head >=0) { //if hes closer to your left
				xoffset = -xoffset; //go to your right
			}
		}

		chargeBall = true;
		avoidOpponent = true;
		hoverToBall(opp.d, xoffset);
		chargeBall = false;        
		avoidOpponent = false;
		turnCCW = CLIP (vBall.h/2.0, 40.0);
		return;
	}
#endif

	//////////////////////////////////////////////////////////////////////////////////////////
	// OTHER PAW KICK CASES
	//////////////////////////////////////////////////////////////////////////////////////////


	double reqAcc = variableRequiredAccuracy(ballX, ballY, 30);
	double ballToGoalHead = RAD2DEG(atan2(abs(FIELD_LENGTH - ballY),
				abs(FIELD_WIDTH / 2 - ballX)));

	//if you get up to here you should be seeing the ball
	if (((ballY
					>= leftGoalBoxEdgey
					&& ballX
					>= leftGoalBoxEdgex
					&& ballX
					<= rightGoalBoxEdgex
					&&  // ball is in goalie box
					gps->self().h
					<= (ballToGoalHead + reqAcc)
					&& gps->self().h
					>= (ballToGoalHead - reqAcc) //you'll get it in if you hit it
	     )
				|| (ballY
					>= leftGoalBoxEdgey
					&& (ballX <WALL_THICKNESS + 25
						|| ballX> FIELD_WIDTH - WALL_THICKNESS - 25)) // ball in corner
	    )
			&& (ballD > 40 || ballH < 45) //your lined up to paw kick the ball?
	   ) {
		leds(2, 2, 2, 2, 2, 2);
		firePawKick(FIRE_PAWKICK_AUTO);
		return;
	}

	double minDistBetweenBallOpp = minDistOppFromBall(ballX, ballY);

	if ((ballD > 40 || ballH < 45)
			&&  //your lined up to paw kick the ball?
			(minDistBetweenBallOpp < 50 || opp.d < 50)
			&&  //ball or you not in the clear
			gps->self().h > 30
			&& gps->self().h < 150 //your facing kinda up field
	   ) {
		leds(2, 2, 2, 2, 2, 2);
		firePawKick(FIRE_PAWKICK_AUTO);
		return;
	}        

	//this used to be the charge condition
	if (gps->self().h > 0
			&& gps->self().h <180
			&& vBall.cf> 0
			&& allowedToCharge) {
		leds(2, 2, 2, 2, 2, 2);
		firePawKick(FIRE_PAWKICK_AUTO);
		return;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// DEFAULT CASE
	//////////////////////////////////////////////////////////////////////////////////////////

	if (fireBallLights) {
		leds(1, 1, 2, 1, 1, 2);
	}
	hoverToBall(vBall.d, vBall.h);
}


void Maverick::chooseMaverickShootStrategy() {
	//leds(2,1,2,2,1,2);
	double absoluteShootRadius = FIELD_LENGTH / 2.0;
	double faceTargetGoalMargin = 30;
	double chestPushRange = FIELD_LENGTH - WALL_THICKNESS - 30;
	double closestOppNearDistance = 25;

	double reqAccuracy = requiredAccuracy(25);

	//local coords
	double minGoalHead = gps->tGoal().pos.head - reqAccuracy;
	double maxGoalHead = gps->tGoal().pos.head + reqAccuracy;

	double min = MIN(abs(minGoalHead), abs(maxGoalHead));
	double max = MAX(abs(minGoalHead), abs(maxGoalHead));

	grabTime = 65;
	const Vector &opp = gps->getClosestOppPos(LOCAL);

	if (((gps->self().h <30 || gps->self().h> 330)
				&& gps->self().pos.x > (FIELD_WIDTH - WALL_THICKNESS - 20))
			|| (vision->facingFieldEdge && gps->self().pos.x > FIELD_WIDTH / 2.0 + 20)) {
		lockMode = ProperVariableTurnKick;
		setProperVariableTurnKick(90);
		aaProperVariableTurnKick();
		leds(2, 1, 1, 2, 1, 1);
		return;
	}
	if ((gps->self().h > 150
				&& gps->self().h < 210
				&& gps->self().pos.x < WALL_THICKNESS + 20)
			|| (vision->facingFieldEdge && gps->self().pos.x < FIELD_WIDTH / 2.0 - 20)) {
		lockMode = ProperVariableTurnKick;
		setProperVariableTurnKick(-90);    
		aaProperVariableTurnKick();
		leds(2, 1, 1, 2, 1, 1);
		return;
	}
	//probably want to specify the other walls to but ahh..

	//if (gps->tGoal().pos.d <= absoluteShootRadius) {
	if (gps->self().pos.y > FIELD_LENGTH / 2) {
		if (gps->self().pos.y
				>= (FIELD_LENGTH - WALL_THICKNESS - 20)
				&& (gps->self().pos.x <leftGoalBoxEdgex + 10
					|| gps->self().pos.x> rightGoalBoxEdgex - 10)) {
			if (vision->facingFieldEdge) {
				double dir;
				if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
					dir = -90;
				}
				else {
					dir = 90;
				}
				leds(2, 1, 1, 2, 1, 1);
				lockMode = ProperVariableTurnKick;
				setProperVariableTurnKick(dir);
				aaProperVariableTurnKick();
			}
			else {
				lockMode = SpinChestPush;
				aaSpinChestPush();
			}
			return;
		}

		//if already lined up with goal lightning
		if (maxGoalHead >= 0 && minGoalHead <= 0) {
			//check if your looking straight at the side wall..
			//if so your gps is probably wrong
			if (vision->facingFieldEdge) {
				double dir;
				if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
					dir = -90;
				}
				else {
					dir = 90;
				}
				leds(2, 1, 1, 2, 1, 1);
				lockMode = ProperVariableTurnKick;
				setProperVariableTurnKick(dir);
				aaProperVariableTurnKick();
			}
			else {
				lockMode = LightningKick;
				aaLightningKick();
			}
			return;
		}


		//
		//past this point both goal posts must be on the same side
		//

		//out of turn kick range do spin front kick
		if (max <= 65) {
			lockMode = SafeSpinKick;
			aaSafeSpinKick();
			return;
		} 

		if (vision->facingFieldEdge) {
			double dir;
			if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
				dir = -90;
			}
			else {
				dir = 90;
			}
			leds(2, 1, 1, 2, 1, 1);
			lockMode = ProperVariableTurnKick;
			setProperVariableTurnKick(dir);
			aaProperVariableTurnKick();
			return;
		}

		//turn kick go for goal
		double dir;
		if (min <= 90 && max >= 90) {
			dir = (gps->tGoal().pos.head > 0 ? 90 : -90);
			leds(1, 2, 1, 1, 2, 1);
		}
		else if (min <= 180 && max >= 180) {
			dir = (gps->tGoal().pos.head > 0 ? 180 : -180);
			leds(1, 1, 2, 1, 1, 2);
			if (dir >= 0 && opp.d <= 40 && opp.head >= 0) {
				barLed = indLEDON;
				dir = dir - 360;
			}
			else if (dir <= 0 && opp.d <= 40 && opp.head <= 0) {
				barLed = indLEDON;
				dir = 360 + dir;
			}
		}
		else {
			if (abs(gps->tGoal().pos.head) < 135) {
				dir = (gps->tGoal().pos.head > 0) ? 90 : -90;
				leds(1, 2, 1, 1, 2, 1);
			}
			else {
				dir = (gps->tGoal().pos.head > 0) ? 180 : -180;
				leds(1, 1, 2, 1, 1, 2);
			}
		}

		lockMode = ProperVariableTurnKick;
		setProperVariableTurnKick(dir);
		aaProperVariableTurnKick();
		return;
	}
	else {
		reqAccuracy = requiredAccuracy(3.0 * FIELD_WIDTH / 2.0);
		//local coords
		minGoalHead = gps->tGoal().pos.head - reqAccuracy;
		maxGoalHead = gps->tGoal().pos.head + reqAccuracy;
		min = MIN(abs(minGoalHead), abs(maxGoalHead));
		max = MAX(abs(minGoalHead), abs(maxGoalHead));        

		if (maxGoalHead >= 0 && minGoalHead <= 0) {
			//check if your looking straight at the side wall..
			//if so your gps is probably wrong
			if (vision->facingFieldEdge) {
				double dir;
				if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
					dir = -90;
				}
				else {
					dir = 90;
				}
				leds(2, 1, 1, 2, 1, 1);                 
				lockMode = ProperVariableTurnKick;
				setProperVariableTurnKick(dir);
				aaProperVariableTurnKick();
			}
			else {
				lockMode = LightningKick;
				aaLightningKick();
			}
			return;
		}    

		//
		//past this point both goal posts must be on the same side
		//

		reqAccuracy = requiredAccuracy(FIELD_WIDTH); //this is smaller, so both goal post should still be on the same side
		//local coords
		minGoalHead = gps->tGoal().pos.head - reqAccuracy;
		maxGoalHead = gps->tGoal().pos.head + reqAccuracy;
		min = MIN(abs(minGoalHead), abs(maxGoalHead));
		max = MAX(abs(minGoalHead), abs(maxGoalHead));        

		double dir;
		if (min <= 90 && max >= 90) {
			dir = (gps->tGoal().pos.head > 0 ? 90 : -90);
			leds(1, 2, 1, 1, 2, 1);
		}
		else if (min <= 180 && max >= 180) {
			dir = (gps->tGoal().pos.head > 0 ? 180 : -180);
			leds(1, 1, 2, 1, 1, 2);
			if (dir >= 0 && opp.d <= 40 && opp.head >= 0) {
				barLed = indLEDON;
				dir = dir - 360;
			}
			else if (dir <= 0 && opp.d <= 40 && opp.head <= 0) {
				barLed = indLEDON;
				dir = 360 + dir;
			}
		}
		else {
			if (abs(gps->tGoal().pos.head) < 125) {
				dir = (gps->tGoal().pos.head > 0) ? 90 : -90;
				leds(1, 2, 1, 1, 2, 1);
			}
			else {
				dir = (gps->tGoal().pos.head > 0) ? 180 : -180;
				if (dir >= 0 && opp.d <= 40 && opp.head >= 0) {
					barLed = indLEDON;
					dir = dir - 360;
				}
				else if (dir <= 0 && opp.d <= 40 && opp.head <= 0) {
					barLed = indLEDON;
					dir = 360 + dir;
				}
				leds(1, 1, 2, 1, 1, 2);
			}
		}

		lockMode = ProperVariableTurnKick;
		setProperVariableTurnKick(dir);
		aaProperVariableTurnKick();
		return;
	}
}



/////////////////////////////////////////////////
// START MAVERICK BACKOFF FUNCTIONS
/////////////////////////////////////////////////

//the inputs are the global position from which you want to back off
void Maverick::chooseBackOffStrategy(double tmx, double tmy) {
	if (lastBackOffStrategy == BO_ATTACK) {
		maverickAttackBall();
		lastBackOffStrategy = BO_ATTACK;
		if (backOffEyes)
			leds(2, 1, 1, 1, 1, 1);
		return;
	}
	else if (lastBackOffStrategy == BO_GETBEHIND) {
		double towardsFromTargetGoal = RAD2DEG(atan2(FIELD_LENGTH - vBall.y,
					(FIELD_WIDTH / 2.0) - vBall.x));
		if (abs(gps->self().h - towardsFromTargetGoal) < 20) {
			calcBackOffPosition(tmx, tmy);
			return;
		}
		saFarGetBehindBall(prevAttackMode != GetBehindBall, towardsFromTargetGoal);
		lastBackOffStrategy = BO_GETBEHIND;
		if (backOffEyes)
			leds(1, 2, 1, 1, 2, 1);
		return;
	}

	for (int i = 1; i <= NUM_TEAM_MEMBER; i++) {
		//for all people infront of you using gps and further than the ball {
		const WMObj &tm = gps->tmObj(i - 1);
		if (i != PLAYER_NUMBER
				&& tm.counter > 0
				&& tm.playerType == FORWARD
				&& gps->teammate(i).posVar <= get95CF(50)
				&& abs(gps->teammate(i).pos.head) < 30
				&& gps->teammate(i).pos.d > gps->getBall(LOCAL).d + 10
				&& gps->getBallMaxVar() < get95CF(50)) {
			//if you can see the ball and your looking up and they are looking down attack ball
			if (vBall.cf >= 3
					&& gps->self().h > 40
					&& gps->self().h <140
					&& gps->teammate(i).h> 220
					&& gps->teammate(i).h < 320) {
				maverickAttackBall();
				lastBackOffStrategy = BO_ATTACK;
				if (backOffEyes)
					leds(2, 1, 1, 2, 1, 1);
				return;
			}

			//if your looking down and they are looking up back off
			if (gps->self().h > 220
					&& gps->self().h <320
					&& gps->teammate(i).h> 40
					&& gps->teammate(i).h < 140
					&& tm.ballDist < 50) {
				calcBackOffPosition(tmx, tmy);
				return;
			}

			//if they are looking pretty straight up or their heading is similar to yours
			if ((gps->teammate(i).h
						> 55
						&& gps->teammate(i).h
						< 125
						|| abs(gps->self().h
							- gps->teammate(i).h)
						< 27)
					&& gps->tmObj(i
						- 1).ballDist
					< 50) {
				if (backOffEyes)
					leds(1, 1, 2, 1, 1, 2);
				calcBackOffPosition(tmx, tmy);
				return;
			}

			/*
			   if ((gps->teammate(i).pos.y < FIELD_LENGTH/5.0) {
			   calcBackOffPosition(tmx, tmy);
			   return;
			   }
			   */

			//else get behind ball
			double towardsFromTargetGoal = RAD2DEG(atan2(FIELD_LENGTH
						- vBall.y,
						(FIELD_WIDTH / 2.0)
						- vBall.x));
			saFarGetBehindBall(prevAttackMode != GetBehindBall,
					towardsFromTargetGoal);
			lastBackOffStrategy = BO_GETBEHIND;
			if (backOffEyes)
				leds(1, 2, 1, 1, 2, 1);
			return;
		}
	}

	//for all people infront of you and further than ball using vision
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		//if your facing upish and your downfield from the ball attack ball
		if (vTeammate[i].var <get95CF(50)
				&& ABS(vTeammate[i].h) < 30
				&& vBall.cf > 3
				&& vBall.d <50
				&& vTeammate[i].d> vBall.d + 10
				&& gps->self().h> 50
				&& gps->self().h < 130) {
			if (backOffEyes)
				leds(2, 1, 1, 2, 1, 1);
			lastBackOffStrategy = BO_ATTACK;
			maverickAttackBall();
			return;
		}
	}            

	calcBackOffPosition(tmx, tmy);
	}

	void Maverick::calcBackOffPosition(double tmx, double tmy) {
		if (backOffEyes) {
			barLed = indLEDON;
		}
		lastBackOffStrategy = BO_BACKOFF;

		static const int xedgeBuffer = WALL_THICKNESS + 30;
		static const int yedgeBuffer = WALL_THICKNESS + 50;
		static const int xoffset = 80;
		static const int yoffset = -47;

		double m = (FIELD_LENGTH - tmy) / ((FIELD_WIDTH / 2.0) - tmx);
		double b = FIELD_LENGTH - m*(FIELD_WIDTH / 2.0);
		double xMatchingYourY = (gps->self().pos.y - b) / m;

		//might want to make him face alittle more towards the goal
		if (gps->self().pos.x <= xMatchingYourY) {
			double x = MAX(xedgeBuffer, tmx - xoffset);
			double y = MIN(FIELD_LENGTH - yedgeBuffer, tmy + yoffset);
			double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy - gps->self().pos.y,
							tmx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}
		else {
			double x = MIN(FIELD_WIDTH - xedgeBuffer, tmx + xoffset);
			double y = MIN(FIELD_LENGTH - yedgeBuffer, tmy + yoffset);
			double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy - gps->self().pos.y,
							tmx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}    

		if (tmy >= (3 * FIELD_LENGTH / 4.0)) {
			if (gps->self().pos.x <= xMatchingYourY) {
				double x = MAX(xedgeBuffer, tmx - 80);
				double y = MIN(FIELD_LENGTH - yedgeBuffer, tmy - 80);
				double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy - gps->self().pos.y,
								tmx - gps->self().pos.x)));
				saGoToTargetFacingHeading(x, y, h);
			}
			else {
				double x = MIN(FIELD_WIDTH - xedgeBuffer, tmx + 80);
				double y = MIN(FIELD_LENGTH - yedgeBuffer, tmy - 80);
				double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy - gps->self().pos.y,
								tmx - gps->self().pos.x)));
				saGoToTargetFacingHeading(x, y, h);
			}
		}

		if (tmy <= (FIELD_LENGTH / 4.0)) {
			if (gps->self().pos.x <= xMatchingYourY) {
				double x = MAX(xedgeBuffer, tmx - 80);
				double y = MAX(yedgeBuffer, tmy + 80);
				double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy - gps->self().pos.y,
								tmx - gps->self().pos.x)));
				saGoToTargetFacingHeading(x, y, h);
			}
			else {
				double x = MIN(FIELD_WIDTH - xedgeBuffer, tmx + 80);
				double y = MAX(yedgeBuffer, tmy + 80);
				double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy - gps->self().pos.y,
								tmx - gps->self().pos.x)));
				saGoToTargetFacingHeading(x, y, h);
			}
		}

		/*
		   if (tmy <= (FIELD_LENGTH/4.0)) {
		   if (gps->self().pos.x <= xMatchingYourY && leftGoalBoxEdgex <= (leftGoalBoxEdgey - b)/m ) {
		   double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy-leftGoalBoxEdgey, tmx-leftGoalBoxEdgex)));
//cout << "go to left goal corner"  << endl;
saGoToTargetFacingHeading(leftGoalBoxEdgex, leftGoalBoxEdgey, h);
if (backOffEyes) leds(2,1,2,2,1,2);
}
else if (gps->self().pos.x >= xMatchingYourY && rightGoalBoxEdgex >= (rightGoalBoxEdgey - b)/m) {
double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy-rightGoalBoxEdgey, tmx-rightGoalBoxEdgex)));
//cout << "go to right goal corner"  << endl;
saGoToTargetFacingHeading(rightGoalBoxEdgex, rightGoalBoxEdgey, h);        
if (backOffEyes) leds(2,1,2,2,1,2);
}
}*/
}



/////////////////////////////////////////////////
// START MAVERICK THIRD PLAYER FUNCTIONS
/////////////////////////////////////////////////

bool Maverick::is3rdClosest() {
	if (is3rdClosestDebug) {
		cout << "************is3rdClosest************" << endl;
	}

	int closeRank = 0;

	double myBallx = 0;
	double myBally = 0;

	double myDist = 0;
	//if can see ball
	if (vBall.cf > 3) {
		myDist = vBall.d;
		myBallx = vBall.x;
		myBally = vBall.y;
		if (is3rdClosestDebug)
			cout << "my vision ball d,x,y "
				<< myDist /*<< "," << myBallx << "," << myBally*/ << endl;
	} 
	//if gps ball still good
	else if (gps->getBallMaxVar() <= get95CF(35)) {
		myDist = gps->getBall(LOCAL).d;
		myBallx = gps->getBall(GLOBAL).x;
		myBally = gps->getBall(GLOBAL).y;
		if (is3rdClosestDebug)
			cout << "my gps ball d,x,y "
				<< myDist /*<< "," << myBallx << "," << myBally*/ << endl;
	} 
	//if wireless ball ok
	else if (gps->shareBallvar < LARGE_VAL
			&& gps->teammate(gps->sbRobotNum).posVar <= get95CF(50)) {
		myDist = getDistanceBetween(gps->self().pos.x,
				gps->self().pos.y,
				gps->shareBallx,
				gps->shareBally);
		myBallx = gps->shareBallx;
		myBally = gps->shareBally;
		if (is3rdClosestDebug)
			cout << "my wireless ball d,x,y "
				<< myDist /*<< "," << myBallx << "," << myBally*/ << endl;
	} 
	//else dont know where ball is so cant possibly know im the 3rd closest
	else {
		if (is3rdClosestDebug)
			cout << "dont know where the ball is" << endl;
		return false;
	}

	for (int i = 1; i <= NUM_TEAM_MEMBER; i++) {
		if (i != PLAYER_NUMBER) {
			if (is3rdClosestDebug) {
				cout << "-----------------------------" << endl;
				cout
					<< "-- checking teammate "
					<< i
					<< " counter "
					<< gps->tmObj(i
							- 1).counter
					<< " playerType "
					<< gps->tmObj(i
							- 1).playerType
					<< " posVar "
					<< 2 * sqrt(gps->teammate(i).posVar)
					<< endl;
			}

			if (gps->tmObj(i - 1).counter > 0
					&& gps->tmObj(i - 1).playerType == FORWARD
					&& gps->teammate(i).posVar <= get95CF(50)) {
				//if they are already doing the third player thing and they have a higher player number than you, dont do third player
				if (gps->tmObj(i - 1).amThirdPlayer == 1 && PLAYER_NUMBER < i) {
					return false;
				}

				double theirDist = LOSTBALL_DIST;
				//if they can see the ball
				if (gps->tmObj(i - 1).ballDist < (unsigned int) abs(LOSTBALL_DIST)) {
					theirDist = gps->tmObj(i - 1).ballDist;
					if (is3rdClosestDebug)
						cout << "teammate " << i << " vision ball dist " << theirDist
							<< endl;
					//if (is3rdClosestDebug) cout << "their position " << gps->teammate(i, 'g').pos.x << "," << gps->teammate(i, 'g').pos.y << gps->teammate(i, 'g').h << endl;
				} 
				//if i know where the ball is
				else {
					theirDist = getDistanceBetween(myBallx,
							myBally,
							gps->teammate(i, 'g').pos.x,
							gps->teammate(i, 'g').pos.y);
					if (is3rdClosestDebug)
						cout << "teammate i wireless ball dist " << theirDist << endl;
					//if (is3rdClosestDebug) cout << "their position " << gps->teammate(i, 'g').pos.x << "," << gps->teammate(i, 'g').pos.y << gps->teammate(i, 'g').h << endl;
				}

				// increment counter whenever a teammate is 
				// closer to the ball (if the distances are fairly similar, 
				// if you have the lower player number, still increment the counter)
				double myDistMinusTheirs = myDist - theirDist;
				if (is3rdClosestDebug)
					cout << "myDistMinusTheirs " << myDistMinusTheirs << endl;
				if (myDistMinusTheirs > +20) {
					closeRank++;
					if (is3rdClosestDebug)
						cout << "closeRank " << closeRank
							<< " increasing because they're closer" << endl;
				}
				else if (abs(myDistMinusTheirs) < 20 && PLAYER_NUMBER > i) {
					closeRank++;
					if (is3rdClosestDebug)
						cout << "closeRank " << closeRank
							<< " increasing because playerNumberTieBreak" << endl;
				}
			}
		}
	}    
	if (closeRank >= 2) {
		lock3rdClosest = 20;
		if (is3rdClosestDebug)
			cout << "!!! 3rd closest is TRUE TRUE TRUE" << endl;
		if (is3rdClosestDebug)
			cout << endl << endl;
		return true;
	}
	else if (lock3rdClosest && closeRank >= 1) {
		//ie your not the closest
		if (is3rdClosestDebug)
			cout << "!!! 2nd closest is TRUE TRUE TRUE" << endl;
		if (is3rdClosestDebug)
			cout << endl << endl;
		return true;
	}
	else {
		if (is3rdClosestDebug)
			cout << "!!! 3rd closest is FALSE FALSE FALSE" << endl;
		if (is3rdClosestDebug)
			cout << endl << endl;
		return false;
	}
}

void Maverick::choose3rdPlayerStrategy() {
	/*
	   probably going to need to do some sort of hysteresis or lock type thing so you dont change your mind too much
	   especially when the other dogs are in scrums
	   probably want to avoid obstacles too
	   and keep looking towards the ball as your walking to that position
	   what if you dont know which way the play is going???
	   if the play going towards own goal
	   run up the less crowded side and try to get closer to your own goal than the ball is (y wise) 
	   incase they try and change the side of the play
	   if ball in target half and the play is going towards target goal
	   go near the goal box corner of the side your on??

	   if ball in own goal half and play is going towards target goal
	   run up the less crowded side and try to get closer to the target goal than the ball is (y wise)
	   */
	if ((lostBall > vBallLost || vBall.d > 80) && !disallowActiveLocalise) {
		setNewBeacons(LARGE_VAL, 50);
		activeGpsLocalise(false);
		activeLocaliseTimer = 10;
	}
	else if (vBall.cf > 3) {
		trackBallHeadParams();
	}
	else if (lostBall <= vBallLost) {
		gpsTrackBallHeadParams();
		//HACKY!!!
		ballX = gps->getBall(GLOBAL).x;
		ballY = gps->getBall(GLOBAL).y;
	}
	else {
		headFindBall(-20);
	}    

	if (is3rdUpDownDebug) {
		cout << "********************************" << endl;
	}
	if (is3rdUpDownDebug) {
		cout << "ball x y " << ballX << " , " << ballY;
	}

	//ball in target half
	if (ballY >= FIELD_LENGTH / 2.0) {
		if (is3rdUpDownDebug)
			cout << "do offence " << endl;
		choose3rdPlayerOffence(ballX, ballY);
	} 
	//ball in own half
	else {
		if (is3rdUpDownDebug)
			cout << "do defence " << endl;
		choose3rdPlayerDefence(ballX, ballY);
	}
}

void Maverick::choose3rdPlayerOffence(double tmx, double tmy) {
	if (is3rdPositionDebug) {
		cout << "@@@@@@@@@@@@offense positioning@@@@@@@@@@@@" << endl;
	}    

	static const int xedgeBuffer = WALL_THICKNESS + 30;
	static const int yedgeBuffer = WALL_THICKNESS + 25;

	static const int xoffset = abs(200);
	static const int yoffset = +150;

	double m = (TARGETGOALY - tmy) / (TARGETGOALX - tmx);
	double b = TARGETGOALY - m *TARGETGOALX;
	double xMatchingYourY = (gps->self().pos.y - b) / m;

	if (gps->self().pos.x <= xMatchingYourY) {
		//double x = MAX(xedgeBuffer, tmx-xoffset);
		//double y = MIN(FIELD_LENGTH-yedgeBuffer, tmy+yoffset);
		double x = leftGoalBoxEdgex - 18;
		double y = leftGoalBoxEdgey - 28;
		double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy - gps->self().pos.y,
						tmx - gps->self().pos.x)));
		saGoToTargetFacingHeading(x, y, h);
		if (is3rdPositionDebug) {
			cout << "my pos head " << gps->self().pos.x << " , "
				<< gps->self().pos.y << " , " << gps->self().h << endl;
			cout << "going to pos head " << x << " , " << y << " , " << h << endl;
		}
	}
	else {
		//double x = MIN(FIELD_WIDTH-xedgeBuffer, tmx+xoffset);
		//double y = MIN(FIELD_LENGTH-yedgeBuffer, tmy+yoffset);
		double x = rightGoalBoxEdgex + 18;
		double y = rightGoalBoxEdgey - 28;
		double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy - gps->self().pos.y,
						tmx - gps->self().pos.x)));
		saGoToTargetFacingHeading(x, y, h);
		if (is3rdPositionDebug) {
			cout << "my pos head " << gps->self().pos.x << " , "
				<< gps->self().pos.y << " , " << gps->self().h << endl;
			cout << "going to pos head " << x << " , " << y << " , " << h << endl;
		}
	}
}

void Maverick::choose3rdPlayerDefence(double tmx, double tmy) {
	if (is3rdPositionDebug) {
		cout << "^^^^^^^^^^^^defence positioning^^^^^^^^^^^^" << endl;
	}

	static const int xedgeBuffer = WALL_THICKNESS + 30;
	static const int yedgeBuffer = WALL_THICKNESS + 10;

	static const int xoffset = abs(200);
	static const int yoffset = -100;

	//the action is on the left of the field so go to the right
	if (tmx < (FIELD_WIDTH / 2.0)) {
		//double x = MIN(FIELD_WIDTH-xedgeBuffer, tmx+xoffset);
		//double y = MAX(0+yedgeBuffer, tmy+yoffset);
		double x = FIELD_WIDTH / 2.0 + 80;
		double y = WALL_THICKNESS + 140;
		double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy - gps->self().pos.y,
						tmx - gps->self().pos.x)));
		saGoToTargetFacingHeading(x, y, h);
		if (is3rdPositionDebug) {
			cout << "my pos head " << gps->self().pos.x << " , "
				<< gps->self().pos.y << " , " << gps->self().h << endl;
			cout << "going to pos head " << x << " , " << y << " , " << h << endl;
		}
	} 
	//the action is on the right of the field so go to the left
	else {
		//double x = MAX(xedgeBuffer, tmx-xoffset);
		//double y = MAX(0+yedgeBuffer, tmy+yoffset);
		double x = FIELD_WIDTH / 2.0 - 80;
		double y = WALL_THICKNESS + 140;
		double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy - gps->self().pos.y,
						tmx - gps->self().pos.x)));
		saGoToTargetFacingHeading(x, y, h);
		if (is3rdPositionDebug) {
			cout << "my pos head " << gps->self().pos.x << " , "
				<< gps->self().pos.y << " , " << gps->self().h << endl;
			cout << "going to pos head " << x << " , " << y << " , " << h << endl;
		}
	}
}





/////////////////////////////////////////////////
// START MAVERICK COMMON FUNCTIONS
/////////////////////////////////////////////////

	void Maverick::hoverToBall(double ballDist, double ballHead) {
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


		double maxCanterForwardSpeed = 8;
		double maxCanterLeftSpeed = 6;


		double relH = ballHead;

		//this is a hack do it properly!
		//ie it overrides the ballHead you use as input to this function and uses visual ball instead
		if (avoidOpponent) {
			relH = vBall.h;
		}

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

		// if the ball is at a high angle, walk forward and left appropriately whilst turning
		//if (abs(relH) > 18) {

		if (abs(turnCCW) < 15 && ballDist > 30) {
			setWalkParams();
			walkType = ZoidalWalkWT;
			left = 0;
			forward = MAX_OFFSET_FORWARD;
		}
		else if (abs(relH) > 18) {
			setOmniParams();
			//if (!ericSilence) leds(1,1,2,1,1,1);

			walkType = CanterWalkWT;

			double maxF = maxCanterForwardSpeed;
			double maxL = maxCanterLeftSpeed;

			if (turnCCW < 10 && turnCCW >= 0) {
				maxF = 6;
				maxL = 6;
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
			forward = CLIP(forward, maxCanterForwardSpeed);
		}

		//if (!chargeBall) {
		/*
		   if (ballDist < 16) {
		   walkType = CanterWalkWT;
		   forward = 7.5;
		   left = 0;
		   turnCCW = CLIP(vBall.h/3.0, 20.0);
		   if (ABS(turnCCW) > 15) {
		   forward = 0.5;
		   }
		   }
		   */
		if (ballDist < 25) {
			walkType = CanterWalkWT;
			forward = 7.0;
			left = 0;
			turnCCW = CLIP(vBall.h / 2.0, 30.0);
			if (ABS(turnCCW) > 20 && ballDist < 10) {
				forward = 0.5;
			}
		}
		//}

		if (walkType == CanterWalkWT) {
			//if (!ericSilence) leds(2,1,1,1,1,1);
			if (forward > maxCanterForwardSpeed) {
				forward = maxCanterForwardSpeed;
			}

			if (turnCCW < 10 && turnCCW >= 0) {
				//cout << "clip 10 anti 7.5 , 6" << endl;
				forward = CLIP(forward, 7.5);
				left = CLIP(left, 6.0);
				//maxF=6;
				//maxL=6;
			} 
			//counter clockwise case turning faster
			else if (turnCCW < 20 && turnCCW >= 0) {
				//cout << "clip 20 anti 5.7 , 5" << endl;
				forward = CLIP(forward, 5.7);
				left = CLIP(left, 5.0);
				//maxF=5;
				//maxL=5;
			} 
			//turn too fast anticlockwise case
			else if (turnCCW >= 0) {
				//cout << "clip other anti 5 , 5" << endl;        
				forward = CLIP(forward, 5.0);
				left = CLIP(left, 5.0);
				//maxF=3;
				//maxL=2;
			}

			//slow clockwise turn case
			else if (turnCCW > -10 && turnCCW < 0) {
				//cout << "clip 10 clockwise 5 , 5" << endl;        
				forward = CLIP(forward, 5.0);
				left = CLIP(left, 5.0);
				//maxF=5;
				//maxL=5;
			} 
			//all other clockwise cases
			else {
				//cout << "clip other clockwise 4.3 , 5" << endl;        
				forward = CLIP(forward, 4.3);
				left = CLIP(left, 5.0);
				//maxF=1;
				//maxL=3;
			}
		}
		else {
			//if (!ericSilence) leds(1,1,1,2,1,1);
			if (forward > MAX_OFFSET_FORWARD) {
				forward = MAX_OFFSET_FORWARD;
			}
		}
	}

	static double old_tilt = 0;
	static double old_pan = 0;
#define FOV 10

	void Maverick::trackBallHeadParams() {
		if (vision->vob[vobBall].cf > 0) {
#ifdef FIREBALL
			double ballx = sin(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
			double bally = cos(DEG2RAD(vBall.vob->h)) * vBall.vob->d;

			headtype = ABS_XYZ;
			panx = ballx;
			tilty = BALL_DIAMETER;
			cranez = bally;
#else
			tilty = vision->vob[vobBall].elev;
			panx = vision->vob[vobBall].head;

			if (abs(panx - hPan) > FOV) {
				double factor = 1.0 + ABS(panx - hPan) / 60;
				panx = hPan + (panx - hPan) * factor;
			}

			double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, z6;



			x1 = vBall.d * tan(radians(vBall.imgHead));

			//track middle causes to much head-butts when you try and grab the ball
			//if (vBall.d > 30) {
			//track top
			y1 = vBall.d * tan(radians(PointToElevation(vision->vob[0].misc)));
			//} else {
			//track middle
			//y1 = vBall.d * tan(radians(vBall.imgElev));
			//}
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
#endif
		}
		else {
			tilty = old_tilt;
			panx = old_pan;
		}
	}

	void Maverick::gpsTrackBallHeadParams() {
		headtype = ABS_XYZ;

		panx = -gps->getBall(LOCAL).x;
		tilty = BALL_DIAMETER; //BALL_DIAMETER/2.0;
		cranez = gps->getBall(LOCAL).y;
	}


	//returns the angle (either side) from the point specified to the goal posts
	double Maverick::variableRequiredAccuracy(double tmx,
			double tmy,
			double reqGoalWidth) {
		double globalXoffset = FIELD_WIDTH / 2 - tmx;
		double globalYoffset = FIELD_LENGTH - tmy;

		double poleDistance = sqrt(SQUARE(abs(globalXoffset) - reqGoalWidth)
				+ SQUARE(globalYoffset));
		double globalGoalHead = RAD2DEG(atan2(abs(globalYoffset), abs(globalXoffset)));
		double requiredAccuracy = RAD2DEG(asin(reqGoalWidth *sin(DEG2RAD(globalGoalHead))
					/ poleDistance)); 
		return requiredAccuracy;
	}

	//bx by is global ball
	//return distance between ball and opp closest to ball
	double Maverick::minDistOppFromBall(double bx, double by) {
		double minSoFar = LARGE_VAL;
		for (int i = 0; i < NUM_OPPONENTS; i++) {
			double d = getDistanceBetween(gps->getOppPos(i, GLOBAL).x,
					gps->getOppPos(i, GLOBAL).y,
					bx,
					by);
			if (d < minSoFar) {
				minSoFar = d;
			}
		}
		return minSoFar;
	}

	void Maverick::hoverToTargetHeadingConsiderBall(double targetx,
			double targety,
			double targeth,
			double relBallHead) {
		// variables relative to self localisation
		double relx = targetx - (gps->self().pos.x);
		double rely = targety - (gps->self().pos.y);
		double reld = sqrt(SQUARE(relx) + SQUARE(rely));

		//where you want to walk to
		double relTheta = NormalizeAngle_180(RAD2DEG(atan2(rely, relx))
				- (gps->self().h));
		//which way you want to be facing
		double relh = NormalizeAngle_180(targeth - (gps->self().h));


		if (reld > 70 && abs(relTheta) <= 15 && abs(relBallHead) < 45) {
			leds(2, 1, 2, 2, 1, 2);
			setWalkParams();
			walkType = ZoidalWalkWT;
			left = 0;        
			forward = MAX_OFFSET_FORWARD;
			turnCCW = relTheta;
		}
		else {
			leds(1, 2, 1, 1, 2, 1);

			forward = CLIP(reld, 6.0) * cos(DEG2RAD(relTheta));
			left = CLIP(reld, 6.0) * sin(DEG2RAD(relTheta));

			double move = sqrt(SQUARE(forward) + SQUARE(left));
			double maxTurn = 15;//(move > 4) ? 5 : (9-move);
			turnCCW = CLIP(relh, maxTurn);
		}
	}

	/////////////////////////////////////////////////
	// END MAVERICK FUNCTIONS
	/////////////////////////////////////////////////
