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
 * Strategy:
 *  aaVisOppAvoidanceKick
 *
 * Description:
 *  spins with ball under snout,
 *  looks up at target goal,
 *  attempts to score by shooting
 *  away from goalie
 */

#include "Behaviours.h"
#include <algorithm>
#include <vector>

// visual opp avoidance kick state
static bool visOppAvoidanceStarted = false;
static int visOppAvoidanceTimer = 0;

// shooting strategy lock state
static const int NO_SHOOTING_STRATEGY = 0;
static const int MIDDLE_TARGET_GOAL = 1;
static const int LEFT_GAP_TARGET_GOAL = 2;
static const int RIGHT_GAP_TARGET_GOAL = 3;
static const int OPP_AVOIDANCE_LEFT = 4;
static const int OPP_AVOIDANCE_RIGHT = 5;

static int shootingStrategyLock = NO_SHOOTING_STRATEGY;

static double currOffsetFromLeftGoalPost = 0;
static double currOffsetFromRightGoalPost = 0;

static double currHeadingToLeftMostOppBetweenPosts = 0;
static double currHeadingToRightMostOppBetweenPosts = 0;

static bool currSelectedLeftGap = false;
static bool currSelectedRightGap = false;

static double currSelectedLeftGapHeading = 0;
static double currSelectedRightGapHeading = 0;

static int numFramesWithinDesiredHeadRange = 0;

void Behaviours::aaVisOppAvoidanceKick() {
	leds(2, 2, 2, 2, 2, 2);
	/*
	// break out of kick if timer expires
	if (!grabTime) {
	gps->currentlySpinKicking = false;
	// reset state
	resetVisOppAvoidanceKickState();
	// if facing edge
	if (vision->facingFieldEdge) {
	double dir;
	if (gps->self().pos.x < FIELD_WIDTH/2.0) {
	dir = -90;
	} else {
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
	*/

	if (visOppAvoidanceStarted) {
		performVisOppAvoidanceKick();
		return;
	}

	double accuracyRequirement = requiredAccuracy(20);
	accuracyRequirement = MAX(accuracyRequirement, 5);
	if (ABS(gps->tGoal().pos.getHead()) <= accuracyRequirement) {
		gps->currentlySpinKicking = false;

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

		visOppAvoidanceStarted = true;
		visOppAvoidanceTimer = 0;
		shootingStrategyLock = NO_SHOOTING_STRATEGY;

		performVisOppAvoidanceKick();
		return;
	}
	else {
		setSpinKickParams();
		return;
	}
}

