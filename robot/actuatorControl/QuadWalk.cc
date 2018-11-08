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
 * $Id: QuadWalk.cc 6607 2005-06-14 08:01:35Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "QuadWalk.h"
#include "legsGeometry.h"
#include "../share/Common.h"

static const bool bDebugStepTrim = false;

#ifdef OFFLINE
static const bool bDebugInverseKinematics = true;
#else
static const bool bDebugInverseKinematics = false;
#endif

QuadWalk::QuadWalk() {

    useBinarySearchCalibration = true;
	walktype = -1;
	forward = left = turnCCW = 0;

	/* Forward, sideways and turn components. */
	fc = 0;
	sc = 0;
	tc = 0; /* All zero, correspond to no movement. */

	/* The following are the locomotion parameters, a brief description is given for each, refer to 
	   Chapter 5 in 2002 thesis for detailed descriptions. */ 

	/* Period Ground (PG) is the time to complete a half-step. */ 
	PG    = 40; 
	/* Time to complete a full step. */
	step_ = 2 * PG;

	/* Height Forward (hf) is the vertical height in mm from the shoulder joint to the ground. */
	hf    = 70;
	/* Height Back (hb) is the vertical height in mm from the hip joint to the ground. */
	hb    = 110;
	/* Height Delta Forward (hdf) is the height in mm of the walk locus for the front paws. */
	hdf   = 5;
	/* Height Delta Back (hdb) is the height in mm of the walk locus for the back paws. */
	hdb   = 25;

	/* Hmmm... */
	hfG   = hf;
	hbG   = hb;

	/* Distance in mm from shoulder ground to the home position of the front paws in the f-axis. */
	ffo   = 55;
	/* Distance in mm from shoulder gounrd to the home position of the front paws in the s-axis. */
	fso   = 15;
	/* Distance in mm from hip ground to the home position of the back paws in the f-axis. */
	bfo   = -55;
	/* Distance in mm from hip ground to the home position of the back paws in the s-axis. */
	bso   = 15;

	canterHeight = 5.0;

	/* Obtain robot length constants, note that sin5 and cos5 are provided, do not recalculate them. */
	theta5 = atan(134.4 / lsh);
	sin5   = sin(theta5);
	cos5   = cos(theta5);

	/* Initialize all joint angles. */
	jfl1 = 0;
	jfl2 = 0;
	jfl3 = 0;
	jfr1 = 0;
	jfr2 = 0;
	jfr3 = 0;
	jbl1 = 0;
	jbl2 = 0;
	jbl3 = 0;
	jbr1 = 0;
	jbr2 = 0;
	jbr3 = 0;

	alpha = 1.0;
}

