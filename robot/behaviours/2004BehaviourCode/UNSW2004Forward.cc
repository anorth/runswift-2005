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

#include "UNSW2004.h"
#ifdef COMPILE_ALL_CPP

#define INSTRUMENT(x) if (bINSTRUMENTATION ) cout << x

static const bool bINSTRUMENTATION = false;

static const enum strikerEnum {
	oldStriker,
	claudeStriker,
	willStrikerA,
	willStrikerB
} strikerType = willStrikerB;

// own goal coordinates
static const double OWN_GOAL_X = FIELD_WIDTH / 2.0;
static const double OWN_GOAL_Y = 0;

// target goal coordinates
static const double TARGET_GOAL_X = FIELD_WIDTH / 2.0;
static const double TARGET_GOAL_Y = FIELD_LENGTH;

static const double LEFT_GOALBOX_EDGE_X = TARGET_GOAL_X - GOALBOX_WIDTH / 2.0;
static const double LEFT_GOALBOX_EDGE_Y = TARGET_GOAL_Y - WALL_THICKNESS - GOALBOX_DEPTH;

static const double RIGHT_GOALBOX_EDGE_X = TARGET_GOAL_X + GOALBOX_WIDTH / 2.0;
static const double RIGHT_GOALBOX_EDGE_Y = TARGET_GOAL_Y - WALL_THICKNESS - GOALBOX_DEPTH;

// vision ball constants for findball trigger
static const int vBallLost = 10;
static const int vBallAnywhere = 40;

// constants for locking backoff strategies
static const int BO_ATTACK = 0;
static const int BO_GETBEHIND = 1;
static const int BO_BACKOFF = 2;
static const int BO_NONE = 3;

// variables to maintain backoff strategy locks
static int lastBackOffStrategy = BO_NONE; 

// active localise timer
static int activeLocaliseTimer = 0;
static int disallowActiveLocalise = 0;
static const int localiseLockLength = 9;

// variables used in deciding whether or not to charge for the ball
static bool allowedToCharge = false;
static int lastSeenOpponent = 0;

// variables needed to use wireless ball
static const double wirelessBallMinThreshold = 100;
static double UNSW2004WirelessBallDistThreshold = wirelessBallMinThreshold;
static double storeWirelessBallDistThreshold = 0;
static int framesSinceInVarianceCircle = 0;

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

// constants used for ball prediction hovering
static const bool usePrediction = true;
static const double velocityMinPredict = 0.3;
static const double distanceMinPredict = 20.0;
//static const double varianceMaxPredict = get95CF(10.0);
static const double normalHoverAngle = 27.5;
static const double normalHoverSpeed = 3.0;

// StealthDog - lets you turn it off
static const bool useStealth = true;
static int lastStealth = 0;

//number of frames since there was 3 forwards communicating to one another
static int framesSince3ForwardsPresent = 0;

static const int bottomBODist = 60;
static const int BACKOFF_BALL_BOTTOM_EDGE_DISTANCE = bottomBODist + WALL_THICKNESS;

// turns on bird of prey
static const bool doDefence = true;

// turns on the gps teammate matching in backoff
static const bool useTmMatching = true;

// turns on penalty shooter
static const bool doPenaltyShooter = false;

// turns on offTheWall kick
static const bool doOffEdgeKick = false;

//turns on the forwards supporting when someone else says "have ball"
static const bool haveBallSupporting = true;

// turns on the Hail Mary - DUH
static const bool doHailMary = false;

// move striker to something decided last rather than first
static const bool hackStrikerIntoSupport = true;

// setting this true makes sure the bird is not the striker
static const bool birdIsNotStriker = false;

// turns on deliberately going into "find ball" as a kick strategy.
// its not as stupid as it sounds.. 
static const bool enableLocateBallKick = true;

// when turning to face the ball at edge, if turning means knocking ball backwards
// turn the other way
static const bool edgeSpinOverride = false;

static bool roleEyes = false;
#define attackRoleEyes        leds(2,1,1,2,1,1)
#define supportRoleEyes       leds(1,2,1,1,2,1)
#define backoffRoleEyes       leds(2,2,1,2,2,1) //represents the eyes of attack and support
#define inFaceBackoffRoleEyes leds(2,2,2,2,2,2)
#define strikerRoleEyes       leds(1,1,2,1,1,2)
#define haveBallRoleEyes      leds(2,2,2,0,0,0) //you should still be able to see the roles from the eyes on the right
static bool ballSourceEyes = false;
#define visionBallEyes        leds(2,1,1,2,1,1)
#define recentGpsBallEyes     leds(2,2,1,2,2,1) //represents the eyes of vision and gps
#define gpsBallEyes           leds(1,2,1,1,2,1)
#define wirelessBallEyes      leds(1,1,2,1,1,2)
static bool pawKickEyes = false;
#define edgePawKickEyes       leds(2,1,1,2,1,1)
#define goalBoxPawKickEyes    leds(1,2,1,1,2,1)
#define oppNearPawKickEyes    leds(1,1,2,1,1,2)
#define chargePawKickEyes     leds(2,2,2,2,2,2)
static bool backOffEyes = false;
#define forcedAttackBOEyes    leds(2,1,1,2,1,1)
#define getBehindBOEyes       leds(1,2,1,1,2,1)
#define supportBOEyes         leds(1,1,2,1,1,2)
#define sideBOEyes            leds(2,1,2,2,1,2) //when your teammate is beside you
#define edgeBOEyes            leds(2,2,2,2,2,2) //when your teammate is near the edge (the dummy half thing)
static bool tmPosBackOffEyes = false; 
#define tmTopRegionEyes       leds(1,1,2,1,1,2)
#define tmMidRegionEyes       leds(1,2,1,1,2,1)
#define tmBotRegionEyes       leds(2,1,1,2,1,1)
#define myTopRegionEyes       leds(0,0,0,1,1,2)
#define myMidRegionEyes       leds(0,0,0,1,2,1)
#define myBotRegionEyes       leds(0,0,0,2,1,1)
#define gbbStepBackEyes       leds(2,2,2,2,2,2)
#define tmAttackEyes          ;
#define tmGetBehindEyes       ;
#define tmSupportEyes         ;
static bool tmPosBackOffEyesExt = false; 
#define tmExtSupport          leds(2,2,1,2,2,1)
#define tmExtGetBehind        leds(2,1,2,2,1,2)
#define tmExtNotCloseEnough   leds(1,2,2,1,2,2)
static bool wirelessEyes = false;
#define inTheWayEyes          leds(2,1,1,2,1,1)
#define inTheCircleEyes       leds(1,2,1,1,2,1)
#define usingWirelessEyes     leds(1,1,2,1,1,2)
static bool defendEyes = false;
#define defenceTriggeredEyes  leds(2,1,2,2,1,2)
#define defenceBallLeftEyes   leds(1,2,1,1,1,1)
#define defenceBallRightEyes  leds(1,1,1,1,2,1)
#define defenceBallElseEyes   leds(2,2,2,2,2,2)
#define defenceCircleLEyes    leds(2,1,2,1,1,1)
#define defenceCircleREyes    leds(1,1,1,2,1,2)
#define defenceBoxAvoidLEyes  leds(2,2,2,1,1,1)
#define defenceBoxAvoidREyes  leds(1,1,1,2,2,2)
#define defencePostAvoidEyes  leds(2,1,1,2,1,1)
static const bool predictionHoverEyes = false;
static const bool offEdgeKickEyes = false;
static bool stealthEyes = false;
#define noStealthEyes         leds(2,1,1,2,1,1)
#define stealthLeftEyes       leds(2,2,2,1,1,1)
#define stealthRightEyes      leds(1,1,1,2,2,2)
#define stealthLeftBackEyes   leds(2,1,2,1,1,1)
#define stealthRightBackEyes  leds(1,1,1,2,1,2)
static bool locateBallKickEyes = false;
static bool getBehindBallEyes = false;
#define topLeftEdgeGbbEyes    leds(1,1,2,1,1,1)
#define topRightEdgeGbbEyes   leds(1,1,1,1,1,2)
#define topLeftCornerGbbEyes  leds(1,2,1,1,1,1)
#define topRightCornerGbbEyes leds(1,1,1,1,2,1)
#define botEdgeGbbEyes        leds(2,1,1,2,1,1)
#define nearGoalGbbEyes       leds(2,1,2,2,1,2)
static bool headEyes = false;
#define seeBallEyes           leds(2,1,1,2,1,1)
#define looklastBallEyes      leds(1,2,1,1,2,1)
#define lookGpsBallEyes       leds(1,1,2,1,1,2)
#define findEyes              leds(2,1,2,2,1,2)

// Debugging booleans.
static bool roleCout = false;
static bool ballSourceCout = false;
static bool pawKickCout = false;
static bool closeBallCout = false;
static bool backOffCout = false;
static bool matchTMCout = false; 
static bool zoidCout = false; 
static bool birdCout = false;
static bool debugOffEdgeKick = false;

// Initalise the variables.
void UNSW2004::initUNSW2004Forward() {
	lockMode = NoMode;
	lostBall = 0;
	grabTime = 0;

	kickingOff = false;
	kickingOffTimer = 0;
	initActiveLocaliseBeaconData();
}

static bool UNSW2004Debug = true;

// Called by Behaviours::DecideNextAction.
void UNSW2004::doUNSW2004Forward() {
    if (UNSW2004Debug)
        cout << "###############################" << endl;

	setDefaultParams();
	setWalkParams();
	
	// The core functions.
	setUNSW2004ForwardValues();    
	chooseUNSW2004ForwardStrategy();

    /*  Ted: Kevin's proper don't need to switch to PG = 40, am I right?    
    	if (lockMode == ProperVariableTurnKick) {
		    PG = 40;
	    }
    */
}

void UNSW2004::setUNSW2004ForwardKickOffValues() {
	if (mode_ == mNormal) {
		// Time out kickoff after 3 seconds
		if (kickingOff && kickingOffTimer < 75) {
			kickingOffTimer++;
		}
		else {
			kickingOff = false;
		}            
		
	/* Ted: commented it because these variables are removed from the behaviour namespace.

		// kick-off receiver uses ball pred for 3 secs after kick-off
		// attacker broadcasts that it has kicked-off
		if (kickingOff && isUNSW2004KickOffReceiver() && isUNSW2004TeammateHaveKickedOff()) {
			kickOffReceiverUseBallPred = true;
			kickOffReceiverUseBallPredTimer = 0;
		}
		if (kickOffReceiverUseBallPred && kickOffReceiverUseBallPredTimer < 75) {
			kickOffReceiverUseBallPredTimer++;
		}
		else {
			kickOffReceiverUseBallPred = false;
		}
		
	*/
	
	}
}

// Return true if vision ball could be used.
bool UNSW2004::canUseVisionBall() {
    if (vBall.cf > 0)
        return true;
    else
        return false;
}

// This function need to be tuned because wireless is faster than last year, the condition should be put less strict.
bool UNSW2004::canUseWirelessBall() {
    // 10 frames is 2003 values. Basically wireless ball is used only when lostBall is over 10 frames. May need to 
    // change in 2004 due to improved wireless speed.
    if (gps->shareBallvar < LARGE_VAL && lostBall > 10)
        return true;
    else
        return false;
}

// In 2003, there is not such function because Nathan assumes if vision and wireless ball source can't be used, then
// GPS ball source will be used. However this year maybe we can try to ignore all ball sources if none of the source
// is reliable. Currently this function returns true, need to tune and give some values in the future. See determineBallSource()
// below.
bool UNSW2004::canUseGPSBall() {
    return true;
}

// Determine which ball source to use.
// This function need to be tune according to the new GPS ball accuracy.
// In 2003, this function search for vision, wireless then GPS ball. It always determine
// a ball source. However it may be better for the robot to ignore all ball sources and search it
// by itself.
void UNSW2004::determineBallSource() {
    bool findBall = false;
    // If vision ball is available, use it since it is the most accurate.
	if (canUseVisionBall()) {
		ballSource = VISION_BALL;
		ballX = vBall.x;
		ballY = vBall.y;
		ballH = vBall.h;
		ballD = vBall.d;
		UNSW2004WirelessBallDistThreshold = wirelessBallMinThreshold;
		findBall = true;
	}
	else if (canUseWirelessBall()) {    // Check if wireless ball is accurate.
            ballSource = WIRELESS_BALL;
	    ballX = gps->shareBallx;
	    ballY = gps->shareBally;
	    ballH = NormalizeAngle_180(RAD2DEG(getHeadingBetween(gps->self().pos.x, 
								 gps->self().pos.y, ballX, 
								 ballY)) - gps->self().h);
	    ballD = getDistanceBetween(ballX, ballY, gps->self().pos.x, gps->self().pos.y);
            findBall = true;
	    UNSW2004WirelessBallDistThreshold = MAX(wirelessBallMinThreshold, 
				     2 * sqrt(gps->teammate(gps->sbRobotNum).posVar));
        }
        else if (canUseGPSBall()) {   // Check if GPS ball is accurate.
    		    ballSource = GPS_BALL;
		    ballX = gps->getBall(GLOBAL).x;
		    ballY = gps->getBall(GLOBAL).y;
		    ballH = gps->getBall(LOCAL).head;
		    ballD = gps->getBall(LOCAL).d;
		    UNSW2004WirelessBallDistThreshold = wirelessBallMinThreshold;
		    findBall = true;
	}    

	// Can't find any reliable ball source.    
	if (!findBall)
	     ballSource = NO_BALL;
}

void UNSW2004::setUNSW2004ForwardValues() {
    setUNSW2004ForwardKickOffValues();

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].var < get95CF(25)) {
			teammateSeenTime++;
			break;
		}
		if (i == NUM_TEAM_MEMBER - 1) {
			teammateSeenTime = 0;
		}
	}

	if (gps->canSee(vobBall) && !activeLocaliseTimer) {
		checkLastBallPos = 0;
	}

	if (gps->canSee(vobBall)) {
		storeWirelessBallDistThreshold = 0;
		//checkLastBallPos = 0;
		setUNSW2004BallDist(vBall.d);
	}
	else {
		setUNSW2004BallDist(LOSTBALL_DIST);
	}

	// Check for visual opponent, to see if we should charge.
	static const int CHARGE_THRESHOLD = 12;
	static const int BLOCK_THRESHOLD = 15;    
	allowedToCharge = false;
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		// should this be changed to check variance? Asking since it is the norm
		// and i will be checking variance when deciding whether to copy values - Ross
		if (vOpponent[i].cf > 0 && abs(vOpponent[i].h - vBall.h) < BLOCK_THRESHOLD
		    && abs(vOpponent[i].d - vBall.d) < BLOCK_THRESHOLD) {
			lastSeenOpponent = 0;
			break;
		}
	}
	if (lastSeenOpponent < CHARGE_THRESHOLD) {
		lastSeenOpponent++;
		allowedToCharge = true;
	}    

	if (numForwardsCommunicating() >= 3) {
		framesSince3ForwardsPresent = 0;
	}
	else {
		framesSince3ForwardsPresent++;
	}

	// Determine the ball source. Vision? Wireless? or GPS?
	determineBallSource();

	if (UNSW2004WirelessBallDistThreshold < storeWirelessBallDistThreshold) {
		UNSW2004WirelessBallDistThreshold = storeWirelessBallDistThreshold;
	}

	framesSinceInVarianceCircle++; //wireless hack

	// decrement counters & timers
	decrementBackOff();
	DECREMENT(activeLocaliseTimer);
	DECREMENT(disallowActiveLocalise);
	DECREMENT(grabTime);
	DECREMENT(checkLastBallPos);

	attackMode = GoToBall;
	utilDetermineDKD();

	updateActiveLocaliseBeaconData();

	// Bernhard's visual ball smoothing for ball grabbing. I don't know
	// what this is, any idea?
	// changed 3/7/03 Ross if (gps->canSee(vobBall)) {
	// 6 / 4 / 04 not used at all
	/*if (vBall.cf > 0) {
		thdVisBallDist = sndVisBallDist;
		thdVisBallHead = sndVisBallHead;
		sndVisBallDist = fstVisBallDist;
		sndVisBallHead = fstVisBallHead;
		fstVisBallDist = vBall.d;
		fstVisBallHead = vBall.h;
		}*/
}

// Define the actions executed when mode_ = mStartMatch. This method
// assume when it is called, mode_is equal to mStartMatch.
void UNSW2004::UNSW2004StartMatchMode() {
		if (activeLocaliseTimer) {                        
			// smartActiveGpsLocalise();            
            superActiveLocalise();                        
			if (activeLocaliseTimer == 1) {
				disallowActiveLocalise = 75;
				// Reset the lost ball so that you will look to gps ball on return from active localise.
				if (lostBall > vBallLost) {
					lostBall = 0;
				}
			}
			return;
		}
		else if (checkLastBallPos) {
    		headtype = lastHeadType;
			tilty = lastHeadTilty;
			panx = lastHeadPanx;
			cranez = lastHeadNullz;
		}
		else if (!disallowActiveLocalise) {
			if (gps->canSee(vobBall)) {
				checkLastBallPos = 6;
				lastHeadType = headtype;
				lastHeadTilty = tilty;
				lastHeadPanx = panx;
				lastHeadNullz = cranez;
			}
			// smartSetBeacon();
			// smartActiveGpsLocalise();
            superActiveLocalise();
			activeLocaliseTimer = 10;
		}
		if (gps->canSee(vobBall)) {
			doUNSW2004TrackVisualBall();
			setUNSW2004BallDist(vBall.d);
		}
#ifndef OFFLINE
        // Turn off by Ted Wong.
		else if (lostBall <= 3 && false) {
			headtype = ABS_XYZ;
			panx = fstVisBallDist * sin(DEG2RAD(fstVisBallHead));
			tilty = BALL_RADIUS;
			cranez = fstVisBallDist * cos(DEG2RAD(fstVisBallHead));
			if (abs(hPan - *desiredPan) > 8 || abs(hTilt - *desiredTilt) > 8)
				lostBall = 0;
			setUNSW2004BallDist(LOSTBALL_DIST);
		}
#endif // OFFLINE
		else if (lostBall <= vBallLost && abs(gps->getBall(LOCAL).head) < 90) {
			doUNSW2004TrackGpsBall();
			setUNSW2004BallDist(LOSTBALL_DIST);
		}
		else {
			headFindBall(-55);
			standTall = true;
			setUNSW2004BallDist(LOSTBALL_DIST);
		}
		// If you are the third forward during kick-off, continue to pan.
		if (kickingOff && !isUNSW2004KickOffAttacker() && !isUNSW2004KickOffReceiver()) {
			actionTimer = 139;
			StationaryLocalize();
		}
}

static bool lookDebug = true;

void UNSW2004::UNSW2004ExecuteLockModeAction() {
    if (lookDebug)
        cout << "The lockmode at 2004 forward: " << lockMode << endl;
        
	switch (lockMode) {
        case ActiveLocalise: 
            aaActiveLocalise();
            break;

        case ChestPush: 
            aaChestPush();
            break;

        case SpinKick:
            aaSpinKick();
            break;

        case SafeSpinKick: 
            aaSafeSpinKick();
            break;

        case VisualSpinKick: 
            aaVisualSpinKick();
            break;

        case VisOppAvoidanceKick: 
            aaVisOppAvoidanceKick();
            break;

        case SpinChestPush: 
            aaSpinChestPush();
            break;

        case SpinDribble: 
            aaSpinDribble();
            break;

        case Dribble: 
            aaDribble();
            break;
            
        // Don't think we do chest pass at all.
    	case BeckhamChestPass: lockMode = NoMode; //Beckham::aaBeckhamChestPass();
	                           break;

        case GoalieKick:
            aaGoalieKick();
            break;

        case LightningKick: 
            aaLightningKick();
            break;

	case ProperVariableTurnKick: 
	  aaProperVariableTurnKick();
	  break;

	case UpennRight: 
	    aaUPkickRight();
            break;

	case UpennLeft:
	    aaUPkickLeft();
            break;
	  

        case UNSW2004Defend: doUNSW2004Defend();
	    			         break;

        case LocateBallKick: 
            aaLocateBallKick();
            framesSinceInVarianceCircle = 0;
            break;
    
    }   
}

static int lookTimer = 0;
static bool strategyDebug = true;

