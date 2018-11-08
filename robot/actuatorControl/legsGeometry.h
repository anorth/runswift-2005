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
 * $Id: legsGeometry.h 6774 2005-06-21 07:29:02Z willu $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _LEGS_GEOMETRY_H_
#define _LEGS_GEOMETRY_H_

#include "../share/RobotDef.h"
#include "../share/Common.h"
#include <iostream>
#include <iomanip>
#include "../share/XYZ_Coord.h"

using namespace std;

/* Robot body length constants */
static const double l1  = SHOULDER_TO_KNEE;
static const double l2  = KNEE_TO_PAW_FRONT;
static const double l4  = KNEE_TO_PAW_REAR;
static const double l3  = KNEE_OFFSET;
static const double lsh = BODY_LENGTH;

//
static const double PI = M_PI ;
inline double myabs (double x) { return fabs (x); }


const int maxPSTEP = 1000;

struct FSH_Coord{
	double f,s,h;
};

struct JOINT_Values{
	double rotator, shoulder, knee;
};

extern const XYZ_Coord tLeg;
extern const XYZ_Coord fbLegPadCenter;
extern const XYZ_Coord fbLegPadEdge;
extern const XYZ_Coord bbLegPad;
extern const XYZ_Coord fbLegBase;
extern const XYZ_Coord fbLegBaseEdge;
extern const XYZ_Coord bbLegToes;
extern const XYZ_Coord bbLegToesGnd;
extern const XYZ_Coord bbLegAirToes;
extern const XYZ_Coord fbCowlCurveCenter;
extern const XYZ_Coord fbCowlCurveCenterEdge;

ostream &operator<<(ostream &out , const FSH_Coord &a);

ostream &operator<<(ostream &out , const JOINT_Values &a);

extern double acos2(double a, double b);
extern double asin2(double a, double b);

extern XYZ_Coord frontBottomLegPoint(double angle, bool edge = true);	// a point on the cowling on the front lower leg
extern double frontBottomLegPointDist(double angle);	// the circumferencial distance of the point from the 0 point
extern double frontBottomLegPointAngle(double dist);	// the angle (radians) required for a given circumferencial distance

extern void invKinematics(const XYZ_Coord &pt, const XYZ_Coord &topL,
	       const XYZ_Coord &botL, JOINT_Values &theta);

extern void kinematics(const JOINT_Values &theta, const XYZ_Coord &topL,
		const XYZ_Coord &botL, XYZ_Coord &pt);

extern void inverseKinematicsFront(double x, double y, double z,
		double &theta1, double &theta2, double &theta3);

extern void inverseKinematicsFront(const XYZ_Coord &A, JOINT_Values &B);

extern void inverseKinematicsBack(double x, double y, double z,
		double &theta1, double &theta2, double &theta3);

extern void inverseKinematicsBack(const XYZ_Coord &A, JOINT_Values &B);

extern void transformFSH2XYZ(FSH_Coord &A , XYZ_Coord &B, double theta4);

extern void transformMoveFS(int numPoints, FSH_Coord *fshArr, double ForwardOffset, double SideOffset);

//rotate a point in a plane by alpha degree
extern void rotate2DCoordinate(double &x, double &y, double alpha);

//scale a point around a origin
extern void scale2DCoordinate(double &x, double &y, double OrgX, double OrgY, double ratio);

extern void interpolateJoints(JOINT_Values &result, const JOINT_Values start, const JOINT_Values end, double mix);

extern void legsGeometryDebug();	// prints out all the leg constants, both raw and calculated

#endif // _LEGS_GEOMETRY_H_