void QuadWalk::setParameters(const AtomicAction &command) {
    // These constants will be placed somewhere later.
    static const int DEFAULT = 0;
    static const int LEARNING = 1;
    static const int GRAB_DRIBBLE = 2;
    static const int GRAB_FORWARD_ONLY = 3;
    static const int GRAB_SIDE_ONLY = 4; 
    static const int GRAB_TURN_ONLY = 5; 
    static const int TURN_KICK = 6;


    walktype = command.walkType;

    forward = command.forwardMaxStep;
    left = command.leftMaxStep;
    turnCCW = command.turnCCWMaxStep;
    
    switch (command.minorWalkType) { 
        case DEFAULT:
            PG = 40;
            hfG = 90;
            hbG = 110;
            hdb = 20;
            hdf = 20;
            ffo = 59;
            fso = 10;
            bfo = -50;
            bso = 5;             
            break;
        case LEARNING:
            // Assuming walk parameters have been already set by 
            // setWalkLearningParams.
            break;
        case GRAB_DRIBBLE:
            PG = 25;
            hfG = 85;
            hbG = 110;
            hdb = 18;
            hdf = 18;
            ffo = 85;
            fso = 10;
            bfo = -50;
            bso = 5;
            break;
        case GRAB_FORWARD_ONLY:
            PG = 30;
            hfG = 90;
            hbG = 110;
            hdb = 20;
            hdf = 20;
            ffo = 66;
            fso = 10;
            bfo = -50;
            bso = 5;
            break;
        case GRAB_SIDE_ONLY:
            PG = 20;
            hfG = 95;
            hbG = 110;
            hdb = 15;
            hdf = 25;
            ffo = 89;
            fso = 10;
            bfo = -55;
            bso = 5; 
            break;
        case GRAB_TURN_ONLY: 
            PG = 25;
            hfG = 85;
            hbG = 110;
            hdb = 18;
            hdf = 18;
            ffo = 85;
            fso = 10;
            bfo = -50;
            bso = 5;
            break;
        case TURN_KICK: 
            PG = 25;
            hfG = 80;
            hbG = 100;
            hdb = 18;
            hdf = 18;
            ffo = 85;
            fso = 10;
            bfo = -55;
            bso = 5;
            break;                   
    }
    




    /*
    //WARNING:
    //Kim: This is  Paul's stuff, sorry for comment it out, it's due to a bug, and now i'm afraid to put it back coz i'm not sure how it's supposed to work.

    if (step_ == 0) {
    // Avoid confusion with OPENR OdataType. 
    OdometerData data;

    data.ffo = ffo;
    data.fso = fso;
    data.bfo = bfo;
    data.bso = bso;
    data.hf  = hf;
    data.hb  = hb;

    sOdometer->ClearBuffer();
    sOdometer->SetData(&data, sizeof(OdometerData));
    sOdometer->NotifyObservers();
    }
    */

    double doTurnCCW = command.turnCCWMaxStep;
    double doForward = command.forwardMaxStep;
    double doLeft = command.leftMaxStep;

    //if the dog is standing, PG should be 5 sothat the next command would bear no delay
    if (doForward == 0 && doLeft == 0 && doTurnCCW == 0){
      PG = 5;
      hdf = 0;
      hdb = 0;
    }

    // Set invalid inputs to zero
    if (doTurnCCW != doTurnCCW) {
        cout << "TurnCCW NaN" << endl;
        doTurnCCW = 0.0;
    }
    if (doForward != doForward) {
        cout << "Forward NaN" << endl;
        doForward = 0.0;
    }
    if (doLeft != doLeft) {
        cout << "Left NaN" << endl;
        doLeft = 0.0;
    }

	if (!useBinarySearchCalibration) {
        forward = doForward;
        turnCCW = doTurnCCW;
        left = doLeft;
		this->calibrate(doForward, doLeft, doTurnCCW);
 	} else {
		alpha = binSearch(doForward, doLeft, doTurnCCW);
		if (bDebugStepTrim && (alpha != 1)) {
			cout << "trimming forward, left and turn by a factor of " << alpha << endl;
		}
		//cout << "ORIG forward: " << forward << endl;
		forward = doForward*alpha;
		left = doLeft*alpha;
		turnCCW = doTurnCCW*alpha;
		this->calibrate(forward, left, turnCCW);
		//cout << "ACTUAL               forward: " << forward << endl;
	}

}

