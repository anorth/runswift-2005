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
 * $Id: PointProjection.cc 4362 2004-09-22 06:54:12Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/




/*
 * This is a testing for getPointProjection function.
 * First written by Kim, modified by Daniel.
    
    g++ -o PointProjection PointProjection.cc -Wall -isystem/usr/local/OPEN_R_SDK/OPEN_R/include
   
   to compile.
 */
#include <stdio.h>
#include "PointProjection.h"
#include "../share/RobotDef.h"

int main(int argc, char **argv) {

    using namespace::std;

	if (argc != 7) {
		cout << "Usage: a.out Panx NeckTilt NeckTilt2 ipx ipy height" << endl;
		exit(1);
	}

	double p = atof(argv[1]);
	double t = atof(argv[2]);
	double t2 = atof(argv[3]);
	double ipx = atof(argv[4]);
	double ipy = atof(argv[5]);
	double height = atof(argv[6]);
	double head = 0, elev = 0;

	double shoulderHeight   = 90;
	double hipHeight        = 130;
	double bodyTilt         = RAD2DEG(asin((hipHeight - shoulderHeight) / BODY_LENGTH));

	double NECK_BASE_HEIGHT = getNeckBaseHeightFromStance(shoulderHeight,
			hipHeight) ;

    cout << "Note - All units in mm's. " << endl;

    cout << "Point elevation " << getElevation( p, t, t2, ipx, ipy, 10 * FACE_LENGTH, 10 * NECK_LENGTH, 199.188 , CAMERA_OFFSET) << endl;
    cout << "Height " << getHeightKnowingDistance( p, t, t2, ipx, ipy, 0, FACE_LENGTH * 10 , NECK_LENGTH * 10, APERTURE_DISTANCE, CAMERA_OFFSET) << endl;
    /*

    getHeadElev(&head,
				&elev,
				p,
				t   + bodyTilt,
				t2,
				ipx,
				ipy,

                height, // Assuem height of the point.

				76.9,
				80.0,
				
                // NECK_BASE_HEIGHT - BALL_RADIUS * 10.0,
                0, // Calculate with ref to base of neck now. 
                
				APERTURE_DISTANCE,
				CAMERA_OFFSET,
                true);

	cout << " head = " << head << endl;
	cout << " elev = " << elev << endl;
    */
    cout << "Point Projection:" << endl;
    if (getPointProjection(&x,
			    &y,
			    p,
			    t,
			    t2,
			    ipx,
			    ipy,
			    76.9,
			    80.0,
			    NECK_BASE_HEIGHT - BALL_RADIUS * 10.0,
			    APERTURE_DISTANCE,
			    CAMERA_OFFSET,
			    true))
	    //76.9, 80.0, 0, APERTURE_DISTANCE, CAMERA_OFFSET))
    {
	    cout << "Sucessful" << endl;
    }
    else {
	    cout << "Unsucessful" << endl;
    }
    cout << " x = " << x << endl;
    cout << " y = " << y << endl;


    return 0;
}

