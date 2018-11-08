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
 * $Id$
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



/*
   This file contains legacy code ! Code that is not currently used but reluctant to delete, might be kept as a historical stuff :)

Note: If you want to use these code, dont include this file, paste it into proper place
*/

//old -> #define APERTURE_DISTANCE 161.714647712  
#define APERTURE_DISTANCE 195.7237945 //Set to HEIGHT/(2*tan(yField/2));//64.829091184;/

// Old project point transformation, lack crane angle, didn't model NECK_HEIGHT acurately 
// See Raymond's thesis, p17 for details
//TODO: move to referenceCode.h
void VisualCortex::oldProjectPointsDAlt(int *inPoints,
		int numPoints,
		double *outPoints,
		double projectHeight) {
	//EVIL here , hs & b must be taken from Behaviour or actuatorControl (hf,hb)
	double hs = 7;
	double b = 11;
	double s = 13;
	//END OF EVIL
	cout << "Projecting " << endl;
	cout << " pan = " << pan << endl;
	cout << " tilt = " << tilt << endl;
	cout << " roll = " << roll << endl;

	double h = NECKBASE_HEIGHT;
	double T = tilt ; //(tilt - TILT_OFFSET + evilTiltOffset);
	double P = -pan;
	double c = FACE_LENGTH;
	double f = (double) APERTURE_DISTANCE;
	double n = NECK_LENGTH;
	double p = (double) projectHeight;
	for (int i = 0; i < numPoints; i ++) {
		double A, R, xc, yc, zc, xi, yi, zi, k, x, y, u, w;

		w = (double) (inPoints[i * 2 + 0]);
		u = (double) (inPoints[i * 2 + 1]);
		// Effective tilt = body tilt + head tilt
		A = asin((b - hs) / s);
		R = A + T;

		// Coordiates of camera point(c)
		xc = c * sin(P);
		yc = n * sin(R) + c * cos(R) * cos(P);
		zc = h - p - c * sin(R) * cos(P) + n * cos(R);

		// Coordinates of image point (i) to be projected
		xi = w * cos(P) + (c + f) * sin(P);
		yi = -w * sin(P) * cos(R) + (c + f) * cos(R) * cos(P) + (n + u) * sin(R);
		zi = h
			- p
			+ w * sin(P) * sin(R)
			- (c + f) * sin(R) * cos(P)
			+ (n + u) * cos(R);
		//Raymond: Added: Check to make sure that zi-zc is greater than 0 (actually check 
		//against 1E-5 just to make sure the FPU doesn't stuff up). If this is the 
		//case then the points exist over the horizon so set the outpoints at 
		//that point to equal infinity. 
		if (zi - zc > -(1E-5)) {
			outPoints[i * 2 + 0] = LARGE_VAL;
			outPoints[i * 2 + 1] = LARGE_VAL;
		}
		else {
			// Ground plane z=0 intersection of line going through points c and i
			k = zi / (zi - zc);
			x = xi + k * (xc - xi);
			y = yi + k * (yc - yi);


			outPoints[i * 2 + 0] = x;
			outPoints[i * 2 + 1] = y;
		}
	}
}