bool QuadWalk::testParams(double alpha, double fwd, double lft, double trn) {
	//double oldFwd = forward;
	//double oldLeft = left;
	//double oldTurn = turnCCW;
	
	this->calibrate(fwd*alpha, lft*alpha, trn*alpha);
	
	/* theta4 = angle of robot body, relative to ground */
	double theta1, theta2, theta3, theta4;
	double f, s, h, x, y, z;

	theta4 = asin((hbG - hfG) / lsh);
	f = s = h = x = y = z = 0;

    //for (int frame=0; frame<numOfFrames; ++frame) {
    //This has only one frame now.
    //TO BE DELETED

    noSolution = false;

	for (int pt=1; pt <= 4; pt++) {
		this->getFRCornerPoint(pt,f,s,h);
		/* f-s to x-y-z transformation (need f, s and h) */
		fsh2xyz(f,s,h,theta4,x,y,z);
		/* x-y-z to theta1,2,3 transformation */
		frontJointAngles(x,y,z,theta1,theta2,theta3);
		if (noSolution){
			break;
		}
	
		this->getFLCornerPoint(pt,f,s,h);
		/* f-s to x-y-z transformation (need f, s and h) */
		fsh2xyz(f,s,h,theta4,x,y,z);
		/* x-y-z to theta1,2,3 transformation */
		frontJointAngles(x,y,z,theta1,theta2,theta3);
		if (noSolution){
			break;
		}
	
		this->getBLCornerPoint(pt,f,s,h);
		/* f-s to x-y-z transformation (need f, s and h) */
		fsh2xyz(f,s,h,theta4,x,y,z);
		/* x-y-z to theta1,2,3 transformation */
		backJointAngles(x,y,z,theta1,theta2,theta3);
		if (noSolution){
			break;
		}
	
		this->getBRCornerPoint(pt,f,s,h);
		/* f-s to x-y-z transformation (need f, s and h) */
		fsh2xyz(f,s,h,theta4,x,y,z);
		/* x-y-z to theta1,2,3 transformation */
		backJointAngles(x,y,z,theta1,theta2,theta3);
		if (noSolution){
			break;
		}
    }
    
	// this->calibrate(oldFwd, oldLeft, oldTurn);
	
    return !noSolution;
}

double QuadWalk::binSearch(double fwd, double lft, double trn) {
	double alphLow = 0;
	double alphHigh = 1.0;
	
	if (testParams(alphHigh, fwd, lft, trn))
		return alphHigh;
	
	if (!testParams(alphLow, fwd, lft, trn)) {
		cout << "Warning - even 0 params are out of range in QuadWalk!!!" << endl;
        cout << "Fwd: " << fwd << " lft: " << lft << " trn: " << trn << endl;
		return 0;
	}
	
	double delta = 0;
	double walkDiff = sqrt(SQUARE(fwd)+ SQUARE(lft) + SQUARE(trn));
	
	do {
	
		double alphMid = (alphLow + alphHigh) / 2;
		
		bool midWorks = testParams(alphMid, fwd, lft, trn);
		
		if (midWorks)
			alphLow = alphMid;
		else
			alphHigh = alphMid;
	
		delta = (alphHigh - alphLow) * walkDiff;
	} while (delta > 0.1);
	
	return alphLow;
}

