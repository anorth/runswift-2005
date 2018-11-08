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
 * $Id: CommonSense.h 7169 2005-07-03 14:49:31Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * contains basic (but more advanced than those in common.h) utilities for the robots
 * involves the robot sensors
 *
 **/


#ifndef CommonSense_h_DEFINED
#define CommonSense_h_DEFINED

/*
#include <OPENR/ODataFormats.h>
#include <OMWares/OMGsensorData.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserverVector.h>
#include <OPENR/OPENREvent.h>
#include <OPENR/OPENRAPI.h>
*/
#include "../share/Common.h"
#include "../share/sensors.h"

typedef class OObserver OObserverVector;
class ONotifyEvent;

typedef enum {
	NotFallenFS,
	FallenSideFS,
	FallenFrontFS,
	FallenRearFS
} FallState;

enum switches {
	SWITCH_CHIN = 0,
	SWITCH_FL_PALM = 1,
	SWITCH_RL_PALM = 2,
	SWITCH_FR_PALM = 3,
	SWITCH_RR_PALM = 4,
	NUM_SWITCHES = 5
};

struct SensorSwitch {
	int count;

	void Update(long state_) {
		if (state_ > 0) {
			count++;
		}
		else {
			count = 0;
		}
	}
};

enum presses {
	PRESS_HEAD = 0,
	PRESS_BACK_FRONT = 1,
	PRESS_BACK_MIDDLE = 2,
	PRESS_BACK_REAR = 3,
	NUM_PRESSES = 4
};

struct SensorPress {
	long pressure;
	long threshold;
	int count;

	void SetThreshold(long threshold_) {
		threshold = threshold_;
		count = pressure = 0;
	}
	void Update(long pressure_) {
		pressure = pressure_;

		if (pressure > threshold) {
			count++;
		}
		else {
			count = 0;
		}
	}
};

typedef enum {
	hpDEMOBILIZE = 0,
	hpLOCALIZE = 1,
	hpMOBILIZE = 2
}
HeadPressState;

const int NUM_HEADPRESS_STATES = 3;

class CommonSense {
	public:

		CommonSense();
		~CommonSense() {
		}
		void ResultSensor(const ONotifyEvent &, unsigned int);
		double robotRelativeForPSD(double, double, double);
		void resetFallenState();

		OObserverVector *oGSensor;

		long sensorFrame;
		long sensorVal[NUM_SENSORS];
		long dutyCycleValue[NUM_SENSORS];

		long lastMaxStepPWM[NUM_SENSORS];
		unsigned int lastStepID;

		FallState hasFallen;
		double range2obstacle;
		SensorSwitch switch_sensor[NUM_SWITCHES];
		SensorPress press_sensor[NUM_PRESSES];

		int fallenBack, fallenRear, fallenRight, fallenLeft, fallenFront;

		bool getStanceValues(double *shoulderHeight, double *hipHeight);
        bool isHeadMoving() {return headMoving;};
        double getHeadSpeed() {return headSpeed;};
        long getMaxPWMValue() {return maxPWMValue;};

    private:

        void checkHeadMoving();
        
        bool headMoving;
        double headSpeed;
        double lastDT1, lastDT2, lastDP;
        double lastTilt1, lastTilt2, lastPan;
        
        void checkMaxPWM(unsigned int);
	// values for the current step, so may not be accurate yet
	long currentMaxStepPWM[NUM_SENSORS];
	unsigned int currentStepID;
	long maxPWMValue;
};

#endif // CommonSense_h_DEFINED

