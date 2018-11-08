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
 * $Id: JointAgentMediator.cc 6985 2005-06-28 08:32:53Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "JointAgentMediator.h"
#include "legsGeometry.h"

#include <cmath>

#ifdef OFFLINE

#include <fstream>

void JointAgentMediator::setUpBuffer(){
    cout << "JointAgentMediator is setting up buffer" << endl;
}

static void writeToFile(const XYZ_Coord& flXYZ, const XYZ_Coord& frXYZ, const XYZ_Coord& blXYZ, const XYZ_Coord& brXYZ) {
  ofstream fout("locus.dat", ios::app);
  if (!fout || fout.bad()) {
    FFCOUT << "cannot open file: locus.dat" << endl;
    return;
  }
  fout << flXYZ.x << " " << flXYZ.y << " " << flXYZ.z << " "
       << frXYZ.x << " " << frXYZ.y << " " << frXYZ.z << " "
       << blXYZ.x << " " << blXYZ.y << " " << blXYZ.z << " "
       << brXYZ.x << " " << brXYZ.y << " " << brXYZ.z << " "
       << endl;
  fout.flush();
  fout.close();
}

void JointAgentMediator::executeCommand(JointCommand *jCommand){
  //cout << "JointAgentMediator is executing commands:" << *jCommand << endl;
  
  lastJointCommand = *jCommand;
  
  JOINT_Values flTheta = {lastJointCommand.getJointValueInRads(leftFrontJoint-1),
			  lastJointCommand.getJointValueInRads(leftFrontShoulder-1),
			  lastJointCommand.getJointValueInRads(leftFrontKnee-1)};
  XYZ_Coord flXYZ;
  kinematics(flTheta, tLeg, fbLegBase, flXYZ);
  
  JOINT_Values blTheta = {lastJointCommand.getJointValueInRads(leftRearJoint-1),
			  lastJointCommand.getJointValueInRads(leftRearShoulder-1),
			  lastJointCommand.getJointValueInRads(leftRearKnee-1)};
  XYZ_Coord blXYZ;
  kinematics(blTheta, tLeg, bbLegToesGnd, blXYZ);
  
  JOINT_Values frTheta = {lastJointCommand.getJointValueInRads(rightFrontJoint-1),
			  lastJointCommand.getJointValueInRads(rightFrontShoulder-1),
			  lastJointCommand.getJointValueInRads(rightFrontKnee-1)};
  XYZ_Coord frXYZ;
  kinematics(frTheta, tLeg, fbLegBase, frXYZ);
  
  JOINT_Values brTheta = {lastJointCommand.getJointValueInRads(rightRearJoint-1),
			  lastJointCommand.getJointValueInRads(rightRearShoulder-1),
			  lastJointCommand.getJointValueInRads(rightRearKnee-1)};
  XYZ_Coord brXYZ;
  kinematics(brTheta, tLeg, bbLegToesGnd, brXYZ);
  
  writeToFile(flXYZ, frXYZ, blXYZ, brXYZ);

}

void JointAgentMediator::relaxJoints(){
    cout << "JointAgentMediator is relaxing joints" << endl;
}

void JointAgentMediator::reEnableJoints(){
    cout << "JointAgentMediator is re-enabling joints" << endl;
}

void JointAgentMediator::relaxJoints(int whichJoint){
    cout << "JointAgentMediator is relaxing the " << whichJoint<< " joints" << endl;
}

void JointAgentMediator::reEnableJoints(int whichJoint){
    cout << "JointAgentMediator is re-enabling the " << whichJoint<< " joints" << endl;
}

bool JointAgentMediator::isReadyForCommand(){
    return true;
}

void JointAgentMediator::setSpeedLimit(double limit){
    cout << "JointAgentMediator is setting SpeedLimit " << limit  << endl;
}

void JointAgentMediator::setJointGain ( int whichJoint, int P , int I , int D , bool highGain){
    cout << "JointAgentMediator is setting joint gain " << whichJoint << " P=" << P << " I=" << I << " D=" << D << " highGain=" << highGain << endl;
}

