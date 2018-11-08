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
 * $Id: sensors.h 7169 2005-07-03 14:49:31Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef __sensors_h
#define __sensors_h

/* Enumeration for sensor values
** Note:
** 1. This is already following the Sony's standard, so dont change the values
**    of the enum or its ordering. (Kevin 10/04/04)
** 2. The range of Infrared sensors, and all the sensors locations, can be
**    read in the ERS MODEL INFORMATION, if u need them
*/

enum SensorEnum{
	ssMOUTH,        //0
	ssCHIN,
	ssHEAD_CRANE,
	ssHEAD,
	ssINFRARED_NEAR,
	ssINFRARED_FAR, //5
	ssHEAD_PAN,
	ssHEAD_TILT,
	ssFL_PALM,
	ssFL_KNEE,
	ssFL_ABDUCTOR,  //10
	ssFL_ROTATOR,
	ssRL_PALM,
	ssRL_KNEE,
	ssRL_ABDUCTOR,
	ssRL_ROTATOR,   //15
	ssFR_PALM,
	ssFR_KNEE,
	ssFR_ABDUCTOR,
	ssFR_ROTATOR,
	ssRR_PALM,      //20
	ssRR_KNEE,
	ssRR_ABDUCTOR,
	ssRR_ROTATOR,
	ssTAIL_H,
	ssTAIL_V,       //25
	ssACCEL_FOR,
	ssACCEL_SIDE,
	ssACCEL_Z,
	ssCHEST_INFRARED,
	ssWLAN,         //30
	ssBACK_REAR,
	ssBACK_MIDDLE,
	ssBACK_FRONT,
	NUM_SENSORS    // 34
};

//static const int NUM_SENSORS = 34;

#endif
