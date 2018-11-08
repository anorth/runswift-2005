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
 * $Id: PyEmbed.h 6487 2005-06-08 11:35:39Z alexn $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/




#ifndef __PyEmbed_h_
#define __PyEmbed_h_

#include <Python.h>

#ifdef OFFLINE
//#include "../../base/offvision/behavioursOffline.h"
#else // OFFLINE
//#include <OPENR/OUnits.h>
#endif // OFFLINE

#include "../share/Common.h"
#include "../share/WirelessSwitchboard.h"
#include "CommonSense.h"
#include "VisualCortex.h"
#include "gps.h"
#include "gpsAux.h"
#include "Vector.h"
#include "Obstacle.h"

// leave this out to prevent repeated struct definitions
//#include "../share/RoboCupGameControlData.h"


#include "../share/IndicatorsDef.h"
#include "../share/PWalkDef.h"
#include "../share/ActionProDef.h"
#include "../share/VisionDef.h"
#include "../share/SwitchBoard.h"
#include "../share/CommandData.h"

#ifndef OFFLINE
#include "Vision.h"
class Vision;    // forward declaration of Vision class
#endif // OFFLINE

namespace PyBehaviours {
	extern void initializeBehaviours();
	extern void initPWMObj(PyObject *VisionLinkModule);
	extern void initPVector(PyObject *VisionLinkModule);

	extern void processFrame();
	extern void processCommand(const char* cmd, int length);
	extern void reloadModule();
	extern char* evaluateExpression(const char *expression);

	extern void runString(char *cmd);

	// extern PyObject *newPWMObj(const WMObj *o);
	// extern PyObject *newPVector(const Vector *v);

#ifndef OFFLINE
	extern Vision *transmission;
#endif // OFFLINE
	extern VisualCortex *vision;
	extern CommonSense *sensors;
	extern GPS *gps;
    extern Obstacle *obstacle;
	extern struct PWalkInfoStruct *PWalkInfo;
	extern bool shareState;
}

/*
extern "C" {
	typedef struct {
		PyObject_HEAD
			const WMObj *o;
	} PWMObj;

	typedef struct {
		PyObject_HEAD
			const Vector *v;
	} PVector;
}
*/

#endif // __PyEmbed_h_
