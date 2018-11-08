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
 * $Id: CommonData.cc 6988 2005-06-28 09:08:24Z nmor250 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



/*
 * This file defines all the variable that is shared by all the modules
 * Notes :  
 *  1. These variables must have got extern definition in some shared header files
 *  2. Dont use static, as it will get gcc warning "unused variables"
 *
 */

const char *headTypeStr[] = {
    "NULL_H",
    "REL_H",
    "ABS_H",
    "ABS_PT",
    "ABS_XYZ",
    "ABS_XYZ_HACK",
    "ABS_Z"
};

const char *walkTypeToStr[] = {
	"NormalWalk",
	"CanterWalkWT",
	"ZoidalWalkWT",
	"OffsetWalkWT",
	"EllipticalWalkWT",
	"AlanWalkWT",
	"ChestPushWT",
	"FwdKickWT",
	"FastKickWT",
	"GetupFrontWT",
	"GetupSideWT",
	"GetupRearWT",
	"BackKickWT",
	"HoldBallWT",
	"CheerWT",
	"DanceWT",
	"UpennRightWT",
	"UpennLeftWT",
	"BackWalkWT",
	"SkiWalkWT",
	"SkellipticalWalkWT",
	"DiveKickWT",
	"HandKickRightWT",
	"HandKickLeftWT",
        "BlockWT",
        "BlockLeftWT",
        "BlockRightWT",
        "HeadLeftWT",
        "HeadRightWT",
        "GTChestPushWT",
        "NUFwdKickWT",
        "BuBuFwdKickWT",
        "HeadTapWT",
        "SoftTapWT",
        "TestKickWT"
};

const char *commandToStr[] = {
  "aaStop",
  "aaStill",
  "aaTailUp",
  "aaParaWalk" ,
  "aaRelaxed",
  "aaTurnOnJointDebug",
  "aaSetJointSpeedLimit",
  "aaSetAJointGain",
  "aaSetHighGain",
  "aaSetLowGain",
  "aaReloadSpecialAction",
  "aaForceStepComplete"
};
