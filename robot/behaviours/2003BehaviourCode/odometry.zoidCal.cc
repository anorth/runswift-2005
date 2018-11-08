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
   offset walk
   10.5 forward 8 -8 work
   10 forward 13.75 -13.75 work

   10.8 forward 8 -8 work
   */

/*
 * Last modification background information
 * $Id$
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * Odometry Calibration tool
 *
 **/ 

#include "Behaviours.h"
#include "../share/BallDef.h"
#include "../share/Common.h"
#include "../share/CanterCalib.h"
#include "odometry.h"
#include "walkVals.h"

using Odometry::state;
using Odometry::stateType;

static const int walkFrames = 200;
static const int pauseFrames = 70;
static int stateCounter = 0;
stateType state;

static bool stateEyes = true;
#define widEyes  leds(2,1,1,2,1,1)
#define lawEyes  leds(2,2,1,2,2,1)
#define wbEyes   leds(1,1,2,1,1,2)
#define lawbEyes leds(1,2,2,1,2,2)
#define cpEyes   leds(2,1,2,2,1,2)
#define lacpEyes leds(2,2,2,2,2,2)

void Odometry::initOdometryTester() {
	lockMode = NoMode;        
	lostBall = 0;
	grabTime = 0;
	state = pause;
	srand48(PLAYER_NUMBER);
}

void Odometry::doOdometryTester() {
	setDefaultParams();
	setWalkParams();
	setOmniParams();
	setOdometryTesterValues();
	chooseOdometryTesterStrategy();
}

void Odometry::setOdometryTesterValues() {
	leds(1, 1, 1, 1, 1, 1); //reset the leds everyframe
	switch (state) {
		case antiSlow: leds(2, 1, 1, 1, 1, 1);
			       break;
		case antiMed: leds(1, 2, 1, 1, 1, 1);
			      break;
		case antiFast: leds(1, 1, 2, 1, 1, 1);
			       break;
		case clockSlow: leds(1, 1, 1, 2, 1, 1);
				break;
		case clockMed: leds(1, 1, 1, 1, 2, 1);
			       break;
		case clockFast: leds(1, 1, 1, 1, 1, 2);
				break;
		default: leds(1, 1, 1, 1, 1, 1);
			 break;
	}
}

void Odometry::chooseOdometryTesterStrategy() {
	if (mode_ == mStartMatch) {
		setStandParams();
		StationaryLocalize();
	}
	else {
		switch (state) {
			case pause: if (stateCounter < pauseFrames) {
					    setStandParams();
				    }
				    else {
					    state = antiSlow;
					    stateCounter = 0;
					    return;
				    }
				    stateCounter++;
				    break;  


			case antiSlow: if (stateCounter < walkFrames) {
					       setWalkParams();
					       walkType = ZoidalWalkWT;
					       forward = 10.6; //MAX_OFFSET_FORWARD;    
					       left = 0;
					       if (vtGoal.cf > 3)
						       turnCCW = CLIP(vtGoal.h / 2, 8.0);
					       else
						       turnCCW = 0;
				       }
				       else {
					       state = pantiSlow;
					       stateCounter = 0;
					       return;
				       }
				       stateCounter++;
				       break;  
			case pantiSlow: if (stateCounter < pauseFrames) {
						setStandParams();
					}
					else {
						state = antiMed;
						stateCounter = 0;
						return;
					}
					stateCounter++;
					break;  



			case antiMed: if (stateCounter < walkFrames) {
					      setWalkParams();
					      walkType = ZoidalWalkWT;
					      forward = 10.7; //MAX_OFFSET_FORWARD;   
					      left = 0;
					      /*if(vtGoal.cf > 3)
						turnCCW = CLIP(vtGoal.h/2,8.0);
						else*/
					      turnCCW = 8.0;
				      }
				      else {
					      state = pantiMed;
					      stateCounter = 0;
					      return;
				      }
				      stateCounter++;
				      break;  
			case pantiMed: if (stateCounter < pauseFrames) {
					       setStandParams();
				       }
				       else {
					       state = antiFast;
					       stateCounter = 0;
					       return;
				       }
				       stateCounter++;
				       break;  


			case antiFast: if (stateCounter < walkFrames) {
					       setWalkParams();
					       walkType = ZoidalWalkWT;
					       forward = 10.8; //MAX_OFFSET_FORWARD;    
					       left = 0;
					       /*if(vtGoal.cf > 3)
						 turnCCW = CLIP(vtGoal.h/2,8.0);
						 else*/
					       turnCCW = -8.0;
				       }
				       else {
					       state = pantiFast;
					       stateCounter = 0;
					       return;
				       }
				       stateCounter++;
				       break;  
			case pantiFast: if (stateCounter < pauseFrames) {
						setStandParams();
					}
					else {
						state = pause;
						stateCounter = 0;
						return;
					}
					stateCounter++;
					break;  



			default: setStandParams();
				 state = pause;
				 stateCounter = 0;
				 return;
		}
	}
}
