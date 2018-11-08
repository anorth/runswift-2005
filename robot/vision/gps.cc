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
 * $Id: gps.cc 
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

#include <cmath>
#include <cstdio>
#include <iostream>
#ifdef OFFLINE
#include <sstream>
#include <iomanip>
#endif
#ifdef VALGRIND
#include <valgrind/memcheck.h>
#endif

#include "gps.h"
#include "Possible.h"
#include "KalmanInfo2D.h"
#include "KI2DWithVelocity.h"
#include "../share/Common.h"
#include "../share/SharedMemoryDef.h"
#include "../share/WirelessSwitchboard.h"
#include "../../base/subvision/simplexMin.h"


using namespace std;
//#define GRADIENT_ASCENT_VER_1
#define GRADIENT_ASCENT_VER_2

#define ERICDEBUG 0
//#define GPS_VISION_UPDATE_DEBUG
//#define GPS_EDGE_UPDATE_DEBUG
//#define GPS_VISUAL_CONFIDENCE_DEBUG

#ifdef OFFLINE
//#define GPS_GA_PLOTDATA
//#define DIRECT_POSITION_DEBUG
#endif


static const bool goalSkippedSilence = true;
static const int TEAMMATE_GPS_COUNTER = 45;

static const bool useSymmetry = false;
static const double symmetryWeight = 0.05;

// Possible coordinates for the vobs
static const long coordRedBased[6][2] =
{
    {FIELD_WIDTH / 2, FIELD_LENGTH},                    //far goal
    {FIELD_WIDTH / 2, 0},                               //close goal

    {BEACON_LEFT_X, BEACON_FAR_Y},                      //far left beacon
    {BEACON_RIGHT_X, BEACON_FAR_Y},                     //far right beacon
    {BEACON_LEFT_X, BEACON_CLOSE_Y},                    //close left beacon
    {BEACON_RIGHT_X, BEACON_CLOSE_Y}                    //close right beacon
};

static const long coordBlueBased[6][2] =
{
    {FIELD_WIDTH / 2, 0},                               //close goal
    {FIELD_WIDTH / 2, FIELD_LENGTH},                    //far goal

    {BEACON_RIGHT_X, BEACON_CLOSE_Y},                   //close right beacon
    {BEACON_LEFT_X, BEACON_CLOSE_Y},                    //close left beacon
    {BEACON_RIGHT_X, BEACON_FAR_Y},                     //far right beacon
    {BEACON_LEFT_X, BEACON_FAR_Y},                      //far left beacon
};

// for opponent tracking
static const double oppIniX[4] = 
        {FIELD_WIDTH/2, FIELD_WIDTH/2, 3*FIELD_WIDTH/4, FIELD_WIDTH/2};
static const double oppIniY[4] = 
        {FIELD_LENGTH, 3*FIELD_LENGTH/4 , 3*FIELD_LENGTH/4, FIELD_LENGTH/2};
static const double friendIniX[NUM_FRIENDS_VIS] = 
        {FIELD_WIDTH/2, FIELD_WIDTH/2, 3*FIELD_WIDTH/4/*, FIELD_WIDTH/2*/};
static const double friendIniY[NUM_FRIENDS_VIS] = 
        {WALL_THICKNESS,FIELD_LENGTH/4, FIELD_LENGTH/4/*, FIELD_LENGTH/2*/};
static const double oppIniXVar = get95CF(100);
static const double oppIniYVar = get95CF(100);

static const bool raySilence = true;

// maximum dimension of a "measurement" for the frame
static const int MAX_MEASUREMENT_DIM = 9;

// the growth in opponent position variance
static const double OPPONENT_VAR_GROWTH = get95CF(2);

// varience in movement = P
static const int MOTION_VAR = 10;
static const int MOTION_H_VAR = 6;
static const int WIRELESS_VAR = 50;

static const int vobMinBlueDog = vobBlueDog;
static const int vobMaxBlueDog = vobBlueDog4;
static const int vobMinRedDog = vobRedDog;
static const int vobMaxRedDog = vobRedDog4;

// These determine how small a gaussian component of r has to be before
//  it is culled from the sum, and how close together two components
//  can be before they are merged.
static const double cullThreshold = 1E-50;
//give lesser threshold because it too broad range (left right and keep joining)
static const double mergeThreshold = 5.0; //cm 100.0 cm2; //20 cm2;
static const double mergeCovThreshold = 200.0;//10000.0;
    
// This determines at what probability a measurement is regarded as
//  noise, and so will be ignored
static const double noiseThreshold = 1E-2;

// weight for exponential decay of visPull
static const int vpHistoryLength = 25;
//static const double visPullWeight = 0.8;

static const double badVisionUpdateWeight = 0;	// 0.01;	// probability that a vision update is bad

static const bool useRelativeCullWeight = true;

GPS::GPS () {
    targetGoal = vobBlueGoal;    // by default the robot is going towards blue goal
    goalNotSet = true;

    frameId = 0;
	selfAccessId = -1;
    ballAccessId = -1;
	lastBallOrigin = 'n';
    teammateAccessId = -1;
    lastTeammateAccessed = -1;
    lastTeammateOrigin = 'n';
	oGoalAccessId = -1;
    tGoalAccessId = -1;
    vBallAccessId = -1;
	lastvBallOrigin = 'n';

	// Update or not?
	doGPSMotionUpdate = true;
	doGPSVisionUpdate = true;
    doGPSOpponentUpdate = true;

    //!!ERIC these are temp for distance calibration
    sendDebug = false;
    debugCounter = 0;
    debugParm = 0;
	
	isPenalised = false;

    double rx = FIELD_WIDTH / 2.0;
    double ry = FIELD_LENGTH / 2.0;
    	
	// Robot position probability starts with only one gaussian
    numGaussians = 1;
    r[0].mean.pos.setVector(vCART,rx,ry);
    r[0].mean.h = HALF_CIRCLE/2;
    r[0].cov(0,0)= get95CF (FIELD_WIDTH);
    r[0].cov(0,1)= 0.0;
    r[0].cov(0,2)= 0.0;
    r[0].cov(1,0)= 0.0;
    r[0].cov(1,1)= get95CF (FIELD_LENGTH);
    r[0].cov(1,2)= 0.0;
    r[0].cov(2,0)= 0.0;
    r[0].cov(2,1)= 0.0;
    r[0].cov(2,2)= get95CF (HALF_CIRCLE);
	r[0].weight = 1.0;
	r[0].mean.updateVar(&r[0].cov);
	
	vbPull(0,0) = 0.0;
	vbPull(1,0) = 0.0;
	vbPull(2,0) = 0.0;
	
	visPull(0,0) = 0.0;
	visPull(1,0) = 0.0;
	visPull(2,0) = 0.0;
	vpHistory = new MVec3[vpHistoryLength];
	for(int i=0 ; i<vpHistoryLength ; i++)
		vpHistory[i].reset();
	vpIndex = 0;

    unitMatrix3 = new MMatrix3(1);
    unitMatrix4 = new MMatrix4(1);

    noBallFrames = 1;

    //initialise all teammates to not exist
    //when you get a message from them over ride this
    for (int i=0; i<NUM_TEAM_MEMBER; i++) {
		wmTeammate[i].hVar=INVALID_FLAG;
	}

	for (int i=0; i<NUM_TEAM_MEMBER; i++) {
		tmBall[i].pos.setVector(vCART,0,0);
		tmBall[i].posVar = get95CF(500);
	}

    // initialise noone can see ball
    shareBallx = 0;
    shareBally = 0;
    shareBallvar = VERY_LARGE_INT;
    sbRobotNum = -1;
	
    newball = new KI2DWithVelocity(this, FIELD_WIDTH/2, FIELD_LENGTH/2,
                        get95CF(FIELD_WIDTH/2), get95CF(FIELD_LENGTH/2));
    
    // initialise opponent robots' positions
    for(int i=0 ; i<NUM_OPPONENTS ; i++)
        opponents[i] = new KalmanInfo2D(this,oppIniX[i],oppIniY[i],oppIniXVar,oppIniYVar);

#ifdef VISUAL_TRACK_FRIENDS
    for(int i=0 ; i<NUM_FRIENDS_VIS ; i++)
        friends[i] = new KalmanInfo2D(this,friendIniX[i],friendIniY[i],oppIniXVar,oppIniYVar);
#endif // VISUAL_TRACK_FRIENDS

    view = 0;
    
    directx = -1.0;
    directy = -1.0;
    directh = -1.0;

    motionForward = 0.0;
    motionLeft = 0.0;
    motionTurn = 0.0;
    motionFCounter = 0;
    prevMotionForward = prevMotionLeft = prevMotionTurn = 0.0;
    prevMotionPWM = motionPWM = 0;
    
    doLatTest = false;

    oldx = r[0].mean.pos.x;
    oldy = r[0].mean.pos.y;
    oldh = r[0].mean.h;
    oldvx = r[0].cov(0,0);
    oldvy = r[0].cov(1,1);
    oldvh = r[0].cov(2,2);
    oldcxy = r[0].cov(0,1);
    oldcxh = r[0].cov(0,2);
    oldcyh = r[0].cov(1,2);
    
    edgeDebugCounter = 0;
    
    // For localisation challenge
    numPink = 0;
    pinkDecreaseCounter = 0;
    pinkUpdate = false;
    allowPinkMapping = false;
    landmarksInitialised = false;
    for (int i = 0; i < maxPink; i++)
        pinkConf[i] = 0;
    
    // For weight scaler statistics
    for (int i = 0; i < 10; i++) {
        nX[i] = 0;
        sumX[i] = 0.0;
        sumXSquared[i] = 0.0;
    }

#ifdef OFFLINE
    outputStream = NULL;
    tiltCalibrationOutputStream = NULL;
#endif
#ifdef LOCALISATION_CHALLENGE
    // For localisation challenge
    numStationaryRecordedAngle = 0;
    bodyMoving = true;
#endif //LOCALISATION_CHALLENGE
}

GPS::~GPS()
{
    if (vpHistory != NULL)
    {
        delete vpHistory;
        vpHistory = NULL;
    }
    if (unitMatrix3 != NULL)
    {
        delete unitMatrix3;
        unitMatrix3 = NULL;
    }
    if (unitMatrix4 != NULL)
    {
        delete unitMatrix4;
        unitMatrix4 = NULL;
    }
    if (newball != NULL)
    {
        newball = NULL;
    }
    for(int i=0 ; i<NUM_OPPONENTS ; i++)
    {
        if (opponents[i] != NULL)
            delete opponents[i];
    }
}
// append gps info
int GPS::storeGps(unsigned char *add, int base) {
    int ret = base;
    
	/*
    memcpy(add + ret, &numGaussians, sizeof(numGaussians));
	ret += sizeof(numGaussians);
    
    for (int i = 0; i < numGaussians; i++) {
	    ret = r[i].mean.serialize(add, ret);
    	ret = r[i].cov.serialize(add, ret);
		memcpy(add + ret, &r[i].weight, sizeof(double));
		ret += sizeof(double);
	}*/
    // Only store first gaussian for the moment (this should be temporary)
    ret = r[0].mean.serialize(add, ret);
    ret = r[0].cov.serialize(add, ret);
		
    ret = visPull.serialize(add, ret);
    ret = newball->serialize(add, ret);
    ret = vbPull.serialize(add, ret);
    
    for(int i=0 ; i<NUM_TEAM_MEMBER ; i++)
        ret = wmTeammate[i].serialize(add, ret);
    for(int i=0 ; i<NUM_OPPONENTS ; i++)
        ret = opponents[i]->serialize(add, ret);
    
	return ret;
}

// restore gps information
int GPS::restoreGps(unsigned char *add, int base) {
    
    int ret = base;
	
    /*
    memcpy(&numGaussians, add + ret, sizeof(numGaussians));
	ret += sizeof(numGaussians);
    
	for (int i = 0; i < numGaussians; i++) {
	    ret = r[i].mean.unserialize(add, ret);
    	ret = r[i].cov.unserialize(add, ret);
		memcpy(&r[i].weight, add + ret, sizeof(double));
		ret += sizeof(double);
	}*/
    
    // Only restore first gaussian for the moment (this should be temporary)
    numGaussians = 1;
    ret = r[0].mean.unserialize(add, ret);
    ret = r[0].cov.unserialize(add, ret);    
    r[0].weight = 1.0;
	r[0].mean.updateVar(&r[0].cov);
    
    ret = visPull.unserialize(add, ret);
    ret = newball->unserialize(add, ret);
    ret = vbPull.unserialize(add, ret);
    
    for(int i=0 ; i<NUM_TEAM_MEMBER ; i++)
        ret = wmTeammate[i].unserialize(add, ret);
    for(int i=0 ; i<NUM_OPPONENTS ; i++)
        ret = opponents[i]->unserialize(add, ret);
	
	return ret;
}

//This is called by Vision.cc
//to set your goals and orientation of the beacons.
void GPS::SetGoals(bool isRedRobot) {
    if (!isRedRobot) {
        targetGoal        = vobYellowGoal;
        ownGoal           = vobBlueGoal;
        backLeftBeacon    = vobPinkBlueBeacon;
        backRightBeacon   = vobBluePinkBeacon;
        farLeftBeacon     = vobPinkYellowBeacon;
        farRightBeacon    = vobYellowPinkBeacon;

        coord              = &coordBlueBased;

		for (int i = 0; i < maxGaussians; i++)
        	r[i].mean.vobType = vobBlueDog; 
        
        vobMinOpponent = vobMinRedDog;
        vobMaxOpponent = vobMaxRedDog;
#ifdef VISUAL_TRACK_FRIENDS
        vobMinFriend = vobMinBlueDog;
        vobMaxFriend = vobMaxBlueDog;
#endif // VISUAL_TRACK_FRIENDS

    } else {
        targetGoal        = vobBlueGoal;
        ownGoal           = vobYellowGoal;
        backLeftBeacon    = vobYellowPinkBeacon;
        backRightBeacon   = vobPinkYellowBeacon;
        farLeftBeacon     = vobBluePinkBeacon;
        farRightBeacon    = vobPinkBlueBeacon;

        coord              = &coordRedBased;

        for (int i = 0; i < maxGaussians; i++)
        	r[i].mean.vobType = vobRedDog;
        
        vobMinOpponent = vobMinBlueDog;
        vobMaxOpponent = vobMaxBlueDog;
#ifdef VISUAL_TRACK_FRIENDS
        vobMinFriend = vobMinRedDog;
        vobMaxFriend = vobMaxRedDog;
#endif // VISUAL_TRACK_FRIENDS
        
    }
    goalNotSet = false;
}