void Behaviours::performVisOppAvoidanceKick() {
	visOppAvoidanceTimer++;

	// to keep ball from popping
	if (visOppAvoidanceTimer < 5) {
		saBallTracking();
		setHoldParams();
		return;
	}

	// look up, expecting to see target goal
	if (visOppAvoidanceTimer < 8) {
		headtype = ABS_H;
		tilty = 5;
		panx = 0;

		setHoldParams();
		return;
	}

	// if already chosen shooting strategy, continue to perform it
	if (shootingStrategyLock != NO_SHOOTING_STRATEGY) {
		switch (shootingStrategyLock) {
			case MIDDLE_TARGET_GOAL: shootAtMiddleOfTargetGoal();
						 break;

			case LEFT_GAP_TARGET_GOAL: shootAtLeftGapOfTargetGoal(currOffsetFromLeftGoalPost);
						   break;

			case RIGHT_GAP_TARGET_GOAL: shootAtRightGapOfTargetGoal(currOffsetFromRightGoalPost);
						    break;

			case OPP_AVOIDANCE_LEFT: shootBetweenOppsLeft(); 
						 break;

			case OPP_AVOIDANCE_RIGHT: shootBetweenOppsRight();
						  break;
		}

		return;
	}

	// can see target goal
	if (vtGoal.cf > 25) {
		// set head to continue to look at target goal
		headtype = ABS_H;
		tilty = 5;
		panx = Cap(vtGoal.h, 30.0);

		// no opponents detected between target goal posts
		if (!oppBetweenTargetGoalPosts()) {
			cout << "*** no goalie in way" << endl;
			shootingStrategyLock = MIDDLE_TARGET_GOAL;
			shootAtMiddleOfTargetGoal();
			return;
		}

		/*
		// if distance to goal too great
		if (vtGoal.d > FIELD_LENGTH/2) {
		cout << "*** too far from goal" << endl;
		shootingStrategyLock = MIDDLE_TARGET_GOAL;
		shootAtMiddleOfTargetGoal();
		return;
		}
		*/

		double leftGapSize, rightGapSize;
		double headingToLeftGap, headingToRightGap;

		getLeftGapSizeHeading(&leftGapSize, &headingToLeftGap);
		getRightGapSizeHeading(&rightGapSize, &headingToRightGap);

		// if both left and right gaps are large enough to shoot through
		if (isGapLargeEnough(leftGapSize) && isGapLargeEnough(rightGapSize)) {
			cout << "*** both gaps large enough" << endl;

			if (leftGapSize > rightGapSize) {
				shootingStrategyLock = LEFT_GAP_TARGET_GOAL;
				currOffsetFromLeftGoalPost = leftGapSize / 2;
				shootAtLeftGapOfTargetGoal(currOffsetFromLeftGoalPost);
				return;
			}
			else if (leftGapSize < rightGapSize) {
				shootingStrategyLock = RIGHT_GAP_TARGET_GOAL;
				currOffsetFromRightGoalPost = rightGapSize / 2;
				shootAtRightGapOfTargetGoal(currOffsetFromRightGoalPost);
				return;
			}
			else {
				// left gap size equals right gap size,
				// select gap which needs the least turn

				if (abs(headingToLeftGap) < abs(headingToRightGap)) {
					shootingStrategyLock = LEFT_GAP_TARGET_GOAL;
					currOffsetFromLeftGoalPost = leftGapSize / 2;
					shootAtLeftGapOfTargetGoal(currOffsetFromLeftGoalPost);
					return;
				}
				else {
					shootingStrategyLock = RIGHT_GAP_TARGET_GOAL;
					currOffsetFromRightGoalPost = rightGapSize / 2;
					shootAtRightGapOfTargetGoal(currOffsetFromRightGoalPost);
					return;
				}
			}
		}

		// if left gap large enough to shoot through
		if (isGapLargeEnough(leftGapSize)) {
			cout << "*** only left gap large enough" << endl;
			shootingStrategyLock = LEFT_GAP_TARGET_GOAL;
			currOffsetFromLeftGoalPost = leftGapSize / 2;
			shootAtLeftGapOfTargetGoal(currOffsetFromLeftGoalPost);
			return;
		}

		// if right gap large enough to shoot through
		if (isGapLargeEnough(rightGapSize)) {
			cout << "*** only right gap large enough" << endl;
			shootingStrategyLock = RIGHT_GAP_TARGET_GOAL;
			currOffsetFromRightGoalPost = rightGapSize / 2;
			shootAtRightGapOfTargetGoal(currOffsetFromRightGoalPost);
			return;
		}

		// even if the gaps are too narrow, if own heading
		// is too stilted, shoot at the larger gap
		if (gps->self().h <30 || gps->self().h> 150) {
			cout << "*** heading too stilted" << endl;
			if (leftGapSize > rightGapSize) {
				shootingStrategyLock = LEFT_GAP_TARGET_GOAL;
				currOffsetFromLeftGoalPost = leftGapSize / 2;
				shootAtLeftGapOfTargetGoal(currOffsetFromLeftGoalPost);
				return;
			}
			else {
				shootingStrategyLock = RIGHT_GAP_TARGET_GOAL;
				currOffsetFromRightGoalPost = rightGapSize / 2;
				shootAtRightGapOfTargetGoal(currOffsetFromRightGoalPost);
				return;
			}
		}

		// only for the purpose of debugging/testing
		lockMode = ChestPush;
		aaChestPush();

		resetVisOppAvoidanceKickState();


		/////////////////////////////////////////////////////////////
		// The gaps are not big enough to allow us to score
		// Attempt to shoot between opponents
		/////////////////////////////////////////////////////////////
		/*
		   if (isTeammateInLeftQuadrant()) {
		   shootingStrategyLock = OPP_AVOIDANCE_LEFT;
		   shootBetweenOppsLeft();
		   return;
		   }

		   if (isTeammateInRightQuadrant()) {
		   shootingStrategyLock = OPP_AVOIDANCE_RIGHT;
		   shootBetweenOppsRight();
		   return;
		   }

		// no teammates in either quadrants,
		// select quadrant by considering own heading
		if (isFacingLeftHalf()) {
		shootingStrategyLock = OPP_AVOIDANCE_LEFT;
		shootBetweenOppsLeft();
		return;
		} else {
		shootingStrategyLock = OPP_AVOIDANCE_RIGHT;
		shootBetweenOppsRight();
		return;
		}
		*/
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

bool Behaviours::oppBetweenTargetGoalPosts() {
	bool flag = false;

	double headingToLeftGoalPost = vtGoal.vob->lhead;
	double headingToRightGoalPost = vtGoal.vob->rhead;

	cout << "----------------------------------------------" << endl;
	cout << "left post: " << headingToLeftGoalPost << endl;
	cout << "right post: " << headingToRightGoalPost << endl;

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		// is this correct?
		// should be using variance - Ross
		if (vOpponent[i].cf <= 0) {
			cout << i << ": not seen" << endl;
			continue;
		}    

		cout << i << " : " << vOpponent[i].h << endl; 

		if (vOpponent[i].h <= (headingToLeftGoalPost + 5)
				&& vOpponent[i].h >= (headingToRightGoalPost - 5))
			flag = true;
	}

	cout << "----------------------------------------------" << endl;

	if (flag) {
		return true;
	}
	return false;
}

void Behaviours::getLeftGapSizeHeading(double *size, double *head) {
	double headingToLeftGoalPost = vtGoal.vob->lhead;
	double headingToRightGoalPost = vtGoal.vob->rhead;

	double maxHeadingToOpponentSoFar = -LARGE_VAL;

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vOpponent[i].cf <= 0)
			continue;

		if (vOpponent[i].h > (headingToLeftGoalPost + 5)
				|| vOpponent[i].h < (headingToRightGoalPost - 5))
			continue;

		if (vOpponent[i].h > maxHeadingToOpponentSoFar)
			maxHeadingToOpponentSoFar = vOpponent[i].h;
	}

	currHeadingToLeftMostOppBetweenPosts = maxHeadingToOpponentSoFar;
	*size = abs(headingToLeftGoalPost - maxHeadingToOpponentSoFar);
	*head = headingToLeftGoalPost - (*size) / 2;
}

