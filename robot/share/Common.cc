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
 * $Id: Common.cc 6700 2005-06-18 12:44:43Z amsi929 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#include <iostream>
#include "Common.h"

using namespace std;

void getGlobalCoordinate(double myX, double myY, double myH, 
double localX, double localY, double * x, double * y)
{
    double sinA = sin(DEG2RAD(myH - 90));
    double cosA = cos(DEG2RAD(myH - 90));
    
    double dx = cosA * localX - sinA * localY;
    double dy = sinA * localX + cosA * localY;
    
    *x = dx + myX;
    *y = dy + myY;
}

void getLocalCoordinate(double myX, double myY, double myH,
double globalX, double globalY, double * x, double * y)
{
    double sinA = sin(DEG2RAD(90 - myH));
    double cosA = cos(DEG2RAD(90 - myH));    
    

    *x = cosA * (globalX - myX) - sinA * (globalY - myY);
    *y = sinA * (globalX - myX) + cosA * (globalY - myY);
}

int byte2UInt(byte b) {
	int i, s;

	s = (b >> 7) & 0x01;         // save the sign bit
	b &= 0x7f;                   // strip the sign bit
	i = (((int) b) | (s << 7));  // reassemble number

	return i;
}

double NormalizeAngle_0_360(double angle) {
	while (angle < 0) {
		angle += 360;
	}
	while (angle >= 360) {
		angle -= 360;
	}
	return angle;
}

double NormalizeAngle_180(double angle) {
	while (angle < -180) {
		angle += 360;
	}
	while (angle >= 180) {
		angle -= 360;
	}
	return angle;
}

double NormalizeAngle_PI(double angle) {
	while (angle < -M_PI) {
		angle += 2 * M_PI;
	}
	while (angle >= M_PI) {
		angle -= 2 * M_PI;
	}
	return angle;
}

double NormalizeAngle_m90_270(double angle) {
	while (angle < -QUARTER_CIRCLE) {
		angle += FULL_CIRCLE;
	}
	while (angle >= (3 * QUARTER_CIRCLE)) {
		angle -= FULL_CIRCLE;
	}
	return angle;
}

double NormalizeAngle_90_450(double angle) {
	while (angle < QUARTER_CIRCLE) {
		angle += FULL_CIRCLE;
	}
	while (angle >= (5 * QUARTER_CIRCLE)) {
		angle -= FULL_CIRCLE;
	}
	return angle;
}

// Returns true if x or y values were modified, false otherwise
bool FieldClipping(double &x, double &y) {
  double oldX, oldY;
  
  oldX = x; oldY = y;
  
  // left boundary
  if (x < -SIDE_FRINGE_WIDTH) {
    x = -SIDE_FRINGE_WIDTH;
  }
  
  // right boundary
  else if (x > FIELD_WIDTH + SIDE_FRINGE_WIDTH) {
    x = FIELD_WIDTH + SIDE_FRINGE_WIDTH;
  }
  
  // inside own goal
  if (y < -(GOAL_LENGTH / 2.0)
      && ABS(x - FIELD_WIDTH / 2.0) < GOAL_WIDTH / 2.0) {
    y = -(GOAL_LENGTH / 2.0);
  }

  // inside target goal - clip to field to make dogs shoot at goal rather than backwards
  else if (y > FIELD_LENGTH && ABS(x - FIELD_WIDTH / 2.0) < GOAL_WIDTH / 2.0) {
    y = FIELD_LENGTH;
  }

  // own goal boundary
  if (y < -GOAL_FRINGE_WIDTH && ABS(x - FIELD_WIDTH / 2.0)> GOAL_WIDTH / 2.0) {
    y = -GOAL_FRINGE_WIDTH;
  }

  // target goal boundary
  else if (y > FIELD_LENGTH + GOAL_FRINGE_WIDTH
	   && ABS(x - FIELD_WIDTH / 2.0) > GOAL_WIDTH / 2.0) {
    y = FIELD_LENGTH + GOAL_FRINGE_WIDTH;
  }
  
  return (oldX != x || oldY != y);
}

bool IsInField(double x, double y) {
  // if (x,y) didn't get clipped, it means that the robot is in the field
  return not FieldClipping(x, y);
}

