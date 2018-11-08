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
 * $Id: SkiWalk.cc 6560 2005-06-12 13:53:10Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "legsGeometry.h"
#include "SkiWalk.h"
#include "../share/Common.h"
#include "JointCommand.h"
#include "../share/sensors.h"
#include "PWalk.h"

static const bool debugFSH = false;
/*
 * Command for walk calibration
 * __05wtcal__99 19 7 0 0 40 2 2 4 4 3 3 5 55 15 80 -55 5 115 0 0 3 0 0
 * __05wtcal__99 19 7 0 0 40 2 2 4 4 3 3 5 55 15 80 -25 5 120 0 0 3 0 0
 */

SkiWalk::SkiWalk(const PWalk *walkStuff) {
	pwalk = walkStuff;
	setDefaultParameters();
}

void SkiWalk::setDefaultParameters() {
	struct skiWalkParms tempWalkParams;
	struct skiStepParms tempStepParams;
	
	tempStepParams.cmdFwd = 0;
	tempStepParams.cmdLeft = 0;
	tempStepParams.cmdTurnCCW = 0;
	
	tempWalkParams.init();

	step = 0;
	time = 1;

	setWalkParams(tempWalkParams);
	setStepParams(tempStepParams);
}

void SkiWalk::setParameters(const AtomicAction &bufferedCommand) {

    /*
	if (false) {
		skiWalkParms tempWalkParams;

		tempWalkParams.init();

		tempWalkParams.halfStepTime = bufferedCommand.speed;
		tempWalkParams.frontClearance = bufferedCommand.hdF;
		tempWalkParams.backClearance = bufferedCommand.hdB;
		tempWalkParams.frontF = bufferedCommand.ffO;
		tempWalkParams.frontS = bufferedCommand.fsO;
		tempWalkParams.frontH = bufferedCommand.hF;
		tempWalkParams.backF = bufferedCommand.bfO;
		tempWalkParams.backS = bufferedCommand.bsO;
		tempWalkParams.backH = bufferedCommand.hB;

		setWalkParams(tempWalkParams);
	}*/

	skiStepParms tempStepParams;
	
	tempStepParams.cmdFwd = bufferedCommand.forwardSpeed;
	tempStepParams.cmdLeft = bufferedCommand.leftSpeed;
	tempStepParams.cmdTurnCCW = bufferedCommand.turnCCWSpeed;

	setStepParams(tempStepParams);
}

void SkiWalk::setWalkParams(const struct skiWalkParms &newParms) {
	walkParms = newParms;

	/*	cout << "Set SkiWalk parameters: ";
	newParms.print(cout);
	cout << endl; */

	if (walkParms.halfStepTime < 1) {
		// stop a whole lot of problems with div by 0
		// You can't do anything in zero or -ve time
		walkParms.halfStepTime = 1;
		standing = true;
	} else {
		standing = false;
	}

	// add some limits to the parameters so that the timing makes sense

	if (walkParms.frontUpTime < 1) {
		walkParms.frontUpTime = 1;
	} else if (walkParms.frontUpTime > walkParms.halfStepTime/2) {
		walkParms.frontUpTime = walkParms.halfStepTime/2;
	}
	if (walkParms.frontDownTime < 1) {
		walkParms.frontDownTime = 1;
	} else if (walkParms.frontDownTime > walkParms.halfStepTime/2) {
		walkParms.frontDownTime = walkParms.halfStepTime/2;
	}
	if (walkParms.backUpTime < 1) {
		walkParms.backUpTime = 1;
	} else if (walkParms.backUpTime > walkParms.halfStepTime/2) {
		walkParms.backUpTime = walkParms.halfStepTime/2;
	}
	if (walkParms.backDownTime < 1) {
		walkParms.backDownTime = 1;
	} else if (walkParms.backDownTime > walkParms.halfStepTime/2) {
		walkParms.backDownTime = walkParms.halfStepTime/2;
	}

	bodyTilt = asin((walkParms.backH - walkParms.frontH)/BODY_LENGTH);
	shoulderDist = BODY_LENGTH*cos(bodyTilt);
	
	double dF = shoulderDist/2;
	double dS = BODY_WIDTH/2;
	
	flFOffset = dF + walkParms.frontF - walkParms.turnCenterF;
	flLOffset = -dS - walkParms.frontS + walkParms.turnCenterL + walkParms.sideOffset;
	flTheta0 = atan2(flFOffset, flLOffset);
	flRadius = sqrt(flFOffset*flFOffset + flLOffset*flLOffset);

	frFOffset = dF + walkParms.frontF - walkParms.turnCenterF;
	frLOffset = dS + walkParms.frontS + walkParms.turnCenterL + walkParms.sideOffset;
	frTheta0 = atan2(frFOffset, frLOffset);
	frRadius = sqrt(frFOffset*frFOffset + frLOffset*frLOffset);

	blFOffset = -dF + walkParms.backF - walkParms.turnCenterF;
	blLOffset = -dS - walkParms.backS + walkParms.turnCenterL + walkParms.sideOffset;
	blTheta0 = atan2(blFOffset, blLOffset);
	blRadius = sqrt(blFOffset*blFOffset + blLOffset*blLOffset);

	brFOffset = -dF + walkParms.backF - walkParms.turnCenterF;
	brLOffset = dS + walkParms.backS + walkParms.turnCenterL + walkParms.sideOffset;
	brTheta0 = atan2(brFOffset, brLOffset);
	brRadius = sqrt(brFOffset*brFOffset + brLOffset*brLOffset);
}