void Behaviours::getRightGapSizeHeading(double *size, double *head) {
	double headingToLeftGoalPost = vtGoal.vob->lhead;
	double headingToRightGoalPost = vtGoal.vob->rhead;

	double minHeadingToOpponentSoFar = LARGE_VAL;

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (vOpponent[i].cf <= 0)
			continue;

		if (vOpponent[i].h > (headingToLeftGoalPost + 5)
				|| vOpponent[i].h < (headingToRightGoalPost - 5))
			continue;

		if (vOpponent[i].h < minHeadingToOpponentSoFar)
			minHeadingToOpponentSoFar = vOpponent[i].h;
	}

	currHeadingToRightMostOppBetweenPosts = minHeadingToOpponentSoFar;
	*size = abs(minHeadingToOpponentSoFar - headingToRightGoalPost);
	*head = headingToRightGoalPost + (*size) / 2;
}

bool Behaviours::isGapLargeEnough(double gapSize) {
	return (gapSize > 10);
}

void Behaviours::shootAtMiddleOfTargetGoal() {
	cout << "MIDDLE" << endl;
	// shoot at middle of target goal
	if (ABS(vtGoal.h) < 4 && numFramesWithinDesiredHeadRange > 4) {
		headtype = ABS_H;
		tilty = 0;
		panx = 0;

		lockMode = GoalieKick;
		aaGoalieKick();

		// reset state
		resetVisOppAvoidanceKickState();
	}
	else {
		if (ABS(vtGoal.h) < 4)
			numFramesWithinDesiredHeadRange++;

		setCarryParams();
		turnCCW = ABS(vtGoal.h) > 9.5 ? vtGoal.h : (vtGoal.h > 0 ? 9.5 : -9.5);
		left = -turnCCW / 6.0;
	}
}

void Behaviours::shootAtLeftGapOfTargetGoal(double offsetFromLeftPostHeading) {
	cout << "LEFT" << endl;
	// shoot at left gap
	double desiredHeading = vtGoal.vob->lhead - offsetFromLeftPostHeading;
	if (ABS(desiredHeading) < 4 && numFramesWithinDesiredHeadRange > 4) {
		headtype = ABS_H;
		tilty = 0;
		panx = 0;

		lockMode = GoalieKick;
		aaGoalieKick();

		// reset state
		resetVisOppAvoidanceKickState();
	}
	else {
		if (ABS(desiredHeading) < 4)
			numFramesWithinDesiredHeadRange++;

		setCarryParams();
		turnCCW = ABS(desiredHeading)
			> 9.5
			? desiredHeading
			: (desiredHeading > 0 ? 9.5 : -9.5);
		left = -turnCCW / 6.0;
	}
}

