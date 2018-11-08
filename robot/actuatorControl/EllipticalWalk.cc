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
 * $Id: EllipticalWalk.cc 6393 2005-06-03 03:56:02Z nmor250 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



//TODO: do LAZY calculation to improve speed.(When PG is large)
#include "EllipticalWalk.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cassert>
#include <cmath>

using namespace std;

const bool bDebugTurnComponent = false;
const bool bDebugLearning = false;
const bool bDebugLearningParameters = true;

#ifdef OFFLINE
const char * walkLogFile ="eWalk.log" ;
const bool bLoggedToFile = false;
#else
const char * walkLogFile ="/MS/OPEN-R/eWalk.log" ;
const bool bLoggedToFile = false;
#endif

double magicConstant=0.2;
double DF_MAGIC =  0.27632462021909698; //0.23801047281270363 ;//0.24533919425797207;
double DT_MAGIC = 2.1853035143769972; //4.8178219177919122; //2.1046153846153848;//4.8178219177919122;
double DL_MAGIC = 0.000742; //0.00087

//temporary function, to be moved to Common.h or legsGeometry.h
double arc_cotan(double cotg){
    double t = atan(1.0/cotg);
    if (t > 0) return t;
    else return t + PI;
}

double original_turnCCW;

EllipticalWalk::EllipticalWalk() : SpecialAction(){
    //PG = 160;
    //history of parameters:
    //first hand tune : 80 90 130 45 15 -65 5 40 20 45 40
    //first learned (with soft command) :    73 88 130 48 13 -66 7 44 22 47 40
    //learned with no soft command 76 94 126 43 13 -63 6 43 22 49 40 
    //learned with no soft command2 76 90 124 45 13 -62 4  45 20 49 40
    PG = 76;
    hf    = 94;
    hb    = 126;
    ffo   = 50;
    fso   = 13;
    bfo   = -63;
    bso   = 6; 

    //WARNING : this is half of the length
    elpWidthFront = 43;
    elpHeightFront = 22;
    elpWidthBack = 49;
    elpHeightBack = 40;


    turnAdjustment = 100.0;
    /*
    //first hand tune (with soft command) : 80 90 130 45 15 -65 5 40 20 45 40
    PG = 80;
    hf    = 90;
    hb    = 130;
    ffo   = 45;
    fso   = 15;
    bfo   = -65;
    bso   = 5; 

    //first learned (with soft command) :    73 88 130 48 13 -66 7 44 22 47 40
    PG = 73;
    hf    = 88;
    hb    = 130;
    ffo   = 48;
    fso   = 13;
    bfo   = -66;
    bso   = 7; 
    //WARNING : this is half of the length
    elpWidthFront = 44;
    elpHeightFront = 22;
    elpWidthBack = 47;
    elpHeightBack = 40;
    */

    theta = atan( ( BODY_WIDTH + 20) / (BODY_LENGTH + ffo - bfo) );

    forward = 0;
    left = 0;
    turnCCW = 0;
    updateNewLegsLocus();
}

void EllipticalWalk::calculateFSHCoordinates(){
    calculateEllipticFSH(fshFrontLeft, fshFrontRight, elpWidthFront, elpHeightFront,hf );
    calculateTurnCCWFrontLeft(fshFrontLeft,elpWidthFront, turnCCW);
    calculateTurnCCWFrontRight(fshFrontRight,elpWidthFront, turnCCW);

    transformMoveFS(PG, fshFrontLeft, ffo ,fso);
    transformMoveFS(PG, fshFrontRight, ffo ,fso);

    calculateEllipticFSH(fshBackLeft, fshBackRight, elpWidthBack, elpHeightBack, hf ); //this bug makes the walk faster . Fix it when do a proper learning
    calculateTurnCCWBackLeft(fshBackLeft,elpWidthBack, turnCCW);
    calculateTurnCCWBackRight(fshBackRight,elpWidthBack, turnCCW);

    transformMoveFS(PG, fshBackLeft, bfo ,bso);
    transformMoveFS(PG, fshBackRight, bfo ,bso);
}