// Raymond's project point transformation
void VisualCortex::projectPointsD(int *inPoints,
		int numPoints,
		double *outPoints,
		double projectHeight) {
	//{{{
	//Find co-ordinates of the end points. 
	double **endVectors, *origin, *newOrigin, **newEndVectors;
	double *fieldPoints;
	//Matrices.
	//single letters are individual transform matrix elements. 
	double *xfrmMatrix, a, b, c, d, e, f, g, h, ii, jj, kk, u;// *camToPan, *removePan, *panToTilt, *removeTilt, *tiltToGround;
	int i, j, k;
	endVectors = (double * *) malloc(numPoints * sizeof(double *));
	newEndVectors = (double * *) malloc(numPoints * sizeof(double *));
	fieldPoints = (double *) malloc(numPoints * 2 * sizeof(double));  //array of numpoints times numbers per point (2)
	for (i = 0; i < numPoints; i ++) {
		//{{{
		endVectors[i] = (double *) malloc(4 * sizeof(double));  //4 for homogenous vectors
		//Allocate the last dimension of newEndVectors later 
		//(when applying the matrix)
	}//}}}
	origin = (double *) malloc(4 * sizeof(double));  //location of the origin. 
	//  newOrigin = (double*)malloc(4*sizeof(double));  //I don't think we need this as the matrix apply function should allocate this for us later. 
	for (i = 0; i < numPoints; i ++) {
		//{{{
		endVectors[i][0] = inPoints[i * 2 + 0];
		endVectors[i][1] = inPoints[i * 2 + 1];
		endVectors[i][2] = -(int) APERTURE_DISTANCE;  
		endVectors[i][3] = (int) 0;
	}//}}}


	//We now have the endpoints of the Hough lines in endVectors[i*4+k]. Form 
	//rays that go through the camera aperture. 
	origin[0] = 0;
	origin[1] = 0;
	origin[2] = 0;
	origin[3] = 1;

	//Initialize transformation matrices.
	a = -FACE_LENGTH;
	b = cosRAD(pan);
	c = sinRAD(pan);
	d = -c;
	e = b;
	f = NECK_LENGTH;
	g = cosRAD((-(tilt + evilTiltOffset)));
	h = -sinRAD((-(tilt + evilTiltOffset)));
	ii = -h;
	jj = g;
	kk = NECKBASE_HEIGHT - projectHeight;
	xfrmMatrix = createMatrix4x4(b,
			0,
			c,
			c * a,
			d * h,
			g,
			h * e,
			a * h * e + f * g + kk,
			d * jj,
			ii,
			e * jj,
			a * e * jj + ii * f,
			0,
			0,
			0,
			1);
	newOrigin = applyMatrix(xfrmMatrix, origin);
	for (i = 0; i < numPoints; i ++) {
		//{{{
		if (inPoints[i * 2 + 0] != -1 || projectHeight != 0) {
			//{{{
			newEndVectors[i] = applyMatrix(xfrmMatrix, endVectors[i]);
		}//}}}
	}//}}}
	//Now we have the rays through the lens which start at the point newOrigin and 
	//extend in the direction of each newEndVectors element by an amount u.
	//Now intersect it with the plane y=0. 
	for (i = 0; i < numPoints; i ++) {
		//{{{
		//NOTE: These vectors are STILL in the co-ordinate system centred about the spot 
		//on the ground under the robot's neck where X is to the right, Y is straight up 
		//and Z is pointing towards the BACK of the robot, where we have [X][Y][Z].
		if (inPoints[i * 2 + 0] != -1 || projectHeight != 0) {
			//{{{
			u = newOrigin[1] / (newOrigin[1] - newEndVectors[i][1]);
			//EVIL HACK:
			if (u < 0) {
				//{{{
				//  u = -u;
			}//}}}
			fieldPoints[i * 2 + 0] = (newOrigin[0]
					+ u * (newEndVectors[i][0] - newOrigin[0]));
			fieldPoints[i * 2 + 1] =
				-(newOrigin[2] + u * (newEndVectors[i][2] - newOrigin[2]));
		}//}}}
		else {
			//{{{
			//Line was deleted ...
			fieldPoints[i * 2 + 0] = -1;
			fieldPoints[i * 2 + 1] = -1;
		}//}}}

	}//}}}
	//For now, we just copy this straight to the out array (for debugging!). 
	memcpy(outPoints, fieldPoints, numPoints * 2 * sizeof(double));
	free(newOrigin);
	free(origin);
	free(fieldPoints);
	for (i = 0; i < numPoints; i ++) {
		//{{{
		free(endVectors[i]);
		if (inPoints[i * 2 + 0] != -1 || projectHeight != 0) {
			//Remember, we didn't allocate this if inPoints was -1.
			free(newEndVectors[i]);
		}
	}//}}}
	free(endVectors);
	free(newEndVectors);
}//}}}


