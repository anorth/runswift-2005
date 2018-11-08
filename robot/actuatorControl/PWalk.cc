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
 * $Id: PWalk.cc 7648 2005-07-16 00:55:07Z weiming $
 *
 * Copyright(c) 2004 UNSW
 * All Rights Reserved.
 * 
 **/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <math.h>
#include "PWalk.h"
#include "../share/OdometerDef.h"
#include "legsGeometry.h"
#include "../share/sensors.h"

using namespace std;


static const bool bDebugWalkCommand = false;
static const bool bDebugPSTEP = false;
static const bool bDebugAll = false;
static const bool useSensorsForHeightTilt = true;

#ifdef OFFLINE
static string POS_FILE_PATH = "pos";
#else
static string POS_FILE_PATH = "/MS/pos";
#endif


static const string CHESH_PUSH_FILE = POS_FILE_PATH + "/cpush.pos";
static const string FWDFAST_FILE = POS_FILE_PATH + "/fwdFast.pos";
static const string FWDHARD_FILE = POS_FILE_PATH + "/fwdHard.pos";
static const string GETUP_F_FILE = POS_FILE_PATH + "/getupf.pos";
static const string GETUP_S_FILE = POS_FILE_PATH + "/getups.pos";
static const string GETUP_R_FILE = POS_FILE_PATH + "/getupr.pos";
static const string BACKKICK = POS_FILE_PATH + "/bckkick.pos";
static const string CHEER_FILE = POS_FILE_PATH + "/cheer.pos";
static const string DANCE_FILE = POS_FILE_PATH + "/dance.pos";
static const string UPENNRIGH_FILE = POS_FILE_PATH + "/UPright.pos";
static const string UPENNLEFT_FILE = POS_FILE_PATH + "/UPleft.pos";

static const string HANK_KICK_RIGHT_FILE = POS_FILE_PATH + "/HKright.pos";
static const string HANK_KICK_LEFT_FILE = POS_FILE_PATH + "/HKleft.pos";
static const string DIVE_KICK_FILE = POS_FILE_PATH + "/divekick.pos";

static const string BLOCK_FILE = POS_FILE_PATH + "/block.pos"; 
static const string BLOCK_LEFT_FILE = POS_FILE_PATH + "/blockL.pos"; 
static const string BLOCK_RIGHT_FILE = POS_FILE_PATH + "/blockR.pos";

static const string UNBLOCK_FILE = POS_FILE_PATH + "/unblock.pos"; 
static const string UNBLOCK_LEFT_FILE = POS_FILE_PATH + "/unblockL.pos";
static const string UNBLOCK_RIGHT_FILE = POS_FILE_PATH + "/unblockR.pos";

static const string HEAD_LEFT_FILE = POS_FILE_PATH + "/headL.pos"; 
static const string HEAD_RIGHT_FILE = POS_FILE_PATH + "/headR.pos";

static const string GT_CHEST_PUSH_FILE = POS_FILE_PATH + "/GTChest.pos";
static const string NU_FWD_KICK_FILE = POS_FILE_PATH + "/NUFwd.pos";
static const string BUBU_FWD_KICK_FILE = POS_FILE_PATH + "/BuBuFwd.pos";

static const string HEAD_TAP_FILE = POS_FILE_PATH + "/headTap.pos"; 
static const string SOFT_TAP_FILE = POS_FILE_PATH + "/softTap.pos"; 

static const string TEST_KICK_FILE = POS_FILE_PATH + "/testKick.pos"; 

static const string STARTUP_FILE = POS_FILE_PATH + "/startup.pos";

static const string GRAB_FILE = POS_FILE_PATH + "/grab.pos";

/*
 * PWalk
 */