// Core function - choose the forward strategy.
void UNSW2004::chooseUNSW2004ForwardStrategy() {
    // Initally no role assigned.

  INSTRUMENT("UNSW2004::chooseUNSW2004ForwardStrategy" << endl);

    myRole = NOTHING;
    if (lockMode != NoMode && lockMode != ResetMode) {
        if (lockMode == UNSW2004Defend)
            myRole = DEFENDER;
        else
            myRole = LOCKMODE;
        doUNSW2004TrackVisualBall();
        setUNSW2004BallDist(0);
        UNSW2004ExecuteLockModeAction();
        return;
    }
	if (mode_ == mStartMatch) {
		UNSW2004StartMatchMode();
	} 
    else {

    /*
        Ted Wong: LookAround for any robot implementation. This code doesn't mean needed to be place here.
    
        lookTimer++;
        if (lookTimer >= 200) {
            lookTimer = 0;
            resetLookAround();
            lookAround();
            return;    
        }
    */

        // Track vision or gps ball to decide whether ball is under chin.
        UNSW2004GeneralHead();

        // Ted: Removed offEdgeCounter garbage written by Nathan Wong. I am not sure we will
        //      use offEdgeCounter anymore in the future. At least in 2003 competition, this
        //      was not used and not tuned very more. Revision 2761 and referenceCode.cc.
				
	    // If UNSW2004 should backoff
	    // if unsure of own heading, active localise. Need to check if this 
	    //
	    // Note that active localise is given higher priority
	    // over grab ball condition.
    	if (gps->self().hVar > get95CF(60)) {
	  KEV_DEBUG(cout << "active localise in choosing forward\n";);
	  	    lockMode = ActiveLocalise;
		    resetActiveLocalise();
		    aaActiveLocalise();
		    setWalkParams();            
		    if (gps->self().pos.x < FIELD_WIDTH / 2.0)
		        turnCCW = -20;
		    else
	    		turnCCW = 20;
		    return;
        }        


        double smallestDist = 10000;
        for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
            const WMObj &tm = gps->tmObj(i);
            interpBehav info = (interpBehav) tm.behavioursVal;
            if (info.ballDist < smallestDist && tm.counter > 0)
                smallestDist = info.ballDist;        
        }

        bool closeBall;
        
        if (ballD <= smallestDist)
            closeBall = true;
        else
            closeBall = false;

        if (!closeBall) {

            // Shouldn't apply for the robot that is closest to the ball. Go without any visual backoff!
            bool nearRobot = false;
    	    for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		        if (vTeammate[i].var < get95CF(200) && vTeammate[i].d <= 55) {
                    nearRobot = true;
	    	    }
    	    }
        
            if (nearRobot) {
                myRole = NOTHING; // A dummy debug.
                left = 0;
                turnCCW = 0;
                forward = -5;
                return;
            }
        
        }


    	// If ball is under chin. Need to tune UnderChin() again.
	    if (isUNSW2004BallUnderChin()) {
    		chooseUNSW2004ShootStrategy();
            // Ted: What is fstVis stuffs? If anybody prove this is Nathan's another garbage, please remove it.
	    	fstVisBallDist = 10;
    		fstVisBallHead = 0;
	    	return;
        }

	/* Ted removed it because these variables are removed from the namespace.
	
	    // Kick-off receiver runs forward for 3 seconds or until it
	    // receives the 'have kicked-off' signal from teammate.
	    if (kickingOff && isUNSW2004KickOffReceiver() && !kickOffReceiverUseBallPred) {
    	    KEV_DEBUG(cout << "kicking off go forward in choosing forward\n";);
	  	    setWalkParams();
		    walkType = NormalWalkWT; //ZoidalWalkWT;
		    forward = 6.5;
		    turnCCW = 0;
		    left = 0;                    
		    return;
          } 

	*/

        // At this point, check if extreme defence is needed.
	    // in other words activate bird of prey if necessary.
	    if (doDefence && UNSW2004DefenceRequired()) {
	      KEV_DEBUG(cout << "defending in choosing forward\n";);
            myRole = DEFENDER;
    	    lockMode = UNSW2004Defend;
	        doUNSW2004Defend();
	        return;
	    }

      // If can't seen the ball become striker and strike for a particular point.
      // Basically if hackStrikerIntoSupport is true, then a striker, support is prefer than a striker.
	  // if (!hackStrikerIntoSupport || ballSource != VISION_BALL) {
         if (true) {
         
        // Find striker point.
	    // Kevin: the striker now should do what support does
	    //        but much furhter away.

			double strikerPtX = 0; 
			double strikerPtY = 0;
			getUNSW2004StrikerPoint(&strikerPtX, &strikerPtY);
			// if UNSW2004 should assume striker role
			if (isUNSW2004ToAssumeStrikerRole(strikerPtX, strikerPtY)) {
			  // cout << "I am the striker!" << endl;             
              myRole = STRIKER1;
			  setUNSW2004Striker();
              // What is hailMary ????
			  if (doHailMary && (gps->getBall(GLOBAL).y > FIELD_LENGTH * 0.5)
			      && (getNumTeammatesOnSide() >= 2) && (getNumOpponentsOnSide() <= 1))
			    doUNSW2004HailMaryReceive();
			  else
    			  doUNSW2004StrikerForward(strikerPtX, strikerPtY);
			  clipUNSW2004FireballPan();
			  return;            
            }
	  }

	   // If UNSW2004 should backoff. Notice that backoff is always visual, gps is just too bad for backoff purpose.
	  double headToTeammate, teammatex, teammatey;
	  if (isUNSW2004ToVisualBackOff(&headToTeammate, &teammatex, &teammatey)) {

	    KEV_DEBUG(cout << "backing off in choosing forward\n";);
          if (backOffCout)
              cout << "I am going to do visual backoff!" << endl; 
          myRole = BACKOFF;
          left = 0;
          turnCCW = 0;
          forward = -5;
          // doUNSW2004BackOffForward(headToTeammate, teammatex, teammatey);
	      clipUNSW2004FireballPan();
	      return;
	  }

	  // If UNSW2004 should signal 'have ball'.
//	  if (haveBallSupporting && isUNSW2004HaveBall()) {
	  if (isUNSW2004HaveBall()) {
	    KEV_DEBUG(cout << "having ball in choosing forward\n";);
            myRole = NORMAL;
        	doUNSW2004NormalForward();
	    	if (isUNSW2004ToSideBackOff()) {
	    		forward *= 0.6;                                
		    }                
    		clipUNSW2004FireballPan();

            // May need to tune this condition.
	    	if (!isUNSW2004Stuck()) {
		    	setUNSW2004HaveBall();
		    }
	        return;
	  }

	  // If a UNSW2004 teammate signals 'have ball'.
//	  if (haveBallSupporting && isUNSW2004ToAssumeSupportRole()) {
	  if (isUNSW2004ToAssumeSupportRole()) {
            if (strategyDebug)
                cout << "Doing the support!" << endl;
	    KEV_DEBUG(cout << "supporting in choosing forward\n";);
            myRole = SUPPORT;  
    		doUNSW2004SupportForward(ballX, ballY);
	    	clipUNSW2004FireballPan();
		    return;
	  }    

      myRole = NORMAL;    
	  doUNSW2004NormalForward();
     
      // If a robot thinks it is beside some other robots, then move a bit slowly to prevent any possible leg locked. 
	  if (isUNSW2004ToSideBackOff()) {
			forward *= 0.6;
	  }
    }

	// Needed for getBehindBall to work.
	prevAttackMode = attackMode;

	// safeguard to ensure panx is clipped on close approach to ball
	// necessary in order to grab the ball reliably
	clipUNSW2004FireballPan();
}

/////////////////////////////////////////////////////////////////////////
// conditional functions
/////////////////////////////////////////////////////////////////////////

bool UNSW2004::isUNSW2004BallUnderChin() {
	INSTRUMENT("UNSW2004::isUNSW2004BallUnderChin" << endl);

	if (!gps->canSee(vobBall) && lostBall >= 10) {
		return false;
	} 
	
	// calculate vision ball local coordinates
	double ballx = sin(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
	double bally = cos(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
	int chestIR = sensors->sensorVal[ssCHEST_INFRARED];

	// set pan to zero in preparation for ball grabbing when ball is close
	clipUNSW2004FireballPan();

	/*	if (ABS(ballx) <= 3.0 && bally <= 1.5 * BALL_RADIUS && hTilt <= -40
		&& vision->vob[vobBall].misc > HEIGHT / 2) {*/

	//KEV_DEBUG(cout << "\n --- ballx: " << ballx << "    bally: " << bally << "   " << "    hTilt: " << hTilt 
	//<< "    chest: " << chestIR << endl;);
	
	if (ABS(ballx) <= 3.0 && bally <= 2.7 * BALL_RADIUS && hTilt <= -11 
	    && ((chestIR > 125000  /*&& chestIR < 170000*/)||chestIR < 109000) ) {
	  //KEV_DEBUG(cout << "TRUE -- case 1" << endl;);
	    return true;
	}

	/*	if (lostBall < 10     && ((chestIR > 130000  && chestIR < 150000) || 
				  chestIR < 109000) && 
	    abs(lastHPan) < 10 && lastHTilt <= -15 ) {
	    KEV_DEBUG(cout << "TRUE -- case 2" << endl;);
	    return true;
	    }
        
    */
    /*  Kevin's code. Commmented because not working.
	    if (lostBall < 10 && chestIR > 130000  && chestIR < 150000 && 
	        abs(lastHPan) < 10 && lastHTilt <= -15 ) {
	        cout << "\nTRUE -- case 2" << endl;
	        return true;
	    }
    */
    
	//KEV_DEBUG(cout << "FALSE -- case 3" << endl;);
	return false;
}        

void UNSW2004::clipUNSW2004FireballPan() {
  INSTRUMENT("UNSW2004::clipUNSW2004FireballPan" << endl);

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

bool UNSW2004::isUNSW2004ToAssumeSupportRole() {
	INSTRUMENT("UNSW2004::isUNSW2004ToAssumeSupportRole" << endl);
	return isUNSW2004TeammateHaveBall();
}

// these oldVis... values used to be needed when there was locking, but are not anymore..
static double oldVisTeammateX = 0;
static double oldVisTeammateY = 0;
static double oldVisTeammateHead = 0;

// Returns your local heading to that teammate, and the teammates global x and y pos. jan
bool UNSW2004::isUNSW2004ToVisualBackOff(double *headingToTM, double *retx, double *rety) {
	INSTRUMENT("UNSW2004::UNSW2004NeedVisualBackoffWithReturn" << endl);

	int bestIndex = -1;
	double bestAbsAng = -10;

	// Check if teammate in front of you with ball.
	// if more than 1 teammate.. pick the teammate thats furthest around the ball.
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].var < get95CF(200)
				&& vTeammate[i].d < 55
				&& vBall.cf > 0
				&& vBall.d  < 100) {
			//not this code makes assumptions about the UNSW2004 code..
			//ie it assumes ballX and ballY are set before ever running this function
			double ang = absAngleBetweenTwoPointsFromPivotPoint(vTeammate[i].x,
			                vTeammate[i].y, FIELD_WIDTH / 2.0, FIELD_LENGTH, ballX,
					        ballY);
			if (ang > bestAbsAng) {
				bestAbsAng = ang;
				bestIndex = i;
			}
		}
	}
	
	if (bestIndex >= 0) {
		oldVisTeammateX = vTeammate[bestIndex].x;
		oldVisTeammateY = vTeammate[bestIndex].y;
		oldVisTeammateHead = vTeammate[bestIndex].h;
		*retx = oldVisTeammateX;
		*rety = oldVisTeammateY;
		*headingToTM = oldVisTeammateHead;
		return true;
	}

	bestIndex = -1;
	double smallestDistance = (double) LARGE_VAL;

	// Check if teammate in front of you regardless of ball
	// if more than 1 teammate.. pick the biggest teammate blob.
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vTeammate[i].var < get95CF(200)
				&& ABS(vTeammate[i].h) < 45
				&& vTeammate[i].d < 55) {
			if (vTeammate[i].d < smallestDistance) {
				smallestDistance = vTeammate[i].d;
				bestIndex = i;
			}
		}
	}

	if (bestIndex >= 0) {
		oldVisTeammateX = vTeammate[bestIndex].x;
		oldVisTeammateY = vTeammate[bestIndex].y;
		oldVisTeammateHead = vTeammate[bestIndex].h;
		*retx = oldVisTeammateX;
		*rety = oldVisTeammateY;
		*headingToTM = oldVisTeammateHead;

		return true;
	}

	// else didn't find anything reset the old/memory things
	oldVisTeammateX = 0;
	oldVisTeammateY = 0;
	oldVisTeammateHead = 0;
	*retx = oldVisTeammateX;
	*rety = oldVisTeammateY;
	*headingToTM = oldVisTeammateHead;
	return false;
}

bool UNSW2004::isUNSW2004ToSideBackOff() {
	INSTRUMENT("UNSW2004::isUNSW2004ToSideBackOff" << endl);

    // Not sure this is good. Maybe we want the robot to side-backoff a bit even it can't see a ball as long as
    // a robot is beside it.
	if (!(vBall.cf >= 3 && vBall.h <= 15 && vBall.d >= 20)) {
		//cout << "cant see" << endl;
		return false;
	}

	for (int i = 1; i <= NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i - 1);
		interpBehav info = (interpBehav) tm.behavioursVal;

		//  cout << "var, head, dist, diff in ball dist (mine - theirs) "
		//  2*sqrt(gps->teammate(i).posVar) << " " <<
		//  gps->teammate(i).pos.head << " " <<
		//  gps->teammate(i).pos.d << " " <<
		//  (vBall.d - info.ballDist) << endl;

		// Check that they are a forward, and that the counter hasn't run out either.
        // This seriously need to be tuned in terms of new robot distance calculated by Daniel.
		if (i != PLAYER_NUMBER
				&& gps->tmObj(i - 1).counter > 0
				&& info.amGoalie == 0
				&& gps->teammate(i).posVar < get95CF(50)
				&& ABS(gps->teammate(i).pos.head) > 55
				&& ABS(gps->teammate(i).pos.head) < 125
				&& gps->teammate(i).pos.d < 60) { // jan
			//if they can see the ball
			if (info.ballDist <= LOSTBALL_DIST) {
				//if your further
				if (vBall.d - info.ballDist > 15) {
					//cout << "side back off true " << endl;
					return true;
				} 
				//if they are further
				else if (info.ballDist - vBall.d > 15) {
					continue;
				} 
				//if your the higher player number
				else if (PLAYER_NUMBER > i) {
					//cout << "side back off true " << endl;
					return true;
				}
			}
		}
	}
	//cout << "side back off false " << endl;
	return false;
}


bool UNSW2004::isUNSW2004HaveBall() {
	INSTRUMENT("UNSW2004::isUNSW2004HaveBall" << endl);

	// close to ball & can't see teammate
	if (gps->canSee(vobBall)
			&& vBall.d < 60
			&& !canUNSW2004SeeTeammateWithinDist(60)) {
		return true;
	}

	// close to ball & heading roughly correct
	if (gps->canSee(vobBall) && vBall.d < 80 && utilIsUp()) {
		return true;
	}        

	return false;
}

bool UNSW2004::canUNSW2004SeeTeammateWithinDist(double dist) {
	INSTRUMENT("UNSW2004::canUNSW2004SeeTeammateWithinDist" << endl);
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		// should this be changed to check variance? Asking since it is the norm
		// and i will be checking variance when deciding whether to copy values - Ross
		// if changed please change in malak aswell
		if (vTeammate[i].cf > 0 && vTeammate[i].d < dist)
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// Constants & variables used only by 
//  isUNSW2004Stuck();
//////////////////////////////////////////////////////////////////////

static const double STUCK_DIST_TO_BALL_THRESHOLD = 10;
static const double STUCK_NUM_OF_FRAMES_WITHIN_THRESHOLD_LIMIT = 60;
static const double STUCK_FORWARD_THRESHOLD = 5;

static double stuckPrevDistToBall = LARGE_VAL;
static int stuckNumOfFramesWithinThreshold = 0;

//////////////////////////////////////////////////////////////////////
// End constants & variables
//////////////////////////////////////////////////////////////////////

bool UNSW2004::isUNSW2004Stuck() {
	INSTRUMENT("UNSW2004::isUNSW2004Stuck" << endl);

	if (isUNSW2004BallNotMoving() && forward > STUCK_FORWARD_THRESHOLD) {
		return true;
	}
	return false;
}

bool UNSW2004::isUNSW2004BallNotMoving() {
	INSTRUMENT("UNSW2004::isUNSW2004BallNotMoving" << endl);

	if (gps->canSee(vobBall) && vBall.d > 20) {
		if (ABS(vBall.d - stuckPrevDistToBall) < STUCK_DIST_TO_BALL_THRESHOLD) {
			stuckNumOfFramesWithinThreshold++;
		}
		else {
			stuckNumOfFramesWithinThreshold = 0;
		}

		if (stuckNumOfFramesWithinThreshold
				>= STUCK_NUM_OF_FRAMES_WITHIN_THRESHOLD_LIMIT) {
			return true;
		}
		else {
			return false;
		}

		stuckPrevDistToBall = vBall.d;
	}
	else {
		return false;
	}
}


/////////////////////////////////////////////////////////////////////////
// striker functions
/////////////////////////////////////////////////////////////////////////

//variables to do with the striker position hysterisis thing
static const int strikerAccSmallCircle = 10;
static const int strikerAccLargeCircle = 20;
static int strikerPointReqAccuracy = strikerAccSmallCircle;

void UNSW2004::doUNSW2004StrikerForward(double ptx, double pty) {
	INSTRUMENT("UNSW2004::doUNSW2004StrikerForward" << endl);

	double reqAcc = requiredAccuracy(ballX, ballY, 20);
	//abs fix
	double ballToGoalHead = RAD2DEG(atan2((FIELD_LENGTH - ballY),
				(FIELD_WIDTH / 2 - ballX)));

	if (!hackStrikerIntoSupport) {
		//if you get up to here you should be seeing the ball
		if (ballSource == VISION_BALL && ballD < 45 &&  //your lined up to paw kick the ball?
		    ballY >= LEFT_GOALBOX_EDGE_Y && ballX >= LEFT_GOALBOX_EDGE_X
		    && ballX <= RIGHT_GOALBOX_EDGE_X &&  // ball is in target goalie box
		    gps->self().h <= (ballToGoalHead + reqAcc) && gps->self().h
		    >= (ballToGoalHead - reqAcc)) { // you'll get it in if you hit it)
		    
			doUNSW2004AttackBall();
			if (roleEyes)
				leds(2, 1, 2, 2, 1, 2);
			return;
		}
	}

	//allow the striker to active localise
	if (activeLocaliseTimer) {
		//activeGpsLocalise(false);
		smartActiveGpsLocalise();
		if (activeLocaliseTimer == 1) {
			disallowActiveLocalise = 75;
			//reset the lost ball so that you will look to gps ball on return from active localise
			//if (lostBall > vBallLost) {
			//lostBall = 0;
			//}
		}
	}
	else if (checkLastBallPos) {
		headtype = lastHeadType;
		tilty = lastHeadTilty;
		panx = lastHeadPanx;
		cranez = lastHeadNullz;
	}
	else if (!disallowActiveLocalise && ballD > 60) {
		if (gps->canSee(vobBall)) {
			doUNSW2004TrackVisualBall();
			checkLastBallPos = localiseLockLength + 6;
			lastHeadType = headtype;
			lastHeadTilty = tilty;
			lastHeadPanx = panx;
			lastHeadNullz = cranez;
		}

		setNewBeacons(LARGE_VAL, 90);            
		activeGpsLocalise(false);
		activeLocaliseTimer = localiseLockLength;
	}

	double h = NormalizeAngle_0_360(RAD2DEG(atan2(ballY - gps->self().pos.y,
					ballX - gps->self().pos.x)));
	setWalkParams();
	walkType = CanterWalkWT;
	saGoToTargetFacingHeading(ptx, pty, h);

	//hysterisis for whether or not your at the striker point
	double dist = sqrt(SQUARE(ptx - gps->self().pos.x)
			+ SQUARE(pty - gps->self().pos.y));
	if (dist <= strikerPointReqAccuracy) {
		strikerPointReqAccuracy = strikerAccLargeCircle;
		forward = 0;
		left = 0;
	}
	else {
		strikerPointReqAccuracy = strikerAccSmallCircle;
	}
}

int UNSW2004::numForwardsCommunicating() {
	INSTRUMENT("UNSW2004::numForwardsCommunicating" << endl);
	int ret = 1;
	for (int i = 0 ; i < NUM_TEAM_MEMBER ; i++) {
		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;
		if (gps->tmObj(i).counter > 0 && info.amGoalie != 1) {
			ret++;
		}
	}
	return ret;
}

static bool strikerCouts = false;

// ivy. only this striker function has a problem.
bool UNSW2004::isUNSW2004ToAssumeStrikerRole(double strikerPtX, double strikerPtY) {
	INSTRUMENT("UNSW2004::isUNSW2004ToAssumeStrikerRole" << endl);

	// if you dont know where the ball is dont assume striker role
	if (ballSource == GPS_BALL && lostBall > vBallLost) {
		if (strikerCouts)
			cout << "FALSE, cant see ball" << endl;
		return false;
	}

	double ballDistToTargetGoal = getDistanceBetween(TARGET_GOAL_X, TARGET_GOAL_Y, ballX, ballY);

	// If ball is very close to target goal, assume striker role
	// if you don't have ball, another robot has ball, and you are 
	// closest to the striker point
	//
	// ie if the ball is close to the target goal, try to ensure that there will be a striker
	if (ballDistToTargetGoal < 65) {
		if (!isUNSW2004HaveBall()
				&& isUNSW2004TeammateHaveBall()
				&& isUNSW2004ClosestToStrikerPoint(strikerPtX,
					strikerPtY)) {
			// if there exists a striker already, check for higher player number    
			for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
				const WMObj &tm = gps->tmObj(i);
				interpBehav info = (interpBehav) tm.behavioursVal;

				// ignore goalie and invalid teammates
				if (tm.counter <= 0 || info.amGoalie == 1)
					continue;

				// ignore yourself
				if (i == PLAYER_NUMBER - 1)
					continue;

				// if they are already doing the striker role and they have a 
				// higher player number than you, dont do third player
				// player number thing is just to break symmetery
				if (info.amStriker && PLAYER_NUMBER < (i + 1)) {
					return false;
				}
			}    
			return true;
		}
	}

	//if you only know of 1 other forward dont become striker (its better to become supporter)
	if (numForwardsCommunicating() < 3 && framesSince3ForwardsPresent >= 3) {
		if (strikerCouts)
			cout << "FALSE, cant find enough forwards" << endl;
		return false;
	}

	// calculate own distance to ball
	const Vector &own = gps->self().pos;
	double ownDistToBall = sqrt(SQUARE(ballX - own.x) + SQUARE(ballY - own.y));

	if (strikerCouts) {
		cout << "my distance (" << PLAYER_NUMBER << ") to the ball " << ownDistToBall << endl;
	}

	// find the teammate that is furthest from ball
	int teammateFurthestFromBallPlayer = -1;
	double teammateFurthestFromBallDistance = -1;

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;

		// ignore goalie and invalid teammates
		if (tm.counter <= 0 || info.amGoalie != 0)
			continue;

		// ignore yourself
		if (i == PLAYER_NUMBER - 1)
			continue;

		// ignore someone who is "defending"
		if (birdIsNotStriker && info.defending != 0)
			continue;

		// if they are already doing the striker role and they have a 
		// higher player number than you, dont do third player
		// player number thing is just to break symmetery
		if (info.amStriker && PLAYER_NUMBER < (i + 1)) {
			if (strikerCouts)
				cout << "FALSE, theres already another striker out there" << endl;
			return false;
		}

		double teammateDist = LOSTBALL_DIST;

		// if they can see the ball themselves
		if (info.ballDist < (unsigned int) abs(LOSTBALL_DIST)) {
			teammateDist = info.ballDist;
			if (strikerCouts)
				cout
					<< "teammates REPORTED distance to ball "
					<< teammateDist
					<< endl;
		}
		else {
            // Modify this code fragment so that the robot who can't see the ball won't affect the striker.
            /* janice
    			const Vector &teammate = tm.pos;
	    		teammateDist = sqrt(SQUARE(ballX - teammate.x) + SQUARE(ballY - teammate.y));
            */
            teammateDist = 0;                    
		    if (strikerCouts)
			    cout << "teammates CALCULATED distance to ball " << teammateDist << endl;
		}

		if (teammateFurthestFromBallDistance < teammateDist) {
			teammateFurthestFromBallPlayer = i;
			teammateFurthestFromBallDistance = teammateDist;
		}
	}

	// if you're the only forward alive, don't become striker
	if (teammateFurthestFromBallPlayer == -1) {
		if (strikerCouts)
			cout << "FALSE im the only one alive" << endl;
		return false;
	}

	// if you're obviously furthest away from ball, become wing striker
	if (ownDistToBall - teammateFurthestFromBallDistance > 30) {
		if (strikerCouts)
			cout << "TRUE im obviously the furthest from the ball" << endl;
		return true;
	}

	// if you're obviously not furthest away from ball, don't become wing striker
	if (teammateFurthestFromBallDistance - ownDistToBall > 30) {
		if (strikerCouts)
			cout << "FALSE im obviously not furthest away from the ball" << endl;
		return false;
	}

	///////////////////////////////////////////////////////////////////
	// you're not obviously furthest away from the ball
	///////////////////////////////////////////////////////////////////

	// obtain own distance to striker point
	double ownDistToStrikerPt = sqrt(SQUARE(strikerPtX - own.x)
			+ SQUARE(strikerPtY - own.y));
	if (strikerCouts) {
		cout << "my distance to striker point " << ownDistToStrikerPt << endl;
	}

	// obtain teammate's distance to striker point
	const Vector &teammateToCompareWith = gps->tmObj(teammateFurthestFromBallPlayer).pos;

	double teammateDistToStrikerPt = sqrt(SQUARE(strikerPtX - teammateToCompareWith.x) + SQUARE(strikerPtY - teammateToCompareWith.y));
	if (strikerCouts) {
		cout << "teammates distance to striker point " << teammateDistToStrikerPt
			<< endl;
	}        

	// if closer to the striker point, become striker
	if (ownDistToStrikerPt < teammateDistToStrikerPt) {
		if (strikerCouts)
			cout << "TRUE im the closer one to the striker point" << endl;
		return true;
	}
	else {
		if (strikerCouts)
			cout << "FALSE im NOT the closer one to the striker point" << endl;
		return false;
	}
}

