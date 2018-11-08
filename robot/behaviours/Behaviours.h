/*

   Copyright 2003 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
   redistribute it and/or modify it under the terms of the GNU General  
   Public License as published by the Free Software Foundation; either  
   version 2 of the License, or (at your option) any later version as  
   modified below.  As the original licfensors, we add the following  
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
 * $Id: Behaviours.h 6487 2005-06-08 11:35:39Z alexn $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * Behaviours
 *
 * An interface with different behaviours separated into different files.
 *
 **/

#ifndef _Behaviours_h_DEFINED
#define _Behaviours_h_DEFINED

#ifdef OFFLINE
#include "../../base/offvision/behavioursOffline.h"
#else // OFFLINE
#include <OPENR/OUnits.h>
#endif // OFFLINE

#include "../share/Common.h"
#include "../share/WirelessSwitchboard.h"
#include "../vision/CommonSense.h"
#include "../vision/VisualCortex.h"
#include "../vision/gps.h"
#include "../vision/Vector.h"
#include "../share/RoboCupGameControlData.h"

#include "../share/IndicatorsDef.h"
#include "../share/PWalkDef.h"
#include "../share/ActionProDef.h"
#include "../share/OffsetDef.h"
#include "../share/VisionDef.h"
#include "../share/SwitchBoard.h"
#include "../share/CommandData.h"

#ifndef OFFLINE
class Vision;    // forward declaration of Vision class
#endif // OFFLINE

#ifdef OFFLINE
class RCRegion;
class ONotifyEvent;
#endif // OFFLINE

static const int MAX_NAME_SIZE = 116;
static const int MAX_VALUE_SIZE = 116;

static const int LOST_BALL_THRESHOLD = 5;

static const double LOW_TILT = -50.0;
static const double HIGH_TILT = 5.0;

static const double PAN_LIMIT = 100;

static const int BALL_IN_FACE_AREA = 1200;
static const int CLOSE_BALL_AREA = 800;
static const int FAR_BALL_AREA = 400;

// a different length compared to what's in robot/share/RobotDef.h
static const double FACE_LENGTH2 = 7.0;
static const double NECK_LENGTH2 = 5.5;

static const int NO_CIRCLE = 0;
static const int CLOCKWISE = 1;
static const int ANTICLOCKWISE = 2;

static const int MAX_NUM_DOG = 4;    // maximum number of dogs per team

// the maximum variance for vobs that we will consider using the vision values
// this will not necessarily be used for all vobs, especially robots
static const double MAX_VAR_USE_VISION = get95CF(100);

static const double MAX_OFFSET_FORWARD = 8.6666; // 9.26;
static const double MAX_OFFSET_FORWARD_SMALL_TURN = 8.6666; //10.4;
static const double MAX_OFFSET_FORWARD_LARGE_TURN = 8.6666; //10;

typedef enum {
	PAW_KICK = 100,
	LATERAL_KICK = 101,
	REVERSE_TURN = 102,
	RUNUP = 103
} PawKickState;

struct CurObject {
	/* heading to object relative to robot in degrees. 
	 *  0 for straight ahead, -90 for left, 90 for right
	 **/
	double h;

	/* distance to object in cm */
	double d;

	/* confidence factor */
	double cf;

	/* variance */
	double var;

	/* x location in field */
	double x;

	/* y location in field */
	double y;

	/* facing */
	double f;
	double imgElev; // image elevation
	double imgHead; // image heading

	const VisualObject *vob;

	CurObject() : vob(NULL)
	{
	}

	// can't use x and y in visual object, because they don't use it
	CurObject & operator =(const VisualObject &obj) {
		var = obj.var;
		// Only copy over information if it is valid.
        // A 9231 hack made by Ted Wong.
		if (var < MAX_VAR_USE_VISION || obj.cf > 0) {
			h = obj.head;
			d = obj.dist2;
			cf = obj.cf;
//			f = obj.f;
			imgHead = obj.imgHead;
			imgElev = obj.imgElev;
			vob = &obj;
		}
		else {
			h = 0;
			d = 0;
			cf = 0;
//			f = 0;
			imgHead = 0;
			imgElev = 0;
			var = VERY_LARGE_INT;
			vob = &obj;
		}
		return *this;
	}


	// calculates the x y value from heading and distance.
	// This way the x y value it is flipped for the blue team
	void calcXY(double rx, double ry, double rh) {
		calPosition(rx, ry, rh, d, h, &x, &y);
	}

	void calcHD(double arx, double ary, double arh) {
		h = NormalizeAngle_180(RAD2DEG(getHeadingBetween(arx, ary, x, y)) - arh);
		d = getDistanceBetween(arx, ary, x, y);
	}

	void reset() {
		x = y = h = d = cf = imgHead = imgElev = 0;
		var = VERY_LARGE_INT;
	}

	Vector operator -(CurObject &obj) {
		Vector ret(vCART, x - obj.x, y - obj.y);
		return ret;
	}
};

static const int SUPPORT   = 0;
static const int STRIKER1  = 1;
static const int STRIKER2  = 2;
static const int NORMAL    = 3;
static const int LOCKMODE  = 4;
static const int DEFENDER  = 5;
static const int BACKOFF   = 6;
static const int NOTHING   = 7;

namespace Behaviours {