void SkiWalk::setStepParams(const struct skiStepParms &newParms) {
	stepParms = newParms;
	
	// cout << "Set SkiWalk step parameters: " << newParms.cmdFwd << ", " << newParms.cmdLeft << ", " << newParms.cmdTurnCCW << endl;

	double oldTime = time;
	double oldStep = step;
	
	time = 2*walkParms.halfStepTime;
	
	if (oldTime == 0) {
		step = 0;
	} else {
		step = oldStep/oldTime*time;	// move to the correct location in the new step
	}
	
	calibrateWalk(stepParms.cmdFwd, stepParms.cmdLeft, stepParms.cmdTurnCCW,
				fwdLength, leftLength, turnLength);
	
	forwardSpeed = stepParms.cmdFwd/walkParms.halfStepTime;
	leftSpeed = stepParms.cmdLeft/walkParms.halfStepTime;
	turnSpeed = stepParms.cmdTurnCCW/walkParms.halfStepTime;

	frontStartLeadOutTime = time/4;
	backStartLeadOutTime = time/4;
	frontEndLeadInTime = time - time/4;
	backEndLeadInTime = time - time/4;
	frontEndLeadOutTime = frontStartLeadOutTime + walkParms.frontUpTime;
	backEndLeadOutTime = backStartLeadOutTime + walkParms.backUpTime;
	frontStartLeadInTime = frontEndLeadInTime - walkParms.frontDownTime;
	backStartLeadInTime = backEndLeadInTime - walkParms.backDownTime;

}

void SkiWalk::calibrateWalk(double &cmdFwd, double &cmdLeft, double &cmdTurn,
				double &fwdLen, double &leftLen, double &turnLen) {
	// first clip things to the valid range
	// this changes the commands themselves
	cmdFwd = CLIP(cmdFwd, MAX_SKI_FORWARD);
	cmdLeft = CLIP(cmdLeft, MAX_SKI_LEFT);
	cmdTurn = CLIP(cmdTurn, MAX_SKI_TURN);
	
	// now calibrate the lengths to get the correct walking speed
	// If you're doing calibration, this is the bit you want to change
	// If the walk theory was perfect, these would be simple assignments:
	// fwdLen = cmdFwd, etc.
	
	fwdLen = cmdFwd;
	leftLen = cmdLeft;
	turnLen = cmdTurn;
	
	// finally, convert into the correct units
	
	fwdLen *= 10;	// cm into mm
	leftLen *= 10;	// cm into mm
	turnLen = DEG2RAD(turnLen);	// degrees into rads
}

void SkiWalk::getOdometry(double &delta_forward, double &delta_left, double &delta_turn, bool highGain) {
    (void)highGain; // compiler warning
	delta_forward = forwardSpeed;
	delta_left = leftSpeed;
	delta_turn = turnSpeed;
}

void SkiWalk::handleTilt(JOINT_Values &joints, bool sensedTilt, bool front) {

	double tilt = bodyTilt;

	if (sensedTilt) {
		tilt = sensedBodyTilt;	// while foot is in the air, use the sensed bodyTilt
	}

	if (front) {
		joints.rotator += tilt;	// handle the tilt
	} else {
		joints.rotator -= tilt;	// handle the tilt
	}
}

void SkiWalk::getLocusX(double step,
					double &X,
					const bool front,
					double length) {

	const double startLeadInTime = front?frontStartLeadInTime:backStartLeadInTime;
	const double endLeadInTime = front?frontEndLeadInTime:backEndLeadInTime;
	const double startLeadOutTime = front?frontStartLeadOutTime:backStartLeadOutTime;
	const double endLeadOutTime = front?frontEndLeadOutTime:backEndLeadOutTime;

	const double baseSpeed = length*2.0/time;

 	if ((step < endLeadOutTime) || (step > startLeadInTime)) {	
	
		// bottom of locus
		double wrappedStep = step;
		if (wrappedStep > time/2)
			wrappedStep -= time;

		X = -wrappedStep*baseSpeed;
	} else if (step < time/2) {
		length = length/2 + baseSpeed*(endLeadOutTime - startLeadOutTime);
		double part = (step - endLeadOutTime)/(time/2 - endLeadOutTime);
		X = -length*(1-part);
	} else {
		length = length/2 + baseSpeed*(endLeadInTime - startLeadInTime);
		double part = (startLeadInTime - step)/(startLeadInTime - time/2);
		X = length*(1-part);
	}
}