void Behaviours::shootAtRightGapOfTargetGoal(double offsetFromRightPostHeading) {
	cout << "RIGHT" << endl;
	// shoot at right gap
	double desiredHeading = vtGoal.vob->rhead + offsetFromRightPostHeading;
	if (ABS(desiredHeading) < 4 && numFramesWithinDesiredHeadRange > 4) {
		headtype = ABS_H;
		tilty = 0;
		panx = 0;

		lockMode = GoalieKick;
		aaGoalieKick();

		// reset state
		resetVisOppAvoidanceKickState();
	}
	else {
		if (ABS(desiredHeading) < 4)
			numFramesWithinDesiredHeadRange++;

		setCarryParams();
		turnCCW = ABS(desiredHeading)
			> 9.5
			? desiredHeading
			: (desiredHeading > 0 ? 9.5 : -9.5);
		left = -turnCCW / 6.0;
	}
}

bool Behaviours::isTeammateInLeftQuadrant() {
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		// should consider variance?

		const Vector &tm = gps->teammate(i).pos;
		if (tm.head < 0 && tm.head >= -90)
			return true;
	}
	return false;
}

bool Behaviours::isTeammateInRightQuadrant() {
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		// should consider variance?

		const Vector &tm = gps->teammate(i).pos;
		if (tm.head > 0 && tm.head <= 90)
			return true;
	}
	return false;
}

bool Behaviours::isFacingLeftHalf() {
	return (gps->self().h > 90 && gps->self().h < 270);
}

void Behaviours::shootBetweenOppsLeft() {
	// set head to look to the left for opponents
	// continue to look to the left until you no longer
	// see the goal
	if (vtGoal.cf > 0) {
		headtype = REL_H;
		tilty = 0;
		panx = 5;
		cranez = 0;
		setCarryParams();
		return;
	}

	// set head params to look straight ahead
	headtype = ABS_H;
	tilty = 5;
	panx = 0;
	cranez = 0;

	// if haven't chosen the most desirable gap to shoot, choose ...
	if (!currSelectedLeftGap) {
		// no longer sees target goal
		// pick the best gap in which to shoot
		double startBoundary = currHeadingToLeftMostOppBetweenPosts;
		double endBoundary = startBoundary + 45;

		// store boundaries
		vector<double> boundaries;

		// append start boundary to vector
		boundaries.push_back(startBoundary);

		// if a visual opponent falls within the start and end boundaries
		// consider it as a boundary also
		for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
			if (vOpponent[i].cf <= 0)
				continue;                

			// if heading to opponent falls within limit boundaries
			if (vOpponent[i].h > startBoundary && vOpponent[i].h < endBoundary) {
				boundaries.push_back(vOpponent[i].h);
			}
		}    

		// append end boundary to vector
		boundaries.push_back(endBoundary);

		// sort boundaries
		sort(boundaries.begin(), boundaries.end());

		// find the first gap that is big enough to shoot through
		static const int GAP_THRESHOLD = 15;

		int gapsTooSmall = true;
		double headingToSelectedGap = -1;

		for (unsigned int i = 0; i < boundaries.size() - 1; i++) {
			double gapSize = boundaries[i + 1] - boundaries[i];
			if (gapSize > GAP_THRESHOLD) {
				gapsTooSmall = false;
				headingToSelectedGap = (boundaries[i + 1] + boundaries[i]) / 2;
				break;
			}
		}    

		if (gapsTooSmall) {
			// gap not found, shoot immediately
			lockMode = GoalieKick;
			aaGoalieKick();
			resetVisOppAvoidanceKickState();
			return;
		}
		else {
			// found a large enough gap, shoot through that gap
			// use headingToSelectedGap to estimate how much to spin?

			currSelectedLeftGap = true;
			currSelectedLeftGapHeading = headingToSelectedGap + gps->self().h;

			// if aligned closely with gap, shoot
			if (abs(currSelectedLeftGapHeading - gps->self().h) <= 5) {
				lockMode = GoalieKick;
				aaGoalieKick();
				resetVisOppAvoidanceKickState();
				return;
			}    

			gps->currentlyCarrying = true;
			setCarryParams();
			turnCCW = 30;
			left = -5;
		}
	}
	else {
		// if aligned closely with gap, shoot
		if (abs(currSelectedLeftGapHeading - gps->self().h) <= 5) {
			lockMode = GoalieKick;
			aaGoalieKick();
			resetVisOppAvoidanceKickState();
			return;
		}    

		gps->currentlyCarrying = true;
		setCarryParams();
		turnCCW = 30;
		left = -5;
	}
}

