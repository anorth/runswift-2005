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
 * $Id: PWalk.h 7601 2005-07-14 03:11:17Z weiming $
 *
 * Copyright(c) 2003 UNSW
 * All Rights Reserved.
 * 
 * contains the actual calculations used to execute the walking and kicking routines, handling the leg motion
 **/

#ifndef _PWalk_h_DEFINED
#define _PWalk_h_DEFINED

#include <iostream>
#include <cmath>

#include "../share/RobotDef.h"
#include "../share/Common.h"
#include "../share/PWalkDef.h"
#include "../share/ActionProDef.h"
#include "legsGeometry.h"
#include "JointCommand.h"
#include "SpecialAction.h"
#include "NormalWalk.h"
#include "BackWalk.h"
#include "OffsetWalk.h"
#include "EllipticalWalk.h"
#include "AlanWalk.h"
#include "SkiWalk.h"
#include "SkellipticalWalk.h"

class PWalk {
 public:
    PWalk(const long *sensorValTemp, const long *dutyCycleValTemp);
	~PWalk() {
		if (chestpush)
			delete(chestpush);
		if (fwdFast)
			delete(fwdFast);
		if (fwdHard)
			delete(fwdHard); 
		if (getupf)
			delete(getupf);
		if (getups)
			delete(getups);
		if (getupr)
			delete(getupr);
		if (backKick)
			delete(backKick);
		if (cheer)
			delete(cheer);
		if (dance)
			delete(dance);
		if (upennRight)
			delete(upennRight);
		if (upennLeft)
			delete(upennLeft);
		if (headRight)
			delete(headRight);
		if (headLeft)
			delete(headLeft);
	}

    void pStep(bool);
    void pStep(AtomicAction &newCommand);

    /* Specify whether the current walk uses the head, if it does not, player-specified head motion
       varibles will be used. */
    bool NotUsingHead();

    //For offset walk
    void readLocusOffets(const char *filename);

    //Odometer values to be sent to GPS
    double delta_forward, delta_left, delta_turn;

    JointCommand jointCommand;
    AtomicAction bufferedCommand;

    //current executing action
    BaseAction *sp;

    /* Special actions. */                

    NormalWalk *normalWalk;
    BackWalk *backWalk;
    OffsetWalk *offsetWalk;
    EllipticalWalk *ellipticalWalk;
    AlanWalk *alanWalk;
    SkiWalk *skiWalk;
    SkellipticalWalk *skellipticalWalk;
    SpecialAction *chestpush;
    SpecialAction *fwdFast;
    SpecialAction *fwdHard;

    SpecialAction *getupf;
    SpecialAction *getups;
    SpecialAction *getupr;

    SpecialAction *backKick;
    SpecialAction *cheer;        
    SpecialAction *dance;        
    SpecialAction *startup;

    SpecialAction *upennLeft;
    SpecialAction *upennRight;

    SpecialAction *hkLeft;
    SpecialAction *hkRight;

    SpecialAction *diveKick;
    
    SpecialAction *block; 
    SpecialAction *blockLeft;
    SpecialAction *blockRight;

    SpecialAction *unblock;
    SpecialAction *unblockLeft;
    SpecialAction *unblockRight;
    
    SpecialAction *headLeft; 
    SpecialAction *headRight;
    SpecialAction *gtChestPush;
    SpecialAction *nuFwdKick;
    SpecialAction *bubuFwdKick;
    
    SpecialAction *headTap; 
    SpecialAction *softTap;
    
    SpecialAction *grab;
    
    SpecialAction *testKick; 

    /* Pointer to shared variables walktype, step_ and PG, provide read-only access outside PWalk. */
    const int* ptr_walktype;
    int walktype;
    const int* ptr_minorWalkType;
    int minorWalkType;
    const int* ptr_step;
    int step;
    const int* ptr_PG;
    double stepCycle;
    int PG;
    //leading_leg is shared between walk, featuring smooth switching between them
    //the pointer is passed from ActionPro (to again,share with Behaviours)
    int *leading_leg; //* -1 = left, 1 = right, 0 = unknown */

    const double* ptr_backHipHeight;
    double backHipHeight;
    const double* ptr_frontShoulderHeight;
    double frontShoulderHeight;
    const double* ptr_bodyTilt;
    double bodyTilt;
    
    const long *sensorVal;
    const long *dutyCycleVal;
    
    bool idle;
    bool forceComplete;

    unsigned int stepID;
    bool isCommandChanged;

    bool isCompleted();

    void updateParameters();
    void reloadAllSpecialAction();

    void getAngleHeight(double &angle, double &height, double bX, double bY, double fX, double fY);
    void calcHeightTilt(const JOINT_Values &flTheta, const JOINT_Values &blTheta, const JOINT_Values &frTheta, const JOINT_Values &brTheta);
};

#endif // _PWalk_h_DEFINED

