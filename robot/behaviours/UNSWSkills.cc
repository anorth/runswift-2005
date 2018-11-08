/*

   Copyright 2004 The University of New South Wales(UNSW) and National  
   ICT Australia(NICTA).

   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
   redistribute it and/or modify it under the terms of the GNU General  
   Public License as published by the Free Software Foundation; either  
   version 2 of the License, or(at your option) any later version as  
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
 * $Id: UNSWSkills.cc 7501 2005-07-08 14:15:21Z alexn $
 *
 * Copyright(c) 2004 UNSW
 * All Rights Reserved.
 *
 **/

#include "Behaviours.h"
#include "pythonBehaviours.h"
//#include "UNSW2004.h"
#include <sys/time.h>

#ifndef OFFLINE
#include <MCOOP.h>
#endif

#include "../vision/CommonSense.h"
#ifndef OFFLINE
#include "../vision/Vision.h"
#endif // OFFLINE
#include "../share/WirelessTypeDef.h"
#include <sstream>

//TODO : figure out how to send burst command properly
static const int BURST_NUMBER            = 5;
static const int MOBILIZE_COUNT_HEAD     = 4;
static const int DISPLAY_WORLD_MODEL_ALL = 9;
static const int NUM_FRAMES_TO_PAN       = 148;

// counter for photo taker filename
static int yuvSaveNumber   = -1;  
static int logYUVFrequency = -1;
static int logYUVTimer     = -1; // -1 means no yuvLogging, 0 means logging

// parameters that is sent to actuator control
static AtomicAction nextAA;

// flags whether the robot is trying to stand up after falling down
static bool gettingUp = false;
static bool isHighGain = false;
 

/* A higher level decision-making to determine which stage of a game the dog
** is in. Just started? Penalty? Getting up? etc. Next level depends on
** player. */ 
void Behaviours::NextAction() {

    /* int inds[NUM_OF_INDICATORS]; */

	frame++;
    
    /* photo taker hack */
    if (PLAYER == 1 && 
            sensors->press_sensor[PRESS_HEAD].count == MOBILIZE_COUNT_HEAD) {        
        saveYUVPhoto();
        return;
    }
    
    /* remote player hack - this shouldn't even do anything now */
    /*
    if (PLAYER == 0) {
        makeIndicators(inds);
        transmission->sendIndicators(inds, sizeof(inds));
    }
    */
    
    /* break off if the dog fell over or is standing up */
    if (standUp()) return;
    
	initialize();
   

	// Keep track of how long we have lost the ball for.
	if (gps->canSee(vobBall)) {        
		lostBall = 0;        
	} else {
		lostBall++;            
	}

	switch(gameData.state) {
        
        /* do nothing in initial state */
        case STATE_INITIAL:        
            makeParaWalk(SkellipticalWalkWT, 3, 0, 0, 0, 0, 0, 0, ABS_H, 0, 0, 0);            
            break;                       
    
        /* playing, set, penalised */
        default:
            DecideNextAction();
                
	}    

    // reset head for the initial state
    if (gameData.state != STATE_INITIAL) {
        makeHead(headtype, panx, tilty, cranez);
    } else {
        makeHead(ABS_PT, 0, 0, 0);
    }
    
    // send information for other dogs
#ifndef OFFLINE
        
        if (myTeam.players[PLAYER_NUMBER-1].penalty == PENALTY_NONE) {
            transmission->shareEnvironment();
        }
    
        // send any debug information
        // send burst data
        if ((uint32)cmdBurst == PLAYER_NUMBER && Behaviours::burstCounter > 0) {
            CorrectedImage::fillCPlane();
            CorrectedImage::sendCPlane();
            burstCounter--;
        }
    
        // send world model stuff
        if ((uint32)cmdWDisplay == PLAYER_NUMBER || cmdWDisplay == DISPLAY_WORLD_MODEL_ALL) {
            transmission->displayEnvironment();
        }
        if (logYUVTimer == logYUVFrequency) {		
            transmission->sendYUVPlane();
            logYUVTimer = 0;            
        } else {
            if (logYUVTimer >= 0) logYUVTimer++;            
        }            
    
        ultimateCalibrate(nextAA);
        lastHPan = hPan;
        lastHTilt = hTilt;
        lastHCrane = hCrane;
        
        transmission->sendAA(nextAA);
#endif // OFFLINE

}


