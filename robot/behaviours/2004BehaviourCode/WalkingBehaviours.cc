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
 * $Id: WalkingBehaviours.cc 5072 2005-03-10 22:59:09Z alexn $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



/*
 * define all procedure related to walking behaviours (closely related to PWalk)
 */
 
#include "WalkingBehaviours.h"

//only initialize PG, hF,hB, hdF,hdB, ffo, fso, bfo, bso
AtomicAction NormalWalkFast             = {0,0,0,0,0  , 40,90,120,10,25,55,15,-55,5,  0,0,0,0,0,0};
AtomicAction NormalWalkOmniDirection    = {0,0,0,0,0  , 40,90,120,10,25,55,10,-55,10,  0,0,0,0,0,0};
AtomicAction NormalWalkSmallStep    = {0,0,0,0,0  , 32,95,110,20,25,55,10,-55,10,  0,0,0,0,0,0};


// Kevin: Previously PG = 5 in stand, but why? 
// Kim: because it lets the dog switch to other walk with less delay (delay = PG)
void Behaviours::setStandParams() {
    walkType = NormalWalkWT;
    forward = 0;
    left = 0;
    turnCCW = 0;
    PG = 5;
    hF = NormalWalkOmniDirection.hF;
    hB = NormalWalkOmniDirection.hB ;
    hdF = 0;
    hdB = 0;
    ffO = NormalWalkOmniDirection.ffO;
    fsO = NormalWalkOmniDirection.fsO;
    bfO = NormalWalkOmniDirection.bfO;
    bsO = NormalWalkOmniDirection.bsO;
}

void Behaviours::setWalkParams() {
    walkType = NormalWalkWT;
    PG = NormalWalkFast.speed;
    hF = NormalWalkFast.hF;
    hB = NormalWalkFast.hB;
    hdF = NormalWalkFast.hdF;
    hdB = NormalWalkFast.hdB;
    ffO = NormalWalkFast.ffO;
    fsO = NormalWalkFast.fsO;
    bfO = NormalWalkFast.bfO;
    bsO = NormalWalkFast.bsO;
}

void Behaviours::setCanterParams() {
    // Hack into Normal WalkType
    setWalkParams();
}

void Behaviours::setOffsetParams() {
    // Hack into Normal WalkType
    setWalkParams(); 
}

void Behaviours::setOmniParams() {
    walkType = NormalWalkWT; //CanterWalkWT;
    PG = NormalWalkOmniDirection.speed;
    hF = NormalWalkOmniDirection.hF; 
    hB = NormalWalkOmniDirection.hB;
    hdF = NormalWalkOmniDirection.hdF;
    hdB = NormalWalkOmniDirection.hdB;
    ffO = NormalWalkOmniDirection.ffO;
    fsO = NormalWalkOmniDirection.fsO;
    bfO = NormalWalkOmniDirection.bfO;
    bsO = NormalWalkOmniDirection.bsO;
}

// Kevin: Previously PG = 5 in stand, but why?
void Behaviours::setBlockParams() {
    walkType = NormalWalkWT; //CanterWalkWT;
    forward = 0;
    left = 0;
    turnCCW = 0;
    PG = 40;
    hF = 30;  // height of shoulder
    hB = 70;  // height of hip
    hdF = 0;   // how high front paw is lifted when walking
    hdB = 0;   // how high back paw is lifted when walking
    ffO = 60;  //ffO     = 100; // how far forward the front paws are
    fsO = 110; //fsO     = 70; // how far outwards from the body the front paws are
    bfO = -50; // how far forward the back paws are
    bsO = 15;  // how far outwards from the body the back paws are
}

// Kevin: Previously PG = 5 in stand, but why?
void Behaviours::setHoldParams() {
    walkType = NormalWalkWT; //CanterWalkWT;
    forward = 0;
    left = 0;
    turnCCW = 0;
    PG = 40;
    hF = HF_STANDARD;
    hB = HB_STANDARD;
    hdF = 0;
    hdB = 0;
    ffO = 100;
    fsO = -12;
    bfO = -55;
    bsO = 15;
}

void Behaviours::setHoldSideWalkParams() {
    walkType = NormalWalkWT; // CanterWalkWT
    forward = 0;
    left = 0;
    turnCCW = 0;
    PG = 40;
    hF = HF_STANDARD;
    hB = HB_STANDARD;
    hdF = 5;
    hdB = 25;
    ffO = 85;
    fsO = 15;
    bfO = -55;
    bsO = 15;
}
/*	static inline void makeTurnWithBall(double turnCCW) {
		makeParaWalk(CanterWalkWT,
				0,-turnCCW / 6.0,turnCCW,
				40,70,110,5,25,	100,-12,-55,15);
	}
*/
void Behaviours::setCarryParams() {
    walkType = NormalWalkWT; //CanterWalkWT;
    PG = 40;
    hF = HF_STANDARD;
    hB = HB_STANDARD;
    hdF = 0;
    hdB = 30;
    ffO = 97.5;
    fsO = -10;//-15;
    bfO = -55;
    bsO = 15;
}


// ATTENTION: VERY SERIOUS

// 1. I write this to replace any action that holds the ball and turn,
// it avoids using turn & left together, (as in the 210 does), so it
// wont crash the dog and dose it fast

// 2. Make sure the following vaiables are not re-written again later 
// in the loop, or make sure it is the last function that sets this
// variables in the behaviors frame. otherwise it doesnt work and 
// dont blame, simply  donno how to use it ^_^

void Behaviours::setTurnKickParams(double turnccw) {
    walkType = NormalWalkWT;

    PG = PG_STANDARD;
    hF = HF_STANDARD;
    hB = HB_STANDARD;
    hdF = 5;
    hdB = 30;
    ffO = 70;
    fsO = 0;
    bfO = BFO_STANDARD;
    bsO = BSO_STANDARD;

    forward = 0;
    left    = 0;
    turnCCW = turnccw;

    headtype = ABS_H;

    panx    = 0;
    tilty   = -60;
    cranez  = 30;
}


void Behaviours::setRotateParams() {
    walkType = NormalWalkWT; //CanterWalkWT;
    forward = 0;
    left = 0;
    turnCCW = -2;
    PG = 40;
    hF = HF_STANDARD;
    hB = HB_STANDARD;
    hdF = 5;
    hdB = 30;
    ffO = 55;
    fsO = 15;
    bfO = -55;
    bsO = 15;
}


