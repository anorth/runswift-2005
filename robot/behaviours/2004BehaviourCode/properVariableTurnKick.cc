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
 * $Id: properVariableTurnKick.cc 5072 2005-03-10 22:59:09Z alexn $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

/*
 * Strategy:
 *  aaProperVariableTurnKick
 *
 * Description:
 */


#include "Behaviours.h"
#include "UNSW2004.h"

#ifdef COMPILE_ALL_CPP
namespace Behaviours {
	int turnAmount;
}

static int aaProperVariableTurnKickCounter = 0;
static int grabTimer = 0;
static bool pvtkDebug = true;

void Behaviours::aaProperVariableTurnKick() {

  
  KEV_DEBUG(cout << "***********************\n";);
  KEV_DEBUG(cout << "     PVTK\n";);
  KEV_DEBUG(cout << "***********************\n";);

  if ( pvtkDebug )
    cout << "pvtk: timer: " << aaProperVariableTurnKickCounter << 
      "   turnAmount: " << turnAmount <<endl;

  DECREMENT (aaProperVariableTurnKickCounter);

  if (grabTimer < aaProperVariableTurnKickCounter) {
  //if (aaProperVariableTurnKickCounter) {
    if ( pvtkDebug )
    cout << "Grab the ball firmly "  <<endl;

    setTurnKickParams (0);
    return;
  }

  if ( aaProperVariableTurnKickCounter )
    {
      // might be a buggy check
      if ( vBall.cf >= 3 && !UNSW2004::isUNSW2004BallUnderChin() ) {
	lockMode = NoMode;
	return;
      }
      if ( turnAmount > 0 )
	{
	  if ( pvtkDebug )
	    cout << "turnAmount > 0    " << turnAmount<< endl;
	  setTurnKickParams(30);
	}
      else 
	{
	  if ( pvtkDebug )
	    cout << "turnAmount < 0      " << turnAmount << endl;
	  setTurnKickParams(-30);
	}
    }
  else
    {
       if ( turnAmount > 0 )
	{
	  if ( pvtkDebug )
	    cout << "TIMER EXPIRES && turnAmount > 0    " << turnAmount
		 << "  " << *currentWalkType << endl;
	  lockMode = UpennRight;
	  aaUPkickRight();
	}
       else
	{
	  if ( pvtkDebug )
	    cout << "TIMER EXPIRES && turnAmount < 0    " << turnAmount
		 << "  " << *currentWalkType << endl;
	  lockMode = UpennLeft;
	  aaUPkickLeft();
	}
       // u must not have no equal case
    }
  return;

}

//direction is the double of which direction you want the kick to go in
//+anticlockwise -clockwise
void Behaviours::setProperVariableTurnKick(double direction) {

  turnAmount = (int) direction;

  if ( pvtkDebug )
    cout << "In set pvtk: " << lockMode << endl;

  if ( turnAmount == 90 )
    {
	  lockMode = UpennRight;
	  aaUPkickRight();
	  return;
    }
  if ( turnAmount == -90 )
    {
	  lockMode = UpennLeft;
	  aaUPkickLeft();
	  return;
    }

  while ( turnAmount> 90) {
      turnAmount -= 90;
  }
  while (turnAmount < -90) {
      turnAmount += 90;
  }
  //double loc = (2.5)*((double)turnAmount)
  aaProperVariableTurnKickCounter = abs(turnAmount)+13;
  grabTimer = abs(turnAmount);
  if ( pvtkDebug )
    cout << "initialise pvtk: timer: " << aaProperVariableTurnKickCounter 
	 << "   turnAmount: " << turnAmount <<endl;
} 

/*
namespace Behaviours {
	int pvtkDuration;
	int pvtkDirection;
	int step_;
	int headLiftTime;
	int turnAmount;
	int errorMargin;
	int applyForwardUntilThisFromEnd;
	double forwardVector;
	int startStep_;
	double startForwardVector;
	int desKickWindow;

	int reachForffo;
}

static bool liftedHead = false;
static int framesAfterLiftHead = 0;

static bool inStartPosition = false;

static bool belowEnd = true;
static int kickWindowsPast = 0;
*/



// void Behaviours::setTurnKickParams() {
  /* --------- ask kevin ---------
	setWalkParams();
	walkType = CanterWalkWT;
	forward = 0;
	left = 0;
	turnCCW = turnDir * turnAmount;
	cout << "setTurnKickParamns:    FORWARD: " << forward <<"   LEFT: " << Behaviours::left
		 << "   TURNCCW: " << turnCCW << endl;
	ffO = reachForffo;
	PG = 40;

	headtype = ABS_H;
	panx = 0;
	tilty = -25;
	cranez = 0;
  --------- ask kevin ---------
  */