/* to help eliminate the need for the variable mode_, the detection
   of falling over and getting up is no longer a mode */
bool Behaviours::standUp() {   
    
    // fell over, make the dog stand up
    if (sensors->hasFallen != NotFallenFS && gettingUp == false && *stepComplete) {
        if (sensors->hasFallen == FallenFrontFS) {
            makeParaWalk(GetupFrontWT,0,0,0,0,0,0,0,0,0,0,0);
        } else if (sensors->hasFallen == FallenRearFS) {
            makeParaWalk(GetupRearWT,0,0,0,0,0,0,0,0,0,0,0);
        } else if (sensors->hasFallen == FallenSideFS) {
            makeParaWalk(GetupSideWT,0,0,0,0,0,0,0,0,0,0,0);
        }
#ifndef OFFLINE
        transmission->sendAA(nextAA);
#endif
        gettingUp = true;
        return true;
        
    // still getting up
    } else if (sensors->hasFallen != NotFallenFS && gettingUp) {
        return true;    
        
    // finished standing up, reste variables
    } else if (sensors->hasFallen == NotFallenFS && gettingUp) {
        sensors->resetFallenState();
        gettingUp = false;
        return true;
    }
    return false;

}


/* Kim:06/03/04 This is the last calibrate, since the walk is changed, every 
   thing need to be re-calibrate. This is kind of a hack to push my calibration.
   Remove it when we have stable walks
   Sunny:9/2/05 Added debug so we know when it does its work, we don't know 
   whether this still has any use */
void Behaviours::ultimateCalibrate(AtomicAction &nextAA) {
    if (nextAA.walkType == CanterWalkWT || 
        nextAA.walkType == ZoidalWalkWT || 
        nextAA.walkType == OffsetWalkWT ) {
        nextAA.walkType = NormalWalkWT;
        cout << "ultimateCalibrate reverted to NormalWalkWT" << endl;
    }
}


// Localise standing still. This is done in the Ready state before ReadyPlayer
// in Python starts (?)
void Behaviours::StationaryLocalize() {
	if (actionTimer > 0) {
		if (ABS(hPan - destPan) <= 10) {
			destPan =(destPan < 0) ? 85 : -85;
		}
		headtype = REL_H;
		panx =(destPan > 0) ? 5 : -5;
		tilty =(10 - hTilt) / 5.0;
		cranez = -hCrane / 4.0;
		makeHead(headtype, panx, tilty, 0);
		actionTimer--;
	}
}

/* Note:

** 1. whichJoint is according to JointEnum in ../share/PWalkDef.h
** 2. How come a makeParaWalk is different to makeRelaxJoint,
**    makeEnableJoint, setDogRelaxed?
**
**    Actually, they are the same. But at the end of the behav loop,
**    makeParaWalk is called, which sets the nextAA.cmd to walk,
**    so one way to have our own cmd working is to create an ActomicAction
**    and send it all ourselves, in the exact way that the walk does 
*/

void Behaviours::setLowGain () {
    isHighGain = false;
    AtomicAction aa ;
	aa.cmd = aaSetLowGain;
    #ifndef OFFLINE
	    transmission->sendAA(aa);
    #endif //OFFLINE
}


void Behaviours::setHighGain () {
    isHighGain = true;
    AtomicAction aa ;
	aa.cmd = aaSetHighGain;
    #ifndef OFFLINE
	    transmission->sendAA(aa);
    #endif //OFFLINE
}


void Behaviours::makeParaWalk(int walkType, int minorWalkType, 
                double forward, double left, double turnCCW, 
                double forwardMaxStep, double leftMaxStep, double turnCCWMaxStep) {
	nextAA.cmd           = acCmd;
	nextAA.walkType      = walkType;
        nextAA.minorWalkType = minorWalkType;
	
        nextAA.forwardSpeed = forward;
	nextAA.leftSpeed    = left;
	nextAA.turnCCWSpeed = turnCCW;
        
        nextAA.forwardMaxStep = forwardMaxStep; 
        nextAA.leftMaxStep    = leftMaxStep; 
        nextAA.turnCCWMaxStep = turnCCWMaxStep;
        
        // what's this?
        nextAA.headType = NULL_H;
        
	defaultInit();
}