PWalk::PWalk(const long *sensorValTemp, const long *dutyCycleValTemp)
        : jointCommand() {
    //cout << "This is PWalk constructor" << endl;

    cout << "Am I a new 2005 Dog? " << (isNewDog2005?"YES":"NO") << endl;
  
    sp = NULL;
    idle = false;
    forceComplete = false;

    stepID = 0;
    isCommandChanged = false;

    walktype = -1;
    bufferedCommand.walkType = -1;

    delta_forward = delta_left = delta_turn = 0;

    sensorVal = sensorValTemp;
    dutyCycleVal = dutyCycleValTemp;

    /* Special actions. */
    chestpush = new SpecialAction();
    chestpush->load(CHESH_PUSH_FILE);    
    fwdFast = new SpecialAction();
    fwdFast->load(FWDFAST_FILE);
    fwdHard = new SpecialAction();
    fwdHard->load(FWDHARD_FILE);
    getupf = new SpecialAction();
    getupf->load(GETUP_F_FILE);
    getups = new SpecialAction();
    getups->load(GETUP_S_FILE);
    getupr = new SpecialAction();
    getupr->load(GETUP_R_FILE);
    backKick = new SpecialAction();
    backKick->load(BACKKICK);
    cheer = new SpecialAction();
    cheer->load(CHEER_FILE);
    dance = new SpecialAction();
    dance->load(DANCE_FILE);
    startup = new SpecialAction();
    startup->load(STARTUP_FILE);

    upennRight = new SpecialAction();    
    upennRight->load(UPENNRIGH_FILE);
    upennRight->setOdometry(9.5, 5, 90/4.0);    

    upennLeft = new SpecialAction();
    upennLeft->load(UPENNLEFT_FILE);
    upennLeft->setOdometry(10, -6, 90/2.5);

    hkLeft = new SpecialAction();
    hkLeft->load(HANK_KICK_LEFT_FILE);
    hkRight = new SpecialAction();
    hkRight->load(HANK_KICK_RIGHT_FILE);
    diveKick = new SpecialAction();
    diveKick->load(DIVE_KICK_FILE);

    block = new SpecialAction(); 
    block->load(BLOCK_FILE);
    block->setOdometry(-2.5, 0, 0);

    blockLeft = new SpecialAction(); 
    blockLeft->load(BLOCK_LEFT_FILE);
    blockLeft->setOdometry(0, 0, 18);

    blockRight = new SpecialAction(); 
    blockRight->load(BLOCK_RIGHT_FILE);
    blockRight->setOdometry(0, 0, -18);

    unblock = new SpecialAction(); 
    unblock->load(UNBLOCK_FILE);
    
    unblockLeft = new SpecialAction(); 
    unblockLeft->load(UNBLOCK_LEFT_FILE);
    unblockLeft->setOdometry(0, 0, -18);
    
    unblockRight = new SpecialAction(); 
    unblockRight->load(UNBLOCK_RIGHT_FILE);
    unblockRight->setOdometry(0, 0, 18);

    grab = new SpecialAction(); 
    grab->load(GRAB_FILE);

    headLeft = new SpecialAction(); 
    headLeft->load(HEAD_LEFT_FILE);
    headRight = new SpecialAction(); 
    headRight->load(HEAD_RIGHT_FILE); 
    gtChestPush = new SpecialAction(); 
    gtChestPush->load(GT_CHEST_PUSH_FILE);
    nuFwdKick = new SpecialAction();
    nuFwdKick->load(NU_FWD_KICK_FILE);
    bubuFwdKick = new SpecialAction();
    bubuFwdKick->load(BUBU_FWD_KICK_FILE);
    headTap = new SpecialAction(); 
    headTap->load(HEAD_TAP_FILE);
    softTap = new SpecialAction(); 
    softTap->load(SOFT_TAP_FILE);
    testKick = new SpecialAction(); 
    testKick->load(TEST_KICK_FILE);

    normalWalk = new NormalWalk();
    backWalk = new BackWalk();
    offsetWalk = new OffsetWalk();
    ellipticalWalk = new EllipticalWalk();
    alanWalk = new AlanWalk();
    skiWalk = new SkiWalk(this);
    skellipticalWalk = new SkellipticalWalk();


    /* Pointer to shared variables walktype, step_ and PG, provide read-only
     * access outside PWalk. */
    ptr_walktype = &walktype;
    ptr_minorWalkType = &minorWalkType;
    ptr_step = &step;
    ptr_PG = &PG;
    ptr_frontShoulderHeight = &frontShoulderHeight;
    ptr_bodyTilt = &bodyTilt;

    sp = startup;
    sp->setPointNum( 0 );

#ifdef OFFLINE
    readLocusOffets("cfg/offsets.txt");   
#else
    readLocusOffets("/MS/offsets.txt");
#endif    
}

