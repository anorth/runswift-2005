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
 * $Id: SkellipticalWalk.cc 7645 2005-07-16 00:04:52Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "legsGeometry.h"
#include "SkellipticalWalk.h"
#include "../share/Common.h"
#include "JointCommand.h"

static const bool ellipticalLeadIn = false;
static const bool debugFSH = false;
static const bool bDebugLearningParams = false;
static const bool calibrateFromScratch = false;
static bool bUseGroundPoints = true;


static skelWalkParms wpPG31;
static skelWalkParms wpPG22;
static skelWalkParms wpFastForward;
static skelWalkParms wpGrabDribble;
static skelWalkParms wpGrabDribbleHold;

void SkellipticalWalk::initWalkParameters() {
  wpPG31.init(); // use the default value
  // please refer to the order in skelWalkParms::read()
  wpPG22.read(       "22 95   110   20   20 35   10  -20   5   20 20 50   60 5   5   2   2   11 2   2   4   3   1 0 0 0 105  100 100 0 0");
  wpFastForward.read("22 74.4 119.5 34.3 46 27.1 9.6 -11.2 6.3 20 20 47.8 59 5.7 5.4 2.7 4.1 12 1.3 4.1 5.6 5.1 3 0 0 0 74.4 100 100 0 0");
  wpGrabDribble.read("18 74.4 119.5 34.3 46 40.0 9.6 -11.2 6.3 34 30 47.8 59 5.7 5.4 2.7 4.1 12 1.3 4.1 5.6 5.1 3 0 0 0 74.4 50  100 0 15");
  memcpy(&wpGrabDribbleHold, &wpGrabDribble, sizeof(wpGrabDribble));
  wpGrabDribbleHold.frontF = 50.0;
  wpGrabDribbleHold.frontS = -5;
}


void SkellipticalWalk::setDefaultParameters() {
	struct skelStepParms tempStepParams;	
	tempStepParams.cmdFwd = 0;
	tempStepParams.cmdLeft = 0;
	tempStepParams.cmdTurnCCW = 0;
	tempStepParams.shortStep = 1;	
	minorWalkType = MWT_DEFAULT;
	step = 0;
	time = 1;
	setWalkParams(wpPG31);
	setStepParams(tempStepParams);
}


void SkellipticalWalk::setParameters(const AtomicAction &bufferedCommand) {
    
  minorWalkType = bufferedCommand.minorWalkType;

  if (calibrateFromScratch || minorWalkType == MWT_RAW_COMMAND) {
    bUseGroundPoints = true;
    skelStepParms tempStepParams;
    tempStepParams.cmdFwd = bufferedCommand.forwardMaxStep;
    tempStepParams.cmdLeft = bufferedCommand.leftMaxStep;
    tempStepParams.cmdTurnCCW = bufferedCommand.turnCCWMaxStep;
    tempStepParams.shortStep = 1;
    setStepParams(tempStepParams);
    return;
  }
      
  skelWalkParms tempWalkParams;
  tempWalkParams.init();

  bUseGroundPoints = true;
  switch (minorWalkType) {
  case MWT_DEFAULT:
  case MWT_SKE_TURN_WALK:      // based on fast forward
  case MWT_SKE_NECK_TURN_WALK: // based on fast forward
  case MWT_SKE_FAST_FORWARD:
    setWalkParams(wpFastForward);
    break;

  case MWT_SKE_PG31:
    bUseGroundPoints = false;
    setWalkParams(tempWalkParams);
    break;
     
  case MWT_RAW_COMMAND:
    // already handled in the above code
    break;

  case MWT_SKE_GRAB_DRIBBLE:
    setWalkParams(wpGrabDribble);
    break;

  case MWT_SKE_PG22:
    setWalkParams(wpPG22);
    break;

  case MWT_SKE_GRAB_DRIBBLE_HOLD:
    setWalkParams(wpGrabDribbleHold);
    break;

  default:
    FFCERR << "Error: Unknown minor walk type: " << minorWalkType << " switched to Fast Forward."<< endl;
    setWalkParams(wpFastForward);    
    break;
  } 

  skelStepParms tempStepParams;

  double full_rate = walkParms.halfStepTime * 2 * 8.0 / 1000.0; ;
        
  double fwdStep = bufferedCommand.forwardSpeed * full_rate;
  double leftStep = bufferedCommand.leftSpeed * full_rate;
  double turnStep = bufferedCommand.turnCCWSpeed * full_rate; 
  
  double fwdMaxStep = fabs(bufferedCommand.forwardMaxStep);
  double leftMaxStep = fabs(bufferedCommand.leftMaxStep);
  double turnMaxStep = fabs(bufferedCommand.turnCCWMaxStep); 
  // calculating desired full step size
  tempStepParams.cmdFwd = CLIP(fwdStep, fwdMaxStep);
  tempStepParams.cmdLeft = CLIP(leftStep, leftMaxStep);
  tempStepParams.cmdTurnCCW = CLIP(turnStep, turnMaxStep);
  
  // below here is just calculating whether we can take a short step.
  // If *all* compotempStepParams.shortStep = 0;nents have a max-step size set that is less than
  // the usual maximum then we can increase the step frequency
  
  fwdStep = MAX(fwdMaxStep,fabs(fwdStep));
  leftStep = MAX(leftMaxStep,fabs(leftStep)); 
  turnStep = MAX(turnMaxStep,fabs(turnStep));    

#ifdef CALWALK_OFFLINE
  cerr << "shortstep fwd " << fwdStep << " (" << fwdMaxStep << ") "
       << "left " << leftStep << " (" << leftMaxStep << ") "
       << "turn " << turnStep << " (" << turnMaxStep << ") => ";
#endif    
  double fwdComp = 0;     // 1
  double leftComp = 0;    // 1
  double turnComp = 0;    // 1
  if (fwdStep != 0 && fwdMaxStep != 0) {
    fwdComp = fwdMaxStep/fwdStep; 
  }
  if (leftStep != 0 && leftMaxStep != 0) {
    leftComp = leftMaxStep/leftStep;    
  }
  if (turnStep != 0 && turnMaxStep != 0) { 
    turnComp = turnMaxStep/turnStep;
  }         
  
  // Take the largest short-step of the components
  tempStepParams.shortStep = MAX(MAX(fwdComp,leftComp), turnComp);
  
  if (bufferedCommand.forwardSpeed == 0 
      && bufferedCommand.leftSpeed == 0 
      && bufferedCommand.turnCCWSpeed == 0){
    tempStepParams.shortStep = 0; // make the dog not walk
  }
  setStepParams(tempStepParams);

}

