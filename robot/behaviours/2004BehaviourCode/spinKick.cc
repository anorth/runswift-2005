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
 * $Id: spinKick.cc 5072 2005-03-10 22:59:09Z alexn $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

/*
 * Strategy:
 *  aaSpinKick
 *
 * Description:
 *  spins with ball under snout,
 *  performs normal forward kick when
 *  correctly aligned.
 *  faster than turning while carrying the ball because of head down
 *
 * Strategy member variables used:
 *  grabTime -
 *      make sure we do not grab
 *      the ball for over 3 secs
 *  lostBall -
 *      num of frames ball lost
 *
 * Local variables used:
 *  none
 *
 * Restrictions:
 *  none
 *
 * Comments:
 *  the original version of aaSpinKick
 *  was much more complex and didn't really
 *  make sense; this is a much more simplified
 *  version on which additional checks would
 *  probably have to be made.
 */

#include "Behaviours.h"

// shooting at visual goal state
static bool shootingAtVisualGoal = false;   // are we locked into shooting at visual goal
static int shootingAtVisualGoalTimer = 0;   // num of frames since first start shooting at vgoal

void Behaviours::aaSpinKick() {
	// Ted: removed this - saActiveBallTracking();

	// break out of spin kick if timer expires
	if (!grabTime) {
		// reset shooting at visual goal state
		shootingAtVisualGoal = false;
		shootingAtVisualGoalTimer = 0;

		// perhaps we should check to see whether opponents are nearby?
		// sometimes, might be more beneficial to do a chestpush
		lockMode = GoalieKick;
		aaGoalieKick();
		return;
	}

	if (shootingAtVisualGoal) {
		shootAtVisualGoal();
		return;
	}        

	double accuracyRequirement = requiredAccuracy(20);
	accuracyRequirement = MAX(accuracyRequirement, 5);
	if (ABS(gps->tGoal().pos.getHead()) <= accuracyRequirement) {
		shootingAtVisualGoal = true;
		shootingAtVisualGoalTimer = 0;
		shootAtVisualGoal();
		return;
	}
	else {
		setSpinKickParams();
		return;
	}
}

void Behaviours::aaSafeSpinKick() {
	// Ted: removed it - saActiveBallTracking();

	// break out of spin kick if timer expires
	if (!grabTime) {
		// reset visual goal state
		shootingAtVisualGoal = false;
		shootingAtVisualGoalTimer = 0;

		// if facing edge
		if (vision->facingFieldEdge) {
			double dir;
			if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
				dir = -90;
			}
			else {
				dir = 90;
			}
			lockMode = ProperVariableTurnKick;
			setProperVariableTurnKick(dir);
			aaProperVariableTurnKick();    
			return;
		}

		// perhaps perform a chest-push when in the clear?
		lockMode = GoalieKick;
		aaGoalieKick();
		return;
	}

	if (shootingAtVisualGoal) {
		shootAtVisualGoal();
		return;
	}        

	double accuracyRequirement = requiredAccuracy(20);
	accuracyRequirement = MAX(accuracyRequirement, 5);
	if (ABS(gps->tGoal().pos.getHead()) <= accuracyRequirement) {
		if (vision->facingFieldEdge) {
			double dir;
			if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
				dir = -90;
			}
			else {
				dir = 90;
			}
			lockMode = ProperVariableTurnKick;
			setProperVariableTurnKick(dir);
			aaProperVariableTurnKick();      
			return;
		}

		shootingAtVisualGoal = true;
		shootingAtVisualGoalTimer = 0;

		shootAtVisualGoal();
		return;
	}
	else {
		setSpinKickParams();
		return;
	}
}

void Behaviours::aaVisualSpinKick() {
	// Ted: removed it  - saActiveBallTracking();

	// break out of spin kick if timer expires
	if (!grabTime) {
		// reset visual goal state
		shootingAtVisualGoal = false;
		shootingAtVisualGoalTimer = 0;

		// if facing edge
		if (vision->facingFieldEdge) {
			double dir;
			if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
				dir = -90;
			}
			else {
				dir = 90;
			}
			lockMode = ProperVariableTurnKick;
			setProperVariableTurnKick(dir);
			aaProperVariableTurnKick();    
			return;
		}

		// perhaps perform a chest-push when in the clear?
		lockMode = GoalieKick;
		aaGoalieKick();
		return;
	}

	if (shootingAtVisualGoal) {
		shootAtVisualGoal();
		return;
	}        

	double accuracyRequirement = requiredAccuracy(20);
	accuracyRequirement = MAX(accuracyRequirement, 5);
	if (ABS(gps->tGoal().pos.getHead()) <= accuracyRequirement) {
		if (vision->facingFieldEdge) {
			double dir;
			if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
				dir = -90;
			}
			else {
				dir = 90;
			}
			lockMode = ProperVariableTurnKick;
			setProperVariableTurnKick(dir);
			aaProperVariableTurnKick();      
			return;
		}

		shootingAtVisualGoal = true;
		shootingAtVisualGoalTimer = 0;

		shootAtVisualGoal();
		return;
	}
	else {
		setSpinKickParams();
		return;
	}
}

void Behaviours::shootAtVisualGoal() {
	shootingAtVisualGoalTimer++;

	// to keep ball from popping
	if (shootingAtVisualGoalTimer < 5) {
		// Ted: removed it - UNSW2004::doUNSW2004TrackVisualBall();
		setHoldParams();
		return;
	}

	// look up, expecting to see target goal
	if (shootingAtVisualGoalTimer < 8) {
		headtype = ABS_H;
		tilty = 5;
		panx = 0;

		setHoldParams();
		return;
	}

	// can see target goal
	if (vtGoal.cf > 25) {
		headtype = ABS_H;
		tilty = 5;
		panx = Cap(vtGoal.h, 30.0);

		if (ABS(vtGoal.h) < 5) {
			headtype = ABS_H;
			tilty = 0;
			panx = 0;

			lockMode = GoalieKick;
			aaGoalieKick();

			shootingAtVisualGoal = false;
			shootingAtVisualGoalTimer = 0;
		}
		else {
			setCarryParams();
			turnCCW = ABS(vtGoal.h) > 9.5 ? vtGoal.h : (vtGoal.h > 0 ? 9.5 : -9.5);
			left = -turnCCW / 6.0;
		}
		return;
	}

	// cannot see goal, spin around
	headtype = ABS_H;
	tilty = 5;
	panx = 0;

	setCarryParams();
	if (gps->tGoal().pos.getHead() > 0) {
		turnCCW = 30.0;
		left = -5.0;
	}
	else {
		turnCCW = -30.0;
		left = 5.0;
	}
}

void Behaviours::lookAroundForGoal() {
	headtype = ABS_H;
	tilty = 20;

	if (panx > 55.0) {
		panDirection = -1;
	}

	if (panx < -55.0) {
		panDirection = +1;
	}

	panx += (panDirection * 8.0);
}

// spin with ball under snout
void Behaviours::setSpinKickParams() {
	walkType = NormalWalkWT;
	fsO = 25;
	ffO = 80;
	forward = 1;
	turnCCW = CLIP(gps->tGoal().pos.getHead(), 40.0);
	left = (gps->tGoal().pos.getHead() > 0) ? -3 : 3;
	//left = 0;
	//left = -turnCCW/6.0;
	headtype = ABS_H;
	panx = 0;
	tilty = -50;
}