void Behaviours::makeParaWalk(int walkType, int minorWalkType,
                double forward, double left, double turnCCW, 
                double forwardMaxStep, double leftMaxStep, double turnCCWMaxStep,
                int headType, double panx, double tilty, double cranez) {

        makeParaWalk(walkType,minorWalkType,
                     forward,left,turnCCW,
                     forwardMaxStep,leftMaxStep,turnCCWMaxStep);
                     
	nextAA.headType = headType;
	nextAA.tilty    = tilty;
	nextAA.panx     = panx;
	nextAA.cranez   = cranez;
	defaultInit();
}

void Behaviours::makeHead(int headType, double panx, double tilty, double cranez) {
	nextAA.cmd      = acCmd;
	nextAA.headType = headType;
	nextAA.panx     = panx;
	nextAA.tilty    = tilty;
	nextAA.cranez   = cranez;
	defaultInit();
}

// common initialization for all commands
void Behaviours::defaultInit(void) {	
	nextAA.step = 0;
	nextAA.finished = false;
}

double Behaviours::microRad2Deg(double rad) {
	return rad * 180.0 /(1000000.0 * pi);
}


// cleaned up code by using operator= and several
// object operations to make it simple
void Behaviours::initialize(void) {
    
	// ball from current image
	if (vision->vob[vobBall].cf == 0) {
		vBall.vob = &(vision->vob[vobBall]);
		vBall.reset();
	} else {
		vBall = vision->vob[vobBall];
		const WMObj& r = gps->self();
		vBall.calcXY(r.pos.x, r.pos.y, r.h);
	}

	// tGoal from current image
	if (vision->vob[gps->targetGoal].cf == 0) {
		vtGoal.reset();
	} else {
		// tGoal from current image
		vtGoal = vision->vob[gps->targetGoal];
		vtGoal.x = FIELD_WIDTH / 2;
		vtGoal.y = FIELD_LENGTH + GOAL_LENGTH / 2;
	}

	// oGoal from current image
	if (vision->vob[gps->ownGoal].cf == 0) {
		voGoal.reset();
	} else {
		voGoal = vision->vob[gps->ownGoal];
		voGoal.x = FIELD_WIDTH / 2;
		voGoal.y = -GOAL_LENGTH / 2;
	}

	// convert head sensors to degrees
	hTilt  = microRad2Deg(sensors->sensorVal[ssHEAD_TILT]);
	hPan   = microRad2Deg(sensors->sensorVal[ssHEAD_PAN]);
	hCrane = microRad2Deg(sensors->sensorVal[ssHEAD_CRANE]);

	// other robots
	int wmOpponentBase = vobBlueDog, wmTeamBase = vobRedDog;
	int wlOpponentBase = NUM_TEAM_MEMBER, wlTeamBase = 0;
	if (gps->targetGoal == vobYellowGoal) {
		wmOpponentBase = vobRedDog, wmTeamBase = vobBlueDog;
		wlOpponentBase = 0, wlTeamBase = NUM_TEAM_MEMBER;
	}
	int vOpponentBase = wmOpponentBase, vTeamBase = wmTeamBase;

	// this is needed for visual backoff!
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		// team members from vision
		vTeammate[i] = vision->vob[vTeamBase++];
		vTeammate[i].calcXY(gps->self().pos.x, gps->self().pos.y, gps->self().getHackedH());
		// opponent members from vision
		vOpponent[i] = vision->vob[vOpponentBase++];
		vOpponent[i].calcXY(gps->self().pos.x, gps->self().pos.y, gps->self().getHackedH());
	}            

	standTall = false;
}

