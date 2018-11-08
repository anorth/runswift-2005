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
 * $Id: SkellipticalWalk.h 7459 2005-07-08 07:06:29Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _SkellipticalWalk_h
#define _SkellipticalWalk_h

#include <iostream>
#include <sstream>

#include "legsGeometry.h"
#include "BaseAction.h"
#include "../share/ActionProDef.h"
#include "../share/PWalkDef.h"



class skelWalkParms {
	public:
	int halfStepTime;	// the number of points (8ms apart) in half a step
	double frontFwdHeight;	// the height off the ground of the front leg recovery when moving forward
	double frontTrnLftHeight;	// the height off the ground of the front leg recovery when moving sideways/turning
	double backFwdHeight;	// the height off the ground of the back leg recovery when moving forward
	double backTrnLftHeight;	// the height off the ground of the back leg recovery when moving sideways/turning
	double frontDutyCycle;	// the percentage (0-1) of the step that the front feet are on the ground
	double backDutyCycle;	// the percentage (0-1) of the step that the back feet are on the ground
	double frontLeadInFrac;	// the percentage (0-1) of the first half of the ground stroke that is 'lead in' for the front legs
	double frontLeadDownFrac;	// the percentage (0-1) of the recovery height that is 'lead in' for the front legs
	double frontLeadOutFrac;	// the percentage (0-1) of the second half of the ground stroke that is 'lead out' for the front legs
	double frontLeadUpFrac;	// the percentage (0-1) of the recovery height that is 'lead out' for the front legs
	double backLeadInFrac;	// the percentage (0-1) of the first half of the ground stroke that is 'lead in' for the back legs
	double backLeadDownFrac;	// the percentage (0-1) of the recovery height that is 'lead in' for the back legs
	double backLeadOutFrac;	// the percentage (0-1) of the second half of the ground stroke that is 'lead out' for the back legs
	double backLeadUpFrac;	// the percentage (0-1) of the recovery height that is 'lead out' for the back legs
	double frontF;	// the distance forward from the shoulder of the home point of the front legs
	double frontS;	// the distance sideways from the shoulder of the home point of the front legs
	double frontH;	// the vertical distance from the shoulder of the home point of the front legs (when walking forward)
	double frontLeftH;	// the vertical distance from the shoulder of the home point of the front legs (when walking sideways)
	double backF;	// the distance forward from the shoulder of the home point of the back legs
	double backS;	// the distance sideways from the shoulder of the home point of the back legs
	double backH;	// the vertical distance from the shoulder of the home point of the back legs
	double sideOffset;	// a left/right offset to move the center of balance
	double turnCenterF;	// a front/back adjustment to the centre of turn
	double turnCenterL;	// a left/right adjustment to the centre of turn
	double frontBackForwardRatio;	// an adjustment to the forward distance travelled by the front legs relative to the back legs (to account for slip)
	double frontBackLeftRatio;	// an adjustment to the sideways distance travelled by the front legs relative to the back legs (to account for slip)
	double frontHomeAngleDeg;	// the angle in degrees of the home point on the front lower leg
	double frontRollWalkRatio;	// the amount of rolling vs. walking that the front leg should do.  0 == all walk, 1 == all roll
	double thrustHeight;
	double canterHeight;

	void init() {
	  halfStepTime = 31; //PG   //1
	  frontH = 81; //hF
	  backH = 115; //hB
	  frontFwdHeight = 20; //hdF
	  backFwdHeight = 31; //hdB //5
	  frontF = 56.88; // ffo
	  frontS = 15; // fso
	  backF = -54; // bfo
	  backS = 6;   // bso

	  frontTrnLftHeight = 20; // 10
	  backTrnLftHeight = 26;	  

	  frontDutyCycle = 0.45;
	  backDutyCycle = 0.55; // % on the ground
	  frontLeadInFrac = 0.06;
	  frontLeadDownFrac = 0.05; // 15
	  frontLeadOutFrac = 0.02;
	  frontLeadUpFrac = 0.02;
	  backLeadInFrac = 0.11;
	  backLeadDownFrac = 0.2;
	  backLeadOutFrac = 0.2; // 20
	  backLeadUpFrac = 0.4;
	  
	  thrustHeight = 0;
	  canterHeight = 1;
	  sideOffset = 3;
	  turnCenterF = 0; // 25
	  turnCenterL = 0;

	  frontLeftH = 110;
	  
	  frontBackForwardRatio = 1;
	  frontBackLeftRatio = 1;
	  frontRollWalkRatio = 0; // 30	  
	  frontHomeAngleDeg = 0;
	}