void QuadWalk::setCurrentJointCommand(JointCommand &jointCommand) {

	/* Now calculate transform to theta 1,2 & 3
	 *(see thesis for explanation)
	 */

	/* this is a magic part of PWalk -- canter,
	 * it moves the robots' motor up and down in a sine fashion
	 * such that it will moves faster in result.
	 *(not used in NormalWalk and ZoidalWalk)
	 */

#ifdef OFFLINE
  cout << "QuadWalk.cc: setCurretJointCommand(): BEGIN!" << endl;
#endif

	if (walktype == CanterWalkWT) {
		double stepCycle = step_ / PG * 2.0 * pi;
		hf = hfG + canterHeight * cos(stepCycle);
		hb = hbG + canterHeight * cos(stepCycle);
	} 
	else {
		hf = hfG;
		hb = hbG;
	}

	/* theta4 = angle of robot body, relative to ground */
	double theta1, theta2, theta3, theta4;
	double f, s, h, x, y, z;

	theta4 = asin((hb - hf) / lsh);
	f = s = h = x = y = z = 0;

    //for (int frame=0; frame<numOfFrames; ++frame) {
    //This has only one frame now.
    //TO BE DELETED

    /* step = opposite of step_ in step cycle, use
     * step for front right, back left, and
     * step_ for front left, back right(trot gait)
     */
    step = step_ + PG;
    if (step >= 2 * PG) {
        step -= 2 * PG;
    }
    noSolution = false;

    /* */
    this->frontRight(f,s,h);
    /* f-s to x-y-z transformation (need f, s and h) */
    fsh2xyz(f,s,h,theta4,x,y,z);
    /* x-y-z to theta1,2,3 transformation */
    frontJointAngles(x,y,z,theta1,theta2,theta3);
    if (noSolution && bDebugInverseKinematics){
        cout << " Front Right : x=" << x << " y=" << y << " z=" << z << " " << endl; ;
        step_++;
        return ;
    }

    jfr1 = theta1;
    jfr2 = theta2;
    jfr3 = theta3;

    this->frontLeft(f,s,h);
    /* f-s to x-y-z transformation (need f, s and h) */
    fsh2xyz(f,s,h,theta4,x,y,z);
    /* x-y-z to theta1,2,3 transformation */
    frontJointAngles(x,y,z,theta1,theta2,theta3);
    if (noSolution && bDebugInverseKinematics){
        cout << " Front Left : x=" << x << " y=" << y << " z=" << z << " " << endl; ;
        step_++;
        return ;
    }

    jfl1 = theta1;
    jfl2 = theta2;
    jfl3 = theta3;

    this->backLeft(f,s,h); 
    /* f-s to x-y-z transformation (need f, s and h) */
    fsh2xyz(f,s,h,theta4,x,y,z);
    /* x-y-z to theta1,2,3 transformation */
    backJointAngles(x,y,z,theta1,theta2,theta3);
    if (noSolution && bDebugInverseKinematics){
        cout << " Back Left : x=" << x << " y=" << y << " z=" << z << " " << endl; ;
        step_++;
        return ;
    }

    jbl1 = theta1;
    jbl2 = theta2;
    jbl3 = theta3;

    this->backRight(f,s,h);
    /* f-s to x-y-z transformation (need f, s and h) */
    fsh2xyz(f,s,h,theta4,x,y,z);
    /* x-y-z to theta1,2,3 transformation */
    backJointAngles(x,y,z,theta1,theta2,theta3);
    if (noSolution && bDebugInverseKinematics){
        cout << " Back Right : x=" << x << " y=" << y << " z=" << z << " " << endl; ;
        step_++;
        return ;
    }

    jbr1 = theta1;
    jbr2 = theta2;
    jbr3 = theta3;

    if (noSolution){
        if (bDebugInverseKinematics)
            cout <<" No !!!! at step " << step_ << " wt " << walktype << endl;
        return;
    }

#ifdef OFFLINE
    cout << "QuadWalk:setCurrentJointCommand(): " << step_ << jfr2 << endl;
#endif

    jointCommand.setJointEnumValue(rightFrontJoint, RAD2MICRO(jfr2) );
    jointCommand.setJointEnumValue(rightFrontShoulder, RAD2MICRO(jfr1) ); 
    jointCommand.setJointEnumValue(rightFrontKnee, RAD2MICRO(jfr3) ); 
    jointCommand.setJointEnumValue(leftFrontJoint, RAD2MICRO(jfl2) ); 
    jointCommand.setJointEnumValue(leftFrontShoulder, RAD2MICRO(jfl1) ); 
    jointCommand.setJointEnumValue(leftFrontKnee, RAD2MICRO(jfl3) ); 
    jointCommand.setJointEnumValue(rightRearJoint, RAD2MICRO(jbr2) ); 
    jointCommand.setJointEnumValue(rightRearShoulder, RAD2MICRO(jbr1) ); 
    jointCommand.setJointEnumValue(rightRearKnee, RAD2MICRO(jbr3) ); 
    jointCommand.setJointEnumValue(leftRearJoint, RAD2MICRO(jbl2) ); 
    jointCommand.setJointEnumValue(leftRearShoulder, RAD2MICRO(jbl1) ); 
    jointCommand.setJointEnumValue(leftRearKnee, RAD2MICRO(jbl3) ); 
    //} /* end frame loop */
}

void QuadWalk::goNextFrame() {
    /*
     * increment step_, fill motor buffers
     */
    step_++;
    
	/* Reset step_ at the end of a full step cycle. */
	if (step_ >= 2 * PG) {
		step_ = 0;
	}
}

bool QuadWalk::isCompleted() {
	return (step_ == 0 || step_ == PG || step_ == 2*PG);
}

int QuadWalk::getNumPoints() {
	return 2*PG;
}

int QuadWalk::getPointNum() {
	return step_;
}

void QuadWalk::setPointNum(int step) {
	step_ = step;
}

