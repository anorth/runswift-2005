/*

   Copyright 2004 The University of New South Wales (UNSW) and National    
   ICT Australia (NICTA).

   This file is part of the 2004 team rUNSWift RoboCup entry.    You may    
   redistribute it and/or modify it under the terms of the GNU General    
   Public License as published by the Free Software Foundation; either    
   version 2 of the License, or (at your option) any later version as    
   modified below.    As the original licensors, we add the following    
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
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the GNU    
   General Public License for more details.

   You should have received a copy of the GNU General Public License along    
   with this source code; if not, write to the Free Software Foundation,    
   Inc., 59 Temple Place, Suite 330, Boston, MA    02111-1307    USA

*/

/*
 * Last modification background information
 * $Id: ActuatorControl.cc 7107 2005-07-02 05:13:25Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 * 
 **/

#define OPENR_STUBGEN

#include <OPENR/core_macro.h>

#include <MCOOP.h>
#include <OPENR/OPENR.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OPENRMessages.h>
#include <OPENR/core_macro.h>

#include "ActuatorControl.h"
#include "ActionPro.h"
#include "../share/WalkLearningParams.h"

static const bool bDebugDuty = false;

ActuatorControl::ActuatorControl() {
	OSYSDEBUG(("ActuatorControl::ActuatorControl()\n"));
	wait4settle = 30;
    
    bufForward = 0.0;
    bufLeft = 0.0;
    bufTurn = 0.0;
}

OStatus ActuatorControl::DoInit(const OSystemEvent &) {
	OSYSDEBUG(("ActuatorControl::DoInit()\n"));

	// NewComData    /* generates the necessary number of subjects and observers */
	// SetComData    /* registers the communication service with other objects */
	NEW_ALL_SUBJECT_AND_OBSERVER;
	REGISTER_ALL_ENTRY;
    if (subject[sbjGps]->SetBufferSize(10*3) != oSUCCESS)
        cout << "unable to set buffer size in ActuatorControl::DoInit" << endl;
    if (subject[sbjGps]->SetNotifyUnitSize(3) != oSUCCESS)
        cout << "unable to set Notify Unit size in ActuatorControl::DoInit" << endl;
	SET_ALL_READY_AND_NOTIFY_ENTRY;

	sEffector = subject[sbjEffectors];
	sHead = subject[sbjHead];
	sOdometer = subject[sbjOdometer];
	sWirelessMessenger = subject[sbjSendMessageToWireless];

	eCommander->init(subject[sbjECommander]);

	indicatorCommander = new Indicators((subject[sbjIndicators]), *eCommander);
	initMem();
	initMACAddress();

	return oSUCCESS;
}

OStatus ActuatorControl::DoStart(const OSystemEvent &) {
	OSYSDEBUG(("ActuatorControl::DoStart()\n"));

	ENABLE_ALL_SUBJECT;
	ASSERT_READY_TO_ALL_OBSERVER;

	return oSUCCESS;
}

OStatus ActuatorControl::DoStop(const OSystemEvent &) {
	eCommander->stop();

	DISABLE_ALL_SUBJECT;

	for (int i = 0; i < numOfObserver; ++i) {
		observer[i]->DeassertReady();
	}

	return oSUCCESS;
}

OStatus ActuatorControl::DoDestroy(const OSystemEvent &) {
	DeleteComData    /* deletes all observers and subjects spawned
			    in DoInit() */
		return oSUCCESS;
}

void ActuatorControl::ReadyEffectors(const OReadyEvent &) {
	ContinueStep();

    bufForward += pAgent->delta_forward;
    bufLeft += pAgent->delta_left;
    bufTurn += pAgent->delta_turn;

	//This effectively calls a GPSMotionUpdate
    if (subject[sbjGps]->IsAnyReady()) {
    	subject[sbjGps]->SetData(&bufForward, sizeof(bufForward));
    	subject[sbjGps]->SetData(&bufLeft, sizeof(bufLeft));
    	subject[sbjGps]->SetData(&bufTurn, sizeof(bufTurn));
        
    	if (subject[sbjGps]->NotifyObservers() != oSUCCESS)
            cout << "Notify Failed in ActuatorControl::ReadyEffectors" << endl;
            
        bufForward = 0.0;
        bufLeft = 0.0;
        bufTurn = 0.0;
    }
}