	extern void setDogRelaxed();
    extern void goAndCrashTheDog();
    extern void noColor();
    extern void greenColor();
    extern void redColor();
    extern void blueColor();
    extern void yellowColor();
    extern void purpleColor();
    extern void whiteColor();
    extern void lightBlueColor();
    extern void debugCurrent();
    extern void debugBattery();
    extern void debugGPS();
    extern void debugVision();
    extern void debugRole();
    extern void debugSkill();
    extern void debugBall();
    extern void debugFrame();
    extern int  myRole;

    // Ted: My super debug function.
    extern void superDebug();

    extern double cpuUsage;
    extern int batteryCurrent;
    extern long currentFrameID;
    extern long lastFrameID;
    extern void InitBehaviours();
    extern void superActiveLocalise();
    extern void checkAllBehaviourVariables();

    /* Set time-critical mode for the next TIME_CRITICAL_MAX ms or until
     * cleared
     */
    extern void setTimeCritical();
    extern void clearTimeCritical();
    extern bool isTimeCritical();

	/**
	 * operation called after an image is received from the camera and
	 * analysed
	 * Note: this function should NOT be overridden
	 **/ 
	extern void NextAction();

	extern void sw();

	/**
	 * operation called with in NextAction() if:
	 * <UL>
	 *      <LI> robot NOT mFallDown (fallen).
	 *      <LI> robot NOT mGettingUp (getting up from falling).
	 *      <LI> robot NOT mDemobilized.
	 *      <LI> robot NOT MStationaryLocalize (stationary and looking around).
	 * </UL>
	 **/

	// decide appropriate action to take after processing 
	// image feed and world model; called only if mode_ 
	// is mStartMatch and mNormal
	extern void DecideNextAction();
	extern void printCPlusPlusValues(int selection);

	// currently only called after get up
	extern void Reset();

	/**
	 * called when information is received from wireless base station
	 * @param from the robot that sent the info
	 * @param type type of data
	 * @param size size of data
	 * @param data is the pointer to the begining of the data
	 **/
//	extern void wreceive(int from, int type, int size, const void *data);

	/** Called when an external command comes from robocommander, game
     * controller or teammate
	 **/
	extern void processCommand(const CommandData &cmd);
    extern void processControllerData();
	extern void setPenaltyPos();
    extern void processTeamTalk(const char *message, int length);
    
    // used to penalise the dog
    extern void penaliseDog();
    
    // team / dog number things
    extern bool standUp();
    extern void ultimateCalibrate(AtomicAction &nextAA);
	extern uint32  PLAYER_NUMBER;
    extern uint32  TEAM_NUMBER;
    extern RoboCupGameControlData gameData, prevGameData;
    extern int gameDataTime; // frame we last received game data
    extern TeamInfo myTeam;
        
    extern void  processGameData(void* data);
    extern bool  isThisGame(void* data);
    extern bool  dataEqual(byte* data, byte* prev);
    extern byte* copyData(byte* data);
    extern bool  checkHeader(char* header);

	extern CommonSense *sensors;
	extern VisualCortex *vision;
	extern GPS *gps;
#ifndef OFFLINE
	extern Vision *transmission;
#endif // OFFLINE

	extern int *leading_leg;
	extern int *currentWalkType;
	extern int *shareStep_;
	extern int *currentPG;
	extern double *desiredPan;
	extern double *desiredTilt;
	extern double *desiredCrane;

	extern bool *stepComplete;

	extern int kickOff;

	// the system wide camera frame counter
	extern unsigned long frame;

	//for sending WORLD_MODEL
	extern int cmdWDisplay;

	//for sending CPLANE
	extern int cmdBurst;
	extern int burstCounter;

	extern int cmdStationaryLocalise;
	extern int cmdTurnKick;
	extern int cmdActiveGpsLocalise;

	extern int cmdStance;
	static const int NORMAL_STANCE = 0;
	static const int CHEST_PUSH = 1;
	static const int GOALIE_KICK = 2;
	static const int LIGHTNING_KICK = 3;
	static const int SPIN_KICK = 4;
	static const int TURN_KICK = 5;
	static const int BLOCK_STANCE = 6;
	static const int HOLD_STANCE = 7;
	static const int TK90_LEFT = 8;
	static const int TK90_RIGHT = 9;
	static const int TK180_LEFT = 10;
	static const int TK180_RIGHT = 11;            
	static const int NOLEFT_STANCE = 12;
	static const int NOFORWARD_STANCE = 13;    
	static const int ABORT_KICK = 14;

	extern double cmdForward, cmdLeft, cmdTurn, cmdTilt, cmdCrane, cmdPan;
	extern double cmdhf, cmdhb, cmdhdf, cmdhdb, cmdffo, cmdfso, cmdbfo, cmdbso;
	extern int cmdpg;
	extern int cmdHeadtype, cmdWalktype;

	extern int cmdMouth, cmdTailH, cmdTailV, cmdEarL, cmdEarR;
	extern int cmdLed1, cmdLed2, cmdLed3, cmdLed4, cmdLed5, cmdLed6, cmdLed7;
	extern int cmdLed8, cmdLed9, cmdLed10, cmdLed11, cmdLed12, cmdLed13, cmdLed14;
	extern int cmdHeadColor, cmdHeadWhite;
	extern int cmdModeRed, cmdModeGreen, cmdModeBlue;
	extern int cmdWireless;
	extern int cmdBackFrontColor, cmdBackFrontWhite;
	extern int cmdBackMiddleColor, cmdBackMiddleWhite;
	extern int cmdBackRearColor, cmdBackRearWhite;

