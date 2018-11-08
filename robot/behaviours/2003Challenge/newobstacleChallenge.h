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
 * $Id: newobstacleChallenge.h 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * newobstacleChallenge
 *
 * An interface with different behaviours separated into different files.
 *
 **/

#ifndef _newobstacleChallenge_h_DEFINED
#define _newobstacleChallenge_h_DEFINED

#include "Behaviours.h"

using namespace Behaviours;

namespace newobstacleChallenge {
	using Behaviours::left;


	//////////////////////////////////
	// Obstacle avoidance challenge //
	//////////////////////////////////

	extern void initNewObsChallenge();
	extern void doNewObsChallenge();
	extern void setNewObsChallengeValues();
	extern void decideNewObsChallengeNextAction();

	extern void newobsChallengeLookAroundHeadParams();
	extern void newobsChallengeWalkParams();
	extern void newobsChallengeStationary();
	extern void newobsChallengeUTurn();
	extern void newobsChallengeGoal();

	extern void newobsChallengeInitWorldMap();
	extern void newobsChallengeUpdateWorldMap();
	extern void newobsChallengePrintWorldMap();

	extern void newobsChallengeInitActionGrid();
	extern void newobsChallengeUpdateActionGrid();
	extern void newobsChallengeFullUpdateActionGrid();

	extern void newobsChallengeGenerateActionGrid();
	extern void newobsChallengePrintActionGrid();
	extern void newobsChallengePrintQValues();

	extern double newobsChallengeReward(int fromR, int fromC, int toR, int toC);
	extern double newobsChallengeProb(int fromR,
			int fromC,
			int toR,
			int toC,
			int action);
	extern double newobsChallengeMaxQ(int r, int c);
	extern int newobsChallengeMaxQAction(int r, int c);

	extern void newobsChallengeHeapClear();
	extern void newobsChallengeHeapAdd(int row, int col, double priority);
	extern int newobsChallengeHeapRemove();
	extern void newobsChallengeHeapPrint();

	extern void newobsChallengeQueueClear();
	extern void newobsChallengeQueueAdd(int row, int col);
	extern int newobsChallengeQueueRemove();

	extern void newobsChallengeUpdateDebugCount();
	extern void newobsChallengeUpdatePosition(); 
	extern void newobsChallengeUpdateActLocaliseCount(); 
	extern void newobsChallengeDetermineSpeed(int fromR,
			int fromC,
			double walkHeading);
	extern bool newobsChallengeIsOnEdge();
	extern bool newobsChallengeLockInOwnGoal();
	extern bool newobsChallengeNearOwnGoal();

	extern bool newobsChallengeRobotTracking(double head = 0, double range = 10);
	extern double newobsChallengeInfraRedRobotDist(double head = 0,
			double range = 10);
	extern double newobsChallengeVisionRobotDist(double head = 0,
			double range = 10);
}

#endif // _newobstacleChallenge_h_DEFINED