//void Behaviours::aaProperVariableTurnKick() {
  /*	--------- ask kevin ---------

	if (!inStartPosition) {
		if (*shareStep_ >= startStep_ - errorMargin
				&& *shareStep_ <= startStep_ + (errorMargin + 2)
				&& *currentPG == 40) {
			inStartPosition = true;
		}
		else {
			forward = startForwardVector;
			return;
		}
	}
	setTurnKickParams();
	turnCCW = turnDir * turnAmount;
	aaProperVariableTurnKickCounter++;

	if (liftedHead) {
		framesAfterLiftHead++;
		panx = 0;
		tilty = -20;
	}

	if (belowEnd && *shareStep_ >= step_) {
		//cout << "transition from LOW to HIGH" << endl;
		kickWindowsPast++;
		belowEnd = !belowEnd;
	}
	else if (!belowEnd && *shareStep_ < step_) {
		//cout << "transition from HIGH to LOW " << endl;
		belowEnd = !belowEnd;
	}
	--------- ask kevin ---------
  */

	//WARNING!!
	//window method wont work yet until you fix the 
	//clockwise 90 kick case
	/*
	   if (kickWindowsPast>=desKickWindow) {
	   liftedHead = true;
	   panx = 0;
	   tilty = -20;
	   }
	   */

	//dont start lifting your head until until youve turned far enough
	//and your feet are in the proper position to exit the turn kick

  /* --------- ask kevin ---------
	if (aaProperVariableTurnKickCounter >= (pvtkDuration - headLiftTime)
			&& *shareStep_ >= step_ - errorMargin
			&& *shareStep_ <= step_ + (errorMargin + 2)) {
		liftedHead = true;
		panx = 0;
		tilty = -20;
	}



	// apply forward vector, until near release
	if (framesAfterLiftHead < applyForwardUntilThisFromEnd) {
		forward = forwardVector;
	}

	// exit turn kick upon completion
	if (framesAfterLiftHead >= (2 * 8 - 4)) {
		cout << "finished kick" << endl;
		lockMode = ResetMode;
		setWalkParams();
		// don't grab ball for next 50 frames
		grabInterval = 50;
		saBallTracking();
	}
	--------- ask kevin ---------
  */
//void Behaviours::setProperVariableTurnKick(double direction) {
  /* --------- ask kevin ---------
	if (*shareStep_ < step_) {
		belowEnd = true;
	}
	else {
		belowEnd = false;
	}
	kickWindowsPast = 0;

	aaProperVariableTurnKickCounter = 0;
	liftedHead = false;
	framesAfterLiftHead = 0;
	inStartPosition = false;

	//theses should be fairly constant
	//when im sure they are constant i will make them static const
	headLiftTime = 17;
	errorMargin = 4;
	applyForwardUntilThisFromEnd = 8;
	startForwardVector = 4;

	//you need this when calibrating turn kick
	//turnDir = pvtkDirection;

	//everything from here down goes away when calibrating turn kick
	forwardVector = 0; //1.5;
	turnAmount = 50;
	reachForffo = 80;

	//clockwise   
	if (direction < 0) {
		pvtkDirection = -1;
		turnDir = pvtkDirection;
		step_ = 5;
		startStep_ = 55;

		direction = abs(direction);
		if (direction < 80) {
			//comes off at about 45
			reachForffo = 57;
			pvtkDuration = 10;
		}
		else if (direction >= 80 && direction <= 100) {
			//these numbers calibrated at padova
			pvtkDuration = 1;
			startStep_ = 55; 
			step_ = 75;
		}
		else if (direction <= 115) {
			pvtkDuration = 30;
			inStartPosition = true;
		}
		else if (direction < 170) {
			pvtkDuration = 32;
			inStartPosition = true;
		}
		else {
			pvtkDuration = 35;
			inStartPosition = true;
		}
	} 
	//anticlockwise
	else {
		pvtkDirection = 1;
		turnDir = pvtkDirection;
		step_ = 35;
		startStep_ = 15;

		direction = abs(direction);
		if (direction < 80) {
			//comes off at about 45
			reachForffo = 55;
			pvtkDuration = 10;
		}
		else if (direction >= 80 && direction <= 95) {
			pvtkDuration = 20;
		}
		else if (direction <= 115) {
			pvtkDuration = 23; //not tested
		}
		else if (direction < 160) {
			pvtkDuration = 25;
		}
		else if (direction >= 160 && direction <= 190) {
			pvtkDuration = 30;
			inStartPosition = true;
		}
		else if (direction < 210) {
			pvtkDuration = 35;
			inStartPosition = true;
		}
		else {
			pvtkDuration = 40;
			inStartPosition = true;
		}
	}
	--------- ask kevin ---------
  */

#endif //COMPILE_ALL_CPP