/*
void Behaviours::makeIndicators(int inds[]) {
	inds[0]  = mouth;
	inds[1]  = tail_v;
	inds[2]  = tail_h;
	inds[3]  = ear_l;
	inds[4]  = ear_r;   
	inds[5]  = led1  | mode_led1;
	inds[6]  = led2  | mode_led2;
	inds[7]  = led3  | mode_led3;
	inds[8]  = led4  | mode_led4; 
	inds[9]  = led5  | mode_led5;
	inds[10] = led6  | mode_led6;
	inds[11] = led7  | mode_led7;
	inds[12] = led8  | mode_led8;
	inds[13] = led9  | mode_led9;
	inds[14] = led10 | mode_led10;
	inds[15] = led11 | mode_led11;
	inds[16] = led12 | mode_led12;
	inds[17] = led13 | mode_led13;
	inds[18] = led14 | mode_led14;
	inds[19] = head_color;
	inds[20] = head_white;
	inds[21] = mode_red;
	inds[22] = mode_green;
	inds[23] = mode_blue;
	inds[24] = wireless;
	inds[25] = back_rear_color;
	inds[26] = back_rear_white;
	inds[27] = back_middle_color;
	inds[28] = back_middle_white;
	inds[29] = back_front_color;
	inds[30] = back_front_white;
}
*/


/* this function is called by Vision when it receives a message
   from the wireless module */
void Behaviours::processCommand(const CommandData& cmd) {
   
	if (cmd.type == ROBOCOMMANDER_DATA) {
		if (cmd.RC_COMMAND == UTHER_KICKING_OFF) {
            std::cerr << "Received UTHER_KICKING_OFF command, not implemented" << endl;
            
        // this one is no longer valid from RoboCommander
        // mode changes are done from the GameController
		} else if (cmd.RC_COMMAND == MODE_CHANGE) {            
            /*
			mode_ = (StateModes)(cmd.value);

			if (mode_==mStationaryLocalize) {
				actionTimer = NUM_FRAMES_TO_PAN;
			}
			command.clear();
            */
            std::cerr << "Received MODE_CHANGE command, not implemented" << endl;

        // setting the sending of debug, moved from Behaviours.cc            
		} else if (cmd.RC_COMMAND == CPLANE_SEND) {
#ifndef OFFLINE
            send_data[RLNK_CPLANE] = ! send_data[RLNK_CPLANE];
#endif
            
        // setting the sending of debug            
		} else if (cmd.RC_COMMAND == BURST_MODE) {
			cmdBurst =(int)(cmd.value);
			burstCounter = BURST_NUMBER;
		
        } else if (cmd.RC_COMMAND == WM_SEND) {
			cmdWDisplay =(int)(cmd.value);

		} else if (cmd.RC_COMMAND == YUVPLANE_SEND) {
            #ifndef OFFLINE            
			    transmission->sendYUVPlane();
            #endif //OFFLINE            
		
        } else if (cmd.RC_COMMAND == OPLANE_SEND) {
			// disabled
		
        } else if (cmd.RC_COMMAND == YUVPLANE_STREAM){
			logYUVFrequency = (int) (cmd.value);
			if (logYUVFrequency == 0)
				logYUVTimer = -1;
			else
				logYUVTimer = 0;
		
        } else if (cmd.RC_COMMAND == COMMAND_TO_PYTHON_MODULE){
            PythonBehaviours::gotWirelessPythonCommand(cmd.rawValue);
        } else {
		    // leave it for Behaviours::processWControl which gets checked
            // each frame
            command = cmd;
        }
        
	} 

}


