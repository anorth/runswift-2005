/*

   Copyright 2005 The University of New South Wales (UNSW) and National  
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
#include "../vision/VisualCortex.h"

#include <algorithm>
#include <vector>
#include <utility>

static const bool voakSilence = true;

// visual opp avoidance kick state
static bool visOppAvoidanceStarted = false;
static int visOppAvoidanceTimer = 0;

static int numFramesWithinBestGapRange = 0;
static bool currFrameNoGaps = false;
static bool firstFrameToDecideBestGap = true;
static unsigned int prevFrameGapIndexSelected = 0;
static unsigned int prevFrameNumOfGaps = 0;

static int bestGapIndexSelected = false;
static int bestGapIndexRecorded = 0;
static int fstFrameBestGapIndex = 0;
static int sndFrameBestGapIndex = 0;
static int thdFrameBestGapIndex = 0;

//erics odometry test stuff
static int acount = 0;
static int doubleCount = 0;

#ifdef COMPILE_ALL_CPP
void Behaviours::aaVisOppAvoidanceKick() {
	if ( !voakSilence)
	  cout << "GRAB TIME: " << grabTime << endl;
	
	// break out of kick if timer expires
	if (!grabTime) {
		if ( !voakSilence)
		  cout << "TIME OUT!" << endl;

		// reset state
		resetVisOppAvoidanceKickState();

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

		lockMode = GoalieKick;
		aaGoalieKick();

		return;
	}

	if (visOppAvoidanceStarted) {
		performVisOppAvoidanceKick();
		return;
	}

	double accuracyRequirement = requiredAccuracy(20);
	accuracyRequirement = MAX(accuracyRequirement, 5);

	if (!voakSilence)	    cout << "voak: required Accuracy "<<
				      accuracyRequirement << endl ;

	if (ABS(gps->tGoal().pos.getHead()) <= accuracyRequirement) {
		if (vision->facingFieldEdge) {
			double dir;
			if (gps->self().pos.x < FIELD_WIDTH / 2.0) {
			  aaUPkickRight ();
			  cout << "voak: upenn right\n";
			  //dir = -90;
			}
			else {
			  cout << "voak: upenn left\n";
			  aaUPkickLeft ();
			  //dir = 90;
			}
			//lockMode = ProperVariableTurnKick; //setProperVariableTurnKick(dir);
			//aaProperVariableTurnKick();      
			return;
		}

		visOppAvoidanceStarted = true;
		visOppAvoidanceTimer = 0;
		performVisOppAvoidanceKick();
		return;
	}
	else {
		setSpinKickParams();
		cout << "voak: Set Spin kick params\n";
		return;
	}
}

void Behaviours::performVisOppAvoidanceKick() {

	visOppAvoidanceTimer++;
	if (!voakSilence)	    cout << "voak :visOppAvoidanceTimer "<<
				      visOppAvoidanceTimer<<endl ;
	// to keep ball from popping
	if (visOppAvoidanceTimer < 12) {
	  if (!voakSilence)
	    cout << "holding the ball\n";
		headtype = ABS_H;
		panx = 0;
		tilty = -50;
		cranez = 30;
		setHoldParams();
		return;
	}

	// look up, expecting to see target goal
	// takes 7-8 frames to go from tilt -50 to 5
	if (visOppAvoidanceTimer < 19) {
	  if (!voakSilence)
	    cout << "look up and holding the ball\n";
		headtype = ABS_H;
		tilty = 5;
		panx = 0;

		setHoldParams();
		return;
	}

	// if you can see the ball, unlock from routine
	if (vBall.cf > 20 && vBall.d > 15) {
	  if (!voakSilence)
	    cout << "how come i see the ball\n";
		lockMode = NoMode;    
		chooseStrategy();
		return;
	}

	// can see target goal
	if (vtGoal.cf > 0) {
		// set head to continue to look at target goal
		headtype = ABS_H;
		tilty = 5;
		panx = Cap(vtGoal.h, 30.0);

		// find best gap to shoot through
		double min, max, headingOfBestGap;
		getHeadingToBestGap(&min, &max);
		headingOfBestGap = (min + max) / 2.0;
		if (!voakSilence) {
			cout << "***************************" << endl;
//			cout << "left goal:   " << vtGoal.vob->lhead << endl;
//			cout << "middle goal: " << vtGoal.h << endl;
//			cout << "right goal:  " << vtGoal.vob->rhead << endl << endl;
			cout << "gap:         " << headingOfBestGap << endl;
			cout << "min - max :  " << min << " to " << max << endl;
			cout << "***************************" << endl;
		}

		if (currFrameNoGaps) {
			if (!voakSilence)
				cout << "no gaps!" << endl;
			resetVisOppAvoidanceKickState();

			lockMode = NoMode;
			// Ted: removed chest pass.
			// lockMode = BeckhamChestPass;
			// Beckham::aaBeckhamChestPass();
			return;
		}

		if ((ABS(headingOfBestGap) <= 2 || (min <= -4 && max >= 4))
				&& numFramesWithinBestGapRange > 2) {
			headtype = ABS_H;
			tilty = 0;
			panx = 0;

			lockMode = GoalieKick;
			aaGoalieKick();

			// reset state
			resetVisOppAvoidanceKickState();
			if (!voakSilence)
			    cout << "SHOOT!" << endl;
		}
		else {
			if (ABS(headingOfBestGap) <= 2 || (min <= -1 && max >= 1))
				numFramesWithinBestGapRange++;
			setCarryParams();
			headtype = ABS_H;
			panx = 0;
			tilty = -50;
			cranez = 30;
			turnCCW = ABS(headingOfBestGap)	> 9.5
				? headingOfBestGap
				: (headingOfBestGap > 0 ? 9.5 : -9.5);
			left = -turnCCW / 6.0;
			if (!voakSilence)
			    cout << "NO SHOOT! turn " << turnCCW << endl;
		}
		if (firstFrameToDecideBestGap)
			firstFrameToDecideBestGap = false;
		return;
	}

	// cannot see goal, spin around
	headtype = ABS_H;
	tilty = 5;
	panx = 0;
	//panx = Cap(gps->tGoal().pos.head, 30);

	if (!voakSilence)
	    cout << "CANT SEE GOAL!" << endl;

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

void Behaviours::resetVisOppAvoidanceKickState() {
	visOppAvoidanceStarted = false;
	visOppAvoidanceTimer = 0;

	numFramesWithinBestGapRange = 0;
	currFrameNoGaps = false;
	firstFrameToDecideBestGap = true;
	prevFrameGapIndexSelected = 0;
	prevFrameNumOfGaps = 0;

	bestGapIndexSelected = false;
	bestGapIndexRecorded = 0;
	fstFrameBestGapIndex = 0;
	sndFrameBestGapIndex = 0;
	thdFrameBestGapIndex = 0;
	if (!voakSilence)
	  cout << "viak reset\n";
}
#endif //COMPILE_ALL_CPP

bool Behaviours::sortRobotsFromLeftToRight(CurObject obj1, CurObject obj2) {
	return (obj1.h > obj2.h);
}


void Behaviours::getHeadingToBestGap2005(double *min, double *max) {
  /* This new method will do a line scan just below the center of the goal box to find out the largest gap.
     -- weiming
  */	

    // FIXME: removed for subvision - this needs to be added back
    *min = *max = 0;