//read parameters that's set by vision
void PWalk::updateParameters(){
    if (bDebugPSTEP)
        cout << "wt = " << walktype << " completed = " << isCompleted() << " " << bufferedCommand.walkType << endl;
#ifndef OFFLINE
    if ( !isCompleted() ) {// isCompleted tells whether or not the step can be changed
    //if ( bufferedCommand.walkType != UpennLeftWT  && bufferedCommand.walkType != UpennRightWT && ! isCompleted() ){ 
        return ;
    }
#endif
    
    if (bDebugWalkCommand) {
        cout << "walk " << bufferedCommand.walkType << " " 
            << bufferedCommand.forwardSpeed << " " 
            << bufferedCommand.leftSpeed << " " 
            << bufferedCommand.turnCCWSpeed << " " << endl;
    }
    

    // if we were blocking, unblock first unless forceComplete
    if (!forceComplete) {
      if (sp == block && bufferedCommand.walkType != BlockWT) {
	sp = unblock;
	return;
      } else if (sp == blockLeft && bufferedCommand.walkType != BlockLeftWT) {
	sp = unblockLeft;
	return;
      } else if (sp == blockRight && bufferedCommand.walkType != BlockRightWT) {
	sp = unblockRight;
	return;
      } 
    }

    walktype = bufferedCommand.walkType;
    minorWalkType = bufferedCommand.minorWalkType;

    BaseAction *prevStep = sp;
	switch (walktype){
        case -1:
            // haven't receieved a command yet - do nothing
            sp = NULL;
            break;
        case NormalWalkWT:
        case CanterWalkWT:
            normalWalk->setParameters(bufferedCommand);
            sp = normalWalk;
            break;
        case BackWalkWT:
            backWalk->setParameters(bufferedCommand);
            sp = backWalk;
            break;
        case OffsetWalkWT:
            offsetWalk->setParameters(bufferedCommand);
            sp = offsetWalk;
            break;
        case EllipseWalkWT:
            ellipticalWalk->setParameters(bufferedCommand);
            sp = ellipticalWalk;
            break;
        case AlanWalkWT:
            alanWalk->setParameters(bufferedCommand);
            sp = static_cast<SpecialAction*>(alanWalk);
            break;
        case SkiWalkWT:
	    skiWalk->setParameters(bufferedCommand);
            sp = skiWalk;
            break;
        case SkellipticalWalkWT:
	    skellipticalWalk->setParameters(bufferedCommand);
            sp = skellipticalWalk;
            break;
        case ChestPushWT:
            sp = chestpush;
            break;
        case FwdFastWT:
            sp = fwdFast;
            break;
        case FwdHardWT:
	        sp = fwdHard;
	        break;
        case GetupFrontWT:
	        sp = getupf;
	        break;
        case GetupSideWT:
	        sp = getups;
	        break;
        case GetupRearWT:
	        sp = getupr;
	        break;
        case BackKickWT:
	        sp = backKick;
	        break;
        case CheerWT:
	        sp = cheer;
	        break;
        case DanceWT:
            sp = dance;
            break;
        case UpennRightWT:
            sp = upennRight;
            break;
        case UpennLeftWT:
            sp = upennLeft;
            break;
        case DiveKickWT:
            sp = diveKick;
            break;
        case HandKickRightWT:
            sp = hkRight;
            break;
        case HandKickLeftWT:
            sp = hkLeft;
            break;
        case BlockWT:
            sp = block;
            break;
        case BlockLeftWT: 
            sp = blockLeft;
            break; 
        case BlockRightWT: 
            sp = blockRight; 
            break;
        case HeadLeftWT: 
            sp = headLeft; 
            break; 
        case HeadRightWT: 
            sp = headRight; 
            break;
        case GTChestPushWT:
            sp = gtChestPush; 
            break;
        case NUFwdKickWT:    
            sp = nuFwdKick;
            break;
        case BuBuFwdKickWT:
            sp = bubuFwdKick;    
            break;
        case HeadTapWT:
            sp = headTap; 
            break; 
        case SoftTapWT: 
            sp = softTap; 
            break;
	case GrabWT:
	  sp = grab;
	  break;
        case TestKickWT: 
            sp = testKick; 
            break;
        default:
            cout << "Unknown walktype " << walktype << endl;
			sp = NULL;
			break;
	}

	if (sp != NULL) {
	  PG = sp->getNumPoints();	  
	  if (prevStep != NULL && prevStep != sp){ 
	    //if the walk is switched, set the appropiate step cycle 
	    //(smooth switching, 0 percentage = right front hand moving forward)
            if (!sp->isSpecialAction())
	      if (prevStep->isSpecialAction() ){
		sp->setPointNum( 0 );
		stepID++;
		//cout << "set cycle 0" << endl;
	      } else {
		sp->setPointNum( int(stepCycle * PG) );
		//cout << "set cycle " << int(stepCycle * PG) <<  endl;
	      }
	  }
	}
}