void SkellipticalWalk::setWalkParams(const struct skelWalkParms &newParms) {

	walkParms = newParms;

	if (bDebugLearningParams) {
	  FFCOUT << "walkParms:";
	  walkParms.print(cout);
	  cout << endl;
	}

	/*cout << "Set SkellipticalWalk parameters: ";
	newParms.print(cout);
	cout << endl; */

	// add some limits to the parameters so that the timing makes sense

	if (walkParms.frontDutyCycle < 0) {
		walkParms.frontDutyCycle = 0;
	} else if (walkParms.frontDutyCycle > 1) {
		walkParms.frontDutyCycle = 1;
	}
	if (walkParms.backDutyCycle < 0) {
		walkParms.backDutyCycle = 0;
	} else if (walkParms.backDutyCycle > 1) {
		walkParms.backDutyCycle = 1;
	}

	if (walkParms.frontLeadOutFrac < 0) {
		walkParms.frontLeadOutFrac = 0;
	} else if (walkParms.frontLeadOutFrac > 1) {
		walkParms.frontLeadOutFrac = 1;
	}
	if (walkParms.backLeadOutFrac < 0) {
		walkParms.backLeadOutFrac = 0;
	} else if (walkParms.backLeadOutFrac > 1) {
		walkParms.backLeadOutFrac = 1;
	}
	if (walkParms.frontLeadInFrac < 0) {
		walkParms.frontLeadInFrac = 0;
	} else if (walkParms.frontLeadInFrac > 1) {
		walkParms.frontLeadInFrac = 1;
	}
	if (walkParms.backLeadInFrac < 0) {
		walkParms.backLeadInFrac = 0;
	} else if (walkParms.backLeadInFrac > 1) {
		walkParms.backLeadInFrac = 1;
	}

	
	//setFrontH(); // set the front Height

	step = (int)step;	// round this when we set walk parameters
}

void SkellipticalWalk::setFrontH() {
  // setting the front Height according to walk commands
  // also sets all other attributes depends on front Height
	double frontHMix;
	if (stepParms.cmdFwd == 0 && stepParms.cmdLeft == 0)
	  frontHMix = 1;
	else {
	  frontHMix = fabs(stepParms.cmdFwd) /
	    (fabs(stepParms.cmdFwd) + fabs(stepParms.cmdLeft));
	}
	frontH = frontHMix * walkParms.frontH + (1-frontHMix) * walkParms.frontLeftH;

	bodyTilt = asin((walkParms.backH - frontH)/lsh);
	shoulderDist = lsh*cos(bodyTilt);
	
	double dF = shoulderDist/2;
	double dS = BODY_WIDTH/2;
	
	flFOffset = dF + walkParms.frontF - walkParms.turnCenterF;
	flLOffset = -dS - walkParms.frontS + walkParms.turnCenterL + walkParms.sideOffset;
	flTheta0 = atan2(flFOffset, flLOffset);
	flRadius = sqrt(flFOffset*flFOffset + flLOffset*flLOffset);

	frFOffset = dF + walkParms.frontF - walkParms.turnCenterF;
	frLOffset = dS + walkParms.frontS + walkParms.turnCenterL + walkParms.sideOffset;
	frTheta0 = atan2(frFOffset, frLOffset);
	frRadius = sqrt(frFOffset*frFOffset + frLOffset*frLOffset);

	blFOffset = -dF + walkParms.backF - walkParms.turnCenterF;
	blLOffset = -dS - walkParms.backS + walkParms.turnCenterL + walkParms.sideOffset;
	blTheta0 = atan2(blFOffset, blLOffset);
	blRadius = sqrt(blFOffset*blFOffset + blLOffset*blLOffset);

	brFOffset = -dF + walkParms.backF - walkParms.turnCenterF;
	brLOffset = dS + walkParms.backS + walkParms.turnCenterL + walkParms.sideOffset;
	brTheta0 = atan2(brFOffset, brLOffset);
	brRadius = sqrt(brFOffset*brFOffset + brLOffset*brLOffset);
}