void SkiWalk::getLocusY(const double step,
					double &Y,
					const bool front,
					const double height) {

	const double startLeadInTime = front?frontStartLeadInTime:backStartLeadInTime;
	const double endLeadInTime = front?frontEndLeadInTime:backEndLeadInTime;
	const double startLeadOutTime = front?frontStartLeadOutTime:backStartLeadOutTime;
	const double endLeadOutTime = front?frontEndLeadOutTime:backEndLeadOutTime;

	if ((step <= startLeadOutTime) || (step >= endLeadInTime)) {
		// flat section along bottom of locus
		Y = 0;
	} else if ((step < endLeadOutTime) || (step > startLeadInTime)) {
		// handle lead in and out
		double eX;
		// first calculate posn in the leadout
		if (step < time/2) {
			eX =  (step - startLeadOutTime)/(endLeadOutTime - startLeadOutTime);
		} else {
			eX =  (endLeadInTime - step)/(endLeadInTime - startLeadInTime);
		}
		
		// linear lead
		Y = eX * height;
		
	} else {
		Y = height;
	}
}

void SkiWalk::makeFSFwd(double X, double &f, double &s) {
	f = X;
	s = 0;
}

void SkiWalk::makeFSLft(double X, double &f, double &s, bool left) {
	f = 0;
	s = left?X:-X;
}

void SkiWalk::makeFSTrn(double X, double &f, double &s, bool left, bool front) {
	if (front) {
		if (left) {
			f = sin(X + flTheta0)*flRadius - flFOffset;
			s = -(cos(X + flTheta0)*flRadius - flLOffset);
		} else {
			f = sin(X + frTheta0)*frRadius - frFOffset;
			s = cos(X + frTheta0)*frRadius - frLOffset;
		}
	} else {
		if (left) {
			f = sin(X + blTheta0)*blRadius - blFOffset;
			s = -(cos(X + blTheta0)*blRadius - blLOffset);
		} else {
			f = sin(X + brTheta0)*brRadius - brFOffset;
			s = cos(X + brTheta0)*brRadius - brLOffset;
		}
	}
}

void SkiWalk::getLocusJoints(double step, bool front, bool left, JOINT_Values &joints) {
	double X, f, s, h, fTemp, sTemp;
	XYZ_Coord xyz;

	const double startLeadInTime = front?frontStartLeadInTime:backStartLeadInTime;
	// const double endLeadInTime = front?frontEndLeadInTime:backEndLeadInTime;
	const double startLeadOutTime = front?frontStartLeadOutTime:backStartLeadOutTime;
	const double endLeadOutTime = front?frontEndLeadOutTime:backEndLeadOutTime;

	// this is active - wait until the leg is up before continuing

	if (!front) {
		if ((step > startLeadOutTime) && (step < endLeadOutTime)) {
			backLegUp = false;
		}
		if ((step >= endLeadOutTime) && (step < time/2)) {
			if (!backLegUp) {
				XYZ_Coord pt;
				JOINT_Values sensJoints;
				if (left) {
					sensJoints.rotator = MICRO2RAD(pwalk->sensorVal[ssRL_ROTATOR]);
					sensJoints.shoulder = MICRO2RAD(pwalk->sensorVal[ssRL_ABDUCTOR]);
					sensJoints.knee = MICRO2RAD(pwalk->sensorVal[ssRL_KNEE]);
				} else {
					sensJoints.rotator = MICRO2RAD(pwalk->sensorVal[ssRR_ROTATOR]);
					sensJoints.shoulder = MICRO2RAD(pwalk->sensorVal[ssRR_ABDUCTOR]);
					sensJoints.knee = MICRO2RAD(pwalk->sensorVal[ssRR_KNEE]);
				}
				sensJoints.rotator += sensedBodyTilt;
				kinematics(sensJoints, tLeg, bbLegAirToes, pt);
				
				double waitHeight = sensedHipHeight -
							walkParms.backClearance + walkParms.waitHeightSlack;
				
				cout << "py: " << pt.y << " wh: " << waitHeight << endl;
				
				if (pt.y < waitHeight) {
					backLegUp = true;
				}
			}
			if (!backLegUp) {
				int delay = (int) (step - endLeadOutTime);
				cout << "d: " << delay << endl;
				step = endLeadOutTime;
			}
		}
	}

	// now the normal passive stuff

	getLocusY(step, h, front, front?frontHeight:backHeight);

	// add the thrust
	{
		if (step < startLeadOutTime)
			h -= step / startLeadOutTime * (walkParms.thrustHeight);
		else if (step < endLeadOutTime) {
			double diff = endLeadOutTime - startLeadOutTime;
			h -= (endLeadOutTime - step) / diff * (walkParms.thrustHeight);
		}
	}
	// and the canter
	double stepCycle = step / time * 4.0 * pi;
	h -= walkParms.canterHeight * cos(stepCycle);
	
	getLocusX(step, X, front, fwdLength);	// fwd
	makeFSFwd(X, f, s);
	
	getLocusX(step, X, front, leftLength);	// left
	makeFSLft(X, fTemp, sTemp, left);
	f += fTemp;
	s += sTemp;
	
	getLocusX(step, X, front, turnLength);	// turn
	makeFSTrn(X, fTemp, sTemp, left, front);
	f += fTemp;
	s += sTemp;

	if (debugFSH)
		cout << "fl: " << front << left << " fsh: " << f << " " << s << " " << h << endl;
	
	// convert to xyz coords
	if (front) {
		xyz.x = walkParms.frontF + f;
		xyz.z = walkParms.frontS + s + (left?-walkParms.sideOffset:walkParms.sideOffset);
		xyz.y = walkParms.frontH - h; 
	} else {
		xyz.x = walkParms.backF + f;
		xyz.z = walkParms.backS + s + (left?-walkParms.sideOffset:walkParms.sideOffset);
		xyz.y = walkParms.backH - h; 
	}
	
	if (front) {
		invKinematics(xyz, tLeg, fbLegPadCenter, joints);
	} else {
		xyz.x *= -1;
		XYZ_Coord backLeg;
		
		if (step < startLeadOutTime) {
			backLeg = bbLegToes;
		} else if (step < startLeadInTime) {
			backLeg = bbLegAirToes;
		} else {
			backLeg = bbLegToes;
		}
		
		invKinematics(xyz, tLeg, backLeg, joints);
	}
	
	handleTilt(joints, ((step > startLeadOutTime) && (step < startLeadInTime)), front);
}