//continuing steps, is called when OPENR is ready to receive command
void PWalk::pStep(bool highGain){
  idle = false;
  if (bDebugPSTEP)
    cout << "pStep() " << walktype << " PG = " << PG << endl;
  
  updateParameters();
  jointCommand.reset();

  if (sp == NULL) {
    if (bDebugAll) 
      cout << "sp is NULL in pStep where it shouldn't be" << endl;
    return;
  }

  int PG = sp->getNumPoints();
  if (PG > 5 && sp->getPointNum() == PG)
    stepID++;
  isCommandChanged = false;


  if (sp->isCompleted()){
    sp->reset();
    forceComplete = false;
  }


  sp->setCurrentJointCommand(jointCommand);
  sp->goNextFrame();
  sp->getOdometry(delta_forward, delta_left, delta_turn, highGain);
  step = sp->getPointNum();
  if (!sp->isSpecialAction() ) //if it is a walk, memorize where it is in the cycle (percentage)
    {
      stepCycle = step * 1.0 / sp->getNumPoints();
    }

    if (useSensorsForHeightTilt) {
      // N.B. Sensors will be set from previous joint angles when offline
      JOINT_Values flTheta = {MICRO2RAD(sensorVal[ssFL_ROTATOR]),
			      MICRO2RAD(sensorVal[ssFL_ABDUCTOR]),
			      MICRO2RAD(sensorVal[ssFL_KNEE])};
      JOINT_Values blTheta = {MICRO2RAD(sensorVal[ssRL_ROTATOR]),
			      MICRO2RAD(sensorVal[ssRL_ABDUCTOR]),
			      MICRO2RAD(sensorVal[ssRL_KNEE])};
      JOINT_Values frTheta = {MICRO2RAD(sensorVal[ssFR_ROTATOR]),
			      MICRO2RAD(sensorVal[ssFR_ABDUCTOR]),
			      MICRO2RAD(sensorVal[ssFR_KNEE])};
      JOINT_Values brTheta = {MICRO2RAD(sensorVal[ssRR_ROTATOR]),
			      MICRO2RAD(sensorVal[ssRR_ABDUCTOR]),
			      MICRO2RAD(sensorVal[ssRR_KNEE])};
      
      calcHeightTilt(flTheta, blTheta, frTheta, brTheta);
    } else {
      JOINT_Values flTheta = {jointCommand.getJointValueInRads(leftFrontJoint-1),
			      jointCommand.getJointValueInRads(leftFrontShoulder-1),
			      jointCommand.getJointValueInRads(leftFrontKnee-1)};
      JOINT_Values blTheta = {jointCommand.getJointValueInRads(leftRearJoint-1),
			      jointCommand.getJointValueInRads(leftRearShoulder-1),
			      jointCommand.getJointValueInRads(leftRearKnee-1)};
      JOINT_Values frTheta = {jointCommand.getJointValueInRads(rightFrontJoint-1),
			      jointCommand.getJointValueInRads(rightFrontShoulder-1),
			      jointCommand.getJointValueInRads(rightFrontKnee-1)};
      JOINT_Values brTheta = {jointCommand.getJointValueInRads(rightRearJoint-1),
			      jointCommand.getJointValueInRads(rightRearShoulder-1),
			      jointCommand.getJointValueInRads(rightRearKnee-1)};
      
      calcHeightTilt(flTheta, blTheta, frTheta, brTheta);
    }
	
    //FFCOUT << jointCommand << endl;
}