// this was the method andres developed in 2002, it was intended for locating 
// close balls accurately. It works for moderately close balls, however, it does 
// not work well for balls that are very close.
// 
bool VisualCortex::andresFindBall(HalfBeacon *preball, VisualObject &ball) {
	double xmin = preball->getXMin();
	double xmax = preball->getXMax();
	double ymin = preball->getYMin();
	double ymax = preball->getYMax();
	double raw_y = ymin;
	Point p[3];
	int i;
	for (i = 0; i < 3; i++) {
		if (!preball->getNextPoint(p[i])) {
			break;
		}
	}
	// If we didn't obtain enough points, then there are not enough points to form a circle
	if (i != 3) {
#ifdef OFFLINE
#ifndef PRE_SANITY_BLOB
		(*outfile) << "3pt circle crap " << endl;
#endif
#endif
		fireSanity(NEED_3_POINTS_TO_FORM_BALL);
		return false;
	}

	double centreX = 0, centreY = 0, radius = 0, expected = 1;

	int sqr_y[3];
	sqr_y[0] = SQUARE(p[0].y);
	sqr_y[1] = SQUARE(p[1].y);
	sqr_y[2] = SQUARE(p[2].y);
	int sqr_x[3];
	sqr_x[0] = SQUARE(p[0].x);
	sqr_x[1] = SQUARE(p[1].x);
	sqr_x[2] = SQUARE(p[2].x);

	double a = determinant(p[0].x,
			p[0].y,
			1,
			p[1].x,
			p[1].y,
			1,
			p[2].x,
			p[2].y,
			1);

	double d = -determinant(sqr_x[0] + sqr_y[0],
			p[0].y,
			1,
			sqr_x[1] + sqr_y[1],
			p[1].y,
			1,
			sqr_x[2] + sqr_y[2],
			p[2].y,
			1);

	double e = determinant(sqr_x[0] + sqr_y[0],
			p[0].x,
			1,
			sqr_x[1] + sqr_y[1],
			p[1].x,
			1,
			sqr_x[2] + sqr_y[2],
			p[2].x,
			1);

	double f = -determinant(sqr_x[0] + sqr_y[0],
			p[0].x,
			p[0].y,
			sqr_x[1] + sqr_y[1],
			p[1].x,
			p[1].y,
			sqr_x[2] + sqr_y[2],
			p[2].x,
			p[2].y);

	// a= 0 if the points are on a straight line
	if (a == 0) {
#ifdef OFFLINE
#ifndef PRE_SANITY_BLOB
		(*outfile) << "points in straight line" << endl;
#endif
#endif
		return false;
	}

	centreX = -d / (2 * a);
	centreY = -e / (2 * a);
	radius = sqrt((SQUARE(d) + SQUARE(e)) / (4 * SQUARE(a)) - f / a);
	if (!radius) {
		// andres, probably don't need this check. I hypothesize that a=0 => radius=0
		return false;
	}

	ball.blob = preball;
	ball.area = preball->area;        //M_PI* SQUARE(radius)
	ball.radius = radius;
	ball.cx = centreX;
	ball.cy = centreY;
	ball.width = radius * 2;
	ball.height = radius * 2;
	ball.x = centreX - radius;
	ball.y = centreY - radius;

	xmin = ball.x;
	xmax = ball.x + ball.width;
	ymin = ball.y;
	ymax = ball.y + ball.height;

	// calculating confidence
	expected = MIN(pi * SQUARE(ball.radius), HEIGHT * WIDTH);        //25/1/02 used to be width/2
	ball.cf = (int) MIN(ball.area * 1000.0 / expected, 1000);

	// apply clippings
	int pxmin = (int) MAX(xmin, 0);
	int pxmax = (int) MIN(xmax, WIDTH);
	int pymin = (int) MAX(ymin, 0);
	int pymax = (int) MIN(ymax, HEIGHT);

	ball.projected_x = pxmin;
	ball.projected_y = pymin;
	ball.projected_w = pxmax - pxmin;
	ball.projected_h = pymax - pymin;

	ball.tilt = 0;
	ball.imgHead = PointToHeading(centreX);
	ball.imgElev = PointToElevation(centreY);
	ball.misc = raw_y;

#ifdef HIGH
	double divisor;
	if (cmdAdjustDistance) {
		divisor = ballCalibrator->getAdjustedDistance(ball.radius);
	}
	else {
		divisor = 600.0;
	}

	ball.cam_dist = 600.0 / ball.radius;
	ball.dist = divisor / ball.radius;
	ball.var = exp(-0.864452 + 0.0215735 * ball.dist) / pow(ball.area
			/ expected,
			errorScale);
#endif
#ifdef MED
	ball.dist = SizeToDistance(BALL_RADIUS, ball.radius);
#endif
	ball.cam_centroid_dist = ball.dist;
	RobotRelative(ball);
	return true;
}

