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
 * $Id: HeadMotion.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 * 
 * Head Controls : 
 *      Handle various type of head control, use setCurrentJointCommand to set 
 *      the JointCommand ( which , then is sent to OVirtualComm by ActionPro.cc
 * Note that this is offline compatible.
 * (Historical notes: This used to send command directly to OVirtualComm, but it might duplicate the command, and conflict with the one sent by ActionPro. From now on, only ActionPro does that job. Kim(18/03/04)
 *
 **/

#ifndef _HeadMotion_h_DEFINED
#define _HeadMotion_h_DEFINED


#include "primitives.h"
#include "../share/Common.h"
#include "../share/ActionProDef.h"
#include "../share/PWalkDef.h"
#include "JointLimit.h"



class HeadMotion {
	private:
		double destTilt, destPan, destCrane;
		double *desiredTilt;
		double *desiredPan;
        double *desiredCrane;
		const double *frontShoulderHeight;
		const double *bodyTilt;

	public:
		HeadMotion( double *dPan,
				double *dTilt,
			        double *dCrane,
			        const double *frontShoulderHeight,
			        const double *bodyTilt);
		~HeadMotion() {
		}

		/*
		 * @param headType is the type of head movement, either ABS_H (absolute), REL_H (relative), or NUL_H, (null)
		 * @param tilt is the vertical angle of head in degrees. Up is positive
		 * @param pan is the horizontal angle of head in degrees. Left is positive
		 **/
		void SendHeadCommand(int headType,
				double panx,
				double tilty,
				double cranez);
        void setCurrentJointCommand(JointCommand &jointCommand);
};

#endif //_HeadMotion_h_DEFINED

