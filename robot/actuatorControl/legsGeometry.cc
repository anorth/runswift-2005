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
 * $Id: legsGeometry.cc 6830 2005-06-23 07:10:53Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "legsGeometry.h"
#include "../share/Common.h"
#include "../share/XYZ_Coord.h"

static bool bDebugIllegal = false;
static const bool printDebugKinematics = false;

ostream &operator<<(ostream &out , const FSH_Coord &a){
	out << '[' << a.f << ", " << a.s << ", " << a.h << ']';
	return out;
}

ostream &operator<<(ostream &out , const JOINT_Values &a){
	out << '[' << RAD2DEG(a.rotator) << ", " << RAD2DEG(a.shoulder) << ", " << RAD2DEG(a.knee) << ']';
	return out;
}

/*
XYZ_Coord tLeg = {KNEE_OFFSET, SHOULDER_TO_KNEE, 0};
XYZ_Coord fbLeg = {KNEE_OFFSET, KNEE_TO_PAW_FRONT, 0};
XYZ_Coord bbLeg = {KNEE_OFFSET, KNEE_TO_PAW_REAR, 0};
*/

const XYZ_Coord tLeg(9, 69.5, 4.7); // top leg section


static const XYZ_Coord fbPadCenterRaw(28.3, 71.5, 0);
static const XYZ_Coord fbPadEdgeRaw(28.3, 71.5, -15);


static const XYZ_Coord bbPadRaw(21.3, 76.5, 0);
static const XYZ_Coord bbToesRaw(-28, -10, 0);
static const XYZ_Coord bbToesOffset(0, -10, 0);
static const double maxToeBend = DEG2RAD(-10);
static const XYZ_Coord bbGndToesRaw = addCopy(rotateCopyXY(subCopy(bbToesRaw, bbToesOffset), maxToeBend), bbToesOffset);

const XYZ_Coord fbLegPadCenter = rotateCopyXY(fbPadCenterRaw, DEG2RAD(-30));
const XYZ_Coord fbLegPadEdge = rotateCopyXY(fbPadEdgeRaw, DEG2RAD(-30));
const XYZ_Coord bbLegPad = rotateCopyXY(bbPadRaw, DEG2RAD(-30));

// measurment by weiming 11/06/05
const XYZ_Coord fbLegBase(-25, 40, 0);
const XYZ_Coord bbLegToesGnd(-35, 50, 0);


const XYZ_Coord fbLegBaseEdge(-25, 40, -15);
const XYZ_Coord bbLegAirToes = rotateCopyXY(addCopy(bbPadRaw, bbToesRaw), DEG2RAD(-30));
const XYZ_Coord bbLegToes = rotateCopyXY(addCopy(bbPadRaw, bbGndToesRaw), DEG2RAD(-30));


const XYZ_Coord fbCowlCurveCenter(40, 40, 0);
const XYZ_Coord fbCowlCurveCenterEdge(40, 40,-15);
static const XYZ_Coord fbCowlCurveRadius(-FRONT_LEG_COWL_RADIUS,0,0);

void legsGeometryDebug() {
	cout << "legsGeometryDebug::" << endl << endl;
	cout << "tLeg: " << tLeg << endl;
	cout << "fbPadCenterRaw: " << fbPadCenterRaw << endl;
	cout << "fbLegBase(measured by weiming): " << fbLegBase << endl;
	cout << "bbLegToesGnd(measured by weiming): " << bbLegToesGnd << endl;
	cout << "fbPadEdgeRaw: " << fbPadEdgeRaw << endl;
	cout << "bbPadRaw: " << bbPadRaw << endl;
	cout << "bbToesRaw: " << bbToesRaw << endl;
	cout << "bbToesOffset: " << bbToesOffset << endl;
	cout << "bbGndToesRaw: " << bbGndToesRaw << endl;
	cout << "fbLegPadCenter: " << fbLegPadCenter << endl;
	cout << "fbLegPadEdge: " << fbLegPadEdge << endl;
	cout << "bbLegPad: " << bbLegPad << endl;
	cout << "fbLegBaseEdge: " << fbLegBaseEdge << endl;
	cout << "bbLegAirToes: " << bbLegAirToes << endl;
	cout << "bbLegToes: " << bbLegToes << endl;
	cout << "frontBottomLegPoint(0): " << frontBottomLegPoint(0) << endl;
	double basePtAngleRad = DEG2RAD(0);
	XYZ_Coord fbLegPoint = frontBottomLegPoint(basePtAngleRad, false);
	cout << "fbLegPoint: " << fbLegPoint << endl;
	cout << "frontBottomLegPoint(DEG2RAD(30)): " << frontBottomLegPoint(DEG2RAD(30)) << endl;
	cout << "frontBottomLegPointDist(DEG2RAD(30)): " << frontBottomLegPointDist(DEG2RAD(30)) << endl;
	cout << endl;
}

