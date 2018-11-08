/*

   Copyright 2004 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
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
 * $Id: Behaviours.cc 6487 2005-06-08 11:35:39Z alexn $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 * 
 **/

#include "Behaviours.h"

#ifdef COMPILE_ALL_CPP
#include "UNSW2004.h"
#include "defender.h"
#include "gaitOptimiser.h"
#endif //COMPILE_ALL_CPP

#include "WalkingLearner.h"
#include "pythonBehaviours.h"
#ifndef OFFLINE
#include "../vision/Vision.h"
#include <OPENR/OPENRAPI.h>
#endif // OFFLINE
#include <fstream>

//#include "joystickSkills.h"
#include "../share/RobotDef.h"

#define OUT(x) if (bWRITE_TO_CONSOLE) cout << x

using namespace Behaviours;

const bool bDebugBatteryCurrent = true;
const bool bDebugWalkCommand = false;

static const int DEFAULT_PLAYER_NUMBER = 1;

static const int REMOTE_ONLY = 0;
static const int PHOTO_TAKER = 1;
static const int FIRE_PAWKICK = 17;
static const int GAIT_OPTIMISER = 26;
static const int FIGO_DEFENDER = 27;
static const int PYTHON_PLAYER = 28;
static const int WALKING_LEARNER = 102;
static const int WALK_CALIBRATOR = 103;
static const int UNSW2004_FORWARD = 999;
static const int UNSW2004_GOALIE = 888;
static const int GENERAL_TESTER = 88;

// static variables used when TURN_KICK is set to cmdStance
static const int NO_TURN = 0;
static const int NORMAL_TURN = 1;
static const int VSOFT_TURN = 2;
static const int SMALL_TURN = 3;
static const int MED_TURN = 4;
static const int FULL_TURN = 5;

static const bool delayDebugMsg     = false;
static const bool showCurrentOnHead = false;
static const bool emonLogDebugMsg   = false;

static bool tempForStepComplete = true;

// constants for locating ball while still
static const int BALL_LOST = 5;
static const int INTERVAL = 20;

// store the default behaviour on startup
#ifdef OFFLINE
static const char *DEFAULT_PLAYER_FILE = "../../robot/cfg/player.cfg";
#else // OFFLINE
static const char *DEFAULT_PLAYER_FILE = "/MS/player.cfg";
#endif // OFFLINE

// generate the assembly code of the program when it crash
#ifdef OFFLINE
//static const char *EMON_LOG_FILE = "emon.log"; // shouldn't be used
#else // OFFLINE
static const char *EMON_LOG_FILE = "/MS/OPEN-R/emon.log";
// a notice to indicate the clear of the emon.log
static const char *EMON_LOG_CLEARED_NOTICE = "This file is clear by Behaviours::clearEmonLog";
#endif // OFFLINE

static bool crashed = false;

// max time to allow time_critical mode before it is automatically cleared, ms.
// 3 sec to allow grab-turn-shoot, but it should really be cleared as soon
// as possible.
static const unsigned long TIME_CRITICAL_MAX = 3000;


double Behaviours::cpuUsage = 0;
long   Behaviours::currentFrameID = 0;
int    Behaviours::batteryCurrent = 0;
long   Behaviours::lastFrameID = 0;
static int tail_down_count = 0; //for CPU time
bool   Behaviours::bWRITE_TO_CONSOLE = false;

#ifndef OFFLINE
static SystemTime timeCritical; // last timestamp behaviours was critical
#endif

// Private behaviours functions and variables
namespace Behaviours {
	void gotCommandBallTrack(int intValue);
	void gotCommandPlayerChange(int intValue);
	void gotCommandSetTILT(double doubleValue);
	void gotCommandSetTILT2(double doubleValue);
	void gotCommandSetPAN(double doubleValue);
	void gotCommandSetColour(int intValue);
	void gotCommandSetIndicator(int intValue, int intValue2);
	void gotCommandCalibrateTurnKick(int intValue, int intValue2);
	void doRemotePlayer();
	void setFrontRightJoints(int joint, int shoulder, int knee);
}

void Behaviours::InitBehaviours() {
	clearEmonLog();

	playerIndicatorNotSet = true;
	actionTimer = 0;
	destPan = 85;
	frame = 0;

	PLAYER_NUMBER = DEFAULT_PLAYER_NUMBER;

	//by default (just incase we are using pstep pull interface) just make  stepComplete point
	//to something that is always true... if we are using the push interface Vision.cc with reset
	//stepComplete to point to something more sensible

	stepComplete = &tempForStepComplete;

	panx = tilty = cranez = 0;
	headtype = ABS_H;

	loadDefaultPlayer(DEFAULT_PLAYER_FILE);
	initIndicators();
	initPlayer();

	// store the old player the same as player in case the control is released the first time
	oldPLAYER = PLAYER;
}

void Behaviours::initPlayer() {
	switch (PLAYER) {
#ifdef COMPILE_ALL_CPP
        case UNSW2004_FORWARD:
                                UNSW2004::initUNSW2004Forward();
                                break;
        case UNSW2004_GOALIE:	
                                UNSW2004::initUNSW2004Goalie();
                                break;	
    
        case GAIT_OPTIMISER: 
                                GaitOptimiser::initGaitOptimiser();
                                break;
        case FIGO_DEFENDER: 
                                Defender::initDefender();
                                break;

#endif // COMPILE_ALL_CPP

//        case KIM_TESTING: 
//                                Kim::initKimForward();
//                                break;
//        case KIM_FORWARD: 
//                                Kim::initKimForward();
//                                break;
        case PYTHON_PLAYER: 
                                #ifndef OFFLINE
                                PythonBehaviours::initPythonBehaviours();
                                break;
                                #endif // OFFLINE
//        case WALKING_LEARNER: 
//                                WalkingLearner::initPlayer();
//                                break;
        default: 
                                initRemoteControl();
                                break;
    }

	printCurrentPlayer();
}

void Behaviours::initRemoteControl() {
	cmdWDisplay = 0;

	cmdStationaryLocalise = 0;
	cmdActiveGpsLocalise = 0;
	cmdStance = NORMAL_STANCE;
	cmdTurnKick = 0;

	resetMovement();
}

void Behaviours::Reset() {
	initPlayer();
}

void Behaviours::printCPlusPlusValues(int selection) {
#ifdef COMPILE_ALL_CPP
	if (selection == 0) {
		double strikeX, strikeY;
		UNSW2004::getUNSW2004StrikerPoint(&strikeX, &strikeY);
		cout << "C++ strike x: " << strikeX << endl;
		cout << "C++ strike y: " << strikeY << endl;
		UNSW2004::doUNSW2004StrikerForward(strikeX, strikeY);
		cout << "C++ strike forward: " << forward << endl;
		cout << "C++ strike turnCCW: " << turnCCW << endl;
		cout << "C++ strike left:    " << turnCCW << endl;
		forward = 0;
		turnCCW = 0;
		left    = 0;
	}
#endif //COMPILE_ALL_CPP
}