void Behaviours::shootBetweenOppsRight() {
	// set head to look to the right for opponents
	// continue to look to the right until you no longer
	// see the goal
	if (vtGoal.cf > 0) {
		headtype = REL_H;
		tilty = 0;
		panx = -5;
		cranez = 0;
		setCarryParams();
		return;
	}

	// set head params to look straight ahead
	headtype = ABS_H;
	tilty = 5;
	panx = 0;
	cranez = 0;

	// if haven't chosen the most desirable gap to shoot, choose ...
	if (!currSelectedRightGap) {
		// no longer sees target goal
		// pick the best gap in which to shoot
		double startBoundary = currHeadingToRightMostOppBetweenPosts;
		double endBoundary = startBoundary - 45;

		// store boundaries
		vector<double> boundaries;

		// append start boundary to vector
		boundaries.push_back(startBoundary);

		// if a visual opponent falls within the start and end boundaries
		// consider it as a boundary also
		for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
			if (vOpponent[i].cf <= 0)
				continue;                

			// if heading to opponent falls within limit boundaries
			if (vOpponent[i].h <startBoundary && vOpponent[i].h> endBoundary) {
				boundaries.push_back(vOpponent[i].h);
			}
		}    

		// append end boundary to vector
		boundaries.push_back(endBoundary);

		// reverse sort boundaries
		sort(boundaries.begin(), boundaries.end());
		reverse(boundaries.begin(), boundaries.end());

		// find the first gap that is big enough to shoot through
		static const int GAP_THRESHOLD = 15;

		int gapsTooSmall = true;
		double headingToSelectedGap = -1;

		for (unsigned int i = 0; i < boundaries.size() - 1; i++) {
			// should take the absolute value anyway?
			double gapSize = boundaries[i] - boundaries[i + 1];
			if (gapSize > GAP_THRESHOLD) {
				gapsTooSmall = false;
				headingToSelectedGap = (boundaries[i + 1] + boundaries[i]) / 2;
				break;
			}
		}    

		if (gapsTooSmall) {
			// gap not found, shoot immediately
			lockMode = GoalieKick;
			aaGoalieKick();
			resetVisOppAvoidanceKickState();
			return;
		}
		else {
			// found a large enough gap, shoot through that gap
			// use headingToSelectedGap to estimate how much to spin?

			currSelectedRightGap = true;
			currSelectedRightGapHeading = headingToSelectedGap + gps->self().h;

			// if aligned closely with gap, shoot
			if (abs(currSelectedRightGapHeading - gps->self().h) <= 5) {
				lockMode = GoalieKick;
				aaGoalieKick();
				resetVisOppAvoidanceKickState();
				return;
			}    

			// start to turn right
			gps->currentlyCarrying = true;
			setCarryParams();
			turnCCW = -30;
			left = 5;
		}
	}
	else {
		// if aligned closely with gap, shoot
		if (abs(currSelectedRightGapHeading - gps->self().h) <= 5) {
			lockMode = GoalieKick;
			aaGoalieKick();
			resetVisOppAvoidanceKickState();
			return;
		}    

		// continue to turn right
		gps->currentlyCarrying = true;
		setCarryParams();
		turnCCW = -30;
		left = 5;
	}
}

void Behaviours::resetVisOppAvoidanceKickState() {
	visOppAvoidanceStarted = false;
	visOppAvoidanceTimer = 0;

	shootingStrategyLock = NO_SHOOTING_STRATEGY;

	currOffsetFromLeftGoalPost = 0;
	currOffsetFromRightGoalPost = 0;

	currHeadingToLeftMostOppBetweenPosts = 0;
	currHeadingToRightMostOppBetweenPosts = 0;

	currSelectedLeftGap = false;
	currSelectedRightGap = false;

	currSelectedLeftGapHeading = 0;
	currSelectedRightGapHeading = 0;

	numFramesWithinDesiredHeadRange = 0;
}