/**
 * void VisualCortex::projectPoints(int * inPoints, int numPoints, int * outPoints)
 * 
 * Takes the input line endpoints, projects them 
 * through the 'lens' of the camera and, based on the head pan and tilt, finds the 
 * heading and direction of the equivalent lines on the field based on co-ordinates 
 * starting at the base of the neck. 
 * NOTE: ZERO IS IN THE MIDDLE OF THE IMAGE AND INCREASES UPWARDS (standard co-ords). 
 * You therefore need to convert from the co-ordinate system used elsewhere in this file
 * (zero is in the upper left corner and increases downwards). 
 * //{{{
 * Uses the following global vars and defines:
 * pan                Pan of head in radians
 * tilt                Tilt of head in radians
 * FACE_LENGTH        Horizontal distance from camera to neck pan point
 * NECK_LENGTH        Vertical distance from camera to neck tilt point
 * NECKBASE_HEIGHT    Vertical distance from neck tilt point to ground (based on stance)
 *
 * Note that internal multidimensional arrays are implemented as pointer-pointer arrays.
 * External multidimensional arrays are implemented as single pointer arrays.
 * //}}}
 */
//TODO : delete
void VisualCortex::projectPoints(int *inPoints,
		int numPoints,
		int *outPoints,
		int projectHeight) {
	//{{{
	if (altProject) {
		projectPointsDAlt(inPoints, numPoints, outPoints, projectHeight);
		return;
	}
	//Find co-ordinates of the end points. 
	double **endVectors, *origin, *newOrigin, **newEndVectors;
	int *fieldPoints;
	//Matrices.
	//single letters are individual transform matrix elements. 
	double *xfrmMatrix, a, b, c, d, e, f, g, h, ii, jj, kk, u;// *camToPan, *removePan, *panToTilt, *removeTilt, *tiltToGround;
	int i, j, k;
	endVectors = (double * *) malloc(numPoints * sizeof(double *));
	newEndVectors = (double * *) malloc(numPoints * sizeof(double *));
	fieldPoints = (int *) malloc(numPoints * 2 * sizeof(int));  //array of numpoints times numbers per point (2)
	for (i = 0; i < numPoints; i ++) {
		//{{{
		endVectors[i] = (double *) malloc(4 * sizeof(double));  //4 for homogenous vectors
		//Allocate the last dimension of newEndVectors later 
		//(when applying the matrix)
	}//}}}
	origin = (double *) malloc(4 * sizeof(double));  //location of the origin. 
	for (i = 0; i < numPoints; i ++) {
		//{{{
		endVectors[i][0] = inPoints[i * 2 + 0];
		endVectors[i][1] = inPoints[i * 2 + 1];
		endVectors[i][2] = -(int) APERTURE_DISTANCE;  
		endVectors[i][3] = (int) 0;
	}//}}}


	//We now have the endpoints of the Hough lines in endVectors[i*4+k]. Form 
	//rays that go through the camera aperture. 
	origin[0] = 0;
	origin[1] = 0;
	origin[2] = 0;
	origin[3] = 1;

	//Initialize transformation matrices.
	a = -FACE_LENGTH;
	b = cosRAD(pan);
	c = sinRAD(pan);
	d = -c;
	e = b;
	f = NECK_LENGTH;
	g = cosRAD((-(tilt + evilTiltOffset)));
	h = -sinRAD((-(tilt + evilTiltOffset)));
	ii = -h;
	jj = g;
	kk = NECKBASE_HEIGHT - projectHeight;
	xfrmMatrix = createMatrix4x4(b,
			0,
			c,
			c * a,
			d * h,
			g,
			h * e,
			a * h * e + f * g + kk,
			d * jj,
			ii,
			e * jj,
			a * e * jj + ii * f,
			0,
			0,
			0,
			1);
	newOrigin = applyMatrix(xfrmMatrix, origin);
	for (i = 0; i < numPoints; i ++) {
		//{{{
#ifdef PROJECT_SENTINEL
		if (inPoints[i * 2 + 0] != -1)
#endif //PROJECT_SENTINEL
		{
			//{{{
			newEndVectors[i] = applyMatrix(xfrmMatrix, endVectors[i]);
		}//}}}
	}//}}}
	//Now we have the rays through the lens which start at the point newOrigin and 
	//extend in the direction of each newEndVectors element by an amount u.
	//Now intersect it with the plane y=0. 
	for (i = 0; i < numPoints; i ++) {
		//{{{
		//NOTE: These vectors are STILL in the co-ordinate system centred about the spot 
		//on the ground under the robot's neck where X is to the right, Y is straight up 
		//and Z is pointing towards the BACK of the robot, where we have [X][Y][Z].
#ifdef PROJECT_SENTINEL      
		if (inPoints[i * 2 + 0] != -1)
#endif //PROJECT_SENTINEL
		{
			//{{{
			u = newOrigin[1] / (newOrigin[1] - newEndVectors[i][1]);
			//EVIL HACK:
			if (u < 0) {
				//{{{
				//  u = -u;
			}//}}}
			fieldPoints[i * 2 + 0] = (int)
				(newOrigin[0] + u * (newEndVectors[i][0] - newOrigin[0]));
			fieldPoints[i * 2 + 1] =
				-(int) (newOrigin[2] + u * (newEndVectors[i][2] - newOrigin[2]));

			//WARNING: EVIL HACK!
			//
			//We're having a problem where points that are at an angle to the robot 
			//(left or right) seem to have a depression value that is too high (and 
			//the error is proportional to the angle). This can be seen where points 
			//that are really in a line, after passing through all this transform, look 
			//like they lie on an arc with the centre somewhere around the robot's tail. 
			//This is invariant on head tilt or pan (you take a given point and move the 
			//head around whilst keeping that point in view somehow and it doesn't get 
			//much better or worse except for random noise). 
			//
			//To fix it for now, we're going to artificially increase the u value 
			//proportional to the angle the vector to that point forms from 
			//dead centre. This will cause the next process to extend the line more 
			//and hopefully correct for this problem. 
			//
			//Measuring roughly, it appears that when points are dead centre, distances
			//are right. When points are at 90 degrees either side, distances are 
			//about 4/5 what they should be. 
			/*
			   double correction;
			//correction = (double)fieldEndPoints[i*4+j*2+0]/(double)fieldEndPoints[i*4+j*2+1];//was sine(atan()) of this
			if (abs(fieldPoints[i*2+0]) < abs(fieldPoints[i*2+1]))
			{//{{{
			correction = sin(atan((double)fieldPoints[i*2+0]/(double)fieldPoints[i*2+1]));
			}//}}}
			else
			{//{{{
			correction = sin(3.14159/2.0 - atan((double)fieldPoints[i*2+1]/(double)fieldPoints[i*2+0]));
			}//}}}
			if (correction < 0)
			{//{{{
			correction = -correction;
			}//}}}
			//cout << "correction for line " << i << " end " << j << " = " << correction << endl;
			fieldPoints[i*2+0] = (int)((double)fieldPoints[i*2+0]*(1.0+evilHackFactor1*correction));
			fieldPoints[i*2+1] = (int)((double)fieldPoints[i*2+1]*(1.0+evilHackFactor1*correction));
			*/
			//EVIL HACK NUMBER 2:
			//
			//After the previous hack was implemented, it was discovered that there was also
			//a component of displacement forward that seems dependant on head pan (the angle of a 
			//straight line is now correct but as the head pans to the left or right, it moves 
			//closer to the robot). The following fudge is to correct for this (only in axis 
			//1 which points out of the robot). 
			/*
			   correction = sinRAD(pan);
			   if (correction < 0)
			   {//{{{
			   correction = -correction;
			   }//}}}
			   fieldPoints[i*2+1] = (int)((double)fieldPoints[i*2+1]*(1.0+evilHackFactor2*correction));
			//Now apply evil distance hack.
			fieldPoints[i*2+1] = (int)((double)fieldPoints[i*2+1]*evilDistanceHack);
			*/
		}//}}}
#ifdef PROJECT_SENTINEL
		else {
			//{{{
			//Line was deleted ...
			fieldPoints[i * 2 + 0] = -1;
			fieldPoints[i * 2 + 1] = -1;
		}//}}}
#endif //PROJECT_SENTINEL

	}//}}}
	//For now, we just copy this straight to the out array (for debugging!). 
	memcpy(outPoints, fieldPoints, numPoints * 2 * sizeof(int));
	free(newOrigin);
	free(origin);
	free(fieldPoints);
	for (i = 0; i < numPoints; i ++) {
		//{{{
		free(endVectors[i]);
#ifdef PROJECT_SENTINEL
		if (inPoints[i * 2 + 0] != -1)
#endif //PROJECT_SENTINEL
		{
			//Remember, we didn't allocate this if inPoints was -1.
			free(newEndVectors[i]);
		}
	}//}}}
	free(endVectors);
	free(newEndVectors);
}//}}}