double acos2(double a, double b) {
	if ((fabs(b) < EPSILON) || (fabs(a/b) > 1)) {
        if (bDebugIllegal)
            cout << "Illegal inverse (kinematics) - acos2(" << a << " / " << b << ")";
		return ((a < 0) == (b < 0))?acos(1.0):acos(-1.0);
	}
	return acos(a/b);
}

double asin2(double a, double b) {
	if ((fabs(b) < EPSILON) || (fabs(a/b) > 1)) {
        if (bDebugIllegal)
            cout << "Illegal inverse (kinematics) - asin2(" << a << " / " << b << ")";
		return ((a < 0) == (b < 0))?asin(1.0):asin(-1.0);
	}
	return asin(a/b);
}

XYZ_Coord frontBottomLegPoint(double angle, bool edge) {
	return addCopy(edge?fbCowlCurveCenterEdge:fbCowlCurveCenter,rotateCopyXY(fbCowlCurveRadius, -angle));
}

double frontBottomLegPointDist(double angle) {
	return angle*FRONT_LEG_COWL_RADIUS;
}

double frontBottomLegPointAngle(double dist) {
	return dist/FRONT_LEG_COWL_RADIUS;
}

void kinematics(const JOINT_Values &theta, const XYZ_Coord &topL,
		const XYZ_Coord &botL, XYZ_Coord &pt) {

	if (printDebugKinematics) {
		cout << "checking kinematics" << endl;
	}

	pt = botL;
	pt.rotateXY(theta.knee);
	if (printDebugKinematics) {
		cout << "rot knee: " << pt << endl;
	}
	pt.add(topL);
	if (printDebugKinematics) {
		cout << "leg plane: " << pt << endl;
	}
	pt.rotateYZ(theta.shoulder);
	if (printDebugKinematics) {
		cout << "rot out shoulder: " << pt << endl;
	}
	pt.rotateXY(theta.rotator);
	if (printDebugKinematics) {
		cout << "with rotator: " << pt << endl;
	}
}

static inline double invKinHelp(double ly, double lx, double ptx) {
	double theta1 = atan2(lx, ly);
	double dist = sqrt(SQUARE(lx) + SQUARE(ly));
	double theta2 = asin2(ptx, dist);

	return theta2 - theta1;
}

void invKinematics(const XYZ_Coord &pt, const XYZ_Coord &topL, const XYZ_Coord &botL, JOINT_Values &theta) {
	// calculate knee bend
	double topXYlenSqr = SQUARE(topL.x) + SQUARE(topL.y);
	double botXYlenSqr = SQUARE(botL.x) + SQUARE(botL.y);
	double zLen = topL.z + botL.z;
	double topXYtheta = atan2(topL.x, topL.y);
	double botXYtheta = atan2(botL.x, botL.y);

	double top = topXYlenSqr + botXYlenSqr + SQUARE(zLen) - SQUARE(pt.x) - SQUARE(pt.y) - SQUARE(pt.z);
	double bot = 2 * sqrt(topXYlenSqr * botXYlenSqr);
	theta.knee = PI + topXYtheta - botXYtheta - acos2(top, bot);

	// now find the whole leg size
	XYZ_Coord leg = botL;
	leg.rotateXY(theta.knee);
	leg.add(topL);

	// now find the shoulder angle (angle out from side of body)
	theta.shoulder = invKinHelp(leg.y, leg.z, pt.z);

	leg.rotateYZ(theta.shoulder);

	// now find the rotator angle
	theta.rotator = invKinHelp(leg.y, leg.x, pt.x);
}

static const double frontConstA = 2 * sqrt ((l1 * l1 + l3 * l3) * (l2 * l2 + l3 * l3));
static const double backConstA = 2 * sqrt ((l1 * l1 + l3 * l3) * (l4 * l4 + l3 * l3));
static const double legConstB = atan (l1 / l3);
static const double frontConstC = atan (l2 / l3);
static const double backConstC = atan (l4 / l3);