// Sunny: GameController object has now been implemented. The game controller
//        data received is passed on to Vision, which is then passed on here
void Behaviours::processControllerData() {
        
    // get pointer to the team I am currently on
    if (gameData.teams[TEAM_BLUE].teamNumber == (uint32)TEAM_NUMBER) {
        myTeam = gameData.teams[TEAM_BLUE];
    } else if (gameData.teams[TEAM_RED].teamNumber == (uint32)TEAM_NUMBER) {
        myTeam = gameData.teams[TEAM_RED];
    } else {
        // this packet does not belong in this game since I am neither red nor 
        // blue, just ignore it instead of creating spam
        // this should already have been checked by the GameController object
        return;
    }

    gameDataTime = vision->vFrame;
    
    // Look at the game state.
    switch(gameData.state) {
            
        case STATE_INITIAL:
			gps->setPenalised(false);
			actionTimer = NUM_FRAMES_TO_PAN;
            break;
  
        case STATE_READY:
            gps->setPenalised(false);
            break;

        case STATE_SET:
		    gps->setPenalised(false);
			break;                            

        case STATE_PLAYING:
            gps->setPenalised((myTeam.players[PLAYER_NUMBER-1].penalty != PENALTY_NONE));
			break;

        case STATE_FINISHED:
            //if (prevGameData.state != STATE_FINISHED)
            //    Behaviours::setDogRelaxed(); // relax joints
		    break;
    }

    //if (prevGameData.state == STATE_FINISHED && gameData.state != STATE_FINISHED)
    //    Behaviours::setDogRelaxed(); // unrelax

    // Look at the color.
    switch(myTeam.teamColour) {

        case TEAM_RED:
			gps->SetGoals(true);
            #ifndef OFFLINE
		        transmission->sendPlayerNumber(PLAYER_NUMBER, true);
            #endif
            break;
        
        case TEAM_BLUE:
            gps->SetGoals(false);
            #ifndef OFFLINE
		        transmission->sendPlayerNumber(PLAYER_NUMBER, false);
            #endif
            break;
    }


    // Look at the kickoff
    // the kickOff appears to be applied by PyEmbed.cc which looks at the 
    // kickOff variable and passes it onto the Python behaviours
    // the values being passed here should emulate what the old system 
    // used to be (see Constants.py)
    if (gameData.kickOffTeam == myTeam.teamColour) {
        kickOff = 0x01;
    } else {
        kickOff = 0x02;
    }

    // Look at the ball out information. For two seconds after the
    // drop in we reset the ball to the likely replacement point (if
    // ball is out of sight)
    if (gameData.dropInTime <= 1) {
        gps->setBallOut(gameData.dropInTeam != myTeam.teamColour);
    }

}

void Behaviours::saveYUVPhoto() {
    
    using Behaviours::vision;
    using Behaviours::leds;
    
    //save YUV plane to the stick.
    char * bflFile;
    bflFile =(char*)malloc(512);
    yuvSaveNumber++;
    cout << "Saving YUV photo " << yuvSaveNumber << endl;
    int inds[NUM_OF_INDICATORS];

    sprintf(bflFile, "/MS/yuv%.2d.bfl",(int)yuvSaveNumber);

    ofstream yuvFile(bflFile);
    if (yuvFile) {

        memset(inds,0,sizeof(inds));
        inds[5] = 255;
        inds[6] = 255;
        inds[7] = 255;
        inds[8] = 255;
        inds[9] = 255;

        #ifndef OFFLINE
            Behaviours::transmission->sendIndicators(inds, sizeof(inds));
        #endif // OFFLINE

        for (int h = 0; h < CPLANE_HEIGHT; h ++) {
            /* This format is the bfl format. Has to be same as it
             * get read in mc/colour calibration tools. */
            //write Y
            for (int w = 0; w < CPLANE_WIDTH; w ++)
                yuvFile <<(char)(((char*)(vision->yplane))[h*6*CPLANE_WIDTH+w]);

            //write U
            for (int w = 0; w < CPLANE_WIDTH; w ++)
                yuvFile <<(char)(((char*)(vision->yplane))[(h*6+1)*CPLANE_WIDTH+w]);

            //write V
            for (int w = 0; w < CPLANE_WIDTH; w ++)
                yuvFile <<(char)(((char*)(vision->yplane))[(h*6+2)*CPLANE_WIDTH+w]);

            // write the 3 unused Y values.  
            for (int w = 0; w < CPLANE_WIDTH; w ++)
                yuvFile <<(char)(((char*)(vision->yplane))[(h*6+3)*CPLANE_WIDTH+w]);

            for (int w = 0; w < CPLANE_WIDTH; w ++) 
                yuvFile <<(char)(((char*)(vision->yplane))[(h*6+4)*CPLANE_WIDTH+w]);

            for (int w = 0; w < CPLANE_WIDTH; w ++)
                yuvFile <<(char)(((char*)(vision->yplane))[(h*6+5)*CPLANE_WIDTH+w]);

            //write C
            for (int w = 0; w < CPLANE_WIDTH; w ++)
                yuvFile << (char) '\0';
        }
        
        if (yuvSaveNumber%2) {
            leds(1,1,1,2,2,2);
        } else {
            leds(2,2,2,1,1,1);
        }
        yuvFile.close();

        memset(inds,0,sizeof(inds));
        #ifndef OFFLINE
            Behaviours::transmission->sendIndicators(inds, sizeof(inds));
        #endif // OFFLINE
    
    } else {
        leds(1,2,1,1,2,1);
    }
}