void SkiWalk::setCurrentJointCommand(JointCommand &jointCommand) {
	JOINT_Values leg;
	
	double step_ = step + time/2;
	if (step_ > time)
		step_ -= time;
	
	if (standing) {	// handle standing still
		step = step_ = 0;
	}

	sensedShoulderHeight = pwalk->frontShoulderHeight;
	sensedHipHeight = pwalk->backHipHeight;
	sensedBodyTilt = pwalk->bodyTilt;
/*
	cout << "ssh: " << sensedShoulderHeight << " csh: " << walkParms.frontH << endl;
	cout << "shh: " << sensedHipHeight << " chh: " << walkParms.backH << endl;
	cout << "sbt: " << RAD2DEG(sensedBodyTilt) << " cbt: " << RAD2DEG(bodyTilt) << endl;
*/
	if (sensedShoulderHeight > walkParms.frontH) {
		sensedShoulderHeight = walkParms.frontH;
	}

	if (sensedHipHeight > walkParms.backH) {
		sensedHipHeight = walkParms.backH;
	}

	frontHeight = walkParms.frontH - sensedShoulderHeight + walkParms.frontClearance;
	backHeight = walkParms.backH - sensedHipHeight + walkParms.backClearance;

	getLocusJoints(step, true, true, leg);
    jointCommand.setJointEnumValue(leftFrontJoint, RAD2MICRO(leg.rotator) ); 
    jointCommand.setJointEnumValue(leftFrontShoulder, RAD2MICRO(leg.shoulder) ); 
    jointCommand.setJointEnumValue(leftFrontKnee, RAD2MICRO(leg.knee) ); 
	
	getLocusJoints(step_, true, false, leg);
    jointCommand.setJointEnumValue(rightFrontJoint, RAD2MICRO(leg.rotator) );
    jointCommand.setJointEnumValue(rightFrontShoulder, RAD2MICRO(leg.shoulder) ); 
    jointCommand.setJointEnumValue(rightFrontKnee, RAD2MICRO(leg.knee) ); 
	
	getLocusJoints(step_, false, true, leg);
    jointCommand.setJointEnumValue(leftRearJoint, RAD2MICRO(leg.rotator) ); 
    jointCommand.setJointEnumValue(leftRearShoulder, RAD2MICRO(leg.shoulder) ); 
    jointCommand.setJointEnumValue(leftRearKnee, RAD2MICRO(leg.knee) ); 

	getLocusJoints(step, false, false, leg);
    jointCommand.setJointEnumValue(rightRearJoint, RAD2MICRO(leg.rotator) ); 
    jointCommand.setJointEnumValue(rightRearShoulder, RAD2MICRO(leg.shoulder) ); 
    jointCommand.setJointEnumValue(rightRearKnee, RAD2MICRO(leg.knee) ); 
}
