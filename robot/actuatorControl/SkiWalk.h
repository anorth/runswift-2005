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
 * $Id: SkiWalk.h 5110 2005-03-17 00:33:56Z alexn $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



class SkiWalk;

#ifndef _SkiWalk_h
#define _SkiWalk_h

#include <iostream>

#include "legsGeometry.h"
#include "BaseAction.h"
#include "../share/ActionProDef.h"
#include "../share/PWalkDef.h"

class PWalk;

static const double MAX_SKI_FORWARD = 20;
static const double MAX_SKI_LEFT = 20;
static const double MAX_SKI_TURN = 90;

class skiWalkParms {
	public:
	int halfStepTime;
	double frontUpTime;
	double frontDownTime;
	double backUpTime;
	double backDownTime;
	double frontClearance;
	double backClearance;
	double waitHeightSlack;
	double frontF;
	double frontS;
	double frontH;
	double backF;
	double backS;
	double backH;
	double thrustHeight;
	double canterHeight;
	double sideOffset;
	double turnCenterF;
	double turnCenterL;

	void init() {
		halfStepTime = 50;
		frontUpTime = 2;
		frontDownTime = 2;
		backUpTime = 4;
		backDownTime = 4;
		frontClearance = 3;
		backClearance = 3;
		waitHeightSlack = 5;
		frontF = 55;
		frontS = 15;
		frontH = 80;
		backF = -25;
		backS = 5;
		backH = 120;
		thrustHeight = 0;
		canterHeight = 0;
		sideOffset = -2;
		turnCenterF = 0;
		turnCenterL = 0;
	}

	void read(std::istream &in) {
		in >> halfStepTime;
		in >> frontUpTime;
		in >> frontDownTime;
		in >> backUpTime;
		in >> backDownTime;
		in >> frontClearance;
		in >> backClearance;
		in >> waitHeightSlack;
		in >> frontF;
		in >> frontS;
		in >> frontH;
		in >> backF;
		in >> backS;
		in >> backH;
		in >> thrustHeight;
		in >> canterHeight;
		in >> sideOffset;
		in >> turnCenterF;
		in >> turnCenterL;
	}

	void print(std::ostream &out) const {
		out << "[";
		out << halfStepTime << ", ";
		out << frontUpTime << ", ";
		out << frontDownTime << ", ";
		out << backUpTime << ", ";
		out << backDownTime << ", ";
		out << frontClearance << ", ";
		out << backClearance << ", ";
		out << waitHeightSlack << ", ";
		out << frontF << ", ";
		out << frontS << ", ";
		out << frontH << ", ";
		out << backF << ", ";
		out << backS << ", ";
		out << backH << ", ";
		out << thrustHeight << ", ";
		out << canterHeight << ", ";
		out << sideOffset << ", ";
		out << turnCenterF << ", ";
		out << turnCenterL;
		out << "]";
	}
};

struct skiStepParms {
	double cmdFwd;
	double cmdLeft;
	double cmdTurnCCW;	// in degrees
};

class SkiWalk : public BaseAction {
	protected:
	
	const PWalk *pwalk;
	
	double step;
	
	struct skiWalkParms walkParms;
	struct skiStepParms stepParms;

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
	
	double frontHeight;
	double backHeight;
	
	double sensedBodyTilt;
	double sensedShoulderHeight;
	double sensedHipHeight;
	
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
	
	double frontStartLeadOutTime;
	double frontEndLeadOutTime;
	double frontStartLeadInTime;
	double frontEndLeadInTime;
	double backStartLeadOutTime;
	double backEndLeadOutTime;
	double backStartLeadInTime;
	double backEndLeadInTime;

	bool backLegUp;

	// calibrate the walking motion
	// this can have two effects
	//   - The walk commands can be clipped
	//   - The lengths fed into the lower routines can be modified from the original commands
	
	void calibrateWalk(double &cmdFwd, double &cmdLeft, double &cmdTurn,
				double &fwdLen, double &leftLen, double &turnLen);
	
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
	void handleTilt(JOINT_Values &joints, bool sensTilt, bool front);
	
	public:

	SkiWalk(const PWalk *walkStuff);
	
	virtual ~SkiWalk() { }
	
	virtual void reset() { }
	
	void setDefaultParameters();
	
	void setWalkParams(const struct skiWalkParms &newParms);
	
	void setStepParams(const struct skiStepParms &newParms);
	
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

#endif	// _SkiWalk_h