void Behaviours::DecideNextAction() {
	vision->player = PLAYER;

	// Prevent execution in EVERY iteration for EVERY player when there are no wireless commands to be processed.
	if (command.RC_COMMAND != NO_COMMAND)
		processWControl();

	switch (PLAYER) {
#ifdef COMPILE_ALL_CPP
		case UNSW2004_FORWARD:
				UNSW2004::doUNSW2004Forward();
				break;
		case UNSW2004_GOALIE:		
				UNSW2004::doUNSW2004Goalie();
				break;		
		case GAIT_OPTIMISER: 
                GaitOptimiser::doGaitOptimiser();
                break;
		case FIGO_DEFENDER: 
                Defender::doDefender();
                break;
#endif //COMPILE_ALL_CPP
		case REMOTE_ONLY: 
		        doRemotePlayer();
                debugBall();
				break;
//		case KIM_TESTING: 
//                Kim::doKimTesting();
//                break;
//		case WALK_CALIBRATOR: 
//                Kim::doWalkCalibrate();
//                break;
//		case KIM_FORWARD: 
//                Kim::doKimForward();
//                break;
       case PYTHON_PLAYER: 
                #ifndef OFFLINE
                PythonBehaviours::DecideNextAction();
                break;
                #endif // OFFLINE
//		case WALKING_LEARNER: 
//                WalkingLearner::DecideNextAction();
//                break;
		default: 
                //cout << "Which player number you have assigned?" << endl;
                setDefaultParams();
                break;
    }

    //VERY BAD HACK -- FIND THE SOURCE OF THIS HAPPENING NOT HACK IT
    //PG = 5 is the set in setStandParams, setBlockParams and setHoldParams
    // Kevin has changed them to 40 , and leave a comment there, no reason why they cant be 40
    // Kevin: The reason they are set to 5 is that it is quicker to change from stand pos into a walk

    // If the robot is not standing but PG <= 5, hack it to normal PG = 40;
    if (PG <= 5 && !(forward == 0 && left == 0 && turnCCW == 0)) {
        OUT("It;s hacked at Behaviours.cc:~314" << endl);
        PG = 40;
    }
    //what case is it?
    //It's very like to be a dodgy bug here that set hF,hB =0, 
    //If your player don't call setWalkParams() every DecideNextAction() loop
    if (forward != 0 || left != 0 || turnCCW != 0) {
        if ( hdF == 0  && hdB == 0) {
            hdF = HDF_STANDARD;
            hdB = HDB_STANDARD;
        }
    }
    if (standTall)
        hF = HF_STANDARD + 15;

    makeParaWalk(walkType, minorWalkType, forward, left, turnCCW, 
                 forwardMaxStep, leftMaxStep, turnCCWMaxStep);
    lastFrameID = currentFrameID;
	prevAttackMode = attackMode;

}

// this function is only needed because it is referred by GrabBall, SpinChestPush and SpinKick.
// It is not referred in any of teamA or TeamB strategies.
void Behaviours::chooseStrategy() {
#ifdef COMPILE_ALL_CPP
	switch (PLAYER) {	
	     // Other cases had been removed by Ted. Get it back from revision 2498.
	     case UNSW2004_FORWARD: UNSW2004::chooseUNSW2004ForwardStrategy();
	     	  break;
		  
	     default: break;	     
	}
#endif //COMPILE_ALL_CPP
}

void Behaviours::chooseShootStrategy() {
#ifdef COMPILE_ALL_CPP
	switch (PLAYER) {
		case UNSW2004_FORWARD: UNSW2004::chooseUNSW2004ShootStrategy();
				      break;
		default: break;
	}
#endif //COMPILE_ALL_CPP
}

void Behaviours::setDefaultParams() {
    // Reset Walk paramters if previous step has completed
    if (*stepComplete) {
        setStandParams();
    }
    // Head parameters
    headtype = ABS_H;
    cranez = 0;    
    tilty = 0;
    panx = 0;

    // Indicator
    mouth = cmdMouth;
    tail_h = cmdTailH;
    tail_v = cmdTailV;
    ear_l = cmdEarL;
    ear_r = cmdEarR;

    led1 = cmdLed1;
    led2 = cmdLed2;
    led3 = cmdLed3;
    led4 = cmdLed4;
    led5 = cmdLed5;
    led6 = cmdLed6;
    led7 = cmdLed7;
    led8 = cmdLed8;
    led9 = cmdLed9;
    led10 = cmdLed10;
    led11 = cmdLed11;
    led12 = cmdLed12;
    led13 = cmdLed13;
    led14 = cmdLed14;

    head_color = cmdHeadColor;
    head_white = cmdHeadWhite;
    wireless = cmdWireless;

    back_front_color = cmdBackFrontColor;
    back_front_white = cmdBackFrontWhite;
    back_middle_color = cmdBackMiddleColor;
    back_middle_white = cmdBackMiddleWhite;
    back_rear_color = cmdBackRearColor;
    back_rear_white = cmdBackRearWhite;
}

/* Set time-critical mode for the next TIME_CRITICAL_MAX ms or until
 * cleared
 */
void Behaviours::setTimeCritical() {
#ifndef OFFLINE
    GetSystemTime(&timeCritical);
//    cerr << "setTimeCritical critical time = " << timeCritical.seconds
//        << ", " << timeCritical.useconds << endl;
#endif
}
void Behaviours::clearTimeCritical() {
#ifndef OFFLINE
    timeCritical.seconds = timeCritical.useconds = 0;
//    cerr << "clearTimeCritical critical time = " << timeCritical.seconds
//        << ", " << timeCritical.useconds << endl;
#endif
}
bool Behaviours::isTimeCritical() { 
#ifndef OFFLINE
    if ((timeCritical.useconds == 0) && (timeCritical.seconds == 0)) {
        //cerr << "false (cleared)" << endl;
        return false;
    }
    SystemTime now;
    GetSystemTime(&now);
    if (now.useconds > timeCritical.useconds) {
        now.seconds = now.seconds - timeCritical.seconds;
        now.useconds = now.useconds - timeCritical.useconds;
    } else {
        now.seconds = now.seconds - 1 - timeCritical.seconds;
        now.useconds = now.useconds + 1000000 
            - timeCritical.useconds;
    }
    bool isCritical = (now.seconds * 1000) + (now.useconds / 1000)
                            < TIME_CRITICAL_MAX;
    if (! isCritical)
        clearTimeCritical();
//    cerr << "isTimeCritical critical time = " << timeCritical.seconds
//        << ", " << timeCritical.useconds << " "
//        << "diff = " << now.seconds << "sec" << now.useconds << "usec = " 
//        << ((now.seconds * 1000) + (now.useconds / 1000)) << "ms -> "
//        << isCritical << endl;
    return isCritical;
#else
    return false;
#endif
}