void SkellipticalWalk::setStepParams(const struct skelStepParms &newParms) {
	stepParms = newParms;	
	double oldTime = time;
	double oldStep = step;
	
	time = 2*walkParms.halfStepTime*stepParms.shortStep;
	
	if (time < 1) {
		// stop a whole lot of problems with div by 0
		// You can't do anything in zero or -ve time
		time = 1;
		standing = true;
	} else {
		standing = false;
	}

	if (oldTime == 0) {
		step = 0;
	} else {
		step = oldStep/oldTime*time;	// move to the correct location in the new step
	}


	
	double heightMix;
	if (stepParms.cmdFwd != 0 || stepParms.cmdLeft != 0 || stepParms.cmdTurnCCW != 0) {
	  heightMix = 
	    fabs(stepParms.cmdFwd) /
	    (fabs(stepParms.cmdFwd) + fabs(stepParms.cmdLeft) + fabs(stepParms.cmdTurnCCW)*flRadius);
	  standing = false;
	} else {
	  heightMix = 1;
	  standing = true;
	}	

	
	calibrateWalk(stepParms.cmdFwd, stepParms.cmdLeft, stepParms.cmdTurnCCW, stepParms.shortStep,
				fwdLength, leftLength, turnLength);


	setFrontH(); // set the front Height

#ifdef CALWALK_OFFLINE
        cout << __func__ << " cmdFwd:" <<  stepParms.cmdFwd << " cmdLeft:"<< stepParms.cmdLeft << " cmdTurnCCW:" << stepParms.cmdTurnCCW <<endl;
#endif        
	forwardSpeed = stepParms.cmdFwd/2.0/walkParms.halfStepTime;
	leftSpeed = stepParms.cmdLeft/2.0/walkParms.halfStepTime;
	turnSpeed = stepParms.cmdTurnCCW/2.0/walkParms.halfStepTime;

//  	double heightMix;
//  	if ((fwdLength != 0) || (leftLength != 0) || (turnLength != 0)) {
//  		heightMix = fabs(fwdLength)/(fabs(fwdLength) + fabs(leftLength) + fabs(turnLength)*flRadius);
//  	} else {
//  		heightMix = 1;
//  	}

	
	frontHeight = heightMix*walkParms.frontFwdHeight + (1-heightMix)*walkParms.frontTrnLftHeight;
	backHeight = heightMix*walkParms.backFwdHeight + (1-heightMix)*walkParms.backTrnLftHeight;

	frontStartLeadOutTime = time/2*walkParms.frontDutyCycle * (1-walkParms.frontLeadOutFrac);
	backStartLeadOutTime = time/2*walkParms.backDutyCycle * (1-walkParms.backLeadOutFrac);
	frontEndLeadInTime = time - time/2*walkParms.frontDutyCycle * (1-walkParms.frontLeadInFrac);
	backEndLeadInTime = time - time/2*walkParms.backDutyCycle * (1-walkParms.backLeadInFrac);
	frontEndLeadOutTime = time/2 * walkParms.frontDutyCycle;
	backEndLeadOutTime = time/2 * walkParms.backDutyCycle;
	frontStartLeadInTime = time - time/2 * walkParms.frontDutyCycle;
	backStartLeadInTime = time - time/2 * walkParms.backDutyCycle;
}

static double SKEinversePositiveQuadratic(double x, double a, double b, double c) {
  if (x == 0) // if command is 0, length should be 0, too.
    return 0;

  double det = b*b - 4*a*(c-x);
  if (det < 0) {
    det = 0; // use max/min value of the quadratic function
  }
  double value = (-b + sqrt(det)) / 2 / a;

  if (value * x < 0) //if their signs don't agree
    value = -value;
  return value;
}