void JointAgentMediator::highGainJoints(){
    cout << "JointAgentMediator is setting high gain joint " << endl;
}

void JointAgentMediator::executeSoftCommand(JointCommand *jCommand){
  //cout << "JointAgentMediator is executing soft commands: " << *jCommand << endl;
    
  static const slongword jointSpeedLimit = DEG2MICRO(2.0); // 2degree/8ms
  static const slongword jointSpeedHighLimit = DEG2MICRO(4.0);
  
  bool clipped = false;
  slongword maxSpeed = 0;
  
  for (int i=0;i<NUM_OF_EFFECTORS;i++){
    slongword oldValue = lastJointCommand.getJointValue(i);
    slongword newValue = jCommand->getJointValue(i);
    if (oldValue == VERY_LARGE_SLONGWORD)	// initialise to the first real angle we see
      oldValue = newValue;
    slongword change = newValue - oldValue;
    if (abs(change) > maxSpeed) {
      maxSpeed = abs(change);
    }
    if (abs(change) > jointSpeedLimit) {
      slongword sign = change<0?-1:1;
      cout << "limiting joint speed " << i << ".  Old speed " << MICRO2DEG(change);
      change = abs(change) - jointSpeedLimit;
      
      change /= 2;
      
      change += jointSpeedLimit;
      
      if (change > jointSpeedHighLimit)
	change = jointSpeedHighLimit;
      
      change *= sign;
      cout << ". New speed " << MICRO2DEG(change) << endl;
      clipped = true;
    }
    oldValue += change; //change is never over jointSpeedLimit
    lastJointCommand.setJointValue(i, oldValue);
  }
  
  if (maxSpeed > 0) {
    cout << "Max Joint delta: " << MICRO2DEG(maxSpeed) << endl;
  }
  
  if (clipped) {
    cout << "Clipped joint angles: " << lastJointCommand << endl;
  }
  
  JOINT_Values flTheta = {lastJointCommand.getJointValueInRads(leftFrontJoint-1),
			  lastJointCommand.getJointValueInRads(leftFrontShoulder-1),
			  lastJointCommand.getJointValueInRads(leftFrontKnee-1)};
  XYZ_Coord flXYZ;
  kinematics(flTheta, tLeg, fbLegPadCenter, flXYZ);
  
  JOINT_Values blTheta = {lastJointCommand.getJointValueInRads(leftRearJoint-1),
			  lastJointCommand.getJointValueInRads(leftRearShoulder-1),
			  lastJointCommand.getJointValueInRads(leftRearKnee-1)};
  XYZ_Coord blXYZ;
  kinematics(blTheta, tLeg, bbLegPad, blXYZ);
  
  JOINT_Values frTheta = {lastJointCommand.getJointValueInRads(rightFrontJoint-1),
			  lastJointCommand.getJointValueInRads(rightFrontShoulder-1),
			  lastJointCommand.getJointValueInRads(rightFrontKnee-1)};
  XYZ_Coord frXYZ;
  kinematics(frTheta, tLeg, fbLegPadCenter, frXYZ);
  
  JOINT_Values brTheta = {lastJointCommand.getJointValueInRads(rightRearJoint-1),
			  lastJointCommand.getJointValueInRads(rightRearShoulder-1),
			  lastJointCommand.getJointValueInRads(rightRearKnee-1)};
  XYZ_Coord brXYZ;
  kinematics(brTheta, tLeg, bbLegPad, brXYZ);
  
  /* cout << "Front left loc: " << flXYZ << endl;
     cout << "Front right loc: " << frXYZ << endl;
     cout << "Back left loc: " << blXYZ << endl;
     cout << "Back right loc: " << brXYZ << endl; */

  writeToFile(flXYZ, frXYZ, blXYZ, brXYZ);

}

bool JointAgentMediator::isUsingHighGain() {
	return false;
}

#else

#endif //OFFLINE