// ##################################################################################################################
// Super Debug.
// ##################################################################################################################

// This function tries to crash the dog, so that we can study the emon.log.
void Behaviours::goAndCrashTheDog() {
    int zero = 0;
    cout << 0 / zero << endl; // Since the compiler refuses to compile 0 / 0.
}

void Behaviours::debugCurrent() {
#ifndef OFFLINE
    // Open the mouth if the current is lower than a limit.
    OPowerStatus currentStatus;
    OPENR::GetPowerStatus(&currentStatus);
    if (currentStatus.current <= -3000){
        //mouth = IND_MOUTH_OPEN;
         back_middle_white = IND_LED3_INTENSITY_MAX;
    } else{
        //mouth = IND_MOUTH_CLOSED;   
        back_middle_white = IND_LED3_INTENSITY_MIN;
    }
#endif
}

void Behaviours::debugGPS() {
    return;
    // If the robot's position var is large, then turn the head color to white.
    if (gps->self().posVar > get95CF(75))
        head_white = IND_LED2_ON;
    else
        head_white = IND_LED2_OFF;
    
    // If unsure own's heading, then turn the head color to orange.
    if (gps->self().hVar > get95CF(60))
        head_color = IND_LED2_ON;
    else
        head_color = IND_LED2_OFF;
}

void Behaviours::debugVision() {
    bool foundTeam = false;
    bool foundOpp  = false;

    // Check if seen any teammate or opponent.
    for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
        if (vTeammate[i].cf > 0)
            foundTeam = true;
        if (vOpponent[i].cf > 0)
            foundOpp = true;    
    }
    
    // If the robot see teammate and opponent then set the back-back to white + red.
    if (foundTeam && foundOpp) {
        back_front_color = IND_LED3_INTENSITY_MAX;    
        back_front_white = IND_LED3_INTENSITY_MAX;        
    }
    else {
        // If only teammate seen then set the back-back to red.
        if (foundTeam) {
            back_front_color = IND_LED3_INTENSITY_MAX;
            back_front_white = IND_LED3_INTENSITY_MIN;
        }
        else {
            // If only opponent seen then set the back-back to white.
            if (foundOpp) {
                back_front_color = IND_LED3_INTENSITY_MIN;
                back_front_white = IND_LED3_INTENSITY_MAX;                
            }
            // Now none of the robot is seen. No need to set the light indicator.
            else {
                back_front_color = IND_LED3_INTENSITY_MIN;
                back_front_white = IND_LED3_INTENSITY_MIN;
            }        
        }    
    }
    // If seen the target goal then set the back-middle to white.
    if (vtGoal.cf > 0)
        back_middle_white = IND_LED3_INTENSITY_MAX;
    else
        back_middle_white = IND_LED3_INTENSITY_MIN;

    // If seen the own goal then set the back-front to red.
    if (voGoal.cf > 0)
        back_rear_color = IND_LED3_INTENSITY_MAX;
    else
        back_rear_color = IND_LED3_INTENSITY_MIN;

    if (vBall.cf > 0) {
        showEyes(IND_LED3_INTENSITY_MAX, IND_LED3_INTENSITY_MIN, IND_LED3_INTENSITY_MIN, IND_LED3_INTENSITY_MIN, IND_LED3_INTENSITY_MIN);
    }
}

void Behaviours::debugRole() {
    switch (myRole) {
        case SUPPORT:
            yellowColor();
            break;
        case STRIKER1:
            blueColor();
            break;
        case STRIKER2:
            greenColor();
            break;
        case NORMAL:
            redColor();
            break;
        case LOCKMODE:
            purpleColor();
            break;                    
        case BACKOFF:
            whiteColor();
            break;
        case NOTHING:
            noColor();
            break;
        default:
            cout << "debugRole() has a problem!" << endl;
    }
}

void Behaviours::debugSkill() {}

void Behaviours::debugBattery() {}

void Behaviours::debugBall() {
	if (vBall.cf > 0) {
		head_color = IND_LED2_ON;
		head_white = IND_LED2_OFF;
	}
	else {
		head_color = IND_LED2_OFF;
		head_color = IND_LED2_OFF;
	}
}

// Move the tail down if frame drops.
void Behaviours::debugFrame() {

    // Frames come at a rate of 30 frames/sec.
    // If cpu can't finish processing the frame in 1/30 sec, frame drops
    // occur.
    
    // Max time before the frame become dropped.
	const double CPU_OVER = (1.0 / 30.0) * 1000000.0;

    // Avoid fluctuation of the tail by giving some limit before the tail will
    // come up again.
	const double CPU_80 = CPU_OVER * 0.8; 

	const double FRAME_SKIP = CAMERA_FRAME_MS*1.2 / JOINT_FRAME_MS + 2;

    // Each frame-drop occur will at least incur how many frames of tail down.
	const int TAIL_DOWN_DURATION = 25;
	//static const int TAIL_INCREMENT_25 = (IND_TAIL_RIGHT - IND_TAIL_LEFT) / 25;

	// Tail indicator for CPU time.
	if (tail_down_count <= 0) {
		if (currentFrameID - lastFrameID >= FRAME_SKIP && lastFrameID > 0) {
			tail_v = IND_TAIL_DOWN;
			tail_down_count = TAIL_DOWN_DURATION;
			cout << "FRAME DROPPED. Old: " << lastFrameID << " new: " << currentFrameID << endl;
		}
		else {
			if (cpuUsage < CPU_80) {
				tail_v = IND_TAIL_UP;
			}
			else {
				tail_v = MAX((IND_TAIL_UP - (int) (((cpuUsage - CPU_80) / (CPU_OVER - CPU_80)) * IND_TAIL_UP)), 0);
			}
		}
	}
	else {
		tail_v = IND_TAIL_DOWN;
		tail_down_count--;
	}
}

// This function displays all the indicators according to the rules and debugging.
// From 2004, this function replace setIndicator().
void Behaviours::superDebug() {
    // Turn this on if you want to make the indicators for only new rule.
    bool onlyNewRule = false;

    // If not not only new rule, debug the current.
    if (!onlyNewRule)
        debugCurrent();

    // The back three indicators are designed for kickoff and team information.
    // This is done by the GameController now
    //showTeamIndicator();

    // Display role debug if not only using the new rule.
    if (!onlyNewRule)
        debugRole();

    debugBall();
    debugFrame();

    // debugBattery();
    // debugGPS();
    // debugVision();
    // debugSkill();

}

// This function checks all important behaviour variables and see if there any broken variable caused by GPS, Vision and ActuatorControl.
void Behaviours::checkAllBehaviourVariables() {
    cout << "####################################" << endl;
    bool team = false;
    bool opp  = false;
    for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
        if (vTeammate[i].cf > 0)
            team = true;
        cout << "Teammate " << i << " distance: " << vTeammate[i].d << " ,with cf: " << vTeammate[i].cf << endl;    
    }
    for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
        if (vOpponent[i].cf > 0)
            opp = true;
        cout << "Opponent " << i << " distance: " << vOpponent[i].d << " ,with cf: " << vOpponent[i].cf << endl;    
    }
}