void EllipticalWalk::calculateXYZCoordinatesFromFSH(){
    double theta4 = asin ((hb - hf) / lsh);
    for (int i=0;i<PG;i++){
        transformFSH2XYZ(fshFrontLeft[i], xyzFrontLeft[i], theta4);
        transformFSH2XYZ(fshFrontRight[i], xyzFrontRight[i], theta4);
        transformFSH2XYZ(fshBackLeft[i], xyzBackLeft[i], theta4);
        transformFSH2XYZ(fshBackRight[i], xyzBackRight[i], theta4);
    }
}

void EllipticalWalk::calculateJointValues(){
    for (int i=0;i<PG;i++){
        inverseKinematicsFront(xyzFrontLeft[i], jointsFrontLeft[i]);
        inverseKinematicsFront(xyzFrontRight[i], jointsFrontRight[i]);

        inverseKinematicsBack(xyzBackLeft[i], jointsBackLeft[i]);
        inverseKinematicsBack(xyzBackRight[i], jointsBackRight[i]);
    }
}

double coTgAlphaTypeOne(double f, double t, double theta){
    t = abs(t);
    return ( f/t - sin(theta) ) / cos(theta);
}

double coTgAlphaTypeTwo(double f, double t, double theta){
    t = abs(t);
    return ( f/t + sin(theta) ) / cos(theta);
}

void EllipticalWalk::calculateTurnCCWFrontLeft(FSH_Coord *fsh, double f, double t){
    if ( abs(t) < EPSILON ) return;
    double cotg_alpha = (t>0)?coTgAlphaTypeOne(f,t,theta):coTgAlphaTypeTwo(f,t,theta);
    
    if ( abs(cotg_alpha) < EPSILON){
        cout << "Bad alpha " << f << " " << t << " " << cotg_alpha << endl;
    }

    double alpha = (t>0)?-arc_cotan(cotg_alpha):arc_cotan(cotg_alpha);

    double d = abs( cos(theta) * t / sin(alpha) );
    if (bDebugTurnComponent)
        cout << "FL f="<< f << " t=" << t << " anfa=" << RAD2DEG(alpha) << " d=" << d<< endl;

    for (int i=0;i<PG;i++){
        rotate2DCoordinate(fsh[i].f, fsh[i].s, alpha);
        scale2DCoordinate(fsh[i].f, fsh[i].s, 0,0, d / f);
    }

}

void EllipticalWalk::calculateTurnCCWFrontRight(FSH_Coord *fsh, double f, double t){
    if ( abs(t) < EPSILON ) return;
    double cotg_alpha = (t>0)?coTgAlphaTypeTwo(f,t,theta):coTgAlphaTypeOne(f,t,theta);
    if ( abs(cotg_alpha) < EPSILON){
        cout << "Bad alpha " << f << " " << t << " " << cotg_alpha << endl;
        assert( 0 );
    }
    double alpha = (t>0)? arc_cotan(cotg_alpha):-arc_cotan(cotg_alpha);

    double d = abs( cos(theta) * t / sin(alpha) );
    if (bDebugTurnComponent)
        cout << "FR f="<< f << " t=" << t << " anfa=" << RAD2DEG(alpha) << " d=" << d<<  endl;

    for (int i=0;i<PG;i++){
        rotate2DCoordinate(fsh[i].f, fsh[i].s, alpha);
        scale2DCoordinate(fsh[i].f, fsh[i].s, 0,0, d / f);
    }

}