void PWalk::pStep(AtomicAction &newCommand){
  if (bDebugPSTEP){
    cout << "buffer is updated old WT=" << bufferedCommand.walkType << " new WT=" << newCommand.walkType << endl;
  }
  
  if (memcmp((void*)&bufferedCommand, (void*)&newCommand, sizeof(bufferedCommand)) != 0)
    isCommandChanged = true;

  bufferedCommand = newCommand;


  if (bufferedCommand.cmd == aaForceStepComplete)
    forceComplete = true;
  else
    forceComplete = false;

  if (idle){
    cout << "PWalk is Idle, weird, please tell Kim ";
    pStep(true);
  }
  idle = true;
}


bool PWalk::isCompleted(){
    if (sp == NULL)
        return true;
    if ( forceComplete && 
	 ( !sp->isSpecialAction() || sp == block || sp == blockLeft || sp == blockRight)
	 )
        return true; // we dont want to force special action unless it's a block.
    return sp->isCompleted();
}

void PWalk::readLocusOffets(const char *filename){
  offsetWalk->readLocusOffets(filename);
}

bool PWalk::NotUsingHead(){
    if (sp == NULL)
        return false;
    else
        return !sp->usingHead();
}

void PWalk::reloadAllSpecialAction() {
chestpush->reload();
fwdFast->reload();
    fwdHard->reload();
    getupf->reload();
    getups->reload();
    getupr->reload();
    backKick->reload();
    cheer->reload();        
    dance->reload();        
    upennLeft->reload();
    upennRight->reload();
    hkRight->reload();
    hkLeft->reload();
    diveKick->reload();
    block->reload();
    blockLeft->reload();
    blockRight->reload();
unblock->reload();
    unblockLeft->reload();
    unblockRight->reload();

    headLeft->reload(); 
    headRight->reload();
    gtChestPush->reload();
    nuFwdKick->reload();
    headTap->reload();
    softTap->reload();
    grab->reload();
    testKick->reload();
}

/* switch between calcHeightTilt that uses static points on the front legs, and a version that
uses the circular cowling on the front legs. */

#if 1

void PWalk::getAngleHeight(double &angle, double &shoulderHeight,
		double bX, double bY, double fX, double fY) {
	// given two X,Y coordinates (front and back)
	// Both with origin the front shoulder, but with opposite signs for X
	// And with the X axis being horozontal through the dogs body
	// find the resting body tilt angle of the dog
	// and the height of the front shoulder from the ground
	angle = atan2(bY-fY, bX+fX);
	double alpha = atan2(fX, fY);
	double gamma = alpha - angle;
	double fdist = sqrt(SQUARE(fX)+SQUARE(fY));
	shoulderHeight = fdist*cos(gamma);


	// skelliptical walk with minor walk type 'grab dribble'
	// has put the front legs in the air and walks with the front elbows
	// therefore need to correct the front shoulder height.
	static const double MIN_SHOULDER_HEIGHT = 80.0;
	if (shoulderHeight < MIN_SHOULDER_HEIGHT
	    && sp == skellipticalWalk
	    && ((SkellipticalWalk*)sp)->minorWalkType == MWT_SKE_GRAB_DRIBBLE
	    ) {
	  // if the shoulder height is too small, recalculate it using the minimal one.
	  shoulderHeight = MIN_SHOULDER_HEIGHT;
	  double bLegToFrontShoulderDist = sqrt(bX*bX + bY*bY);
	  double alpha = acos(MIN_SHOULDER_HEIGHT/bLegToFrontShoulderDist);
	  double bLegAngle = atan2(bX, bY);
	  angle = alpha - bLegAngle;
	}

	// cout << "bX: " << bX << " bY: " << bY << " fX: " << fX << " fY: " << fY << " ang: " << RAD2DEG(angle) << " h: " << height << endl;
}


