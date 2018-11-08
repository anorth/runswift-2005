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
 * $Id: Routines.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Basic utility routines
 * 
 **/

#ifndef ROUTINES_H
#define ROUTINES_H
#include "../share/Common.h"
//#include "StrategyVariables.h"

class Routines {
	// : public StrategyVariables {

	public:
		/* General Routines */
		void findAngleDistTo(CurObject &to, WMObject &from);
		void findAngleDistTo(WMObject &to, WMObject &from);
		double cap(double value, double max);
		// double  pathDistance(int a, int b, int c, int d, int e);
		double normaliseAngle(double a);
		bool isAlignedToScore(WMObject &robot, CurObject &ball);
};

inline double Routines::cap(double param, double abs_limit) {
	if (param > abs_limit) {
		return abs_limit;
	}
	else if (param < -abs_limit) {
		return -abs_limit;
	}
	return param;
}

inline void Routines::findAngleDistTo(CurObject &to, WMObject &from) {
	double relx = to.x - from.x;
	double rely = to.y - from.y;
	to.d = sqrt(SQUARE(relx) + SQUARE(rely));
	to.h = (relx == 0 && rely == 0)
		? 0
		: NormalizeAngle_180(RAD2DEG(atan2(rely, relx))
				- from.f);
}

inline void Routines::findAngleDistTo(WMObject &to, WMObject &from) {
	double relx = to.x - from.x;
	double rely = to.y - from.y;
	to.d = sqrt(SQUARE(relx) + SQUARE(rely));
	to.h = (relx == 0 && rely == 0)
		? 0
		: NormalizeAngle_180(RAD2DEG(atan2(rely, relx))
				- from.f);
}

inline double Routines::normaliseAngle(double a) {
	while (a <= -180.0) {
		a += 360.0;
	}
	while (a > 180.0) {
		a -= 360.0;
	}
	return a;
}

inline bool Routines::isAlignedToScore(WMObject &robot, CurObject &ball) {
	// if robot is close to end wall then it's not in a good aligned position
	if (robot.y > FIELD_LENGTH - WALL_THICKNESS - 5 && ABS(robot.x
				- FIELD_WIDTH
				/ 2) > GOAL_WIDTH / 2) {
		return false;
	}

	// projecting the position of the ball towards end wall in the same direction as robot to ball.
	double projection = robot.x
		- (FIELD_LENGTH - WALL_THICKNESS - robot.y) * tan(DEG2RAD(robot.h
					+ ball.h
					- 90));

	//cout << "Projection: " << projection << endl;

	return ABS(projection - FIELD_WIDTH / 2) < 20;
}

#endif
