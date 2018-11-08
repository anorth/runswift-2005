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
 * $Id: JointAgent.cc 6926 2005-06-26 06:24:58Z alexn $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "JointAgent.h"
#include "primitives.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <OPENR/RCRegion.h>

using namespace std;

static bool bEnableSoftHead = true;

ofstream jlogFile;

JointAgent::JointAgent(ActionPro * grandParent, OSubject *sEffector, EffectorCommander *eCommander){
	this->grandParent = grandParent;
	this->sEffector = sEffector;
	this->eCommander = eCommander;
    isHighGain = false;
    jointSpeedLimit = DEFAULT_JOINT_SPEED_LIMIT;

    for (int i=0;i<NUM_OF_EFFECTORS;i++)
        lastJointCommand.setJointValue(i,0); //start off with 0 position

	//open new joints.log
	jlogFile.open("/MS/OPEN-R/joints.log");
	if (! jlogFile.good()) {
		cout << "Log file cannot be created" << endl;
	}
	jlogFile.close();
}

void JointAgent::setUpBuffer(){
	/* Create a command vector data that holds OJointCommandValue2 for all joints. */
	for (int j=0; j<numOfBuffer; ++j) {
		OPENR::NewCommandVectorData(NUM_OF_EFFECTORS, &cmdVecID_[j], &cmdVec_[j]);
		cmdVec_[j]->SetNumData(NUM_OF_EFFECTORS);
		cmdRegion[j] = new RCRegion(cmdVec_[j]->vectorInfo.memRegionID,
				cmdVec_[j]->vectorInfo.offset,
				(void *)(cmdVec_[j]),
				cmdVec_[j]->vectorInfo.totalSize);
	}

	/* 
	 * Get joint primitve so that data can to be sent using OVRComm
	 * For the definition of Joint,  refer to RL2.h.
	 * Prepare a buffer where data to OVRComm is written.
	 * Set so that the joint pass is used for a Joint control
	 */
	OPrimitiveID *walkPrimID[NUM_OF_EFFECTORS];

	for (int i=0; i<NUM_OF_EFFECTORS; ++i) {
		walkPrimID[i] = eCommander->getPrimID(walkJoints[i]);
		OCommandInfo *info = cmdVec_[0]->GetInfo(i);
		info->Set(odataJOINT_COMMAND2, *walkPrimID[i], numOfFrames);
		info = cmdVec_[1]->GetInfo(i);
		info->Set(odataJOINT_COMMAND2, *walkPrimID[i], numOfFrames);
	}

}

/* Set head command to 0 frame in buffer since some walks do not use the head. */
//not be nessessary any more because executeCommand has taken into account this case
void JointAgent::setNotUsingHead(){
	if (freeBufID == -1) {
		cout << "No free buffer " << endl;
		return; //has no free buffer
	}
	for (int i=0; i<NUM_OF_HEAD_JOINTS; ++i) {
		OCommandInfo *info = cmdVec_[freeBufID]->GetInfo(i);
		info->numFrames = 0;
	}
}

bool JointAgent::checkBufferEmpty(){
    cout << "checking " ;
	for (int bufID=0; bufID<numOfBuffer; ++bufID) {
        cout << cmdRegion[bufID]->NumberOfReference() <<  "  " ;
		if (cmdRegion[bufID]->NumberOfReference() > 1){
			return false;
		}
	}
    cout << endl;
    return true;
}

bool JointAgent::isReadyForCommand(){
	freeBufID = -1; 
	for (int bufID=0; bufID<numOfBuffer; ++bufID) {
		if (cmdRegion[bufID]->NumberOfReference() == 1){
			freeBufID = bufID;
			return true;
		}
	}
	return false;
}

void JointAgent::executeLastJointCommand(JointCommand *jCommand){
	if (freeBufID == -1) {
		cout << "No free buffer " << endl;
		return; //has no free buffer
	}

    
	if ( badJointLimit(jCommand)){
		if (debugJointLimit) cout << "Hardware limit violated" << endl;
		clipBadJointLimit(jCommand);
	}

	//put in the memory
	OJointCommandValue2 *commandValue_[NUM_OF_EFFECTORS];
	for (int i=0; i<NUM_OF_EFFECTORS; ++i) {
		slongword t = jCommand->getJointValue(i);
		OCommandInfo *info = cmdVec_[freeBufID]->GetInfo(i);

		if (t == VERY_LARGE_SLONGWORD){ //the joints is not set at all
			info->numFrames = 0;//sothat the joints will stay the same
		}
		else{
			info->numFrames = 1;//only set one frame at a time
			OCommandData *data = cmdVec_[freeBufID]->GetData(i);
			commandValue_[i] = (OJointCommandValue2*)(data->value);
			commandValue_[i][0].value = t; //since only one frame value is set
		}
	}


	//notify observer
	sEffector->SetData(cmdRegion[freeBufID]);
	sEffector->NotifyObservers();
}