	void read(const char* c_str) {
	  string str(c_str);
	  std::istringstream iss(str);
	  read(iss);
	}

	void read(std::istream &in) {
	  double tempPG;
	  in >> tempPG;  // 1
	  halfStepTime = (int)rint(tempPG); //PG
	  in >> frontH; //hF
	  in >> backH;  //hB
	  in >> frontFwdHeight; //hdF
	  in >> backFwdHeight; //hdB //5
	  in >> frontF;  //ffo
	  in >> frontS;  //fso
	  in >> backF;  //bfo
	  in >> backS;  //bso

	  in >> frontTrnLftHeight; // 10
	  in >> backTrnLftHeight;

	  in >> frontDutyCycle;
	  frontDutyCycle /= 100;
	  in >> backDutyCycle;
	  backDutyCycle /= 100;
	  in >> frontLeadInFrac;
	  frontLeadInFrac /= 100;
	  in >> frontLeadDownFrac; // 15
	  frontLeadDownFrac /= 100;
	  in >> frontLeadOutFrac;
	  frontLeadOutFrac /= 100;
	  in >> frontLeadUpFrac;
	  frontLeadUpFrac /= 100;
	  in >> backLeadInFrac;
	  backLeadInFrac /= 100;
	  in >> backLeadDownFrac;
	  backLeadDownFrac /= 100;
	  in >> backLeadOutFrac; // 20
	  backLeadOutFrac /= 100;
	  in >> backLeadUpFrac;
	  backLeadUpFrac /= 100;

	  in >> thrustHeight;
	  in >> canterHeight;
	  in >> sideOffset;
	  in >> turnCenterF; // 25
	  in >> turnCenterL;

	  in >> frontLeftH;
	  in >> frontBackForwardRatio;
	  frontBackForwardRatio /= 100;
	  in >> frontBackLeftRatio;
	  frontBackLeftRatio /= 100;
	  in >> frontRollWalkRatio; // 30
	  frontRollWalkRatio /= 100;
	  in >> frontHomeAngleDeg;
	}

	void print(std::ostream &out) const {
		out << "[";
		out << halfStepTime << ", "; // 1
		out << frontH << ", ";
		out << backH << ", ";
		out << frontFwdHeight << ", ";
		out << backFwdHeight << ", "; // 5
		out << frontF << ", ";
		out << frontS << ", ";
		out << backF << ", ";
		out << backS << ", ";

		out << frontTrnLftHeight << ", "; // 10
		out << backTrnLftHeight << ", ";

		out << frontDutyCycle << ", ";
		out << backDutyCycle << ", ";
		out << frontLeadInFrac << ", ";
		out << frontLeadDownFrac << ", "; // 15
		out << frontLeadOutFrac << ", ";
		out << frontLeadUpFrac << ", ";
		out << backLeadInFrac << ", ";
		out << backLeadDownFrac << ", ";
		out << backLeadOutFrac << ", "; // 20
		out << backLeadUpFrac << ", ";


		out << thrustHeight << ", ";
		out << canterHeight << ", ";
		out << sideOffset << ", ";
		out << turnCenterF << ", " // 25
		    << turnCenterL << ", "
		    << frontLeftH << ", "
		    << frontBackForwardRatio << ", "
		    << frontBackLeftRatio << ", "
		    << frontRollWalkRatio << ", " // 30
		    << frontHomeAngleDeg
		    << "]";

	}
};



struct skelStepParms {
	double cmdFwd;
	double cmdLeft;
	double cmdTurnCCW;	// in degrees
	double shortStep;   // proportion of normal step length
};

class SkellipticalWalk : public BaseAction {
 public:
  int minorWalkType;

	protected:
	
	double step;
	

	struct skelWalkParms walkParms;
	struct skelStepParms stepParms;

	// now a list of parameters calculated each time

	double time;
	bool standing;