void SkellipticalWalk::calibrateWalk(double &cmdFwd, double &cmdLeft, double &cmdTurn, double shortStep,
				     double &fwdLen, double &leftLen, double &turnLen) {

  // some walk may use these values, so reset them each time
  walkParms.turnCenterF = walkParms.turnCenterL = 0;

  if (calibrateFromScratch || minorWalkType == MWT_RAW_COMMAND) {
    // use raw command for calibration and learning
    fwdLen = cmdFwd;
    leftLen = cmdLeft;
    turnLen = cmdTurn;
    // convert into the correct units	
    fwdLen *= 10;	// cm into mm
    leftLen *= 10;	// cm into mm
    turnLen = DEG2RAD(turnLen); // degrees into rads

    return;
  } 


  // step 1. clip things to the valid range, this changes the commands themselves
  // step 2. convert to the lengths to get the correct walking forward, left & turn speed
  switch (minorWalkType) {
  case MWT_SKE_PG31: // PG=31
    // forward
    cmdFwd = CLIP(cmdFwd, 17.3);  // acheived by raw command 8.0
    if (cmdFwd > 15)  
      cmdTurn -= 3.3; // added turn adjustment to walk straight
    // becasue, cmdFwd = 2.17145 * fwdLen - 0.878, therefore,
    if (cmdFwd == 0)
      fwdLen = 0;
    else if (cmdFwd > 0)
      fwdLen = (cmdFwd + 0.878) / 2.17145;
    else // if < 0
      fwdLen = - (-cmdFwd + 0.878)/2.17145;
    
    cmdLeft = CLIP(cmdLeft, -6.66, 8.48); // min, max acheived by raw command -4, 6
    if (cmdLeft == 0) 
      leftLen = 0;
    else if (cmdLeft > 0)
      leftLen = SKEinversePositiveQuadratic(cmdLeft, -0.307744, 3.71544, -2.73379);
    else // if < 0
      leftLen = SKEinversePositiveQuadratic(cmdLeft, 0.546935, 4.61079, 3.02598);
    
    cmdTurn = CLIP(cmdTurn, -58.3, 80.0); // min, max acheived by raw command -40, 50
    if (cmdTurn == 0) 
      turnLen = 0;
    else if (cmdTurn > 0)
      turnLen = SKEinversePositiveQuadratic(cmdTurn, -0.0220171, 2.82034, -5.49334);
    else // if < 0
      turnLen = SKEinversePositiveQuadratic(cmdTurn, 0.032188, 2.74671, 0.118574);      
    
    break; // BREAK!!
    
  case MWT_RAW_COMMAND:
    // should already be handled in the above code
    break; // BREAK!!
    
  case MWT_SKE_PG22: // PG=22
    // forward
    cmdFwd = CLIP(cmdFwd, -7.2, 11.1); //min, max acheived by raw command -4.5, 5
    if (cmdFwd > 15)  
      cmdTurn -= 3.3; // added turn adjustment to walk straight
    if (cmdFwd == 0)
      fwdLen = 0;
    else if (cmdFwd > 0)
      fwdLen = (cmdFwd + 1.15867) / 2.47045; //since cmdFwd = 2.47045 * fwdLen - 1.15867, for forward
    else // if < 0
      fwdLen = (cmdFwd - 0.773478) / 1.8283; //since cmdFwd = 1.8283 * fwdLen + 0.773478, for backward
    
    // left
    cmdLeft = CLIP(cmdLeft, -8.1, 8.3); //min, max acheived by raw command -7, 7
    if (cmdLeft == 0) 
      leftLen = 0;
    else if (cmdLeft > 0)
      leftLen = SKEinversePositiveQuadratic(cmdLeft, -0.118486, 2.01623, 0.131686);
    else // if < 0
      leftLen = SKEinversePositiveQuadratic(cmdLeft,  0.189221, 2.666, 1.13903);
    
    // turn
    cmdTurn = CLIP(cmdTurn, -64.0, 66.0); //min, max acheived by raw command -64, 66
    if (cmdTurn == 0) 
      turnLen = 0;
    else if (cmdTurn > 0) // turn ccw
      turnLen = SKEinversePositiveQuadratic(cmdTurn, -0.0262719, 2.85357, -11.018);
    else // if < 0 // turn cw
      turnLen = SKEinversePositiveQuadratic(cmdTurn, 0.0240438, 2.70008, 10.841);
    
    break; // BREAK!!

  case MWT_SKE_GRAB_DRIBBLE_HOLD: // this does NOT walk the same way, but if just for holding the ball still,
                                  // then it's ok.
  case MWT_SKE_GRAB_DRIBBLE: // PG=18
#ifdef OSAKA
    cmdFwd = CLIP(cmdFwd, 0.0, 9.1); // 10.4
    if (cmdFwd == 0)
      fwdLen = 0;
    else if (cmdFwd > 0) // forward
      fwdLen = SKEinversePositiveQuadratic(cmdFwd, -0.109313, 2.32087, -3.15073);
    else // if < 0 // backward
      fwdLen = 0;
   
    cmdLeft = CLIP(cmdLeft, -7.05, 6.46); //raw command -7.33, 6.31
    if (cmdLeft == 0)
      leftLen = 0;
    else if (cmdLeft > 0) // left
      leftLen = SKEinversePositiveQuadratic(cmdLeft, -0.176383, 2.28451, -0.921446);
    else // if < 0 // right
      leftLen = SKEinversePositiveQuadratic(cmdLeft, 0.126302, 1.95826, 0.510103);

    // if the dog is turning a lot, the ball may slip out, 
    // making the front leg to move sideways less will solve this problem
    //if (fabs(cmdFwd) < 2 && fabs(cmdLeft) < 2 && fabs(cmdTurn)> 50) {
      walkParms.frontBackLeftRatio = 0.8;
      cmdTurn = CLIP(cmdTurn, -61.1, 61.0);
      //cmdTurn = CLIP(cmdTurn, -64.8, 64.6); //min, max acheived by raw command -61.1, 61.6
      if (cmdTurn == 0)
	turnLen = 0;
      else if (cmdTurn > 0) // turn ccw
	// pg = 17
	turnLen = SKEinversePositiveQuadratic(cmdTurn, -0.0170131, 2.11792, -4.78588);
      // pg = 18
      //turnLen = SKEinversePositiveQuadratic(cmdTurn, -0.0180139, 2.2425, -5.06741);
      else // if < 0 // turn cw
	// pg = 17
	turnLen = SKEinversePositiveQuadratic(cmdTurn, 0.0166782, 2.09493, 4.44436);
      // pg = 18

      //turnLen = SKEinversePositiveQuadratic(cmdTurn, 0.0163433, 2.10624, 2.76978);
      //turnLen *= 1.1;
      //} 
//     else {
//       walkParms.frontBackLeftRatio = 1;
//       cmdTurn = CLIP(cmdTurn, -57.6, 58.4); //min, max acheived by raw command -52, 60
//       if (cmdTurn == 0)
// 	turnLen = 0;
//       else if (cmdTurn > 0) // turn ccw
// 	turnLen = SKEinversePositiveQuadratic(cmdTurn, -0.0204719, 2.30153, -6.12393);
//       else // if < 0 // turn cw
// 	turnLen = SKEinversePositiveQuadratic(cmdTurn, 0.0210316, 2.32074, 6.01979);
//     }
#else

    cmdFwd = CLIP(cmdFwd, 0.0, 9.57);
    if (cmdFwd == 0)
      fwdLen = 0;
    else if (cmdFwd > 0) // forward
      fwdLen = SKEinversePositiveQuadratic(cmdFwd, -0.145577, 2.81451, -4.01167);
    else // if < 0 // backward
      fwdLen = 0;
   
    cmdLeft = CLIP(cmdLeft, -6.42, 6.1); //raw command -4.78, 4.7
    if (cmdLeft == 0)
      leftLen = 0;
    else if (cmdLeft > 0) // left
      leftLen = SKEinversePositiveQuadratic(cmdLeft, -0.302606, 2.91435, -0.864576);
    else // if < 0 // right
      leftLen = SKEinversePositiveQuadratic(cmdLeft, 0.323383, 3.2154, 1.54829);

    cmdTurn = CLIP(cmdTurn, -63.8, 60.2); //min, max acheived by raw command -60, 60
    if (cmdTurn == 0)
      turnLen = 0;
    else if (cmdTurn > 0) // turn ccw
      turnLen = SKEinversePositiveQuadratic(cmdTurn, -0.0200516, 2.30298, -5.76908);
    else // if < 0 // turn cw
      turnLen = SKEinversePositiveQuadratic(cmdTurn, 0.0176889, 2.19871, 4.38185);

    // if the dog is turning a lot, the ball may slip out, 
    // making the front leg to move sideways less will solve this problem
    if (fabs(cmdFwd) < 2 && fabs(cmdLeft) < 2 && fabs(cmdTurn)> 50)
      walkParms.frontBackLeftRatio = 0.8;
    else
      walkParms.frontBackLeftRatio = 1;
#endif // OSAKA

    break; // BREAK!!

  case MWT_SKE_NECK_TURN_WALK:
    walkParms.turnCenterF = TURN_CENTRE_TO_NECK * 10;
    calibrateWalk_FastForward(cmdFwd, cmdLeft, cmdTurn, shortStep, fwdLen, leftLen, turnLen);
    break;

  case MWT_SKE_TURN_WALK: // try (6,6,-40) with no calibration, the dog is turning on it's right front leg!
    if (cmdTurn != 0 && (cmdTurn < 90 || cmdTurn > -90) ) {
      double dist = sqrt(cmdFwd * cmdFwd + cmdLeft * cmdLeft);
      dist = CLIP(dist, 21.0); // assume it's our step limit in cm.
            
      turnLen = DEG2RAD(cmdTurn);
      double radius = dist / tan(turnLen);

      double thetaTangent = atan2(cmdLeft, cmdFwd);
      walkParms.turnCenterF = -radius * sin(thetaTangent) * 10;
      walkParms.turnCenterL = radius * cos(thetaTangent) * 10;
      //FFCOUT << "F: " << walkParms.turnCenterF << " L: " << walkParms.turnCenterL << endl;
      
      turnLen = turnLen * 1; // <- calibration goes here
      //FFCOUT << "TurnLen: " << turnLen << endl;      
      fwdLen = leftLen = 0;
      return; // RETURN!!
    }

    // NO BREAK here so other cases will fall into Fast Forward

  default:
  case MWT_DEFAULT:
  case MWT_SKE_FAST_FORWARD: // PG=22
    calibrateWalk_FastForward(cmdFwd, cmdLeft, cmdTurn, shortStep, fwdLen, leftLen, turnLen);
    break; // BREAK!!
  }

  // now shrink the lengths to take account of the short step
  fwdLen *= shortStep;
  leftLen *= shortStep;
  turnLen *= shortStep;
  
  // finally, convert into the correct units	
  fwdLen *= 10;	// cm into mm
  leftLen *= 10;	// cm into mm
  turnLen = DEG2RAD(turnLen); // degrees into rads

}