void GPS::resetGaussians(double x, double y, double h,
                        double covX, double covY, double covH) { 
        numGaussians = 0;

        SingleGaussian g1; 

        MMatrix3 cov(0);
        cov(0,0) = covX;
        cov(1,1) = covY;
        cov(2,2) = covH;

        g1.mean.pos.setVector(vCART,x,y);
        g1.mean.h = h; 
        g1.cov = cov;
        g1.weight = 1; 

        insertGaussian(g1); 
}

// Reset the ball to the given x, y, velocity
void GPS::resetBall(double x, double y, double dx, double dy) {
    newball->setVal(x, y, 30.0, dx, dy);
}

void GPS::setPenalised(bool p) {
	isPenalised = p;
    
    // Reset the dog's gaussians.
    // We know the two possible locations when the dog is penalised.   
    if (isPenalised) {
        numGaussians = 0;

        SingleGaussian g1; 
        SingleGaussian g2; 

        MMatrix3 cov(0);         
        cov(0,0) = 100;  // the robot's x position is known.
        cov(1,1) = 900;  // the robot's y position might vary.
        cov(2,2) = 900;  // the robot's heading is known.              

        g1.mean.pos.setVector(vCART,0,FIELD_LENGTH/2);
        g1.mean.h = 0; 
        g1.cov = cov; 
        g1.weight = 0.5; 

        g2.mean.pos.setVector(vCART,FIELD_WIDTH,FIELD_LENGTH/2); 
        g2.mean.h = 180;
        g2.cov = cov; 
        g2.weight = 0.5;

        insertGaussian(g1); 
        insertGaussian(g2);
    }
}

void GPS::setBallOut(bool outByOpponents) {
    // if we can see the ball ignore the signal.
    // FIXME: if seen in the past few frames?
    if (canSee(vobBall))
        return;

    Vector ballv = newball->getPos(GLOBAL);
    double x, y;

    // Note that it is safe to call this multiple times (with the same
    // outByOpponents value) as the ball stays in the same quadrant of
    // the field
    
    // y position depends on who knocked it out as well as which half
    if (outByOpponents) {
        if (ballv.getY() > FIELD_LENGTH/2)
            y = FIELD_LENGTH - DROPIN_OFFSET_Y;
        else
            y = FIELD_LENGTH/2.0 - 1; // -1 so it's safe to call repeatedly
    } else {
        if (ballv.getY() > FIELD_LENGTH/2)
            y = FIELD_LENGTH/2.0 + 1; // +1 ditto
        else
            y = DROPIN_OFFSET_Y;
    }

    // FIXME: what should we do if it's close to the halfway and we are
    // not sure which side it went out on?
    
    // x position depends on last position. If it goes out near the goal
    // then we can't be so sure which side, so one robot picks the 
    // the other side
    bool IAmLeftMost = false, IAmRightMost = false;
#if 0 
    // disabled - this hurts often
    double myX = self().pos.getX();
    // determine whether I am left or right most forward player
    for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
		if (teammate(i, 'g').pos.getX() > myX 
                && teammate(i, 'g').pos.getY() > 0.5 * FIELD_LENGTH)
            IAmRightMost = false;
        else if (teammate(i, 'g').pos.getX() < myX 
                && teammate(i, 'g').pos.getY() > 0.5 * FIELD_LENGTH)
            IAmLeftMost = false;
	}
#endif
    if (ballv.getX() > 0.5 * FIELD_WIDTH) {
        x = FIELD_WIDTH - DROPIN_OFFSET_X;
        // in centre field, one robot goes the other way
        if (ballv.getX() < 0.75 * FIELD_WIDTH 
                && (ballv.getY() > 0.8 * FIELD_WIDTH 
                    || ballv.getY() < 0.2 * FIELD_WIDTH)
                && IAmLeftMost)
            x = DROPIN_OFFSET_X;
    } else {
        x = DROPIN_OFFSET_X;
        // in centre field, one robot goes the other way
        if (ballv.getX() > 0.25 * FIELD_WIDTH 
                && (ballv.getY() > 0.8 * FIELD_WIDTH 
                    || ballv.getY() < 0.2 * FIELD_WIDTH)
                && IAmRightMost)
            x = FIELD_WIDTH - DROPIN_OFFSET_X;
    }

    //cerr << "gps setting ball to " << x << ", " << y << endl;
    newball->setVal(x, y, 30); 
}
    

//gets called by ReadyEffectors in ActuatorControl
//which means it gets called everytime a pwalk half step is completed
//
// This is the time update part of the Kalman filter
// Takes in feedback from locomotion and predicts a new location
//
// Calculates a priori state
// dForward - amount moved forward in cm
// dLeft - amount moved to the left in cm
// dTurn - degrees turned to the left from straight ahead
// note - assumes that turn is performed last
//

int MotionUpdateCount = 0; //for debugging only

void GPS::GPSMotionUpdate(double dForward, double dLeft, double dTurn, long PWM, 
			  bool debug, bool isTurnNeckBase) {

#ifdef OFFLINE
    if (debug)
    {
        cout << __func__ << "dForward:"<<dForward <<"dLeft:" <<dLeft <<"dTurn:"<<dTurn << "PWM:" << PWM <<endl; 
        cout << __func__ << "doGPSMotionUpdate:" <<doGPSMotionUpdate<<" isPenalised:"<<isPenalised<<endl;
    }
#endif
   
    if (!doGPSMotionUpdate)
        return;

    if(isPenalised)
        return;
    
    // Check for NaN or Inf inputs
    if (dForward != dForward || dLeft != dLeft || dTurn != dTurn) {
        cout << "GPS::GPSMotionUpdate error: One or more inputs are NaN or Inf!\n";
        return;
    }

    newball->shiftLastBallPos(dForward, dLeft, dTurn); // to shift the velocity history    
           
    motionForward += dForward;
    motionLeft += dLeft;
    motionTurn += dTurn;
    motionFCounter ++;
    motionPWM += PWM;
    
    
    // Increase variance faster if we have detected that we are stuck.
    double stuckVariance, stuckScaler;
    if (PWM < 900) stuckVariance = 0.0;
    else 
    {
#ifdef LOCALISATION_CHALLENGE
        cout << __func__ << " Possibly stuck." << endl;
#endif        
        stuckVariance = (PWM - 300) * 0.05;
        //stuckVariance = 0.0; uncomment this if we want to not consider if stuck.
    }
    
    stuckScaler = 1.0;
    DER_DEBUG(
        // Don't use full motion update if stuck
        if (PWM > 750) {
            stuckScaler = 1.0 / (1.0 + SQUARE(PWM - 750) * 0.0001);
            cout << "Stuck scaler: " << stuckScaler << "\n";
        }
    )
  
    for (int i = 0; i < numGaussians; i++) {
        double dx, dy, dh;
            
        // determine movement according to our original heading
        dx = dForward * cos (DEG2RAD(r[i].mean.h));
        dx -= dLeft * sin (DEG2RAD(r[i].mean.h));
        dy = dForward * sin (DEG2RAD(r[i].mean.h));
        dy += dLeft * cos (DEG2RAD(r[i].mean.h));
        dh = dTurn;

        dx *= stuckScaler;
        dy *= stuckScaler;
        dh *= stuckScaler;

        // save original heading and then alter it
        double oh = r[i].mean.h;
        r[i].mean.h += dh;
    

	if ( !isTurnNeckBase ) {
	  // alter dx and dy due to the difference between the turn centre and the
	  // centre of localisation (neckbase) - this is directly from old code 2002
	  dx += TURN_CENTRE_TO_NECK * (cos(DEG2RAD(r[i].mean.h)) - cos(DEG2RAD(oh)));
	  dy += TURN_CENTRE_TO_NECK * (sin(DEG2RAD(r[i].mean.h)) - sin(DEG2RAD(oh)));
	}    
	

        // update current position and clip (to be added)
        double rx = r[i].mean.pos.x + dx;
        double ry = r[i].mean.pos.y + dy;
        if (debug)
        { 
            cout << __func__ <<" "<<i<<":: posX:"<<r[i].mean.pos.x<<" rx:"<<rx 
            << " posY:" << r[i].mean.pos.y << " ry:" << ry <<endl;
        }
        ANGLE_FIX(r[i].mean.h);
        FieldClipping(rx, ry);
        r[i].mean.pos.setVector(vCART,rx,ry);
    
        // update variance
        r[i].cov(0,0) += 2 + get95CF(dx) + stuckVariance;
        r[i].cov(1,1) += 2 + get95CF(dy) + stuckVariance;
        r[i].cov(2,2) += 0.3 + get95CF(dh) + stuckVariance * 0.5;
    
        r[i].mean.updateVar(&r[i].cov);
    }
    if (debug)
    {
        cout <<"End of "<<__func__ <<endl;
    }        
}

// This generally gets called when robot is penalised and it's position is
// to be reset
void GPS::GPSSetVals(double newx, double newy, double newh, const MMatrix3& newC) {
    // Check for NaN inputs
    if (newx != newx || newy != newy || newh != newh) {
        cout << "GPS::GPSSetVals error: One or more inputs are NaN or Inf!\n";
        return;
    }
    
	numGaussians = 1;
    
	r[0].mean.pos.setVector(vCART,newx,newy);
	r[0].mean.h = newh;
	r[0].cov = newC;
	r[0].weight = 1.0;	
	r[0].mean.updateVar(&r[0].cov);
}

// Debugging method that outputs current multi-modal distribution data

void GPS::GPSOutputSelfData() {
    cout << "=== GPSOutputSelfData: " << numGaussians << " modes ===\n";

    for (int i = 0; i < numGaussians; i++) {
        cout << "Gaussian " << i << " has weight " << r[i].weight << "\n";
        cout << "Mean: x " << r[i].mean.pos.x << "   y " << r[i].mean.pos.y;
        cout << "   h " << r[i].mean.h << "   Covariance:\n";
        r[i].cov.printOut();
    }
    
    // Now output update stastics
    for (int i = 0; i < 10; i++) {
        double mean, stdDev;
    
        if (nX[i] > 0) {
            mean = sumX[i] / nX[i];
            stdDev = sqrt(sumXSquared[i] - SQUARE(mean));
        } else {
            mean = 0.0;
            stdDev = 0.0;
        }
    
        cout << nX[i] << " " << i << " dimensional updates. ";
        cout << "Mean: " << mean << "   Standard dev: " << stdDev << "\n";
    }
}

// Gets called by ResultCamera in Vision.cc
// which means it gets called on every camera frame.
// it is the measurement correction part of the kalman filter.
//
// This is the Measurement update part of the Kalman filter
// Takes in an observation from vision and "corrects" the time update predition
//
// Calculates a posteriori state
// nvo - an array of all types of VisualObjects
// On every camera frame vision sets this array (ie sets distances and variances
// to all the types of vobs)
// If a vob was not seen in that frame then its variance is set high

