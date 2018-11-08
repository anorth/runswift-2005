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
 * $Id: ActionPro.cc 7169 2005-07-03 14:49:31Z weiming $
 *
 * Copyright(c) 2004 UNSW
 * All Rights Reserved.
 * 
 **/

#include "ActionPro.h"

#ifdef OFFLINE
bool bSendDebugJointValue = true;
#else
bool bSendDebugJointValue = false;
#endif //OFFLINE
bool bUseSoftCommand = false; // if true, use joint speed limits
static const bool bDebugCommand = false;
static const bool bDebugActuatorLoop = false;

ActionPro::ActionPro() {
#ifndef OFFLINE
    eCommander = new EffectorCommander();
#endif //OFFLINE

    takenFirstStep = false;
    leading_leg = NULL;
    currentWalkType = NULL;
    currentMinorWalkType = NULL;
    currentStep = NULL;
    currentPG = NULL;
    frontShoulderHeight = NULL;
    bodyTilt = NULL;
    desiredPan = NULL;
    desiredTilt = NULL;
    desiredCrane = NULL;
    isHighGain = NULL;

    headAgent = NULL;
    pAgent = NULL;
    jointAgent = NULL;
    jointRelaxed = false;
    nextFreeDataLoc = 0;
    
    for (int i=0; i<NUM_SENSORS; i++) {
    	sensorVal[i] = 0;
    	dutyCycleValue[i] = 0;
    }

    // init the ActuatorWirelessDataEntry for peak value storage
    awdPeak.type = actSensorPeakT;
    awdPeak.p.reset();
}

void ActionPro::initMem() {
  initMACAddress();
    int sizeOfNeededRegion = sizeof(ActuatorSharedMemory);     //for stepComplete only needed in push interface
    shmPWalk = new RCRegion(sizeOfNeededRegion);

    ActuatorSharedMemory *ptr =(ActuatorSharedMemory*)shmPWalk->Base();

    if (ptr == NULL) {
        *(char*)NULL = 5;    // crash
    }

    cout << "ActuatorControl shared memory: " <<(int)ptr << endl;

    leading_leg = &(ptr->leading_leg);
    currentWalkType = &(ptr->currentWalkType);
    currentMinorWalkType = &(ptr->currentMinorWalkType);
    currentStep = &(ptr->currentStep);
    currentPG = &(ptr->currentPG);
    frontShoulderHeight = &(ptr->frontShoulderHeight);
    bodyTilt = &(ptr->bodyTilt);
    desiredPan = &(ptr->desiredPan);
    desiredTilt = &(ptr->desiredTilt);
    desiredCrane = &(ptr->desiredCrane);
    isHighGain = &(ptr->isHighGain);
    stepID = &(ptr->stepID);

    //initialise to avoid crashing caused by accessing random memory
    *leading_leg = 0;
    *currentWalkType = false;
    *currentMinorWalkType = false;
    *frontShoulderHeight = 95;  // Default value (cm)
    *bodyTilt = 0.23287;        // Default value (radians) (=13.34 degrees)
    *desiredPan = 0;
    *desiredTilt = 0;
    *desiredCrane = 0;
    *isHighGain = false;
    *stepID = 0;

    pAgent = new PWalk(sensorVal, dutyCycleValue);
    awdPeak.p.stepID = pAgent->stepID;

#ifdef OFFLINE
    headAgent = new HeadAgentMediator();
    jointAgent = new JointAgentMediator();
#else
    headAgent = new HeadAgentMediator(desiredPan, desiredTilt, desiredCrane,
		    pAgent->ptr_frontShoulderHeight, pAgent->ptr_bodyTilt);
    jointAgent = new JointAgentMediator(this, sEffector, eCommander);
    eCommander->start();
#endif //OFFLINE
    jointAgent->setUpBuffer();


    /*
    *shareStep_  = *(pAgent->ptr_step_);
    *currentPG = *(pAgent->ptr_PG);
    */

}