#ifdef OSAKA
// this mwt is so popular, several other mwt use it with small adjustments
void SkellipticalWalk::calibrateWalk_FastForward(double &cmdFwd, double &cmdLeft, double &cmdTurn, double shortStep,
				     double &fwdLen, double &leftLen, double &turnLen) {
  shortStep = shortStep; // not use
  
  cmdFwd = CLIP(cmdFwd, -10.2, 14.28); // raw command -7.8, 8.1  
  cmdLeft = CLIP(cmdLeft, -6.86, 7.0); // raw cmd -4.79, 6.4   
  cmdTurn = CLIP(cmdTurn, -77.7, 74.5); // raw cmd -51.2, 47
  
  // forward
  if (cmdFwd == 0) 
    fwdLen = 0;
  else if (cmdFwd > 0)
    fwdLen = SKEinversePositiveQuadratic(cmdFwd, -0.1762, 3.51343, -2.65701);
  else // if < 0
    fwdLen = SKEinversePositiveQuadratic(cmdFwd, 0.153758, 2.41804, -0.725855);

  // left 
  if (cmdLeft == 0) 
    leftLen = 0;
  else if (cmdLeft > 0)
    leftLen = SKEinversePositiveQuadratic(cmdLeft, -0.128301, 1.79841, 0.708267);
  else // if < 0
    leftLen = SKEinversePositiveQuadratic(cmdLeft, 0.32737, 3.17528, 0.789831);
    
  // turn
  if ( fabs(cmdFwd) > (14.28/2.0) || fabs(cmdLeft) > (7.0 / 2.0) ) {
    // if turning with forward and left
    cmdTurn = CLIP(cmdTurn, -30.0, 30.0);     
    if (cmdTurn == 0) 
      turnLen = 0;
    else if (cmdTurn > 0)
      turnLen = SKEinversePositiveQuadratic(cmdTurn, -0.025587, 1.84008, -1.37847);
    else // if < 0
	turnLen = SKEinversePositiveQuadratic(cmdTurn, 0.0179269, 1.40234, -4.61686);
  }
  else {
    // if mostly turning
    if (cmdTurn == 0.0)
      turnLen = 0;
    else if (cmdTurn > 0) {
      turnLen = SKEinversePositiveQuadratic(cmdTurn, -0.036707, 3.54666, -10.4519);
    } else // if < 0 
      turnLen = SKEinversePositiveQuadratic(cmdTurn,  0.0308069, 3.26455, 8.57866);
  }

}