void Behaviours::greenColor() {
    mode_green = IND_LED2_ON;
    mode_blue  = IND_LED2_OFF;
    mode_red   = IND_LED2_OFF;
}

void Behaviours::redColor() {
    mode_green = IND_LED2_OFF;
    mode_blue  = IND_LED2_OFF;
    mode_red   = IND_LED2_ON;
}

void Behaviours::blueColor() {
    mode_green = IND_LED2_OFF;
    mode_blue  = IND_LED2_ON;
    mode_red   = IND_LED2_OFF;
}

void Behaviours::yellowColor() {
    mode_green = IND_LED2_ON;
    mode_blue  = IND_LED2_OFF;
    mode_red   = IND_LED2_ON;
}

void Behaviours::purpleColor() {
    mode_green = IND_LED2_OFF;
    mode_blue  = IND_LED2_ON;
    mode_red   = IND_LED2_ON;
}

void Behaviours::whiteColor() {
    mode_green = IND_LED2_ON;
    mode_blue  = IND_LED2_ON;
    mode_red   = IND_LED2_ON;
}

void Behaviours::lightBlueColor() {
    mode_green = IND_LED2_ON;
    mode_blue  = IND_LED2_ON;
    mode_red   = IND_LED2_OFF;
}

void Behaviours::noColor() {
    mode_green = IND_LED2_OFF;
    mode_blue  = IND_LED2_OFF;
    mode_red   = IND_LED2_OFF;
}

// ##################################################################################################################
// ##################################################################################################################

void Behaviours::gotCommandBallTrack(int intValue) {
#ifdef COMPILE_ALL_CPP
    if (intValue == 0) {
        JoystickSkills::setBallTrk(false);
    }
    else {
        JoystickSkills::setBallTrk(true);
    }
#endif // COMPILE_ALL_CPP
}

void Behaviours::gotCommandPlayerChange(int intValue) {
    if (PLAYER != intValue) {
        // switch if player is different
        PLAYER = intValue;
        initPlayer();
    }
}

void Behaviours::gotCommandSetTILT(double doubleValue) {
    cmdTilt = doubleValue;
    cmdHeadtype = REL_H;
}

void Behaviours::gotCommandSetTILT2(double doubleValue) {
    cmdCrane = doubleValue;
    cmdHeadtype = REL_H;
}

void Behaviours::gotCommandSetPAN(double doubleValue) {
    cmdPan = doubleValue;
    cmdHeadtype = REL_H;
}

void Behaviours::gotCommandSetColour(int intValue) {
    if ((uint32)intValue == PLAYER_NUMBER) {
        isRedRobot = true;
    }
    else if (intValue == 0) {
        isRedRobot = false;
    }
    else {
        return;
    }

    gps->SetGoals(isRedRobot);

#ifndef OFFLINE
    transmission->sendPlayerNumber(PLAYER_NUMBER, isRedRobot);
#endif

    cout << "Robot colour changed to: ";

    if (isRedRobot) {
        cmdModeRed = 2;
        cmdModeGreen = 1;
        cmdModeBlue = 1;
        cout << "Red" << endl;
    } else {
        cmdModeRed = 1;
        cmdModeGreen = 1;
        cmdModeBlue = 2;
        cout << "Blue" << endl;
    }
}

/* intValue2 should range from IND_LED3_INTENSITY_MIN to ... MAX. */
/* Max value is 256. */

/* NOTE: Right now there's no where that will set the REAL leds using the
** cmdLeds variables. So by this function itself, it's not going to change the
** display. Something needs to be done in DecideNextAction to achieve it. */
void Behaviours::gotCommandSetIndicator(int intValue, int intValue2) {
    // Change the indicator specified.
    switch (intValue) {
        case 1:  cmdMouth = intValue2;
                 mouth = cmdMouth;
                 break;
        case 2:  cmdTailH = intValue2;
                 tail_h = cmdTailH;
                 break;
        case 3:  cmdTailV = intValue2;
                 tail_v = cmdTailV;
                 break;
        case 4:  cmdEarL = intValue2;
                 ear_l = cmdEarL;
                 break;
        case 5:  cmdEarR = intValue2;
                 ear_r = cmdEarR;
                 break;
        case 6:  cmdLed1 = intValue2;
                 led1 = cmdLed1;
                 break;
        case 7:  cmdLed2 = intValue2;
                 led2 = cmdLed2;
                 break;
        case 8:  cmdLed3 = intValue2;
                 led3 = cmdLed3;
                 break;
        case 9:  cmdLed4 = intValue2;
                 led4 = cmdLed4;
                 break;
        case 10: cmdLed5 = intValue2;
                 led5 = cmdLed5;    
                 break;
        case 11: cmdLed6 = intValue2;
                 led6 = cmdLed6;
                 break;
        case 12: cmdLed7 = intValue2;
                 led7 = cmdLed7;
                 break;
        case 13: cmdLed8 = intValue2;            
                 led8 = cmdLed8;
                 break;
        case 14: cmdLed9 = intValue2;
                 led9 = cmdLed9;
                 break;
        case 15: cmdLed10 = intValue2;
                 led10 = cmdLed10;	             
                 break;
        case 16: cmdLed11 = intValue2;
                 led11 = cmdLed11;
                 break;
        case 17: cmdLed12 = intValue2;
                 led12 = cmdLed12;
                 break;
        case 18: cmdLed13 = intValue2;
                 led13 = cmdLed13;
                 break;
        case 19: cmdLed14 = intValue2;
                 led14 = cmdLed14;
                 break;
        case 20: cmdHeadColor = intValue2; 
                 head_color = cmdHeadColor;
                 break;
        case 21: cmdHeadWhite = intValue2;
                 head_white = cmdHeadWhite;
                 break;
        case 22: cmdModeRed = intValue2;
                 mode_red = cmdModeRed;
                 break;
        case 23: cmdModeGreen = intValue2;
                 mode_green = cmdModeGreen;
                 break;
        case 24: cmdModeBlue = intValue2;
                 mode_blue = cmdModeBlue;
                 break;
        case 25: cmdWireless = intValue2;
                 wireless = cmdWireless;
                 break;
        case 26: cmdBackFrontColor = intValue2;
                 back_front_color = cmdBackFrontColor;
                 break;
        case 27: cmdBackFrontWhite = intValue2;
                 back_front_white = cmdBackFrontWhite;
                 break;
        case 28: cmdBackMiddleColor = intValue2;
                 back_middle_color = cmdBackMiddleColor;
                 break;
        case 29: cmdBackMiddleWhite = intValue2;
                 back_middle_white = cmdBackMiddleWhite;
                 break;
        case 30: cmdBackRearColor = intValue2;
                 back_rear_color = cmdBackRearColor;
                 break;
        case 31: cmdBackRearWhite = intValue2;
                 back_rear_white = cmdBackRearWhite;
                 break;
    }
}