void PWalk::calcHeightTilt(const JOINT_Values &flTheta, const JOINT_Values &blTheta, const JOINT_Values &frTheta, const JOINT_Values &brTheta) {
/*
	cout << "fl: " << flTheta << endl;
	cout << "fr: " << frTheta << endl;
	cout << "bl: " << blTheta << endl;
	cout << "br: " << brTheta << endl;
*/
	XYZ_Coord flToe;
	kinematics(flTheta, tLeg, fbLegBase, flToe); // fbLegPadEdge, flToe); //
	
	XYZ_Coord frToe;
	kinematics(frTheta, tLeg, fbLegBase, frToe); // fbLegPadEdge, frToe); //
	
	XYZ_Coord flBase;
	kinematics(flTheta, tLeg, fbLegBaseEdge, flBase);
	
	XYZ_Coord frBase;
	kinematics(frTheta, tLeg, fbLegBaseEdge, frBase);
	
	XYZ_Coord blToe;
	kinematics(blTheta, tLeg, bbLegToesGnd, blToe); //bbLegToes, blToe); //
	
	XYZ_Coord brToe;
	kinematics(brTheta, tLeg, bbLegToesGnd, brToe); //bbLegToes, brToe);//
	
	double bX, bY;
	
	if (blToe.y < brToe.y) {
		bX = brToe.x;
		bY = brToe.y;
	} else {
		bX = blToe.x;
		bY = blToe.y;
	}
	
	if (bY < SHOULDER_TO_BASE) {
		bY = SHOULDER_TO_BASE;
		bX = SHOULDER_TO_REAR;
	}
	
	bX += BODY_LENGTH;
	
	double tempBodyTilt = PI/2;
	double tempFrontShoulderHeight = -100;

	double bestBodyTilt = tempBodyTilt;
	double bestFrontShoulderHeight = tempFrontShoulderHeight;

	// now check all the possible front contact points in turn
	
	// front left toes
	getAngleHeight(tempBodyTilt, tempFrontShoulderHeight, bX, bY, flToe.x, flToe.y);
	if (tempBodyTilt < bestBodyTilt) {
		bestBodyTilt = tempBodyTilt;
		bestFrontShoulderHeight = tempFrontShoulderHeight;
	}
	
	// front right toes
	getAngleHeight(tempBodyTilt, tempFrontShoulderHeight, bX, bY, frToe.x, frToe.y);
	if (tempBodyTilt < bestBodyTilt) {
		bestBodyTilt = tempBodyTilt;
		bestFrontShoulderHeight = tempFrontShoulderHeight;
	}
	
	// front left base
	getAngleHeight(tempBodyTilt, tempFrontShoulderHeight, bX, bY, flBase.x, flBase.y);
	if (tempBodyTilt < bestBodyTilt) {
		bestBodyTilt = tempBodyTilt;
		bestFrontShoulderHeight = tempFrontShoulderHeight;
	}
	
	// front right base
	getAngleHeight(tempBodyTilt, tempFrontShoulderHeight, bX, bY, frBase.x, frBase.y);
	if (tempBodyTilt < bestBodyTilt) {
		bestBodyTilt = tempBodyTilt;
		bestFrontShoulderHeight = tempFrontShoulderHeight;
	}
	
	// front of chest
	getAngleHeight(tempBodyTilt, tempFrontShoulderHeight, bX, bY, SHOULDER_TO_FRONT, SHOULDER_TO_BASE);
	if (tempBodyTilt < bestBodyTilt) {
		bestBodyTilt = tempBodyTilt;
		bestFrontShoulderHeight = tempFrontShoulderHeight;
	}
	
	frontShoulderHeight = bestFrontShoulderHeight;
	bodyTilt = bestBodyTilt;
	backHipHeight = frontShoulderHeight + BODY_LENGTH*sin(bodyTilt);

	// BODY_LEGNTH = 130mm
	/*
	static int counter = 0;
	if (++counter % 130 == 0) {
	  FFCOUT << " front Shoulder height: " << frontShoulderHeight 
		 << " bodytilt: " << RAD2DEG(bodyTilt) 
		 << " backHipHeight: " << backHipHeight 
		 << endl; 
	  counter = 0;
	}
	*/
}

#else

void PWalk::getAngleHeight(double &angle, double &shoulderHeight,
		double bX, double bY, double fX, double fY) {
	// given two X,Y coordinates (front cowl centre and back toe posn)
	// Both with origin the front shoulder, but with opposite signs for X
	// And with the X axis being horozontal through the dogs body
	// find the resting body tilt angle of the dog
	// and the height of the front shoulder from the ground
	double beta = atan2(bY-fY, bX+fX);
	double distBeta = sqrt(SQUARE(bY-fY)+SQUARE(bX+fX));
	
	double mu = asin2(FRONT_LEG_COWL_RADIUS, distBeta);
	
	double delta = beta - mu;
	
	double alpha = atan2(fX, fY);	
	
	double gamma = alpha - delta;
	
	double fdist = sqrt(SQUARE(fX)+SQUARE(fY));
	double centreHeight = fdist*cos(gamma);
	shoulderHeight = centreHeight + FRONT_LEG_COWL_RADIUS;
	angle = delta;
	// cout << "bX: " << bX << " bY: " << bY << " fX: " << fX << " fY: " << fY << " ang: " << RAD2DEG(angle) << " h: " << height << endl;
}