#else

// this mwt is so popular, several other mwt use it with small adjustments
void SkellipticalWalk::calibrateWalk_FastForward(double &cmdFwd, double &cmdLeft, double &cmdTurn, double shortStep,
				     double &fwdLen, double &leftLen, double &turnLen) {
  shortStep = shortStep; // not use
  
  cmdFwd = CLIP(cmdFwd, -10.1, 15.04); // raw command -8.6, 
  cmdLeft = CLIP(cmdLeft, -8.35, 8.78); // max acheived by raw command 5.67   
  cmdTurn = CLIP(cmdTurn, -81.5, 85.5);
  
  // forward
  if (cmdFwd == 0) 
    fwdLen = 0;
  else if (cmdFwd > 0)
    fwdLen = SKEinversePositiveQuadratic(cmdFwd, -0.200274, 3.75082, -2.21174);
  else // if < 0
    fwdLen = SKEinversePositiveQuadratic(cmdFwd, 0.117147, 2.12587, -0.528203);
  
  // left 
  if (cmdLeft == 0) 
    leftLen = 0;
  else if (cmdLeft > 0)
    leftLen = SKEinversePositiveQuadratic(cmdLeft, -0.291666, 3.32228, -0.666006);
  else // if < 0
    leftLen = SKEinversePositiveQuadratic(cmdLeft, 0.252133, 2.9438, 0.0547584);
    
  // turn
  if (fabs(cmdFwd) < 1.0 && fabs(cmdLeft) < 1.0) {
    // if mostly turning
    cmdTurn = CLIP(cmdTurn, -81.5, 85.5);
    
    if (cmdTurn == 0.0)
      turnLen = 0;
    else if (cmdTurn > 0) {
      turnLen = SKEinversePositiveQuadratic(cmdTurn, -0.0390517, 3.86342, -9.82579);
      if (turnLen < 0) turnLen = -turnLen;
    } else // if < 0 
      {
	turnLen = SKEinversePositiveQuadratic(cmdTurn, 0.03164, 3.40488, 9.68153);	
	if (turnLen > 0) turnLen = -turnLen; // if the value goes to another quadrand
	}
  } else {
    // if turning with forward and left
    cmdTurn = CLIP(cmdTurn, -30.0, 30.0);      
    if (cmdTurn == 0) 
      turnLen = 0;
    else if (cmdTurn > 0) {
      turnLen = SKEinversePositiveQuadratic(cmdTurn, -0.0226357, 1.74685, 0.201133);
      if (turnLen < 0) turnLen = -turnLen;
    } else // if < 0
      {
	turnLen = SKEinversePositiveQuadratic(cmdTurn, 0.0214915, 1.63917, -1.76263);
	if (turnLen > 0) turnLen = -turnLen;
      }
  }

}

#endif

void SkellipticalWalk::getOdometry(double &delta_forward, double &delta_left, double &delta_turn, bool highGain) {
    (void)highGain; //compiler warning
    delta_forward = forwardSpeed;
    delta_left = leftSpeed;
    delta_turn = turnSpeed;
}

void SkellipticalWalk::handleTilt(JOINT_Values &joints, bool front) {
	if (front) {
		joints.rotator += bodyTilt;	// handle the tilt
	} else {
		joints.rotator -= bodyTilt;	// handle the tilt
	}
}