#if 0

  bool debug = false;
  
  const VisualObject *gVob = vtGoal.vob;

  if (gVob->cf <= 0) {
    *min = *max = 0;
    return;
  }

  int targetGoalColour = (gps->targetGoal == vobYellowGoal) ? cBEACON_YELLOW : cBEACON_BLUE;

  int leftGoalPostX = (int) gVob->x;
  int rightGoalPostX = (int) (gVob->x + gVob->width);
  
  int scanLineMinY = (int) (gVob->y + gVob->height/2.0);
  int scanLineHeight = (int) (gVob->height / 3.0);
  int scanLineMaxY = scanLineMinY + scanLineHeight - 1;

  /* chop the scan line into 100 small blocks */
  int blockWidth = (int) gVob->width / 100;
  if (blockWidth == 0)
    blockWidth = 1;
  int blockSize = blockWidth * scanLineHeight;
  int pixelSize;

  int bestGapX;
  int bestGapWidth;
  int currentGapX;
  int currentGapWidth;

  bestGapX = currentGapX = leftGoalPostX;
  bestGapWidth = currentGapWidth = 0;

  if (debug)
    cout << "leftGoalPostX: " << leftGoalPostX << " rightGoalPostX: " << rightGoalPostX
	 << endl
	 << "scanLineMinY: " << scanLineMinY << " scanLineHeight: " << scanLineHeight
	 << endl;


  for (int blockX = leftGoalPostX; blockX+blockWidth < rightGoalPostX; blockX += blockWidth) {
    pixelSize = vision->countColouredPixels(targetGoalColour,
					    blockX,
					    scanLineMinY,
					    blockX + blockWidth - 1,
					    scanLineMaxY);

    /*
    if (debug)
      cout << "blockX: " << blockX << "    rightGoalPostX: " << rightGoalPostX
	   << endl
	   << "Color Ratio:" << pixelSize/(double)blockSize
	   << endl;
    */
    
    if (pixelSize / (double) blockSize >= .51) { //.51 is a really big ratio!!
      // if the block is pure enough, add the block to the current gap
      currentGapWidth += blockWidth;

      if (currentGapWidth > bestGapWidth) {
	// if current gap is wider than best gap, make it the best gap
	bestGapX = currentGapX;
	bestGapWidth = currentGapWidth;
      }
    } 
    else {
      // if the block is not pure, reset current gap to a new gap
      currentGapX = blockX + blockWidth;
      currentGapWidth = 0;
    }    
  }

  if (debug)
    cout << "bestGapX:" << bestGapX << " bestGapWidth: " << bestGapWidth << endl;

  if (bestGapWidth == 0) {
    *min = *max = 0;
  }
  else {
    double headingToLeftSideOfGap = vtGoal.h
      + (PointToHeading(bestGapX)
	 - PointToHeading(vtGoal.vob->cx));
    double headingToRightSideOfGap = vtGoal.h
      + (PointToHeading(bestGapX+bestGapWidth-1)
	 - PointToHeading(vtGoal.vob->cx));
    *min = MIN(headingToLeftSideOfGap, headingToRightSideOfGap);
    *max = MAX(headingToLeftSideOfGap, headingToRightSideOfGap);
  }