	extern bool isRedRobot;

	//PWalk related constants
	static const int PG_STANDARD = 40;
	static const int HF_STANDARD = 90;
	static const int HB_STANDARD = 120;
	static const int HDF_STANDARD = 10;
	static const int HDB_STANDARD = 25;
	static const int FFO_STANDARD = 55;
	static const int FSO_STANDARD = 15;
	static const int BFO_STANDARD = -55;
	static const int BSO_STANDARD = 5;

	extern int PLAYER;     // store the current behaviour
	extern int oldPLAYER;  // store the old behaviour while taking control by human
	extern double kickInix, kickIniy, kickInih; // default starting position if kicking off
	extern double recvInix, recvIniy, recvInih; // default starting position if not kicking off
    //Kim: acCmd is a HACK sothat Python can set nextAA.cmd (old code always uses aaParaWalk)
    //feel free to fix it
	extern int acCmd;
	extern int walkType, minorWalkType;
	extern double forward, left, turnCCW;
        extern double forwardMaxStep, leftMaxStep, turnCCWMaxStep;
	extern double hF, hB, hdF, hdB, ffO, fsO, bfO, bsO;
	extern int PG;

	extern int headtype;
	extern double panx, tilty, cranez;

	extern int mouth, tail_h, tail_v, ear_l, ear_r;
	extern int mode_led1,
	mode_led2,
	mode_led3,
	mode_led4,
	mode_led5,
	mode_led6,
	mode_led7;
	extern int mode_led8,
	mode_led9,
	mode_led10,
	mode_led11,
	mode_led12,
	mode_led13,
	mode_led14;
	extern int led1, led2, led3, led4, led5, led6, led7, led8, led9, led10, led11, led12, led13, led14;
	extern int head_color, head_white;
	extern int mode_red, mode_green, mode_blue;
	extern int wireless;
	extern int back_front_color, back_front_white;
	extern int back_middle_color, back_middle_white;
	extern int back_rear_color, back_rear_white;

	extern void SetPAgent(const ONotifyEvent &event);

	//double rx,ry,rh,rcf; // robot's position, now in GPS

	/////////////////
	// convention used here is
	// prefix:
	//  v  = vision
	//  wm = world model
	//  wl = wireless model
	//
	// for goals:
	//  vo = vision (own goal)
	//  vt = vision (target goal)
	//  wmo = world model (own goal)
	//  wmt = world model (target goal)
	////////////////
	extern CurObject vBall, voGoal, vtGoal;    // objects that the robot can see now
	extern CurObject vTeammate[NUM_TEAM_MEMBER], vOpponent[NUM_TEAM_MEMBER];
	extern CurObject wmBall,
	wmtGoal,
	wmoGoal,
	wmTeammate[NUM_TEAM_MEMBER],
	wmOpponent[NUM_TEAM_MEMBER];    // objects info from the 
	// world model

	// wireless ball
	extern CurObject wlBall;

	// Robots with the same colour as your own team from wireless
	// This is different to your teammates telling you where they, themselves are located.
	// This is the teammate robots from other teammates sensors.
	extern CurObject wlTeammate[NUM_TEAM_MEMBER];
	// Robots with different colour to your team from wireless
	extern CurObject wlOpponent[NUM_TEAM_MEMBER];

	extern double hPan, hTilt, hRoll, hCrane;    // in degrees


	// this stuff was in UNSWSkills
	extern void initialize(void);

