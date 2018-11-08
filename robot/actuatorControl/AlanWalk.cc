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
 * $Id: AlanWalk.cc 4364 2004-09-22 07:04:12Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "AlanWalk.h"
#include "legsGeometry.h"

//utility functions
bool isFrontRightJointIndex(int index);
bool isRearLeftJointIndex(int index);


//AlanWalk member functions
void AlanWalk::setCurrentJointCommand(JointCommand &jointCommand){
    SpecialAction::setCurrentJointCommand(jointCommand);
}

void AlanWalk::linearInterpolatePoint(JointCommand &corner1, JointCommand &corner2, JointCommand &output, int k, int total){
    for (int i=0;i<NUM_OF_EFFECTORS;i++){
        slongword value1 = corner1.getJointValue(i);
        slongword value2 = corner2.getJointValue(i);
        output.setJointValue(i, value1 + (value2-value1) * k / total); //WARNING: integer division
        //cout << k << " / " << total << " -> v1 = " << value1 << " v2 = " << value2 << " out= " << output.getJointValue(i) << endl;
    }
}

void AlanWalk::makeLocusPoint(JointCommand *cornerJoints, int PGR, int PGH, vector<JointCommand> &locusPoints){
    for (int i=0;i<PGH;i++){
        linearInterpolatePoint(cornerJoints[0],cornerJoints[1],locusPoints[i], i , PGH);
    }
    for (int i=PGH;i<PGH+PGR;i++){
        linearInterpolatePoint(cornerJoints[1],cornerJoints[2],locusPoints[i], i-PGH , PGR);
    }
    for (int i=PGH+PGR;i<PGH+PGR+PGH;i++){
        linearInterpolatePoint(cornerJoints[2],cornerJoints[3],locusPoints[i], i-PGH-PGR , PGH);
    }
    for (int i=PGH+PGR+PGH;i<2*PG;i++){ //PGH+PGR+PGH == PG
        linearInterpolatePoint(cornerJoints[3],cornerJoints[0],locusPoints[i], i-PGH-PGR-PGH , PGH+PGR+PGH);
    }
}

void AlanWalk::reverseAndRotateLocusPoints(vector<JointCommand> &locusPoints, int PG){
    int len = locusPoints.size();
    vector<JointCommand> temp(len);
    if (len != 2*PG ){
        cout << "Warning: 2*PG != locusPoints.size() " << endl;
    }
    //reverse:
    for (int i=0;i < len;i++){
        temp[i] = locusPoints[len-1 -i];
    }
    //rotate:
    for (int i=0;i<len;i++){
        locusPoints[i] = temp[(i-PG/2+ len ) % len];
    }
}

void AlanWalk::composeTheMove(){
    
	/* theta4 = angle of robot body, relative to ground */
	double theta1, theta2, theta3, theta4;
	double f, s, h, x, y, z;
    JointCommand cornerJoints[4];

	theta4 = asin((hb - hf) / lsh);
    for (int i=0;i<4;i++){
        getFLCornerPoint(i+1, f,s,h);
        fsh2xyz(f,s,h,theta4,x,y,z);
        frontJointAngles(x,y,z,theta1,theta2,theta3);
        jfl1 = theta1;
        jfl2 = theta2;
        jfl3 = theta3;


        getFRCornerPoint(i+1, f,s,h);
        fsh2xyz(f,s,h,theta4,x,y,z);
        frontJointAngles(x,y,z,theta1,theta2,theta3);
        jfr1 = theta1;
        jfr2 = theta2;
        jfr3 = theta3;

        getBLCornerPoint(i+1, f,s,h);
        fsh2xyz(f,s,h,theta4,x,y,z);
        backJointAngles(x,y,z,theta1,theta2,theta3);
        jbl1 = theta1;
        jbl2 = theta2;
        jbl3 = theta3;


        getBRCornerPoint(i+1, f,s,h);
        fsh2xyz(f,s,h,theta4,x,y,z);
        backJointAngles(x,y,z,theta1,theta2,theta3);
        jbr1 = theta1;
        jbr2 = theta2;
        jbr3 = theta3;


        cornerJoints[i].setJointEnumValue(rightFrontJoint, RAD2MICRO(jfr2) );
        cornerJoints[i].setJointEnumValue(rightFrontShoulder, RAD2MICRO(jfr1) ); 
        cornerJoints[i].setJointEnumValue(rightFrontKnee, RAD2MICRO(jfr3) ); 
        cornerJoints[i].setJointEnumValue(leftFrontJoint, RAD2MICRO(jfl2) ); 
        cornerJoints[i].setJointEnumValue(leftFrontShoulder, RAD2MICRO(jfl1) ); 
        cornerJoints[i].setJointEnumValue(leftFrontKnee, RAD2MICRO(jfl3) ); 
        cornerJoints[i].setJointEnumValue(rightRearJoint, RAD2MICRO(jbr2) ); 
        cornerJoints[i].setJointEnumValue(rightRearShoulder, RAD2MICRO(jbr1) ); 
        cornerJoints[i].setJointEnumValue(rightRearKnee, RAD2MICRO(jbr3) ); 
        cornerJoints[i].setJointEnumValue(leftRearJoint, RAD2MICRO(jbl2) ); 
        cornerJoints[i].setJointEnumValue(leftRearShoulder, RAD2MICRO(jbl1) ); 
        cornerJoints[i].setJointEnumValue(leftRearKnee, RAD2MICRO(jbl3) ); 

        //cout << "Corner " << i << " : " <<  cornerJoints[i] << endl;
    }
    
    
    //cout << " PG = " << PG << endl;
    vector<JointCommand> locusPoints(2*PG); 

    double fstart, fend, sstart, send;
    int PGH, PGR;

    getFLStartEnd(fstart, sstart, fend, send);
    getFLPG(fstart, sstart, fend, send, PGR, PGH);

    cout << " PGR = " << PGR << " PGH = " << PGH << " PG = " << PG << endl;

    makeLocusPoint(cornerJoints, PGR, PGH,locusPoints);
    reverseAndRotateLocusPoints(locusPoints,PG);
    
    //make posa array
    if (posa) delete(posa);
    numOfSteps = duration = 2*PG;
    //cout << "PG = " << PG << "  numOfSteps = " << numOfSteps << endl;
    posa = (double *)new double[numOfSteps * posaLength];
    for (int i=0;i<numOfSteps;i++){
        posa[i*posaLength + 0] = i + 1;
        		posa[i*posaLength + 0] = i + 1; //one frame at a step

		for (int j=headTilt;j<=headCrane;j++) posa[i*posaLength+j] = VERY_LARGE_SLONGWORD;//meaning we don't use it at all

        for (int k=rightFrontJoint;k<=leftRearKnee;k++){
            int jindex = i;
            if ( isFrontRightJointIndex(k) || isRearLeftJointIndex(k) ){
                jindex = (i + PG) % numOfSteps;
            }
            posa[i * posaLength + k] = MICRO2DEG(locusPoints[jindex].getJointEnumValue((JointEnum)k)) ;
        }
	}
}

void AlanWalk::setParameters(const AtomicAction &command){
    QuadWalk::setParameters(command);
    composeTheMove();
}

void AlanWalk::calibrate(double f, double l, double t){
    NormalWalk::calibrate(f,l,t);
}

bool isFrontRightJointIndex(int index){
    return index >= rightFrontJoint && index <= rightFrontKnee;
}

bool isRearLeftJointIndex(int index){
    return index >= leftRearJoint && index <= leftRearKnee;
}