void Behaviours::gotCommandCalibrateTurnKick(int intValue, int intValue2) {
  /*switch (intValue) {
        case 1: turnAmount = intValue2;
                 break;
        default: break;
    }*/
}

/* Remote player store its own command, the wireless command set these values, 
   and DecideNextAction calls doRemotePlayer, hence executes these commands
   */
void Behaviours::doRemotePlayer() {

    setStandParams();

    headtype = cmdHeadtype;
    panx     = cmdPan;
    tilty    = cmdTilt;
    cranez   = cmdCrane;

    walkType = cmdWalktype;
    forward  = cmdForward;
    left     = cmdLeft;
    turnCCW  = cmdTurn;

    PG = cmdpg;
    hF  = cmdhf;
    hB  = cmdhb;
    hdF = cmdhdf;
    hdB = cmdhdb;
    ffO = cmdffo;
    fsO = cmdfso;
    bfO = cmdbfo;
    bsO = cmdbso;
}

// Removed setFrontRightJoints(). Revision 2499.

void Behaviours::setDogRelaxed() {
    cout << "Behaviour::Relaxing" << endl;
    //TODO: use global nextAA thing
    AtomicAction aa ;
    aa.cmd = aaRelaxed;

#ifndef OFFLINE
    transmission->sendAA(aa);
#endif //OFFLINE
}


void sendJointDebugCommand(){
	cout << "Behaviour::enable joint debug" << endl;
	//TODO: use global nextAA thing
	AtomicAction aa ;
	aa.cmd = aaTurnOnJointDebug;
#ifndef OFFLINE
	Behaviours::transmission->sendAA(aa);
#endif //OFFLINE
}

void setJointSpeedLimit(double speedLimit){
    cout << "Behaviour::got joint speed limit" << speedLimit << endl;
    //TODO: use global nextAA thing
    AtomicAction aa ;
    aa.cmd = aaSetJointSpeedLimit;
    aa.forwardSpeed = speedLimit*1000; //work around int,double conflict
#ifndef OFFLINE
    Behaviours::transmission->sendAA(aa);
#endif //OFFLINE
}

void gotReloadSpecialCommand(int playerNum){
    if ((uint32)playerNum == Behaviours::PLAYER_NUMBER){
        cout << "Behaviour::got reload special actions command " << endl;
        //TODO: use global nextAA thing
        AtomicAction aa ;
        aa.cmd = aaReloadSpecialAction;
#ifndef OFFLINE
        Behaviours::transmission->sendAA(aa);
#endif //OFFLINE
    }
}

//TODO: move to other file
void shutDownRobot(){

    #ifndef OFFLINE
        OBootCondition bootCond(obcbPAUSE_SW);
        OStatus result = OPENR::Shutdown(bootCond);
        if (result != oSUCCESS){
            OSYSLOG1((osyslogERROR, "%s : %d",
                        "OPENR::Shutdown() FAILED", result));
        }
    #endif
}

void gotSetGainCommand(int value){
    if (value == 0)
        setLowGain();
    else
        setHighGain();
}

/*
 * Here is how processWControl works:
 *      If you want to test something from wireless base, do it in remote player.
 *      If you want to send command to your player, call , say , Kim::gotWirelessCommand(...)

 * If you want to check what commands from RoboCommander match which
   RC_COMMAND value, check matchArray in share/CommandData.cc.
 *      
 */