bool cutVertical(double cX,
		double cY,
		double pX,
		double pY,
		double Xval,
		double *resY) {
	if (cX == pX) {
		return false;
	}
	if ((Xval - cX) * (Xval - pX) > 0) {
		return false;
	}

	*resY = (Xval - cX) * (cY - pY) / (cX - pX) + cY;
	//  cout << " Vertical " << Xval << " " <<pX<<" "<<pY<< " " << *resY << endl;
	if ((*resY - WALL_THICKNESS) * (*resY - FIELD_LENGTH + WALL_THICKNESS) > 0) {
		return false;
	}
	else {
		return true;
	}
}

bool cutHorizontal(double cX,
		double cY,
		double pX,
		double pY,
		double Yval,
		double *resX) {
	if (cY == pY) {
		return false;
	}
	if ((Yval - cY) * (Yval - pY) > 0) {
		return false;
	}

	*resX = (Yval - cY) * (cX - pX) / (cY - pY) + cX;
	//  cout << " Horizontal " << Yval << " " <<pX<<" "<<pY<< " " << *resX 
	//       << endl;
	if ((*resX - WALL_THICKNESS) * (*resX - FIELD_WIDTH + WALL_THICKNESS) > 0) {
		return false;
	}
	else {
		return true;
	}
}

void angleClipping(double cX, double cY, double *pX, double *pY) {
	double x, y;
	//  cout << "angleclipping " << cX<<" "<<cY<<" "<<*pX<<" "<<*pY<<endl;
	if (cX > WALL_THICKNESS
			&& cutVertical(cX, cY, *pX, *pY, WALL_THICKNESS, &y) == true) {
		// cout << "Angle Clipping " << (long) *pX << " "<<(long) *pY;
		*pX = WALL_THICKNESS;
		*pY = y;
		// cout <<"  " << (long) *pX << " "<<(long) *pY << endl;
		return;
	}
	if (cX < FIELD_WIDTH - WALL_THICKNESS && cutVertical(cX,
				cY,
				*pX,
				*pY,
				FIELD_WIDTH
				- WALL_THICKNESS,
				&y) == true) {
		// cout << "Angle Clipping " << (long) *pX << " "<<(long) *pY;
		*pX = FIELD_WIDTH - WALL_THICKNESS;
		*pY = y;
		// cout <<"  " << (long) *pX << " "<<(long) *pY << endl;
		return;
	}
	if (cY > WALL_THICKNESS && cutHorizontal(cX,
				cY,
				*pX,
				*pY,
				WALL_THICKNESS,
				&x) == true) {
		// cout << "Angle Clipping " << (long) *pX << " "<<(long) *pY;
		*pX = x;
		*pY = WALL_THICKNESS;
		// cout <<"  " << (long) *pX << " "<<(long) *pY << endl;
		return;
	}
	if (cY < FIELD_LENGTH - WALL_THICKNESS && cutHorizontal(cX,
				cY,
				*pX,
				*pY,
				FIELD_LENGTH
				- WALL_THICKNESS,
				&x) == true) {
		// cout << "Angle Clipping " << (long) *pX << " "<<(long) *pY;
		*pX = x;
		*pY = FIELD_LENGTH - WALL_THICKNESS;
		// cout <<"  " << (long) *pX << " "<<(long) *pY << endl;
		return;
	}
	angleClippingMore(cX, cY, pX, pY);
}

bool cutVerticalMore(double cX,
		double cY,
		double pX,
		double pY,
		double Xval,
		double *resY) {
	if (cX == pX) {
		return false;
	}
	if ((Xval - cX) * (Xval - pX) > 0) {
		return false;
	}

	*resY = (Xval - cX) * (cY - pY) / (cX - pX) + cY;
	//  cout << " Vertical " << Xval << " " <<pX<<" "<<pY<< " " << *resY << endl;
	if ((*resY - 0.0) * (*resY - FIELD_LENGTH) > 0) {
		return false;
	}
	else {
		return true;
	}
}

bool cutHorizontalMore(double cX,
		double cY,
		double pX,
		double pY,
		double Yval,
		double *resX) {
	if (cY == pY) {
		return false;
	}
	if ((Yval - cY) * (Yval - pY) > 0) {
		return false;
	}

	*resX = (Yval - cY) * (cX - pX) / (cY - pY) + cX;
	//  cout << " Horizontal " << Yval << " " <<pX<<" "<<pY<< " " 
	//       << *resX << endl;
	if ((*resX - 0.0) * (*resX - FIELD_WIDTH) > 0) {
		return false;
	}
	else {
		return true;
	}
}