	extern void setLowGain ();
	extern void setHighGain ();
	extern void dynamic_gain(AtomicAction &);
	/**
	 * simple walk with default parameters
	 * @param forward is the amount of distance to walk forward in centimeters. +ve
	 *        walks forward -ve walks backwards. range -6.0 cm to 6.0 cm by itself,
	 *        but generally depends on Left & turnCCW
	 * @param left is the amount of distance to walk sideways in centimeters. +ve
	 *        walks left -ve walks right. range -4.0 cm to 4.0 cm by itself, but
	 *        generally depends on Forward and turnCCW
	 * @param turnCCW is the amount of turing in walking +ve turns counter clockwise
	 *        -ve turns anti-counter clockwise. range -40 deg to 40 deg by itself,
	 *        but generally deepends on Forward and Left
	 * @param speed is the speed to walk, or the speed of the rotation of the legs.
	 *        use 80 as default. Can be any +ve multiple of 5
	 * @param hF front hip height in mm. use 100 mm as default. Limited by physical constraints.
	 * @param hB back hip height in mm. use 120 mm as default. Limited by physical constraints.
	 * @param hdF front feet height in mm. use 15 mm as default. Limited by physical constraints. Must be >0
	 * @param hdB back feet height in mm. use 18 mm as default. Limited by physical constraints. Must be >0
	 * @param ffO front feet center in the walking direction. use 25 mm as default. Limited by physical constraints.
	 * @param fsO front feet center perpendicular to walk. use 20 mm as default. Limited by physical constraints.
	 * @param bfO back feet center in the walking direction. use -25 mm as default. Limited by physical constraints.
	 * @param bsO back feet center perpendicular to walk. use 20 mm as default. Limited by physical constraints.
	 **/
	extern void makeParaWalk(int walkType,
			int minorWalkType,
                        double forward,
			double left,
			double turnCCW,
                        double forwardMaxStep, 
                        double leftMaxStep, 
                        double turnCCWMaxStep);
	/**
	 * simple walk with default parameters
	 * @param forward is the amount of distance to walk forward in centimeters. +ve
	 *        walks forward -ve walks backwards. range -6.0 cm to 6.0 cm by itself,
	 *        but generally depends on Left & turnCCW
	 * @param left is the amount of distance to walk sideways in centimeters. +ve
	 *        walks left -ve walks right. range -4.0 cm to 4.0 cm by itself, but
	 *        generally depends on Forward and turnCCW
	 * @param turnCCW is the amount of turing in walking +ve turns counter clockwise
	 *        -ve turns anti-counter clockwise. range -40 deg to 40 deg by itself,
	 *        but generally deepends on Forward and Left
	 * @param speed is the speed to walk, or the speed of the rotation of the legs.
	 *        use 80 as default. Can be any +ve multiple of 5
	 * @param hF front hip height in mm. use 100 mm as default. Limited by physical constraints.
	 * @param hB back hip height in mm. use 120 mm as default. Limited by physical constraints.
	 * @param hdF front feet height in mm. use 15 mm as default. Limited by physical constraints. Must be >0
	 * @param hdB back feet height in mm. use 18 mm as default. Limited by physical constraints. Must be >0
	 * @param ffO front feet center in the walking direction. use 25 mm as default. Limited by physical constraints.
	 * @param fsO front feet center perpendicular to walk. use 20 mm as default. Limited by physical constraints.
	 * @param bfO back feet center in the walking direction. use -25 mm as default. Limited by physical constraints.
	 * @param bsO back feet center perpendicular to walk. use 20 mm as default. Limited by physical constraints.
	 * @param headType is the type of head movement. NULL_H = 0 - no action, REL_H = 1 - relative, ABS_H = 2 - absolute
	 * @param tilt tilting of the head in degrees. +ve is looking up, -ve is looking down
	 * @param pan turning of the head in degrees. 0 is looking to the right, 180 to the left, 90 straight ahead, range 0 to 180
	 **/
	extern void makeParaWalk(int walkType,
                        int minorWalkType,
			double forward,
			double left,
			double turnCCW,
                        double forwardMaxStep, 
                        double leftMaxStep, 
                        double turnCCWMaxStep,
			int headType,
			double panx,
			double tilty,
			double cranez);
	/**
	 * move the head
	 * @param headType is the type of head movement. NULL_H = 0 - no action, REL_H = 1 - relative, ABS_H = 2 - absolute
	 * @param tilt tilting of the head in degrees. +ve is looking up, -ve is looking down
	 * @param pan turning of the head in degrees. 0 is looking to the right, 180 to the left, 90 straight ahead, range 0 to 180
	 **/
	extern void makeHead(int headType, double panx, double tilty, double cranez);

	extern void defaultInit(void);
	extern double microRad2Deg(double);

	// Indicators
	/**
	 * make various indicators. for all the values 1 = off, 2 == on
	 * @image html headDes.gif
	 **/
	extern void makeIndicators(int inds[]);
	extern bool playerIndicatorNotSet;

	// stores the last command
	extern CommandData command;

	// quick and dirty hack; putting it here that is
	extern int actionTimer;

	//TEMPORARY just for testing! ask Nico to remove this.         
	extern char dataTesting[2 * 196 * 144];

    // This is done by the GameController now
    //extern void showTeamIndicator();
    extern void saveYUVPhoto();
	extern double destPan;
	extern void StationaryLocalize();

	// use the LEDs to show the battery charge level
	extern void batteryLEDS();

	extern bool debugMsg;

	extern int lostBall;                 // num frames ball unseen
	extern double lastTrackHead;         // last heading that you were tracking
	// (as set by saBallTracking or other source
	extern int lastSawBeacon;            // num frames last seen beacon
	extern bool isWirelessDown;          // true iff wireless is down
	extern double useZoidWalk;           // whether to use zoidalwalk or not.
	extern double dkd;                   // optimal kick direction
	extern double ballUnderChinTilt;     // tilt when ball under chin

	extern int attackMode;               // current far ball action
	extern int prevAttackMode;           // previous far ball action
	extern int attackLock;

	extern int grabTime;                 // num frames ball grabbed  
	extern int grabInterval;             // num frames disallow ball grab

	extern int seeBall;                  // num frames ball seen

	// find ball variables
	extern int lookdir;                  // direction to look first when find ball
	extern int turnDir;                  // direction to turn in finding ball
	extern double lastHPan;
        extern double lastHTilt;
        extern double lastHCrane;
	extern int lookAroundCounter;
	extern double lastBallTilt;          // the last tilt when tracking ball

	// determines if robot should stand slightly taller
	// mainly for looking for the ball
	extern bool standTall;

	// role determination variables
	extern int role;
	extern int closestRobotToBall;
	extern bool is3rdClosest;
	extern int backoffTeammateIndex; //this is not used .. was used by malak guys

	enum role {
		AGGRESSOR = 0,
		ASSISTANT = 1
	};

	extern int lockMode;
	extern int prevLockMode;