void ActionPro::SetPAgent(OSubject *sbjShmPWalk) 
{
  sbjShmPWalk = sbjShmPWalk; // get rid of warning
#ifndef OFFLINE
	MemoryRegionID mem = shmPWalk->MemID();
	size_t offset = shmPWalk->Offset();
	char* base = shmPWalk->Base();
	size_t size = shmPWalk->Size(); 

	sbjShmPWalk->SetData(&mem,    sizeof(mem));
	sbjShmPWalk->SetData(&offset, sizeof(offset));
	sbjShmPWalk->SetData(&base,   sizeof(base));
	sbjShmPWalk->SetData(&size,   sizeof(size));
	sbjShmPWalk->NotifyObservers();
#endif //OFFLINE
}

#ifdef OFFLINE
void ActionPro::execute(const AtomicAction& atomicAction) {
    next = atomicAction;
#else
void ActionPro::execute(const ONotifyEvent& event) {
  next = (AtomicAction)*(const AtomicAction *)event.Data(0);
#endif //OFFLINE

	//*shareStep_ = *(pAgent->ptr_step_);
	//*currentPG = *(pAgent->ptr_PG);

    if (bDebugCommand){
      cout << "executing :" << next.cmd << " " << next.walkType << endl;
    }

    switch (next.cmd){
        case aaRelaxed:
            relaxJoints();
            break;
        case aaTurnOnJointDebug:
            bSendDebugJointValue = ! bSendDebugJointValue;
            cout << "Send debug " << ( (bSendDebugJointValue)?"on":"off") << endl;
            break;
        case aaParaWalk:
        case aaForceStepComplete:
            pAgent->pStep(next);
            break;
        // this doesn't get set anywhere...
        case aaSetJointSpeedLimit:
            jointAgent->setSpeedLimit(next.forwardSpeed/1000.0);
            break;
        case aaSetAJointGain:
            jointAgent->setJointGain(next.walkType, (int) next.panx, 
                    (int) next.tilty,(int)  next.cranez,false);
            break;
        case aaSetHighGain:
            jointAgent->highGainJoints();
            break;
        case aaSetLowGain:
            jointAgent->reEnableJoints ();
            break;
        case aaReloadSpecialAction:
            pAgent->reloadAllSpecialAction();
            break;
        default:
            cout << "Warning : unknown aaCommand" << endl;
            break;
    }
    takenFirstStep = true;
    
    if (pAgent->NotUsingHead() && takenFirstStep) { 
      //TODO: add check sHead->isReady in HeadAgent
      //cout << "ActionPro::execute| panx: " << next.panx << " tilty: " << next.tilty << 
      //    " cranez: " << next.cranez << endl;
      headAgent->SendHeadCommand(next.headType, next.panx, next.tilty, next.cranez);
      //actionSuccess |= execHEAD;
    }    

}

void ActionPro::relaxJoints()
{
    if (!jointRelaxed){
        cout << "Relaxing joints" << endl;
        jointAgent->relaxJoints();
        jointRelaxed = true;
    }
    else{
        cout << "Re-enabling joints" << endl;
        jointAgent->reEnableJoints();
        jointRelaxed = false;
    }
}

void ActionPro::updateSharedInfo(){
    *currentWalkType = *(pAgent->ptr_walktype);
    *currentMinorWalkType = *(pAgent->ptr_minorWalkType);
    *currentStep = *(pAgent->ptr_step);
    *currentPG  = *(pAgent->ptr_PG);
    //*stepComplete = pAgent->isCompleted();
    *frontShoulderHeight = *(pAgent->ptr_frontShoulderHeight);
    *bodyTilt = *(pAgent->ptr_bodyTilt);
    *stepID = pAgent->stepID;
    *isHighGain = jointAgent->isUsingHighGain();
}

void ActionPro::ContinueStep() {
  if (bDebugActuatorLoop){
    cout << "ContinueStep " << endl;
  }

  if (!jointAgent->isReadyForCommand()) {
    cout << "Warning: no buffer found" << endl;
    return;
  }
  
  pAgent->pStep(jointAgent->isUsingHighGain()); //continue step
  updateSharedInfo();
  if (pAgent->NotUsingHead() ){
    headAgent->setCurrentJointCommand(pAgent->jointCommand);
  }

#ifndef OFFLINE    
    // for special actions disable the speed limit. Have to do it this way
    // so soft head is ignored as well. WARNING: joint speed limits
    // disabled for special actions and pos files
    if (pAgent->sp->isSpecialAction()) {
        jointAgent->setSpeedLimit(10 * DEFAULT_JOINT_SPEED_LIMIT);
    }
#endif

    if (bUseSoftCommand)
      jointAgent->executeSoftCommand(&(pAgent->jointCommand));
    else
      jointAgent->executeCommand(&(pAgent->jointCommand));
    
#ifndef OFFLINE
    // re-enable joint speed limits
    if (pAgent->sp->isSpecialAction()) {
      jointAgent->setSpeedLimit(DEFAULT_JOINT_SPEED_LIMIT);
    }
#endif

#ifdef OFFLINE
    setSensorVals(jointAgent->lastJointCommand);
    storeSensorValueToSend(0);
    sendWirelessMessage(NULL);
#endif

    //storeCommandValueToSend(jointAgent->lastJointCommand);

}


// for offline debugging use
void ActionPro::setSensorVals(const JointCommand &joints) {
	sensorVal[ssHEAD_TILT] = joints.getJointEnumValue(headTilt);
	sensorVal[ssHEAD_PAN] = joints.getJointEnumValue(headPan);
	sensorVal[ssHEAD_CRANE] = joints.getJointEnumValue(headCrane);

	sensorVal[ssFR_ROTATOR] = joints.getJointEnumValue(rightFrontJoint);
	sensorVal[ssFR_ABDUCTOR] = joints.getJointEnumValue(rightFrontShoulder);
	sensorVal[ssFR_KNEE] = joints.getJointEnumValue(rightFrontKnee);

	sensorVal[ssFL_ROTATOR] = joints.getJointEnumValue(leftFrontJoint);
	sensorVal[ssFL_ABDUCTOR] = joints.getJointEnumValue(leftFrontShoulder);
	sensorVal[ssFL_KNEE] = joints.getJointEnumValue(leftFrontKnee);

	sensorVal[ssRR_ROTATOR] = joints.getJointEnumValue(rightRearJoint);
	sensorVal[ssRR_ABDUCTOR] = joints.getJointEnumValue(rightRearShoulder);
	sensorVal[ssRR_KNEE] = joints.getJointEnumValue(rightRearKnee);

	sensorVal[ssRL_ROTATOR] = joints.getJointEnumValue(leftRearJoint);
	sensorVal[ssRL_ABDUCTOR] = joints.getJointEnumValue(leftRearShoulder);
	sensorVal[ssRL_KNEE] = joints.getJointEnumValue(leftRearKnee);
}


//2 usages: sendWirelessMessage(NULL) send the default AWD object 
//          sendWirelessMessage(data) send whatever data we want
bool ActionPro::sendWirelessMessage(ActuatorWirelessDataEntry *data){
  int count  = 1;	
  if (data == NULL) {
    data = &(awd[0]);
    count = nextFreeDataLoc;
    if (count == 0) {
      return true;
    }
    nextFreeDataLoc = 0;
    //return true; // hack to debug max/min pwm only
  }
    
  //note that when the dog is running, actuator gets alot of changing commands
#ifndef OFFLINE
  int size = sizeof(ActuatorWirelessDataEntry) * count;
  
  sWirelessMessenger->ClearBuffer();
  sWirelessMessenger->SetData(&count, sizeof(int));
  sWirelessMessenger->SetData(data, size);
  sWirelessMessenger->NotifyObservers();
#endif //OFFLINE  
  return true; //should have checked whether the subject is ready or not
}


 void ActionPro::storeSensorValueToSend(long frameID) {
   if (!bSendDebugJointValue)
     return;
   
   int loc = nextFreeDataLoc++;
   
   if (loc >= NUM_AWD)
     return;


   awd[loc].type = actSensorDataT;   
   awd[loc].s.frameID = awdPeak.p.frameID = frameID;   
   
   if (awdPeak.p.stepID != pAgent->stepID) {
     if (!pAgent->sp->isSpecialAction())// && !pAgent->isCommandChanged)
       sendWirelessMessage(&awdPeak);
     awdPeak.p.reset();     
     awdPeak.p.stepID = pAgent->stepID;
   }
   
   awd[loc].s.data[asAccelFor]  = awdPeak.p.checkMinMax(asAccelFor,  sensorVal[ssACCEL_FOR]);
   awd[loc].s.data[asAccelSide] = awdPeak.p.checkMinMax(asAccelSide, sensorVal[ssACCEL_SIDE]);
   awd[loc].s.data[asAccelZ]    = awdPeak.p.checkMinMax(asAccelZ,    sensorVal[ssACCEL_Z]);   

   awd[loc].s.data[asHeadTilt]  = awdPeak.p.checkMinMax(asHeadTilt,  sensorVal[ssHEAD_TILT]);
   awd[loc].s.data[asHeadPan]   = awdPeak.p.checkMinMax(asHeadPan,   sensorVal[ssHEAD_PAN]);
   awd[loc].s.data[asHeadCrane] = awdPeak.p.checkMinMax(asHeadCrane, sensorVal[ssHEAD_CRANE]);
   
   awd[loc].s.data[asFrontLeftJoint]     = awdPeak.p.checkMinMax(asFrontLeftJoint    , sensorVal[ssFL_ROTATOR]);
   awd[loc].s.data[asFrontLeftShoulder]  = awdPeak.p.checkMinMax(asFrontLeftShoulder , sensorVal[ssFL_ABDUCTOR]);
   awd[loc].s.data[asFrontLeftKnee]      = awdPeak.p.checkMinMax(asFrontLeftKnee     , sensorVal[ssFL_KNEE]);
    
   awd[loc].s.data[asFrontRightJoint]    = awdPeak.p.checkMinMax(asFrontRightJoint   , sensorVal[ssFR_ROTATOR]);
   awd[loc].s.data[asFrontRightShoulder] = awdPeak.p.checkMinMax(asFrontRightShoulder, sensorVal[ssFR_ABDUCTOR]);
   awd[loc].s.data[asFrontRightKnee]     = awdPeak.p.checkMinMax(asFrontRightKnee    , sensorVal[ssFR_KNEE]);

   awd[loc].s.data[asRearLeftJoint]      = awdPeak.p.checkMinMax(asRearLeftJoint     , sensorVal[ssRL_ROTATOR]);
   awd[loc].s.data[asRearLeftShoulder]   = awdPeak.p.checkMinMax(asRearLeftShoulder  , sensorVal[ssRL_ABDUCTOR]);
   awd[loc].s.data[asRearLeftKnee]       = awdPeak.p.checkMinMax(asRearLeftKnee      , sensorVal[ssRL_KNEE]);

   awd[loc].s.data[asRearRightJoint]     = awdPeak.p.checkMinMax(asRearRightJoint    , sensorVal[ssRR_ROTATOR]);
   awd[loc].s.data[asRearRightShoulder]  = awdPeak.p.checkMinMax(asRearRightShoulder , sensorVal[ssRR_ABDUCTOR]);
   awd[loc].s.data[asRearRightKnee]      = awdPeak.p.checkMinMax(asRearRightKnee     , sensorVal[ssRR_KNEE]);

   awd[loc].s.data[asPWMHeadTilt]  = awdPeak.p.checkMinMax(asPWMHeadTilt , dutyCycleValue[ssHEAD_TILT]);
   awd[loc].s.data[asPWMHeadPan]   = awdPeak.p.checkMinMax(asPWMHeadPan  , dutyCycleValue[ssHEAD_PAN]);
   awd[loc].s.data[asPWMHeadCrane] = awdPeak.p.checkMinMax(asPWMHeadCrane, dutyCycleValue[ssHEAD_CRANE]);
   
   awd[loc].s.data[asPWMFrontLeftJoint]     = awdPeak.p.checkMinMax(asPWMFrontLeftJoint    , dutyCycleValue[ssFL_ROTATOR]);
   awd[loc].s.data[asPWMFrontLeftShoulder]  = awdPeak.p.checkMinMax(asPWMFrontLeftShoulder , dutyCycleValue[ssFL_ABDUCTOR]);
   awd[loc].s.data[asPWMFrontLeftKnee]      = awdPeak.p.checkMinMax(asPWMFrontLeftKnee     , dutyCycleValue[ssFL_KNEE]);
   
   awd[loc].s.data[asPWMFrontRightJoint]    = awdPeak.p.checkMinMax(asPWMFrontRightJoint   , dutyCycleValue[ssFR_ROTATOR]);
   awd[loc].s.data[asPWMFrontRightShoulder] = awdPeak.p.checkMinMax(asPWMFrontRightShoulder, dutyCycleValue[ssFR_ABDUCTOR]);
   awd[loc].s.data[asPWMFrontRightKnee]     = awdPeak.p.checkMinMax(asPWMFrontRightKnee    , dutyCycleValue[ssFR_KNEE]);
   
   awd[loc].s.data[asPWMRearLeftJoint]     = awdPeak.p.checkMinMax(asPWMRearLeftJoint      , dutyCycleValue[ssRL_ROTATOR]);
   awd[loc].s.data[asPWMRearLeftShoulder]  = awdPeak.p.checkMinMax(asPWMRearLeftShoulder   , dutyCycleValue[ssRL_ABDUCTOR]);
   awd[loc].s.data[asPWMRearLeftKnee]      = awdPeak.p.checkMinMax(asPWMRearLeftKnee       , dutyCycleValue[ssRL_KNEE]);
   
   awd[loc].s.data[asPWMRearRightJoint]    = awdPeak.p.checkMinMax(asPWMRearRightJoint     , dutyCycleValue[ssRR_ROTATOR]);
   awd[loc].s.data[asPWMRearRightShoulder] = awdPeak.p.checkMinMax(asPWMRearRightShoulder  , dutyCycleValue[ssRR_ABDUCTOR]);
   awd[loc].s.data[asPWMRearRightKnee]     = awdPeak.p.checkMinMax(asPWMRearRightKnee      , dutyCycleValue[ssRR_KNEE]);    
}

void ActionPro::storeCommandValueToSend(JointCommand &jCommand){

	if (!bSendDebugJointValue)
		return;

	int loc = nextFreeDataLoc++;

	if (loc >= NUM_AWD)
		return;

	awd[loc].type = actCommandDataT;

	awd[loc].c.lastSensedFrameID = sensorFrameID;

    awd[loc].c.cmdHeadTilt = jCommand.getJointEnumValue(headTilt);
    awd[loc].c.cmdHeadPan = jCommand.getJointEnumValue(headPan);
    awd[loc].c.cmdHeadCrane = jCommand.getJointEnumValue(headCrane);

    awd[loc].c.cmdFrontLeftJoint = jCommand.getJointEnumValue(leftFrontJoint);
    awd[loc].c.cmdFrontLeftShoulder = jCommand.getJointEnumValue(leftFrontShoulder);
    awd[loc].c.cmdFrontLeftKnee = jCommand.getJointEnumValue(leftFrontKnee);

    awd[loc].c.cmdFrontRightJoint = jCommand.getJointEnumValue(rightFrontJoint);
    awd[loc].c.cmdFrontRightShoulder = jCommand.getJointEnumValue(rightFrontShoulder);
    awd[loc].c.cmdFrontRightKnee = jCommand.getJointEnumValue(rightFrontKnee);

    awd[loc].c.cmdRearLeftJoint = jCommand.getJointEnumValue(leftRearJoint);
    awd[loc].c.cmdRearLeftShoulder = jCommand.getJointEnumValue(leftRearShoulder);
    awd[loc].c.cmdRearLeftKnee = jCommand.getJointEnumValue(leftRearKnee);

    awd[loc].c.cmdRearRightJoint = jCommand.getJointEnumValue(rightRearJoint);
    awd[loc].c.cmdRearRightShoulder = jCommand.getJointEnumValue(rightRearShoulder);
    awd[loc].c.cmdRearRightKnee = jCommand.getJointEnumValue(rightRearKnee);
}


/*
   static bool isHighGain = false;

   void dynamicGian (const ActomicAction& next) {

   OPowerStatus currentStatus;
   OPENR::GetPowerStatus(&currentStatus);

// Turn on low gain if the current is high and/or movement is not straight

//if ( currentStatus.current < currentGainSwitch && !isHighGain ) 
//  return;

if ( currentStatus.current < currentGainSwitch && isHighGain ) {
isHighGain = false;
for ( int i = 4 ; i < numOfJoints ; i++ ) {
setJointGain ( i , 1000 , 1000, 1000, false);
}
return;
}

if ( next.left != 0 && isHighGain )

// Turn on high gain if the current is low and no left and small turn

if ( currentStatus.curent >= -2000.0 && isHighGain ) {
return;
} 
}
 */