void GPS::GPSVisionUpdate (VisualObject * nvo, bool isHeadMoving) {
    SingleGaussian temp;
    double lastWeight = 0.0;

	frameId++;

	if (!doGPSVisionUpdate)
		return;

	// Prevent the frameId growing too large..
	// in a 20 minute game, the id should grow anywhere close to this though.
	if (frameId > 1000000)
		frameId=0;
	
	// Reset behaviours value.		
    for (int i = 0; i < numGaussians; i++)
        r[i].mean.behavioursVal = 0;
	
	// First process friendly teammates, throwing out info that is too old.
	for(int i=0 ; i < NUM_TEAM_MEMBER ; i++) {
		if(wmTeammate[i].counter > 0) {
			wmTeammate[i].counter--;
			if((wmTeammate[i].counter == 0) && (sbRobotNum == (i+1)))
				shareBallvar = VERY_LARGE_INT;
		}
	}


    UpdateObservedVobs(nvo);
    view = 0;
    
    directx = -1.0;
    directy = -1.0;
    directh = -1.0;
	
	if(isPenalised)
		return;

	
	double jacX[MAX_MEASUREMENT_DIM];
	double jacY[MAX_MEASUREMENT_DIM];
	double jacH[MAX_MEASUREMENT_DIM];
	double var[MAX_MEASUREMENT_DIM];
	double iv[MAX_MEASUREMENT_DIM];
	int measDim, mode;

    
    // Make a low probability, large variance mode
    if (numGaussians < maxGaussians) ++numGaussians;
    MMatrix3 largeCov(get95CF(500));
    largeCov(2, 2) = get95CF(300);
    r[numGaussians-1].mean.pos.setVector(vCART,FIELD_WIDTH/2,FIELD_LENGTH/2);
    r[numGaussians-1].mean.h = HALF_CIRCLE/2;
    r[numGaussians-1].cov    = largeCov;
    r[numGaussians-1].weight = 1E-12;
    r[numGaussians-1].mean.updateVar(&r[numGaussians-1].cov);
    
    
    // copy the current modes
    // with a low weight
    // to account for the possibility of bad vision
    int oldNumGaussians = numGaussians;
    for (mode = 0; mode < numGaussians; mode++) {
        rCopy[mode] = r[mode];
        r[mode].weight *= noiseThreshold;
    }
    
    /*
    // Make a low probability, large variance mode
    // This will be convolved with the observation to introduce a low weight obs gaussian
    if (oldNumGaussians < maxGaussians) {
        oldNumGaussians++;
        MMatrix3 largeCov(get95CF(10*FIELD_DIAGONAL));
        largeCov(2, 2) = get95CF(10*FULL_CIRCLE);
        rCopy[oldNumGaussians-1].mean.pos.setVector(vCART,FIELD_WIDTH/2,FIELD_LENGTH/2);
        rCopy[oldNumGaussians-1].mean.h = HALF_CIRCLE/2;
        rCopy[oldNumGaussians-1].cov    = largeCov;
        rCopy[oldNumGaussians-1].weight = 1E-4;
        rCopy[oldNumGaussians-1].mean.updateVar(&rCopy[oldNumGaussians-1].cov);
    }
    */
    int maxSymmetry = 1;
    if (useSymmetry) maxSymmetry = 2;
    for (mode = 0; mode < oldNumGaussians; mode++) {
        for (int symmetry = 0; symmetry < maxSymmetry; symmetry++) {
            for (int i = 0; i < MAX_MEASUREMENT_DIM; i++) {
                jacX[i] = 0.0;
                jacY[i] = 0.0;
                jacH[i] = 0.0;
                var[i] = 0.0;
                iv[i] = 0.0;
            }
            measDim = 0;

            temp = rCopy[mode];
            
            if (symmetry == 1) {
                temp.mean.pos.setVector(vCART,
                    FIELD_WIDTH - temp.mean.pos.x,
                    FIELD_LENGTH - temp.mean.pos.y);
                temp.mean.h = NormalizeAngle_0_360(temp.mean.h + 180);
            }

            bool skipGoal, useDist;
            //double cx = temp.mean.pos.x;
            double cy = temp.mean.pos.y;

            // Goals
            for (int i = vobMinFixed; i < vobMinFixed + 2; i++) {
                // Don't localise off goal if angular variance is too big, or
                // we're too close (heading and distance accuracy can be bad).
                if (!observedVobs) 
                    cout << "null" <<endl;
                skipGoal = (observedVobs[i].angleVar > get95CF(30));
#ifdef OFFLINE
#ifdef GPS_DEBUG
                if (skipGoal) {
                    cout << __func__ << " vob:" << i << " skipped (observedVobs[i].angleVar > get95CF(30)):"<< observedVobs[i].angleVar  << endl;
                    continue;
                }
#endif                
#endif                 
                
                skipGoal |= (observedVobs[i].d < 70);
#ifdef OFFLINE
#ifdef GPS_DEBUG
                if (skipGoal) {
                    cout << __func__ << " vob:" << i << " skipped observedVobs[i].d < 70 "<< observedVobs[i].d  << endl;
                    continue;
                }
#endif                 
#endif                 
                skipGoal |= (observedVobs[i].d > FIELD_DIAGONAL);
#ifdef OFFLINE
#ifdef GPS_DEBUG
                if (skipGoal) {
                    cout << __func__ << " vob:" << i << " observedVobs[i].d > FIELD_DIAGONAL "<< observedVobs[i].d << "D:" << FIELD_DIAGONAL << endl;
                    continue;
                }
#endif                 
#endif                 

                // Don't localise off goal if we know we're in a position where
                // the goal heading is going to be bad.
                skipGoal |= (i == ownGoal && cy < FIELD_LENGTH * 0.15);
                skipGoal |= (i == targetGoal && cy > FIELD_LENGTH * 0.85);
#ifdef OFFLINE
#ifdef GPS_DEBUG
                if (skipGoal) {
                    cout << __func__ << " vob:" << i << " skipped cy > FIELD_LENGTH * 0.85 || cy < FIELD_LENGTH * 0.15 "<< cy  << endl;
                    continue;
                }
#endif                 
#endif                 

                // Don't localise off goal if we can't fit any more measurements in
                skipGoal |= (measDim >= MAX_MEASUREMENT_DIM);

                if(!goalSkippedSilence && skipGoal)
                    cout << "goal skipped" << endl;
#ifdef OFFLINE
#ifdef GPS_DEBUG
                if (skipGoal) {
                    cout << __func__ << " vob:" << i << " skipped measDim >= MAX_MEASUREMENT_DIM:"<< measDim << endl;
                    continue;
                }
#endif                 
#endif                 
                    
                if (!skipGoal) {
                    view |= 1 << (i - vobMinFixed + 1);
                    
                    useDist = (observedVobs[i].var < get95CF(100));
                    useDist &= (observedVobs[i].d < 220.0);
                    useDist &= (measDim < MAX_MEASUREMENT_DIM - 1);

#ifdef GPS_VISION_UPDATE_DEBUG                                        
    OSYSDEBUG(("%s : Goal Processing : useDist %d, angle %f, distance %f\n",
                __func__,useDist,observedVobs[i].h,observedVobs[i].d));
#endif                             
                    addMeasurement(&temp, observedVobs[i], jacX, jacY, jacH,
                                   iv, var, measDim, useDist);
                                 
                    if (useDist) measDim += 2;
                    else measDim++;
                
                }
            }

            //for all beacons where your estimate of the distance to the
            //beacon has a 95% confidence interval of less than 100cm
            //localise using that beacon
            for (int i = vobMinFixed + 2; i <= vobMaxFixed; i++) {
                if (observedVobs[i].angleVar < get95CF(30)) {                
                    view |= 1 << (i - vobMinFixed + 1);               

                    useDist = (observedVobs[i].var < get95CF(100));
                    useDist &= (observedVobs[i].d < 450);
                    useDist &= (measDim < MAX_MEASUREMENT_DIM - 1);
                
                    
                
                    if(measDim < MAX_MEASUREMENT_DIM) {
#ifdef GPS_VISION_UPDATE_DEBUG                        
    OSYSDEBUG(("%s : Beacon Processing : useDist %d, angle %f, distance %f\n",
                __func__,useDist,observedVobs[i].h,observedVobs[i].d));
#endif
                    
                        addMeasurement(&temp, observedVobs[i], jacX, jacY, jacH,
                                       iv, var, measDim, useDist);
                                   
                        if (useDist) measDim += 2;
                        else measDim++;
                    }
                }
            }
        
            switch(measDim) {
            case 0: break;
            case 1: kalmanUpdate<1>(&temp, jacX, jacY, jacH, iv, var); break;
            case 2: kalmanUpdate<2>(&temp, jacX, jacY, jacH, iv, var); break;
            case 3: kalmanUpdate<3>(&temp, jacX, jacY, jacH, iv, var); break;
            case 4: kalmanUpdate<4>(&temp, jacX, jacY, jacH, iv, var); break;
            case 5: kalmanUpdate<5>(&temp, jacX, jacY, jacH, iv, var); break;
            case 6: kalmanUpdate<6>(&temp, jacX, jacY, jacH, iv, var); break;
            case 7: kalmanUpdate<7>(&temp, jacX, jacY, jacH, iv, var); break;
            case 8: kalmanUpdate<8>(&temp, jacX, jacY, jacH, iv, var); break;
            case 9: kalmanUpdate<9>(&temp, jacX, jacY, jacH, iv, var); break;
            }

            if (symmetry == 0) {
                // Insert the updated mode into the new distribution
                insertGaussian(temp);
                lastWeight = temp.weight;
            } else {
                // For the symmetrically opposite mode, only insert if it
                // matched up much better than the normal update, and matched
                // up well in general.
                double weightSymmetric = temp.weight / lastWeight;
                double weightScale = temp.weight / rCopy[mode].weight;

                if (weightSymmetric > 100 && weightScale > 0.9 && measDim > 1) {
                    DER_DEBUG (
                        cout << "Introduced symmetric mode\n";
                        cout << "Weight compared to normal mode: " << weightSymmetric;
                        cout << "\nWeight compared to original: " << weightScale;
                        cout << "\nUpdate dimensions: " << measDim << "\n";
                        sendCPlane = true;
                    );
                    temp.weight = lastWeight * symmetryWeight;
                    insertGaussian(temp);
                }
            } 
        }
    }

    formDistribution();


    // ball update has to be after self location update
    // since that info is used
    newball->growVariance(4);
    newball->infoKalman(observedVobs[vobBall], !isHeadMoving);

    if (observedVobs[vobBall].cf > 0) {
        view |= 1;
    }

#ifdef VISUAL_TRACK_FRIENDS
    // perform prediction update on each friend structure
    for(int i = 0; i < NUM_FRIENDS_VIS; i++)
        friends[i]->growVariance(OPPONENT_VAR_GROWTH);

    // perform correction update for each friend seen
    for(int i = vobMinFriend; i <= vobMaxFriend; i++) {
        if(observedVobs[i].var < get95CF(150))
            //applyKalmanUpdateHybridBlah(friends, NUM_FRIENDS_VIS, observedVobs[i]);
    }
#endif // VISUAL_TRACK_FRIENDS

    if (doGPSOpponentUpdate) {
        // perform prediction update on each opponent structure
        for(int i = 0; i < NUM_OPPONENTS; i++)
            opponents[i]->growVariance(OPPONENT_VAR_GROWTH);

        for(int i = vobMinOpponent; i <= vobMaxOpponent; i++) {
            if(observedVobs[i].var < get95CF(150))
                applyKalmanUpdateHybrid(opponents, NUM_OPPONENTS, observedVobs[i]);
        }
    }
    prevMotionForward = motionForward;
    prevMotionLeft = motionLeft;
    prevMotionTurn = motionTurn;
    prevMotionPWM = motionPWM;
    prevMotionFCounter = motionFCounter;
    
    motionPWM = 0;
    motionFCounter = 0;
    motionForward = 0.0;
    motionLeft = 0.0;
    motionTurn = 0.0;
    
}

// This gets called by ResultCamera in Vision.cc. It uses the field line/edge
// methods from VisualCortex to do a measurement correction on the kalman
// filter.
#ifndef NEW_LINE_UPDATE
//void GPS::GPSEdgeUpdate(VisualCortex *vc, CommonSense *sensors) {
void GPS::GPSEdgeUpdate(VisualCortex *vc, double headSpeed) {
    static const int noLinePointIncreaseVariance = 20;

    MVec3 oldRobot, newRobot, grad;
    MVec2 projEdgeCentre;
    double newMatch;
    
    useLineData = false;
    
#ifdef OFFLINE
    //Clear the vectors containing previous edge update
    GA_robotsPos.clear();
    GA_gradient.clear();
    GA_match.clear();
    GA_LR.clear();
    GA_Duplicates.clear();
#endif
    if (isPenalised || !doGPSVisionUpdate)
        return;
    
    // Don't do line localisation if we don't have any points
    if (vc->numEdgePnts == 0) {
#ifdef OFFLINE    
        if (outputStream != NULL)
            *outputStream <<"NO EDGE POINTS:" <<vc->numEdgePnts<<endl;
#endif            
        return;
    }
    
    //If the number of line points is not enough, it probably random error,
    //Do not edge update
    if (vc->numEdgeFieldPnts < 2 || vc->numEdgeLinePnts < 8) { //25//4 before
#ifdef OFFLINE    
        if (outputStream != NULL)
            *outputStream <<__func__ << " Not enough line points " <<vc->numEdgeLinePnts<<endl;
#endif
        return;
    }
    
#ifdef OFFLINE    
    if (outputStream != NULL)
        *outputStream <<__func__ << " No Line points " <<vc->numEdgeLinePnts<<endl;
    if (outputStream != NULL)
        *outputStream <<__func__ << " No Field points " <<vc->numEdgeFieldPnts<<endl;
#endif            

    oldRobot(0, 0) = r[0].mean.pos.x;
    oldRobot(1, 0) = r[0].mean.pos.y;
    oldRobot(2, 0) = DEG2RAD(r[0].mean.h);

    newMatch = gradientAscent(oldRobot, newRobot, vc);
    // Don't do the update when we get a perfect match with no movement,
    // since this is caused by only seeing field points (eg. when looking
    // down), which match to anywhere in the field area.
    if (newMatch == 1.0 && newRobot.equals(oldRobot)) 
    {
#ifdef OFFLINE
#ifdef GPS_DEBUG
        cout << __func__ << " perfect match, not doing edge update." <<endl;
#endif 
#endif       
        return;
    }

    vc->calcEdgeCentre(projEdgeCentre);
    
    int possibles, bestSymmetry; 
    double weights[MAX_EDGE_DUPLICITY], bestWeight;
    double posX[MAX_EDGE_DUPLICITY], varX[MAX_EDGE_DUPLICITY];
    double posY[MAX_EDGE_DUPLICITY], varY[MAX_EDGE_DUPLICITY];
    double posH[MAX_EDGE_DUPLICITY], varH[MAX_EDGE_DUPLICITY];
    MVec3 pos[MAX_EDGE_DUPLICITY], posVar[MAX_EDGE_DUPLICITY];

    possibles = calcAlternateLocations(newRobot, projEdgeCentre, pos, posVar);
    
    // If the head is moving quickly, then the match value probably won't be
    // as good (due to image distortion), so lower the minimum match
    // threshold. However, because of the distortion the final update accuracy
    // won't be as good either, so increase the update variance.
    /* With sensors read
    double varScale = 1.0 + SQUARE(sensors->getHeadSpeed() * 50.0);
    double minMatch = 0.5 + 0.3 / (1.0 + sensors->getHeadSpeed() * 50);
    */
    // With headspeed
    double varScale = 1.0 + SQUARE(headSpeed * 50.0);
    double minMatch = 0.5 + 0.3 / (1.0 + headSpeed * 50.0);
    
    //Adding variance using sum of weight line points
        
    if (vc->numEdgeLinePnts < noLinePointIncreaseVariance)
        varScale += 0.1 * (noLinePointIncreaseVariance - vc->numEdgeLinePnts);
    
        
#ifdef OFFLINE
    if (outputStream != NULL) {
        *outputStream << "Edge update with " << vc->numEdgePnts << " points.\n";
        *outputStream << "Match " << newMatch << ", minimum match " << minMatch;
        *outputStream << " headspeed:" << headSpeed;
        *outputStream << ", with " << possibles << " symmetries.\n";

        *outputStream << "Original update at (";
        *outputStream << newRobot(0, 0) << ", ";
        *outputStream << newRobot(1, 0) << ", ";
        *outputStream << RAD2DEG(newRobot(2, 0)) << ")\n";

        *outputStream << "Edge centre at (";
        *outputStream << projEdgeCentre(0, 0) << ", ";
        *outputStream << projEdgeCentre(1, 0) << ")\n";
        *outputStream << "varScale:" << varScale << endl;
    }
    GA_Duplicates.clear();
#endif
        
    // Copy the vector array to individual component arrays
    int n = 0;
    bestSymmetry = 0; bestWeight = -1.0;
    double varScaleEach = varScale;
    for (int i = 0; i < possibles; i++) {
        weights[n] = vc->calcRobotMatch(pos[i], grad);
#ifdef OFFLINE
        GA_Duplicates.push_back(pos[i]);
        if (outputStream != NULL) {
            *outputStream << "Possibles " << n << ": mean (";
            *outputStream << pos[i](0, 0) << ", " << pos[i](1, 0) << ", " << RAD2DEG(pos[i](2, 0));
            *outputStream << ")    variance (";
            *outputStream << posVar[i](0, 0) << ", " << posVar[i](1, 0) << ", " << posVar[i](2, 0);
            *outputStream << ")    weight " << weights[n] << "\n";
        }        
        vc->outputStream = NULL;        
#endif     
        
        //Punish updates that were off-field
        //double testx = pos[i](0, 0);
        //double testy = pos[i](1, 0);
        //FieldClipping(testx, testy);
        //double offFieldPenalty = SQUARE(testx - pos[i](0, 0))
        //                       + SQUARE(testy - pos[i](1, 0));
        //weights[n] *= exp(-0.02 * offFieldPenalty);

        // Only use this observation if point match is reasonable
        if (weights[n] >= minMatch) {
            varScaleEach = varScale;
            
            //Adding variance using match value
            //varScaleEach += 1.0 - weights[n];
            varScaleEach += 0.8/(weights[n]*weights[n]);
            
            posX[n] = pos[i](0, 0); varX[n] = posVar[i](0, 0) * varScaleEach;
            posY[n] = pos[i](1, 0); varY[n] = posVar[i](1, 0) * varScaleEach;
            posH[n] = pos[i](2, 0); varH[n] = posVar[i](2, 0) * varScaleEach;
            posH[n] = RAD2DEG(posH[n]);
            varH[n] = RAD2DEG(RAD2DEG(varH[n]));
        
            if (weights[n] > bestWeight) {
                bestSymmetry = i;
                bestWeight = weights[n];
            }

#ifdef OFFLINE                        
            if (outputStream != NULL) {
                *outputStream << "Update " << n << ": mean (";
                *outputStream << posX[n] << ", " << posY[n] << ", " << posH[n];
                *outputStream << ")    variance (";
                *outputStream << varX[n] << ", " << varY[n] << ", " << varH[n];
                *outputStream << ")    weight " << weights[n] << " varScaleEach:" << varScaleEach << "\n";
            }
#endif          
            ++n;
        }
    }

    // Apply the edge updates to our self position.
    if (n > 0) {
        GPSDirectPositionUpdate(n, posX, posY, posH, varX, varY, varH, weights);
        useLineData = true;
    
        // This sets the world model display to show the edge points positions
        // with respect to the best symmetry
        vc->calcRobotMatch(pos[bestSymmetry], grad);
        //directx = pos[bestSymmetry](0, 0);
        //directy = pos[bestSymmetry](1, 0);
        //directh = RAD2DEG(pos[bestSymmetry](2, 0));

    } else {
        vc->calcRobotMatch(newRobot, grad);
        directx = -1;
        directy = -1;
        directh = -1;
    }
}
#endif // NEW_LINE_UPDATE

