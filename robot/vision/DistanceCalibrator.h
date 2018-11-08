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
 * $Id: DistanceCalibrator.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * loads up the calibration data for the ball distances.
 *
 **/

// #include <cctype>
#include <cstdio>
#include "../share/Common.h"
#include "LineEquation.h"

static const char DOT = '.';
static const char SPACE = ' ';
static const char NEW_LINE = '\n';
static const char MINUS = '-';
static const int BUFFER_SIZE = 10240;
static const int DC_MINIMUM_LIMIT = 0;
static const int DC_MAXIMUM_LIMIT = 500; // you might think this would be the max
// height of the image HEIGHT(144), but the 
// heights are much larger given that we 
// project shapes

/*
 * LineRangeElt consists of the line equation and the base value is applies too.
 * Limitations:  the user must send the correct base value
 **/
struct LineRangeElt {
	float bottom;
	LineEquation *le;
	void set(float b, LineEquation *l) {
		le = l;
		bottom = b;
	}
};

/*
 * Currently calibrates assuming that the ball is in the centre of the
 * dog's vision: getAdjustedCamera()
 * The other methods are only used on loading.  
 * The calibration file is used to create an approximation function.  
 * This function consists of a series of connected linear approximations 
 * to the actual adjustment function (which is unknown). 
 * They are stored in a variable named calibrationRanges
 **/
class DistanceCalibrator {
	LineRangeElt *calibrationRanges;
	int maxsize;
	int length;
	FILE *fp;

	// get next number from calibration file
	float getNumber(int &c);

	// adds the current line equation to the collection
	void addLE(LineEquation *le, float low, float high);

	// counts number of lines in calibration file
	int countLines();


	public:
	// loads calibration file and creates the approximation functions
	DistanceCalibrator();

	// adjusts the raw camera value accoriding to calibration data
	float getAdjustedDistance(float original);
};

inline void DistanceCalibrator::addLE(LineEquation *le, float low, float high) {
	calibrationRanges[length++].set(low, le);
}

inline int DistanceCalibrator::countLines() {
	int count = 0;
	int c;
	while ((c = fgetc(fp)) != EOF) {
		if (c == NEW_LINE) {
			count++;
		}
	}
	fseek(fp, 0, SEEK_SET);
	return count;
}