void angleClippingMore(double cX, double cY, double *pX, double *pY) {
	double x, y;
	//  cout << "angleclipping " << cX<<" "<<cY<<" "<<*pX<<" "<<*pY<<endl;
	if (cX > 0.0 && cutVerticalMore(cX, cY, *pX, *pY, 0.0, &y) == true) {
		// cout << "Angle Clipping " << (long) *pX << " "<<(long) *pY;
		*pX = 0.0;
		*pY = y;
		// cout <<"  " << (long) *pX << " "<<(long) *pY << endl;
		return;
	}
	if (cX < FIELD_WIDTH
			&& cutVerticalMore(cX, cY, *pX, *pY, FIELD_WIDTH, &y) == true) {
		// cout << "Angle Clipping " << (long) *pX << " "<<(long) *pY;
		*pX = FIELD_WIDTH;
		*pY = y;
		// cout <<"  " << (long) *pX << " "<<(long) *pY << endl;
		return;
	}
	if (cY > 0.0 && cutHorizontalMore(cX, cY, *pX, *pY, 0.0, &x) == true) {
		// cout << "Angle Clipping " << (long) *pX << " "<<(long) *pY;
		*pX = x;
		*pY = 0.0;
		// cout <<"  " << (long) *pX << " "<<(long) *pY << endl;
		return;
	}
	if (cY < FIELD_LENGTH && cutHorizontalMore(cX,
				cY,
				*pX,
				*pY,
				FIELD_LENGTH,
				&x) == true) {
		// cout << "Angle Clipping " << (long) *pX << " "<<(long) *pY;
		*pX = x;
		*pY = FIELD_LENGTH;
		// cout <<"  " << (long) *pX << " "<<(long) *pY << endl;
		return;
	}
}

int ReadSignedInt(void) {
	char str[64];
	cin >> str;
	return atoi(str);
}

double getHeadingBetween(double fromX, double fromY, double toX, double toY) {
	double relx = toX - fromX;
	double rely = toY - fromY;
	if (relx == 0 && rely == 0) {
		return 0;
	}
	return atan2(rely, relx);
}

double getDistanceBetween(double fromX, double fromY, double toX, double toY) {
	double relx = toX - fromX;
	double rely = toY - fromY;
	return sqrt(SQUARE(relx) + SQUARE(rely));
}

void calPosition(double x,
		double y,
		double h,
		double d,
		double ang,
		double *pX,
		double *pY) {
	double deg, rad;

	deg = h + (double) ang;    // angle of object relative to horizontal line of the field
	while (deg >= 360) {
		deg -= 360;
	}
	while (deg < 0) {
		deg += 360;
	}

	rad = deg / 180.0 * pi;

	*pX = (x + d * cos(rad));
	*pY = (y + d * sin(rad));
	//angleClipping(x,y,pX,pY);
}

/*
 *Takes inNumPoints points as local co-ordinate pairs in inLocal and transforms
 *them into global co-ordinate pairs in outGlobal based on inX, inY and inT.
 *NOTE: outGlobal must be twice inNumPoints in size (as must inLocal) or we'll
 *have problems. This routine works even if inLocal and outGlobal are the same. 
 *note, dog heading in Radian.
 */
void localToGlobal(double dogX, double dogY, double dogHeading, int inNumPoints, double *inLocal, double *outGlobal)
{
    //First, we rotate all the points about the origin by an amount equal to
    //inT-90 where positive angle is COUNTERCLOCKWISE. For instance, if we're
    //facing left (inT = 180) we need to rotate all the points counterclockwise
    //by 90 degrees.

    double newAng = dogHeading - pi * 0.5;
    double sinT = sin(newAng);
    double cosT = cos(newAng);

    for (int i = 0; i < inNumPoints; i ++) {
        double in0 = inLocal[i*2];
        double in1 = inLocal[i*2 + 1];

        outGlobal[i*2] = (in0 * cosT - in1 * sinT) + dogX;
        outGlobal[i*2 + 1] = (in0 * sinT + in1 * cosT) + dogY;
    }
}