void UNSW2004::getUNSW2004StrikerPoint(double *ptx, double *pty) {
	INSTRUMENT("UNSW2004::getUNSW2004StrikerPoint" << endl);

	switch (strikerType) {
		case claudeStriker:  {
			double desx;
			double desy;

			if (posnTowardsOwnGoalFromPointWithCrop(ballX, ballY, 155, &desx,
								&desy) > 5) {
						     // ball in target half
						     *ptx = desx;
						     *pty = desy;
					     }
					     else {
						     // ball in own half
						     getUNSW2004StrikerPointDefense(ptx, pty);
					     }
		}
				     
		break;
				     
		case oldStriker:  {
			if (ballY >= FIELD_LENGTH / 2.0) {
				// ball in target half
				getUNSW2004StrikerPointOffense(ptx, pty);
			}
			else {
				// ball in own half
				getUNSW2004StrikerPointDefense(ptx, pty);
			}
		} 
				  
		break;
				  
		case willStrikerA:  {
			double centx;
			double centy;
			const double normalBackoffDist = 155;
			const double attackSwitchYPosn = FIELD_LENGTH - 125;
			const double attackSwitchDist = 60;
			double posnDist = posnTowardsOwnGoalFromPointWithCrop(ballX,
					    ballY, normalBackoffDist, &centx, &centy);

			if ((centy < FIELD_LENGTH / 4.0) || (posnDist < 5)) {
				// hard defense case
				// position on the far side of the field

				if (ballX < (FIELD_WIDTH / 2.0)) {
					*ptx = FIELD_WIDTH / 2.0 + 80;
					*pty = FIELD_LENGTH / 4.0;
				}
				else {
					*ptx = FIELD_WIDTH / 2.0 - 80;
					*pty = FIELD_LENGTH / 4.0;
				}
			}
			else if (ballY < attackSwitchYPosn - attackSwitchDist) {
				// middle of the field
				// position between the ball and the goal

				*ptx = centx;
				*pty = centy;
			}
			else {
			// hard attack case
			// move to the offense position
			double attPosx, attPosy;

			// first find the attack position
			{
				// this draws a line from the center of the goal to the ball
				// you pick your side based on that line
				// this makes you less likely to switch sides in front of the goal
				double safeBallX = ballX;
				if (TARGET_GOAL_X - safeBallX) {
					safeBallX += 0.01;
				} //avoid divide by 0

				double m = (TARGET_GOAL_Y - ballY) / (TARGET_GOAL_X - safeBallX);
				double b = TARGET_GOAL_Y - m *TARGET_GOAL_X;
				if (m == 0) {
					m += 0.01;
				} //avoid divide by 0
				
				double xMatchingYourY = (gps->self().pos.y - b) / m;

				// if ball is far upfield, go to the static striker points
				if (gps->self().pos.x <= xMatchingYourY) {
					attPosx = LEFT_GOALBOX_EDGE_X - 10;
					attPosy = attackSwitchYPosn;
				}
				else {
					attPosx = RIGHT_GOALBOX_EDGE_X + 10;
					attPosy = attackSwitchYPosn;
				}
			}
			// now interpolate over attackSwitchDist between the two points

			double alpha = 1;

			if (ballY < attackSwitchYPosn) {
				//leds(1,1,2,1,1,1);
				alpha = (ballY - (attackSwitchYPosn - attackSwitchDist)) / attackSwitchDist;
				if (alpha > 1) {
					alpha = 1;
				}
				else if (alpha < 0) {
					alpha = 0;
				}
			}

			*ptx = (1 - alpha) * centx + alpha * attPosx;
			*pty = (1 - alpha) * centy + alpha * attPosy;
			
			}
		} 
		
		break;
				    
		case willStrikerB:  {
			const double normalBackoffYDist = 140;
			const double mostForwardYPosn = FIELD_LENGTH - 125;
			const double mostBackYPosn = FIELD_LENGTH / 4.0;
			const double switchPosn = FIELD_LENGTH / 3.0;

			// Stay on the opposite side from the ball
			// Draw two lines, one from the defensive point to the center,
			// and the other from the center to the attack point
			// You will be somewhere on those lines.
			// In the defensive half, you will be normalBackoffYDist behind the ball in the y dir
			// in the offensive half, you will be further forward so that you are at the 
			// forward position when the ball hits the far end of the field.

			double lowX, lowY, highX, highY;
			double myY;

			myY = ballY - normalBackoffYDist;

			if (myY < mostBackYPosn) {
				myY = mostBackYPosn;
			}
			else if (myY > mostForwardYPosn) {
				myY = mostForwardYPosn;
			}

			if (myY > switchPosn) {
				// scale myY linearly so that we reach the endpoints
				const double topOfOldTravel = TARGET_GOAL_Y - WALL_THICKNESS
							      - GOALBOX_DEPTH / 2;
				const double oldYTravel = topOfOldTravel - (switchPosn + normalBackoffYDist);
				const double newYTravel = mostForwardYPosn - switchPosn;
				const double alpha = newYTravel / oldYTravel;

				myY = (myY - switchPosn) * alpha + switchPosn;
					    
			}

			if (myY < switchPosn) {
				if (ballX < (FIELD_WIDTH / 2.0)) {
					lowX = FIELD_WIDTH / 2.0 + 80;
				}
				else {
					lowX = FIELD_WIDTH / 2.0 - 80;
				}
				lowY = mostBackYPosn;
				highX = FIELD_WIDTH / 2.0;
				highY = switchPosn;
			}
			else {
				if (ballX < (FIELD_WIDTH / 2.0)) {
					highX = RIGHT_GOALBOX_EDGE_X + 10;
				}
				else {
					highX = LEFT_GOALBOX_EDGE_X - 10;
				}
				highY = mostForwardYPosn;
				lowX = FIELD_WIDTH / 2.0;
				lowY = switchPosn;
			}

			*pty = myY;

			double alpha = (myY - lowY) / (highY - lowY);
			*ptx = alpha * (highX - lowX) + lowX;
		} 
		
		break;
	}
}

void UNSW2004::getUNSW2004StrikerPointOffense(double *ptx, double *pty) {
	INSTRUMENT("UNSW2004::getUNSW2004StrikerPointOffense" << endl);

	double safeBallX = ballX;
	if (TARGET_GOAL_X - safeBallX) {
		safeBallX += 0.01;
	} //avoid divide by 0

	double m = (TARGET_GOAL_Y - ballY) / (TARGET_GOAL_X - safeBallX);
	double b = TARGET_GOAL_Y - m *TARGET_GOAL_X;
	if (m == 0) {
		m += 0.01;
	} //avoid divide by 0
	double xMatchingYourY = (gps->self().pos.y - b) / m;

	//if ball is far upfield, go to the static striker points
	if (ballY >= (FIELD_LENGTH - (FIELD_LENGTH / 4.0))) {
		if (gps->self().pos.x <= xMatchingYourY) {
			*ptx = LEFT_GOALBOX_EDGE_X - 18;
			*pty = LEFT_GOALBOX_EDGE_Y - 28;
		}
		else {
			*ptx = RIGHT_GOALBOX_EDGE_X + 18;
			*pty = RIGHT_GOALBOX_EDGE_Y - 28;
		}
	}
	//else stay downfiend and away from the ball
	else {
		if (gps->self().pos.x <= xMatchingYourY) {
			*ptx = LEFT_GOALBOX_EDGE_X - 18;
			*pty = MIN(FIELD_LENGTH - 78, ballY - 30);
		}
		else {
			*ptx = RIGHT_GOALBOX_EDGE_X + 18;
			*pty = MIN(FIELD_LENGTH - 78, ballY - 30);
		}
	}
}

void UNSW2004::getUNSW2004StrikerPointDefense(double *ptx, double *pty) {
	INSTRUMENT("UNSW2004::getUNSW2004StrikerPointDefense" << endl);
	// note that Will's striker strategy doesn't use this function
	if (ballX < (FIELD_WIDTH / 2.0)) {
		*ptx = FIELD_WIDTH / 2.0 + 80;
		*pty = WALL_THICKNESS + 140;
	}
	else {
		*ptx = FIELD_WIDTH / 2.0 - 80;
		*pty = WALL_THICKNESS + 140;
	}
}

bool UNSW2004::isUNSW2004ClosestToStrikerPoint(double strikerPtX,
		double strikerPtY) {
	INSTRUMENT("UNSW2004::isUNSW2004ClosestToStrikerPoint" << endl);
	// determine own distance to striker point
	const Vector &own = gps->self().pos;
	double ownDistToStrikerPt = sqrt(SQUARE(strikerPtX - own.x)
			+ SQUARE(strikerPtY - own.y));

	int numTeammatesCloserToStrikerPt = 0;

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;

		// ignore goalie
		if (tm.counter <= 0 || info.amGoalie != 0)
			continue;

		// ignore yourself
		if (i == PLAYER_NUMBER - 1)
			continue;

		// ignore teammate who's sending 'have ball' signal
		if (info.haveBall != 0)
			continue;    

		double tmDistToStrikerPt = sqrt(SQUARE(strikerPtX - tm.pos.x)
				+ SQUARE(strikerPtY - tm.pos.y));

		if (tmDistToStrikerPt < ownDistToStrikerPt) {
			numTeammatesCloserToStrikerPt++;
		}
	}

	if (numTeammatesCloserToStrikerPt > 0) {
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////
// backoff and supporter functions
/////////////////////////////////////////////////////////////////////////

//given the local heading and global position of a teammate (ie you get this info from vision)
//try to match it to one of the gps teammates and return its PLAYER NUMBER ie a number between 1-4
//this however doesnt match to the goalie (because this is a tailored function for the backoff)
//and doesnt match if the distance difference between the vision tm and the closest gps tm is greater than 50 cm
//or if the heading error is greater than 40 degrees 
//
//returns -1 if it cant find a match

int UNSW2004::matchVisualTeammateToGpsTeammate(double headToTeammate, double teammatex, double teammatey) {
	INSTRUMENT("UNSW2004::matchVisualTeammateToGpsTeammate" << endl);

	double distToVisTm = getDistanceBetween(gps->self().pos.x, gps->self().pos.y, teammatex, teammatey);
	int bestTeammateSoFarIndex = -1;
	double bestTeammatesDistance = LARGE_VAL;

	if (matchTMCout) {
		cout << "in the matching function" << endl;
	}
	if (matchTMCout) {
		cout << "matching vision teammate h d " << headToTeammate << " , "
			<< distToVisTm << endl;
	} 

	for (int i = 1; i <= NUM_TEAM_MEMBER; i++) {
		interpBehav info = (interpBehav) gps->tmObj(i - 1).behavioursVal;

		static const int headRange = 20;

		//find all the valid dogs that sort of match the heading to the dog your seeing
		if (i != PLAYER_NUMBER && gps->tmObj(i - 1).counter > 0 && info.amGoalie == 0
		    && gps->teammate(i).posVar <= get95CF(50) && ((abs(headToTeammate) <= headRange
		    && abs(gps->teammate(i).pos.head) < 30) ||   //FIX THIS UP
		    (gps->teammate(i).pos.head <headToTeammate + headRange
		    && gps->teammate(i).pos.head> headToTeammate - headRange))) {

			double tempDist = gps->teammate(i).pos.d;

			if (matchTMCout) {
				cout << "teammate " << i << " heading " << gps->teammate(i).pos.head
			   	     << " error " << abs(gps->teammate(i).pos.head - headToTeammate)
				     << endl;
				cout << "teammate " << i << " dist    " << tempDist << " error "
				     << abs(tempDist - distToVisTm) << endl;
			}

			//if the dog is really close, see how well he matches distance wise
			if (tempDist < 40) {
				if (abs(tempDist - distToVisTm) < abs(bestTeammatesDistance
							- distToVisTm)) {
					bestTeammateSoFarIndex = i;
					bestTeammatesDistance = tempDist;
					if (matchTMCout) {
						cout << "matching UNDER 40 cm .. error this "
						     << abs(tempDist - distToVisTm)
					   	     << " error prev best "
						     << abs(bestTeammatesDistance - distToVisTm)
						     << endl;
					}
				}
				else {
					if (matchTMCout) {
						cout << "didn't UNDER 40 cm   .. error this "
						     << abs(tempDist - distToVisTm)
						     << " error prev best "
						     << abs(bestTeammatesDistance - distToVisTm)
						     << endl;
					}
				}
			} 
			
			//else pick the closer dog (optimisation because this function is going to be used by the backoff routine)
			//ie if your seeing a dog, its most likely the closer one
			else {
				if (tempDist < bestTeammatesDistance) {
					bestTeammateSoFarIndex = i;
					bestTeammatesDistance = tempDist;
					if (matchTMCout) {
						cout << "matching OVER 40 cm  .. distance this "
					             << tempDist << " error prev best "
						     << abs(bestTeammatesDistance - distToVisTm)
						     << endl;
					}
				}
				else {
					if (matchTMCout) {
						cout << "didn't OVER  40 cm   .. distance this "
						     << tempDist << " error prev best "
						     << abs(bestTeammatesDistance - distToVisTm)
						     << endl;
					}
				}
			}
		}
	}

	if (abs(bestTeammatesDistance - distToVisTm) < 50) {
		if (matchTMCout)
			cout << "matched to player " << bestTeammateSoFarIndex << endl;
		return bestTeammateSoFarIndex;
	}
	else {
		if (matchTMCout)
			cout << "didn't match any teammates" << endl;
		return -1;
	}
}

// Daniel believes 55 is the best backoff distance.
bool UNSW2004::anyTeammateNearSeen() {
    for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
        if (vTeammate[i].cf > 0 && vTeammate[i].d <= 55)
            return true;
    }
    return false;
}

// Not implemented yet.
bool UNSW2004::anyOpponentNearSeen() {
    return false;
}

// Teammates plus opponents.
bool UNSW2004::anyObjectNearSeen() {
    if (anyTeammateNearSeen() || anyOpponentNearSeen())
        return true;
    return false;
}

static bool objectCout = true;

// Looking right and return true if seen any object.
bool UNSW2004::lookRight() {
    headtype = ABS_H;
    panx   = -90;
    tilty  = 0;
    cranez = -20;
    if (anyObjectNearSeen()) {
        if (objectCout)
            cout << "Right side has some objects!" << endl;
        return true;
    }
    return false;
}

// Looking left and return true if seen any object.
bool UNSW2004::lookLeft() {
    headtype = ABS_H;
    panx   = 90;
    tilty  = 0;
    cranez = -20;
    if (anyObjectNearSeen()) {
        if (objectCout)
            cout << "Left side has some objects!" << endl;\
        return true;
    }
    return false;
}

// Looking forward and return true if seen any object.
bool UNSW2004::lookForward() {
    headtype = ABS_H;
    panx   = 0;
    tilty  = 0;
    cranez = 0;
    if (anyObjectNearSeen()) {
        if (objectCout)
            cout << "Forward has some objects!" << endl;\
        return true;
    }
    return false;
}

static int lookCounter = 0;
static bool rightObject, leftObject, forwObject = false;

void UNSW2004::resetLookAround() {
    lookCounter = 0;
    rightObject = false;
    leftObject  = false;
    forwObject  = false;  
}

// Look around and react to what it sees.
void UNSW2004::lookAround() {
    lockMode = LookAround;
    lookCounter++;
    if (lookCounter < 50) {
        if (lookForward())
            forwObject = true;
        return;
    }
    if (lookCounter < 100) {
        if (lookLeft())
            leftObject = true;
        return;
    }
    if (lookCounter < 150) {
        if (lookRight())
            rightObject = true;
        return;
    }    
    lockMode = NoMode;
//    lookAroundBackOff(leftObject, rightObject, forwObject);
}

void UNSW2004::goBackward() {
    forward = -5;
    turnCCW = 0;
    left    = 0;
}

void UNSW2004::goRight() {
    forward = 0;
    turnCCW = 0;
    left    = -5;
}

void UNSW2004::goLeft() {
    forward = 0;
    turnCCW = 0;
    left = 5;    
}

void UNSW2004::lookAroundBackOff(bool left_, bool right_, bool forward_) {
    // Blocking objects in front.
    if (forward_) {
        // Blocking objects in front and right.
        if (right_) {
            // Blocking objects in front and right and left.
            if (left_) {
            }
            // Blocking objects in front and right and not left.
            else {
            }        
        }
        // Blocking objects in front and not right.
        else {
            // Blocking objects in front and not right and left.
            if (left_) {
            }
            // Blocking objects in front and not right and not left.
            else {
            }        
        }
    }
    // No blocking objects in front.
    else {
        // Np blocking objects in front and right.
        if (right_) {
            // No blocking objects in front and right and left.
            if (left_) {            
            }
            // No blocking objects in front and right and not left.
            else {            
            }        
        }
        // No blocking objects in front and not right.
        else {
            // No blocking objects in front and not right and left.
            if (left_) {            
            }
            // No blocking objects in front and not right and not left.
            else {            
            }        
        }        
    }           
}

// Ted: This function need to be revised because this backoff forward need a accurate gps.
//these values define angle between a player and the goal via the ball
//ie angle between player and goal with the ball as the pivot point jan
static const double aboveBallThreshold = 70; //less than this, your between ball and goal (in the way region)
static const double belowBallThreshold = 125; //more than this your behind ball and goal (attacking region)