	// different atomic actions to execute when ball under chin
	enum LockMode {
		ResetMode,
		NoMode,
		GrabBall,
		GotBall,
		ChestPush,
		TurnKick,
		SpinKick,
		SafeSpinKick,
		VisualSpinKick,
		VisOppAvoidanceKick,
		PenaltyShooterAimKick, // 10
		ActiveLocalise,
		SpinChestPush,
		BeckhamChestPass,
		HoldBall,
		GoalieKick,
		UpennRight,
		UpennLeft,
		LightningKick,
		VariableTurnKick,
		ProperVariableTurnKick,
		SpinDribble,
		Dribble,               // 20
		GoalieLookAround,
		LookAtBeacon,
		GoalieHoldBall,
		MalakTurnKick90,
		MalakTurnKick180,
		MalakClearKick,
		MalakClearPush,
		MalakSpinKick,
		FireSpinKick,
		BeckhamDefend,         // 30
        UNSW2004Defend,
		LocateBallKick,
		CelebrationLock,
        LookAround                 // Added by Ted Wong.
			/*TeamCForwardHoldBall,
			  TeamCForwardGotBall,
			  TeamCForwardSpinChestPush,
			  TeamCForwardSpinChestPass*/
	};

	// different ways of approaching ball
	enum AttackMode {
		GoToBall,
		ChargeBall,
		PawKick,
		GetBehindBall
	};

	extern void doFindBall();
	extern bool hasFindBallStarted();
	extern void resetFindBallCounter();

	extern void doAttackBall();
	extern void decrementAttackLock();

	extern void doWirelessPositioning();

	extern void saGoToTargetFacingHeading(const double &targetx, const double &targety,
			                              const double &targeth, double maxSpeed = 5.5);

	// ball tracks and looks at beacon occasionally
	extern void saActiveBallTracking();
	extern void saBallTracking(bool trackWM = false);
	extern void saLookAtNearestBeacon();
	extern void saLookAtHDZ(double heading, double distance, double z);

	extern void saGoToBall(double slowDownDist = 20);
	extern void saChargeBall();
	extern void saCircleAround(double step_angle, double radius);

	extern void saGetBehindBall(double sd,
			double aa,
			int turndir,
			double ballX = INVALID_FLAG,
			double ballY = INVALID_FLAG,
			double ballH = INVALID_FLAG);

	extern void saFarGetBehindBall(bool chooseTurnDir, double aa = INVALID_FLAG);


	extern void saMalakGetBehindBall(double sd, double aa, int turndir);
	extern void saMalakFarGetBehindBall(bool chooseTurnDir);

	extern void resetSaRunBehindBall ();
	extern void saRunBehindBall(double radius,
			double aa,
			int turndir,
			double taOff,
			double raOff);    
	extern double saHeadingBetween(double fromX,
			double fromY,
			double toX,
			double toY);
	extern void saFindAngleDist(double x, double y, double *head, double *dist);
	extern void saGetPointRelative(double baseX,
			double baseY,
			double offsetHead,
			double offsetDist,
			double *resX,
			double *resY);

	extern bool utilNeedVisualBackoff();
	extern bool utilNeedGpsBackoff();
	extern bool utilNeedGpsBackoffWithReturn(double *headingToTM,
			double *retx,
			double *rety);
	extern bool utilNeedVisualBackoffWithReturn(double *headingToTM,
			double *retx,
			double *rety);
	extern void utilDetermineRole();
	extern bool utilIsBallUnderChin();

	extern void setTeammateSeenTime();
	extern bool shouldBackOffTeammateWithBall();
	extern void decrementBackOff();

	extern void utilLookAroundParams(int speed);
	extern void setPanDirection(int dir);
	extern void setPrevPan(double pan);

	extern void utilDetermineDKD();
	extern bool utilIsFrontObstructed();
	extern bool utilOppWithinDist(double dist);
	extern double utilNormaliseAngle(double a);

    /* Commented out because I believe not used
        extern void utilDetermineDKDwithRobotAvoidance();
	    extern void utilDetermineDKDaimAtCorner();
    */

	static const int closeEdgeDist = 35;
	static const int ballCloseEdgeDist = 25;

	extern bool utilIsInPenaltyArea();
	extern bool utilIsInOwnHalf();
	extern bool utilIsOnLREdge(double dist = closeEdgeDist);
	extern bool utilIsOnLEdge(double dist = closeEdgeDist);
	extern bool utilIsOnREdge(double dist = closeEdgeDist);
	extern bool utilIsOnTEdge(double dist = closeEdgeDist);
	extern bool utilIsOnBEdge(double dist = closeEdgeDist);

	extern bool utilBallOnLREdge(double dist = ballCloseEdgeDist);
	extern bool utilBallOnLEdge(double dist = ballCloseEdgeDist);
	extern bool utilBallOnREdge(double dist = ballCloseEdgeDist);
	extern bool utilBallOnTEdge(double dist = ballCloseEdgeDist);
	extern bool utilBallOnBEdge(double dist = ballCloseEdgeDist);

	extern bool utilBallInTRCorner();
	extern bool utilBallInTLCorner();

	extern bool utilBallInTargetGoalBox(double ballx, double bally);

	extern bool utilIsInFrontTargetGoal();
	extern bool utilIsUp();
	extern bool utilIsDown();
	//    extern bool utilOpponentInFace();
	extern bool utilTeammateInFace();
	extern bool utilIs3rdClosest();
	extern bool utilBallInLeftHalf();
	extern bool utilBallInOwnHalf();
	extern double Cap(double value, double limit);

