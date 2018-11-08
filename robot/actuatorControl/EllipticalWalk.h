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
 * $Id: EllipticalWalk.h 4364 2004-09-22 07:04:12Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _ELLIPTICAL_WALK_H
#define _ELLIPTICAL_WALK_H

#include "SpecialAction.h"
#include <iostream>
#include <iomanip>
#include "legsGeometry.h"
#include "../share/RobotDef.h"
#include "../share/ActionProDef.h"

using namespace std;

struct ElliplicalWalkParms {
    int elpWidthFront, elpWidthBack, elpHeightFront, elpHeightBack;
    int PG, hf, hb, hdf, hdb, ffo, fso, bfo, bso,turnAdjustment;
    void read(istream &in){
        in >> PG >> hf >> hb >> ffo >> fso >> bfo >> bso >> elpWidthFront >> elpHeightFront >> elpWidthBack >> elpHeightBack >> turnAdjustment;
    }
    void print(){
    }
};


class EllipticalWalk : public SpecialAction{
    public:
        EllipticalWalk();

        void setParameters(const AtomicAction &command);

        void setLearnedParameters(double eWidthFront, double eHeightFront, double eWidthBack, double eHeightBack);

        void setLearningParameters(const ElliplicalWalkParms &params);

        double getCurrentJointValue(int jointIndex);
        void goNextFrame();
        void reset();
        void writeToLog(const char *logFileName);
        bool isCompleted();
        bool usingHead(){
            return false;
        }
        void calibrate(double &forward, double &left, double &turnCCW);
        void getOdometry(double &delta_forward, double &delta_left, double &delta_turn, bool highGain);
        bool isSpecialAction(){ return false;}


    private:
        void updateNewLegsLocus();
        void calculateFSHCoordinates();
        void calculateXYZCoordinatesFromFSH();
        void calculateJointValues();
        void composeTheMove();

        void writeAllValues(ostream &out, FSH_Coord *fsh, XYZ_Coord *xyz, JOINT_Values *joints);
        void calculateEllipticFSH(FSH_Coord *fshLeft, FSH_Coord *fshRight, double elpWidth, double elpHeight, double jointHeight);
        void calculateTurnCCWFrontLeft(FSH_Coord *fshFrontLeft, double f, double t);
        void calculateTurnCCWFrontRight(FSH_Coord *fshFrontLeft, double f, double t);
        void calculateTurnCCWBackLeft(FSH_Coord *fshBackRight, double f, double t);
        void calculateTurnCCWBackRight(FSH_Coord *fshBackRight, double f, double t);

        FSH_Coord fshFrontRight[maxPSTEP], fshFrontLeft[maxPSTEP], fshBackRight[maxPSTEP], fshBackLeft[maxPSTEP];
        XYZ_Coord xyzFrontRight[maxPSTEP], xyzFrontLeft[maxPSTEP], xyzBackRight[maxPSTEP], xyzBackLeft[maxPSTEP];
        JOINT_Values jointsFrontRight[maxPSTEP], jointsFrontLeft[maxPSTEP], jointsBackRight[maxPSTEP], jointsBackLeft[maxPSTEP];

        int PG ;

        double hf ;
        double hb ;
        double hdf ;
        double hdb;
        double ffo;
        double fso;
        double bfo;
        double bso;

        double elpWidthFront;
        double elpHeightFront;
        double elpWidthBack;
        double elpHeightBack;
        double turnAdjustment;

        double forward, left, turnCCW;
        double theta; //theta5 in old code

};

#endif //_ELLIPTICAL_WALK_H