// Updates the GPS variances based on the confidence of visual cortex that it
// is detecting enough information. If confidence is between zero and
// MAX_VISUAL_INFO then vision has not recently recieved a lot of information
// and variances should widen. In particular this implements a sort of blind
// detection when the confidence drops low.
void GPS::GPSVisualConfidenceUpdate(int confidence) { 
    
    if (isPenalised || !doGPSVisionUpdate)
        return;

    if (confidence >= MAX_VISUAL_INFO/2)
        return; 
    
    double var = (MAX_VISUAL_INFO/2 - confidence) * 0.25;        

#ifdef GPS_VISUAL_CONFIDENCE_DEBUG    
    OSYSDEBUG(("%s : confidence : %d, x var : %.2f, y var : %.2f, h var : %.2f\n",
                __func__,confidence,r[0].cov(0,0),r[0].cov(1,1),r[0].cov(2,2)));
#endif
    
    for (int i = 0; i < numGaussians; i++) {
        // update variance
        r[i].cov(0,0) += var;
        r[i].cov(1,1) += var;
        r[i].cov(2,2) += var * 0.5;
        r[i].mean.updateVar(&r[i].cov);
    }    
}


// A very expensive operation (will cause dropped frames), this was put in for
// the localisation challenge player. Basically performs a gradient ascent for
// many points on the field. And sets the gaussian mode array to the top
// matching end locations. This should only really be used if there is a
// decent amount of edge point data available, eg. if stationary mapping has
// been running for a while.

void GPS::GPSGlobalMaxEdgeSet(VisualCortex *vc) {
    MVec3 startPos, endPos;
    MMatrix3 newVar(500.0);
    SingleGaussian temp;
    
    newVar(2, 2) = 300.0;
    numGaussians = 0;
    
    for (int x = 1; x < 4; x++) {
        for (int y = 1; y < 4; y++) {
            for (int h = 0; h < 8; h++) {
                startPos(0, 0) = x * (FIELD_WIDTH / 4.0);
                startPos(1, 0) = y * (FIELD_LENGTH / 4.0);
                startPos(2, 0) = DEG2RAD(h * 45.0);
                
                temp.weight = gradientAscent(startPos, endPos, vc);
                temp.mean.pos.setVector(vCART, endPos(0, 0), endPos(1, 0));
                temp.mean.h = NormalizeAngle_0_360(RAD2DEG(endPos(2, 0)));
                temp.cov = newVar;

                insertGaussian(temp);
            }   
        }
    }

    formDistribution();
    GPSOutputSelfData();    
}

// Perform a gradient ascent to highest match value

#ifdef GRADIENT_ASCENT_VER_1
static const int maxAscentSteps = 100;
double GPS::gradientAscent(MVec3 &originalRobot, MVec3 &newRobot, VisualCortex *vc) {
    MVec3 oldRobot, oldGradient, newGradient, update;
    double oldMatch, newMatch, angleScale, learnRate;
    //bool output;

    angleScale = 0.0005;
    learnRate = 10.0;//2000.0;
    
    oldMatch = 0.0;
    oldRobot = originalRobot;
    newMatch = oldMatch;
    newRobot = oldRobot;
    newGradient(0, 0) = 0.0;
    newGradient(1, 0) = 0.0;
    newGradient(2, 0) = 0.0;
    int i;
#ifdef OFFLINE
    stringstream temp;
#endif
    for (i = 0; i < maxAscentSteps && learnRate > 1.0; i++) {    
        // Calculate match and match gradient at current point
        newMatch = vc->calcRobotMatch(newRobot, newGradient);
#ifdef OFFLINE    
        GA_robotsPos.push_back(newRobot);
        GA_gradient.push_back(newGradient);
        GA_match.push_back(newMatch);
        GA_LR.push_back(learnRate);
        if (outputStream != NULL) {
            temp << __func__ << " ["<<i<<"] "<< std::setw(8) << newRobot(0, 0) << "\t" 
            << std::setw(8) << newRobot(1, 0) << "\t" 
            << std::setw(8) << RAD2DEG(newRobot(2,0));
            temp << "\t" << setw(8) << newMatch << "\t" 
            << std::setw(8) << oldMatch << "\t" 
            << std::setw(8) << learnRate << "\t" 
            << std::setw(8) << newGradient(0,0) << "\t" 
            << std::setw(8) << newGradient(1,0) <<"\t" 
            << std::setw(8) << newGradient(2,0) << "\n";
        }
#endif //OFFLINE
        //newMatch < EPSILON cover the case if the match return near to 0 
        if (newMatch < oldMatch || newMatch < EPSILON) {
            // If the new match value is worse than the old match value, then we
            // must have overshot a peak. So don't use the new position &
            // decrease the learn rate.
            learnRate *= 0.5; 

            newMatch = oldMatch; //ANDREW: this equation is unnecessary ?
            newRobot = oldRobot;
            newGradient = oldGradient;
        } else {
            // Alternatively, if the match is better, we may be moving too
            // slowly,and so gradually accelerate by increasing learn rate.
            oldMatch = newMatch;
            
            //If the new gradient is 0, means the dog not moving and the match value is 1.
            //Stop iterating.
            if (ABS(newGradient(0,0)) < 0.01*EPSILON && 
                ABS(newGradient(1,0)) < 0.01*EPSILON && 
                ABS(newGradient(2,0)) < 0.01*EPSILON && 
                (1.0 - newMatch) < EPSILON) 
                break;
            learnRate *= 1.5;            
            oldRobot = newRobot;
            oldGradient = newGradient;
        }
        
        /*
         *Just almost get into exit condition of the gradient ascent
         *Don't do update on newRobot position, it could update to less
         *match or wrong direction.
         */
        if (i == maxAscentSteps - 1 || learnRate < 1.0 + EPSILON) {
            break;
        }
        
        // Move in gradient direction
        update = newGradient;
        update(2, 0) *= angleScale;
        update *= learnRate;

        newRobot += update;
    }
#ifdef OFFLINE  
    GA_robotsPos.push_back(newRobot);
    GA_gradient.push_back(newGradient);
    GA_match.push_back(oldMatch);
    GA_LR.push_back(learnRate);

    if (outputStream != NULL)  {
        //cout << "gradient ascent took " << i << " steps" << endl;
        //if (i>100){
            *outputStream << temp.str() <<endl;
        //}
        *outputStream << __func__ << " Total number of steps:" << i << "\t EdgePoints" << vc->numEdgePnts << "\t match:" << oldMatch <<endl;
    }
#ifdef GPS_GA_PLOTDATA
    cout << i << "\t" << vc->numEdgePnts << "\t" << oldMatch <<endl;    
#endif //GPS_GA_PLOTDATA
#endif //OFFLINE   
    return oldMatch;
}

#endif //GRADIENT_ASCENT_VER_1

#ifdef OFFLINE
#ifdef TILT_CALIBRATION
static MVec3 *spareRobot;
static VisualCortex * viscort;

double testTilt(MMatrix<double, 1, 5> &pt, void *additional) {
    const bool debug = false;
    if (debug) {
            cout << "eval at: ";
            pt.printOut();
            cout << " is: ";
    }
    
    MVec3 newGradient;
    
    double result;
    MVec3 oldRobot = *((MVec3*) additional);
    if ((pt(0,0) < -5 || pt(0,0) > 35) || // body + neck tilt
        (pt(0,1) < 101 || pt(0,1) > 103) || // neck base height
        (pt(0,2) < oldRobot(0,0) - 3 || pt(0,2) > oldRobot(0,0) + 3) || //xpos
        (pt(0,3) < oldRobot(1,0) - 3 || pt(0,3) > oldRobot(1,0) + 3) || //ypos 
        (pt(0,4) < RAD2DEG(oldRobot(2,0)) - 5 || pt(0,4) > RAD2DEG(oldRobot(2,0)) + 5)) {   //heading
        return 1000;// stupid values - don't go here
    } else {
        viscort->degTilt = pt(0,0);
        viscort->degBodyTilt = 0.0;
        viscort->neckBaseHeight = pt(0,1);
        (*spareRobot)(0,0) = pt(0,2);
        (*spareRobot)(1,0) = pt(0,3);
        (*spareRobot)(2,0) = DEG2RAD(pt(0,4));

        result = -viscort->calcRobotMatch(*spareRobot, newGradient);
    }
    if (debug) {
            cout << result << "\n";
    }
    return result;
}
#endif //TILT_CALIBRATION

#endif //OFFLINE

