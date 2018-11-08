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
 * $Id:
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 **/

/*
 * Strategy: Specialised kicks for
 *  the Penalty Shooter
 *
 *  doPenaltyShooterAimKick
 *  doPenaltyshooterSpinKick
 **/

#include "penaltyShooter.h"
#include <algorithm>
#include <vector>
#include <utility>

// visual goal kick state
static bool penaltyShooterAimKickStarted = false;
static int penaltyShooterAimKickTimer = 0;

void PenaltyShooter::doPenaltyShooterAimKick() {
	// break out of kick if timer expires
	if (!grabTime) {
		// reset state
		resetPenaltyShooterAimKick();

		// perhaps perform a chest-push when in the clear?
		lockMode = ChestPush;
		aaChestPush();
		return;
	}

	if (penaltyShooterAimKickStarted) {
		performPenaltyShooterAimKick();
		return;
	}

	double accuracyRequirement = requiredAccuracy(20);
	accuracyRequirement = MAX(accuracyRequirement, 5);
	if (ABS(gps->tGoal().pos.getHead()) <= accuracyRequirement) {
		penaltyShooterAimKickStarted = true;
		penaltyShooterAimKickTimer = 0;
		performPenaltyShooterAimKick();
		return;
	}
	else {
		setSpinKickParams();
		return;
	}
}

void PenaltyShooter::performPenaltyShooterAimKick() {
	penaltyShooterAimKickTimer++;

	// to keep ball from popping
	if (penaltyShooterAimKickTimer < 5) {
		saBallTracking();
		setHoldParams();
		return;
	}

	// look up, expecting to see target goal
	if (penaltyShooterAimKickTimer < 8) {
		headtype = ABS_H;
		tilty = 5;
		panx = 0;

		setHoldParams();
		return;
	}

	// can see target goal
	if (vtGoal.cf > 25) {
		// set head to continue to look at target goal
		headtype = ABS_H;
		tilty = 5;
		panx = Cap(vtGoal.h, 30.0);

		if (ABS(vtGoal.h) <= 4) {
			headtype = ABS_H;
			tilty = 0;
			panx = 0;

			lockMode = GoalieKick;
			aaGoalieKick();

			// reset state
			resetPenaltyShooterAimKick();
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

void PenaltyShooter::resetPenaltyShooterAimKick() {
	penaltyShooterAimKickStarted = false;
	penaltyShooterAimKickTimer = 0;
}