void SkellipticalWalk::getLocusX(const double step,
					double &X,
					const bool front,
					const double length) {

	const double endLeadOutTime = front?frontEndLeadOutTime:backEndLeadOutTime;
	const double startLeadInTime = front?frontStartLeadInTime:backStartLeadInTime;

	double wrappedStep = step;
	if (wrappedStep > time/2)
		wrappedStep -= time;

	const double mySpeed = length*2.0/time;

	if ((step <= endLeadOutTime) || (step >= startLeadInTime)) {
		// bottom of locus
		X = -wrappedStep*mySpeed;
	} else {
		// recovery phase
		double theta;
		double sideLength;
		if (step < time/2) {
			theta = (step-endLeadOutTime)/(time/2-endLeadOutTime)*PI/2;
			sideLength = mySpeed*endLeadOutTime;
		} else {
			theta = PI - (startLeadInTime - step)/(startLeadInTime - time/2)*PI/2;
			sideLength = mySpeed*(time-startLeadInTime);
		}
		X = -sideLength * cos(theta);
		// cout << "step: " << step << " theta: " << theta << " length: " << length << " len: " << sideLength << " X: " << X << endl;
	}
}

void SkellipticalWalk::getLocusY(const double step,
					double &Y,
					const bool front,
					const double height) {

	const double startLeadInTime = front?frontStartLeadInTime:backStartLeadInTime;
	const double endLeadInTime = front?frontEndLeadInTime:backEndLeadInTime;
	const double startLeadOutTime = front?frontStartLeadOutTime:backStartLeadOutTime;
	const double endLeadOutTime = front?frontEndLeadOutTime:backEndLeadOutTime;

	double wrappedStep = step;
	if (wrappedStep > time/2)
		wrappedStep -= time;
	
	double frac;
	
	if (wrappedStep > 0) {
		frac = front?walkParms.frontLeadUpFrac:walkParms.backLeadUpFrac;
	} else {
		frac = front?walkParms.frontLeadDownFrac:walkParms.backLeadDownFrac;
	}

	if ((step <= startLeadOutTime) || (step >= endLeadInTime)) {
		// flat section along bottom of locus
		Y = 0;
	} else if ((step < endLeadOutTime) || (step > startLeadInTime)) {
		// handle lead in and out
		double eX;
		// first calculate the X position within the ellipse
		if (wrappedStep > 0) {
			eX =  (step - startLeadOutTime)/(endLeadOutTime - startLeadOutTime);
		} else {
			eX =  (endLeadInTime - step)/(endLeadInTime - startLeadInTime);
		}
		
		// then figure out the Y value
		if (ellipticalLeadIn) {
			// elliptical lead in
			Y = height*frac * (1 - sqrt(1 - eX*eX));
		} else {
			// linear lead in
			Y = eX * height * frac;
		}
		
	} else {
		// return stroke
		double theta;
		if (step < time/2) {
			theta = (step-endLeadOutTime)/(time/2-endLeadOutTime)*PI/2;
		} else {
			theta = PI - (startLeadInTime - step)/(startLeadInTime - time/2)*PI/2;
		}
		Y = height*frac + height*(1-frac)*sin(theta);
	}
}

void SkellipticalWalk::makeFSFwd(double X, double &f, double &s) {
	f = X;
	s = 0;
}

void SkellipticalWalk::makeFSLft(double X, double &f, double &s, bool left) {
	f = 0;
	s = left?X:-X;
}

void SkellipticalWalk::makeFSTrn(double X, double &f, double &s, bool left, bool front) {
	if (front) {
		if (left) {
			f = sin(X + flTheta0)*flRadius - flFOffset;
			s = -(cos(X + flTheta0)*flRadius - flLOffset);
		} else {
			f = sin(X + frTheta0)*frRadius - frFOffset;
			s = cos(X + frTheta0)*frRadius - frLOffset;
		}
	} else {
		if (left) {
			f = sin(X + blTheta0)*blRadius - blFOffset;
			s = -(cos(X + blTheta0)*blRadius - blLOffset);
		} else {
			f = sin(X + brTheta0)*brRadius - brFOffset;
			s = cos(X + brTheta0)*brRadius - brLOffset;
		}
	}
}