#ifdef GRADIENT_ASCENT_VER_2
/* Gradient ascent using variable momentum using sum of gradients*/
static const int maxAscentSteps = 50;
double GPS::gradientAscent(MVec3 &originalRobot, MVec3 &newRobot, VisualCortex *vc) {
    
    MVec3 oldRobot, oldGradient, newGradient, update;
    double oldMatch, newMatch, angleScale, learnRate;
    
    angleScale = 0.00005;//0.0002;//0.0005;
    learnRate = 20.0; //1000.0;
    
    oldMatch = 0.0;
    oldRobot = originalRobot;
    newMatch = oldMatch;
    newRobot = oldRobot;
    newGradient(0, 0) = 0.0;
    newGradient(1, 0) = 0.0;
    newGradient(2, 0) = 0.0;
    update(0, 0) = 0.0;
    update(1, 0) = 0.0;
    update(2, 0) = 0.0;
    int i;
    
#ifdef OFFLINE
    stringstream temp;
    GA_robotsPos.clear();
    GA_gradient.clear();
    GA_match.clear();
    GA_LR.clear();    

#ifdef TILT_CALIBRATION 
    MMatrix<double, 1, 11> output;  
    // *** Used when fitting joint slop
    spareRobot = &newRobot;
    double bodytilt = vc->degBodyTilt;
    double necktilt = vc->degTilt;
    double neckBaseHeight = vc->neckBaseHeight;
    viscort = vc;

    MMatrix<double, 1, 5> initial;
    simplexMin<5> simp;
    
    initial.reset();
    output.reset();
    output(0,0) = initial(0,0)= vc->degBodyTilt + vc->degTilt; //calc the degTilt as well
    output(0,1) = initial(0,1)= vc->neckBaseHeight;
    output(0,2) = initial(0,2)= newRobot(0,0);
    output(0,3) = initial(0,3)=newRobot(1,0);
    output(0,4) = initial(0,4)=RAD2DEG(newRobot(2,0));
    output(0,5) = vc->degBodyTilt;
    output(0,6) = vc->degPan;
    output(0,7) = vc->degTilt;
    output(0,8) = vc->degCrane;
    output(0,9) = initial(0,0);
    output(0,10) = 0.0; // tilt differences
    /*
    initial(0,5)=vc->degPan;
    initial(0,6)=vc->degCrane;
    */
    if (tiltCalibrationOutputStream != NULL)
    {
        output.printOut(tiltCalibrationOutputStream);
    }
    else
    {
        cout << "Prior body-tilt"<< endl;     
        output.printOut();
    }
    initial = simp.min(testTilt, initial, (void*)&oldRobot);
    //Copy the last result to output
    output(0,0) = initial(0,0);
    output(0,1) = initial(0,1);
    output(0,2) = initial(0,2);
    output(0,3) = initial(0,3);
    output(0,4) = initial(0,4);
    output(0,10) = bodytilt + necktilt - initial(0,0); // tilt differences
    
    if (tiltCalibrationOutputStream != NULL)
    {
        output.printOut(tiltCalibrationOutputStream);
    }
    else
    {
        cout << "height-tilt minimization result: " << endl;
        output.printOut();
    }
    
    //Get the best position
    newRobot(0,0) = initial(0,2);
    newRobot(1,0) = initial(0,3);
    newRobot(2,0) = DEG2RAD(initial(0,4));
    GA_robotsPos.push_back(newRobot);

    //Get the best body tilt and neck base
    vc->degTilt = initial(0,0);
    vc->degBodyTilt = 0.0;
    vc->neckBaseHeight = initial(0,1);
    
    //Calculate the points projected and get the match result
    newMatch = vc->calcRobotMatch(newRobot, newGradient);
    GA_gradient.push_back(newGradient);
    GA_match.push_back(newMatch);
    GA_LR.push_back(learnRate);
    
    //Restore position
    newRobot = oldRobot;
    //Restore the body tilt & neck base height
    vc->degTilt = necktilt;
    vc->degBodyTilt = bodytilt;
    vc->neckBaseHeight = neckBaseHeight;
    
    return 1.0;
#endif //TILT_CALIBRATION
#endif //OFFLINE

    newMatch = vc->calcRobotMatch(newRobot, newGradient);
    oldMatch = newMatch;
    newGradient(2, 0) *= angleScale;
    newGradient *= learnRate;
    newGradient(0,0) = CLIP(newGradient(0,0),5.0);
    newGradient(1,0) = CLIP(newGradient(1,0),5.0);
    newGradient(2,0) = CLIP(newGradient(2,0),DEG2RAD(5.0));
    update = newGradient;    
    
    for (i = 0; i < maxAscentSteps; i++) {
        
#ifdef OFFLINE    
        GA_robotsPos.push_back(newRobot);
        GA_gradient.push_back(newGradient);
        GA_match.push_back(newMatch);
        GA_LR.push_back(learnRate);
        if (outputStream != NULL) {
            temp << __func__ << " ["<<i<<"] "
            << std::setw(8) << newRobot(0, 0) << "\t" 
            << std::setw(8) << newRobot(1, 0) << "\t" 
            << std::setw(8) << RAD2DEG(newRobot(2,0));
            temp << "\t" << setw(8) << newMatch << "\t" 
            << std::setw(8) << oldMatch << "\t" 
            << std::setw(8) << learnRate << "\t" 
            << std::setw(8) << newGradient(0,0) << "\t" 
            << std::setw(8) << newGradient(1,0) <<"\t" 
            << std::setw(8) << newGradient(2,0) << "\t"
            << std::setw(8) << update(0,0) << "\t" 
            << std::setw(8) << update(1,0) <<"\t" 
            << std::setw(8) << update(2,0) << "\t"
            
            << std::setw(8) << oldRobot(0, 0) << "\t" 
            << std::setw(8) << oldRobot(1, 0) << "\t" 
            << std::setw(8) << RAD2DEG(oldRobot(2,0)) << "\n";
        }
#endif //OFFLINE
           
        newRobot = oldRobot;
        newRobot += update;
 
        // Calculate match and match gradient at current point
        newMatch = vc->calcRobotMatch(newRobot, newGradient);
        
        if ((ABS(update(0,0)) < 0.1 && ABS(update(1,0)) < 0.1 && ABS(update(2,0)) < 0.1) || ((1.0 - newMatch) < EPSILON)) 
        {
            oldMatch = newMatch;
            break;
        }
         
        if (newMatch+EPSILON < oldMatch) {
            /*
            newGradient(2, 0) *= angleScale;    
            //Another way is ignore the new gradient by just update *0.5 
            //if update and new gradient pointing to the same amount, It goes to nowhere, break.
            if (ABS(newGradient(0,0) - update(0,0)) < EPSILON && ABS(newGradient(1,0) - update(1,0)) < EPSILON && ABS(newGradient(2,0) - update(2,0)) < EPSILON)
            {
                //half the update ? it can be problem if the next gradient is the same vector
                // It will continue until max steps
                break;
            }
            else 
            {
                //Add the some of the new gradient
                update += newGradient;
            }
            */
            update *= 0.5;            
        } else {
            // Alternatively, if the match is better, we may be moving too
            // slowly,and so gradually accelerate by increasing learn rate.
            oldMatch = newMatch;
            
            //If the new gradient is 0, means the dog not moving and the match value is 1.
            //Stop iterating.
            oldRobot = newRobot;
             
            newGradient *= learnRate;
            newGradient(2, 0) *= angleScale;
            
            newGradient(0,0) = CLIP(newGradient(0,0),15.0);
            newGradient(1,0) = CLIP(newGradient(1,0),15.0);
            newGradient(2,0) = CLIP(newGradient(2,0),DEG2RAD(15.0));
            
            update += newGradient;
        }
    
        update(0,0) = CLIP(update(0,0),15.0);
        update(1,0) = CLIP(update(1,0),15.0);
        update(2,0) = CLIP(update(2,0),DEG2RAD(5.0));
        
    }
#ifdef OFFLINE  
    GA_robotsPos.push_back(newRobot);
    GA_gradient.push_back(newGradient);
    GA_match.push_back(oldMatch);
    GA_LR.push_back(learnRate);
    
    if (outputStream != NULL)  {
        temp << __func__ << " ["<<i<<"] "
        << std::setw(8) << newRobot(0, 0) << "\t" 
        << std::setw(8) << newRobot(1, 0) << "\t" 
        << std::setw(8) << RAD2DEG(newRobot(2,0));
        temp << "\t" << setw(8) << newMatch << "\t" 
        << std::setw(8) << oldMatch << "\t" 
        << std::setw(8) << learnRate << "\t" 
        << std::setw(8) << newGradient(0,0) << "\t" 
        << std::setw(8) << newGradient(1,0) <<"\t" 
        << std::setw(8) << newGradient(2,0) << "\t"
        << std::setw(8) << update(0,0) << "\t" 
        << std::setw(8) << update(1,0) <<"\t" 
        << std::setw(8) << update(2,0) << "\t"
        
        << std::setw(8) << oldRobot(0, 0) << "\t" 
        << std::setw(8) << oldRobot(1, 0) << "\t" 
        << std::setw(8) << RAD2DEG(oldRobot(2,0)) << "\n";
        
        //cout << "gradient ascent took " << i << " steps" << endl;
        //if (i>100){
            *outputStream << temp.str() <<endl;
        //}
        *outputStream << __func__ << " Total number of steps:" << i << "\t EdgePoints" << vc->numEdgePnts << "\t match:" << oldMatch <<endl;
    }
#ifdef GPS_GA_PLOTDATA
    cout << i << "\t" << vc->numEdgePnts << "\t" << oldMatch <<endl;    
#endif
#endif //OFFLINE   
    return oldMatch;
}
#endif //GRADIENT_ASCENT_VER_2




// Derrick: The following commented code is an alternate form of field line
// localisation I was developing until I fixed a serious bug in the old line
// localisation that made it work much better. Currently it doesn't seem to
// work, but I included it here for future reference.

#ifdef NEW_LINE_UPDATE
// Performs correction update according to field line matching. For each
// gaussian in the distribution, it generates a set of 27 sample points, based
// on that gaussian's mean and variance. At each sample point, the likelyhood
// of the field lines/edges seen matching the position is calculated.
// A new gaussian(s) is then generated from the sample points, and applied as a
// direct kalman update.

void GPS::GPSEdgeUpdate(VisualCortex *vc, double headSpeed, ostream *outputf = NULL) {
    int i, oldNumGaussians, mode, obsMode, numObs, dimCount;
    double jacX[3], jacY[3], jacH[3], upVar[3], iv[3];
    double oldWeight, baseVar, infoVal;
    MVec3 samplePoint, grad;
    SingleGaussian temp;
    //cout <<  __func__ << " output:" << output << endl;
    output = true;
    if (output)
        cout << __func__ << " start func" <<endl;
    if (isPenalised || !doGPSVisionUpdate)
        return;
    
    //output = false;
    if (vc->numEdgePnts < 1) return;

    infoVal = (1.0 - exp(-0.005 * SQUARE(vc->numEdgePnts)));
    infoVal /= (1.0 + 20.0 * SQUARE(headSpeed));
    baseVar = 1.0 / infoVal;

    if (infoVal < 0.5) return;

    DER_DEBUG(
        edgeDebugCounter++;
        if (edgeDebugCounter > 50) {
            output = true;
            edgeDebugCounter = 0;
        }
    );
    
    // Work from a backup of the old distribution
    for (i = 0; i < numGaussians; i++) {
        rCopy[i] = r[i];
        r[i].weight *= noiseThreshold;
    }
    
    oldNumGaussians = numGaussians;

    if (output) {
        cout << "GPS::GPSEdgeUpdate localising with " << vc->numEdgePnts;
        cout << " points, " << oldNumGaussians << " gaussians.\n";
        cout << "Information value is: " << infoVal << "\n";
        cout << "Minimum variance is: " << baseVar << "\n";
    }

    for (mode = 0; mode < oldNumGaussians; mode++) {
        oldWeight = rCopy[mode].weight;

        // Calculate these convenience values
        curMean(0, 0) = rCopy[mode].mean.pos.x;
        curMean(1, 0) = rCopy[mode].mean.pos.y;
        curMean(2, 0) = DEG2RAD(rCopy[mode].mean.h);
        curDev(0, 0) = sqrt(rCopy[mode].cov(0, 0));
        curDev(1, 0) = sqrt(rCopy[mode].cov(1, 1));
        curDev(2, 0) = DEG2RAD(sqrt(rCopy[mode].cov(2, 2)));
        
        // Sample the current gaussian distribution
        if (output) cout << "Sample matrix is:\n";
        for (i = 0; i < 27; i++) {
            calcSamplePoint(i, samplePoint);                        
            samples[i] = vc->calcRobotMatch(samplePoint, grad);
            if (output) {
                cout << samples[i] << "  ";
                if ((i % 9) == 8) cout << "\n";
            }            
        }
        
        // Create a set of observation gaussians, based on the sampling
        numObs = reformGaussian(output);

        if (output) {
            cout << "Gaussian " << mode << " reformed into ";
            cout << numObs << " observation(s)\n";
        }
        
        // Kalman update the current gaussian with every observation mode.
        for (obsMode = 0; obsMode < numObs; obsMode++) {
            dimCount = 0;

            vc->calcRobotMatch(mean[obsMode], grad);
            
            if (output) {
                cout << "Observation " << obsMode << " has prob ";
                cout << prob[obsMode] << "\n";
                cout << "Mean:\n";
                mean[obsMode].printOut();
                cout << "Variance:\n";
                var[obsMode].printOut();
            }
        
            // Create the jacobian and innovation vector for the update
            for (i = 0; i < 3; i++) {
                if(var[obsMode](i, 0) < VERY_LARGE_INT / 2) {
                    upVar[dimCount] = var[obsMode](i, 0);
                    iv[dimCount] = mean[obsMode](i, 0) - curMean(i, 0);
                    
                    jacX[dimCount] = 0;
                    jacY[dimCount] = 0;
                    jacH[dimCount] = 0;
                    
                    if (i == 0)
                        jacX[dimCount] = 1;
                    else if (i == 1)
                        jacY[dimCount] = 1;
                    else if (i == 2) {
                        jacH[dimCount] = 1;
                                        
                        // If this dimension is the heading, have to convert back
                        // to degrees and normalise innovation vector
                        iv[dimCount] = RAD2DEG(iv[dimCount]);
                        iv[dimCount] = NormalizeAngle_180(iv[dimCount]);
                        
                        // Need to do RAD2DEG twice because upVar is a variance
                        upVar[dimCount] = RAD2DEG(RAD2DEG(upVar[dimCount]));
                    }

                    upVar[dimCount] += baseVar;
        
                    dimCount++;
                }
            }
            
            temp = rCopy[mode];
            
            if (prob[obsMode] > 0.6) {
                switch(dimCount) {
                case 1: kalmanUpdate<1>(&temp, jacX, jacY, jacH, iv, upVar); break;
                case 2: kalmanUpdate<2>(&temp, jacX, jacY, jacH, iv, upVar); break;
                case 3: kalmanUpdate<3>(&temp, jacX, jacY, jacH, iv, upVar); break;
                }
            }
                        
            // Use peak match value instead of kalman weight scaler to
            // determine new weight
            temp.weight = oldWeight * prob[obsMode];
            
            // Insert the corrected mode into the new distribution
            insertGaussian(temp);
        }
    }
    
    formDistribution();
}

// From the sampling of the current gaussian, this creates a set of observation
// gaussians. There is one observation for every local maximum in the sampling
// array. The number of observation gaussians is returned. The mean of each
// observation is put in the x, y, h arrays, variances are put in the xV, yV, hV
// arrays, and the weight scaler for each observation is put in the prob array.