void UNSW2004::doUNSW2004BackOffForward(double headToTeammate, double teammatex, double teammatey) {
	INSTRUMENT("UNSW2004::doUNSW2004BackOffForward" << endl);

	if (lostBall >= vBallAnywhere) {
		boDoSupport();
	}

	if (matchTMCout) {
		cout << "************* matchTM stuff ************* " << endl;
	}

	//if you can match this teammate to one of the gps teammates
	//then overwrite the details of that teammate with his gps details
	//and run the backoff tree with those details instead 
	int matchedTmPlayerNumber = matchVisualTeammateToGpsTeammate(headToTeammate, teammatex, teammatey);
	if (useTmMatching && matchedTmPlayerNumber > 0) {
		if (matchTMCout) {
			cout << "vision details hxy \t" << headToTeammate << " , \t"
				<< teammatex << " , \t" << teammatey << endl;
		}

		headToTeammate = gps->teammate(matchedTmPlayerNumber).pos.head;
		teammatex = gps->teammate(matchedTmPlayerNumber, 'g').pos.x;
		teammatey = gps->teammate(matchedTmPlayerNumber, 'g').pos.y;

		if (matchTMCout) {
			cout << "gps details hxy    \t" << headToTeammate << " , \t"
				<< teammatex << " , \t" << teammatey << endl;
		}
	}

	//default desired direction is towards target goal
	double desDir = getUNSW2004DKD();

	double you = absAngleBetweenTwoPointsFromPivotPoint(gps->self().pos.x, gps->self().pos.y,
							    TARGET_GOAL_X, TARGET_GOAL_Y, ballX,
							    ballY);

	double teammate;

	//if you know the gps details of the teammate, and he knows where the ball is
	//let him calculate his angle to the ball
	if (useTmMatching && matchedTmPlayerNumber > 0 && gps->tmBallObj(matchedTmPlayerNumber - 1).posVar
	    < get95CF(50)) {
		teammate = absAngleBetweenTwoPointsFromPivotPoint(teammatex, teammatey, TARGET_GOAL_X,
								  TARGET_GOAL_Y, gps->tmBallObj(matchedTmPlayerNumber - 1).pos.x,
								  gps->tmBallObj(matchedTmPlayerNumber - 1).pos.y);
	}
	else {
		teammate = absAngleBetweenTwoPointsFromPivotPoint(teammatex, teammatey, TARGET_GOAL_X, TARGET_GOAL_Y,
								  ballX, ballY);
	}                      

	if (tmPosBackOffEyes) {
		if (teammate <= aboveBallThreshold) {
			tmTopRegionEyes;
		}
		else if (teammate < belowBallThreshold) {
			tmMidRegionEyes;
		}
		else if (teammate >= belowBallThreshold) {
			tmBotRegionEyes;
		}
		else {
			//leds(2,2,2,2,2,2);
		}
	}

	if (tmPosBackOffEyes) {
		if (you <= aboveBallThreshold) {
			myTopRegionEyes;
		}
		else if (you < belowBallThreshold) {
			myMidRegionEyes;
		}
		else if (you >= belowBallThreshold) {
			myBotRegionEyes;
		}
		else {
			//leds(2,2,2,2,2,2);
		}
	}

	double strikerPtX = 0; 
	double strikerPtY = 0;
	getUNSW2004StrikerPoint(&strikerPtX, &strikerPtY);

	//if your in the attacking region and they are not then attack
	if (you >= belowBallThreshold && teammate < belowBallThreshold) {
		boMaybeNormal(matchedTmPlayerNumber, teammate, desDir, headToTeammate, you);
	}
    // Striker case 2.
	else if (hackStrikerIntoSupport && isUNSW2004ToAssumeStrikerRole(strikerPtX, strikerPtY)) {
		// Find striker point
		// if UNSW2004 should assume striker role
		if (roleEyes)
			strikerRoleEyes;
		if (roleCout)
			cout << "ROLE        : striker" << endl;
        myRole = STRIKER2;
		setUNSW2004Striker();
		if (doHailMary && (gps->getBall(GLOBAL).y > FIELD_LENGTH * 0.5) && (getNumTeammatesOnSide() >= 2)
		    && (getNumOpponentsOnSide() <= 1)) {
			doUNSW2004HailMaryReceive();
		}
		else {
			doUNSW2004StrikerForward(strikerPtX, strikerPtY);
		}
		clipUNSW2004FireballPan();
		return;
	}
	//if they are in the attacking region and you are not then support
	else if (teammate >= belowBallThreshold && you < belowBallThreshold) {
		//make sure your going in the direction thats getting out of his way.. awesome assignment
		//ie not the stay on your side of the line thing
		boDoSupport();
	}
	//if your both in the above ball region, and your teammate is infront of you
	//get out of the way incase he does a 180 turn kick etc
	//or incase you got it wrong and he actually is in the attacking region
	else if (teammate <= aboveBallThreshold
			&& you <= aboveBallThreshold
			&& abs(headToTeammate) < 22) {
		//make sure your going in the direction thats getting out of his way.. 
		//ie not the stay on your side of the line thing
		boDoSupport();
	}
	//if you are both in the attacking region
	else if (you >= belowBallThreshold && teammate >= belowBallThreshold) {
		//if you did actually find a gps match
		if (matchedTmPlayerNumber > 0) {
			interpBehav info = (interpBehav)
				gps->tmObj(matchedTmPlayerNumber - 1).behavioursVal;

			//if you can see ball and they cant then attack
			if (vBall.cf > 4 && info.ballDist >= LOSTBALL_DIST) {
				boMaybeNormal(matchedTmPlayerNumber,
						teammate,
						desDir,
						headToTeammate,
						you);
				return;
			} 
			//if you cant see ball and they can then support
			else if (vBall.cf <= 4 && info.ballDist < LOSTBALL_DIST) {
				boDoSupport();
				return;
			}
			//if your alot closer then attack
			else if (info.ballDist - vBall.d >= 20) {
				boMaybeNormal(matchedTmPlayerNumber,
						teammate,
						desDir,
						headToTeammate,
						you);
				return;
			} 
			//if they are alot closer then support
			else if (vBall.d - info.ballDist >= 20) {
				boDoSupport();
				return;
			}
			//if your heading is alot better than theirs then attack
			else if (you - teammate >= 30) {
				boMaybeNormal(matchedTmPlayerNumber,
						teammate,
						desDir,
						headToTeammate,
						you);
				return;
			}
			//if their heading is alot better than yours then support
			else if (teammate - you >= 30) {
				boDoSupport();
				return;
			} 
			//if your player number is smaller than theirs then attack
			else if (PLAYER_NUMBER < matchedTmPlayerNumber) {
				boMaybeNormal(matchedTmPlayerNumber,
						teammate,
						desDir,
						headToTeammate,
						you);
				return;
			}
			//else your player number is higher than theirs so support
			else {
				boDoSupport();
				return;
			}
		}

		//if you fall out of the for loop
		//ie wireless is down or you cant find anyone in gps that matched
		//the teammate you detected in vision
		boDoGetBehind(desDir, headToTeammate, you, teammate, false);
		return;
	}
	//in all other cases get behind ball
	else {
		boDoGetBehind(desDir, headToTeammate, you, teammate, false);
		return;
	}
}

//extention from doUNSW2004BackOffForward so theres not too many copy and paste jobs
void UNSW2004::boMaybeNormal(int matchedTmPlayerNumber,
		double teammate,
		double desDir,
		double headToTeammate,
		double you) {
	INSTRUMENT("UNSW2004::boMaybeNormal" << endl);
	if (useTmMatching && matchedTmPlayerNumber > 0) {
		interpBehav info = (interpBehav)
			gps->tmObj(matchedTmPlayerNumber - 1).behavioursVal;
		if (!info.backingOff && info.ballDist <= 15) {
			if (teammate >= belowBallThreshold) {
				boDoSupport();
				if (tmPosBackOffEyesExt)
					tmExtSupport;
				return;
			}
			else {
				boDoGetBehind(desDir, headToTeammate, you, teammate, true);
				if (tmPosBackOffEyesExt)
					tmExtGetBehind;
				return;
			}
		}
		else {
			if (tmPosBackOffEyesExt)
				tmExtNotCloseEnough;
			boDoNormal();
			return;
		}
	}
	else {
		boDoNormal();
		return;
	}
}

void UNSW2004::boDoNormal() {
	INSTRUMENT("UNSW2004::boDoNormal" << endl);
	if (backOffCout) {
		cout << "normal forward" << endl;
	}
	if (tmPosBackOffEyes) {
		tmAttackEyes;
	}
	if (backOffEyes) {
		forcedAttackBOEyes;
	}
	lastBackOffStrategy = BO_ATTACK;
	doUNSW2004NormalForward();
}

void UNSW2004::boDoSupport() {
	INSTRUMENT("UNSW2004::boDoSupport" << endl);
	if (backOffCout) {
		cout << "support forward" << endl;
	}
	setUNSW2004BackingOff();
	if (tmPosBackOffEyes) {
		tmSupportEyes;
	}
	if (backOffEyes) {
		supportBOEyes;
	}
	lastBackOffStrategy = BO_BACKOFF;
	doUNSW2004SupportForward(ballX, ballY);
}

/*
//the boolean stepBack specifies if you always want the getbehind to  also
//add the negative away vector.
//Otherwise it only adds it in certain cases.
*/
void UNSW2004::boDoGetBehind(double desDir, double headToTeammate, double you, double teammate, bool stepBack) {
	INSTRUMENT("UNSW2004::boDoGetBehind" << endl);
	if (backOffCout) {
		cout << "backoff get behind" << endl;
	}
	setUNSW2004BackingOff();
	if (tmPosBackOffEyes) {
		tmGetBehindEyes;
	}
	if (backOffEyes) {
		getBehindBOEyes;
	}
	lastBackOffStrategy = BO_GETBEHIND;
	doUNSW2004BackoffGetBehindBall(desDir, headToTeammate, you, teammate, stepBack);
}

//return the abs angle between 2 points, taking this pivot point as the origin
//eg of use, a is robot, b is goal, pivot is ball
//ie the angle between the robot and the goal via the ball ie angle a-pivot-b
double UNSW2004::absAngleBetweenTwoPointsFromPivotPoint(double pointax, double pointay, double pointbx,
						       double pointby, double pivotx,  double pivoty) {
	INSTRUMENT("UNSW2004::absAngleBetweenTwoPointsFromPivotPoint" << endl);

	double asquared = SQUARE(pointbx - pointax) + SQUARE(pointby - pointay);
	double bsquared = SQUARE(pointbx - pivotx) + SQUARE(pointby - pivoty);
	double csquared = SQUARE(pivotx - pointax) + SQUARE(pivoty - pointay);
	double twobc = 2.0 * sqrt(bsquared) * sqrt(csquared);
	if (abs(twobc) <= 0.0001) {
		twobc += 0.01;
	}

	double result = abs(RAD2DEG(acos((bsquared + csquared - asquared) / twobc)));

	return result;
}    

void UNSW2004::doUNSW2004BackoffGetBehindBall(double desDir, double headToTeammate, double angYouToBallToGoal, double angTeammateToBallToGoal, bool stepBack) {
	INSTRUMENT("UNSW2004::doUNSW2004BackoffGetBehindBall" << endl);
	int dir;
	double radius;

	if (angYouToBallToGoal <= aboveBallThreshold
			&& angTeammateToBallToGoal <= aboveBallThreshold) {
		if (headToTeammate > 0) {
			dir = ANTICLOCKWISE;
		}
		else {
			dir = CLOCKWISE;
		}
	}
	else {
		if (gps->tGoal().pos.head >= 0) {
			dir = ANTICLOCKWISE;
		}
		else {
			dir = CLOCKWISE;
		}
	}

	if (angYouToBallToGoal <= aboveBallThreshold) {
		radius = 40;
	}
	else if (angYouToBallToGoal <= belowBallThreshold) {
		radius = 30;
	}
	else {
		radius = 30;
	}
	//if your further from the correct angle than your teammate,
	//do the negative vector thing in the get behind ball
	if (angYouToBallToGoal + 10 <= angTeammateToBallToGoal || stepBack) {
		boGetBehindBall(radius, desDir, dir, ballX, ballY, ballH, true);
	}
	//else dont
	else {
		boGetBehindBall(radius, desDir, dir, ballX, ballY, ballH, false);
	}
}    

//this is a get behind ball that is customised for backoff
//this is pretty similar to the other get behind ball
//except that it adds an away vector so that your getting behind and moving away if you set the stepBack boolean
//by default the boolean is false
void UNSW2004::boGetBehindBall(double sd, double aa, int turndir, double ballX, double ballY, double ballH, bool stepBack) {
	INSTRUMENT("UNSW2004::boGetBehindBall" << endl);

	if (tmPosBackOffEyes && stepBack) {
		gbbStepBackEyes;
	}

	CurObject target;      // circling point
	CurObject attackPos;   // attacking point (lined up with attack angle)

	//default the pivot point to vision ball
	//unless otherwise specified
	//(default values for ballX ballY ballH are the invalid flag)
	double pivotx = vBall.x;
	double pivoty = vBall.y;
	double pivoth = vBall.h;
	if (ballX != INVALID_FLAG || ballH != INVALID_FLAG || ballH != INVALID_FLAG) {
		pivotx = ballX;
		pivoty = ballY;
		pivoth = ballH;
	}

	double ballToMe = saHeadingBetween(pivotx, pivoty, gps->self().pos.x, gps->self().pos.y);

	saGetPointRelative(pivotx, pivoty, utilNormaliseAngle(aa + 180.0), sd, &attackPos.x, &attackPos.y);
	saFindAngleDist(attackPos.x, attackPos.y, &attackPos.h, &attackPos.d);

	/* edge override */
	if (backOffCout) {
		cout << "ball (" << gps->getBall(GLOBAL).x << ","
			<< gps->getBall(GLOBAL).y << ")";
	}
	if (utilBallOnLEdge()) {
		// LRedge
		if (backOffCout)
			cout << "Left edge override ";
		turndir = CLOCKWISE;
	}
	else if (utilBallOnREdge()) {
		if (backOffCout)
			cout << "Right edge override ";
		turndir = ANTICLOCKWISE;
	}
	else if (utilBallOnTEdge() && gps->getBall(GLOBAL).x < FIELD_WIDTH / 2.0) {
		//top edge cases
		if (backOffCout)
			cout << "Top edge override ";
		turndir = CLOCKWISE;
	}
	else if (utilBallOnTEdge() && gps->getBall(GLOBAL).x >= FIELD_WIDTH / 2.0) {
		if (backOffCout)
			cout << "Top edge override ";
		turndir = ANTICLOCKWISE;
	}
	else if (gps->getBall(GLOBAL).y < BACKOFF_BALL_BOTTOM_EDGE_DISTANCE
			&& gps->getBall(GLOBAL).x < FIELD_WIDTH / 2.0) {
		//bottom edge cases
		if (backOffCout)
			cout << "Bottom edge override ";
		turndir = CLOCKWISE;
	}
	else if (gps->getBall(GLOBAL).y < BACKOFF_BALL_BOTTOM_EDGE_DISTANCE
			&& gps->getBall(GLOBAL).x >= FIELD_WIDTH / 2.0) {
		if (backOffCout)
			cout << "Bottom edge override ";
		turndir = ANTICLOCKWISE;
	}

	if (turndir == CLOCKWISE) {
		if (backOffCout)
			cout << "CLOCKWISE!" << endl;
		saGetPointRelative(pivotx,
				pivoty,
				utilNormaliseAngle(ballToMe - 70.0),
				sd,
				&target.x,
				&target.y);
	}
	else {
		if (backOffCout)
			cout << "ANTICLOCKWISE!" << endl;
		saGetPointRelative(pivotx,
				pivoty,
				utilNormaliseAngle(ballToMe + 70.0),
				sd,
				&target.x,
				&target.y);
	}
	saFindAngleDist(target.x, target.y, &target.h, &target.d);

	double ballDist = getDistanceBetween(pivotx, pivoty, gps->self().pos.x, gps->self().pos.y);

	if (target.d <attackPos.d || abs(aa - gps->self().h)> 70.0) {
		turnCCW = CLIP(pivoth / 1.5, 20.0);

		if (stepBack) {
			forward = CLIP(target.d, 6.5) * cos(DEG2RAD(target.h)) - 4;
			if (forward < -6) {
				forward = -6;
			}
			if (forward > 6) {
				forward = 6;
			}
		}
		else {
			forward = CLIP(target.d, 6.5) * cos(DEG2RAD(target.h));
		}

		double desLeft = target.d *sin(DEG2RAD(target.h));

		if (ABS(forward) < 3.0) {
			if (desLeft >= 0) {
				left = MIN(6.0, desLeft);
			}
			else {
				left = MAX(-6.0, desLeft);
			}
		}
		else {
			if (desLeft >= 0) {
				left = MIN(5.0, desLeft);
			}
			else {
				left = MAX(-5.0, desLeft);
			}
		}
	}
	else {
		turnCCW = CLIP(pivoth / 2.0, 20.0);
		if (stepBack) {
			forward = CLIP(attackPos.d, 6.5) * cos(DEG2RAD(attackPos.h)) - 4;
			if (forward < -6) {
				forward = -6;
			}
			if (forward > 6) {
				forward = 6;
			}
		}
		else {
			forward = CLIP(attackPos.d, 6.5) * cos(DEG2RAD(attackPos.h));
		}    

		double desLeft = attackPos.d *sin(DEG2RAD(attackPos.h));

		if (ABS(forward) < 3.0) {
			if (desLeft >= 0) {
				left = MIN(6.0, desLeft);
			}
			else {
				left = MAX(-6.0, desLeft);
			}
		}
		else {
			if (desLeft >= 0) {
				left = MIN(5.0, desLeft);
			}
			else {
				left = MAX(-5.0, desLeft);
			}
		}
	}
}