void ActuatorControl::ReadyECommander(const OReadyEvent &event) {
	// may not need this check; legacy comment
	if (!eCommander->isGainSet()) {
		eCommander->continueInit(event);
	}
}

void ActuatorControl::ResultSensor(const ONotifyEvent &event){
    OSensorFrameVectorData *vec = reinterpret_cast <OSensorFrameVectorData*>(event.RCData(0)->Base());
    OSensorFrameInfo *info = vec->GetInfo(0);
    long sensorFrame = (long) info->frameNumber;

    // Retrieving the latest sensor values
    for (unsigned int frameIndex = 0; frameIndex < info->numFrames; frameIndex++) {
		for (int i = ssMOUTH; i <= ssACCEL_Z; i++) {
			OSensorFrameData *data = vec->GetData(i);
			// we're only interested in the latest value here
			sensorVal[i] = data->frame[frameIndex].value;
			dutyCycleValue[i] = ( (OJointValue*)&data->frame[frameIndex])->pwmDuty;
		}
		sensorFrameID = sensorFrame+frameIndex;
		storeSensorValueToSend(sensorFrameID);
    }
    if (bDebugDuty){
        cout << "Duty : " ;
        for (int i = ssHEAD_CRANE; i <= ssRR_ROTATOR; i++) {
            cout << setw(4) << dutyCycleValue[i] << " " ;
        }
        cout << endl;
    }

    observer[event.ObsIndex()]->AssertReady();

    sendWirelessMessage(NULL);

    OPowerStatus powerStatus;
    if (OPENR::GetPowerStatus(&powerStatus) == oSUCCESS) {
    	if (powerStatus.robotStatus & orsbPAUSE) {
    		cout << "Pause/Power button pressed.  Shutting down." << endl;
    		Shutdown();
    	}
    }
    
}

void ActuatorControl::executeAssert(const ONotifyEvent &event) {
	execute(event);
	observer[event.ObsIndex()]->AssertReady();
}

void ActuatorControl::setOffsetParams(const ONotifyEvent &event) {
	const OffsetParams *nextOP = (const OffsetParams *) (event.Data(0));

	pAgent->offsetWalk->setOffsetParams(nextOP);

	observer[event.ObsIndex()]->AssertReady();
}

void ActuatorControl::setLearningParams(const ONotifyEvent &event) {
    const WalkLearningParams *params = (const WalkLearningParams*) event.Data(0); 
    switch (params->walkType) {
        case EllipseWalkWT:
            pAgent->ellipticalWalk->setLearningParameters(params->ep);
            break;
        case SkellipticalWalkWT:
	  FFCOUT << "Walk params: ";
	  params->sp.print(cout);
	  pAgent->skellipticalWalk->setWalkParams(params->sp);
	  break;
        case SkiWalkWT:
	    pAgent->skiWalk->setWalkParams(params->skp);
            break;
        case NormalWalkWT:
            pAgent->normalWalk->setWalkParams(params->np); 
            break;
        default:
            cout << "Unknown walktype in ActuatorControl::setLearningParams" << endl;
            break;
    }
    observer[event.ObsIndex()]->AssertReady(); //without this, subject can only send once
}


void ActuatorControl::setIndicationAssert(const ONotifyEvent &event) {
	indicatorCommander->setIndication(event);
	observer[event.ObsIndex()]->AssertReady();
}

void ActuatorControl::ReadySetPAgent(const OReadyEvent &) {
	SetPAgent(subject[sbjShmPWalk]);
}

static int crash() {
	// divide by 0 to cause a crash and shut down that way.
	int myNum = 0;
	return 1 / myNum;
}

void ActuatorControl::Shutdown() {
	cout << "ActuatorControl::Shutdown called.  Shutting down." << endl;
	if (false) {
		OBootCondition bootCond;
		if (OPENR::GetBootCondition(&bootCond) != oSUCCESS) {
			crash();
		}
		bootCond.bitmap = obcbPAUSE_SW;	// | bootCond.bitmap;
		if (OPENR::Shutdown(bootCond) != oSUCCESS) {
			crash();
		}
	} else {
		if (OPENR::Fatal(ofatalPAUSE_SW) != oSUCCESS) {
			crash();
		}
	}
}