void Behaviours::processWControl() {

    double doubleValue;
    int intValue, intValue2, intValue3;

    doubleValue = command.value;
    intValue = (int) (doubleValue);
    intValue2 = (int) (command.value2);
    intValue3 = (int) (command.value3);

    DAN_DEBUG(
        //cout << "intValue: " << intValue << endl;
        //cout << "intValue2: " << intValue2 << endl;
    );


    switch (command.RC_COMMAND) {
        case BALL_TRK: gotCommandBallTrack(intValue);
                   break;

        case PLAYER_CHANGE: gotCommandPlayerChange(intValue);
                    break;

        case WALKPARAM1_CHANGE: cmdhf = doubleValue;
                    cmdhb = command.value2;
                    cmdhdf = command.value3;
                    break;

        case WALKPARAM2_CHANGE: cmdhdb = doubleValue;
                    cmdffo = command.value2;
                    cmdfso = command.value3;
                    break;

        case WALKPARAM3_CHANGE: cmdbfo = doubleValue;
                    cmdbso = command.value2;
                    cmdpg = intValue3;
                    break;

        case WALKTYPE_CHANGE: 
                    cmdWalktype = intValue;
                    cout << "Got walktype " << intValue << endl;

                    break;

        case FORWARD_MOVE: cmdForward = doubleValue;
                   break;

        case LEFT_MOVE: cmdLeft = doubleValue;
                break;

        case TURN_MOVE: cmdTurn = doubleValue;
                break;

        case SET_TILT: gotCommandSetTILT(doubleValue);
                   break;

        case SET_TILT2: gotCommandSetTILT2(doubleValue);
                break;

        case SET_PAN: gotCommandSetPAN(doubleValue);
                  break;

        case DO_BLOCK_STANCE: cmdStance = BLOCK_STANCE;
                      resetMovement();
                      cmdHeadtype = REL_H;
                      break;

        case DO_HOLD_STANCE: cmdStance = HOLD_STANCE;
                     resetMovement();
                     cmdHeadtype = REL_H;
                     break;

        case NORMAL_STANCE_ABSH: cmdStance = NORMAL_STANCE;
                     resetMovement();
                     cmdHeadtype = ABS_H;
                     break;

        case NORMAL_STANCE_RELH: cmdStance = NORMAL_STANCE;
                     resetMovement();
                     cmdHeadtype = REL_H;
                     break;

        case SET_STANCE: cmdStance = intValue;
                 //resetMovement();
                 cmdHeadtype = REL_H;
                 break;

        case DO_TURN_KICK: cmdStance = TURN_KICK;
                   cmdTurnKick = intValue;
                   resetMovement();
                   cmdHeadtype = REL_H;
                   break;

        case PAN_TILT: cmdPan = doubleValue;
                   cmdTilt = command.value2;
                   cmdHeadtype = ABS_PT; // pan first
                   break;

        case TILT_PAN: 
                   cmdPan = doubleValue;
                   cmdTilt = command.value2;
                   cmdHeadtype = ABS_H; // tilt first
                   break;

        case MOVE_COMMAND: cmdForward = doubleValue;
                   cmdLeft = command.value2;
                   cmdTurn = command.value3;
                   //walkType = ZoidalWalkWT;
                   break;
        case HEAD_MOVE:
                   cmdPan = doubleValue;
                   cmdTilt = command.value2;
                   cmdCrane = command.value3;
                   cmdHeadtype = ABS_H;
                   break;

        case HUMAN_CONTROL: if ((uint32)intValue == PLAYER_NUMBER) {
                        // take control
                        oldPLAYER = PLAYER;
                        PLAYER = REMOTE_ONLY;
                    }
                    else if (intValue == 0) {
                        // release control
                        PLAYER = oldPLAYER;
                    }
                    break;

        case SET_COLOUR: gotCommandSetColour(intValue);
                 break;

        case SET_INDICATOR: 
                    cout << "SET_INDICATOR" << endl;
                    gotCommandSetIndicator(intValue, intValue2);
                    break;

        case EDGE_DETECT_DEBUG:
                    break;

        case BEACON_DEBUG: gps->sendDebug = true;
                   gps->debugCounter = 0;
                   gps->debugParm = intValue;
                   break;

        case SET_BEACON_DIST_CONST: // disabled
                   break;
                   /*
                   vision->beaconDistanceConstant = doubleValue;
                    cout << "Setting Beacon Distance Constant to: "
                        << vision->beaconDistanceConstant << "\n";
                                break;
                    */
        case CAL_TURN_KICK: gotCommandCalibrateTurnKick(intValue, intValue2);
                            break;

        case WRITE_TO_CONSOLE: bWRITE_TO_CONSOLE = !bWRITE_TO_CONSOLE; //toggle sothat debugging info is written to (wireless) console
                               break;

                               /*
                                  case FRONT_RIGHT_JOINTS:
                                  setFrontRightJoints(intValue,intValue2,intValue3);
                                  command.clear();
                                  break;
                                */

        case RELAX_DOG: 
                               setDogRelaxed();
                               crashed = false;
                               command.clear();
                               break;

//        case WALK_LEARNING: WalkingLearner::gotWirelessCommand(command.rawValue);
//                    command.clear();
//                    break;
//                case WALK_CALIBRATE:
//                            Kim::calibrateWalk(command.rawValue);
//                                    command.clear();
//                                    break;

            case SET_JOINT_SPEED_LIMIT:
                            setJointSpeedLimit(command.value);
                            command.clear();
                            break;
            case SEND_JOINT_DEBUG:
                            sendJointDebugCommand();
                            command.clear();
                            break;
            case SHUT_DOWN_ROBOT:
                            shutDownRobot();
                            break;
#ifndef OFFLINE
            case PYTHON_COMMAND:
                PythonBehaviours::gotWirelessCommand(command.rawValue);
                command.clear();
                break;
            case COMMAND_TO_PYTHON_MODULE:
                // will send it to Python
                PythonBehaviours::gotWirelessPythonCommand(command.rawValue);
                command.clear();
                break;
            case SET_CPLANE_SENDING_RATE:
                CPLANE_SEND_INTERVAL = intValue;
                SUBVISION_SEND_INTERVAL = intValue;
                cout << "change CPLANE_INTERVAL = " << CPLANE_SEND_INTERVAL
                    << endl;
                command.clear();
                break;
#endif // OFFLINE
            case CHANGE_PIDGAIN:
                gotSetGainCommand(intValue);
                command.clear();
                break;
            case RELOAD_SPECIAL_ACTION:
                gotReloadSpecialCommand(intValue);
                command.clear();
                break;
              
            default: break;
    }

    //command.clear();

    if (abs(forward) + abs(left) + abs(turnCCW) > 0) {
        if (bDebugWalkCommand)
            cout << "processWControl(): " << walkTypeToStr[walkType] << " " << forward << " " << left << " " << turnCCW << endl;
    }

    if (cmdActiveGpsLocalise) {
        if (cmdActiveGpsLocalise == 1) {
            forward = 0;
        }
        else {
            turnCCW = 20;
            cmdActiveGpsLocalise--;
        }
    }

}

void Behaviours::resetMovement() {
    cmdForward = 0;
    cmdLeft = 0;
    cmdTurn = 0;
    cmdWalktype = 0;
    cmdTilt = 0;
    cmdCrane = 0;
    cmdPan = 0;

    setStandParams();

    cmdhf = hF;
    cmdhb = hB;
    cmdhdf = hdF;
    cmdhdb = hdB;
    cmdffo = ffO;
    cmdfso = fsO;
    cmdbfo = bfO;
    cmdbso = bsO;
    cmdpg = PG;
}

//the angle that the body makes with the ground (in Behaviours 's perspective)
double Behaviours::getBodyTilt() {
    return asin((hB - hF) / BODY_LENGTH);
}

// Kevin: Previously PG = 5 in stand, but why?
void Behaviours::setStandParams() {
    acCmd = aaParaWalk;
    walkType = NormalWalkWT;
    forward = 0;
    left = 0;
    turnCCW = 0;
    PG = 40;
    hF = HF_STANDARD;
    hB = HB_STANDARD;
    hdF = 0;
    hdB = 0;
    ffO = FFO_STANDARD;
    fsO = FSO_STANDARD;
    bfO = BFO_STANDARD;
    bsO = BSO_STANDARD;
}
/*void Behaviours::makeEdgeWalk(double Forward, double Left, double turnCCW,
		int headType, double panx, double tilty, double cranez) {
	makeParaWalk(NormalWalkWT, Forward, Left, turnCCW, 60, 75, 110, 5, 25, 60, 5, -50, 5, headType, panx, tilty, cranez);
	}*/
void Behaviours::setWalkParams() {
    walkType = NormalWalkWT;
    PG = PG_STANDARD;
    hF = HF_STANDARD;
    hB = HB_STANDARD;
    hdF = HDF_STANDARD;
    hdB = HDB_STANDARD;
    ffO = FFO_STANDARD;
    fsO = FSO_STANDARD;
    bfO = BFO_STANDARD;
    bsO = BSO_STANDARD;
}

void Behaviours::setCanterParams() {
    // Hack into Normal WalkType
    setWalkParams();
}

void Behaviours::setOffsetParams() {
    // Hack into Normal WalkType
    setWalkParams(); 
}

void Behaviours::setOmniParams() {
    walkType = NormalWalkWT; //CanterWalkWT;
    PG = PG_STANDARD;
    hF = HF_STANDARD; 
    hB = HB_STANDARD;
    hdF = 2;
    hdB = 25;
    ffO = FFO_STANDARD;
    fsO = FSO_STANDARD;
    bfO = BFO_STANDARD;
    bsO = BSO_STANDARD;
}