void PWalk::calcHeightTilt(const JOINT_Values &flTheta, const JOINT_Values &blTheta, const JOINT_Values &frTheta, const JOINT_Values &brTheta) {

	// first get the  back leg positions

	XYZ_Coord blToe;
	kinematics(blTheta, tLeg, bbLegToes, blToe);
	blToe.x += BODY_LENGTH;
	
	XYZ_Coord brToe;
	kinematics(brTheta, tLeg, bbLegToes, brToe);
	brToe.x += BODY_LENGTH;
	
	// get the centres of the front cowlings
	
	XYZ_Coord flCowlCentre;
	kinematics(flTheta, tLeg, fbCowlCurveCenter, flCowlCentre);
	
	XYZ_Coord frCowlCentre;
	kinematics(frTheta, tLeg, fbCowlCurveCenter, frCowlCentre);
	
	// now check all the possible front contact points in turn
	
	double tempBodyTilt = PI/2;
	double tempFrontShoulderHeight = -100;

	double bestBodyTilt = tempBodyTilt;
	double bestFrontShoulderHeight = tempFrontShoulderHeight;

	// front left, back right
	getAngleHeight(tempBodyTilt, tempFrontShoulderHeight, brToe.x, brToe.y, flCowlCentre.x, flCowlCentre.y);
	// cout << "flbr: " << tempFrontShoulderHeight << ", " << RAD2DEG(tempBodyTilt) << endl;
	if (tempFrontShoulderHeight > bestFrontShoulderHeight) {
		bestBodyTilt = tempBodyTilt;
		bestFrontShoulderHeight = tempFrontShoulderHeight;
	}
	
	// front right, back left
	getAngleHeight(tempBodyTilt, tempFrontShoulderHeight, blToe.x, blToe.y, frCowlCentre.x, frCowlCentre.y);
	// cout << "frbl: " << tempFrontShoulderHeight << ", " << RAD2DEG(tempBodyTilt) << endl;
	if (tempFrontShoulderHeight > bestFrontShoulderHeight) {
		bestBodyTilt = tempBodyTilt;
		bestFrontShoulderHeight = tempFrontShoulderHeight;
	}
	
	// front left, back left
	getAngleHeight(tempBodyTilt, tempFrontShoulderHeight, blToe.x, blToe.y, flCowlCentre.x, flCowlCentre.y);
	// cout << "flbl: " << tempFrontShoulderHeight << ", " << RAD2DEG(tempBodyTilt) << endl;
	if (tempFrontShoulderHeight > bestFrontShoulderHeight) {
		bestBodyTilt = tempBodyTilt;
		bestFrontShoulderHeight = tempFrontShoulderHeight;
		// cout << "Weirdness in BodyTilt calculation!" << endl;
	}
	
	// front right, back right
	getAngleHeight(tempBodyTilt, tempFrontShoulderHeight, brToe.x, brToe.y, frCowlCentre.x, frCowlCentre.y);
	// cout << "frbr: " << tempFrontShoulderHeight << ", " << RAD2DEG(tempBodyTilt) << endl;
	if (tempFrontShoulderHeight > bestFrontShoulderHeight) {
		bestBodyTilt = tempBodyTilt;
		bestFrontShoulderHeight = tempFrontShoulderHeight;
		// cout << "Weirdness in BodyTilt calculation!" << endl;
	}
	
	frontShoulderHeight = bestFrontShoulderHeight;
	bodyTilt = bestBodyTilt;
	backHipHeight = frontShoulderHeight + BODY_LENGTH*sin(bodyTilt);
	/*
	FFCOUT << "front Shoulder height: " << frontShoulderHeight 
	       << " bodytilt: " << RAD2DEG(bodyTilt) 
	       << " BODY_LENGTH: " << BODY_LENGTH
	       << " backHipHeight: " << backHipHeight 
	       << endl;
	*/
}

#endif

/*
 * end PWalk
 */