int GPS::reformGaussian(bool output) {
    int maxIndices[NUM_SAMPLES], i, j, numObs, numMax;
    int numNeighbours, neighbours[2];
    double logRatio, peakRatio, sideRatio;

    // Find all local maximums in the sampling array
    numMax = findSampleMaximums(maxIndices);
    
    // For each maximum, calculate a mean and variance
    for (i = 0; i < numMax; i++) {
        if (output) cout << "Local max at index " << maxIndices[i] << "\n";

        calcSamplePoint(maxIndices[i], mean[i]);
        prob[i] = samples[maxIndices[i]];

        for (j = 0; j < 3; j++) {
            numNeighbours = calcNeighbourIndices(j, maxIndices[i], neighbours);
            // If this local maximum only has one neighbour along the current
            // dimension, then assume that maximum match occurs (along this
            // dimension) at the local max.
            if (numNeighbours == 1) neighbours[1] = neighbours[0];
                
            // Note: this can make the dog crash if there are sample values
            // of zero, so make sure there aren't! A likely canditate for
            // getting zero samples is the INFINITY_GRADIENT_VALUE constant
            // in VisualCortex: if it's zero, try setting it to a small value,
            // eg. 1E-8
            sideRatio = prob[i] / samples[neighbours[1]];
            peakRatio = prob[i] * prob[i];
            peakRatio /= (samples[neighbours[0]] * samples[neighbours[1]]);

            if (output) {
                cout << "Dir " << j << ": peakRatio is " << peakRatio;
                cout << "  sideRatio is " << sideRatio << "\n";
            }

            logRatio = log(peakRatio);
            if (logRatio != 0.0) {
                // How did I get these formulas? If you have three function
                // values f(x1), f(x2), f(x3), then you can fit a gaussian
                // distribution to them as long as x1, x2, x3 are equally
                // spaced. So assume the function values lie on a gaussian with
                // unknown mean and variance and solve simultaneously.
                logRatio = 1.0 / logRatio;
                var[i](j, 0) = curDev(j, 0) * curDev(j, 0) * logRatio;
                logRatio *= log(sideRatio);
                mean[i](j, 0) += curDev(j, 0) * (0.5 - logRatio);

            } else var[i](j, 0) = VERY_LARGE_INT;
        }
    }
    
    // Sometimes there will be two local maximums at same x, y position but one
    // with -1 standard deviation heading and the other at +1 s.d. heading.
    // Would rather merge these two into a centrally located maximum.
    int hInd, xyInd, otherhInd, otherxyInd;
    double totalProb;

    numObs = 0;
    for (i = 0; i < numMax; i++) {
        hInd = maxIndices[i] % 3;
        xyInd = maxIndices[i] / 3;
        otherhInd = maxIndices[i + 1] % 3;
        otherxyInd = maxIndices[i + 1] / 3;
        
        if (i < numMax - 1) {
            // If a +-heading pair exists, average them
            if (xyInd == otherxyInd && hInd == 0 && otherhInd == 2) {
                totalProb = prob[i] + prob[i + 1];
                mean[i] *= prob[i]; mean[i + 1] *= prob[i + 1];
                var[i]  *= prob[i]; var[i + 1]  *= prob[i + 1];

                mean[i + 1] += mean[i];
                var[i + 1] += var[i];
                
                mean[i + 1] /= totalProb;
                var[i + 1] /= totalProb;
                prob[i + 1] = totalProb / 2;                
        
                i++;
            }
        }
        
        // If we have merged any observations, have to move the other
        // observations up in the array
        if (i > numObs) {
            prob[numObs] = prob[i];
            mean[numObs] = mean[i];
            var[numObs] = var[i];
        }
        
        numObs++;
    }
    
    return numObs;
}

// Returns the sample indices of all local maximums in the sample array

int GPS::findSampleMaximums(int *indices) {
    int i, j, k, numNeighbours, neighbours[2], numMax;
    bool isMax;
    
    numMax = 0;
    for (i = 0; i < NUM_SAMPLES; i++) {
        isMax = true;
    
        // To be a local maximum, the sample value has to be greater than the
        // values of its neighbours. Check neighbours in all three directions.
        for (j = 0; j < 3; j++) {
            numNeighbours = calcNeighbourIndices(j, i, neighbours);
            
            if (numNeighbours > 1) {
                for (k = 0; k < numNeighbours; k++)
                    isMax = isMax && (samples[i] >= samples[neighbours[k]]);
                
            // If neighbouring sample has the same match value, only the sample
            // closer to the centre will be accepted as a local maximum
            } else isMax = isMax && (samples[i] > samples[neighbours[0]]);
        }
        
        if (isMax)
            indices[numMax++] = i;
    }

    return numMax;
}

// Returns the robot coordinates of the given sample index

void GPS::calcSamplePoint(int index, MVec3 &out) {
    int temp, i;

    out = curMean;
    temp = index;
    
    // Calculate sample points to be 1 standard deviation away from mean,
    // aligned along the x, y, h axes.
    for (i = 2; i >= 0; i--) {
        out(i, 0) += curDev(i, 0) * (double)((temp % 3) - 1);
        temp /= 3;
    }
}

// Returns the number of neighbours a sample has in the specified direction.
// dir = 0 for x axis, 1 for y, 2 for h. Neighbour indices are also returned.

int GPS::calcNeighbourIndices(int dir, int index, int *outIndices) {
    int i, temp, d[3], dInd, neighbours;
    
    // Calculate individual coordinate indices from sample index
    temp = index;
    for (i = 2; i >= 0; i--) {
        d[i] = (temp % 3) - 1;
        temp /= 3;
    }
    
    // Calculate the offset from the sample index the neighbours should be at
    dInd = 1;
    if (dir == 0) dInd = 9;
    if (dir == 1) dInd = 3;
    
    // Check if neighbours are still inside the sample array
    neighbours = 0;
    if (d[dir] > -1) outIndices[neighbours++] = index - dInd;
    if (d[dir] < 1) outIndices[neighbours++] = index + dInd;
    
    return neighbours;
}
#endif //NEW_LINE_UPDATE

void GPS::beaconDistanceCal() {

}


//Copies some things from the array of VisualObjects we are given
//into our own data structure.
//This is done so that if VisualObjects change, it is easier to change
//gps code.
void GPS::UpdateObservedVobs(VisualObject * nvo) {
    for (int i = 0; i < VOB_COUNT; i++) {
        observedVobs[i].vobType = i;
        observedVobs[i].cf = nvo[i].cf;
        
        if(observedVobs[i].cf > 0) {
            observedVobs[i].h   = nvo[i].head;
            observedVobs[i].d   = nvo[i].dist2;
            observedVobs[i].var = nvo[i].var;
            observedVobs[i].angleVar = nvo[i].angleVar;            
        } else {
            observedVobs[i].h   = 0;
            observedVobs[i].d   = 0;
            observedVobs[i].angleVar = VERY_LARGE_INT;
			observedVobs[i].var = VERY_LARGE_INT;
#ifdef VALGRIND
			VALGRIND_MAKE_WRITABLE(&observedVobs[i].h,sizeof(double));
			VALGRIND_MAKE_WRITABLE(&observedVobs[i].d,sizeof(double));
			VALGRIND_MAKE_WRITABLE(&observedVobs[i].angleVar,sizeof(double));
			// note - var could be used
#endif // VALGRIND
        }
    }
}

// Adds another mode to the distribution, making sure it is inserted into the
// correct (weight sorted) position.

void GPS::insertGaussian(SingleGaussian &temp) {
    int i;
    
    for (i = numGaussians; i > 0 && temp.weight > r[i - 1].weight; i--) {
        if (i < maxGaussians) r[i] = r[i - 1];
    }
            
    if (numGaussians < maxGaussians)
        numGaussians++;

    if (i < maxGaussians) {
        r[i] = temp;
        r[i].mean.updateVar(&r[i].cov);

    }
}

// Normalises the sum of gaussians, culls and merges individual gaussians,
//  also sorts gaussians by decreasing weight

void GPS::formDistribution() {
    int i, j, k, l, cullCount, mergeCount, oldGaussians;
    double dist;//, deltaX, deltaY, deltaH;
    SingleGaussian temp;
    MMatrix3 deltaCov;
    
    oldGaussians = numGaussians;
    cullCount = 0; mergeCount = 0;
    
    // Merge gaussians that have similar means and covariances
    for (i = 1; i < numGaussians; i++) {
        if (r[i].weight != 0.0) {
            
            for (j = 0; j < i; j++) {
                if (r[j].weight != 0.0) {
                    /*
                    deltaX = r[i].mean.pos.x - r[j].mean.pos.x;
                    deltaY = r[i].mean.pos.y - r[j].mean.pos.y;
                    deltaH = NormalizeAngle_180(r[i].mean.h - r[j].mean.h);
                    dist = deltaX * deltaX + deltaY * deltaY + deltaH * deltaH;
                    //mergeThreshold was 20
                    */
                    
                    dist = ABS(r[i].mean.pos.x - r[j].mean.pos.x);
                    dist = MAX(dist,ABS(r[i].mean.pos.y - r[j].mean.pos.y));
                    dist = MAX(dist, ABS(NormalizeAngle_180(r[i].mean.h - r[j].mean.h)));
                    /*
                    if (ABS(NormalizeAngle_180(r[i].mean.h - r[j].mean.h) > 3))
                        continue;
                    */
                    if (dist < mergeThreshold) {
                        deltaCov = r[i].cov; 
                        deltaCov -= r[j].cov;
                        
                        dist = 0.0;
                        for (k = 0; k < 3; k++) {
                            for (l = 0; l < 3; l++)
                            {
                                /* This is old version where it counts square of
                                   differences in cov matrix 
                                   mergeCovThreshold was 10000;
                                */
                                /* 
                                dist += deltaCov(k, l) * deltaCov(k, l);                 
                                */
                                
                                if (ABS(deltaCov(k, l)) > dist)
                                    dist = ABS(deltaCov(k, l));
                                
                            }
                        }
                        
                        if (dist < mergeCovThreshold) {
                            mergeGaussians(i, j);
                            mergeCount++;
                        }
                    }
                }
            }
        }
    }
    // Insertion sort. Since array is probably close to being sorted, this
    // should be quite efficient.
    for (i = 1; i < numGaussians; i++) {
        if (r[i - 1].weight < r[i].weight) {
            temp = r[i];
            
            for (j = i - 1; j >= 0 && r[j].weight < temp.weight; j--)
            {
                r[j + 1] = r[j];
            }   
            r[j + 1] = temp;
        }
    }

    normaliseDistribution();
     
    // Cull unlikely gaussians and renormalise again
    /*if (!useRelativeCullWeight) {
        cullWeight = cullThreshold;
    } else {
        double maxWeight = r[0].weight;
        double minWeight = r[0].weight;
        for (i = 0; i < numGaussians; i++) {
            if (r[i].weight > maxWeight) maxWeight = r[i].weight;
            if (r[i].weight < minWeight) minWeight = r[i].weight;
        }

        cullWeight = (maxWeight + minWeight) * 0.5;
        if (cullWeight > cullThreshold) cullWeight = cullThreshold;
    }
		
    for (i = 0; i < numGaussians; i++) {
        if (r[i].weight < cullWeight) {
            r[i].weight = 0.0;
            cullCount++;
        }
    }

    normaliseDistribution();*/

    // Remove zero and NaN weight modes from the mode count
    while (numGaussians > 0 && (r[numGaussians - 1].weight == 0.0 ||
           r[numGaussians - 1].weight != r[numGaussians - 1].weight))
        --numGaussians;

        
    // If all modes have been removed, reset distribution. If this is being
    // called, then something is wrong.
    if (numGaussians == 0) {
        cout << "GPS::formDistribution warning: Total probability zero, resetting.\n";
        
        numGaussians = 1;
        r[0].mean.pos.setVector(vCART, FIELD_WIDTH / 2, FIELD_LENGTH / 2);
        r[0].mean.h = HALF_CIRCLE/2;
        r[0].cov(0,0)= get95CF (FIELD_WIDTH / 2);
        r[0].cov(0,1)= 0.0;
        r[0].cov(0,2)= 0.0;
        r[0].cov(1,0)= 0.0;
        r[0].cov(1,1)= get95CF (FIELD_LENGTH / 2);
        r[0].cov(1,2)= 0.0;
        r[0].cov(2,0)= 0.0;
        r[0].cov(2,1)= 0.0;
        r[0].cov(2,2)= get95CF (HALF_CIRCLE);
        r[0].weight = 1.0;
        r[0].mean.updateVar(&r[0].cov);
    }
}

// Makes sure the total probability of the sum of gaussians is 1

void GPS::normaliseDistribution() {
    double totalWeight = 0.0;
    int i;

    for (i = 0; i < numGaussians; i++) 
        totalWeight += r[i].weight;
    
    if (totalWeight != 0.0) {    
        for (i = 0; i < numGaussians; i++)
            r[i].weight /= totalWeight;
    }
}

// Merges two gaussians into one

/*inline*/ void GPS::mergeGaussians(int a, int b) {
    double totalWeight, scaleA, scaleB;
    
    totalWeight = r[a].weight + r[b].weight;
    if (totalWeight == 0.0) return;
    
    scaleA = r[a].weight / totalWeight;
    scaleB = r[b].weight / totalWeight;
    
    r[a].mean.pos.scale(scaleA); r[a].mean.h *= scaleA; r[a].cov *= scaleA;
    r[b].mean.pos.scale(scaleB); r[b].mean.h *= scaleB; r[b].cov *= scaleB;
    
    r[a].mean.pos.add(r[b].mean.pos);
    r[a].mean.h += r[b].mean.h;
    r[a].cov += r[b].cov;
    r[a].weight += r[b].weight;
    r[a].mean.updateVar(&r[a].cov);
    
    r[b].weight = 0.0;   
}

// Derrick: For debugging.
// Does a small update test