void EllipticalWalk::calculateTurnCCWBackLeft(FSH_Coord *fsh, double f, double t){
    if ( abs(t) < EPSILON ) return;
    double cotg_alpha = (t>0)?coTgAlphaTypeOne(f,t,theta):coTgAlphaTypeTwo(f,t,theta);
    if ( abs(cotg_alpha) < EPSILON){
        cout << "Bad alpha " << f << " " << t << " " << cotg_alpha << endl;
        assert( 0);
    }
    double alpha =  (t>0)?arc_cotan(cotg_alpha):-arc_cotan(cotg_alpha);

    double d = abs( cos(theta) * t / sin(alpha) );
    if (bDebugTurnComponent)
        cout << "BL f="<< f << " t=" << t << " anfa=" << RAD2DEG(alpha) <<  " d=" << d<< endl;

    for (int i=0;i<PG;i++){
        rotate2DCoordinate(fsh[i].f, fsh[i].s, alpha);
        scale2DCoordinate(fsh[i].f, fsh[i].s, 0,0, d / f);
    }

}

void EllipticalWalk::calculateTurnCCWBackRight(FSH_Coord *fsh, double f, double t){
    if ( abs(t) < EPSILON ) return;
    double cotg_alpha = (t>0)?coTgAlphaTypeTwo(f,t,theta):coTgAlphaTypeOne(f,t,theta);
    if ( abs(cotg_alpha) < EPSILON){
        cout << "Bad alpha " << f << " " << t << " " << cotg_alpha << endl;
        assert( 0);
    }
    double alpha = (t>0)?-arc_cotan(cotg_alpha):arc_cotan(cotg_alpha);

    double d = abs( cos(theta) * t / sin(alpha) );
    if (bDebugTurnComponent)
        cout << "BR f="<< f << " t=" << t << " anfa=" << RAD2DEG(alpha) <<  " d=" << d<< endl;

    for (int i=0;i<PG;i++){
        rotate2DCoordinate(fsh[i].f, fsh[i].s, alpha);
        scale2DCoordinate(fsh[i].f, fsh[i].s, 0,0, d / f);
    }
}

    void EllipticalWalk::calculateEllipticFSH(FSH_Coord *fshLeft, FSH_Coord *fshRight, double elpWidth, double elpHeight, double jointHeight){

        int quarterPG = PG/4;

        for (int i=0;i<quarterPG;i++){
            fshLeft[i].f = - 1.0 * elpWidth * i/quarterPG;
            fshLeft[i].s = 0;
            fshLeft[i].h = 0;
        }

		for (int i=quarterPG;i<3*quarterPG;i++){
			double t = 1.0 * (i - quarterPG) * 2*PI / PG ;
			fshLeft[i].f = - cos(t) * elpWidth;
			fshLeft[i].s = 0;
			fshLeft[i].h = sin(t) * elpHeight;
		}

		for (int i = 3*quarterPG; i < 4*quarterPG ; i++){
			double t = i - 3*quarterPG;
			fshLeft[i].f = elpWidth * (1.0 - 1.0 * t/quarterPG);
			fshLeft[i].s = 0;
			fshLeft[i].h = 0;
		}

		//transform h values
		for (int i=0;i<PG;i++){
			fshLeft[i].h = jointHeight- fshLeft[i].h;	// XXX: This only looks at the front legs! hb?
		}

		//mirror fshRight
		for (int i=0;i<PG;i++){
			fshRight[i].f = fshLeft[i].f;
			fshRight[i].s = -fshLeft[i].s;
			fshRight[i].h = fshLeft[i].h;
		}

	}


void EllipticalWalk::writeAllValues(ostream &out , FSH_Coord *fsh, XYZ_Coord *xyz, JOINT_Values *joints){
	for (int i=0;i<PG;i++){
		out << setw(2) << i << " " << fsh[i] << " " << xyz[i] << " " << joints[i] << endl;
	}
}

