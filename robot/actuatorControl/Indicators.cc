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
 * $Id: Indicators.cc 7275 2005-07-05 16:58:15Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 * 
 **/

#include "Indicators.h"

Indicators::Indicators(OSubject *subject, EffectorCommander &eCommander) {
	sIndicators = subject;
	for (int i = 0; i < NUM_OF_INDICATORS; ++i) {
		inds[i] = 0;
	}
	//jointGainNotSet = true; Andres - new binary set
	SetUpBuffer(eCommander);
}

/* Only allow this to work if not in INITIAL, READY or SET states
   The GameController will control the LEDs during these states */
void Indicators::setIndication(const ONotifyEvent &event) {
    
    const int *inds_ = (const int *) (event.Data(0));

    for (int i = 0; i < NUM_OF_INDICATORS; ++i) {
        inds[i] = inds_[i];
    }

    // Buffer still in use
    if (indRegion->NumberOfReference() != 1) {
        return;
    }

    // Mouth - JointCommand2
    for (int i = IND_MOUTH_START; i < IND_TAIL_START; ++i) {
        OJointCommandValue2 *data = (OJointCommandValue2 *)
            (indVec_->GetData(i)->value);
        for (int j = 0; j < NUM_OF_JOINT_IND_FRAMES; j++) {
            data[j].value = inds[i];
        }
    }

    // Tail - JointCommand2
    for (int i = IND_TAIL_START; i < IND_EAR_START; ++i) {
        OJointCommandValue2 *data = (OJointCommandValue2 *)
            (indVec_->GetData(i)->value);
        for (int j = 0; j < NUM_OF_JOINT_IND_FRAMES; j++) {
            data[j].value = inds[i];
        }
    }

    // Ears - JointCommand4
    for (int i = IND_EAR_START; i < IND_FACE_LED3_START; ++i) {
        OJointCommandValue4 *data = (OJointCommandValue4 *)
            (indVec_->GetData(i)->value);
        for (int j = 0; j < NUM_OF_JOINT_IND_FRAMES; j++) {
            data[j].value = (inds[i] == IND_EAR_UP)
                ? ojoint4_STATE1
                : ojoint4_STATE0;
            data[j].period = 1;
        }
    }

    // LEDCommand3
    for (int i = IND_FACE_LED3_START; i < IND_LED2_START; ++i) {
        OLEDCommandValue3 *data = (OLEDCommandValue3 *)
            (indVec_->GetData(i)->value);
        data[0].mode = ((inds[i] & IND_LED3_MODE_B) == IND_LED3_MODE_B)
            ? oled3_MODE_B
            : oled3_MODE_A;
 
        data[0].intensity = (inds[i] & IND_LED3_INTENSITY_MAX);
	/*
	if (inds[i] != 0) {
	  FFCOUT << i << " ind:" <<inds[i] 
		 << " Mode: " << data[0].mode
		 << " Intensity: " << data[0].intensity
		 << endl;
	}
	*/
        data[0].period = 1;
    }


    // LEDCommand2
    for (int i = IND_LED2_START; i < IND_BACK_LED3_START; ++i) {
        OLEDCommandValue2 *data = (OLEDCommandValue2 *)
            (indVec_->GetData(i)->value);
        // Something wrong with the following line.
        data[0].led = (inds[i] == IND_LED2_ON) ? oledON : oledOFF;
        data[0].period = 1;
    }

    // LEDCommand3
    for (int i = IND_BACK_LED3_START; i < NUM_OF_INDICATORS; ++i) {
        OLEDCommandValue3 *data = (OLEDCommandValue3 *)
            (indVec_->GetData(i)->value);
        // Mode should be always oled3_MODE_UNDEF, refer to OPENR1.1.5b samples.
        data[0].mode = oled3_MODE_UNDEF;
        data[0].intensity = inds[i];
        data[0].period = 1;
    }

    sIndicators->SetData(indRegion);
    sIndicators->NotifyObservers();
    
}

void Indicators::SetUpBuffer(EffectorCommander &eCommander) {
	OPENR::NewCommandVectorData(NUM_OF_INDICATORS, &indVecID_, &indVec_);
	indVec_->SetNumData(NUM_OF_INDICATORS);

	indRegion = new RCRegion(indVec_->vectorInfo.memRegionID,
			indVec_->vectorInfo.offset,
			(void *) (indVec_),
			indVec_->vectorInfo.totalSize);

	// Sets up indicator primitives
	OPrimitiveID *indPrimID_[NUM_OF_INDICATORS];
	OPrimitiveID ledPrimitives[NUM_OF_INDICATOR_LEDS];

	for (int i = 0; i < NUM_OF_INDICATOR_JOINTS; ++i) {
		indPrimID_[i] = eCommander.getPrimID(indicatorJoints[i]);
	}

	// Opens the led primitives and then makes the indPrimID array points to them
	for (int i = NUM_OF_INDICATOR_JOINTS, j = 0;
			i < NUM_OF_INDICATORS;
			++i, ++j) {
		OPENR::OpenPrimitive((const char *) (INDICATORS[i]), &ledPrimitives[j]);
		indPrimID_[i] = &ledPrimitives[j];
	}

	// Mouth and Tail - JointCommand2
	for (int i = IND_MOUTH_START; i < IND_EAR_START; ++i) {
		OCommandInfo *info = indVec_->GetInfo(i);
		info->Set(odataJOINT_COMMAND2, *indPrimID_[i], NUM_OF_JOINT_IND_FRAMES);
	}

	// Ears - JointCommand4
	for (int i = IND_EAR_START; i < IND_FACE_LED3_START; ++i) {
		OCommandInfo *info = indVec_->GetInfo(i);
		info->Set(odataJOINT_COMMAND4, *indPrimID_[i], NUM_OF_JOINT_IND_FRAMES);
	}

	// LEDCommand3
	for (int i = IND_FACE_LED3_START; i < IND_LED2_START; ++i) {
		OCommandInfo *info = indVec_->GetInfo(i);
		info->Set(odataLED_COMMAND3, *indPrimID_[i], 1);
	}

	// LEDCommand2
	for (int i = IND_LED2_START; i < IND_BACK_LED3_START; ++i) {
		OCommandInfo *info = indVec_->GetInfo(i);
		info->Set(odataLED_COMMAND2, *indPrimID_[i], 1);
	}

	// LEDCommand3
	for (int i = IND_BACK_LED3_START; i < NUM_OF_INDICATORS; ++i) {
		OCommandInfo *info = indVec_->GetInfo(i);
		info->Set(odataLED_COMMAND3, *indPrimID_[i], 1);
	}
}

