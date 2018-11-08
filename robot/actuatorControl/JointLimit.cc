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
 * $Id: JointLimit.cc 4364 2004-09-22 07:04:12Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "JointLimit.h"
#include "../share/Common.h"
#include <iostream>

using namespace std;

//style: why did I put this here? when i put it in SpecialAction.h, 
//gcc complainted about unused variables !!!
const char* JOINT_NAME[NUM_OF_EFFECTORS + 1] = {
	"nothing",
	"headTilt",
	"headPan",
	"headCrane",
	"rightFrontJoint",
	"rightFrontShoulder",
	"rightFrontKnee",
	"leftFrontJoint",
	"leftFrontShoulder",
	"leftFrontKnee",
	"rightRearJoint",
	"rightRearShoulder",
	"rightRearKnee",
	"leftRearJoint",
	"leftRearShoulder",
	"leftRearKnee"
};


bool checkRange(slongword value, double minDeg, double maxDeg){
    if (value == VERY_LARGE_SLONGWORD) return true; //ignore INFINITY
    else {
        double deg = MICRO2DEG(value);
        return deg >= minDeg && deg <= maxDeg;
    }
}

bool checkJoint(JointEnum joint, slongword value){
	bool isGood = checkRange(value, LOWER_JOINT_LIMIT[joint], UPPER_JOINT_LIMIT[joint]);
    if (! isGood && debugJointLimit) cout << "!!! " << JOINT_NAME[joint] << " out of range " << MICRO2DEG(value) << endl;
    return isGood;
}

#ifndef OFFLINE
//below are overloading functions that do the checks on OCommandData 
bool checkJoint(JointEnum joint, int nframes, OCommandData *data){
	OJointCommandValue2 *command = (OJointCommandValue2 *)data;
	for (int i=0;i<nframes;i++, command++){ 
		if (! checkJoint(joint, command->value)) {
			return false;
		}
	}
	return true;
}
#endif //OFFLINE

void clipRange(slongword &value, double minDeg, double maxDeg){
    if (value != VERY_LARGE_SLONGWORD){ //VERY_LARGE_SLONGWORD means "no set"
        double deg = MICRO2DEG(value);
        if (deg < minDeg) deg = minDeg;
        if (deg > maxDeg) deg = maxDeg;
        value = DEG2MICRO(deg);
    }
}

void clipJointValue(JointEnum joint, slongword &value){
	clipRange(value, LOWER_JOINT_LIMIT[joint], UPPER_JOINT_LIMIT[joint]);
}

#ifndef OFFLINE
void clipJointValue(JointEnum joint, int nframes, OCommandData *data){
	OJointCommandValue2 *command = (OJointCommandValue2 *)data;
	for (int i=0;i<nframes;i++, command++){ 
		clipJointValue(joint,command->value);
	}
}
#endif //OFFLINE

JointEnum badJointLimit(JointCommand *jCommand){
    for (int i=0;i < NUM_OF_EFFECTORS ; i++){
        if ( !checkJoint( (JointEnum) (i+1), jCommand->getJointValue(i) ) )
                return (JointEnum) (i+1);
    }
	return nothing; //JointEnum type
}

#ifndef OFFLINE
JointEnum badJointLimit(RCRegion *cmdRegion){
	OCommandVectorData *cmdVecData = (OCommandVectorData*) cmdRegion->Base();
    for (int i=0;i < NUM_OF_EFFECTORS ; i++){
        if ( !checkJoint( (JointEnum) (i+1), cmdVecData->GetInfo(i)->numFrames, cmdVecData->GetData(i))) 
            return (JointEnum) (i+1);
    }
	return nothing; //JointEnum type
}
#endif //OFFLINE


void clipBadJointLimit(JointCommand *jCommand){
    for (int i=0;i < NUM_OF_EFFECTORS ; i++){
        slongword temp;
        temp = jCommand->getJointValue(i);
        clipJointValue( (JointEnum)(i+1), temp);
        jCommand->setJointValue(i,temp);
    }
}


#ifndef OFFLINE
void clipBadJointLimit(RCRegion *cmdRegion){
	OCommandVectorData *cmdVecData = (OCommandVectorData*) cmdRegion->Base();
    for (int i=0;i<NUM_OF_EFFECTORS;i++)
        clipJointValue((JointEnum)(i+1), cmdVecData->GetInfo(i)->numFrames, cmdVecData->GetData(i));
}
#endif //OFFLINE


