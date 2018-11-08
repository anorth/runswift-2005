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
 * $Id: ActionProDef.h 6483 2005-06-08 10:25:35Z nmor250 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#ifndef _ActionProDef_h_DEFINED
#define _ActionProDef_h_DEFINED

#include <iostream>

using std::cout;
using std::endl;

enum AtomicActionCommandEnum{
	aaStop = 0 ,
	aaStill,
	aaTailUp ,
	aaParaWalk ,  // dp1 forward(cm), dp2 left(cm), // dp3 turn counterclockwise(deg)
	aaRelaxed,
	aaTurnOnJointDebug,         //5
	aaSetJointSpeedLimit,
	aaSetAJointGain,
	aaSetHighGain,
	aaSetLowGain,
	aaReloadSpecialAction,      //10
        aaForceStepComplete
};

// constants for headType
enum HeadType{
    NULL_H = 0,
    REL_H = 1, //move head relative to where the head already is
    ABS_H = 2, //move head relative to where the body is
    ABS_PT = 3, // move head relative to body with pan then tilt
    ABS_XYZ = 4, // move head relative to point on ground under neck in XYZ coords
    ABS_XYZ_HACK = 5, // same as ABS_XYZ, but clips the maximum tilt to some deg 
    REL_Z = 6 // move head using relative cranez, absolute panx,tilty
};

extern  char *headTypeStr[6]; //is defined in CommonData.cc

// for actionSuccess
static const int execFAIL = 0;
static const int execHEAD = 1;
static const int execLEGS = 2;
static const int execALL = 3;

extern const char *walkTypeToStr[]; //is defined in CommonData.cc
extern const char *commandToStr[]; //is defined in CommonData.cc

struct AtomicAction {
	long cmd;
	// parameters for parameterised walks

    /* Specifies the current walktype, or the walk locus to be calculated. */

	int walkType;
        int minorWalkType;

    /* Specifies the distance in cm that the robot should move forward in each half-step.
       Negative values move the robot backwards. */

	double forwardSpeed;

    /* Specifies the distance in cm that the robot should move to the left in each half-step.
       Negative values move the robot backwards. */

	double leftSpeed;

    /* Specifies the angle in degrees that the robot should turn counter clockwise per half-step.
       Negative values move the robot clockwise. */

	double turnCCWSpeed;

        double forwardMaxStep; 
        double leftMaxStep; 
        double turnCCWMaxStep;


    //int speed;

    /* Specifies the height of the shoulder and hip joints from the ground, see below for brief descriptions. */   
//	double hF;
//	double hB;

    /* Specifies the height of the walk locus of the front and back paws, see below for brief descriptions. */

//	double hdF;
//	double hdB;

    /* Home positions of front and back paws, see below for brief descriptions. */

	//double ffO;
	//double fsO;
	//double bfO;
	//double bsO;
	int headType;
	double panx;
	double tilty;
	double cranez;
	// end parameters for para walk

	long step;      // internal state
	bool finished;
	/*
	   void Print () {
	   cout << "Command " << ((char) (cmd >> 24)) << ((char) (cmd >> 16))
	   << ((char) (cmd >> 8)) << ((char) (cmd)) << " params " 
	   << param1 << " and " << param2 << endl;
	   }
       */


    void printFull() const {
      cout <<
	walkTypeToStr[walkType] <<
	"  fwdSpeed:" << forwardSpeed <<
	"  lftSpeed:" << leftSpeed <<
	"  trnSpeed:" << turnCCWSpeed <<
        "  fwdMaxStep:" << forwardMaxStep << 
        "  lftMaxStep:" << leftMaxStep << 
        "  trnMaxStep:" << turnCCWMaxStep << 
	//"  speed:" << speed <<
	//"  hF:" << hF <<
	//"  hB:" << hB <<
	//"  hdF:" << hdF <<
	//"  hdB:" << hdB <<
	//"  ffO:" << ffO <<
	//"  fsO:" << fsO <<
	//"  bfO:" << bfO <<
	//"  bsO:" << bsO <<
	"  " << headTypeStr[headType] <<
	"  tilt:" << tilty <<
        "  pan:" << panx <<
        "  crane:" << cranez << endl;
    }
};

#endif // _ActionProDef_h_DEFINED
