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
 * $Id: JointAgent.h 6909 2005-06-25 10:30:28Z alexn $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _JOINT_AGENT_H_
#define _JOINT_AGENT_H_

#include <Types.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserverVector.h>
#include <OPENR/OPENREvent.h>
#include <OPENR/OPENRAPI.h>

#include "EffectorCommander.h"
#include "JointLimit.h"
#include "JointCommand.h"

static const int numOfFrames = 1; //going to be deprecated
static const slongword DEFAULT_JOINT_SPEED_LIMIT = DEG2MICRO(2.0); // 2degree/8ms

class ActionPro;

class JointAgent{
	public:
		JointAgent(ActionPro *grandParent, OSubject *sEffector, EffectorCommander *eCommander);
		void setUpBuffer();
		void setNotUsingHead();
		bool isReadyForCommand();
		bool checkBufferEmpty();
		void executeCommand(JointCommand *jCommand);
        //"soft" command is command that depends on last command, sothat the different is small enough for the dog to get Over Current
		void executeSoftCommand(JointCommand *jCommand);

		//utility functions
		void logJointValues(RCRegion *cmdRegion);

        //these 2 methods are replicated from EffectorCommander,
        //TODO : merge JointAgent & EffectorCommander (or let JointAgent be subclass of EffectorCommander)
        void relaxJoints();
        void reEnableJoints();
        void setSpeedLimit(double speed);
        JointCommand lastJointCommand; //in order to send "soft" command
        void highGainJoints();
        void setJointGain ( int whichJoint, int P , int I , int D , bool highGain);
        bool isUsingHighGain();

	private:
        bool isHighGain;
		ActionPro *grandParent;
		OSubject *sEffector;
		EffectorCommander *eCommander;
		static const int numOfBuffer = 2;
		OCommandVectorData *cmdVec_[numOfBuffer];
		MemoryRegionID cmdVecID_[numOfBuffer];
		RCRegion *cmdRegion[numOfBuffer];
		int freeBufID;

        void updateLastJointCommand(JointCommand *newJointCommand);
        void updateSoftHeadJointCommand(JointCommand *newJointCommand);
        slongword jointSpeedLimit;

        //got called by executeSoftCommand & executeCommand
		void executeLastJointCommand(JointCommand *jCommand);

};

#endif // _JOINT_AGENT_H_