void EllipticalWalk::writeToLog(const char *logFileName){
	ofstream out(logFileName);
	writeAllValues(out, fshFrontLeft, xyzFrontLeft, jointsFrontLeft);
	out << endl;
	writeAllValues(out, fshFrontRight, xyzFrontRight, jointsFrontRight);
	out << endl;
	writeAllValues(out, fshBackLeft, xyzBackLeft, jointsBackLeft);
	out << endl;
	writeAllValues(out, fshBackRight, xyzBackRight, jointsBackRight);
	out << endl;

	out << "Parameters : " << endl;
	out << "PG = " << PG << endl;
	out << "hf = " << hf << endl;
	out << "hb = " << hb << endl;
	out << "hdf = " << hdf << endl;
	out << "hdb = " << hdb << endl;
	out << "ffo = " << ffo << endl;
	out << "fso = " << fso << endl;
	out << "bfo = " << bfo << endl;
	out << "bso = " << bso << endl;
	out << "elpWidthFront = " << elpWidthFront << endl;
	out << "elpHeightFront = " << elpHeightFront << endl;
	out << "elpWidthBack = " << elpWidthBack << endl;
	out << "elpHeightBack = " << elpHeightBack << endl;

	out.close();

	cout << "Written to log " << endl;
}

void EllipticalWalk::composeTheMove(){
    int halfPG = PG / 2;
	numOfSteps = PG;
	delete(posa);
	posa = (double *) new double[numOfSteps * posaLength];
	for (int i=0; i < numOfSteps ; ++i){
		posa[i*posaLength + 0] = i + 1; //one frame at a step

		for (int j=1;j<4;j++) posa[i*posaLength+j] = VERY_LARGE_SLONGWORD;//meaning we don't use it at all

		posa[i * posaLength + rightFrontJoint] = jointsFrontRight[ (i + halfPG) % numOfSteps].rotator;
		posa[i * posaLength + rightFrontShoulder] = jointsFrontRight[ (i + halfPG) % numOfSteps].shoulder;
		posa[i * posaLength + rightFrontKnee] = jointsFrontRight[ (i + halfPG) % numOfSteps].knee;

		posa[i * posaLength + leftFrontJoint] = jointsFrontLeft[ (i ) % numOfSteps].rotator;
		posa[i * posaLength + leftFrontShoulder] = jointsFrontLeft[ (i) % numOfSteps].shoulder;
		posa[i * posaLength + leftFrontKnee] = jointsFrontLeft[ (i) % numOfSteps].knee;

		posa[i * posaLength + rightRearJoint] = jointsBackRight[ (i ) % numOfSteps].rotator;
		posa[i * posaLength + rightRearShoulder] = jointsBackRight[ (i) % numOfSteps].shoulder;
		posa[i * posaLength + rightRearKnee] = jointsBackRight[ (i ) % numOfSteps].knee;

		posa[i * posaLength + leftRearJoint] = jointsBackLeft[ (i + halfPG) % numOfSteps].rotator;
		posa[i * posaLength + leftRearShoulder] = jointsBackLeft[ (i + halfPG) % numOfSteps].shoulder;
		posa[i * posaLength + leftRearKnee] = jointsBackLeft[ (i + halfPG) % numOfSteps].knee;

		for (int j=rightFrontJoint;j<=leftRearKnee;j++)
			posa[i*posaLength + j] = RAD2DEG(posa[i*posaLength + j]);

	}
	duration = numOfSteps;
}

void EllipticalWalk::updateNewLegsLocus(){
  
    if (bDebugLearning){
        cout << " Ell PG= " << PG << " " << hf << " " << hb << " " << ffo << " " << fso << " " << bfo << " " << bso << " " << elpWidthFront << " " << elpHeightFront << " " << elpWidthBack << " " << elpHeightBack << endl; 
    }
	calculateFSHCoordinates();
	calculateXYZCoordinatesFromFSH();
	calculateJointValues();
	composeTheMove();

	if (bLoggedToFile) writeToLog(walkLogFile);
}

