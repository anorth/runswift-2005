
/*

   Copyright 2003 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
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
 * $Id
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 *
 **/

#ifndef _JoystickSkills_h_DEFINED
#define _JoystickSkills_h_DEFINED

#include "Behaviours.h"

using namespace Behaviours;

namespace JoystickSkills {
	// this is needed to get rid of ambiguity 
	// because std namespace also has a variable called left
	using Behaviours::left;

	extern void grabBall();
	extern void front();

	extern void lightningJK();
	extern void chestPushJK();
	extern void tk90LeftJK();
	extern void tk90RightJK();
	extern void tk180LeftJK();
	extern void tk180RightJK();
	extern void blockJK();

	extern bool isBallKickable();
	extern void setBallTrk(bool b);
	extern void jsMaintenance();
	extern void jsGeneralHead();
	extern void remoteHeadFindBall(double hightilt = -10, double lowtilt = -30);
	extern void hoverCrop(double f,
			double l,
			double t,
			double *fr,
			double *lr,
			double *tr,
			int *wt);

	typedef enum {
		nothingJS,
		lightningJS,
		chestPushJS,
		tk90LeftJS,
		tk90RightJS,
		tk180LeftJS,
		tk180RightJS,
		blockJS
	} JoystickState;

	extern JoystickState jsState;
}

#endif // _JoystickSkills_h_DEFINED

