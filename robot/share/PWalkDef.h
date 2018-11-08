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
 * $Id: PWalkDef.h 7601 2005-07-14 03:11:17Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#ifndef _PWalkDef_h_DEFINED
#define _PWalkDef_h_DEFINED

/* The number of walk locus end-points calculated per leg for each camera frame. */
/* Might want to try ocommandMAX_FRAME. */
//static const int numOfFrames = 5;

typedef enum {
    NormalWalkWT,   //0
    CanterWalkWT,
    ZoidalWalkWT,
    OffsetWalkWT,
    EllipseWalkWT,
    AlanWalkWT,     //5
    ChestPushWT,
    FwdFastWT,        //   from German Team
    FwdHardWT,        //   from German Team
    GetupFrontWT,
    GetupSideWT,    //10
    GetupRearWT,
    BackKickWT,
    HoldBallWT,
    CheerWT,
    DanceWT,        //15
    UpennRightWT,
    UpennLeftWT,
    BackWalkWT,
    SkiWalkWT,
    SkellipticalWalkWT, //20
    DiveKickWT,
    HandKickRightWT,
    HandKickLeftWT,
    BlockWT,            //    from German Team   
    BlockLeftWT,        //25  from German Team
    BlockRightWT,       //    from German Team
    HeadLeftWT,         //    from German Team
    HeadRightWT,        //    from German Team
    GTChestPushWT, 
    NUFwdKickWT,        //30
    BuBuFwdKickWT,
    HeadTapWT,
    SoftTapWT,
    GrabWT,
    TestKickWT         // 35  
} Walktypes;


// minor walk types
const int MWT_DEFAULT = 0;
const int MWT_RAW_COMMAND = 1;

enum skellipticalWalkMinorWalkType {
  MWT_SKE_PG22 = 2,
  MWT_SKE_FAST_FORWARD = 3,
  MWT_SKE_GRAB_DRIBBLE = 4,
  MWT_SKE_PG31 = 5,
  MWT_SKE_TURN_WALK = 6,
  MWT_SKE_NECK_TURN_WALK = 7,
  MWT_SKE_GRAB_DRIBBLE_HOLD = 8,
  MWT_SKE_OSAKA_FAST = 9
};



extern const char *walkTypeToStr[]; //is defined in CommonData.cc
extern const char *commandToStr[]; //is defined in CommonData.cc

static const int NUM_OF_EFFECTORS = 15;
static const int NUM_OF_HEAD_JOINTS = 3;

static const int posaLength = NUM_OF_EFFECTORS + 1;

typedef struct ActuatorSharedMemory_S {
    int currentWalkType;
    int currentMinorWalkType;
    int currentStep;
    int currentPG;        
    int leading_leg;
    double frontShoulderHeight;	// mm
    double bodyTilt;	// radians
    double desiredPan;
    double desiredTilt;
    double desiredCrane;
    bool isHighGain;
  unsigned int stepID;
} ActuatorSharedMemory;


//JointEnum , whenever you want to specify a joint (head,leg), use this
//Note that it started from 1, to be equivalent to the index in posa[] Array
enum JointEnum{
	nothing = 0,
	headTilt = 1,
	headPan,
	headCrane, //this is ERS7
	rightFrontJoint,
	rightFrontShoulder, //5
	rightFrontKnee,
	leftFrontJoint,
	leftFrontShoulder,
	leftFrontKnee,
	rightRearJoint, //10
	rightRearShoulder,
	rightRearKnee,
	leftRearJoint,
	leftRearShoulder,
	leftRearKnee //15
};

extern bool isSpecialAction(int walktype);

#endif // _PWalkDef_h_DEFINED