	double fwdLength;
	double leftLength;
	double turnLength;	// in radians
	
	double forwardSpeed;
	double leftSpeed;
	double turnSpeed;

	double bodyTilt;
	double shoulderDist;

	double frontH;
	
	double flTheta0;
	double frTheta0;
	double blTheta0;
	double brTheta0;
	double flFOffset;
	double flLOffset;
	double frFOffset;
	double frLOffset;
	double blFOffset;
	double blLOffset;
	double brFOffset;
	double brLOffset;
	double frRadius;
	double flRadius;
	double brRadius;
	double blRadius;
	
	double frontHeight;
	double backHeight;
	
	double frontStartLeadOutTime;
	double frontEndLeadOutTime;
	double frontStartLeadInTime;
	double frontEndLeadInTime;
	double backStartLeadOutTime;
	double backEndLeadOutTime;
	double backStartLeadInTime;
	double backEndLeadInTime;

	// calibrate the walking motion
	// this can have two effects
	//   - The walk commands can be clipped
	//   - The lengths fed into the lower routines can be modified from the original commands
	
	void calibrateWalk(double &cmdFwd, double &cmdLeft, double &cmdTurn, double shortStep,
				double &fwdLen, double &leftLen, double &turnLen);

	// fast foward calibration
	// this mwt is so popular, several other mwt use it with small adjustments
	void calibrateWalk_FastForward(double &, double &, double &, double,
				      double &, double &, double &);

	
	// parameterized representation of the bottom half of the locus
	// returns the X offset in the locus for the given step value
	// step = 0 gives X = 0 which is the home position
	// forward motion moves X -ve
	void getLocusX(const double step,
					double &X,
					const bool front,
					const double length);
	
	// parameterized representation of the bottom half of the locus
	// returns the height of the foot above the ground for the given step value
	void getLocusY(const double step,
					double &Y,
					const bool front,
					const double height);
	
	// Convert an X location in the forward locus into fs coordinates for a given leg
	void makeFSFwd(double X, double &f, double &s);
	
	// Convert an X location in the left locus into fs coordinates for a given leg
	void makeFSLft(double X, double &f, double &s, bool left);
	
	// Convert an X location in the turn locus into fs coordinates for a given leg
	void makeFSTrn(double X, double &f, double &s, bool left, bool front);
	
	// get the joint values for a specified leg at any point in the cycle
	// using the locus (note - currently only defined for the bottom half of the cycle
	// step = 0 has the foot on the ground in the home position
	void getLocusJoints(double step, bool front, bool left, JOINT_Values &joints);
	
	// adjust the joint angles to handle the tilt
	void handleTilt(JOINT_Values &joints, bool front);
	
	public:

	SkellipticalWalk() {
	  initWalkParameters(); 
	  setDefaultParameters();
	}
	
	virtual ~SkellipticalWalk() { }
	
	virtual void reset() { }

	void setFrontH();

	void initWalkParameters();
	
	void setDefaultParameters();
	
	void setWalkParams(const struct skelWalkParms &newParms);
	
	void setStepParams(const struct skelStepParms &newParms);
	
	void setParameters(const AtomicAction &bufferedCommand);
	
	virtual void setCurrentJointCommand(JointCommand &j);
	
	virtual void getOdometry(double &delta_forward, double &delta_left, double &delta_turn, bool highGain);

	virtual void goNextFrame() {
		step = step + 1;
		while (step > time) {
			step -= time;
		}
	}

	virtual bool isCompleted() {
		return (standing ||
				(step < 0.5) ||
				(step > (time-0.5)) ||
				((step > (time/2 - 0.5)) && (step < (time/2 + 0.5))));
	}

	virtual int getNumPoints() {
		return (int)time;
	}

	virtual int getPointNum() {
		return (int)step;
	}

	virtual void setPointNum(int s) {
		this->step = s;
	}

	virtual bool usingHead() {
		return false;
	}

	virtual bool isSpecialAction() {
		return false;
	}
};

static const double MAX_SKEL_FORWARD = 20 * 1000;
static const double MAX_SKEL_LEFT = 20 * 1000;
static const double MAX_SKEL_TURN = 100 * 1000;

#endif	// _SkellipticalWalk_h