void QuadWalk::fsh2xyz(double f, double s, double h, double theta4,
		double &x, double &y, double &z)
{
	/* f-s-h coordinate system to x-y-z coordinate system. */
	z = s;
	y = h * cos(theta4) - f * sin(theta4);
	x = h * sin(theta4) + f * cos(theta4);
}

//For the inverse kinematics to have solution, b must > 0 and asin(a/b) has solution
static inline bool checkSolution(double a, double b){
    return ( fabs(b) < EPSILON || fabs(a/b) > 1);
}

void QuadWalk::frontJointAngles(double x, double y, double z,
		double &theta1, double &theta2, double &theta3)
{
	double temp1 = l1 * l1 + 2 * l3 * l3 + l2 * l2 - x * x - y * y - z * z;
	double temp2 = 2 * sqrt((l1 * l1 + l3 * l3) *(l2 * l2 + l3 * l3));
	theta3 = 2 * pi - atan(l1 / l3) - atan(l2 / l3) - acos(temp1 / temp2);
    noSolution = (noSolution || checkSolution(temp1 , temp2));
    
	double tempu = 2 * l3 * sin(theta3 / 2) * sin(theta3 / 2) + l2 * sin(theta3);
	double tempv = l1 + 2 * l3 * sin(theta3 / 2) * cos(theta3 / 2) + l2 * cos(theta3);
	theta1 = asin(z / tempv);
    noSolution = (noSolution || checkSolution(z , tempv));

	temp1  = y * tempv * cos(theta1) + tempu * x;
	temp2  = tempu * tempu + tempv * tempv * cos(theta1) * cos(theta1);
	theta2 = acos(temp1 / temp2);
    noSolution = (noSolution || checkSolution(temp1 , temp2));

	/* bit of fudginess, since formula gives +/- for theta2 */
	if (abs(x - tempv * cos(theta1) * sin(theta2) - tempu * cos(theta2)) > .001)
		theta2 *= -1;
}

void QuadWalk::backJointAngles(double x, double y, double z,
		double &theta1, double &theta2, double &theta3)
{
	double temp1 = l1 * l1 + 2 * l3 * l3 + l4 * l4 - x * x - y * y - z * z;
	double temp2 = 2 * sqrt((l1 * l1 + l3 * l3) *(l4 * l4 + l3 * l3));
	theta3 = 2 * pi - atan(l1 / l3) - atan(l4 / l3) - acos(temp1 / temp2);
    noSolution = (noSolution || checkSolution(temp1 , temp2));

	double tempu = 2 * l3 * sin(theta3 / 2) * sin(theta3 / 2) + l4 * sin(theta3);
	double tempv = l1 + 2 * l3 * sin(theta3 / 2) * cos(theta3 / 2) + l4 * cos(theta3);
	theta1 = asin(z / tempv);
    noSolution = (noSolution || checkSolution(z, tempv));

	temp1  = y * tempv * cos(theta1) - tempu * x;
	temp2  = tempu * tempu + tempv * tempv * cos(theta1) * cos(theta1);
	theta2 = acos(temp1 / temp2);
    noSolution = (noSolution || checkSolution(temp1, temp2));

	if (abs(-x - tempv * cos(theta1) * sin(theta2) - tempu * cos(theta2)) > .001)
		theta2 *= -1;
}

void QuadWalk::getOdometry(double &delta_forward, double &delta_left, double &delta_turn, bool highGain) {
    (void) highGain; // stop compiler warning
    if (PG == 0) {
        cout << "PG is Zero!" << endl;
        delta_forward = 0.0;
        delta_left = 0.0;
        delta_turn = 0.0;
    }
    else{
        delta_forward = forward / 2.0 / PG;
        delta_left = left / 2.0 / PG;
        delta_turn = turnCCW / 2.0 / PG;
    }

    if (delta_forward != delta_forward) {
        cout << "delta_forward NaN" << endl;
    }
    if (delta_left != delta_left) {
        cout << "delta_left NaN" << endl;
    }
    if (delta_turn != delta_turn) {
        cout << "delta_turn NaN" << endl;
    }
}