void VisualCortex::projectPointsAlt(int *inPoints,
		int numPoints,
		int *outPoints,
		int projectHeight) {
	//{{{
	//Project points as per Bernhard's algorithm. 
	double hs = 7;
	double h = NECKBASE_HEIGHT;
	double b = 11;
	double s = 11.9;
	double T = (tilt - TILT_OFFSET + evilTiltOffset);
	double P = -pan;
	double c = FACE_LENGTH;
	double f = (double) APERTURE_DISTANCE;
	double n = NECK_LENGTH;
	double p = (double) projectHeight;
	for (int i = 0; i < numPoints; i ++) {
		double A, R, xc, yc, zc, xi, yi, zi, k, x, y, u, w;

		w = (double) (inPoints[i * 2 + 0]);
		u = (double) (inPoints[i * 2 + 1]);
		// Effective tilt = body tilt + head tilt
		A = asin((b - hs) / s);
		R = A + T;

		// Coordiates of camera point(c)
		xc = c * sin(P);
		yc = n * sin(R) + c * cos(R) * cos(P);
		zc = h - p - c * sin(R) * cos(P) + n * cos(R);

		// Coordinates of image point (i) to be projected
		xi = w * cos(P) + (c + f) * sin(P);
		yi = -w * sin(P) * cos(R) + (c + f) * cos(R) * cos(P) + (n + u) * sin(R);
		zi = h
			- p
			+ w * sin(P) * sin(R)
			- (c + f) * sin(R) * cos(P)
			+ (n + u) * cos(R);

		//Raymond: Added: Check to make sure that zi-zc is greater than 0 (actually check 
		//against 1E-5 just to make sure the FPU doesn't stuff up). If this is the 
		//case then the points exist over the horizon so set the outpoints at 
		//that point to equal infinity. 
		if (zi - zc > -(1E-5)) {
			outPoints[i * 2 + 0] = (int) LARGE_VAL;
			outPoints[i * 2 + 1] = (int) LARGE_VAL;
		}
		else {
			// Ground plane z=0 intersection of line going through points c and i
			k = zi / (zi - zc);
			x = xi + k * (xc - xi);
			y = yi + k * (yc - yi);


			/*    
			// Effective tilt = body tilt + head tilt;
			A = asin((b-h)/s);
			R = A+T;
			// Coordiates of camera point(c);
			xc = c*sin(P);
			yc = n*sin(R)+c*cos(R)*cos(P);
			zc = h-p-c*sin(R)*cos(P)+n*cos(R);
			// Coordinates of image point (i) to be projected;
			xi = w*cos(P)+(c+f)*sin(P);
			yi = -w*sin(P)*cos(R)+(c+f)*cos(R)*cos(P)+(n+u)*sin(R);
			zi = h-p+w*sin(P)*sin(R)-(c+f)*sin(R)*cos(P)+(n+u)*cos(R);
			// Ground plane z=0 intersection of line going through points c and i;
			k = zi/(zi-zc);
			x = xi+k*(xc-xi);
			y = yi+k*(yc-yi);
			*/
			outPoints[i * 2 + 0] = (int) x;
			outPoints[i * 2 + 1] = (int) y;
		}
	}
}//}}}


