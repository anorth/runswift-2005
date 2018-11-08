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
 * $Id: BaseAction.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright(c) 2003 UNSW
 * All Rights Reserved.
 *
 **/

#ifndef _BaseAction_h_DEFINED
#define _BaseAction_h_DEFINED

#include "../share/PWalkDef.h"
#include "JointCommand.h"

using namespace std;

/* posa = position array, format (double):
   duration,
   head tilt, pan, roll,
   right front joint, shoulder, knee,
   left front joint, shoulder, knee,
   right hind joint, shoulder, knee,
   left hind joint, shoulder, knee
   */



extern const char* JOINT_NAME[NUM_OF_EFFECTORS + 1];

class BaseAction {
	public:
		virtual ~BaseAction() { }

		virtual void reset() = 0;

		virtual double getCurrentJointValue(int jointIndex) {
			JointCommand tempCommand;
			setCurrentJointCommand(tempCommand);
			return MICRO2DEG(tempCommand.getJointValue(jointIndex));
		}
		
		virtual void setCurrentJointCommand(JointCommand &j) = 0;

		virtual void getOdometry(double &delta_forward, double &delta_left, double &delta_turn, bool highGain) = 0;

		virtual void goNextFrame() = 0;

		virtual bool isCompleted() = 0;

		virtual int getNumPoints() = 0;

		virtual int getPointNum() = 0;
		virtual void setPointNum(int number) = 0;
		
		virtual bool usingHead() = 0;

        virtual bool isSpecialAction() = 0;
};


#endif //_BaseAction_h_DEFINED

