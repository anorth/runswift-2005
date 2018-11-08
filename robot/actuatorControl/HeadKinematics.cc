/*

   Copyright 2004 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
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
 * $Id: HeadKinematics.cc 6721 2005-06-20 02:41:27Z nmor250 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifdef OFFLINE
#include <iostream>
using namespace std;
#endif

#include "../share/RobotDef.h"
#include "JointLimit.h"
#include "HeadKinematics.h"

#ifdef OFFLINE

#include "../vision/PointProjection.h"

int main() {
	
	double frontShoulderHeight = 90;
	double bodyTilt = DEG2RAD(10);
	
	double neckBaseHeight = frontShoulderHeight + HEAD_TILT_CENTER_TO_SHOULDER*cos(bodyTilt)*10.0 - HEAD_TILT_CENTER_TO_SHOULDER_H*sin(bodyTilt)*10.0;
	
	for (int y=0; y < 100; y+=5) {
		for (int x = -200; x< 200; x+= 5) {
			double pan, tilt, crane;
			
			cout << endl << endl;
			
			cout << "x: " << x << " y: " << y << endl;
			
			setDestVarsXYZ(x, 0, y, tilt, pan, crane, frontShoulderHeight, bodyTilt);
			
			cout << " pan: " << pan << " tilt: " << tilt << " crane: " << crane << endl;
			
			double rX, rY;
			
			bool result = getPointProjection(&rX, &rY, -pan, -tilt + RAD2DEG(bodyTilt), crane, 0, 0,
				FACE_LENGTH * 10.0, NECK_LENGTH * 10.0,
				neckBaseHeight, APERTURE_DISTANCE, CAMERA_OFFSET, true);
			
			if (result) {
				cout << "x: " << x << " y: " << y << endl;
				cout << "px: " << rX/10.0 << " py: " << rY/10.0 << endl;
			} else {
				cout << "x: " << x << " y: " << y << endl;
				cout << "failed" << endl;
			}
		}
	}
}

#endif

// x, y, z is measured in centimeter
void setDestVarsXYZ(double x, double y, double z, double &destTilt, double &destPan, double &destCrane, double frontShoulderHeight, double bodyTilt) {
    // note: calculations performed in radians

    const bool debug = false;
    static const double CAMERA_OFFSET_CM = CAMERA_OFFSET/10.0;

    if (debug)
        cout << "x: " << x << " y: " << y << " z: " << z << endl;

    // move origin to base of neck
    y -= frontShoulderHeight/10.0 + HEAD_TILT_CENTER_TO_SHOULDER*cos(bodyTilt) - HEAD_TILT_CENTER_TO_SHOULDER_H*sin(bodyTilt);

    if (debug) {
        cout << "fsh: " << frontShoulderHeight << " bt: " << RAD2DEG(bodyTilt) << endl;
        cout << "bny: " << y << endl;
    }

    double dist = sqrt(z *z + y *y);

    if (dist == 0) {
        destTilt = -80;
        destPan = (x < 0) ? -88 : 88;
        return;
    }

    double alpha = atan2(y, z);
    double beta = (dist < (NECK_LENGTH-CAMERA_OFFSET_CM)) ?
				    DEG2RAD(45) : asin((NECK_LENGTH-CAMERA_OFFSET_CM) / dist);

    if (debug)
        cout << "a: " << RAD2DEG(alpha) << " b: " << RAD2DEG(beta) << endl;

    double totalTilt = alpha - beta;

    if (debug)
        cout << " totTilt: " << RAD2DEG(totalTilt);

    double gamma = 0.35;

    double cranePart = gamma*totalTilt;	// how much we'd like crane to do

    if (cranePart > DEG2RAD(MAX_CRANE)*0.9)	// limit it to what we can expect crane to do
	    cranePart = DEG2RAD(MAX_CRANE)*0.9;
    else if (cranePart < DEG2RAD(MIN_CRANE)*0.9)
	    cranePart = DEG2RAD(MIN_CRANE)*0.9;

    // make the tilt do the rest, but trimmed to what it can do

    destTilt = MIN(totalTilt - cranePart, DEG2RAD(MAX_TILT) - bodyTilt);
    if (destTilt < DEG2RAD(MIN_TILT)-bodyTilt)
	    destTilt = DEG2RAD(MIN_TILT)-bodyTilt;

    if (debug)
	    cout << " t: " << RAD2DEG(destTilt);

    // tranlate origin to top of neck

    y -= NECK_LENGTH * cos(-destTilt);
    z -= NECK_LENGTH * sin(-destTilt);

    if (debug)
	    cout << " dy: " << (-NECK_LENGTH * cos(-destTilt)) << " dz: " << (-NECK_LENGTH * sin(-destTilt)) << endl;

    // rotate coordinates
    // make xz plane the new pan plane (which moved when we tilted by destTilt)
    double newY = y * cos(-destTilt) + z * sin(-destTilt);
    double newZ = z * cos(-destTilt) - y * sin(-destTilt);

    y = newY;
    z = newZ;

    if (debug)
	    cout << " newY: " << y << " newZ: " << newZ << endl;

    destPan = atan2(x, z);
    dist = sqrt(x * x + z * z);
    destCrane = atan2(y, dist);
    dist = sqrt(x * x + y * y + z * z);
    if (dist > CAMERA_OFFSET_CM) {
    destCrane += asin(CAMERA_OFFSET_CM/dist);
    }

    // rotate everything by the body tilt

    destTilt += bodyTilt;

    // convert back to degrees

    destTilt = RAD2DEG(destTilt);
    destPan = RAD2DEG(destPan);
    destCrane = RAD2DEG(destCrane);

    //if (debug)
    //cout << "Tilt: " << destTilt << " Crane: " << destCrane << endl;
}


// x, y, z is measured in centimeter, but clips max tilty to -10.
void setDestVarsXYZHack(double x, double y, double z, double &destTilt, double &destPan, double &destCrane, double frontShoulderHeight, double bodyTilt) {
    // note: calculations performed in radians

    const bool debug = false;
    static const double CAMERA_OFFSET_CM = CAMERA_OFFSET/10.0;

    if (debug)
        cout << "x: " << x << " y: " << y << " z: " << z << endl;

    // move origin to base of neck
    y -= frontShoulderHeight/10.0 + HEAD_TILT_CENTER_TO_SHOULDER*cos(bodyTilt) - HEAD_TILT_CENTER_TO_SHOULDER_H*sin(bodyTilt);

    if (debug) {
        cout << "fsh: " << frontShoulderHeight << " bt: " << RAD2DEG(bodyTilt) << endl;
        cout << "bny: " << y << endl;
    }

    double dist = sqrt(z *z + y *y);

    if (dist == 0) {
        destTilt = -80;
        destPan = (x < 0) ? -88 : 88;
        return;
    }

    double alpha = atan2(y, z);
    double beta = (dist < (NECK_LENGTH-CAMERA_OFFSET_CM)) ?
				    DEG2RAD(45) : asin((NECK_LENGTH-CAMERA_OFFSET_CM) / dist);

    if (debug)
        cout << "a: " << RAD2DEG(alpha) << " b: " << RAD2DEG(beta) << endl;

    double totalTilt = alpha - beta;

    if (debug)
        cout << " totTilt: " << RAD2DEG(totalTilt);

    double gamma = 0.35;

    double cranePart = gamma*totalTilt;	// how much we'd like crane to do

    if (cranePart > DEG2RAD(MAX_CRANE)*0.9)	// limit it to what we can expect crane to do
	    cranePart = DEG2RAD(MAX_CRANE)*0.9;
    else if (cranePart < DEG2RAD(MIN_CRANE)*0.9)
	    cranePart = DEG2RAD(MIN_CRANE)*0.9;

    // make the tilt do the rest, but trimmed to what it can do

    
    // HACK STARTS HERE
    destTilt = MIN(totalTilt - cranePart, DEG2RAD(-10) - bodyTilt);
    // HACK ENDS HERE
    
    if (destTilt < DEG2RAD(MIN_TILT)-bodyTilt)
	    destTilt = DEG2RAD(MIN_TILT)-bodyTilt;

    if (debug)
	    cout << " t: " << RAD2DEG(destTilt);

    // tranlate origin to top of neck

    y -= NECK_LENGTH * cos(-destTilt);
    z -= NECK_LENGTH * sin(-destTilt);

    if (debug)
	    cout << " dy: " << (-NECK_LENGTH * cos(-destTilt)) << " dz: " << (-NECK_LENGTH * sin(-destTilt)) << endl;

    // rotate coordinates
    // make xz plane the new pan plane (which moved when we tilted by destTilt)
    double newY = y * cos(-destTilt) + z * sin(-destTilt);
    double newZ = z * cos(-destTilt) - y * sin(-destTilt);

    y = newY;
    z = newZ;

    if (debug)
	    cout << " newY: " << y << " newZ: " << newZ << endl;

    destPan = atan2(x, z);
    dist = sqrt(x * x + z * z);
    destCrane = atan2(y, dist);
    dist = sqrt(x * x + y * y + z * z);
    if (dist > CAMERA_OFFSET_CM) {
    destCrane += asin(CAMERA_OFFSET_CM/dist);
    }

    // rotate everything by the body tilt

    destTilt += bodyTilt;

    // convert back to degrees

    destTilt = RAD2DEG(destTilt);
    destPan = RAD2DEG(destPan);
    destCrane = RAD2DEG(destCrane);

    //if (debug)
    //cout << "Tilt: " << destTilt << " Crane: " << destCrane << endl;
}