void EllipticalWalk::calibrate(double &forward, double &left, double &turnCCW){
    static int count=0;
    if (bDebugTurnComponent)
        cout << "calibrate " << turnCCW << " to " << turnCCW * turnAdjustment / 100 << endl;
    original_turnCCW = turnCCW;

    turnCCW = turnCCW * turnAdjustment / 100;

    if (bDebugTurnComponent)
        if (count % 100 == 0)
            cout << "old " << turnCCW ;
    double trueConstant = (BODY_WIDTH+20) / ( 2*sin(theta) ); 
    turnCCW = tan(DEG2RAD(turnCCW) ) * trueConstant * magicConstant;
    if (bDebugTurnComponent)
        if (count ++ % 100 == 0)
            cout << "  -- new " << turnCCW << endl;

    forward = forward;
    left = left;
}

void EllipticalWalk::setParameters(const AtomicAction &command){
    forward = command.forwardSpeed;
    left = command.leftSpeed;
    turnCCW = command.turnCCWSpeed;
    calibrate(forward,left,turnCCW);
    updateNewLegsLocus();
}

void EllipticalWalk::setLearnedParameters(double eWidthFront, double eHeightFront, double eWidthBack, double eHeightBack){
    if (eWidthFront != elpWidthFront || eWidthBack != elpWidthBack || eHeightFront != elpHeightFront || eHeightBack != elpHeightBack){ // if the params are changed
        elpWidthFront = eWidthFront;
        elpWidthBack = eWidthBack;
        elpHeightFront = eHeightFront;
        elpHeightBack = eHeightBack;
        updateNewLegsLocus();
    }
}

double clipRange(double x, double min, double max){
    if (x < min) return min;
    else if (x > max) return max;
    else return x;
}

void EllipticalWalk::setLearningParameters(const ElliplicalWalkParms &params){
    if (bDebugLearningParameters)
        cout << "EllipticalWalk " << params.elpWidthFront << " " << params.elpHeightFront << " " << params.elpWidthBack << " " << params.elpHeightBack << endl;

    elpWidthFront = (int) params.elpWidthFront;
    elpHeightFront = (int) params.elpHeightFront;
    elpWidthBack = (int) params.elpWidthBack;
    elpHeightBack = (int) params.elpHeightBack;
    turnAdjustment = clipRange(params.turnAdjustment,0,200);
    PG = (int) params.PG;
    //magicConstant = params->PG / 100.0;
    //cout << "got magic " << magicConstant << endl;
    hb = (int) params.hb;
    hf = (int) params.hf;
    ffo = (int) params.ffo;
    fso = (int) params.fso;
    bfo = (int) params.bfo;
    bso = (int) params.bso;
    theta = atan( ( BODY_WIDTH + 20) / (BODY_LENGTH + ffo - bfo) );
    updateNewLegsLocus();
}

//jointIndex enumeration is defined in SpecialAction::JointEnum
double EllipticalWalk::getCurrentJointValue(int jointIndex){
	return SpecialAction::getCurrentJointValue(jointIndex);
}

void EllipticalWalk::goNextFrame(){
	//cout << "EllipticalWalk::goNextFrame:" << step << endl;
	//I want to do something here
	SpecialAction::goNextFrame();
}

void EllipticalWalk::reset(){
	//overide function in SpecialAction
	//SpecialAction::reset();
	//actionCompleted = true; //always complete
}

bool EllipticalWalk::isCompleted(){
    int pg2 = PG/2;
    return (step == 0 || step == pg2);
}

void EllipticalWalk::getOdometry(double &delta_forward, double &delta_left, double &delta_turn, bool highGain){
    if (highGain){
        delta_forward = 2.0 * elpWidthBack / PG * DF_MAGIC;
        delta_turn = original_turnCCW / PG * DT_MAGIC;
        delta_left = DL_MAGIC;
    }
    else{
        delta_forward = 2.0 * elpWidthBack / PG * 0.23686549161460843;
        delta_turn = original_turnCCW / PG * 1.7272727272727273;
        delta_left = DL_MAGIC;
    }
    //cout << "getOdometry " << turnCCW << " " << delta_forward << " " << delta_left << " " << delta_turn << endl;
}