void JointAgent::updateSoftHeadJointCommand(JointCommand *newJointCommand){
    for (int i=0;i<3;i++){
        slongword oldValue = lastJointCommand.getJointValue(i);
        slongword newValue = newJointCommand->getJointValue(i);
        slongword change = newValue - oldValue;
        slongword sign = change<0?-1:1;
        slongword absChange = sign*change;
        if (absChange > jointSpeedLimit) {
        	// all speeds over jointSpeedLimit get that proportion of them over speed halved
	        change = (absChange - jointSpeedLimit)/2;
			if (change > jointSpeedLimit)	// with a max speed of 2*jointSpeedLimit
				change = jointSpeedLimit;
			change += jointSpeedLimit;
			change *= sign;
        }
        oldValue += change; //change is never over 2*jointSpeedLimit
        lastJointCommand.setJointValue(i, oldValue);
    }
}

void JointAgent::updateLastJointCommand(JointCommand *newJointCommand){
    for (int i=0;i<NUM_OF_EFFECTORS;i++){
        slongword oldValue = lastJointCommand.getJointValue(i);
        slongword newValue = newJointCommand->getJointValue(i);
        slongword change = newValue - oldValue;
        slongword sign = change<0?-1:1;
        slongword absChange = sign*change;
        if (absChange > jointSpeedLimit) {
        	// all speeds over jointSpeedLimit get that proportion of them over speed halved
	        change = (absChange - jointSpeedLimit)/2;
			if (change > jointSpeedLimit)	// with a max speed of 2*jointSpeedLimit
				change = jointSpeedLimit;
			change += jointSpeedLimit;
			change *= sign;
        }
        oldValue += change; //change is never over 2*jointSpeedLimit
        lastJointCommand.setJointValue(i, oldValue);
    }
}

void JointAgent::executeCommand(JointCommand *jCommand){

    lastJointCommand = *jCommand;
    if (bEnableSoftHead)
        updateSoftHeadJointCommand(jCommand);

    executeLastJointCommand(&lastJointCommand);
}

void JointAgent::executeSoftCommand(JointCommand *jCommand){
    updateLastJointCommand(jCommand);
    executeLastJointCommand(&lastJointCommand);
}

void JointAgent::logJointValues(RCRegion *cmdRegion){
	static int count = 0;
	OCommandVectorData *cmdVecData = (OCommandVectorData*) cmdRegion->Base();
	if (count % 100 == 0) cout << "Up to " << count << endl;
	jlogFile.open("/MS/OPEN-R/joints.log",ofstream::app);
	if (jlogFile.good()){
		jlogFile << count++ << "  " << setiosflags(ios_base::fixed) ;
		for (int i=0;i<NUM_OF_EFFECTORS;i++){
			OCommandData *data = cmdVecData->GetData(i);
			OJointCommandValue2 *command = (OJointCommandValue2 *)data;
			jlogFile << setprecision(2) << MICRO2DEG( command->value ) << " " ;
		}
		jlogFile << endl;
		jlogFile.close();
	}
	else{
		cout <<" Cannot open joints.log" << endl;
	}
}

void JointAgent::setJointGain ( int whichJoint, int P , int I , int D , bool highGain )
{
  eCommander->setJointGain ( whichJoint, P , I , D , highGain );
}

void JointAgent::relaxJoints(){
    eCommander->relaxJoints();
}

void JointAgent::reEnableJoints(){
    isHighGain = false;
    eCommander->reEnableJoints();
}

//speed is in degree
void JointAgent::setSpeedLimit(double speed){
    jointSpeedLimit = DEG2MICRO(speed);
    //cout << "JointAgent: speed limit is set to " << speed << endl;
}

void JointAgent::highGainJoints(){
    isHighGain = true;
    eCommander->highGainJoints();
}

bool JointAgent::isUsingHighGain(){
    return isHighGain;
}

