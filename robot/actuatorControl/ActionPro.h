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
 * $Id: ActionPro.h 7169 2005-07-03 14:49:31Z weiming $
 *
 * Copyright(c) 2004 UNSW
 * All Rights Reserved.
 * 
 * Interface of the ActuatorControl object to call the responsible functions in PWalk and HeadMotion.
 * !!! Not clear why this should be seperated from ActuatorControl.
 *
 **/

#ifndef _ActionPro_h_DEFINED
#define _ActionPro_h_DEFINED

#include "PWalk.h"
#include "JointAgentMediator.h"
#include "HeadMotionMediator.h"
#include "JointCommand.h"
//#include "Indicators.h"
#include "../share/Common.h"
#include "../share/ActionProDef.h"
#include "../share/OffsetDef.h"
#include "../share/SwitchBoard.h"
#include "../share/ActuatorWirelessData.h"
#include "../share/sensors.h"

#ifdef OFFLINE
#include "OfflineOPENR.h"
#include "OverideOfflineClasses.h"
#endif //OFFLINE

static const int NUM_AWD = 20;

class ActionPro {
	public:
		ActionPro();
		~ActionPro() {
			/*
			   if (pAgent) {
			   delete(pAgent);
			   }
			   if (headAgent) {
			   delete(headAgent);
			   }
			   */
		}

		void SetPAgent(OSubject *sbjShmPWalk);
#ifdef OFFLINE
		void execute(const AtomicAction &atomicAction);
#else
		void execute(const ONotifyEvent &event);
#endif
		void ContinueStep();
		void initMem();
		void updateSharedInfo(); //set value in shared memory sothat Behaviours can read
		bool sendWirelessMessage(ActuatorWirelessDataEntry *awd);
		//void ResultSensor(const ONotifyEvent &event);
		void storeCommandValueToSend(JointCommand &jCommand);
		void relaxJoints();
		void turnHighGain();
		void turnLowGain ();

		AtomicAction next;
		bool takenFirstStep;

		PWalk *pAgent; //PWalk is offline compatible
		HeadAgentMediator *headAgent;
		JointAgentMediator *jointAgent; //now JointAgent takes care of everything ( buffer, mem, joints...)


		OSubject *sEffector, *sHead, *sOdometer, *sWirelessMessenger; 
		EffectorCommander *eCommander;

		int *leading_leg; /* -1 = left, 1 = right, 0 = unknown */
		int *currentWalkType;
                int *currentMinorWalkType;
		int *currentStep;
		int *currentPG;
		double *frontShoulderHeight;	// mm
		double *bodyTilt;	// radians
		double *desiredPan;
		double *desiredTilt;		
		double *desiredCrane;
		bool *isHighGain;
		unsigned int* stepID;

		

		void setSensorVals(const JointCommand &joints);

        long sensorVal[NUM_SENSORS];
        long dutyCycleValue[NUM_SENSORS];
        long sensorFrameID;
        ActuatorWirelessDataEntry awd[NUM_AWD];
        int nextFreeDataLoc;

	ActuatorWirelessDataEntry awdPeak;

#ifdef OFFLINE
	protected:
#endif
	RCRegion *shmPWalk; /* shared memory region for telling us what to do */
	
        bool jointRelaxed;
	
        void storeSensorValueToSend(long frameID);

};

#endif // _ActionPro_h_DEFINED