// Kevin: Previously PG = 5 in stand, but why?
void Behaviours::setBlockParams() {
    walkType = NormalWalkWT; //CanterWalkWT;
    forward = 0;
    left = 0;
    turnCCW = 0;
    PG = 40;
    hF = 30;  // height of shoulder
    hB = 70;  // height of hip
    hdF = 0;   // how high front paw is lifted when walking
    hdB = 0;   // how high back paw is lifted when walking
    ffO = 60;  //ffO     = 100; // how far forward the front paws are
    fsO = 110; //fsO     = 70; // how far outwards from the body the front paws are
    bfO = -50; // how far forward the back paws are
    bsO = 15;  // how far outwards from the body the back paws are
}

// Kevin: Previously PG = 5 in stand, but why?
void Behaviours::setHoldParams() {
    walkType = NormalWalkWT; //CanterWalkWT;
    forward = 0;
    left = 0;
    turnCCW = 0;
    PG = 40;
    hF = HF_STANDARD;
    hB = HB_STANDARD;
    hdF = 0;
    hdB = 0;
    ffO = 110;
    fsO = -7;
    bfO = -55;
    bsO = 15;
}

void Behaviours::setHoldSideWalkParams() {
    walkType = NormalWalkWT; // CanterWalkWT
    forward = 0;
    left = 0;
    turnCCW = 0;
    PG = 40;
    hF = HF_STANDARD;
    hB = HB_STANDARD;
    hdF = 5;
    hdB = 25;
    ffO = 85;
    fsO = 15;
    bfO = -55;
    bsO = 15;
}
/*	static inline void makeTurnWithBall(double turnCCW) {
		makeParaWalk(CanterWalkWT,
				0,-turnCCW / 6.0,turnCCW,
				40,70,110,5,25,	100,-12,-55,15);
	}
*/
void Behaviours::setCarryParams() {
    walkType = NormalWalkWT; //CanterWalkWT;
    PG = 40;
    hF = HF_STANDARD;
    hB = HB_STANDARD;
    hdF = 0;
    hdB = 30;
    ffO = 97.5;
    fsO = -10;//-15;
    bfO = -55;
    bsO = 15;
}


// ATTENTION: VERY SERIOUS

// 1. I write this to replace any action that holds the ball and turn,
// it avoids using turn & left together, (as in the 210 does), so it
// wont crash the dog and dose it fast

// 2. Make sure the following vaiables are not re-written again later 
// in the loop, or make sure it is the last function that sets this
// variables in the behaviors frame. otherwise it doesnt work and 
// dont blame, simply  donno how to use it ^_^

void Behaviours::setTurnKickParams(double turnccw) {
    walkType = NormalWalkWT;

    PG = PG_STANDARD;
    hF = HF_STANDARD;
    hB = HB_STANDARD;
    hdF = 5;
    hdB = 30;
    ffO = 70;
    fsO = 0;
    bfO = BFO_STANDARD;
    bsO = BSO_STANDARD;

    forward = 0;
    left    = 0;
    turnCCW = turnccw;

    headtype = ABS_H;

    panx    = 0;
    tilty   = -60;
    cranez  = 30;
}

void Behaviours::printCurrentPlayer() {
    cout << "Current Player: " << PLAYER << " - ";
    switch (PLAYER) {
        case REMOTE_ONLY: cout << "Remote only" << endl;
                  break;
//        case KIM_TESTING: cout << "Kim's testing" << endl;
//                  break;
//        case WALK_CALIBRATOR: cout << "Kim's walk calibrator" << endl;
//                  break;
//        case KIM_FORWARD: cout << "Kim's Forward" << endl;
//                          break;
        case FIGO_DEFENDER: cout << "Figo Defender" << endl;
                  break;
        case PYTHON_PLAYER: cout << "Python player" << endl;
                  break;
//        case WALKING_LEARNER: cout << "Walking learner" << endl;
//                      break;
        default: cout << "Unknown Player " << PLAYER << endl;
             break;
    }
}

// load the default player from a file on startup
void Behaviours::loadDefaultPlayer(const char *filename) {
	ifstream playerFile(filename);
	if (playerFile) {
		playerFile >> PLAYER;
		playerFile >> kickInix;
		playerFile >> kickIniy;
		playerFile >> kickInih;
		playerFile >> recvInix;
		playerFile >> recvIniy;
		playerFile >> recvInih;
		playerFile >> findPositionx;
		playerFile >> findPositiony;
		playerFile.close();
	}
}


void Behaviours::clearEmonLog() {
#ifndef OFFLINE    
	if (emonLogDebugMsg) {
		cout << "In clearEmonLog" << endl;
	}
	ofstream outFile(EMON_LOG_FILE);
	if (emonLogDebugMsg) {
		cout << "writing to " << EMON_LOG_FILE << endl;
	}
	if (outFile.fail()) {
		cout << "Error open " << EMON_LOG_FILE << endl;
		return;
	}
	if (emonLogDebugMsg) {
		cout << "in the if statement" << endl;
	}
	outFile << EMON_LOG_CLEARED_NOTICE << endl;
	if (emonLogDebugMsg) {
		cout << "before closing the " << EMON_LOG_FILE << endl;
	}
	outFile.close();
	cout << EMON_LOG_FILE << " cleared" << endl;
	if (emonLogDebugMsg) {
		cout << "In clearEmonLog **" << endl;
	}
#endif // OFFLINE
}

// eunice
// set the default indicators
void Behaviours::initIndicators() {
	cmdMouth = IND_MOUTH_CLOSED;
	cmdTailH = IND_TAIL_H_CENTRED;
	cmdTailV = IND_TAIL_V_CENTRED;
	cmdEarL = IND_EAR_UP;
	cmdEarR = IND_EAR_UP;
	cmdLed1 = cmdLed2 = cmdLed3 = cmdLed4 = cmdLed5 = cmdLed6 = cmdLed7 = IND_LED3_INTENSITY_MIN;
	cmdLed8 = cmdLed9 = cmdLed10 = cmdLed11 = cmdLed12 = cmdLed13 = cmdLed14 = IND_LED3_INTENSITY_MIN;
	cmdHeadColor = cmdHeadWhite = IND_LED2_OFF;    
	cmdModeRed = cmdModeGreen = cmdModeBlue = IND_LED2_OFF;    
	cmdWireless = IND_LED2_OFF;
	cmdBackFrontColor = cmdBackFrontWhite = IND_LED3_INTENSITY_MIN;
	cmdBackMiddleColor = cmdBackMiddleWhite = IND_LED3_INTENSITY_MIN;
	cmdBackRearColor = cmdBackRearWhite = IND_LED3_INTENSITY_MIN;

	// Do not init led values.
	mode_led1 = mode_led2 = mode_led3 = mode_led4 = mode_led5 = mode_led6 = mode_led7 = IND_LED3_MODE_A;
	mode_led8 = mode_led9 = mode_led10 = mode_led11 = mode_led12 = mode_led13 = mode_led14 = IND_LED3_MODE_A;
	head_color = head_white = IND_LED2_OFF;
	mode_green = IND_LED2_OFF;
	back_front_color = back_front_white = IND_LED3_INTENSITY_MIN;
	back_middle_color = back_middle_white = IND_LED3_INTENSITY_MIN;
	back_rear_color = back_rear_white = IND_LED3_INTENSITY_MIN;
}

