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
 * $Id: CommonSense.cc 7169 2005-07-03 14:49:31Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#ifndef OFFLINE
#include <OPENR/ODataFormats.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserverVector.h>
#include <OPENR/OPENREvent.h>
#include <OPENR/OPENRAPI.h>
#endif // OFFLINE

#include "CommonSense.h"

static const int RANGE2OBSTACLE_MAX = 1501;
static const int MIN_EFFECTIVE_INFRARED_ANGLE = -30;

static const int HEAD_PRESSURE_THRESHOLD = 10;

static const int ACCEL_BACK_FALL = 8600000;
static const int ACCEL_FRONT_FALL = 8600000;
static const int ACCEL_RIGHT_FALL = 9000000;
static const int ACCEL_LEFT_FALL = -8600000;
static const int ACCEL_REAR_FALL = -8500000;
static const int ACCEL_Z_FALLEN = -6500000;
static const int CONFIRM_FALLEN = 5;

const double HEAD_MOVE_THRESHOLD = 1.7E-2;

/**
 * CommenSense constructor
 */
CommonSense::CommonSense() {
  // Initialise all values
  sensorFrame = 0;
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorVal[i] = 0;
  }
  
  hasFallen = NotFallenFS;
  range2obstacle = 0;
  headMoving = false;
  lastTilt1 = 0; lastDT1 = 0;
  lastTilt2 = 0; lastDT2 = 0;
  lastPan = 0; lastDP = 0;
  
  for (int i = 0 ; i < NUM_SWITCHES ; i++) {
    switch_sensor[i].count = 0;
  }
  
  for (int i = 0 ; i < NUM_PRESSES ; i++) {
    press_sensor[i].SetThreshold(HEAD_PRESSURE_THRESHOLD);
  }
  
  fallenBack = fallenRear = fallenRight = fallenLeft = fallenFront = 0;
  
  for(int i=0;i<NUM_SENSORS;i++) {
    lastMaxStepPWM[i] = currentMaxStepPWM[i] = -VERY_LARGE_INT;
  }
  lastStepID = currentStepID = 0;

}

/**
 * Update sensor values.
 *
 * @param       event   Sony event data
 */
void CommonSense::ResultSensor(const ONotifyEvent &event, unsigned int stepID) {
#ifndef OFFLINE
	OSensorFrameVectorData *vec = reinterpret_cast <OSensorFrameVectorData*>(event.RCData(0)->Base());
	OSensorFrameInfo *info = vec->GetInfo(0);
	sensorFrame = (long) vec->info[0].frameNumber;

	// Retrieving the latest sensor values
	long latestFrame = info->numFrames - 1;
	for (int i = 0; i < NUM_SENSORS; i++) {
	  OSensorFrameData *data = vec->GetData(i);
	  // we're only interested in the latest value here
	  sensorVal[i] = data->frame[latestFrame].value;
	  dutyCycleValue[i] = ( (OJointValue*)&data->frame[latestFrame])->pwmDuty;
	}

    // Calculate maximum PWM values
    checkMaxPWM(stepID);
    
    // Update range2obstacle
    // divide by 1000 to get cm measurement 
    // use 60% last measurement and 40% new measurement
    
    range2obstacle = range2obstacle * 0.6
      + 0.4 * sensorVal[ssINFRARED_FAR] / 1000.0;

    // Update headMoving
    checkHeadMoving();

    double headPitch = MICRO2RAD(sensorVal[ssHEAD_TILT]);
    double headYaw = MICRO2RAD(sensorVal[ssHEAD_PAN]);
    double effectivePitch = headPitch *cos(headYaw);
    
    if (RAD2DEG(effectivePitch) < MIN_EFFECTIVE_INFRARED_ANGLE) {
      //cout << "Range discarded (Head Looking too far down)" << endl;
      range2obstacle = RANGE2OBSTACLE_MAX;
    }
	// Update switch status
	switch_sensor[SWITCH_CHIN].Update(sensorVal[ssCHIN]);
	switch_sensor[SWITCH_FL_PALM].Update(sensorVal[ssFL_PALM]);
	switch_sensor[SWITCH_RL_PALM].Update(sensorVal[ssRL_PALM]);
	switch_sensor[SWITCH_FR_PALM].Update(sensorVal[ssFR_PALM]);
	switch_sensor[SWITCH_RR_PALM].Update(sensorVal[ssRR_PALM]);

	// Update head press status
	press_sensor[PRESS_HEAD].Update(sensorVal[ssHEAD]);
	press_sensor[PRESS_BACK_FRONT].Update(sensorVal[ssBACK_FRONT]);
	press_sensor[PRESS_BACK_MIDDLE].Update(sensorVal[ssBACK_MIDDLE]);
	press_sensor[PRESS_BACK_REAR].Update(sensorVal[ssBACK_REAR]);

	if (sensorVal[ssACCEL_Z] > ACCEL_BACK_FALL) {
		fallenBack++;
		fallenRear = fallenRight = fallenLeft = fallenFront = 0;
	}
	else if (sensorVal[ssACCEL_Z] > 0) {
		// z accel is positive up, negative down, so positive z accel value
		// means the robot is more on its top than its bottom, so something
		// is very wrong
		if (abs(sensorVal[ssACCEL_FOR]) > abs(sensorVal[ssACCEL_SIDE])) {
			if (sensorVal[ssACCEL_FOR] < 0) {
				fallenRear++;
				fallenBack = fallenRight = fallenLeft = fallenFront = 0;
			}
			else {
				fallenFront++;
				fallenBack = fallenRear = fallenRight = fallenLeft = 0;
			}
		}
		else {
			if (sensorVal[ssACCEL_SIDE] < 0) {
				fallenLeft++;
				fallenBack = fallenRear = fallenRight = fallenFront = 0;
			}
			else {
				fallenRight++;
				fallenBack = fallenRear = fallenLeft = fallenFront = 0;
			}
		}
	}
	else if (sensorVal[ssACCEL_Z] > ACCEL_Z_FALLEN) {
		if (sensorVal[ssACCEL_FOR] < ACCEL_REAR_FALL) {
			fallenRear++;
			fallenBack = fallenRight = fallenLeft = fallenFront = 0;
		}
		else if (sensorVal[ssACCEL_SIDE] > ACCEL_RIGHT_FALL) {
			fallenRight++;
			fallenBack = fallenRear = fallenLeft = fallenFront = 0;
		}
		else if (sensorVal[ssACCEL_SIDE] < ACCEL_LEFT_FALL) {
			fallenLeft++;
			fallenBack = fallenRear = fallenRight = fallenFront = 0;
		}
		else if (sensorVal[ssACCEL_FOR] > ACCEL_FRONT_FALL) {
			fallenFront++;
			fallenBack = fallenRear = fallenRight = fallenLeft = 0;
		}
		else {
			fallenBack = fallenRear = fallenFront = fallenRight = fallenLeft = 0;
		}
	}
	else {
		fallenBack = fallenRear = fallenFront = fallenRight = fallenLeft = 0;
	}

	if (fallenBack > CONFIRM_FALLEN
			|| fallenLeft > CONFIRM_FALLEN
			|| fallenRight > CONFIRM_FALLEN) {
		hasFallen = FallenSideFS;
	}
	else if (fallenRear > CONFIRM_FALLEN) {
		hasFallen = FallenRearFS;
	}
	else if (fallenFront > CONFIRM_FALLEN) {
		hasFallen = FallenFrontFS;
	}
	else {
		hasFallen = NotFallenFS;
	}

	//oSensor->AssertReady();
#endif // OFFLINE
}

