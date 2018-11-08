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
 * $Id: InfoManager.cc 6985 2005-06-28 08:32:53Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "InfoManager.h"
#include "../behaviours/Behaviours.h"

InfoManager gInfoManager;

const int SENSOR_INDEX[NUM_OF_EFFECTORS+1] = {
    0, //undefined
    ssHEAD_TILT,
    ssHEAD_PAN,
    ssHEAD_CRANE,

    ssFR_ROTATOR,
    ssFR_ABDUCTOR,
    ssFR_KNEE,

    ssFL_ROTATOR,
    ssFL_ABDUCTOR,
    ssFL_KNEE,

    ssRR_ROTATOR,
    ssRR_ABDUCTOR,
    ssRR_KNEE,

    ssRL_ROTATOR,
    ssRL_ABDUCTOR,
    ssRL_KNEE
    
};

InfoManager::InfoManager(){
    sensors = NULL;
    visual = NULL;
    gps = NULL;
}

void InfoManager::setGPS(GPS *g){
    this->gps = g;
}

void InfoManager::setSensors(CommonSense *s){
    this->sensors = s;
}

void InfoManager::setVisualCortex(VisualCortex *v){
    this->visual = v;
}

double InfoManager::getJointSensor(JointEnum jointIndex){
    int ssIndex = SENSOR_INDEX[jointIndex];
    return MICRO2DEG(sensors->sensorVal[ssIndex]);
}

long InfoManager::getAnySensor(int sensorIndex){
    if (sensorIndex >= NUM_SENSORS)
        return -1;
    else
        return sensors->sensorVal[sensorIndex];
}

long InfoManager::getJointPWMDuty(JointEnum jointIndex){
    int ssIndex = SENSOR_INDEX[jointIndex];
    return sensors->dutyCycleValue[ssIndex];
}

const WMObj &
InfoManager::getSelfLocation(){
    return gps->self();
}

const Vector &
InfoManager::getBallLocation(){
    return gps->getBall(GLOBAL);
}

double InfoManager::getBallDistance(){
    return Behaviours::vBall.d;
}

double InfoManager::getBallHeading(){
    return Behaviours::vBall.h;
}

double InfoManager::getBallConfidence(){
    return visual->vob[vobBall].cf;
}

//get the centroid of 3 largest red blob.
//this is for OpenChallenger that assumes that there's only one robot.
#if 0 // disabled for SubVision
void InfoManager::getAverageBlobInfo(int &x,int &y){
    x = 0;
    y = 0;
    int i=0;
    //cout << "getAverageBlobInfo: " << visual->blobs->count(cROBOT_RED) << endl;
    for (i=0;i< visual->blobs->count(cROBOT_RED) ; i++){
        BlobInfo& redBlob = visual->blobs->getBlob(cROBOT_RED,i);
        //cout << "Pointer : " << &redBlob << endl;
        int cx = (redBlob.getXMin() + redBlob.getXMax() ) / 2;
        int cy = (redBlob.getYMin() + redBlob.getYMax() ) / 2;
        //TODO: put elevation check here
        x += int(cx);
        y += int(cy);
        //cout << "cx = " << cx << "  cy = " << cy << endl;
    }
    if (i != 0){
        x /= i;
        y /= i;
    }
    //cout << "x = " << x << "  y = " << y << endl;
}
#endif