double Behaviours::requiredAccuracy(double reqGoalWidth) {
	return requiredAccuracy(gps->self().pos.x, gps->self().pos.y, reqGoalWidth);
}

//returns the angle (either side) from the point specified to the goal posts
double Behaviours::requiredAccuracy(double tmx,
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

namespace Behaviours {
    int myRole;
	
    // team/player information
    uint32 PLAYER_NUMBER;
    uint32 TEAM_NUMBER;    
    RoboCupGameControlData gameData, prevGameData;
    int gameDataTime = 0;
    TeamInfo myTeam;
    
	CommonSense *sensors = NULL;
	VisualCortex *vision = NULL;
	GPS *gps = NULL;
    #ifndef OFFLINE
	    Vision *transmission = NULL;
    #endif // OFFLINE

	int *leading_leg     = NULL;
	int *currentWalkType = NULL;
	int *shareStep_      = NULL;
	int *currentPG       = NULL;
	double *desiredPan   = NULL;
	double *desiredTilt  = NULL;
    double *desiredCrane = NULL;
	bool   *stepComplete = NULL;

	int kickOff;

	// the system wide camera frame counter
	unsigned long frame;

	int cmdWDisplay;
	int cmdBurst;
	int burstCounter = 0;
	int cmdStationaryLocalise;
	int cmdTurnKick;
	int cmdActiveGpsLocalise;
	int cmdStance;

	double cmdForward, cmdLeft, cmdTurn, cmdTilt, cmdCrane, cmdPan;
	double cmdhf, cmdhb, cmdhdf, cmdhdb, cmdffo, cmdfso, cmdbfo, cmdbso;
	int cmdpg;
	int cmdHeadtype, cmdWalktype;
	int cmdMouth, cmdTailH, cmdTailV, cmdEarL, cmdEarR;
	int cmdLed1, cmdLed2, cmdLed3, cmdLed4, cmdLed5, cmdLed6, cmdLed7;
	int cmdLed8, cmdLed9, cmdLed10, cmdLed11, cmdLed12, cmdLed13, cmdLed14;
	int cmdHeadColor, cmdHeadWhite;
	int cmdModeRed, cmdModeGreen, cmdModeBlue;
	int cmdWireless;
	int cmdBackFrontColor, cmdBackFrontWhite;
	int cmdBackMiddleColor, cmdBackMiddleWhite;
	int cmdBackRearColor, cmdBackRearWhite;
	bool isRedRobot;
	int PLAYER;   // store the current behaviour
	int oldPLAYER;
	double kickInix, kickIniy, kickInih;
	double recvInix, recvIniy, recvInih;
	double findPositionx;
	double findPositiony;
	int walkType, minorWalkType;
	int acCmd = aaParaWalk;
	double forward, left, turnCCW;
        double forwardMaxStep, leftMaxStep, turnCCWMaxStep;
	double hF, hB, hdF, hdB, ffO, fsO, bfO, bsO;
	int PG;
	int headtype;
	double panx, tilty, cranez;

	int mouth, tail_h, tail_v, ear_l, ear_r;
	int mode_led1,  mode_led2,  mode_led3,  mode_led4, mode_led5,
	    mode_led6,  mode_led7,  mode_led8,  mode_led9, mode_led10,
	    mode_led11, mode_led12, mode_led13, mode_led14;
	int led1,  led2,  led3,  led4,  led5, led6, led7, led8, led9,
	    led10, led11, led12, led13, led14;
	int head_color, head_white;
	int mode_red, mode_green, mode_blue;
	int wireless;
	int back_front_color, back_front_white;
	int back_middle_color, back_middle_white;
	int back_rear_color, back_rear_white;

	CurObject vBall, voGoal, vtGoal;    // objects that the robot can see now
	CurObject vTeammate[NUM_TEAM_MEMBER], vOpponent[NUM_TEAM_MEMBER];

        // objects info from the world model
	CurObject wmBall, wmtGoal, wmoGoal, wmTeammate[NUM_TEAM_MEMBER], wmOpponent[NUM_TEAM_MEMBER];
	
	// wireless ball
	CurObject wlBall;

	// Robots with the same colour as your own team from wireless
	// This is different to your teammates telling you where they, themselves are located.
	// This is the teammate robots from other teammates sensors.
	CurObject wlTeammate[NUM_TEAM_MEMBER];
	// Robots with different colour to your team from wireless
	CurObject wlOpponent[NUM_TEAM_MEMBER];

	double hPan, hTilt, hCrane;    // in degrees
	bool playerIndicatorNotSet;

	CommandData command;

    int actionTimer;
	char dataTesting[2 * 196 * 144];

	double destPan;
	double debugHf;
	double debugHdf;
	double debugWalkType;
	double debugForward;
	double debugLeft;
	double debugTurn;
	bool   debugMsg;

	int lostBall = 0;             // num frames ball unseen
	double lastTrackHead;         // last heading that you were tracking
	// (as set by saBallTracking or other source
	int lastSawBeacon;            // num frames last seen beacon
	bool isWirelessDown;          // true iff wireless is down
	double useZoidWalk;           // whether to use zoidalwalk or not.
	double dkd;                   // optimal kick direction
	double ballUnderChinTilt;     // tilt when ball under chin
	int attackMode;               // current far ball action
	int prevAttackMode;           // previous far ball action
	int attackLock;
	int grabTime;                 // num frames ball grabbed  
	int grabInterval;             // num frames disallow ball grab
	int seeBall;                  // num frames ball seen

	// find ball variables
	int lookdir;                  // direction to look first when find ball
	int turnDir;                  // direction to turn in finding ball
	int lookAroundCounter;
    double lastHPan;
    double lastHTilt;
    double lastHCrane;
	double lastBallTilt;          // the last tilt when tracking ball
	bool standTall;
	int role;
	int closestRobotToBall;
	bool is3rdClosest;
	int backoffTeammateIndex;
	int lockMode;
	int prevLockMode;
	int findGoalTimer;
	int carryTimer;
	double goalPan;
	int holdTimer;
	int lostBallTimer;
	int backoffTimer;
	int kickTimer;
	int blockTimer;
	int state;
	double prevDistToBall;
	double ballx;
	double bally;
	int spinDirection;

	// note that there is another (static) defn of teammateSeenTime in Utility.cc
	int teammateSeenTime; // number of frames teammate seen
	bool allowedToGoForBall;
	int panDirection;
	int tiltDirection;
}

static void CLIP2(double &x, double &y, const double &bound) {
	/* clips (+/-) x to bound */
	if (abs(x) + abs(y) == 0) {
		return;
	}
	double ratio = bound / (abs(x) + abs(y));
	x *= ratio;
	y *= ratio;
}