	//    CurObject *utilGetClosestOpponentTo(double x, double y, double *dist);

	// atomic actions; executed when ball under chin
	extern void aaGrabBall();
	extern void aaHoldBall();
	extern void aaGotBall();
	extern void aaSpinKick();
	extern void aaSafeSpinKick();

	extern void aaVisualSpinKick();
	extern void shootAtVisualGoal();
	extern void lookAroundForGoal();

	extern void aaVisOppAvoidanceKick();
	extern void performVisOppAvoidanceKick();
	extern void resetVisOppAvoidanceKickState();
	extern void getHeadingToBestGap(double *min, double *max);
	extern void getHeadingToBestGap2005(double *min, double *max);
	extern bool sortRobotsFromLeftToRight(CurObject obj1, CurObject obj2);
	extern bool oppBetweenTargetGoalPosts();
	extern void getLeftGapSizeHeading(double *size, double *head);
	extern void getRightGapSizeHeading(double *size, double *head);
	extern bool isGapLargeEnough(double gapSize);
	extern void shootAtMiddleOfTargetGoal();
	extern void shootAtLeftGapOfTargetGoal(double offsetFromLeftPostHeading);
	extern void shootAtRightGapOfTargetGoal(double offsetFromRightPostHeading);
	extern bool isTeammateInLeftQuadrant();
	extern bool isTeammateInRightQuadrant();
	extern bool isFacingLeftHalf();
	extern void shootBetweenOppsLeft();
	extern void shootBetweenOppsRight();

	extern void aaChestPush();
	extern void aaTurnKick();
	extern void aaActiveLocalise();
	extern bool canActiveLocalise();
	extern void decrementDoNotLocalise();
	extern void resetActiveLocalise();
	extern void aaSpinChestPush();
	extern void aaGoalieKick();
	extern void aaLightningKick();
	extern void aaVariableTurnKick();
	extern void setVSoftVariableTurnKickType();
	extern void setSmallVariableTurnKickType();
	extern void setMedVariableTurnKickType();
	extern void setFullVariableTurnKickType();
	extern void resetVariableTurnKickCounter();

	///////////////////////////////////////////////
	// Upenn L/R kick                            //
	///////////////////////////////////////////////

	extern void aaUPkickRight();
	extern void aaUPkickLeft();

	///////////////////////////////////////////////
	// Off Edge Kick                             //
	///////////////////////////////////////////////    

	extern void offEdgeKick(int paw, double offset = 15.0);
	//static int offEdgeCounter = 0;
	//static int offEdgePaw = 2;
	static const int OFFEDGE_KICK_LEFT = 0;
	static const int OFFEDGE_KICK_RIGHT = 1;
	static const int OFFEDGE_KICK_AUTO = 2;        

	///////////////////////////////////////////////
	// Cheer                                     //
	///////////////////////////////////////////////    

	extern void aaCheerKick();

	///////////////////////////////////////////////
	// Fire Paw Kick                             //
	///////////////////////////////////////////////    

	extern void firePawKick(int paw, double offset = 7.5);
	static const int FIRE_PAWKICK_LEFT = 0;
	static const int FIRE_PAWKICK_RIGHT = 1;
	static const int FIRE_PAWKICK_AUTO = 2;    

	///////////////////////////////////////////////
	// Proper Variable Turnkick                  //
	///////////////////////////////////////////////
	extern void setProperVariableTurnKick(int frames, int headlift);
	extern void setProperVariableTurnKick(double direction);
	extern void aaProperVariableTurnKick();

	extern void aaDribble();
	extern void resetDribbleTimer();
	extern void aaSpinDribble();

	extern int turnAmount; // for calibrate later


	extern void activeGpsLocalise(bool useSlowPan);
	// limit of angle we can look at
	extern void setNewBeacons(double maxDist,
			double panLimit = PAN_LIMIT,
			bool sortByPan = true); //only set beacons if they lie within panLimit
	extern void smartActiveGpsLocalise();
	extern void smartSetBeacon(double maxDist = VERY_LARGE_INT,
			double panLimit = PAN_LIMIT);
	extern void scanLocalise();

	// active localise trigger helper functions
	void initActiveLocaliseBeaconData();
	void updateActiveLocaliseBeaconData();
	bool hasSeenTwoBeaconsLately();
	bool canSeeABeacon();
	int numFramesBeaconSeenLately(int beaconIndex);
	void printBeaconsSeenLately();

	extern void initRemoteControl();
	extern void initPlayer();
	extern void chooseStrategy();
	extern void chooseShootStrategy();


	///////////////////////////////////////////////
	// Parameters settings for the atomic action //
	///////////////////////////////////////////////
	extern void setSpinKickParams();
	extern void setTurnKickParams(double turnccw);

	extern double requiredAccuracy(double reqGoalWidth);
	extern double requiredAccuracy(double tmx, double tmy, double reqGoalWidth);
	extern void ballSearch();

	/////////////////////////////////////////////////////////////////////////

	// load the default player from a file on startup
	extern void loadDefaultPlayer(const char *filename);

	// clear the emon log so that the old one remains from the last crash won't be there
	extern void clearEmonLog();

	// set the default indicators
	extern void initIndicators();