// Ted: I doubt this works.
void UNSW2004::doUNSW2004SupportForward(double ptx, double pty) {
	INSTRUMENT("UNSW2004::doUNSW2004SupportForward" << endl);

    // Need to check utilTeammateInFace.
	if (lostBall > vBallAnywhere && utilTeammateInFace()) {
		forward = -6;
		left = 0;
		turnCCW = 0;
		if (backOffCout)
			cout << "Support just backoff" << endl;
		return;
	}

	if (activeLocaliseTimer) {
		//activeGpsLocalise(false);
		smartActiveGpsLocalise();
		if (activeLocaliseTimer == 1) {
			disallowActiveLocalise = 75;
			//reset the lost ball so that you will look to gps ball on return from active localise
			//if (lostBall > vBallLost) {
			//    lostBall = 0;
			//}
		}
	}
	else if (checkLastBallPos) {
		headtype = lastHeadType;
		tilty = lastHeadTilty;
		panx = lastHeadPanx;
		cranez = lastHeadNullz;
	}
	else if (!disallowActiveLocalise && ballD > 60) {
		if (gps->canSee(vobBall)) {
			doUNSW2004TrackVisualBall();
			checkLastBallPos = localiseLockLength + 6;
			lastHeadType = headtype;
			lastHeadTilty = tilty;
			lastHeadPanx = panx;
			lastHeadNullz = cranez;
		}

		//setNewBeacons(LARGE_VAL, 90);            
		//activeGpsLocalise(false);
		smartSetBeacon();
		smartActiveGpsLocalise();
		activeLocaliseTimer = localiseLockLength;
	}

	static const int xedgeBuffer = WALL_THICKNESS + 30;
	static const int yedgeBuffer = WALL_THICKNESS + 50;
	static const int xoffset = 70;
	static const int yoffset = -50;

	if (ptx == TARGET_GOAL_X) {
		ptx += 0.01;
	} //avoid divide by 0

	double m = (TARGET_GOAL_Y - pty) / (TARGET_GOAL_X - ptx);
	double b = TARGET_GOAL_Y - m *TARGET_GOAL_X;

	if (m == 0) {
		m += 0.01;
	} //avoid divide by 0

	double xMatchingYourY = (gps->self().pos.y - b) / m;
	double x, y, h;
	// position to back-off in top horizontal strip
	// position: centre of field
	if (pty >= (3 * FIELD_LENGTH / 4.0)) {
		if (gps->self().pos.x <= xMatchingYourY) {
			x = MAX(xedgeBuffer, ptx - 60);
			y = MIN(FIELD_LENGTH - yedgeBuffer, pty - 80);
			h = NormalizeAngle_0_360(RAD2DEG(atan2(pty - gps->self().pos.y,
							ptx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}
		else {
			x = MIN(FIELD_WIDTH - xedgeBuffer, ptx + 60);
			y = MIN(FIELD_LENGTH - yedgeBuffer, pty - 80);
			h = NormalizeAngle_0_360(RAD2DEG(atan2(pty - gps->self().pos.y,
							ptx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}
	}

	//if hes not a goalie get between him and the goal??
	//
	// position to back-off in bottom horizontal strip
	// position: offset -80, 80 from ball
	else if (pty <= (FIELD_LENGTH / 4.0)
			&& pty >= (WALL_THICKNESS + GOALBOX_DEPTH)) {
		if (gps->self().pos.x <= xMatchingYourY) {
			x = MAX(xedgeBuffer, ptx - 80);
			y = MAX(yedgeBuffer, pty + 80);
			h = NormalizeAngle_0_360(RAD2DEG(atan2(pty - gps->self().pos.y,
							ptx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}
		else {
			x = MIN(FIELD_WIDTH - xedgeBuffer, ptx + 80);
			y = MAX(yedgeBuffer, pty + 80);
			h = NormalizeAngle_0_360(RAD2DEG(atan2(pty - gps->self().pos.y,
							ptx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}
		/*
		   double xbOffset = 0;
		   double ybOffset = 80;
		//if (ptx >= LEFT_GOALBOX_EDGE_X-10 && ptx <= RIGHT_GOALBOX_EDGE_X+10) {
		if (ptx >= FIELD_WIDTH/2.0) {
		if (gps->self().pos.x <= xMatchingYourY) {
		leds(1,2,1,1,2,1);
		xbOffset = -70;
		ybOffset = 0;
		} else {
		leds(2,1,1,2,1,1);
		xbOffset = 60;
		ybOffset = -50;
		}
		} else {
		if (gps->self().pos.x <= xMatchingYourY) {
		leds(2,1,1,2,1,1);
		xbOffset = -60;
		ybOffset = -50;
		} else {
		leds(1,2,1,1,2,1);
		xbOffset = 70;
		ybOffset = 0;
		}
		}
		//}
		x = MAX(xedgeBuffer, ptx + xbOffset);
		y = MAX(yedgeBuffer, pty + ybOffset);
		h = NormalizeAngle_0_360(RAD2DEG(atan2(pty - gps->self().pos.y, ptx - gps->self().pos.x)));
		saGoToTargetFacingHeading(x, y, h);
		*/
	}
	else if (pty < (WALL_THICKNESS + GOALBOX_DEPTH)) {
		if (gps->self().pos.x <= xMatchingYourY) {
			x = MAX(xedgeBuffer, ptx - 80);
			y = MAX(yedgeBuffer, pty + 80);
			h = NormalizeAngle_0_360(RAD2DEG(atan2(pty - gps->self().pos.y,
							ptx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}
		else {
			x = MIN(FIELD_WIDTH - xedgeBuffer, ptx + 80);
			y = MAX(yedgeBuffer, pty + 80);
			h = NormalizeAngle_0_360(RAD2DEG(atan2(pty - gps->self().pos.y,
							ptx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}
	}

	// position to back-off in middle horizontal strip
	// position: offset 80, -50 from ball
	else {
		//if (pty < (3*FIELD_LENGTH/4.0) && pty > (FIELD_LENGTH/4.0)) {
		if (gps->self().pos.x <= xMatchingYourY) {
			x = MAX(xedgeBuffer, ptx - xoffset);
			y = MIN(FIELD_LENGTH - yedgeBuffer, pty + yoffset);
			h = NormalizeAngle_0_360(RAD2DEG(atan2(pty - gps->self().pos.y,
							ptx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}
		else {
			x = MIN(FIELD_WIDTH - xedgeBuffer, ptx + xoffset);
			y = MIN(FIELD_LENGTH - yedgeBuffer, pty + yoffset);
			h = NormalizeAngle_0_360(RAD2DEG(atan2(pty - gps->self().pos.y,
							ptx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}
	}
	//if (backOffCout) cout << "goto (" << x << "," << y << ") facing " << h << endl;

	//////////////////
	// the conditions above split the field into 3 sections based on y
	// the conditions below split the field into sections based on x
	// ie it handles special cases when your near the edges
	// in some cases it may overwrite offset positions specified by the conditions above
	//////////////////

	//if ball is on the edge then stick to the edge behind just incase the ball pops out
	//ie the dummy half move
	static const int onEdgeOffset = WALL_THICKNESS + 20;
	static const int onEdgeOffsetNearOwnGoal = WALL_THICKNESS + 50;
	if (pty > (FIELD_LENGTH / 4.0)) {
		if (ptx <= onEdgeOffset) {
			if (backOffEyes)
				edgeBOEyes;
			x = 10;
			y = MIN(FIELD_LENGTH - yedgeBuffer, pty - 80);
			h = NormalizeAngle_0_360(RAD2DEG(atan2(pty - gps->self().pos.y,
							ptx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}
		else if (ptx >= FIELD_WIDTH - onEdgeOffset) {
			if (backOffEyes)
				edgeBOEyes;
			x = FIELD_WIDTH - 10;
			y = MIN(FIELD_LENGTH - yedgeBuffer, pty - 80);
			h = NormalizeAngle_0_360(RAD2DEG(atan2(pty - gps->self().pos.y,
							ptx - gps->self().pos.x)));
			saGoToTargetFacingHeading(x, y, h);
		}
		if (backOffCout)
			cout << "goto (" << x << "," << y << ") facing " << h << endl;
	} 
	/*
	   else {
	   double awayFromOwnGoal = NormalizeAngle_0_360(RAD2DEG( atan2((pty - 0), (ptx - FIELD_WIDTH/2.0))));
	   int dir = (isOnLeftOfLine(OWN_GOAL_X, OWN_GOAL_Y, ballX, ballY)) ? ANTICLOCKWISE : CLOCKWISE;
	   if (ptx <= onEdgeOffsetNearOwnGoal) {
	   if (backOffEyes) edgeBOEyes;
	   boGetBehindBall (40, awayFromOwnGoal, dir, ptx, pty, ballH);
	   } else if (ptx >= FIELD_WIDTH-onEdgeOffsetNearOwnGoal) {
	   if (backOffEyes) edgeBOEyes;
	   boGetBehindBall (40, awayFromOwnGoal, dir, ptx, pty, ballH);
	   } else {
	   if (backOffCout) cout << "goto (" << x << "," << y << ") facing " << h << endl;
	   }
	   }
	   */
	}
	//#endif


	/////////////////////////////////////////////////////////////////////////
	// normal forward functions
	/////////////////////////////////////////////////////////////////////////

    static bool normalDebug = false;

	void UNSW2004::doUNSW2004NormalForward() {
		INSTRUMENT("UNSW2004::doUNSW2004NormalForward" << endl);

        if (normalDebug)
            cout << "Call doUNSW2004NormalForward()!" << endl;

		// If locked into active localise, continue to localise.
		if (activeLocaliseTimer) {
            if (normalDebug)
                cout << "Active Localise in doUNSW2004NormalForward()!" << endl;
			// activeGpsLocalise(false);
			smartActiveGpsLocalise();
			hoverToBall(gps->getBall(LOCAL).d, gps->getBall(LOCAL).head);

			if (activeLocaliseTimer == 1) {
				disallowActiveLocalise = 40;
				//reset the lost ball so that you will look to gps ball on return from active localise
				//if (lostBall > vBallLost) {
				//    lostBall = 0;
				//}
			}                    

			return;
		}

		// If lost ball, perform find ball.
		if (lostBall > vBallLost && ballSource != WIRELESS_BALL) {
            if (normalDebug)
                cout << "Nathan and Eric doggy case!" << endl;
			// if you've lost the ball, figure out which way to spin first

			// first time in series
			if (lostBall == vBallLost + 1) {
				setLocateBallParams();
			}

			if (numTeammatesCommunicating() == 0) {
				locateBall(-2, -2);
			}
			else {
				locateBall(-1, -1);
			}    
			//THIS IS REALLY DODGE!! MAKE ERIC & NATHAN FIX IT !!!
			//they wont all go to the same spot.. cause theres an override in locateBall
			return;
		}

		// if we can start to active localise
		int oppNum = 0;
		const Vector &opp = gps->getClosestOppPos(LOCAL, &oppNum);
		int minVarOppNum = getMinVarOppNum();

		if (ballSource == VISION_BALL && vBall.d > 65 && ABS(vBall.h) < 5
		    && ((gps->getOppCovMax(oppNum) < get95CF(50) && opp.d > 40)
		    || gps->getOppCovMax(minVarOppNum) > get95CF(50))
		    && !disallowActiveLocalise) {
            if (normalDebug)
                cout << "Normal forward using vision ball case!" << endl;
			//so far normal forward sets the lastballpos timer but doesn't use it
			if (gps->canSee(vobBall)) {
				doUNSW2004TrackVisualBall();
				checkLastBallPos = localiseLockLength + 6;
				lastHeadType = headtype;
				lastHeadTilty = tilty;
				lastHeadPanx = panx;
				lastHeadNullz = cranez;
			}

			/*
			// sort beacons by num of frames seen in the last 4 seconds
			// instead of by pan values
			bool sortBeaconsByPan = false;
			setNewBeacons(LARGE_VAL, 40, sortBeaconsByPan);
			*/

			//setNewBeacons(LARGE_VAL, 50);
			//activeGpsLocalise(false);
			smartSetBeacon();
			smartActiveGpsLocalise();
			activeLocaliseTimer = localiseLockLength;
			hoverToBall(gps->getBall(LOCAL).d, gps->getBall(LOCAL).head);
			return;
		}

		// if currently using wireless ball
		if (ballSource == WIRELESS_BALL) {
			if (wirelessEyes)
				usingWirelessEyes;
            if (normalDebug)
                cout << "Normal forward using wireless ball case!" << endl;
			// if close enough to share ball, perform find ball
			if (ballD < UNSW2004WirelessBallDistThreshold) {
				if (wirelessEyes)
					inTheCircleEyes;
				if (storeWirelessBallDistThreshold == 0
						&& framesSinceInVarianceCircle > 30) {
					setLocateBallParams(); //this should only happen once ...
					storeWirelessBallDistThreshold = UNSW2004WirelessBallDistThreshold
						+ 50;
					lostBall = vBallLost + 2;
				}
				framesSinceInVarianceCircle = 0;
				locateBall(FIELD_WIDTH / 2.0, gps->self().pos.y); //THIS IS ALSO REALLY DODGE!!
				return;
			}
			else {
				storeWirelessBallDistThreshold = 0;
				headFindBall();
				hoverToBall(ballD, ballH);
				return;
			}
		}

		if (ballSource == GPS_BALL) {
            if (normalDebug)
                cout << "Normal forward using gps ball case!" << endl;
			hoverToBall(gps->getBall(LOCAL).d, gps->getBall(LOCAL).head);
			if (lostBall <= 3) {
				headtype = ABS_XYZ;
				panx = fstVisBallDist * sin(DEG2RAD(fstVisBallHead));
				tilty = BALL_RADIUS;
				cranez = fstVisBallDist * cos(DEG2RAD(fstVisBallHead));
#ifndef OFFLINE
				if (abs(hPan - *desiredPan) > 8 || abs(hTilt - *desiredTilt) > 8)
					lostBall = 0;
#endif // OFFLINE
			}
			else
				doUNSW2004TrackGpsBall();
			return;
		}

		// choose appropriate approach ball behaviour
		doUNSW2004AttackBall();
	}

    static bool attackBallCout = false;

    // This method define actions needed to approach the ball.
	void UNSW2004::doUNSW2004AttackBall() {
		INSTRUMENT("UNSW2004::doUNSW2004AttackBall" << endl);
        if (attackBallCout)
            cout << "Call doUNSW2004AttackBall()!" << endl;
		
		if (kickingOff && isUNSW2004KickOffAttacker()) {
            if (attackBallCout)
                cout << "Normal forward kicking off case!" << endl;
			firePawKick(FIRE_PAWKICK_AUTO);
			return;
		}

		int oppNum = 0;
		const Vector &opp = gps->getClosestOppPos(LOCAL, &oppNum);
		//double reqAccuracy = requiredAccuracy(25);

		//local coords
		//double minGoalHead = gps->tGoal().pos.head - reqAccuracy;
		//double maxGoalHead = gps->tGoal().pos.head + reqAccuracy;

		//////////////////////////////////////////////////////////////////////////////////////////
		// PAWKICK ON EDGE CASES
		//////////////////////////////////////////////////////////////////////////////////////////

		if (doOffEdgeKick) {
			if (offEdgeKickEyes) {
				// offedge debugging leds
				int l1, l2, l3, l4, l5, l6;
				l1 = l2 = l3 = l4 = l5 = l6 = 1;
				cout << "*****\n";
				// l6 = 2;
				if (ballSource == VISION_BALL && vBall.y >= FIELD_LENGTH * 0.4
				    && vBall.y <= FIELD_LENGTH - 40) {
					cout << "ball in position: ";
					if (utilBallOnLEdge()) {
						l1 = 2;
						cout << "l\n";
					}
					else if (utilBallOnREdge()) {
						l4 = 2;
						cout << "r\n";
					}
					else {
						cout << " bx: " << gps->getBall(GLOBAL).x << "\n";
					}
				}
				else {
					cout << " vby: " << vBall.y << "\n";
				}
				if (gps->self().h < 120 && gps->self().h> 60
				    && gps->self().posVar < get95CF(75)) {
					cout << "dog in position: ";
					if (utilIsOnLEdge()) {
						l2 = 2;
						cout << "l\n";
					}
					else if (utilIsOnREdge()) {
						l5 = 2;
						cout << "r\n";
					}
					else {
						cout << " x: " << gps->self().pos.x << "\n";
					}
				}
				else {
					cout << "gh: " << gps->self().h << " posVar: " << gps->self().posVar
						<< "\n";
				}
				if (opp.d >= 60 && getNumOpponentsOnSide() <= 1 && (debugOffEdgeKick || 
				    getNumTeammatesOnSide() >= 1)) {
					cout << "opponent clear\n";
					l3 = 2;
				}
				else {
					cout << "od: " << opp.d << " oos: " << getNumOpponentsOnSide() << "\n";
				}
				cout << endl;
				//leds(l1,l2,l3,l4,l5,l6);
			}

			// get ball off left edge
			if (ballSource == VISION_BALL
					&& utilBallOnLEdge(/*2 * BALL_RADIUS*/)
					&& utilIsOnLEdge()
					&& gps->self().h <120
					&& gps->self().h> 60
					&& gps->self().posVar < get95CF(75)
					&& vBall.y >= FIELD_LENGTH * 0.4
					&& vBall.y <= FIELD_LENGTH - 40
					&& opp.d >= 60
					&& getNumOpponentsOnSide() <= 1
					&& (debugOffEdgeKick || getNumTeammatesOnSide() >= 1)) {
				if (roleCout)
					cout << "OFF EDGE KICK    : left" << endl;
				offEdgeKick(OFFEDGE_KICK_LEFT);
				return;
			}

			// get ball off right edge
			if (ballSource == VISION_BALL
					&& utilBallOnREdge(/*2 * BALL_RADIUS*/)
					&& utilIsOnREdge()
					&& gps->self().h <120
					&& gps->self().h> 60
					&& gps->self().posVar < get95CF(75)
					&& vBall.y >= FIELD_LENGTH * 0.4
					&& vBall.y <= FIELD_LENGTH - 40
					&& opp.d >= 60
					&& getNumOpponentsOnSide() <= 1
					&& (debugOffEdgeKick || getNumTeammatesOnSide() >= 1)) {
				if (roleCout)
					cout << "OFF EDGE KICK    : right" << endl;
				offEdgeKick(OFFEDGE_KICK_RIGHT);
				return;
			}
		}
        // End offEdge kick.

		// pawkick on left edge
		if (ballSource == VISION_BALL
				&& utilBallOnLEdge()
				&& utilIsOnLEdge()
				&& gps->self().h <120
				&& gps->self().h> 60
	//			&& gps->self().posVar < get95CF(75)
				&& abs(vBall.h) < 30) {
			firePawKick(FIRE_PAWKICK_LEFT);
			if (pawKickEyes)
				edgePawKickEyes;
			if (roleCout)
				cout << "PAW KICK    : left edge pawkick" << endl;
			return;
		} 

		// pawkick on right edge
		if (ballSource == VISION_BALL
				&& utilBallOnREdge()
				&& utilIsOnREdge()
				&& gps->self().h <120
				&& gps->self().h> 60
	//			&& gps->self().posVar < get95CF(75)
				&& abs(vBall.h) < 30) {
			firePawKick(FIRE_PAWKICK_RIGHT);
			if (pawKickEyes)
				edgePawKickEyes;
			if (roleCout)
				cout << "PAW KICK    : right edge pawkick" << endl;
			return;
		} 

		// pawkick top edge right half
		if (ballSource == VISION_BALL
				&& utilBallOnTEdge()
				&& utilIsOnTEdge()
				&& gps->self().h > 150
				&& gps->self().h < 180
			//	&& gps->self().posVar <get95CF(75)
				&& abs(vBall.h) < 30
				&& gps->self().pos.x > RIGHT_GOALBOX_EDGE_X
				&& ballX > RIGHT_GOALBOX_EDGE_X
			//	&& gps->getOppCovMax(oppNum) < get95CF(50)
			//	&& opp.d < 60
                ) {
			if (roleCout)
				cout << "PAW KICK    : top edge right pawkick" << endl;
			firePawKick(FIRE_PAWKICK_AUTO);
			return;
		}

		// pawkick top edge left half
		if (ballSource == VISION_BALL
				&& utilBallOnTEdge()
				&& utilIsOnTEdge()
				&& (gps->self().h > 0 && gps->self().h < 30)
			//	&& gps->self().posVar < get95CF(75)
				&& abs(vBall.h) < 30
				&& gps->self().pos.x < LEFT_GOALBOX_EDGE_X
				&& ballX < LEFT_GOALBOX_EDGE_X
			//	&& gps->getOppCovMax(oppNum) < get95CF(50)
			//	&& opp.d < 60
                ) {
			if (pawKickEyes)
				edgePawKickEyes;
			if (roleCout)
				cout << "PAW KICK    : top edge left pawkick" << endl;
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
				&& gps->self().pos.x < FIELD_WIDTH / 2.0
				&& ballX < LEFT_GOALBOX_EDGE_X) {
			if (pawKickEyes)
				edgePawKickEyes;
			if (roleCout)
				cout << "PAW KICK    : bottom left edge pawkick" << endl;
			firePawKick(FIRE_PAWKICK_AUTO);
			return;
		}

		// pawkick bottom right half
		if (ballSource == VISION_BALL
				&& utilBallOnBEdge()
				&& utilIsOnBEdge()
				&& (gps->self().h > 0 && gps->self().h < 30)
				&& gps->self().posVar <get95CF(75)
				&& abs(vBall.h) < 30
				&& gps->self().pos.x > FIELD_WIDTH / 2.0
				&& ballX> RIGHT_GOALBOX_EDGE_X) {
			if (pawKickEyes)
				edgePawKickEyes;
			if (roleCout)
				cout << "PAW KICK    : bottom right edge pawkick" << endl;
			firePawKick(FIRE_PAWKICK_AUTO);
			return;
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// GET BEHIND BALL CASES
		//////////////////////////////////////////////////////////////////////////////////////////

		//if the ball is on one of the side edges and your not facing the right way
		if (ballSource == VISION_BALL
				&& vBall.d < 50
				&& utilBallOnLREdge()
				&& (gps->self().h >= 195 && gps->self().h <= 345)) {
			saFarGetBehindBall(prevAttackMode != GetBehindBall);
			//this is a hack required because the dog gets behind the ball already 
			//and still trys to get to the ball resulting in it walkin backwards   
			if (forward >= 0) {
				return;
			}
            if (roleCout)
                cout << "I am doing getbehind ball case on one of the side edges!" << endl;
		} 

		//using -0.1 cause i think gps self pos is clipped  and -0.1 ensures that the vector is backwards
		//also normalising to 90-450 to avoid wrap around errors around the 0-360 mark
		double towardsOwnGoal = NormalizeAngle_0_360(RAD2DEG(atan2(-0.1 - gps->self().pos.y, (FIELD_WIDTH / 2.0) - gps->self().pos.x)));
		double ballDistToOwnGoal = getDistanceBetween(ballX,
				ballY,
				FIELD_WIDTH / 2.0,
				0);

		//if ball is in front of own goal
		if (ballSource == VISION_BALL
				&& vBall.d <40
				&& ballDistToOwnGoal <= 80
				&& NormalizeAngle_90_450(gps->self().h)> towardsOwnGoal - 60
				&& NormalizeAngle_90_450(gps->self().h) < towardsOwnGoal + 60) {
			double awayFromOwnGoal = RAD2DEG(atan2(vBall.y - 0,
						vBall.x - (FIELD_WIDTH / 2.0)));
			saFarGetBehindBall(prevAttackMode != GetBehindBall, awayFromOwnGoal);
			if (getBehindBallEyes)
				nearGoalGbbEyes;

			//this is a hack required because the dog gets behind the ball already 
			//and still trys to get to the ball resulting in it walkin backwards
			if (forward >= 0) {
				return;
			}
            if (roleCout)
                cout << "Get behind ball case in front of own goal!" << endl;
		}

		//if ball is on bottom edge, get behind ball
		if ((ballSource == VISION_BALL
					&& vBall.d < 50
					&& gps->getBall(GLOBAL).y < BACKOFF_BALL_BOTTOM_EDGE_DISTANCE)
				&& (gps->self().h > 200 && gps->self().h < 340)
				&& (gps->getOppCovMax(oppNum) < get95CF(75) && opp.d < 100)) {
			double awayFromOwnGoal = RAD2DEG(atan2(vBall.y - 0,
						vBall.x - (FIELD_WIDTH / 2.0)));
			saFarGetBehindBall(prevAttackMode != GetBehindBall, awayFromOwnGoal);
			if (getBehindBallEyes)
				botEdgeGbbEyes; 
			//this is a hack required because the dog gets behind the ball already 
			//and still trys to get to the ball resulting in it walkin backwards
			if (forward >= 0) {
				return;
			}
            if (roleCout)
                cout << "Get behind ball case on bottom edge!" << endl;
		} 

		// if ball on top right edge and you're facing wrong way, get behind ball
		if (ballSource == VISION_BALL
				&& utilBallOnTEdge()
				&& vBall.x > FIELD_WIDTH / 2.0
				&& vBall.d > 7
				&& vBall.d <50
				&& ((gps->self().h> 0 && gps->self().h <80) || gps->self().h> 330)) {
			saFarGetBehindBall(prevAttackMode != GetBehindBall, 100);
			if (getBehindBallEyes)
				topRightEdgeGbbEyes;
			if (forward >= 0) {
				return;
			}
            if (roleCout)
                cout << "Get behind ball case on top right edge and you're facing wrong way!" << endl;
		}

		// if ball on top left edge and you're facing wrong way, get behind ball
		if (ballSource == VISION_BALL
				&& utilBallOnTEdge()
				&& vBall.x <FIELD_WIDTH / 2.0
				&& vBall.d> 7
				&& vBall.d <50
				&& gps->self().h> 100
				&& gps->self().h < 210) {
			saFarGetBehindBall(prevAttackMode != GetBehindBall, 80);
			if (getBehindBallEyes)
				topLeftEdgeGbbEyes;
			if (forward >= 0) {
				return;
			}
            if (roleCout)
                cout << "Get behind ball case on top left edge and you are facing wrong way!" << endl;
		}

		// if ball on top left corner and you're facing the wrong way, get behind ball
		if (ballSource == VISION_BALL
				&& utilBallInTLCorner()
				&& vBall.d > 7
				&& vBall.d <50
				&& gps->self().h> 110
				&& gps->self().h < 210) {
			saFarGetBehindBall(prevAttackMode != GetBehindBall, 80);
			if (getBehindBallEyes)
				topLeftCornerGbbEyes;
			if (forward >= 0) {
				return;
			}
            if (roleCout)
                cout << "Get behind ball case on top left corner and you're facing the wrong way." << endl;
		}    

		// if ball on top right corner and you're facing the wrong way, get behind ball
		if (ballSource == VISION_BALL
				&& utilBallInTRCorner()
				&& vBall.d > 7
				&& vBall.d <50
				&& ((gps->self().h> 0 && gps->self().h <70) || gps->self().h> 330)) {
			saFarGetBehindBall(prevAttackMode != GetBehindBall, 100);
			if (getBehindBallEyes)
				topRightCornerGbbEyes;
			if (forward >= 0) {
				return;
			}
            if (roleCout)
                cout << "Get behind ball case on top right corner and you're facing the wrong way." << endl;
		}

		// Before other paw kick cases see if we can use stealth to get around opponents
		double left, right;
		if (useStealth && needStealthDog(left, right)) {
			doStealthDog(left, right);
			return;
		}
		if (stealthEyes) {
			noStealthEyes;
		}
		lastStealth = 0;

		//////////////////////////////////////////////////////////////////////////////////////////
		// OTHER PAW KICK CASES
		//////////////////////////////////////////////////////////////////////////////////////////


		double reqAcc = requiredAccuracy(ballX, ballY, 20);
		//abs fix
		double ballToGoalHead = RAD2DEG(atan2((FIELD_LENGTH - ballY),
					(FIELD_WIDTH / 2 - ballX)));

		//if you get up to here you should be seeing the ball
		if (((utilBallInTargetGoalBox(ballX, ballY)
						&& gps->self().h
						<= (ballToGoalHead + (reqAcc - 2 * sqrt(gps->self().hVar)))
						&& gps->self().h
						>= (ballToGoalHead - (reqAcc - 2 * sqrt(gps->self().hVar)))
						//you'll get it in if you hit it
		     ))
				&& (ballD > 40 || ballH < 45)
				&&  //your lined up to paw kick the ball?
				((gps->getOppCovMax(oppNum) < get95CF(50)
				  && opp.d < 60
				  && !oppInTargetGoal(oppNum))
				 || (gps->getOppCovMax(oppNum) < get95CF(50)
					 && opp.d < 30
					 && oppInTargetGoal(oppNum)))) {
			if (pawKickEyes)
				goalBoxPawKickEyes;
			if (roleCout)
				cout << "PAW KICK    : goalbox pawkick" << endl;
			//leds(2, 2, 2, 1, 2, 1);
			firePawKick(FIRE_PAWKICK_AUTO);
			return;
		}


		double minDistBetweenBallOpp = minDistOppFromBall(ballX, ballY);

		//cout << "\tminDist: " << minDistBetweenBallOpp << endl;
		//cout << "\tcovMax:  " << gps->getOppCovMax(oppNum) << endl;
		//cout << "\tget95CF: " << get95CF(50) << endl; 
		//cout << "\topp.d:   " << opp.d << endl;

		if ((ballD > 40 || ABS(ballH) < 45)
				&&  //your lined up to paw kick the ball?
				(minDistBetweenBallOpp
				 < 35
				 || (gps->getOppCovMax(oppNum) < get95CF(50)
					 && opp.d < 60
					 && !oppInTargetGoal(oppNum))
				 || (gps->getOppCovMax(oppNum) < get95CF(50)
					 && opp.d < 30
					 && oppInTargetGoal(oppNum)))
				&&  
				//ball or you not in the clear
				(ballY <3 * FIELD_LENGTH / 5.0
				 && gps->self().h> 15
				 && gps->self().h < 165
				 ||  //your facing kinda up field
				 ballY >= 3 * FIELD_LENGTH / 5.0
				 && abs(ballToGoalHead - gps->self().h) < 60)
				&& !utilBallInTargetGoalBox(ballX,
					ballY)
				&& !utilIsOnTEdge()) {
			if (pawKickEyes)
				oppNearPawKickEyes;
			if (roleCout)
				cout << "PAW KICK    : opp near pawkick" << endl;
			//cout << "PAW KICK: opp near pawkick" << endl;
			//leds(2,2,2,2,2,2);
			firePawKick(FIRE_PAWKICK_AUTO);
			return;
		}        

		//this used to be the charge condition
		if (gps->self().h
				> 10
				&& gps->self().h
				<170
				&& vBall.cf
				> 0
				&& allowedToCharge
				&& !utilBallInTargetGoalBox(ballX,
					ballY)) {
			if (pawKickEyes)
				chargePawKickEyes;
			if (roleCout)
				cout << "PAW KICK    : charge pawkick" << endl;
			//cout << "PAW KICK: charge pawkick" << endl;
			//leds(2,2,2,2,2,2);
			firePawKick(FIRE_PAWKICK_AUTO);
			return;
		}

		// A quick hack for dkd
		double pawKickDKD = getUNSW2004DKD();
		if ( abs (gps->self().h - pawKickDKD ) < 15 ) {
		  //KEV_DEBUG ( cout << " --------- Quick hack to fire paw kick: ---------- \n";);
		  //KEV_DEBUG ( cout << " self heading " << gps->self().h << "\tdkd " << dkd << "\tpawKickDKD " << 
			      //pawKickDKD << endl;);
		  firePawKick(FIRE_PAWKICK_AUTO);
		  return;
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// DEFAULT CASE
		//////////////////////////////////////////////////////////////////////////////////////////

		// bernhard's visual ball smoothing for ball-grabbing
		// 6 / 4 / 04 not used at all
		/*double weight1 = 1.0;    // used to be 0.5
		double weight2v3 = 0.75; // used to be 0.6
		double weightedVisBallDist = fstVisBallDist *weight1
			+ sndVisBallDist *weight2v3*(1.0 - weight1)
			+ thdVisBallDist*(1.0 - weight2v3) * (1.0
					- weight1);
		double weightedVisBallHead = fstVisBallHead *weight1
			+ sndVisBallHead *weight2v3*(1.0 - weight1)
			+ thdVisBallHead*(1.0 - weight1) * (1.0 - weight1);
			hoverToBall(weightedVisBallDist, weightedVisBallHead);*/

        if (roleCout)
            cout << "Hover To Ball!" << endl;
		hoverToBall(vBall.d, vBall.h);
	}

	//#ifdef calTurnKick
	// int altKickCount = 0;
	//#endif
	
    static bool shootDebug = false;
    
	void UNSW2004::chooseUNSW2004ShootStrategy() {
		INSTRUMENT("UNSW2004::chooseUNSW2004ShootStrategy" << endl);

        // Ted: I removed the code comments. I think the codes removed are for debugging. 
        //      Get it back from revision 2499 if you ever need it.

		// Kevin: Replace the pvtk (90 / -90) with UPenn Right/Lrft for the time
		// being, since less tuning required 
		// 12/03/04

		if (kickingOff && isUNSW2004KickOffAttacker()) {
		        lockMode = LightningKick;
		        aaLightningKick();
		  	kickingOff = false;
			setUNSW2004HaveKickedOff();
			return;
		}

		double reqAccuracy = requiredAccuracy(25);
		double minGoalHead = gps->tGoal().pos.head - reqAccuracy;
		double maxGoalHead = gps->tGoal().pos.head + reqAccuracy;
		double min = MIN(abs(minGoalHead), abs(maxGoalHead));
		double max = MAX(abs(minGoalHead), abs(maxGoalHead));
		grabTime = 72;
		int oppNum = 0;
		const Vector &opp = gps->getClosestOppPos(LOCAL, &oppNum);
		int minVarOppNum = getMinVarOppNum();

		// facing right edge
		if (((gps->self().h <30 || gps->self().h> 330)
		    && gps->self().pos.x > (FIELD_WIDTH - WALL_THICKNESS - 20)) || 
		    (vision->facingFieldEdge && gps->self().pos.x > FIELD_WIDTH / 2.0 + 20)) {

			if (enableLocateBallKick) {
				lockMode = LocateBallKick;
				setLocateBallKickParams();
				aaLocateBallKick();
			}
			else {
			  lockMode = ProperVariableTurnKick;
			  setProperVariableTurnKick(90);
			  aaProperVariableTurnKick();
			}
            
            if (shootDebug)
                cout << "Proper variable facing right edge!" << endl;
			return;
		}

		// facing left edge
		if ((gps->self().h > 150 && gps->self().h < 210
		    && gps->self().pos.x < WALL_THICKNESS + 20) ||
		    (vision->facingFieldEdge && gps->self().pos.x < FIELD_WIDTH / 2.0 - 20)) {
			if (enableLocateBallKick) {
				lockMode = LocateBallKick;
				setLocateBallKickParams();
				aaLocateBallKick();
			}
			else {
			  lockMode = ProperVariableTurnKick;
			  setProperVariableTurnKick(-90);
			  aaProperVariableTurnKick();
			}
            if (shootDebug)
                cout << "Proper variable facing left edge!" << endl;
			return;
		}

		// in the attacking half
		if (gps->self().pos.y > FIELD_LENGTH / 2) {
			// top wall not in goalie box
			if (gps->self().pos.y >= (FIELD_LENGTH - WALL_THICKNESS - 50)
			    && (gps->self().pos.x <LEFT_GOALBOX_EDGE_X + 10
			    || gps->self().pos.x> RIGHT_GOALBOX_EDGE_X - 10)) {

			  /* // u dont want ball to fumble from left and right to left
			     // in front of the goal
			    if (gps->self().pos.x <LEFT_GOALBOX_EDGE_X + 10) {
			    lockMode = UpennLeft;
			    aaUPkickLeft();
			  }
			  else {
			    lockMode = UpennRight;
			    aaUPkickRight();
			  }
			  */
			lockMode = SpinDribble;
			aaSpinDribble();
			return;
		}

		// inside goalie box
		if (gps->self().pos.y >= (FIELD_LENGTH - WALL_THICKNESS - 50)
		    && gps->self().pos.x <= RIGHT_GOALBOX_EDGE_X
		    && gps->self().pos.x >= LEFT_GOALBOX_EDGE_X) {
			
			if (max < 90) {
			  lockMode = SpinDribble;
			  aaSpinDribble();
			  //lockMode = ChestPush;
			  //aaChestPush();
              if (shootDebug)
                cout << "Spin dribble inside goalie box!" << endl;
			  return;
			}

			// turn kick go for goal
			double dir;
			if (min <= 90 && max >= 90) {
				dir = (gps->tGoal().pos.head > 0 ? 90 : -90);
			}
			else if (min <= 180 && max >= 180) {
				dir = (gps->tGoal().pos.head > 0 ? 180 : -180);
			}
			else {
				if (abs(gps->tGoal().pos.head) < 135) {
					dir = (gps->tGoal().pos.head > 0) ? 90 : -90;
				}
				else {
					dir = (gps->tGoal().pos.head > 0) ? 180 : -180;
				}
			}
			lockMode = ProperVariableTurnKick;  // verify 180 pvtk later 
			setProperVariableTurnKick(dir);
			aaProperVariableTurnKick();
            if (shootDebug)
                cout << "Proper variable inside goalie box!" << endl;			
			return;
		}

		// if already lined up with goal, vision goal kick
		if (maxGoalHead >= 0 && minGoalHead <= 0) {
			// check if looking straight at the side wall..
			if (vision->facingFieldEdge) {
				if (gps->self().pos.y > FIELD_LENGTH - WALL_THICKNESS - 50) {
					/*
				   	lockMode = SpinChestPush;
				   	aaSpinChestPush();
				   	*/
					lockMode = SpinDribble;
					aaSpinDribble();
                    if (shootDebug)
                        cout << "Dribble lined up with goal!" << endl;
				        //lockMode = ChestPush;
				        //aaChestPush();
				}
				else {
					double dir;
					if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
					    dir = -90;
					}
					else {
					    dir = 90;
					}
					lockMode = ProperVariableTurnKick;
					setProperVariableTurnKick(dir);
					aaProperVariableTurnKick();
                    if (shootDebug)
                        cout << "Proper stratight at the side wall!" << endl;
				}
			}
			else {
				// if opponent nearby, perform spin-chestpush
				if ((gps->getOppCovMax(oppNum) < get95CF(50)
				     && opp.d <= 60 && !oppInTargetGoal(oppNum))
			             || (gps->getOppCovMax(oppNum) < get95CF(50)
				     && opp.d <= 30 && oppInTargetGoal(oppNum))) {
				     
					if (gps->self().pos.y < FIELD_LENGTH - WALL_THICKNESS - 50) {
					  lockMode = ChestPush;
					  aaChestPush();
					  // Kevin: come back to this later
					  //lockMode = LightningKick;
					  //aaLightningKick();
                      if (shootDebug)
                        cout << "UPenn kick if opponent nearby!" << endl;
					}
					else {
						/*
						   lockMode = SpinChestPush;
						   aaSpinChestPush();
						   */
						lockMode = SpinDribble;
						aaSpinDribble();
					        //lockMode = ChestPush;
					        //aaChestPush();
                        if (shootDebug)
                            cout << "Dribble if opponent nearby!" << endl;
					}
				}
				else {
				  lockMode = SpinDribble;
				  aaSpinDribble();
				  //lockMode = VisOppAvoidanceKick;
				  //aaVisOppAvoidanceKick();
                  if (shootDebug)
                    cout << "Dribble 2 if opponent nearby!" << endl;
				}
			}
			return;
		}

		//////////////////////////////////////////////////////////////
		// past this point both goal posts must be on the same side
		//////////////////////////////////////////////////////////////

		if (vision->facingFieldEdge) {
			if (gps->getOppCovMax(oppNum) < get95CF(50) && opp.d <= 60) {
				double dir;
				if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
				        dir = -90;
				}
				else {
				        dir = 90;
				}
				lockMode = ProperVariableTurnKick;
				setProperVariableTurnKick(dir);
				aaProperVariableTurnKick();
                if (shootDebug)
                    cout << "Proper if facingFieldEdge!" << endl;
			}
			else {
				/*
				   lockMode = SpinChestPush;
				   aaSpinChestPush();
				   */
				lockMode = SpinDribble;
				aaSpinDribble();
                if (shootDebug)
                    cout << "Dribble if facingFieldEdge!" << endl;
				//lockMode = ChestPush;
				//aaChestPush();
			  
			  //walkType = ChestPushWT;
			}
			return;
		}

		int visOppAvoidanceKickAngleTrigger;
		if (!doPenaltyShooter) {
			visOppAvoidanceKickAngleTrigger = 80;
		}
		else {
			visOppAvoidanceKickAngleTrigger = 100;
		}

		// out of turn kick range & not on edges, do spin front kick
		if (max <= visOppAvoidanceKickAngleTrigger) {
			if (!(utilIsOnLREdge() || utilIsOnTEdge() || utilIsOnBEdge())
			    && ((gps->getOppCovMax(oppNum) < get95CF(50)
			    && opp.d > 60 && !oppInTargetGoal(oppNum))
			    || (gps->getOppCovMax(oppNum) < get95CF(50)
			    && opp.d > 30 && oppInTargetGoal(oppNum))
			    || gps->getOppCovMax(minVarOppNum) > get95CF(50))) {

				// not on any of the edges and no opponents near
			  lockMode = SpinDribble;
			  aaSpinDribble();
              if (shootDebug)
                cout << "Dribble if out of turn kick case!" << endl;
			  //lockMode = VisOppAvoidanceKick;
			  //aaVisOppAvoidanceKick();
			}
			else {
				// reach here if opponent near or on an edge
				// in which case, spin-chest push or lightning
				// kick are both fine
	
				if (gps->self().pos.y > FIELD_LENGTH - WALL_THICKNESS - 50
				    || (gps->self().h <70 && gps->self().h> 110)) {
					/*
					// you're in top horizontal strip or facing the wrong way     
					lockMode = SpinChestPush;
					aaSpinChestPush();
					*/
					lockMode = SpinDribble;
					aaSpinDribble();
                    if (shootDebug)
                        cout << "Dribble if opponent near or on an edge case!" << endl;
				  //walkType = ChestPushWT;
				  //lockMode = ChestPush;
				  //aaChestPush();
				}
				else {
					// facing the right way and not in top horizontal strip
				        // Kevin
					//lockMode = LightningKick;
					//aaLightningKick();
				  //walkType = ChestPushWT;
				  lockMode = ChestPush;
				  aaChestPush();
                  if (shootDebug)
                    cout << "Chest push facing right way and not in top horizontal strip!" << endl;
				}
			}
			return;
		}

		// a check to ensure that you do not turn kick for the goal when
		// no opponents are near; spin-chest push instead
		if (gps->self().pos.y >= (FIELD_LENGTH - WALL_THICKNESS - 50)
		    && ((gps->getOppCovMax(oppNum) < get95CF(50) && opp.d <= 60)
		    || gps->getOppCovMax(minVarOppNum) > get95CF(50))) {
			/*
			   lockMode = SpinChestPush;
			   aaSpinChestPush();
		   	*/
			lockMode = SpinDribble;
			aaSpinDribble();
		  //walkType = ChestPushWT;
		  //lockMode = ChestPush;
		  //aaChestPush();
            if (shootDebug)
                cout << "Dribble if do not turn kick for the goal when no opponents are near!" << endl;
			return;
		}    

		// turn kick go for goal
		double dir;
		if (min <= 90 && max >= 90) {
			dir = (gps->tGoal().pos.head > 0 ? 90 : -90);
		}
		else if (min <= 180 && max >= 180) {
			dir = (gps->tGoal().pos.head > 0 ? 180 : -180);
	
			if (dir >= 0 && gps->getOppCovMax(oppNum) < get95CF(50)
			    && opp.d <= 40 && opp.head >= 0) {
				dir = dir - 360;
			}
			else if (dir <= 0 && gps->getOppCovMax(oppNum) < get95CF(50)
				 && opp.d <= 40 && opp.head <= 0) {
				dir = 360 + dir;
			}
		}
		else {
			if (abs(gps->tGoal().pos.head) < 135) {
				dir = (gps->tGoal().pos.head > 0) ? 90 : -90;
			}
			else {
				dir = (gps->tGoal().pos.head > 0) ? 180 : -180;
			}
		}
		lockMode = ProperVariableTurnKick;  // verify 180 pvtk later 
		setProperVariableTurnKick(dir);
		aaProperVariableTurnKick();
        if (shootDebug)
            cout << "Proper if turn kick go for goal!" << endl;
    	return;
	}    
	//in defend half
	else {
		reqAccuracy = requiredAccuracy(3.0 * FIELD_WIDTH / 2.0);

		//local coords
		minGoalHead = gps->tGoal().pos.head - reqAccuracy;
		maxGoalHead = gps->tGoal().pos.head + reqAccuracy;
		min = MIN(abs(minGoalHead), abs(maxGoalHead));
		max = MAX(abs(minGoalHead), abs(maxGoalHead));

		if (maxGoalHead >= 0 && minGoalHead <= 0) {
			//check if your looking straight at the side wall ...
			//if so your gps is probably wrong
			if (vision->facingFieldEdge) {
				double dir;
				if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
				        dir = -90;
				}
				else {
					dir = 90;
				}
				// Kevin commented for new dog
				lockMode = ProperVariableTurnKick;
				setProperVariableTurnKick(dir);
				aaProperVariableTurnKick();
                if (shootDebug)
                    cout << "Proper if defend half!" << endl;
			}
			else {
			  // Kevin 
			  //lockMode = LightningKick;
			  //aaLightningKick();
			  //walkType = ChestPushWT;
			  lockMode = ChestPush;
			  aaChestPush();
              if (shootDebug)
                cout << "Chest Push if defend half!" << endl;
			}
			return;
		}

		//////////////////////////////////////////////////////////////////////
		// past this point both goal posts must be on the same side
		//////////////////////////////////////////////////////////////////////

		reqAccuracy = requiredAccuracy(FIELD_WIDTH);

		minGoalHead = gps->tGoal().pos.head - reqAccuracy;
		maxGoalHead = gps->tGoal().pos.head + reqAccuracy;

		min = MIN(abs(minGoalHead), abs(maxGoalHead));
		max = MAX(abs(minGoalHead), abs(maxGoalHead));        

		double dir;
		if (min <= 90 && max >= 90) {
			dir = (gps->tGoal().pos.head > 0 ? 90 : -90);
		}
		else if (min <= 180 && max >= 180) {
			//go in the direction thats away from own goal
			dir = (gps->self().pos.x > FIELD_WIDTH / 2.0 ? 180 : -180);

			// only avoid opponents when you're not in defensive quarter
			if (gps->self().pos.y > FIELD_LENGTH / 4.0) {
				//override, to try and avoid opponents
				if (dir >= 0 && gps->getOppCovMax(oppNum) < get95CF(50)
				    && opp.d <= 40 && opp.head >= 0) {
					dir = dir - 360;
				}
				else if (dir <= 0 && gps->getOppCovMax(oppNum) < get95CF(50)
					 && opp.d <= 40 && opp.head <= 0) {
					dir = 360 + dir;
				}
			}    
			//might want to put in, if your blocked on both sides
			//try do a 90 (if its safe)
		}
		else {
			if (abs(gps->tGoal().pos.head) < 125) {
				dir = (gps->tGoal().pos.head > 0) ? 90 : -90;
			}
			else {
				//go in the direction thats away from own goal
				dir = (gps->self().pos.x > FIELD_WIDTH / 2.0 ? 180 : -180);

				// only avoid opponents when you're not in defensive quarter
				if (gps->self().pos.y > FIELD_LENGTH / 4.0) {
					if (dir >= 0 && gps->getOppCovMax(oppNum) < get95CF(50)
					    && opp.d <= 40 && opp.head >= 0) {
						dir = dir - 360;
					}
					else if (dir <= 0 && gps->getOppCovMax(oppNum) < get95CF(50)
						 && opp.d <= 40 && opp.head <= 0) {
						dir = 360 + dir;
					}
				}
			}
		}
		lockMode = ProperVariableTurnKick;  // verify 180 pvtk later 
		setProperVariableTurnKick(dir);
		aaProperVariableTurnKick();
        if (shootDebug)
            cout << "Proper last case!" << endl;
		return;
	}
}

/////////////////////////////////////////////////////////////////////////
// action functions
/////////////////////////////////////////////////////////////////////////

//static double old_tilt = 0;
//static double old_pan = 0;

void UNSW2004::doUNSW2004TrackVisualBall() {
        INSTRUMENT("UNSW2004::doUNSW2004TrackVisualBall" << endl);
       	if (gps->canSee(vobBall)) {
	  	double ballx = 0, bally = 0;
		// track visual top of ball        
		int inpoints[2] = {
			((int) (vision->vob[vobBall].cx) - WIDTH / 2),
			-1 * ((int) (vision->vob[vobBall].misc) - HEIGHT / 2)
		};
		double outpoints[2] = {
			-1, -1
		};
		vision->projectPoints(inpoints, 1, outpoints, 0);
		ballx = -outpoints[0];
		bally = outpoints[1];
		headtype = ABS_XYZ;

		panx = ballx;
		tilty = -1.6;
		cranez = bally - 1.0;

		// Kevin: the reason for minus 1.5 because since the robot has a higher view of the top
		//        of the ball, the cplane top of the ball is not the physical top point of the ball,
		//        it actually looks over the top, so if the ball is close, -1.5 from the obtained is
		//        useful and it wont lose the ball easily. For long range, -1.5 makes no difference.
	}

	else {
#ifndef OFFLINE
		headtype = ABS_H;
		tilty = *desiredTilt;
		panx = *desiredPan;
		cranez = *desiredCrane;
#endif // OFFLINE
	}
}        

void UNSW2004::doUNSW2004TrackGpsBall() {
	INSTRUMENT("UNSW2004::doUNSW2004TrackGpsBall" << endl);
	headtype = ABS_XYZ;

	panx = -gps->getBall(LOCAL).x;
	tilty = BALL_DIAMETER;
	cranez = gps->getBall(LOCAL).y;
}        


// Return the heading to ball based on special cases.
double UNSW2004::handleSpecialCases(bool onLeft, bool onRight, bool onTop, bool onBottom, double ballHead) {
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

    return relH;
} 

void UNSW2004::walkStraightNormalMax(double turnccw) {
    setWalkParams();
    walkType         = NormalWalkWT;	
	forward 	     = MAX_NORMAL_WALK;
	turnCCW 	     = turnccw;
	Behaviours::left = 0;
}

void UNSW2004::walkTurnOnlyNormal(double turn) {
    setWalkParams();
    forward          = TURNING_FORWARD;
    Behaviours::left = 0;
    turnCCW          = turn;
}

void UNSW2004::slowDownFacingTarget(double ballDist, double ballHead) {
    walkType = NormalWalkWT;
	forward = 4.0;
	Behaviours::left = 0;
    turnCCW = CLIP(ballHead / 3.0, 30.0);
	if (ABS(turnCCW) > 15 && ballDist < 15) {
	    PG = 30;
		forward = 0.5;
	}
}

void UNSW2004::slowDownFacingOwn(double ballDist, double ballHead) {	
    walkType = NormalWalkWT;
	forward = 4.0;
	Behaviours::left = 0;
	turnCCW = CLIP(ballHead / 3.0, 30.0);
	if (ABS(turnCCW) > 15 && ballDist < 15) {
	    PG = 30;
		forward = 0.5;
	}
}

static bool debugHoverToBall = false;

// Approach the ball. Both ball velocity and stealth dog have been removed.
void UNSW2004::hoverToBall(double ballDist, double ballHead) {

  INSTRUMENT("UNSW2004::hoverToBall" << endl);

    // What to do if high gain?
    if (gain == HIGH_GAIN) {}

    // Special Cases.
	bool onLeft   = utilBallOnLEdge() && utilIsOnLEdge() && gps->self().posVar < get95CF(75);
	bool onRight  = utilBallOnREdge() && utilIsOnREdge() && gps->self().posVar < get95CF(75);
	bool onTop    = utilBallOnTEdge() && utilIsOnTEdge() && gps->self().posVar < get95CF(75);
	bool onBottom = utilBallOnBEdge() && utilIsOnBEdge() && gps->self().posVar < get95CF(75);    
	double relH   = handleSpecialCases(onLeft, onRight, onTop, onBottom, ballHead);

    // Stealth dog is removed at the moment. In 2003 it is activated when the robot is not at left, right, top
    // and bottom. If the stealth conditions trigger satisfied, stealth dog should be activated here and return.
    
    // Ball velocity prediction is removed at the moment. In 2003, it is activated when usePrediction is true
    // and it can see the ball and its distance is below a threshold and the velocity vector is below a given
    // threshold and confidence is below 2.0. If conditions satisifed, ball velocity prediction should be
    // activated here.

    // This turn is used for decision comparing.
	turnCCW = CLIP(relH / 2.0, 40.0);
    walkType = NormalWalkWT;  // only

    if (debugHoverToBall) {
        cout << "turnCCW for comparing: " << turnCCW << endl;
        cout << "Ball distance: " << ballDist << endl;    
    }
    if (debugHoverToBall && false) {
        if (!(ABS(relH) > 18)) {
            return;
        }
    }

    // If the turning is not large and distance is large, then the forward at maximum speed.
    if (ABS(turnCCW) < 15 && ballDist > 35) { 
        walkStraightNormalMax(turnCCW);     // Two degrees - turn and forward.
        if (debugHoverToBall)
            cout << "HoverToBall() case 1" << endl;
    }
    else {
        // If the turning is not very large and distance is very large, then go forward at maximum
        // speed and clip the turnCCW to prevent slipping.
        if (ABS(turnCCW) < 22.0 && ballDist > 60) {
            walkStraightNormalMax(CLIP(turnCCW,15.0)); // Two degrees - turn and forward.
            if (debugHoverToBall)
                cout << "HoverToBall() case 2" << endl;        
        }
        else {
            // If the turn factor is large compared to the distance to the ball.
            // In 2003 this conidition used Table 5.1 in the report. But changing to use only
            // forward and turnCCW should enable the robot to alive longer.
	        if (ABS(relH) > 18) {    // One degree - turn.
                if (debugHoverToBall)
                    cout << "HoverToBall() case 3" << endl;
                walkTurnOnlyNormal(turnCCW);
            }
        	else {
                // Distance small and turn is small but not beckhamIsChin().
                if (debugHoverToBall)
                    cout << "HoverToBall() case 4" << endl;
    		    setWalkParams();    // A doggy case. Two degrees - turn and left.
		        walkType = NormalWalkWT;
		        Behaviours::left = 0;
			forward = 5.0;//5.5
	        }        
        }
    }
    
	// If the robot is facing toward the target goal, and distance to ball is small then slow down.
	if (gps->self().h > 5 && gps->self().h < 175) {
	  if (ballDist < 35)//15
            slowDownFacingTarget(ballDist, ballHead);		
	}
	else {
        // If the robot is facing toward own goal, and distance to ball is small then slow down. But
        // the slowing down distance is larger than the above case because of safety reason. See 2003
        // report page 223.
	  if (ballDist < 35)//20
            slowDownFacingOwn(ballDist, ballHead);
	}      

    if (forward > MAX_NORMAL_WALK) {
        cout << "Warning - forward greater than maximum value!" << endl;           
        forward = MAX_NORMAL_WALK;
    }

    if (turnCCW != 0 && Behaviours::left != 0)
        cout << "Warning - turn and left are both non-zero!" << endl;
   
    // Edge spin kicks.
	if (edgeSpinOverride && (vBall.cf > 3)) {
	  KEV_DEBUG(cout << "*******************edge spin kick override: "; );
		double myHead = gps->self().h;
		double ballx = sin(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
		double bally = cos(DEG2RAD(vBall.vob->h)) * vBall.vob->d;
		bool facingUpL = myHead > 90 && myHead < 180;
		bool facingUpR = myHead > 0 && myHead < 90;
		bool facingLeftB = myHead > 180 && myHead < 270;
		bool facingRightB = myHead <0 && myHead> 270;
		bool facingLeftT = myHead > 90 && myHead < 180;
		bool facingRightT = myHead > 0 && myHead < 90;
		bool inPositionL = ballx > 2 * BALL_RADIUS && ballx < 4 * BALL_RADIUS && bally < 2 * BALL_RADIUS;
		bool inPositionR = ballx <-2 * BALL_RADIUS && ballx> - 4 * BALL_RADIUS && bally < 2 * BALL_RADIUS;

		if (onLeft && facingUpL && inPositionL) {
			setLocateBallKickParams();
			aaLocateBallKick();
			KEV_DEBUG(cout << "case 1 \n"; );
		}
		else if (onTop && facingRightT && inPositionL) {
			setLocateBallKickParams();
			aaLocateBallKick();
			KEV_DEBUG(cout << "case 2 \n"; );
		}
		else if (onBottom && facingLeftB && inPositionL) {
			setLocateBallKickParams();
			aaLocateBallKick();
			KEV_DEBUG(cout << "case 3 \n"; );
		}
		else if (onRight && facingUpR && inPositionR) {
			setLocateBallKickParams();
			aaLocateBallKick();
			KEV_DEBUG(cout << "case 4 \n"; );
		}
		else if (onTop && facingLeftT && inPositionR) {
			setLocateBallKickParams();
			aaLocateBallKick();
			KEV_DEBUG(cout << "case 5 \n"; );
		}
		else if (onBottom && facingRightB && inPositionR) {
			setLocateBallKickParams();
			aaLocateBallKick();
			KEV_DEBUG(cout << "case 6 \n"; );
		}
	}
}   

void UNSW2004::hoverToSpot(double spotDist, double spotHead) {

    // What happen if high gain?
    if (gain == HIGH_GAIN) {}
    
    // This function should incorprate stealth dog in the future.

	double relH = spotHead;
	turnCCW = CLIP(relH / 2.0, 40.0);

    // If the distance is large and turn is not too large, then walk at the maximum speed.
	if (ABS(turnCCW) <= 13.0 && spotDist > 30) {
        walkStraightNormalMax();    
	}
	else {
        // If the heading is large.
        if (ABS(relH) > 18)
            walkTurnOnlyNormal(turnCCW); // In the future should enable the robot to turn and left at the same time.
        else {
            setWalkParams();            // A doggy case.
		    walkType = NormalWalkWT;
		    Behaviours::left = 0;
            forward = 7.5;        
        }    
    }

    // I suppose no need to slow down the robot when the robot is very close to the spot?

	if (walkType == NormalWalkWT) {
        // Guards should be put here in the future.
	}
    
    if (turnCCW != 0 && Behaviours::left != 0)
        cout << "Warning - turn and left are both non-zero!" << endl;    
}    

//bx by is global ball
//return distance between ball and opp closest to ball
double UNSW2004::minDistOppFromBall(double bx, double by) {
	INSTRUMENT("UNSW2004::minDistOppFromBall" << endl);
	double minSoFar = LARGE_VAL;
	for (int i = 0; i < NUM_OPPONENTS; i++) {
		if (gps->getOppCovMax(i) < get95CF(50)) {
			double d = getDistanceBetween(gps->getOppPos(i, GLOBAL).x,
					gps->getOppPos(i, GLOBAL).y,
					bx,
					by);
			if (d < minSoFar) {
				minSoFar = d;
			}
		}
	}
	return minSoFar;
}

bool UNSW2004::isUNSW2004KickOffAttacker() {
	INSTRUMENT("UNSW2004::isUNSW2004KickOffAttacker" << endl);
	// attacker if you're in the middle, i.e. there is one
	// forward to your left and one forward to your right

	int numToLeft = 0;        
	int numToRight = 0;

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;

		// ignore invalid teammate
		if (tm.counter <= 0)
			continue;                

		// ignore yourself
		if (i == PLAYER_NUMBER - 1)
			continue;                

		// ignore goalie
		if (info.amGoalie != 0)
			continue;

		// count num teammates to left
		if (gps->self().pos.x > tm.pos.x)
			numToLeft++;                

		// count num teammates to right
		if (gps->self().pos.x < tm.pos.x)
			numToRight++;
	}            

	if (numToRight == 1 && numToLeft == 1) {
		return true;
	}                        
	return false;
}

bool UNSW2004::isUNSW2004KickOffReceiver() {
	INSTRUMENT("UNSW2004::isUNSW2004KickOffReceiver" << endl);
	// if isUNSW2004KickOffAttacker(), return false
	// if you are closest to the left, find the teammate
	// closest to the right, and compare your y-values
	// if you are closest to the right, find the teammate
	// closest to the left, and compare your y-values

	int numToLeft = 0;
	int numToRight = 0;

	int leftMostPlayerIndex = PLAYER_NUMBER - 1;
	double leftMostPlayerValue = gps->self().pos.x;

	int rightMostPlayerIndex = PLAYER_NUMBER - 1;
	double rightMostPlayerValue = gps->self().pos.x;

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;

		// ignore invalid teammate
		if (tm.counter <= 0)
			continue;                

		// ignore yourself
		if (i == PLAYER_NUMBER - 1)
			continue;                

		// ignore goalie
		if (info.amGoalie != 0)
			continue;

		// count num of teammates to left
		if (gps->self().pos.x > tm.pos.x)
			numToLeft++;

		// count num of teammates to right
		if (gps->self().pos.x < tm.pos.x)
			numToRight++;

		// find rightmost player
		if (tm.pos.x > rightMostPlayerValue) {
			rightMostPlayerIndex = i;
			rightMostPlayerValue = tm.pos.x;
		}                

		// find leftmost player
		if (tm.pos.x < leftMostPlayerValue) {
			leftMostPlayerIndex = i;
			leftMostPlayerValue = tm.pos.x;
		}
	}            

	// middle forward
	if (numToRight == 1 && numToLeft == 1) {
		return false;
	}        

	// left forward, compare with right-most teammate
	if (numToRight == 2) {
		const WMObj &tm = gps->tmObj(rightMostPlayerIndex);            
		if (gps->self().pos.y > tm.pos.y)
			return true;
		else
			return false;
	}            

	// right forward, compare with left-most teammate
	if (numToLeft == 2) {
		const WMObj &tm = gps->tmObj(leftMostPlayerIndex);
		if (gps->self().pos.y > tm.pos.y)
			return true;
		else
			return false;
	}
	return true;
}        

int UNSW2004::chooseUNSW2004KickOffSide() {
	INSTRUMENT("UNSW2004::chooseUNSW2004KickOffSide" << endl);
	// find the teammate with the greatest y value
	double greatestYValueSoFar = 0;
	int greatestYValuePlayer = -1;

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);                

		if (tm.counter <= 0)
			continue;

		if (i == PLAYER_NUMBER - 1)
			continue;

		if (tm.pos.y > greatestYValueSoFar) {
			greatestYValueSoFar = tm.pos.y;
			greatestYValuePlayer = i;
		}
	}

	const WMObj &greatestYValueFriend = gps->teammate(greatestYValuePlayer + 1,
			'g');

	if (greatestYValueFriend.pos.x < FIELD_WIDTH / 2) {
		return 90;
	}
	else {
		return -90;
	}
}        

bool UNSW2004::isUNSW2004KickOffNoOpponentsInFront() {
	INSTRUMENT("UNSW2004::isUNSW2004KickOffNoOpponentsInFront" << endl);
	for (int i = 0; i < NUM_OPPONENTS; i++) {
		const Vector &opp = gps->getOppPos(i, LOCAL);
		if (gps->getOppCovMax(i) < get95CF(50) && ABS(opp.head) <= 10) {
			return false;
		}
	}
	return true;
}    

bool UNSW2004::isOnLeftOfLine(double fromx, double fromy, double tox, double toy) {
	INSTRUMENT("UNSW2004::isOnLeftOfLine" << endl);
	if (fromx == tox) {
		fromx += 0.01;
	} //avoid divide by 0
	double m = (toy - fromy) / (tox - fromx);
	double b = toy - m *tox;
	if (m == 0) {
		m += 0.01;
	} //avoid divide by 0
	double xMatchingYourY = (gps->self().pos.y - b) / m;    

	if (gps->self().pos.x < xMatchingYourY) {
		return true;
	}
	else {
		return false;
	}
}

//provides a position between your own goal and a point, at a certain distance from the point
//has crops so you dont go into the goal box or into the bottom edges etc
//returns the distance it put you at after the crop
//
//ie if the distance after the  crop is very small or negative you might not want to place yourself in the position it calculates
double UNSW2004::posnTowardsOwnGoalFromPointWithCrop(double pointx, double pointy, double distance, double *posx, double *posy) {
	INSTRUMENT("UNSW2004::posnTowardsOwnGoalFromPointWithCrop" << endl);

	double pointToGoalDist = getDistanceBetween(pointx,
			pointy,
			OWN_GOAL_X,
			OWN_GOAL_Y);
	double maxICanDoToStayOutOfGoalBox = pointToGoalDist - 60;

	double d = MIN(distance, maxICanDoToStayOutOfGoalBox);

	double pointToOwnGoalHead = NormalizeAngle_0_360(RAD2DEG(atan2((0 - ballY),
					(FIELD_WIDTH
					 / 2.0
					 - ballX))));
	*posx = pointx + d * cos(DEG2RAD(pointToOwnGoalHead));
	*posy = pointy + d * sin(DEG2RAD(pointToOwnGoalHead));

	return d;
}


void UNSW2004::UNSW2004GeneralHead() {
	INSTRUMENT("UNSW2004::UNSW2004GeneralHead" << endl);

	if (gps->canSee(vobBall)) {
		if (headEyes)
			seeBallEyes;
		doUNSW2004TrackVisualBall();
		if (closeBallCout) {
			cout << "vis - ";
		}
	}
#ifndef OFFLINE
    //  Turn off by Ted Wong.
	else if (lostBall <= 3 && false) {
		if (headEyes)
			looklastBallEyes;
		headtype = ABS_XYZ;
		panx = fstVisBallDist * sin(DEG2RAD(fstVisBallHead));
		tilty = BALL_RADIUS;
		cranez = fstVisBallDist * cos(DEG2RAD(fstVisBallHead));
		if (abs(hPan - *desiredPan) > 8 || abs(hTilt - *desiredTilt) > 8)
			lostBall = 0;
		if (closeBallCout) {
			cout << "ini - ";
		}
	} 
#endif // OFFLINE
	else if (lostBall <= vBallLost && abs(gps->getBall(LOCAL).head) < 90) {
		if (headEyes)
			lookGpsBallEyes;
		doUNSW2004TrackGpsBall();
		if (closeBallCout) {
			cout << "gps - ";
		}
	}
	else {
		if (headEyes)
			findEyes;
		headFindBall();
		if (closeBallCout) {
			cout << "find - ";
		}
	}
	if (closeBallCout) {
		switch (headtype) {
			case ABS_H: cout << "ABS_H ";
				    break;
			case ABS_XYZ: cout << "XYZ ";
				      break;
			case REL_H: cout << "REL_H ";
				    break;
			case ABS_PT: cout << "ABS_PT ";
				     break;
			default: cout << "UNKNOWN ";
				 break;
		}
		cout << "[" << panx << "," << tilty << "," << cranez << "]" << endl;
	}
}

//////////////////////////////////////
// Bird of Prey
/////////////////////////////////////

typedef enum {
	NORMAL_BOP,
	CIRCLE_BOP,
	AVOIDBOX_BOP,
	POSTAVOID_BOP
} BopMode;

static BopMode currentBopMode = NORMAL_BOP;


static const double defenceOffset = 10.0;
static const double defenceAngle = 150.0;
bool UNSW2004::UNSW2004DefenceRequired() {
	INSTRUMENT("UNSW2004::UNSW2004DefenceRequired" << endl);

#ifndef soloBird
	if (numForwardsCommunicating() < 2 && framesSince3ForwardsPresent >= 3) {
		return false;
	}
#endif // soloBird

	if (numBirds() >= 1) {
		return false;
	}

	//if (areClosest())
	if (!areFurthestX()) {
		return false;
	}

	if (ballInOwnBox(5)) {
		return false;
	}

	currentBopMode = NORMAL_BOP;

	return isDefenceProblem(defenceOffset, defenceAngle);
}

bool UNSW2004::areClosest() {
	INSTRUMENT("UNSW2004::areClosest" << endl);

	bool closest = true;
	for (int i = 0 ; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;

		// ignore goalie
		if (info.amGoalie != 0)
			continue;

		if (tm.counter <= 0)
			continue;

		if (ballD - info.ballDist > 0)
			closest = false;
	}

	return closest;
}

bool UNSW2004::areFurthestX() {
	INSTRUMENT("UNSW2004::areFurthestX" << endl);

	for (int i = 0 ; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;

		// ignore goalie
		if (info.amGoalie != 0)
			continue;

		if (tm.counter <= 0)
			continue;

		if (abs(gps->self().pos.x - ballX) - abs(tm.pos.x - ballX) < 0)
			return false;
	}

	return true;
}

int UNSW2004::numBirds() {
	INSTRUMENT("UNSW2004::numBirds" << endl);

	int ret = 0;
	for (int i = 0 ; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;

		// ignore goalie
		if (info.amGoalie != 0)
			continue;

		if (tm.counter <= 0)
			continue;

		if (info.defending != 0)
			ret++;
	}

	return ret;
}

bool UNSW2004::isDefenceProblem(double offset,
		double angle,
		bool mustHaveBall,
		bool includeYou) {
	INSTRUMENT("UNSW2004::isDefenceProblem" << endl);

	double ang = 90.0 - angle / 2.0;
	double m = tan(DEG2RAD(ang));
	double bpos = (ballY + offset) - m *ballX;
	double bneg = (ballY + offset) + m *ballX;

	if (lostBall > 25) {
		return false;
	}

	if (ballY > (2.0 / 3.0) * FIELD_LENGTH) {
		return false;
	}

	//if (ballY <     GOALBOX_DEPTH+15) {
	//    lockMode = ResetMode;
	//}

	// check yourself first
	const WMObj &self = gps->self();

	// if you are far enough back then no problem (hysterisis)
	if ((includeYou && self.pos.y < 40) || self.pos.y < 20) {
		return false;
	}

	if (includeYou
			&& (self.pos.y < m * self.pos.x + bpos)
			&& (self.pos.y < -m * self.pos.x + bneg)) {
		return false;
	}

	for (int i = 0 ; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;

		// ignore goalie
		if (info.amGoalie != 0)
			continue;

		if ((tm.counter > 0)
				&& (tm.pos.y < m * tm.pos.x + bpos)
				&& (tm.pos.y < -m * tm.pos.x + bneg)) {
			if (!mustHaveBall || info.haveBall != 0)
				return false;
		}
	}

	return true;
}

static const double iniDesiredBallAngle = 45;
static const double insideBound = 36.0 * 0.5; //10.0;
static const double turnRate = 3.0;
static const double outsideBound = insideBound + 20.0; //(1.0/turnRate)*iniDesiredBallAngle;
static const double cornerCut = 10.0;
static const double maxTurn = 20.0;
static const double boxPointOffset = 15.0;
static double lastDBH = 0.0;
static int bopCounter = 0;
static const int maxBopCounter = 48;
static double oldBD = 0.0;
static double oldBH = 0.0;
void UNSW2004::doUNSW2004Defend() {
	INSTRUMENT("UNSW2004::doUNSW2004Defend" << endl);

	UNSW2004GeneralHead();

	bopCounter++;
	bopCounter %= maxBopCounter;
	if (ballSource == VISION_BALL) {
		oldBD = ballD;
		oldBH = ballH;
	}
	// need extra localise if we are trying to avoid the box
	if ((bopCounter > 32)
			|| (((currentBopMode == AVOIDBOX_BOP) || (currentBopMode == POSTAVOID_BOP))
				&& bopCounter > 16
				&& bopCounter < 22)) {
		if (bopCounter <40 && ballD> 40) {
			//setNewBeacons(LARGE_VAL, 90);            
			//activeGpsLocalise(false);
			smartSetBeacon();
			smartActiveGpsLocalise();
		}
		else if (ballSource != VISION_BALL) {
			headtype = ABS_XYZ;
			panx = oldBD * sin(DEG2RAD(oldBH));
			tilty = BALL_RADIUS;
			cranez = oldBD * cos(DEG2RAD(oldBH));
		}
	}
	else if (((currentBopMode == AVOIDBOX_BOP)
				|| (currentBopMode == POSTAVOID_BOP))
			&& bopCounter
			> 16
			&& bopCounter
			< 30) {
		if (bopCounter <22 && ballD> 40) {
			//setNewBeacons(LARGE_VAL, 90);            
			//activeGpsLocalise(false);
			smartSetBeacon();
			smartActiveGpsLocalise();
		}
		else if (ballSource != VISION_BALL) {
			headtype = ABS_XYZ;
			panx = oldBD * sin(DEG2RAD(oldBH));
			tilty = BALL_RADIUS;
			cranez = oldBD * cos(DEG2RAD(oldBH));
		}
	}

	setUNSW2004Defending();

	if (isUNSW2004HaveBall()) {
		if (!isUNSW2004Stuck()) {
			setUNSW2004HaveBall();
		}
	}

	const WMObj &self = gps->self();

	double desiredBallAngle = iniDesiredBallAngle;

	if (ballInOwnBox(0)) {
		lockMode = ResetMode;
		if (birdCout)
			cout << "quit due to ball in box" << endl;
	}

	if (!isDefenceProblem(0, 90, true, false)) {
		currentBopMode = NORMAL_BOP;
		lockMode = ResetMode;
		if (birdCout)
			cout << "quit due to no defence problem" << endl;
	}

	if ((currentBopMode == CIRCLE_BOP) && abs(ballH) < 10.0) {
		currentBopMode = NORMAL_BOP;
		lockMode = ResetMode;
		if (birdCout)
			cout << "quit due to now in place" << endl;
		setWalkParams();
		walkType = ZoidalWalkWT;

		if (zoidCout)
			cout << "bop stuff 1" << endl;

		forward = MAX_OFFSET_FORWARD_SMALL_TURN;        
		left = 0;
		turnCCW = CLIP(ballH / 2.0, 8.0);
		return;
	}

	double desiredBH;

	// goal below is a hacked version of the goal
	double goalx = (FIELD_WIDTH / 2 + ballX) / 2.0;

	// determine which side of line between ball and center of the goal the robot is on
	// then set desired ball heading
	double dX = ballX - goalx;
	double dY = ballY;

	// determine if we are around the right side of the ball
	double usToGoal = sqrt(SQUARE(self.pos.x - goalx) + SQUARE(self.pos.y));
	double ballToGoal = sqrt(SQUARE(dX) + SQUARE(dY));
	if (ABS(dX) < 0.1) {
		if (self.pos.x < goalx) {
			desiredBH = desiredBallAngle;
		}
		else {
			desiredBH = -desiredBallAngle;
		}
	}
	else {
		double m = dY / dX;
		double b = ballY - m *ballX;
		if (abs(m) < 0.01)
			desiredBH = 0;
		else if (dY < 0) {
			// this is an error that appeared .. ball must be close to back line
			if (ballX > FIELD_WIDTH / 2)
				desiredBH = desiredBallAngle;
			else
				desiredBH = -desiredBallAngle;
		}
		else if (lastDBH > 0) {
			if (self.pos.x < ((self.pos.y - b) / m) + 5.0) {
				desiredBH = desiredBallAngle;
			}
			else {
				desiredBH = -desiredBallAngle;
			}
		}
		else {
			if (self.pos.x < ((self.pos.y - b) / m) - 5.0) {
				desiredBH = desiredBallAngle;
			}
			else {
				desiredBH = -desiredBallAngle;
			}
		}
	}

	if (usToGoal < ballToGoal) {
		double dp = (self.pos.x - goalx) * dX + self.pos.y *dY;
		if (abs(usToGoal) < 0.1 || abs(ballToGoal) < 0.1) {
			lockMode = ResetMode;
			if (birdCout)
				cout
					<< "quit due to something being to close to goal (should never happen)"
					<< endl;
			return;
		}
		double cosAng = dp / (usToGoal *ballToGoal);
		double ang = acos(cosAng);
		double dist = usToGoal *sin(ang);
		if (ballD < insideBound * 2.0
				|| dist < insideBound
				|| ((currentBopMode == CIRCLE_BOP) && dist < outsideBound)) {
			//lockMode = ResetMode;
			setWalkParams();
			walkType = ZoidalWalkWT;
			forward = MAX_OFFSET_FORWARD_LARGE_TURN;        
			left = 0;

			if (zoidCout)
				cout << "bop stuff 2" << endl;

			if (currentBopMode == CIRCLE_BOP)
				turnCCW = (lastDBH > 0) ? maxTurn : -maxTurn;
			else
				turnCCW = (desiredBH > 0) ? maxTurn : -maxTurn;
			currentBopMode = CIRCLE_BOP;
			if (defendEyes) {
				if (turnCCW > 0)
					defenceCircleLEyes;
				else
					defenceCircleREyes;
			}
			return;
		}
		else {
			if (currentBopMode == CIRCLE_BOP) {
				currentBopMode = NORMAL_BOP;
				lockMode = ResetMode;
				if (birdCout)
					cout << "quit due to no longer requiring circle(" << dist << ")"
						<< endl;
			}
		}
	}
	else {
		if (currentBopMode == CIRCLE_BOP) {
			currentBopMode = NORMAL_BOP;
			lockMode = ResetMode;
			if (birdCout)
				cout << "quit due to no longer requiring circle(2)" << endl;
		}
	}

	lastDBH = desiredBH;

	bool box = (self.pos.y < ((GOALBOX_DEPTH + WALL_THICKNESS) + 25))
		|| (((currentBopMode == AVOIDBOX_BOP)
					|| (currentBopMode == POSTAVOID_BOP))
				&& (self.pos.y < ((GOALBOX_DEPTH + WALL_THICKNESS) + 50)));
	double desiredHeading = NormalizeAngle_0_360(self.h + (ballH - desiredBH));
	if (box
			&& (desiredBH > 0)
			&& (self.pos.x < ((FIELD_WIDTH + GOALBOX_WIDTH) / 2.0) - cornerCut)
			&& (desiredHeading > 180.0)
			&& (desiredHeading < (360.0 - 10.0))) {
		currentBopMode = AVOIDBOX_BOP;

		double px = ((FIELD_WIDTH + GOALBOX_WIDTH) / 2.0) + boxPointOffset;
		double py = (GOALBOX_DEPTH + WALL_THICKNESS) + boxPointOffset;
		double phead = NormalizeAngle_180(RAD2DEG(atan2(py - self.pos.y,
						px - self.pos.x))
				- self.h);

		setWalkParams();
		walkType = ZoidalWalkWT;
		left = 0;
		turnCCW = CLIP(phead / 2.0, maxTurn);

		if (zoidCout)
			cout << "bop stuff 3" << endl;

		if (abs(turnCCW) <= 8) {
			forward = MAX_OFFSET_FORWARD_SMALL_TURN;
		}
		else {
			forward = MAX_OFFSET_FORWARD_LARGE_TURN;
		}

		if (defendEyes) {
			defenceBoxAvoidLEyes;
		}

		return;
	}
	else if (box
			&& (desiredBH < 0)
			&& (self.pos.x > ((FIELD_WIDTH - GOALBOX_WIDTH) / 2.0) + cornerCut)
			&& (desiredHeading > (180.0 + 10.0))) {
		currentBopMode = AVOIDBOX_BOP;

		double px = ((FIELD_WIDTH - GOALBOX_WIDTH) / 2.0) - boxPointOffset;
		double py = (GOALBOX_DEPTH + WALL_THICKNESS) + boxPointOffset;
		double phead = NormalizeAngle_180(RAD2DEG(atan2(py - self.pos.y,
						px - self.pos.x))
				- self.h);

		setWalkParams();
		walkType = ZoidalWalkWT;
		left = 0;
		turnCCW = CLIP(phead / 2.0, maxTurn);

		if (zoidCout)
			cout << "bop stuff 4" << endl;

		if (abs(turnCCW) <= 8) {
			forward = MAX_OFFSET_FORWARD_SMALL_TURN;
		}
		else {
			forward = MAX_OFFSET_FORWARD_LARGE_TURN;
		}        

		if (defendEyes) {
			defenceBoxAvoidREyes;
		}

		return;
	}
	else if (box
			&& ((currentBopMode == AVOIDBOX_BOP)
				|| (currentBopMode == POSTAVOID_BOP))) {
		currentBopMode = POSTAVOID_BOP;
		double relh = ballH - desiredBH;
		if (abs(relh) < 10.0) {
			currentBopMode = NORMAL_BOP;
		}
		else {
			setWalkParams();
			walkType = ZoidalWalkWT;
			left = 0;
			turnCCW = CLIP(relh / 2.0, maxTurn);

			if (zoidCout)
				cout << "bop stuff 5 " << endl;

			if (abs(turnCCW) <= 8) {
				forward = MAX_OFFSET_FORWARD_SMALL_TURN;
			}
			else {
				forward = MAX_OFFSET_FORWARD_LARGE_TURN;
			}

			if (defendEyes) {
				defencePostAvoidEyes;
			}
			return;
		}
	}
	else {
		currentBopMode = NORMAL_BOP;
	}

	if (defendEyes) {
		if (desiredBH > 0) {
			defenceBallLeftEyes;
		}
		else if (desiredBH < 0) {
			defenceBallRightEyes;
		}
		else {
			defenceBallElseEyes;
		}
	}

	double relH = ballH - desiredBH;
	turnCCW = CLIP(relH / 2.0, 40.0);

	if (ABS(turnCCW) <= 13.75) {
		if (zoidCout)
			cout << "bop stuff 6   hover stuff " << endl;

		setWalkParams();
		walkType = ZoidalWalkWT;
		if (abs(turnCCW) <= 8) {
			forward = MAX_OFFSET_FORWARD_SMALL_TURN;
		}
		else {
			forward = MAX_OFFSET_FORWARD_LARGE_TURN;
		}                
		left = 0;
	}
	else if (ABS(relH) > 18) {
                walkTurnOnlyNormal(turnCCW);
	}
	// otherwise use zoidal walk to run to the ball
	else {
		setWalkParams();
		walkType = ZoidalWalkWT;

		if (zoidCout)
			cout << "bop stuff 7  else case in hover " << endl;

		forward = MAX_OFFSET_FORWARD;
		left = 0;
	}
}

//////////////////////////////////////
// End Bird of Prey
/////////////////////////////////////

void UNSW2004::getUNSW2004HailMaryPoint(double *ptx, double *pty) {
	INSTRUMENT("UNSW2004::getUNSW2004HailMaryPoint" << endl);

	double safeBallX = ballX;
	if (TARGET_GOAL_X - safeBallX) {
		safeBallX += 0.01;
	} //avoid divide by 0

	double m = (TARGET_GOAL_Y - ballY) / (TARGET_GOAL_X - safeBallX);
	double b = TARGET_GOAL_Y - m *TARGET_GOAL_X;
	if (m == 0) {
		m += 0.01;
	} //avoid divide by 0
	double xMatchingYourY = (gps->self().pos.y - b) / m;

	if (gps->self().pos.x <= xMatchingYourY) {
		*ptx = LEFT_GOALBOX_EDGE_X - 18;
		*pty = LEFT_GOALBOX_EDGE_Y - 28;
	}
	else {
		*ptx = RIGHT_GOALBOX_EDGE_X + 18;
		*pty = RIGHT_GOALBOX_EDGE_Y - 28;
	}
}

// zoidal straight to this point till we get close, then use saGoToTargetFacingHeading()
void UNSW2004::doFastGoToPointHeading(const double x,
		const double y,
		const double globh) {
	INSTRUMENT("UNSW2004::doFastGoToPointHeading" << endl);

	double dx = x - gps->self().pos.x;
	double dy = y - gps->self().pos.y;

	double hmPtDist = dx *dx + dy *dy;
	double ptDirG = RAD2DEG(atan2(dy, dx));
	double ptDirL = NormalizeAngle_180(ptDirG - gps->self().h);
	double loch = NormalizeAngle_180(globh - gps->self().h);

	double cutoff = (180 - abs(loch)) / 4 + 10;

	//if (stealthEyes) noStealthEyes;
	if (hmPtDist > cutoff * cutoff) {
		// stealth was scrapped since he won't ever be looking forward
		// double left, right;
		//if (needStealthDog(left, right, x, y, sqrt(hmPtDist))) {
		//    doStealthDog(left, right, ptDirL);
		//} else 
		if (ABS(ptDirL) > 45) {
			setWalkParams();
			walkType = CanterWalkWT;
			forward = 0;
			left = 0;
			turnCCW = CLIP(ptDirL, 45.0);
			// cout << "turn ";
		}
		else {
			setWalkParams();
			walkType = ZoidalWalkWT;
			left = 0;
			turnCCW = CLIP(ptDirL, 13.75);

			if (zoidCout)
				cout << "do fast go to point heading stuff " << endl;

			if (abs(turnCCW) <= 8) {
				forward = MAX_OFFSET_FORWARD_SMALL_TURN;
			}
			else {
				forward = MAX_OFFSET_FORWARD_LARGE_TURN;
			}
			// cout << "zoidal ";

		}
	}
	else {
		setWalkParams();
		walkType = CanterWalkWT;
		saGoToTargetFacingHeading(x, y, globh);
		// cout << "hover ";
	}

	// cout << "dist: " << sqrt(hmPtDist) << " cutoff: " << cutoff << " goal ang: " << ptDirL << " final ang: " << loch << endl;

}

void UNSW2004::doUNSW2004HailMaryReceive() {
	INSTRUMENT("UNSW2004::doUNSW2004HailMaryReceive" << endl);
	UNSW2004GeneralHead();

	double destx, desty;

	// cout << "in doUNSW2004HailMaryReceive" << endl;

	getUNSW2004HailMaryPoint(&destx, &desty);
	double h = RAD2DEG(atan2(ballY - gps->self().pos.y,
				ballX - gps->self().pos.x));
	doFastGoToPointHeading(destx, desty, h);
}

// count the number of opponents on *their* defensive side of the goal, including their goalie
int UNSW2004::getNumOpponentsOnSide() {
	INSTRUMENT("UNSW2004::getNumOpponentsOnSide" << endl);
	int result = 0;
	const double bally = gps->getBall(GLOBAL).y;
	for (int i = 0; i < NUM_OPPONENTS; i++) {
		if ((gps->getOppCovMax(i) < get95CF(70))
				&& (gps->getOppPos(i, GLOBAL).y > bally)) {
			result++;
		}
	}
	return result;
}

// count the number of wireless teammates on our defensive side of the ball
int UNSW2004::getNumTeammatesOnSide(bool countGoalie) {
	INSTRUMENT("UNSW2004::getNumTeammatesOnSide" << endl);
	int result = 0;
	const double bally = gps->getBall(GLOBAL).y;
	for (int i = 0 ; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;

		// ignore goalie
		if ((!countGoalie) && (info.amGoalie != 0))
			continue;

		if ((tm.counter > 0) && (tm.pos.y < bally)) {
			result++;
		}
	}

	return result;
}

int UNSW2004::getMinVarOppNum() {
	INSTRUMENT("UNSW2004::getMinVarOppNum" << endl);
	int minVarOppNum = 0;
	double minVarValue = gps->getOppCovMax(0);
	for (int i = 0; i < NUM_OPPONENTS; i++) {
		double currVarValue = gps->getOppCovMax(i);
		if (currVarValue < minVarValue) {
			minVarOppNum = i;
			minVarValue = currVarValue;
		}
	}
	return minVarOppNum;
}

bool UNSW2004::ballInOwnBox(double errorMargin) {
	INSTRUMENT("UNSW2004::ballInOwnBox" << endl);

	bool cond1 = (ballY < (GOALBOX_DEPTH + WALL_THICKNESS) + errorMargin);
	bool cond2 = (ballX < ((FIELD_WIDTH + GOALBOX_WIDTH) / 2.0 + errorMargin));
	bool cond3 = (ballX > ((FIELD_WIDTH - GOALBOX_WIDTH) / 2.0 - errorMargin));
	return cond1 && cond2 && cond3;
}

bool UNSW2004::oppInTargetGoal(int oppIndex) {
	INSTRUMENT("UNSW2004::oppInTargetGoal" << endl);
	const Vector &oppPos = gps->getOppPos(oppIndex, GLOBAL);
	if (oppPos.x >= LEFT_GOALBOX_EDGE_X
			&& oppPos.x <= RIGHT_GOALBOX_EDGE_X
			&& oppPos.y >= LEFT_GOALBOX_EDGE_Y) {
		return true;
	}        
	return false;
}

int UNSW2004::numTeammatesCommunicating() {
	INSTRUMENT("UNSW2004::numTeammatesCommunicating" << endl);
	int ret = 0; 
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		const WMObj &tm = gps->tmObj(i);
		interpBehav info = (interpBehav) tm.behavioursVal;
		if (gps->tmObj(i).counter > 0)
			ret++;
	}
	return ret;
}

static const int nearEdgeDistance = WALL_THICKNESS + 20;

// May integrate with getHeadingToBestGap() from visOppAvoidanceKick.cc.
// The current implementation simply add up number of robots in the current frame.
// Later maybe change to count number of robots actually blocking the shooting
// range.
int UNSW2004::findHowManyBlockingRobot() {
	int count = 0;
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vOpponent[i].cf > 0)
			count++;
		if (vTeammate[i].cf > 0)
			count++;			
	}
	return count;
}

double UNSW2004::getUNSW2004DKD() {
	INSTRUMENT("UNSW2004::getUNSW2004DKD" << endl);

	double detX = ballX;
	double detY = ballY;

	//if you dont know where the ball is determine dkd based on where you are
	if (ballSource == GPS_BALL && gps->getBallMaxVar() >= get95CF(35)) {
		detX = gps->self().pos.x;
		detY = gps->self().pos.y;
	}

	//if ball is near own goal, desired direction becomes away from own goal
	if (detY <= FIELD_LENGTH / 2.5) {
		return RAD2DEG(atan2(detY - 0.0, detX - (FIELD_WIDTH / 2.0)));
	}

	//if the ball is near one of the side edges and not in corners then the direction becomes up the field
	if (detY > nearEdgeDistance) {
		if (detX <nearEdgeDistance || detX>(FIELD_WIDTH - nearEdgeDistance)) {
			return 90; //ie up the field
		}
	}

        // if the robot can see the target goal and it is within shooting distance and there are not many
	// robots in the current frame and the robot has a good heading to shoot, then trigger VOAK dkd.
	// The trigger condition will change after we run a game.
        if (vtGoal.cf > 0 && vtGoal.d <= 60 && findHowManyBlockingRobot() <= 3 &&
	    gps->self().h <= 110 && gps->self().h >= 70) {
                double min, max;
		getHeadingToBestGap(&min, &max);
		return (min + max) / 2;
        }

	//default desired direction is towards target goal
	return RAD2DEG(atan2(FIELD_LENGTH - detY, (FIELD_WIDTH / 2.0) - detX));
}

//////////////////////////////
// Stealth dog
//////////////////////////////

// does stealthDog assuming we are going for the ball
void UNSW2004::doStealthDog(double leftAngle, double rightAngle) {
	INSTRUMENT("UNSW2004::doStealthDog" << endl);

	doStealthDog(leftAngle, rightAngle, ballH);
}


// general stealth dog towards an arbitrary point
void UNSW2004::doStealthDog(double leftAngle, double rightAngle, double head) {
	INSTRUMENT("UNSW2004::doStealthDog" << endl);

	setWalkParams();
	walkType = ZoidalWalkWT;
	forward = MAX_OFFSET_FORWARD; //look down further, forward is over rided based on turnCCW
	left = 0;
	double turn = head;

	double badHead = (leftAngle + rightAngle) / 2.0;
	double stealthTurn;
	double trueHead = NormalizeAngle_180(gps->self().h + head);
	/*if ((trueHead > -90+(5.0*lastStealth)) && (trueHead < (60+5.0*lastStealth))) {
	  stealthTurn = rightAngle - 45.0;
	  if (stealthTurn < turn)
	  turn = stealthTurn;
	  if (stealthEyes) stealthLeftBackEyes;
	  lastStealth = 1;
	  } else if ((trueHead < -90+(5.0*lastStealth)) || (trueHead > 120)+(5.0*lastStealth)) {
	  stealthTurn = leftAngle + 45.0;
	  if (stealthTurn > turn)
	  turn = stealthTurn;
	  if (stealthEyes) stealthRightBackEyes;
	  lastStealth = -1;
	  } else */
	if (head < badHead + 5.0 * lastStealth) {
		if (!((trueHead < -90) || (trueHead > 120))) {
			stealthTurn = rightAngle - 45.0;
			if (stealthTurn < turn)
				turn = stealthTurn;
			if (stealthEyes)
				stealthLeftEyes;
			lastStealth = 1;
		}
	}
	else {
		if (!((trueHead > -90) && (trueHead < 60))) {
			stealthTurn = leftAngle + 45.0;
			if (stealthTurn > turn)
				turn = stealthTurn;
			if (stealthEyes)
				stealthRightEyes;
			lastStealth = -1;
		}
	}

	turnCCW = CLIP(turn / 2.0, maxTurn);

	if (zoidCout) {
		cout << "stealth dog stuff " << endl;
	}
	if (abs(turnCCW) <= 8) {
		forward = MAX_OFFSET_FORWARD_SMALL_TURN;
	}
	else {
		forward = MAX_OFFSET_FORWARD_LARGE_TURN;
	}
}


// determines if stealth should be used for going to the ball, if it is it returns true
// and returns a left and right angle to watch out for in the argument references
bool UNSW2004::needStealthDog(double &leftAngle, double &rightAngle) {
	INSTRUMENT("UNSW2004::needStealthDog" << endl);

	// if not using vision ball don't bother
	if (ballSource != VISION_BALL) {
		return false;
	}

	return needStealthDog(leftAngle, rightAngle, ballX, ballY, ballD);
}

// determines if stealth is required for going to an unknown point, does same as above
bool UNSW2004::needStealthDog(double &leftAngle,
		double &rightAngle,
		double locx,
		double locy,
		double locD) {
	INSTRUMENT("UNSW2004::needStealthDog" << endl);

	// if point is close don't bother
	if (locD < 30.0) {
		return false;
	}

	// if you might hit an edge don't do it
	const WMObj &self = gps->self();
	bool onLeft = utilIsOnLEdge()
		&& self.posVar <get95CF(75)
		&& (self.h> 60 && self.h < 300);
	bool onRight = utilIsOnREdge()
		&& self.posVar < get95CF(75)
		&& (self.h <120 || self.h> 240);
	bool onTop = utilIsOnTEdge()
		&& self.posVar < get95CF(75)
		&& (self.h <210 || self.h> 330);
	bool onBottom = utilIsOnBEdge()
		&& self.posVar < get95CF(75)
		&& (self.h <30 || self.h> 150);
	if (onLeft || onRight || onTop || onBottom) {
		return false;
	}

	int ind1 = -1;
	int ind2 = -1;

	for (int i = 0 ; i < NUM_TEAM_MEMBER ; i++) {
		// no such vob
		if (vOpponent[i].cf <= 3)
			continue;

		// ball closer
		if (vOpponent[i].d >= locD)
			continue;

		// opponent too close to the ball
		if (sqrt(SQUARE(vOpponent[i].x - locx) + SQUARE(vOpponent[i].y - locy))
				< 20)
			continue;

		// wrong distance to start avoiding
		if (vOpponent[i].d >= 50)
			continue;

		// shoulder charge
		if ((vOpponent[i].d < 20) && (abs(vOpponent[0].h) > 25))
			continue;

		if (ind1 == -1 || vOpponent[i].h > vOpponent[ind1].h)
			ind1 = i;

		if (ind2 == -1 || vOpponent[i].h < vOpponent[ind2].h)
			ind2 = i;
	}

	if (ind1 == -1) {
		return false;
	}

	if (sqrt(SQUARE(vOpponent[ind1].x - vOpponent[ind2].x)
				+ SQUARE(vOpponent[ind1].y - vOpponent[ind2].y)) < 20) {
		leftAngle = vOpponent[ind1].h;
		rightAngle = vOpponent[ind2].h;
	}
	else {
		if (vOpponent[ind1].d < vOpponent[ind2].d) {
			leftAngle = vOpponent[ind1].h;
			rightAngle = vOpponent[ind1].h;
		}
		else {
			leftAngle = vOpponent[ind2].h;
			rightAngle = vOpponent[ind2].h;
		}
	}
	return true;
}

//////////////////////////////
// End Stealth dog
//////////////////////////////

#endif // COMPILE_ALL_CPP
