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
 * $Id: ballTracking.cc 2137 2003-12-16 12:44:41Z kcph007 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

/*
 * Strategy:
 *  saBallTracking
 *
 * Description:
 *  tracks ball by setting head pan and tilt
 *
 * Strategy member variables used:
 *  lastBallTilt -
 *      this gets set upon ball tracking
 *
 * Local variables used:
 *  none
 *
 * Restrictions:
 *  none
 *
 * Comments:
 *  because CurObject in UNSWSkills might be removed, strategy
 *  now has to access vobs directly from VisualCortex in order
 *  to get vBall.imgHead and vBall.imgElev; it is now accessed 
 *  thus: vob[vobBall].imgHead
 */

#include "Behaviours.h"
#define FOV 10

// HACK
static double old_tilt = 0;
static double old_pan = 0;

void Behaviours::saBallTracking(bool trackWM) {
	VisualObject &ball = vision->vob[vobBall];
	/*
	   cout << "saBallTracking" << endl;
	   cout << "vision->vob[vobBall].cf = " << vision->vob[vobBall].cf << endl;
	   cout << "vision->vob[vobBall].head = " << vision->vob[vobBall].head << endl;
	   cout << "vision->vob[vobBall].tilt = " << vision->vob[vobBall].tilt << endl;
	   cout << "cx, cy, radius = " << ball.cx << " " << ball.cy << " " << ball.radius << endl;
	   cout << "vBall values" << endl;
	   cout << "vBall.d = " << vBall.d << endl;
	   cout << "vBall.h = " << vBall.h << endl;
	   cout << "vBall.x = " << vBall.x << endl;
	   cout << "vBall.y = " << vBall.y << endl;
	   cout << "vBall.f = " << vBall.f << endl;
	   cout << "vBall.imgElev = " << vBall.imgElev << endl;
	   cout << "vBall.imgHead = " << vBall.imgHead << endl;
	   */


	if (vision->vob[vobBall].cf > 0) {
		lastTrackHead = vision->vob[vobBall].head;
		panx = vision->vob[vobBall].head;
		//cout << "ball d:" << ball.d << " dist:" << ball.dist << " camdist:" << ball.cam_dist << " h:" << ball.h << endl;


		if (abs(panx - hPan) > FOV) {
			double factor = 1.0 + ABS(panx - hPan) / 60;
			panx = hPan + (panx - hPan) * factor;
		}

		double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, z6;

		x1 = vBall.d * tan(radians(vBall.imgHead));
		//y1 = vBall.d * tan(radians(vBall.imgElev));
		y1 = vBall.d * tan(radians(PointToElevation(vision->vob[0].misc)));
		z1 = vBall.d;

		x2 = x1;
		y2 = y1 + NECK_LENGTH;
		z2 = z1 + FACE_LENGTH;

		x3 = x2 * cos(radians(-hPan)) - z2 * sin(radians(-hPan));
		y3 = y2;
		z3 = x2 * sin(radians(-hPan)) + z2 * cos(radians(-hPan));

		x4 = x3;
		y4 = z3 * sin(radians(hTilt)) + y3 * cos(radians(hTilt));
		z4 = z3 * cos(radians(hTilt)) - y3 * sin(radians(hTilt));

		double turn = radians(turnCCW / 8);
		x5 = x4 * cos(turn) - z4 * sin(turn);
		y5 = y4;
		z5 = x4 * sin(turn) + z4 * cos(turn);

		if (z5 < NECK_LENGTH)
			z5 = NECK_LENGTH;

		headtype = ABS_H;
		double dist = sqrt(z5 *z5 + y5 *y5);
		tilty = degrees(atan(y5 / z5) - asin(NECK_LENGTH / dist));
		old_tilt = tilty;
		old_pan = panx;
	}
	else {
		tilty = old_tilt;
		panx = old_pan;
	}
	/*
	   cout << "old_tilt = " << old_tilt << endl;
	   cout << "old_pan = " << old_pan << endl;
	   cout << "new_tilt = " << tilty << endl;
	   cout << "new_pan = " << panx << endl;
	   */
	/*
	   cout << "*******************" << endl;
	   cout << "current pan  = " << hPan << endl;
	   cout << "current tilt = " << hTilt << endl;    
	   cout << "target pan   = " << pan << endl;
	   cout << "target tilt  = " << tilt << endl;
	   cout << "*******************" << endl;
	   */    
	/*
	   double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, z5;
	   x1 = gps->ball().pos.d * tan (radians (vision->vob[vobBall].imgHead));
	   y1 = gps->ball().pos.d * tan (radians (PointToElevation (vision->vob[vobBall].misc)));
	// if middle of ball is higher than top of ball (due to a 
	// misbound), track top of ball
	if (vision->vob[vobBall].imgElev > PointToElevation (vision->vob[vobBall].misc)) {
	y1 = gps->ball().pos.d * tan (radians (PointToElevation(vision->vob[vobBall].misc)));
	}
	z1 = gps->ball().pos.d;
	x2 = x1;
	y2 = y1 + NECK_LENGTH;
	z2 = z1 + FACE_LENGTH;
	x3 = x2 * cos (radians (-hPan)) - z2 * sin (radians (-hPan));
	y3 = y2;
	z3 = x2 * sin (radians (-hPan)) + z2 * cos (radians (-hPan));
	x4 = x3;
	y4 = z3 * sin (radians (hTilt)) + y3 * cos (radians (hTilt));
	z4 = z3 * cos (radians (hTilt)) - y3 * sin (radians (hTilt));
	if (z4 < NECK_LENGTH)
	z4 = NECK_LENGTH;
	tilt = degrees (atan2 (y4, z4) - asin (NECK_LENGTH / sqrt (z4 * z4 + y4 * y4)));
	x5 = x4;
	z5 = z4 * cos (radians (-tilt)) - y4 * sin (radians (-tilt));
	pan = degrees (atan2 (x5, z5));
	headtype = ABS_H;
	if (gps->ball().posVar < get95CF(50))
	lastBallTilt = tilt;
	*/
}