void GPS::testKalman() {
    SetGoals(true);

	// Set robot position to be two equally probable gaussians
    numGaussians = 2;
    r[0].mean.pos.setVector(vCART, 50.0, 150.0);
    r[0].mean.h = 0;
    r[0].cov(0,0)= 100;
    r[0].cov(0,1)= 0;
    r[0].cov(0,2)= 0;
    r[0].cov(1,0)= 0;
    r[0].cov(1,1)= 100;
    r[0].cov(1,2)= 0;
    r[0].cov(2,0)= 0;
    r[0].cov(2,1)= 0;
    r[0].cov(2,2)= 100;
	r[0].weight = 1.0;
    r[0].mean.updateVar(&r[0].cov);
    r[1].mean.pos.setVector(vCART, 115.0, 115.0);
    r[1].mean.h = 315;
    r[1].cov(0,0)= 100;
    r[1].cov(0,1)= 0;
    r[1].cov(0,2)= 0;
    r[1].cov(1,0)= 0;
    r[1].cov(1,1)= 100;
    r[1].cov(1,2)= 0;
    r[1].cov(2,0)= 0;
    r[1].cov(2,1)= 0;
    r[1].cov(2,2)= 100;
	r[1].weight = 1.0;
    r[1].mean.updateVar(&r[1].cov);
    
    cout << "--- Start GPS Weight function testing\n";
    formDistribution();
    
    /*// Set observation heading and distance
    SlimVisOb vi;
    vi.vobType = 8;
    vi.d = 220.0;
    vi.var = 100.0;
    vi.h = 0.0;
    vi.angleVar = 100.0;
    vi.cf = 1.0;
   
    double iv[2], jacX[2], jacY[2], jacH[2], var[2];
    
    // Do an update based on the observation
    for (int mode = 0; mode < 2; mode++) {
        //cout <<"For Gaussian no." << mode << "\n";
        for (int i = 0; i < 2; i++) {
            var[i] = 0.0;
            iv[i] = 0.0;
            jacX[i] = 0.0;
            jacY[i] = 0.0;
            jacH[i] = 0.0;
        }
        
        cout << "Adding measurement...\n";
        addMeasurement(&r[mode], vi, jacX, jacY, jacH, iv, var, 0, true);
        cout << "Updating kalmann filter...\n";
        kalmanUpdate<2>(&r[mode], jacX, jacY, jacH, iv, var);
        cout << "Updating variance...\n";
        r[mode].mean.updateVar(&r[mode].cov);    
    }*/
    
    // Set multiple observation locations
    double x[4], y[4], h[4], vX[4], vY[4], vH[4], w[4];
    x[0] = 80 ; y[0] = 140; h[0] = 10 ; vX[0] = 100; vY[0] = 250; vH[0] = 90 ;
    x[1] = 180; y[1] = 40 ; h[1] = 310; vX[1] = 180; vY[1] = 200; vH[1] = 110;
    x[2] = 50 ; y[2] = 60 ; h[2] = 10 ; vX[2] = 200; vY[2] = 200; vH[2] = 150;
    x[3] = 180; y[3] = 40 ; h[3] = 310; vX[3] = 180; vY[3] = 200; vH[3] = 110;
    w[0] = 1; w[1] = 1; w[2] = 1; w[3] = 1;
    GPSDirectPositionUpdate(4, x, y, h, vX, vY, vH, w);
     
    formDistribution();
    cout << "--- End GPS Weight function testing\n";   

	// Robot position probability starts with only one gaussian
    numGaussians = 1;
    r[0].mean.pos.setVector(vCART, FIELD_WIDTH/2, FIELD_LENGTH/2);
    r[0].mean.h = HALF_CIRCLE/2;
    r[0].cov(0,0)= get95CF(FIELD_WIDTH);
    r[0].cov(1,1)= get95CF(FIELD_LENGTH);
    r[0].cov(2,2)= get95CF(HALF_CIRCLE);
	r[0].weight = 1.0;
	r[0].mean.updateVar(&r[0].cov);
}


void GPS::addMeasurement(SingleGaussian *g, SlimVisOb &vi, 
    double bx, double by,
    double *jacWRTX, double *jacWRTY, double *jacWRTH,
    double *iv, double *vars, int curNum, bool dist) {
#ifdef OFFLINE    
    if (iv==NULL)
        cout <<__func__ <<":iv NULL" <<endl;
    if (jacWRTX==NULL)
        cout <<__func__ <<":jacWRTX NULL" <<endl;
    if (jacWRTY==NULL)
        cout <<__func__ <<":jacWRTY NULL" <<endl;
    if (jacWRTH==NULL)
        cout <<__func__ <<":jacWRTH NULL" <<endl;
    if (vars==NULL)
        cout <<__func__ <<":vars NULL" <<endl;
    if (g==NULL)
        cout <<__func__ <<":g NULL" <<endl;
#endif        
    double dx = g->mean.pos.x - bx;
    double dy = g->mean.pos.y - by;
    double dxsq = SQUARE(dx);
    double dysq = SQUARE(dy);
    double expHead = RAD2DEG(atan2(-dy,-dx)) - g->mean.h;
    double measHead = vi.h;
    double expDist = sqrt(dxsq + dysq);

    // keep expected heading and measurement within 180 of each other
    ANGLE_FIX(expHead);
    if(expHead - measHead > HALF_CIRCLE)
        measHead += FULL_CIRCLE;
    else if(measHead - expHead > HALF_CIRCLE)
        measHead -= FULL_CIRCLE;

    // add to innovation vector
    iv[curNum] = measHead - expHead;
    if(dist) iv[curNum+1] = vi.d - expDist;
    // add to jacobians
    jacWRTX[curNum] = -(DEG_OVER_RAD*dy)/(dxsq + dysq);
    jacWRTY[curNum] = (DEG_OVER_RAD*dx)/(dxsq + dysq);
    jacWRTH[curNum] = -1;
    if(dist) {
        jacWRTX[curNum+1] = dx/expDist;
        jacWRTY[curNum+1] = dy/expDist;
        jacWRTH[curNum+1] = 0;
    }
    
    // add to variances
    vars[curNum] = vi.angleVar;
    if(dist) vars[curNum+1] = 8*vi.var;
}


template <int n> void 
GPS::kalmanUpdate(SingleGaussian *g, double *jacWRTX, double *jacWRTY, double *jacWRTH, double *iv, double *vars) {

    // jacobian of measurement
    MMatrix<double,n,3> jac(0);
    for(int i=0 ; i<n ; i++) {
        jac(i,0) = jacWRTX[i];
        jac(i,1) = jacWRTY[i];
        jac(i,2) = jacWRTH[i];
    }
    MMatrix<double,3,n> jacT;
    jacT.isTranspose(jac);
    
    // innovation vector of measurement
    MMatrix<double,n,1> innVec(0);
    for(int i=0 ; i<n ; i++)
            innVec(i,0) = iv[i];
            
    // compute kalman gain
    MMatrix<double,n,3> jCov;
    jCov.isMult(jac, g->cov);
    MMatrix<double,n,n> obsCov;
    obsCov.isMult(jCov,jacT);
    for(int i=0 ; i<n ; i++)
        obsCov(i,i) += vars[i];
        
    MMatrix<double,3,n> covjT;
    covjT.isMult(g->cov,jacT);
    MMatrix<double,3,n> k;
    MMatrix<double,n,n> obsCovInv;
    obsCovInv.isInverse(obsCov);
    k.isMult(covjT,obsCovInv);

    // Calculate new state
    MVec3 priori;
    priori(0, 0) = g->mean.pos.x;
    priori(1, 0) = g->mean.pos.y;
    priori(2, 0) = g->mean.h;
    MVec3 move;
    move.isMult(k,innVec);
    
    // Calculate new weight
    double exponent, weightScaler;
    MMatrix<double,n,1> exponentVec(innVec);
    exponentVec.preMult(obsCovInv);

    exponent = 0.0;
    for (int i = 0; i < n; i++)
        exponent += innVec(i, 0) * exponentVec(i, 0);
    exponent *= -0.5;

    /*double determinant = obsCov.Det();
    if (determinant == 0.0) weightScaler = 0.0;
    else weightScaler = exp(exponent) / sqrt(fabs(determinant));*/
    weightScaler = exp(exponent);
    
    // Calculate new covariances
    MMatrix3 m(*unitMatrix3);
    MMatrix3 kJ;
    kJ.isMult(k,jac);
    m -= kJ;
    g->cov.preMult(m);

    MVec3 newState(priori);
    newState += move;
    double rx = newState(0, 0);
    double ry = newState(1, 0);
    //Not good if we produce more than unique position
    //Need a flag for checking should we clip or not.
    /*
    if (FieldClipping(rx, ry))
    {
#ifdef OFFLINE    
        if (outputStream != NULL) 
        {
            *outputStream << __func__ << " rx:" << rx << " ry:" << ry << endl;
        }    
#endif        
        return;
    } 
    */   
    g->mean.pos.setVector(vCART, rx, ry);
    g->mean.h = newState(2, 0);
    ANGLE_FIX(g->mean.h);

    // If we got a bad measurement, send a cplane
    if (weightScaler < noiseThreshold)
        sendCPlane = true;

    g->weight *= weightScaler;

    nX[n]++;
    sumX[n] += weightScaler;
    sumXSquared[n] += SQUARE(weightScaler);
    
    // Visual pull has been disabled by Derrick Apr 2004
    visPull(0, 0) = 0; visPull(1, 0) = 0; visPull(2, 0) = 0;
    vbPull(0, 0) = 0; vbPull(1, 0) = 0;
}

void GPS::GPSDirectPositionUpdate(double x, double y, double h,
                double xvar, double yvar, double hvar) {
    double unitWeight = 1.0;
    GPSDirectPositionUpdate(1, &x, &y, &h, &xvar, &yvar, &hvar, &unitWeight);
}

void GPS::GPSDirectPositionUpdate(int numObs, double *x, double *y, double *h,
                double *xvar, double *yvar, double *hvar, double *weight) {

    if (isPenalised || !doGPSVisionUpdate)
        return;
     
    directx = x[0];
    directy = y[0];
    directh = h[0];

    SingleGaussian temp;
    
    double jacX[3], jacY[3], jacH[3];
    double var[3], iv[3];
    double minWeight;

    int i, oldNumGaussians, mode, obsMode, currentDimCount = 0;
    int maxobsindex = -1; // observation with max weight
    double maxobsweight = 0;

    if (numObs < 1) return;
    
    // Work from a backup of the old distribution
    minWeight = r[numGaussians - 1].weight;
    for (i = 0; i < numGaussians; i++) {
        rCopy[i] = r[i];
        r[i].weight *= noiseThreshold;
    }
    
    oldNumGaussians = numGaussians;
    
    for (obsMode = 0; obsMode < numObs; obsMode++) {
        currentDimCount = 0;
        
        if(xvar[obsMode] < VERY_LARGE_INT/2) {
            jacX[currentDimCount] = 1;
            jacY[currentDimCount] = 0;
            jacH[currentDimCount] = 0;
            var[currentDimCount] = xvar[obsMode];
            currentDimCount++;
        }
            
        if(yvar[obsMode] < VERY_LARGE_INT/2) {
            jacX[currentDimCount] = 0;
            jacY[currentDimCount] = 1;
            jacH[currentDimCount] = 0;
            var[currentDimCount] = yvar[obsMode];
            currentDimCount++;                
        }
            
        if(hvar[obsMode] < VERY_LARGE_INT/2) {
            jacX[currentDimCount] = 0;
            jacY[currentDimCount] = 0;
            jacH[currentDimCount] = 1;
            var[currentDimCount] = hvar[obsMode];
            currentDimCount++;
        }
                
        //OSYSDEBUG(("%s : currentDimCount : %d, x %f, y %f, h %f\n",
        //            __func__,currentDimCount,x[obsMode],y[obsMode],h[obsMode]));

#ifdef OFFLINE
#ifdef DIRECT_POSITION_DEBUG
        if (outputStream != NULL)
        {
            *outputStream << __func__ << " currentDimCount:" << currentDimCount << " x:" << x[obsMode] << " y:" << y[obsMode] << " h:" << h[obsMode] << " xvar:" << xvar[obsMode] << " yvar:" << yvar[obsMode] << " hvar:" << hvar[obsMode] << " weight:" << weight[obsMode] << endl;
        }
#endif     
#endif

        mode = 0;    
        while (mode < oldNumGaussians && rCopy[mode].weight >= minWeight) {
            // Calculate the effect of every observation mode on each of
            // the distribution modes
       
            currentDimCount = 0;

            if(xvar[obsMode] < VERY_LARGE_INT/2)
                iv[currentDimCount++] = x[obsMode] - rCopy[mode].mean.pos.x;
            if(yvar[obsMode] < VERY_LARGE_INT/2)
                iv[currentDimCount++] = y[obsMode] - rCopy[mode].mean.pos.y;
            if(hvar[obsMode] < VERY_LARGE_INT/2)
                iv[currentDimCount++] =
                    NormalizeAngle_180(h[obsMode] - rCopy[mode].mean.h);
            
            temp = rCopy[mode];
            temp.weight *= weight[obsMode];
            
            switch(currentDimCount) {
            case 1: kalmanUpdate<1>(&temp, jacX, jacY, jacH, iv, var); break;
            case 2: kalmanUpdate<2>(&temp, jacX, jacY, jacH, iv, var); break;
            case 3: kalmanUpdate<3>(&temp, jacX, jacY, jacH, iv, var); break;
            }
            
            // Insert the corrected mode into the new distribution
            insertGaussian(temp);
            if (numGaussians == maxGaussians)
                minWeight = r[numGaussians - 1].weight;

#ifdef OFFLINE
#ifdef DIRECT_POSITION_DEBUG
                    if (outputStream != NULL)
                    {
                        *outputStream << endl << __func__ << " prev x:" << rCopy[mode].mean.pos.x << " y:" << rCopy[mode].mean.pos.y << " h:" << rCopy[mode].mean.h << " prev weight:" << rCopy[mode].weight << endl;
                        *outputStream << __func__ << " temp x:" << temp.mean.pos.x << " y:" << temp.mean.pos.y << " h:" << temp.mean.h << " temp before form distribution, weight:" << temp.weight << endl << endl;
                    }
#endif
#endif     
            
            // keep track of the best weighted match to display on
            // world model display
            if (mode == 0 && temp.weight > maxobsweight) {
                maxobsweight = temp.weight;
                maxobsindex = obsMode;
            }

            mode++;
        }
    }

#ifdef OFFLINE
#ifdef DIRECT_POSITION_DEBUG
    cout << __func__ << endl;
    GPSOutputSelfData();
    cout << __func__ << endl << endl; 
#endif
#endif     
    
    // This sets the world model display to show the edge points positions
    // with respect to the best symmetry
    directx = x[maxobsindex];
    directy = y[maxobsindex];
    directh = h[maxobsindex];

    formDistribution();
    
#ifdef OFFLINE
#ifdef DIRECT_POSITION_DEBUG
    cout << __func__ << " AFTER FORM DISTRIBUTION" << endl;
    GPSOutputSelfData();
    cout << __func__ << " AFTER FORM DISTRIBUTION" << endl << endl; 
#endif     
#endif 
    
}




int VisionUpdateCount = 0; //for debugging only



const WMObj& GPS::self() {
   return r[0].mean;
}

const MMatrix3& GPS::selfCov() {
	return r[0].cov;
}

unsigned int& GPS::sharedBehaviour() {
	return r[0].mean.behavioursVal;
}