	enum TeamAStates {
		FindBallState,
		TrackBallState,
		BlockState,
		HeadDownState,
		ApproachState,
		KickState,
		ReturnGoalState,
		StandState,
		FindGoalState,
		HoldState,
		CarryState,
		BackOffState
	};

	extern void printCurrentPlayer();
	extern void headBallTracking();
	extern void findBallHeadParams();
	extern void setGotoBallParams();
	extern void setStandParams();
	extern void setWalkParams();
	extern void setCanterParams();
	extern void setOffsetParams();
	extern void setOmniParams();
	extern void setBlockParams();
	extern void setHoldParams();
	extern void setHoldSideWalkParams();
	extern void setCarryParams();
	extern void setDefaultParams();

	extern int panDirection;
	extern int tiltDirection;

	////////////////////
	// Team A Forward //
	////////////////////

	extern int findGoalTimer;
	extern int carryTimer;
	extern double goalPan;
	extern int holdTimer;
	extern int lostBallTimer;
	extern int backoffTimer;

	///////////////////
	// Team A Goalie //
	///////////////////

	extern int kickTimer;
	extern int blockTimer;

	extern int state;
	extern double prevDistToBall;


	///////////////////
	// Team D        //
	///////////////////

	extern double findPositionx;
	extern double findPositiony;


	////////////////////
	// Team E Forward //
	////////////////////

	extern int teammateSeenTime; // number of frames teammate seen


	/////////////////
	// LOCATE BALL //
	/////////////////

	extern void setLocateBallParams();
	extern void setDirection(int dir);
	extern void locateBall();
	extern void locateBall(double findPositionx, double findPositiony);
	extern void spinFindBall();
	extern void spinFindBall(int dir);
	extern void improvedSpinFindBall(int dir);
	extern void headFindBall(double lowtilt = -50.0);
	extern void setLocateBallKickParams();
	extern void aaLocateBallKick();


	////////////////////
	// Test Turn Kick //
	////////////////////
	extern void doTestTurnKick();
	extern void initTestTurnKick();
	extern void ChooseTurnKick();

	extern bool allowedToGoForBall;

	//////////////////
	// Base Station //
	//////////////////

	extern void processWControl();

	extern void resetMovement();

	/* Simple Dribble
	 * 
	 * All should be called between 40-25 cm from the ball.  They must be locked in 
	 * by 25 if you expect it to be > 85% successful.  Though they are robust to
	 * strange situations, it is better that the dog is already aligned with the
	 * ball.
	 * Call RIGHT/LEFT_POWER_PAW for more power and less robustness
	 * Call RIGHT/LEFT_PAW for greatest reliability
	 * Call RIGHT/LEFT_LATERAL_KICK to set up for a lateral kick in the same way that we do paw kick
	 */
	extern PawKickState saDribble();

	/**
	 * Dribble in a particular direction.  This is an untested hack,
	 * so test before use and call andres if problems persist
	 * @param heading desired angular direction in degrees [-180, 180]
	 */
	//PawKickState saDribbleFacingHeading (double heading);

	/**
	 * Paw kick
	 * @offset - specify the paw to kick with
	 */
	extern PawKickState saPawKick(double paw);

	/**
	 * Paw kick
	 * @offset - specify the paw to kick with
	 */
	//PawKickState saPawKickFacingHeading (double paw, double heading);

	//***********Private Paw Kick Skills*************//
	/**
	 * @param offset is either LEFT_PAW or RIGHT_PAW
	 */
	extern void pkGoToBall(double offset = 0);

	/**
	 * goToBall -> usually offset is either LEFT_PAW or RIGHT_PAW.
	 * @param heading desired angular direction in degrees [-180, 180]
	 */
	//void pkGoToBallFacingHeading (double heading, double offset);

	//***********Paw Kick Skills*************//
	/* The shared code which decides which PAW_KICK to execute, if any
	 * @return returns one of:
	 * LATERAL_KICK    if a lateral kick will be executed
	 * PAW_KICK if a paw kick will be executed
	 * RUNUP otherwise
	 */
	extern PawKickState pkChoosePawKick();

	/*
	 * @return true if the ball can be paw kicked (perp distance < 8 for zoidal, 6 for normal)
	 */
	extern bool pkCanPawKick();

	/*
	 * Macro to use Zoidal walk
	 */
	extern void pkMakeZoidalWalk(double F, double L, double t);
	extern bool bWRITE_TO_CONSOLE ;
}


//#define RIGHT_PAW_OFFSET -5.9 // approx 6.6
//#define LEFT_PAW_OFFSET       5.9     //cm
#define RIGHT_PAW -4.1
#define LEFT_PAW 4.1
#define POWER_RIGHT_PAW -4.9
#define POWER_LEFT_PAW 4.9
#define RIGHT_LATERAL_KICK -14
#define LEFT_LATERAL_KICK 14

namespace Behaviours {

