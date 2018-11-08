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
 * $Id: WalkingLearner.h 5113 2005-03-17 04:18:41Z alexn $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _WALKING_LEARNER_H_
#define _WALKING_LEARNER_H_

#include "Behaviours.h"
//#include "KimCommand.h"
#include "Timer.h"
#include "../share/WalkLearningParams.h"
//#include "../vision/Blob.h"

#include <iostream>

using namespace std;

enum LearningState {
	lsIDLE = 0,
    lsMOVING_TO_READY_STATE,
    lsREADY,
	lsSTART_LEARNING,
	lsPREPARATION1,
	lsWALKING_FORTH,      //5  
	lsPREPARATION2,
	lsTURNING180,
	lsPREPARATION3,
	lsWALKING_BACK,
	lsPREPARATION4,     //10    
	lsDONE,
	lsGETTING_LOST,
	lsLOW_BATTERY,
	lsWALK_CALIBRATING,
	lsLEARNING_PINK_DISTANCE_LOW,       //15
	lsLEARNING_PINK_DISTANCE_HIGH,
	lsTURN_AWAY_FROM_PINK,
	lsPRE_PREPARATION1,
	lsNOSTATE
};

namespace WalkingLearner {
	extern const char *stateToStr[];
	extern LearningState nextState[] ;

	extern LearningState learningState;
	extern LearningState lastLearningState;
	extern Timer timer;
	extern int lockFrames;
    extern int actionCounter;
	extern double lastWalkingForthTime, lastWalkingBackTime;

	void initPlayer();
	void initNextState();
	bool DecideNextAction();
	void gotWirelessCommand(const char *value);


	void doIdleState();
	void stayReady();
	void moveToReadyState();
//	void replyQuery(KimCommand &command);
	void startLearning();
	void walkingForth();
	void walkingBack();
	void turnAround180();
	void turnAround180AtLowPinkBeacon();
	void turnAround180AtHighPinkBeacon();
	void doPrePreparation1();
	void preparation1();
	void preparation2();
	void preparation3();
	void preparation4();
	void doneLearning();
	void learnPinkDistanceLow();
	void learnPinkDistanceHigh();
	void turnAwayFromPink();

	//utilities functions
	void walkingToBeacon(const VisualObject &beacon);
	void trackPink();
	void setWalkingStraightForward();
	void setWalkCalibration();
///	void setHeadTowardBeacon(HalfBeacon &); removed for subvision
	void initializeNewAction();
	void goNextState();
	void stopEverything();
    void setLearningParameters(const char *str);
    void setWalkLearningParameters(const char *paraStr);
    void setLearningState(const char *str);
    void setLearningWalkParams();
    void storeDebuggingInformation();
	void learnPinkDistance(double &pinkMean, double &pinkVariance, int &pinkCount ,int LEARNING_PERIOD);


	extern double irSensorValue;

    extern WalkLearningParams wlParams;
};

#endif // _WALKING_LEARNER_H_
