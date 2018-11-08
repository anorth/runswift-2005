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
 * $Id: LineEquation.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * handles equation of a line
 *
 **/

#include <climits>

class LineEquation {
	float m, b;

	public:
	LineEquation(float px, float py, float qx, float qy) {
		if (px - qx == 0) {
			m = INT_MAX;
		}
		m = (py - qy) / (px - qx);
		b = py - m * px;
	}

	//the line accessing functions float getYVal(float x);
	float getXVal(float);
	float getYVal(float);
};

inline float LineEquation::getYVal(float x) {
	if (m != INT_MAX) {
		return (m * x) + b;
	}
	else {
		return 0.0;
	}
}

inline float LineEquation::getXVal(float y) {
	if (m != 0) {
		return (y - b) / m;
	}
	else {
		return 0.0;
	}
}
