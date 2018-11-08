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
 * $Id: InfoManager.h 5367 2005-04-02 06:09:56Z nmor250 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _INFO_MANAGER_H_
#define _INFO_MANAGER_H_

#include "CommonSense.h"
#include "VisualCortex.h"
#include "../share/PWalkDef.h"
#include "gps.h"

static const int LOSTBALL_DIST = 500;
//this struct is extracted from beckham.h, which store (compressed) behaviour information
struct interpBehav {
    short timeToReachBall        : 16; // time to reach a ball
    unsigned int hasGrabbedBall  : 1;  // set if you grabbed a ball
    unsigned int hasSeenBall     : 1;  // set if you see a ball
    unsigned int hasLostBall     : 1;  // set if you have lost a ball long enough 
    unsigned int role            : 5;  // set if you are currently backing off (either getbehind or support pos)
    unsigned int roleCounter     : 8;  // number of frames you are assigned to the same role 

    interpBehav() {
        timeToReachBall = LOSTBALL_DIST;
        hasGrabbedBall = 0;
        hasSeenBall = 0;
        hasLostBall = 0;
        role = 0;
        roleCounter = 0;
    }

    interpBehav(int i) {
        memcpy(this, &i, sizeof(int));
    }

    int convert() {
        int i;
        memcpy(&i, this, sizeof(interpBehav));
        return i;
    }
};

class InfoManager{
    public:
        InfoManager();
        void setSensors(CommonSense *sensors);
        void setGPS(GPS *gps);
        void setVisualCortex(VisualCortex *visual);

        double getJointSensor(JointEnum jointIndex);
        long getAnySensor(int sensorIndex);
        long getJointPWMDuty(JointEnum jointIndex);
        const WMObj &getSelfLocation();
        double getBallConfidence();
        double getBallHeading();
        double getBallDistance();
        const Vector &getBallLocation();
        void getAverageBlobInfo(int &x,int &y);
    private:
        CommonSense *sensors;
        VisualCortex *visual;
        GPS *gps;
};

extern InfoManager gInfoManager;
#endif // _INFO_MANAGER_H_