void SkellipticalWalk::getLocusJoints(double step, bool front, bool left, JOINT_Values &joints) {
	double X, Y, f, s, h, fTemp, sTemp;
	XYZ_Coord xyz;

	getLocusY(step, Y, front, front?frontHeight:backHeight);
	h = Y;

	// add the thrust
	{
		double startLeadOutTime = front?frontStartLeadOutTime:backStartLeadOutTime;
		double endLeadOutTime = front?frontEndLeadOutTime:backEndLeadOutTime;
		if (step < startLeadOutTime)
			h -= step / startLeadOutTime * (walkParms.thrustHeight);
		else if (step < endLeadOutTime) {
			double diff = endLeadOutTime - startLeadOutTime;
			h -= (endLeadOutTime - step) / diff * (walkParms.thrustHeight);
		}
	}
	// and the canter
	double stepCycle = step / time * 4.0 * pi;
	h -= walkParms.canterHeight * cos(stepCycle);
	
	getLocusX(step, X, front, fwdLength);	// fwd
	makeFSFwd(X, f, s);
	
	getLocusX(step, X, front, leftLength);	// left
	makeFSLft(X, fTemp, sTemp, left);
	f += fTemp;
	s += sTemp;
	
	getLocusX(step, X, front, turnLength);	// turn
	makeFSTrn(X, fTemp, sTemp, left, front);
	f += fTemp;
	s += sTemp;
	
	if (front) {
		f *= walkParms.frontBackForwardRatio;
		s *= walkParms.frontBackLeftRatio;
	}
	
	double rollAngle = 0;
	if (front && ((step < frontEndLeadOutTime) || (step > frontStartLeadInTime))) {
		// use the roll on the ground stroke of the front legs
		double rollDist = walkParms.frontRollWalkRatio * f;
		f *= (1 - walkParms.frontRollWalkRatio);
		rollAngle = frontBottomLegPointAngle(rollDist);
		// cout << "rollAngle: " << rollAngle << endl;
	}
	
	// rotate2DCoordinate(f, h, walkParms.locRotate);

	if (debugFSH)
		cout << "fl: " << front << left << " fsh: " << f << " " << s << " " << h << endl;
	
	// convert to xyz coords
	if (front) {
		xyz.x = walkParms.frontF + f;
		xyz.z = walkParms.frontS + s + (left?-walkParms.sideOffset:walkParms.sideOffset);
		xyz.y = frontH - h; 
	} else {
		xyz.x = walkParms.backF + f;
		xyz.z = walkParms.backS + s + (left?-walkParms.sideOffset:walkParms.sideOffset);
		xyz.y = walkParms.backH - h; 
	}
	
	if (front) {
	  if (bUseGroundPoints) {
	    double basePtAngleRad = DEG2RAD(walkParms.frontHomeAngleDeg) + rollAngle;
	    // cout << "basePtAngleRad: " << basePtAngleRad << endl;	    
	    XYZ_Coord fbLegPoint = frontBottomLegPoint(basePtAngleRad, false);	// fbLegBase;
	    invKinematics(xyz, tLeg, fbLegPoint, joints);
	  } else {
	    invKinematics(xyz, tLeg, fbLegPadCenter, joints);
	  }
	} else {		
	  xyz.x *= -1;
	  if (bUseGroundPoints) {
	    invKinematics(xyz, tLeg, bbLegToesGnd, joints);
	  } else {
	    invKinematics(xyz, tLeg, bbLegPad, joints);
	  }
	}
	handleTilt(joints, front);
}

void SkellipticalWalk::setCurrentJointCommand(JointCommand &jointCommand) {
	JOINT_Values leg;
	
	double step_ = step + time/2;
	if (step_ > time)
		step_ -= time;
	
	if (standing) {	// handle standing still
		step = step_ = 0;
	}
	

	getLocusJoints(step, true, true, leg); // front left
    jointCommand.setJointEnumValue(leftFrontJoint, RAD2MICRO(leg.rotator) ); 
    jointCommand.setJointEnumValue(leftFrontShoulder, RAD2MICRO(leg.shoulder) ); 
    jointCommand.setJointEnumValue(leftFrontKnee, RAD2MICRO(leg.knee) ); 

    //FFCOUT << "FL: " << leg << endl;
	
    getLocusJoints(step_, true, false, leg); // front right
    jointCommand.setJointEnumValue(rightFrontJoint, RAD2MICRO(leg.rotator) );
    jointCommand.setJointEnumValue(rightFrontShoulder, RAD2MICRO(leg.shoulder) ); 
    jointCommand.setJointEnumValue(rightFrontKnee, RAD2MICRO(leg.knee) ); 

    //FFCOUT << "FR: " << leg << endl;
	
    getLocusJoints(step_, false, true, leg); // back left
    jointCommand.setJointEnumValue(leftRearJoint, RAD2MICRO(leg.rotator) ); 
    jointCommand.setJointEnumValue(leftRearShoulder, RAD2MICRO(leg.shoulder) ); 
    jointCommand.setJointEnumValue(leftRearKnee, RAD2MICRO(leg.knee) ); 

    //FFCOUT << "BL: " << leg << endl;
    
    getLocusJoints(step, false, false, leg); // back right
    jointCommand.setJointEnumValue(rightRearJoint, RAD2MICRO(leg.rotator) ); 
    jointCommand.setJointEnumValue(rightRearShoulder, RAD2MICRO(leg.shoulder) ); 
    jointCommand.setJointEnumValue(rightRearKnee, RAD2MICRO(leg.knee) ); 

    //FFCOUT << "BR: " << leg << endl;
}

#ifdef CALWALK_OFFLINE
int main() {
  SkellipticalWalk* walk = new SkellipticalWalk();

  AtomicAction a;
  a.forwardSpeed = 0.0;
  a.leftSpeed = 0.0;
  a.turnCCWSpeed = 0.0;
  a.forwardMaxStep = 0.0;
  a.leftMaxStep = 0.0;
  a.turnCCWMaxStep = 0.0;
  a.minorWalkType = GRAB_DRIBBLE;
  double fwd, fwdM, lft, lftM, trn, trnM;
  while(1) {
    cout << "Please enter fwd & maxF & lft & maxL & trn & maxT. To exit type 999[enter] \n ";
    cin >> fwd;
    if (fwd == 999)
        break;
    cin >> fwdM >> lft >> lftM >> trn >> trnM;
    a.forwardSpeed = fwd;
    a.leftSpeed = lft;
    a.turnCCWSpeed = trn;
    a.forwardMaxStep = fwdM;
    a.leftMaxStep = lftM;
    a.turnCCWMaxStep = trnM;
    
    cout << "before calibration, fwd / lft / trn is " << fwd << " " << lft << " " << trn << endl;
    walk->setParameters(a);    
    walk->getOdometry(fwd,lft,trn, true);
    cout << "after calibration, fc / sc / tc is " << fwd << " " << lft << " " << trn << endl;
  }
}
#endif