double CommonSense::robotRelativeForPSD(double psd, double pan, double tilt) {
	double cos_tilt = cos(-DEG2RAD(tilt) + TILT_OFFSET);
	double sin_tilt = sin(DEG2RAD(tilt));
	double cos_pan = cos(DEG2RAD(pan));
	double sin_pan = sin(DEG2RAD(pan));
	double x3 = 0;
	double y3 = 0;
	double z3 = psd;

	// Translation for Length and height offsets for psd sensor
	double x4 = x3;
	double y4 = y3 + PSD_HEADBASE_HEIGHT;
	double z4 = z3 + PSD_HEADBASE_LENGTH;

	// Rotation for Head Pan
	//double x5 = x4 *cos_pan + z4 *sin_pan;
	double y5 = y4;
	double z5 = -x4 *sin_pan + z4 *cos_pan;

	// Translation for Neck Length
	//double x6 = x5;
	double y6 = y5 + NECK_LENGTH;
	double z6 = z5;

	// Rotation for Head Tilt
	//      double x7 = x6;
	//      double y7 = y6 * cos_tilt - z6 * sin_tilt;
	double z7 = y6 *sin_tilt + z6 *cos_tilt;
	return z7;
	// Calculate actual distance and robot relative heading and elevation
}

void CommonSense::resetFallenState() {
	fallenBack = fallenRear = fallenRight = fallenLeft = fallenFront = 0;
	hasFallen = NotFallenFS;
}

bool CommonSense::getStanceValues(double *shoulderHeight, double *hipHeight) {
	//TODO:calculate from sensors values
	// return bodyTilt instead of bool
	//Dodgy values
	*shoulderHeight = 70;
	*hipHeight = 110;
	return true;
}

// Checks the head movement speed
void CommonSense::checkHeadMoving()
{
    double dT1, dT2, dP;
    
    dT1 = MICRO2RAD(sensorVal[ssHEAD_TILT]) - lastTilt1;
    dT2 = MICRO2RAD(sensorVal[ssHEAD_CRANE]) - lastTilt2;
    dP  = MICRO2RAD(sensorVal[ssHEAD_PAN]) - lastPan;
    dT1 = 0.5 * dT1 + 0.5 * lastDT1;
    dT2 = 0.5 * dT2 + 0.5 * lastDT2;
    dP  = 0.5 * dP  + 0.5 * lastDP;
    
    headSpeed = sqrt(dT1 * dT1 + dT2 * dT2 + dP * dP);
    headMoving = (headSpeed > HEAD_MOVE_THRESHOLD);
    
    lastTilt1 = MICRO2RAD(sensorVal[ssHEAD_TILT]);
    lastTilt2 = MICRO2RAD(sensorVal[ssHEAD_CRANE]);
    lastPan   = MICRO2RAD(sensorVal[ssHEAD_PAN]);
    lastDT1 = dT1;
    lastDT2 = dT2;
    lastDP  = dP;
}



// checking the max values in each step
void CommonSense::checkMaxPWM(unsigned int stepID) {
  if (currentStepID != stepID) {
    // if step has changed, 
    // copy current info to last, then reset current
    for(int i=0;i<NUM_SENSORS;i++) {
      lastMaxStepPWM[i] = currentMaxStepPWM[i];
      currentMaxStepPWM[i] = -VERY_LARGE_INT;
    }
    lastStepID = currentStepID;
    currentStepID = stepID;
  }

  maxPWMValue = -VERY_LARGE_INT;
  for(int i=ssHEAD_PAN; i<ssRR_ROTATOR; i++) {
    if (currentMaxStepPWM[i] < dutyCycleValue[i])
      currentMaxStepPWM[i] = dutyCycleValue[i];
    if (maxPWMValue < dutyCycleValue[i])
      maxPWMValue = dutyCycleValue[i];    
  }
}