	// for compatibility of ERS-200 only, do not use for ERS7.
	static inline void leds(int l1, int l2, int l3, int l4, int l5, int l6) {
		if (l1 != IND_UNCHANGED) {
			led1 = (l1 == IND_LED_ON)
				? IND_LED3_INTENSITY_MAX
				: IND_LED3_INTENSITY_MIN;
		}
		if (l2 != IND_UNCHANGED) {
			led2 = (l2 == IND_LED_ON)
				? IND_LED3_INTENSITY_MAX
				: IND_LED3_INTENSITY_MIN;
		}
		if (l3 != IND_UNCHANGED) {
			led3 = (l3 == IND_LED_ON)
				? IND_LED3_INTENSITY_MAX
				: IND_LED3_INTENSITY_MIN;
		}
		if (l4 != IND_UNCHANGED) {
			led4 = (l4 == IND_LED_ON)
				? IND_LED3_INTENSITY_MAX
				: IND_LED3_INTENSITY_MIN;
		}
		if (l5 != IND_UNCHANGED) {
			led5 = (l5 == IND_LED_ON)
				? IND_LED3_INTENSITY_MAX
				: IND_LED3_INTENSITY_MIN;
		}
		if (l6 != IND_UNCHANGED) {
			led6 = (l6 == IND_LED_ON)
				? IND_LED3_INTENSITY_MAX
				: IND_LED3_INTENSITY_MIN;
		}
	}

	/* A simple leds setting function for ERS-7. */
	static inline void leds(int l1,  int l2,  int l3,  int l4, int l5, int l6, int l7, int l8, int l9, int l10,
			                int l11, int l12, int l13, int l14) {
		led1 = l1;
		led2 = l2;
		led3 = l3;
		led4 = l4;
		led5 = l5;
		led6 = l6;
		led7 = l7;
		led8 = l8;
		led9 = l9;
		led10 = l10;
		led11 = l11;
		led12 = l12;
		led13 = l13;
		led14 = l14;
	}

    /* Set eyes pattern. l1, l5 ranges from 0-3, l2-l4 ranges from 0-2. Value
    ** -1 is unchanged, 0-3 is the amount of lights we want to see in each
    ** column. */ 
	static inline void showEyes(int l1, int l2, int l3, int l4, int l5) {

        switch (l1) {
            case IND_UNCHANGED:
                break;
            case IND_LED_OFF:
                led1 = IND_LED3_INTENSITY_MIN;
                led3 = IND_LED3_INTENSITY_MIN;
                led7 = IND_LED3_INTENSITY_MIN;
            case 1:
                led1 = IND_LED3_INTENSITY_MIN;
                led3 = IND_LED3_INTENSITY_MIN;
                led7 = IND_LED3_INTENSITY_MAX;
                break;
            case 2:
                led1 = IND_LED3_INTENSITY_MIN;
                led3 = IND_LED3_INTENSITY_MAX;
                led7 = IND_LED3_INTENSITY_MAX;
                break;
            case 3:
                led1 = IND_LED3_INTENSITY_MAX;
                led3 = IND_LED3_INTENSITY_MAX;
                led7 = IND_LED3_INTENSITY_MAX;
                break;
        }

        switch (l2) {
            case IND_UNCHANGED:
                break;
            case IND_LED_OFF:
                led5 = IND_LED3_INTENSITY_MIN;
                led9 = IND_LED3_INTENSITY_MIN;
                break;
            case 1:
                led5 = IND_LED3_INTENSITY_MIN;
                led9 = IND_LED3_INTENSITY_MAX;
                break;
            case 2:
                led5 = IND_LED3_INTENSITY_MAX;
                led9 = IND_LED3_INTENSITY_MAX;
                break;
        }

        switch (l3) {
            case IND_UNCHANGED:
                break;
            case IND_LED_OFF:
                led11 = IND_LED3_INTENSITY_MIN;
                led12 = IND_LED3_INTENSITY_MIN;
                break;
            case 1:
                led11 = IND_LED3_INTENSITY_MAX;
                led12 = IND_LED3_INTENSITY_MIN;
                break;
            case 2:
                led11 = IND_LED3_INTENSITY_MAX;
                led12 = IND_LED3_INTENSITY_MAX;
                break;
        }

        switch (l4) {
            case IND_UNCHANGED:
                break;
            case IND_LED_OFF:
                led6  = IND_LED3_INTENSITY_MIN;
                led10 = IND_LED3_INTENSITY_MIN;
                break;
            case 1:
                led6  = IND_LED3_INTENSITY_MIN;
                led10 = IND_LED3_INTENSITY_MAX;
                break;
            case 2:
                led6  = IND_LED3_INTENSITY_MAX;
                led10 = IND_LED3_INTENSITY_MAX;
                break;
        }

        switch (l5) {
            case IND_UNCHANGED:
                break;
            case IND_LED_OFF:
                led2 = IND_LED3_INTENSITY_MIN;
                led4 = IND_LED3_INTENSITY_MIN;
                led8 = IND_LED3_INTENSITY_MIN;
            case 1:
                led2 = IND_LED3_INTENSITY_MIN;
                led4 = IND_LED3_INTENSITY_MIN;
                led8 = IND_LED3_INTENSITY_MAX;
                break;
            case 2:
                led2 = IND_LED3_INTENSITY_MIN;
                led4 = IND_LED3_INTENSITY_MAX;
                led8 = IND_LED3_INTENSITY_MAX;
                break;
            case 3:
                led2 = IND_LED3_INTENSITY_MAX;
                led4 = IND_LED3_INTENSITY_MAX;
                led8 = IND_LED3_INTENSITY_MAX;
                break;
        }

	}

	//the angle that the body makes with the ground (in Behaviours 's perspective)
	double getBodyTilt();
}

#endif