const MVec3& GPS::getVisualPull() {
	return visPull;
}

const Vector& GPS::getBall(int context) {

	return newball->getPos(context);

}

const Vector& GPS::getVBall(int context) {
	
	return newball->getVelocity(context);
	
}

double GPS::getBallAngle() {

	const Vector& ball = newball->getPos(LOCAL);
	const Vector& vball = newball->getVelocity(LOCAL);
	
	double dp = ball.x*vball.x + ball.y*vball.y;
	double denom = ball.d * vball.d;
	
	if(abs(dp) > denom) {
		cout << "Error: length of vectors multplied together less than dot product" << endl;
		if(dp > 0)
			return 0;
		else
			return 1;
	}
	
	// hopefully won't happen
	if(abs(denom) < 0.0001) {
		return 180;
	}
	
	return RAD2DEG(acos(dp/denom));
			
}

const MMatrix2& GPS::getBallCov() {

	return newball->getCov();

}

double GPS::getBallMaxVar() {

	const MMatrix2& bp = newball->getCov();
	double b = -(bp(0,0) + bp(1,1));
    double c = bp(0,0)*bp(1,1) - bp(0,1)*bp(1,0);
    double posVar;
    if(b*b-4*c < 1E-5) {
        posVar = (-b+0)/2;
        if (b*b-4*c < -1E-5)
            cout << __func__ << "WARNING: b*b-4*c < 0 : " << (b*b-4*c) << endl;
    } else {
        posVar = (-b + sqrt(b*b-4*c)) / 2;
    }
    return posVar;
}

/* Warning: KI2DWV was hacked and has no variance. This returns a constant */
#if 0
const MMatrix2& GPS::getVelCov() {
    return newball->getVelCov();
}

double GPS::getVBallMaxVar() {

	const MMatrix2& bp = newball->getVelCov();
	double b = -(bp(0,0) + bp(1,1));
    double c = bp(0,0)*bp(1,1) - bp(0,1)*bp(1,0);
    double posVar;
    if(b*b-4*c < 1E-5) {
        posVar = (-b+0)/2;
        if (b*b-4*c < -1E-5)
            cout << __func__ << "WARNING: b*b-4*c < 0 : " << (b*b-4*c) << endl;
    } else {
        posVar = (-b + sqrt(b*b-4*c)) / 2;
    }
    return posVar;
}

double GPS::getVBIVLength() {

	return newball->getIVLength();

}

MMatrix2 GPS::getVCLocal() {

	return newball->getVCovLocal();

}

MMatrix2 GPS::getVCBall() {

	return newball->getVCovBall();

}
#endif

// This function returns the teammate information. What is cache?
// If in the current frame, this function is called several times,
// instead of computing the result again simply return the cache result.
//
WMObj& GPS::teammate(int botNum, char origin) {

    // Check cache.
    if (teammateAccessId==frameId && botNum==lastTeammateAccessed && origin == lastTeammateOrigin) {
		//cout << "using cached teammate for teammate " << botNum << endl;
		return retTeammate;
	}
	teammateAccessId = frameId;
	lastTeammateAccessed = botNum;
	lastTeammateOrigin = origin;

    retTeammate.pos.setAngleNorm(HALF_CIRCLE);
    //if requesting the global coordinates of teammate
    if (origin=='g') {
        //i store in global anyway, so just return the right one
        retTeammate = wmTeammate[botNum-1];
    }
    //else return the coordinates of teammate relative to you
    else {
        retTeammate.vobType = wmTeammate[botNum-1].vobType;
        double dx = wmTeammate[botNum-1].pos.x - r[0].mean.pos.x;
        double dy = wmTeammate[botNum-1].pos.y - r[0].mean.pos.y;
        retTeammate.pos.setVector(vCART,dx,dy);
        retTeammate.posVar = wmTeammate[botNum-1].posVar;
        //
        //INACCURACY ALERT
        //ERIC when converting to local, you should add your own variance again and recalculate posVar
        //
        retTeammate.h = wmTeammate[botNum-1].h;
        retTeammate.hVar = wmTeammate[botNum-1].hVar;
		retTeammate.behavioursVal = wmTeammate[botNum-1].behavioursVal;
		retTeammate.counter = wmTeammate[botNum-1].counter;
        retTeammate.pos.rotate(90-r[0].mean.h);
    }
    return retTeammate;
}

const WMObj& GPS::tmObj(int arrIndex) {
	return wmTeammate[arrIndex];
}

const WMObj& GPS::tmBallObj(int arrIndex) {
	return tmBall[arrIndex];
}

WMObj& GPS::oGoal() {

    //check cache
    if (oGoalAccessId==frameId) {
		//cout << "using cached oGoal" << endl;
		return retOGoal;
	}
	oGoalAccessId = frameId;

    retOGoal.pos.setAngleNorm(HALF_CIRCLE);
    retOGoal.vobType = ownGoal;
    double gx = (FIELD_WIDTH / 2) - r[0].mean.pos.x;
    double gy = (-WALL_THICKNESS) - r[0].mean.pos.y;
    retOGoal.pos.setVector(vCART,gx,gy);
    retOGoal.posVar = r[0].mean.posVar;
    //h and hvar dont apply to the goal
    retOGoal.pos.rotate(90-r[0].mean.h);

    return retOGoal;
}

WMObj& GPS::tGoal() {

    //check cache
    if (tGoalAccessId==frameId) {
		//cout << "using cached tGoal" << endl;
		return retTGoal;
	}

	tGoalAccessId = frameId;

    retTGoal.pos.setAngleNorm(HALF_CIRCLE);
    retTGoal.vobType = targetGoal;
    double gx = (FIELD_WIDTH / 2) - r[0].mean.pos.x;
    double gy = (FIELD_LENGTH + WALL_THICKNESS) - r[0].mean.pos.y;
    retTGoal.pos.setVector(vCART,gx,gy);
    retTGoal.posVar = r[0].mean.posVar;
    //h and hvar dont apply to the goal
    retTGoal.pos.rotate(90-r[0].mean.h);

    return retTGoal;
}


//
// CONSIDERATION
// right now the "location" of the goal is behind the goal line
// so if you are to the side of the goal, you will try and shoot
// to a point within the goal, and the ball will probably bounce
// off the walls beside the opening of the goal..
// Perhaps we should change the "location" of the goal to be
// the mid point of the goal line..
// Or be thinking about the 2 goal posts instead of one point
//


const Vector&
GPS::getOppPos(int oppNum, int context) {
	
	return opponents[oppNum]->getPos(context);
	
}

const MMatrix2&
GPS::getOppCov(int oppNum) {
	
	return opponents[oppNum]->getCov();
	
}

double 
GPS::getOppCovMax(int oppNum) {
    const MMatrix2 mm = opponents[oppNum]->getCov();
    double b = -(mm(0,0) + mm(1,1));
    double c = mm(0,0)*mm(1,1) - mm(0,1)*mm(1,0);
    double posVar;
    if(b*b-4*c < 1E-5) {
        posVar = (-b+0)/2;
        if (b*b-4*c < -1E-5)
            cout << __func__ << "WARNING: b*b-4*c < 0 : " << (b*b-4*c) << endl;
    } else {
        posVar = (-b + sqrt(b*b-4*c)) / 2;
    }
    return posVar;
}

#ifdef VISUAL_TRACK_FRIENDS
const Vector&
GPS::getFriendPos(int friendNum, int context) {
	
	return friends[friendNum]->getPos(context);
	
}
#endif // VISUAL_TRACK_FRIENDS

const Vector&
GPS::getClosestOppPos(int context, int *oppNum) {
	
	double minDist = VERY_LARGE_INT;
	int minInd = -1;
	for(int i=0 ; i<NUM_OPPONENTS ; i++) {
		const Vector& v = opponents[i]->getPos(LOCAL);
		if(v.d < minDist) {
			minDist = v.d;
			minInd = i;
		}
	}
	if (oppNum!=NULL) {
		*oppNum = minInd;
	}
	return opponents[minInd]->getPos(context);
	
}



//ERIC!! WOULDNT IT BE BETTER TO DO SOME SORT OF MOD??


//WMShareObj share;

//called by shareEnvironment function in UNSWSkills which is called by the strategy files
WMShareObj GPS::getShareInfo () {
    WMShareObj share;

    
    if ((r[0].mean.pos.x != r[0].mean.pos.x) ||
    	(r[0].mean.pos.y != r[0].mean.pos.y) ||
    	(r[0].mean.h != r[0].mean.h) ||
    	(r[0].mean.posVar != r[0].mean.posVar) ||
    	(r[0].mean.hVar != r[0].mean.hVar) ||
    	(r[0].mean.behavioursVal != r[0].mean.behavioursVal)) {
    	
        cout << "NaN when trying to share gps info!" << endl;
    	share.posx = share.posy = share.posh = share.posVar = share.hVar = 0;
    	share.behavioursVal = 0;
    } else {
        share.posx = (float)r[0].mean.pos.x;
        share.posy = (float)r[0].mean.pos.y;
        share.posh = (float)r[0].mean.h;
        share.posVar = (float)r[0].mean.posVar;
        share.hVar = (float)r[0].mean.hVar;
        share.behavioursVal = r[0].mean.behavioursVal;
    }
    		
    // Ted : Send ball info to teammates either I can see the ball or I can't see the ball but I am sure the ball is there.
    //       Right? The ball sent is the gps kalman ball even if the robot can see vision ball. However for some reason the
    //       current gps kalman ball is very poor. Derrick is fixing it right now. Come back later. Or use the vision ball
    //       to caluclate the new share.ball ......
    if ((canSee(vobBall) || getBallMaxVar() < get95CF(20))
        &&  self().posVar < get95CF(100)) {
        const Vector& b = getBall(GLOBAL);
        share.ballx = b.x;
        share.bally = b.y;
        share.ballvar = getBallMaxVar();
    } else {
	share.ballx = 0;
	share.bally = 0;
        share.ballvar = VERY_LARGE_INT;
    }
    
    //for(int i=0 ; i<NUM_OPPONENTS ; i++) {
    //    opponents[i]->flushUpdate(share.opponentsUpdate[i]);
    //}    
	
#ifdef IR_LATENCY_TEST
    if(doLatTest) {
        share.latMode = INIT_LAT_WM;
        doLatTest = false;
    } else {
        share.latMode = NORMAL_WM;
    }
#endif // IR_LATENCY_TEST
        
    return share;
}

//called by wreceive in UNSWSKills which is called by processWireless in Vision
//which is called by resultWireless in Vision, which is called when a message is received from
//the wireless object
//@param sourceBot is the robot number where the info came from
void GPS::processShareInfo (int sourceBot, const WMShareObj& info) {
    //wmTeammate[sourceBot-1] = info->obj;
	wmTeammate[sourceBot-1].pos.setVector(vCART, (double)info.posx, (double)info.posy);
	wmTeammate[sourceBot-1].h = (double)info.posh;
    wmTeammate[sourceBot-1].posVar = (double)info.posVar;
    wmTeammate[sourceBot-1].hVar = (double)info.hVar;

	wmTeammate[sourceBot-1].behavioursVal = info.behavioursVal;
	
	//bottom stuff is for maverick only and should go away!
	
	
	//at padua i (eric) got rid of these from wireless transmission as they were only used by maverick
	//i replaced them with dummy values so that the code still compiles
	//
#ifdef MAVERICK_WM_STUFF
	wmTeammate[sourceBot-1].ballDist = 400; //info.ballDist;
	wmTeammate[sourceBot-1].playerType = 1; //info.playerType;
	wmTeammate[sourceBot-1].amThirdPlayer = 1; //info.amThirdPlayer;
#endif
	
    //*wmTeammateCov[sourceBot-1] = info->cov;
	wmTeammate[sourceBot-1].counter = TEAMMATE_GPS_COUNTER;

	//store where the teammate thinks the ball is
	tmBall[sourceBot-1].posVar = info.ballvar;
	tmBall[sourceBot-1].pos.setVector(vCART, info.ballx, info.bally);


    if((sourceBot == sbRobotNum) || (info.ballvar < shareBallvar)) {
        shareBallx = info.ballx;
        shareBally = info.bally;
        shareBallvar = info.ballvar;
        sbRobotNum = sourceBot;
    }
	
    //for(int i=0 ; i < NUM_OPPONENTS; i++) {
    //	opponents[i]->addUpdates(info.opponentsUpdate[i]);
    //}    
}


// returns non-zero if it can "see" (ie - using for localisation) the
// given object, according to indexing in visual cortex.
int GPS::canSee(int object) {
    return (view & (1 << object));
}

void GPS::increaseSelfAngleVariance(double amount) {
    for (int i = 0; i < numGaussians; i++) {
        r[i].cov(2,2) += amount;
        r[i].mean.updateVar(&r[i].cov);
    }
}    

// send the whole gaussians 
void GPS::sendGaussians() {
#ifndef OFFLINE
    //struct rlnk_gaussians allgaussians[maxGaussians];
    static struct rlnk_gaussians allgaussians[maxGaussians];
    int i;
    for ( i = 0; i < numGaussians && i < maxGaussians; ++i) {
        allgaussians[i].x = (float)r[i].mean.pos.x;
        allgaussians[i].y = (float)r[i].mean.pos.y;
        allgaussians[i].h = (float)r[i].mean.h;
        allgaussians[i].weight = (float)r[i].weight;
        for (int x = 0;x < 3; ++x)
            for (int y = 0;y < 3;++y)
                allgaussians[i].cov[x][y] = r[i].cov(x,y);
    }

    if (numGaussians > 0) {
        //cerr << "sendGaussians sending to Robolink "<< numGaussians <<" times one size:"<<sizeof(struct rlnk_gaussians) << "  total:"<<(numGaussians)*sizeof(struct rlnk_gaussians) << endl;
        Robolink::sendToBase((byte*)allgaussians, RLNK_GAUSSIANS, i *sizeof(struct rlnk_gaussians));
        //Robolink::sendToBase((byte*)r, RLNK_GAUSSIANS, (numGaussians)*sizeof(SingleGaussian));
//        cerr << "sendGaussians features sent" << endl;
    }
#endif // OFFLINE
}

