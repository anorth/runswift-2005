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
 * $Id: DistanceCalibrator.cc 4662 2005-01-19 00:36:00Z shnl327 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#include <cstdio>
#include <iostream>
#include <cstring>
#include <cfloat>

#include "DistanceCalibrator.h"

using namespace std;

#ifdef OFFLINE
//const char *distCalFile = "../../robot7/cfg/distance.cal";
const char *distCalFile = "../../robot/cfg/distance.cal";
#else // OFFVISION
const char *distCalFile = "/MS/distance.cal";
#endif // OFFVISION

// assumes one line has one number
// assumes that two consecutive lines contain input (height or radius) 
// and the next line is the appropriate divisor
// assumes the lines are arranged in increasing order of the input values
DistanceCalibrator::DistanceCalibrator() {
#ifndef PRE_SANITY_BLOB
	cout << "DOING DISTANCE CALIBRATION" << endl;
#endif
	length = 0;
	fp = fopen(distCalFile, "r");
	if (fp == 0) {
		cout << "Error opening file: " << distCalFile << endl;
	}
	else {
		maxsize = countLines() - 1; 
#ifndef PRE_SANITY_BLOB
		cout << "*********************************** maxsize=" << maxsize << endl;
#endif
		calibrationRanges = new LineRangeElt[maxsize];
		char buffer[BUFFER_SIZE];
		if (maxsize > 0)
			fgets(buffer, BUFFER_SIZE, fp);
		if (strcmp(buffer, "Distance Calibration File") == 0)
			cout << "File is Good" << endl;
		int c;
		float height = 0;
		float divisor = 0;
		float next_height = getNumber(c);
		float next_divisor = getNumber(c);
#ifndef PRE_SANITY_BLOB
		cout << DC_MINIMUM_LIMIT << " " << next_divisor << " " << next_height
			<< " " << next_divisor << endl;
#endif
		LineEquation *le = new LineEquation(0,
				next_divisor,
				next_height,
				next_divisor);
		addLE(le, DC_MINIMUM_LIMIT, next_height); // extrapolate the first 
		// conversion function
		// use divisorration data for points experimented on.
		while (c != EOF) {
			height = next_height;
			divisor = next_divisor;
			next_height = getNumber(c);
			next_divisor = getNumber(c);
			if (c == EOF)
				break;
#ifndef PRE_SANITY_BLOB
			cout << height << " " << divisor << " " << next_height << " "
				<< next_divisor << endl;
#endif
			le = new LineEquation(height, divisor, next_height, next_divisor);
			addLE(le, height, next_height);
		}
#ifndef PRE_SANITY_BLOB
		cout << height << " " << next_divisor << " " << DC_MAXIMUM_LIMIT << " "
			<< next_divisor << endl;
#endif
		// creates equation which makes no adjustment for the
		le = new LineEquation(0, divisor, height, divisor);

		// last conversion function uses a constant value from 
		// the last point in file    
		addLE(le, height, DC_MAXIMUM_LIMIT);
		// fclose (fp);
#ifndef PRE_SANITY_BLOB
		for (int i = 1; i < length; i++) {
			cout
				<< endl
				<< calibrationRanges[i].bottom
				<< " "
				<< calibrationRanges[i - 1].le->getYVal(calibrationRanges[i].bottom);
		}
#endif
	}
}

float DistanceCalibrator::getNumber(int &c) {
	double exponent = 0, component = 0;
	float num = 0;
	bool decimal = false;
	bool minus = false;

	while ((c = fgetc(fp)) != EOF) {
		if (c == SPACE || c == NEW_LINE)
			if (component == 0 && !decimal)
				continue;
			else
				break;
		if (c == MINUS)
			minus = true;
		if (isdigit(c)) {
			component = component * 10 + (c - '0');
			exponent++;
		}
		if (c == DOT) {
			num = component;
			decimal = true;
			exponent = component = 0;
		}
	}
	if (decimal) {
		num += ((float) component) / (float) pow(10.0, exponent);
	}
	else {
		num = component;
	}
	if (minus) {
		num = -num;
	}
	return num;
}

// uses binary search
float DistanceCalibrator::getAdjustedDistance(float height) {
	int low = 0, high = length - 1;
	int mid;
	for (mid = high / 2; mid + 1 < length; mid = low + (high - low) / 2) {
		if (height > calibrationRanges[mid].bottom) {
			if (height <= calibrationRanges[mid + 1].bottom) {
				return calibrationRanges[mid].le->getYVal(height);
			}
			else {
				low = mid + 1;
			}
			/*
			   } else if (height < calibrationRanges[mid]->bottom)
			   if (mid<1)
			   break;
			   if (orignal >= calibrationRanges[mid-1]->bottom)
			   return calibrationRanges[mid-1]->le->getYVal(height);
			   high = mid-2; // must check that mid-2>=0
			   */
	}
	else {
		high = mid - 1;
	}
}
return calibrationRanges[mid].le->getYVal(height);
}
