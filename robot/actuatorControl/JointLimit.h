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
 * $Id: JointLimit.h 6302 2005-05-31 09:03:44Z nmor250 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _JOINT_LIMIT_H_
#define _JOINT_LIMIT_H_

#ifdef OFFLINE
#include "OfflineOPENR.h"
#else
#include <OPENR/OPENRAPI.h>
#include <OPENR/RCRegion.h>
#endif //OFFLINE

#include "SpecialAction.h"
#include "JointCommand.h"

//Print out to the screen if joint value is out of the limit
static const bool debugJointLimit = false;


/* the magnitude (not sign) should agree with base/work/RoboCommander/RoboConstant.java */
/* Limit values are defined in Model_Information_7_E.pdf */
static const double MIN_TILT = -80.0;
static const double MAX_TILT = 3.0;

static const double MIN_PAN = -93.0;
static const double MAX_PAN = 93.0;

static const double MIN_CRANE = -20;
static const double MAX_CRANE = 50.0;

static const double MIN_FRONT_JOINT = -115.0;  
static const double MAX_FRONT_JOINT = 130.0;

static const double MIN_REAR_JOINT = -130.0; 
static const double MAX_REAR_JOINT = 115.0;

static const double MIN_SHOULDER = -10.0;
static const double MAX_SHOULDER = 88.0;

static const double MIN_KNEE = -25.0;
static const double MAX_KNEE = 125.0;

static const double MIN_TAIL_TILT = 5.0;
static const double MAX_TAIL_TILT = 60.0;

static const double MIN_TAIL_PAN = -45.0;
static const double MAX_TAIL_PAN = 45.0;

static const double LOWER_JOINT_LIMIT[ NUM_OF_EFFECTORS + 1] = {
    0,
    MIN_TILT, MIN_PAN, MIN_CRANE, 
    MIN_FRONT_JOINT, MIN_SHOULDER, MIN_KNEE, 
    MIN_FRONT_JOINT, MIN_SHOULDER, MIN_KNEE, 
    MIN_REAR_JOINT, MIN_SHOULDER, MIN_KNEE, 
    MIN_REAR_JOINT, MIN_SHOULDER, MIN_KNEE, 
};

static const double UPPER_JOINT_LIMIT[ NUM_OF_EFFECTORS + 1] = {
    0,
    MAX_TILT, MAX_PAN, MAX_CRANE, 
    MAX_FRONT_JOINT, MAX_SHOULDER, MAX_KNEE, 
    MAX_FRONT_JOINT, MAX_SHOULDER, MAX_KNEE, 
    MAX_REAR_JOINT, MAX_SHOULDER, MAX_KNEE, 
    MAX_REAR_JOINT, MAX_SHOULDER, MAX_KNEE, 
};

//value is in MICRO RAD unit
extern bool checkJoint(JointEnum joint, slongword value);

//below are overloading functions that do the checks on OCommandData 
extern bool checkJoint(JointEnum joint, int nframes, OCommandData *data);


//value is in MICRO RAD unit
extern void clipRange(slongword &value, double minDeg, double maxDeg);

//clip bad joint value sothat it's within the AIBO's limits
extern void clipJointValue(JointEnum joint, slongword &value);

extern JointEnum badJointLimit(RCRegion *cmdRegion);
extern JointEnum badJointLimit(JointCommand *jCommand);

extern void clipBadJointLimit(RCRegion *cmdRegion);
extern void clipBadJointLimit(JointCommand *jCommand);



#endif // _JOINT_LIMIT_H_