void inverseKinematicsFront(double x, double y, double z,
		double &theta1, double &theta2, double &theta3){
	double temp1,temp2,tempu,tempv;
	temp1  = l1 * l1 + 2 * l3 * l3 + l2 * l2 - x * x - y * y - z * z;
	temp2  = frontConstA;
	theta3 = 2 * PI - legConstB - frontConstC - acos2 (temp1, temp2);
	tempu  = 2 * l3 * sin (theta3 / 2) * sin (theta3 / 2) + l2 * sin (theta3);
	tempv  = l1 + 2 * l3 * sin (theta3 / 2) * cos (theta3 / 2) + l2 * cos (theta3);
	theta1 = asin2 (z, tempv);
	temp1  = y * tempv * cos (theta1) + tempu * x;
	temp2  = tempu * tempu + tempv * tempv * cos (theta1) * cos (theta1);
	theta2 = acos2 (temp1, temp2);
	/* bit of fudginess, since formula gives +/- for theta2 */
	if (myabs(x - tempv * cos (theta1) * sin (theta2) - tempu * cos (theta2)) > .001)
		theta2 *= -1;
}

void inverseKinematicsFront(const XYZ_Coord &A, JOINT_Values &B){
	inverseKinematicsFront(A.x, A.y, A.z, B.shoulder, B.rotator, B.knee);
}

void inverseKinematicsBack(double x, double y, double z,
		double &theta1, double &theta2, double &theta3){
	double temp1,temp2,tempu,tempv;
	temp1  = l1 * l1 + 2 * l3 * l3 + l4 * l4 - x * x - y * y - z * z;
	temp2  = backConstA;
	theta3 = 2 * PI - legConstB - backConstC - acos2 (temp1, temp2);
	tempu  = 2 * l3 * sin (theta3 / 2) * sin (theta3 / 2) + l4 * sin (theta3);
	tempv  = l1 + 2 * l3 * sin (theta3 / 2) * cos (theta3 / 2) + l4 * cos (theta3);
	theta1 = asin2 (z, tempv);
	temp1  = y * tempv * cos (theta1) - tempu * x;
	temp2  = tempu * tempu + tempv * tempv * cos (theta1) * cos (theta1);
	theta2 = acos2 (temp1, temp2);
	if (myabs(-x - tempv * cos (theta1) * sin (theta2) - tempu * cos (theta2)) > .001) {
		theta2 *= -1;
	}
}

void inverseKinematicsBack(const XYZ_Coord &A, JOINT_Values &B){
	inverseKinematicsBack(A.x, A.y, A.z, B.shoulder, B.rotator, B.knee);
}

void transformFSH2XYZ(FSH_Coord &A , XYZ_Coord &B, double theta4){
	B.z = A.s;
	B.y = A.h * cos (theta4) - A.f * sin (theta4);
	B.x = A.h * sin (theta4) + A.f * cos (theta4);
}


void transformMoveFS(int numPoints, FSH_Coord *fshArr, double ForwardOffset, double SideOffset){
	for (int i=0;i<numPoints;i++){
		fshArr[i].f += ForwardOffset;
		fshArr[i].s += SideOffset;
	}
}

void rotate2DCoordinate(double &x, double &y, double alpha){
    double xx = x * cos(alpha) + y * sin(alpha);
    double yy = - sin(alpha) * x + cos(alpha) * y;
    x = xx;
    y = yy;
}

void scale2DCoordinate(double &x, double &y, double OrgX, double OrgY, double ratio){
    x = ratio * (x - OrgX) + OrgX;
    y = ratio * (y - OrgY) + OrgY;
}

void interpolateJoints(JOINT_Values &result, const JOINT_Values start, const JOINT_Values end, double mix) {
	if (mix > 1)
		mix = 1;
	else if (mix < 0)
		mix = 0;

	result.rotator = (1-mix)*start.rotator + mix*end.rotator;
	result.shoulder = (1-mix)*start.shoulder + mix*end.shoulder;
	result.knee = (1-mix)*start.knee + mix*end.knee;
}


#ifdef LEGS_GEO
int main() {
  XYZ_Coord xyz;
  JOINT_Values leg;
  legsGeometryDebug();
  while(1) {
    cout << "x y z: ";
    cin >> xyz.x >> xyz.y >> xyz.z;
    invKinematics(xyz, tLeg, fbLegPadCenter, leg);
    cout << "knee: " << RAD2DEG(leg.knee) 
	 << " abuctor: " << RAD2DEG(leg.shoulder) 
	 << " rotator: " << RAD2DEG(leg.rotator) << endl;
  }

};
#endif // LEGS_GEO