#endif
}

void Behaviours::getHeadingToBestGap(double *min, double *max) {
    // Removed. See getHeadingToBestGap2005
    *min = *max = 0;
    return;
#if 0

	double headingToLeftGoalPost = vtGoal.vob->lhead;
	double headingToRightGoalPost = vtGoal.vob->rhead;

	const double xPixelValueLeftGoalPost = vtGoal.vob->x;
	const double xPixelValueRightGoalPost = vtGoal.vob->x + vtGoal.vob->width;


	vector<CurObject> blockingRobots;
	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		// consider opponent robots
		if (vOpponent[i].cf <= 0)
			continue;
		if (vOpponent[i].vob->rhead > headingToLeftGoalPost)
			continue;
		if (vOpponent[i].vob->lhead < headingToRightGoalPost)
			continue;

		blockingRobots.push_back(vOpponent[i]);
	}

	for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		// consider friendly robots
		if (vTeammate[i].cf <= 0)
			continue;
		if (vTeammate[i].vob->rhead > headingToLeftGoalPost)
			continue;
		if (vTeammate[i].vob->lhead < headingToRightGoalPost)
			continue;

		blockingRobots.push_back(vTeammate[i]);
	}

	if (blockingRobots.size() == 0) {
		*min = MIN(headingToLeftGoalPost, headingToRightGoalPost);
		*max = MAX(headingToLeftGoalPost, headingToRightGoalPost);
		return;
	}

	// sort robots from leftmost to rightmost
	sort(blockingRobots.begin(), blockingRobots.end(), sortRobotsFromLeftToRight);

	// create a vector of gaps
	// pair->first = min x, pair->second max x
	vector<pair<double, double> > gaps;

	for (unsigned int i = 0; i < blockingRobots.size() - 1; i++) {
		pair<double, double> gap(blockingRobots[i].vob->x
				+ blockingRobots[i].vob->width,
				blockingRobots[i + 1].vob->x);
		gaps.push_back(gap);
	}

	// check for left boundary
	if (blockingRobots[0].vob->lhead < headingToLeftGoalPost) {
		pair<double, double> gap(xPixelValueLeftGoalPost, blockingRobots[0].vob->x);
		gaps.push_back(gap);
	}

	// check for right boundary
	if (blockingRobots[blockingRobots.size() - 1].vob->rhead
			> headingToRightGoalPost) {
		pair<double, double> gap(blockingRobots[blockingRobots.size() - 1].vob->x
				+ blockingRobots[blockingRobots.size() - 1].vob->width,
				xPixelValueRightGoalPost);
		gaps.push_back(gap);
	}

	// if no gap, perform a chest push
	if (gaps.size() == 0) {
		currFrameNoGaps = true;
		*min = 0;
		*max = 0;
		return;
	}

	// find the biggest gap
	int biggestGapIndex = -1;
	int biggestGapSize = -1;

	int targetGoalColour = (gps->targetGoal == vobYellowGoal) ? cBEACON_YELLOW : cBEACON_BLUE;
	double minY = vtGoal.vob->y + vtGoal.vob->height / 2;
	double maxY = vtGoal.vob->y + vtGoal.vob->height - 1;
	for (unsigned int i = 0; i < gaps.size(); i++) {
		double minX = gaps[i].first;
		double maxX = gaps[i].second;
		int size = vision->countColouredPixels(targetGoalColour,
				(int) minX,
				(int) minY,
				(int) maxX,
				(int) maxY);
		if (!voakSilence) {
			cout << "\t" << i << " minX: " << minX << endl;
			cout << "\t" << i << " maxX: " << maxX << endl;
			cout << "\t" << i << " minY: " << minY << endl;
			cout << "\t" << i << " maxY: " << maxY << endl;
			cout << "\t" << i << " size: " << size << endl;
		}
		if (size > biggestGapSize) {
			biggestGapIndex = i;
			biggestGapSize = size;
		}
	}

	// find second biggest gap
	int secondBiggestGapIndex = -1;
	int secondBiggestGapSize = -1;
	if (gaps.size() > 1) {
		for (unsigned int i = 0; i < gaps.size(); i++) {
			if (i == (unsigned int) biggestGapIndex)
				continue;

			double minX = gaps[i].first;
			double maxX = gaps[i].second;
			int size = vision->countColouredPixels(targetGoalColour,
					(int) minX,
					(int) minY,
					(int) maxX,
					(int) maxY);
			if (size > secondBiggestGapSize) {
				secondBiggestGapIndex = i;
				secondBiggestGapSize = size;
			}
		}
	}

	double xPixelValueLeftSideOfGap;
	double xPixelValueRightSideOfGap;

	// update best gap indices        
	thdFrameBestGapIndex = sndFrameBestGapIndex;
	sndFrameBestGapIndex = fstFrameBestGapIndex;
	fstFrameBestGapIndex = biggestGapIndex;

	// if same gap index for three consecutive frames, then stick to it always
	if (!firstFrameToDecideBestGap
			&& !bestGapIndexSelected
			&& gaps.size() > 1
			&& ABS(biggestGapSize - secondBiggestGapSize) < 150
			&& fstFrameBestGapIndex == sndFrameBestGapIndex
			&& sndFrameBestGapIndex == thdFrameBestGapIndex) {
		bestGapIndexSelected = true;
		bestGapIndexRecorded = biggestGapIndex;
	}        

	// make sure number of gaps stay constant from frame to frame
	if (bestGapIndexSelected && gaps.size() == prevFrameNumOfGaps) {
		if (!voakSilence)
			cout << "using previous gap recorded" << endl;
		xPixelValueLeftSideOfGap = gaps[bestGapIndexRecorded].first;
		xPixelValueRightSideOfGap = gaps[bestGapIndexRecorded].second;
	}
	else {
		xPixelValueLeftSideOfGap = gaps[biggestGapIndex].first;
		xPixelValueRightSideOfGap = gaps[biggestGapIndex].second;
	}

	prevFrameNumOfGaps = gaps.size();

	double headingToLeftSideOfGap = vtGoal.h
		+ (PointToHeading(xPixelValueLeftSideOfGap)
				- PointToHeading(vtGoal.vob->cx));
	double headingToRightSideOfGap = vtGoal.h
		+ (PointToHeading(xPixelValueRightSideOfGap)
				- PointToHeading(vtGoal.vob->cx));
	*min = MIN(headingToLeftSideOfGap, headingToRightSideOfGap);
	*max = MAX(headingToLeftSideOfGap, headingToRightSideOfGap);
#endif
}

