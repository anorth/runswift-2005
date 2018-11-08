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
 * $Id
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * Beckham Forward
 *
 **/ 


#include "defender.h"
#include "UNSW2004.h"

#ifdef COMPILE_ALL_CPP
using namespace Behaviours;
using namespace std;

namespace Defender{
    VisualObject ball;

    void initDefender(){
    }

    void initAction(){
        ball = vision->vob[vobBall];
    }

    bool canSeeGpsBall(int context, double &x, double &y){
        const Vector &gpsBall = gps->getBall(context);
        x = gpsBall.x;
        y = gpsBall.y;
        cout << "gpsBall " << x <<  ", " << y << " seen=" << gps->canSee(vobBall) << endl ;
        return true;
    }

    double getHeading(double x,double y){
        if ( y <= 0) return 0; //y can't be < 0, gps error
        if ( x == 0) return 0;
        if ( x > 0) return atan( y / x) - M_PI / 2 ;
        else return atan( y / x) + M_PI / 2 ;
    }

    bool goGetBall(double x,double y) {
        double head = getHeading(x,y);
        turnCCW = RAD2DEG(head);
        cout <<" Getting ball " << x << " " << y << " h=" << turnCCW << endl;

        setWalkParams();

        turnCCW = CLIP(turnCCW, 50.0);
        if (abs(turnCCW) < 5) {
            //avoid oscilating
            turnCCW = 0;
        }

        walkType = EllipseWalkWT;
        Behaviours::left = 0;
        return true;
    }

    void doDefender(){
        double x,y;
        initAction();
        if ( canSeeGpsBall(LOCAL, x,y) )
            goGetBall(x,y);
        if ( ball.cf > 0){
            UNSW2004::doUNSW2004TrackVisualBall();
        }
    }
}

#endif //COMPILE_ALL_CPP
