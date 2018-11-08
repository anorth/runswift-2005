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
 * $Id: ActuatorControl.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright(c) 2004 UNSW
 * All Rights Reserved.
 * 
 * handle the robot motion: head/foot walking and kicking(Locomotion)
 * 
 **/

#ifndef _ActuatorControl_h_DEFINED
#define _ActuatorControl_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/ObjcommMessages.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserverVector.h>
#include <OPENR/OPENREvent.h>
#include <OPENR/OPENRAPI.h>

#include "def.h"
#include "Indicators.h"
#include "ActionPro.h"
#include "../share/Common.h"
#include "../share/SharedMemoryDef.h"

class ActuatorControl : public OObject, public ActionPro {
	public:
		OSubject *subject[numOfSubject];

		OObserverVector *observer[numOfObserver];

		ActuatorControl();

		~ActuatorControl() {
			delete(indicatorCommander);
		}

		//  OPENR Methods
		virtual OStatus DoInit(const OSystemEvent &event);
		virtual OStatus DoStart(const OSystemEvent &event);
		virtual OStatus DoStop(const OSystemEvent &event);
		virtual OStatus DoDestroy(const OSystemEvent &event);

		//  Inter-Object Communication Methods
		void ReadyECommander(const OReadyEvent &event);
        void ReadyEffectors(const OReadyEvent &);

		void executeAssert(const ONotifyEvent &event);
		void setOffsetParams(const ONotifyEvent &event);
        void setLearningParams(const ONotifyEvent &event);

		void setIndicationAssert(const ONotifyEvent &event);    

		void ReadySetPAgent(const OReadyEvent &event);
        void ResultSensor(const ONotifyEvent &event);

		void Shutdown();
	private:
		int wait4settle;  // for debugging
		Indicators *indicatorCommander;
        
        double bufForward, bufLeft, bufTurn;
};

#endif // _ActuatorControl_h_DEFINED

