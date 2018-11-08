/*

   Copyright 2005 The University of New South Wales (UNSW) and National  
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
 * $Id: VisualCortex.cc 1953 2003-08-21 03:51:39Z eileenm $
 * modified weiming 17/12/04
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/


#ifndef OFFLINE
#include <OPENR/OSubject.h>
#include <OPENR/OObserverVector.h>
#include <OPENR/OPENREvent.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OFbkImage.h>
#include "OPENR/ODataFormats.h"
#include <MCOOP.h>
#include "Robolink.h"
#endif	// OFFLINE

#include <cstdio>
#include <cstring>
#include <cfloat>
#include <sys/time.h>

#include "VisualCortex.h"
#include "SanityChecks.h"
#include "gps.h"
#include "PointProjection.h"
#include "Profile.h"
#include "LinearShifts.h"
#include "../share/Common.h"
#include "../share/SwitchBoard.h"
#include "../share/BallDef.h"
#include "../share/PWalkDef.h"

#ifdef VALGRIND
#include <valgrind/memcheck.h>
#endif // VALGRIND

//#define VC_DEBUG

using namespace std;


// These are calibrated manually using one beacon/goal and then fitting
// dist = CONST/(height - OFFSET) using the gnuplot script in
// base/subvision

#if 0
//OSAKA BEACONS
static const double BEACON_DIST_CONSTANT = 9675.65;
static const double BEACON_DIST_OFFSET   = -4.56257;
#else
static const double BEACON_DIST_CONSTANT = 8995.2;
static const double BEACON_DIST_OFFSET   = -2.45167;
#endif

static const double GOAL_DIST_CONSTANT = 5433.4;
static const double GOAL_DIST_OFFSET = -0.641;

static const double BALL_DIST_CONSTANT = 1103.94;
static const double BALL_DIST_OFFSET = -0.847627;

// NEWCASTLE BEACON CALIB
static const double NU_BEACON_DIST_CONSTANT = 9123.97;
static const double NU_BEACON_DIST_OFFSET   = -1.97458;

// The number of non-obstacle features that corresponds to a visual confidence
// of MAX_VISUAL_INFO for GPS. Manually guestimated.
static const int MAX_VISUAL_INFO_FEATURES = 120; 

// Above radius BALL_INT_RADIUS_MAX, we use only projection.
// Below radius BALL_PRJ_RADIUS_MIN, we use only interpolation.
// Between them, we use linear sum of both.
//static const int BALL_PRJ_RADIUS_MIN = 18;
//static const int BALL_INT_RADIUS_MAX = 28;  


static const bool calibrateProjectPoints    = false;
static const bool calibrateBeaconDist       = false;

//static const double zoidalVarFactor     = 40.0;
//static const double zoidalAngVarFactor  = 1.5;

//static double lastd = 0;

static const int xOrigin = CPLANE_WIDTH / 2;
static const int yOrigin = CPLANE_HEIGHT / 2;


/*****************************************************************************/

VisualCortex::VisualCortex()
    :   subvision(img),
        subobject(SubObject(subvision, &vob[vobBall], &vob[vobBluePinkBeacon],
                &vob[vobBlueGoal], localisePink, MAX_NO_OF_PINK_OBJECTS))
{
    vFrame = 0;
    outfile = 0;
    currentWalkType = 0;
    cplane = 0;
    lookForBeaconsAndGoals = true;

	initEdgeDetect();
//	initGoalDistanceLookup();
    SanityChecks::init(this);


	OFFLINE_CODE(cout << "VisualCortex.cc: OFFLINE turned on" << endl;);
	ONLINE_CODE(cout << "VisualCortex.cc: OFFLINE turned off" << endl;);
}

void VisualCortex::setDogParams(unsigned long mac) {
    DogColourDistortion colDistort;
	int numFields = sizeof(DogMacLookup)/sizeof(DogMacLookupType);
	bool foundDog = false;
        
	for (int i=0; i<numFields; i++) {
		if (DogMacLookup[i].mac == mac) {
			colDistort = DogMacLookup[i].colMunge;
			FCOUT << "Found Dog Colour Distortion for MAC " << mac << endl;
			foundDog = true;
			break;
		}
	}
	if (!foundDog) {
		colDistort = DogMacLookup[0].colMunge;
		FCOUT << "No color distortion table found for dog.  Using first in table: " /*<< colDistort*/ << endl;
	}

	img.initDCD(colDistort);
}

// Get methods for the sensors (used in method storeObject in Vision.cc)
// pan, tilt, crane all in micro radians.
long VisualCortex::getSensor(int i) {
	switch (i) {
		case 0: return utilt - RAD2MICRO(bodyTilt); // tilt
		case 1: return upan; // pan
		case 2: return ucrane; // crane
	}
	return -1;
}

/*
 *  Resets the variables for a new image feed.
 **/
void VisualCortex::Reset() {
	// reset all visual objects
	for (int x = 0; x < VOB_COUNT; x++) {
		vob[x].reset();
	}
    SanityChecks::clear();
}


/* Init variables relating to pan, tilt and crane, fundamental to
 * projectPoints and sanity checks. Assumes that this->pWalkInfo has
 * been correctly initialised (or 0 for offline)
 */
void VisualCortex::TrigInit(const long &micro_tilt, const long &micro_pan,
                            const long &micro_crane) {
#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex::TrigInit\n"));
#endif

    if (pWalkInfo == 0) {
        cerr << "ERROR: VisualCortex::pWalkInfo is null" << endl;
    }

    pan      = MICRO2RAD(micro_pan);
    crane    = MICRO2RAD(micro_crane);
    tilt     = -MICRO2RAD(micro_tilt);  // Negative because we want to take
                                        // tilting downward as positive. 
#if 0
    // used for testing - makes sure that we don't have errors caused by
    // the physical robot
    pan     = DEG2RAD(*pWalkInfo->desiredPan);
    crane   = DEG2RAD(*pWalkInfo->desiredCrane);
    tilt    = -DEG2RAD(*pWalkInfo->desiredTilt);
#endif

    /* If it is offline, set bodyTilt to 0 because it should have been included
     * in tilt. */
#if 0
    if (pWalkInfo == 0) {

        // Some logs contain bodyTilt, some don't, depending on codes of
        // robot in times of logging.
        static const bool LOG_CONTAIN_REAL_BODY_TILT = true;

        shoulderHeight  = 91;   // Fine-tuned from horizon when standing still.
        hipHeight       = 123;

        if (LOG_CONTAIN_REAL_BODY_TILT) {
            /* bodyTilt should now be included in tilt in cplane logs. */
	        bodyTilt        = 0;
	        degBodyTilt     = 0;
        } else {
            bodyTilt        = asin((hipHeight - shoulderHeight) / BODY_LENGTH);
            degBodyTilt     = RAD2DEG(bodyTilt);
        }

    /* Otherwise get it from pWalkInfo. */
    } else {
#endif
        bodyTilt       = *(pWalkInfo->bodyTilt); // originally radian
        shoulderHeight = *(pWalkInfo->frontShoulderHeight);
        hipHeight      = shoulderHeight + sin(bodyTilt) * BODY_LENGTH;
        degBodyTilt    = RAD2DEG(bodyTilt); // then change to degree
        
#if 0
    }
#endif

    effective_tilt = tilt - crane;      // tilt +ive val is up, but crane +ive
                                        // val is down.

	sin_etilt = sin(effective_tilt);
	cos_etilt = cos(effective_tilt);

	sin_tilt = sin(tilt);
	cos_tilt = cos(tilt);

	sin_pan = sin(pan);
	cos_pan = cos(pan);

    /* Hack: RobotRelative function crane to be in same direction as tilt, so,
    ** here this is. Right now I know sin_crane is only used by RobotRelative.*/
	sin_crane = sin(-crane);
	cos_crane = cos(-crane);

    /* Roll is a combine effect of pan and tilt. Crane doesn't affect much. */
	roll = asin(sin_pan * sin(tilt + bodyTilt));

	if (tilt > M_PI / 2) {
		if (roll < 0) {
			roll = -M_PI - roll;
		}
		else {
			roll = M_PI - roll;
		}
	}

	sin_eroll   = sin(roll);
	cos_eroll   = cos(roll);

	usin_eroll  = sin(-roll); // To unrotate things.
	ucos_eroll  = cos(-roll);

    degPan      = RAD2DEG(pan);
    degTilt     = RAD2DEG(tilt);
    degCrane    = RAD2DEG(crane);
    degRoll     = RAD2DEG(roll);
    degETilt    = RAD2DEG(effective_tilt);

    neckBaseHeight = getNeckBaseHeightFromStance(shoulderHeight, hipHeight);
    
#ifdef OFFLINE
#ifdef VC_DEBUG
    cout <<  "degBodyTilt:" << degBodyTilt << " shoulderHeight:"<<shoulderHeight<< " hipHeight:"<<hipHeight << " neckBaseHeight:" << neckBaseHeight << endl;
    cout << "degPan:" << degPan << " degTilt:" << degTilt << " degBodyTilt:" << degBodyTilt << " degCrane:"<< degCrane << endl;
    cout << "FACE_LENGTH * 10.0:" << FACE_LENGTH * 10.0 << " NECK_LENGTH * 10.0:" << NECK_LENGTH * 10.0 << endl;
    cout << "neckBaseHeight - 0 * 10.0:" << neckBaseHeight * 10.0 << endl;
    cout << "APERTURE_DISTANCE:" << APERTURE_DISTANCE << " CAMERA_OFFSET:" << CAMERA_OFFSET << endl;
#endif    
#endif    
    /* Get the horizon definitions for current pan, tilt and crane. */
    /* It return true if a horizon line can be found, and hzArray will store
    ** the xy coordinates of the 2 points, hzUpIsSky tell be true if pixels
    ** above the hz line is the real up.
    ** If no hz is found, hzAllAboveHorizon means that the whole screen is
    ** above the horizon (everything should be filtered).
     */

    /* Initialise the horizon array to be empty. */
    for (int i = 0; i < 3; i++) {
        hzArray[i] = 0;
    }
    hzExists = getHorizon(hzArray, &hzUpIsSky, &hzAllAboveHorizon,
                            degPan, degTilt + degBodyTilt, degCrane,
                            76.9,
                            80.0,
                            0,
                            APERTURE_DISTANCE,
                            CAMERA_OFFSET,
                            false);

    // Alexn: Note that these points are calculated with positive-y being
    // up, the opposite to the image format, So the gradient below is upside
    // down. The points are relative to the centre of the camera image
    // so hz_c is the x = CPLANE_WIDTH/2 intercept above the image centre
    // in the image.
    // See OffVisionPanel.java
    if (hzExists) { 
        // Manually tuned horizon adjustment to give good vision. Positive is up
        double hzAdjustment = 6 -(ABS(degPan) / 12.00);

        hzArray[1] = hzArray[1] + hzAdjustment; // Change the y-values of
        hzArray[3] = hzArray[3] + hzAdjustment; // horizon.
    }

    /* hzArray: x1, y1, x2, y2. */
    /* m = (y2-y1)/(x2-x1) */
    /* c = m * x1 + y1 (2 point form line-eq) */
    double  x1 = hzArray[0],
            y1 = hzArray[1],
            x2 = hzArray[2],
            y2 = hzArray[3];

    hz_m = (y2-y1)/(x2-x1); // gradient of horizon in frame
    hz_c = -(hz_m * x1) + y1; // y axis intercept of horizon with origin
                               // at image centre

    hz_roll = RAD2DEG(atan(hz_m));

    /* Printing the horizon in offvision to be displayed in OfflineVision. */
    if (flag_OFFLINE && outfile) {
        if (hzExists) {
            (*outfile) 
                << "h " 
                << hzArray[0] << " "
                << hzArray[1] << " "
                << hzArray[2] << " "
                << hzArray[3] << " "
                << hzUpIsSky << " "
                << endl;
        }
    }
#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex: Done TrigInit\n"));
#endif
}


/*----------------------------------------------------*/
/* Check if a given point in cplane is below horizon. x, y are top-left image
 * coordinates. canGoOverBy is measured in pixels.
 */
bool VisualCortex::isBelowHorizon(
        int x, int y, 
        int canGoOverBy) {

    static const string debugHead = "isBelowHorizon| ";
   
    /* Check whether there is an observable horizon. If not then the points we
     * are checking (assumed to be in the image plane) are below the horizon
     * iff not everything visible is above the horizon. */
    if (!hzExists) {
        return !hzAllAboveHorizon;
    }

    /* Now there is horizon, have to construct the line params to check.*/
    double inX = TOPLEFT2CENTER_X(x);
    double inY = TOPLEFT2CENTER_Y(y);

    bool isBelowLine;

    if (ABS(hz_m) <= 1) {
        double hzY_from_inX = (hz_m * inX) + hz_c;

        /* Sometimes the horizon is a bit low, so we want some threshold for
        ** go-over. */
        if ((hzY_from_inX - inY) > -canGoOverBy) {
            isBelowLine = true;
        } else {
            isBelowLine = false;
        }

        /* If up is not sky, then invert the result. */
        return (hzUpIsSky ? isBelowLine : !isBelowLine);

    } else {
        /* When gradient is more than 1, we would like to compare the horizontal
        ** offset instead of vertical offset. */
        double hzX_from_inY = (inY - hz_c) / hz_m;

        /* There are four cases in total. Sky could be up or down, m could be
        ** > or < 0. Better understand it with pictures.*/
        if ((hzUpIsSky && hz_m < 0)
            || (!hzUpIsSky && hz_m >= 0)) {

            if ((hzX_from_inY - inX) > -canGoOverBy) {
                return true;
            } else {
                return false;
            }
        } else {
            if ((inX - hzX_from_inY) > -canGoOverBy) {
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

/*----------------------------------------------------*/
/* Given an x or y c-plane coordinate this calculates the other coordinate, */
/* where it intersects the horizon.                                         */
/* Inputs: in is the x or y coordinate of the intercept                     */
/*         dir = 0 specifies 'in' is a x value, dir = 1 means 'in' is y     */
/* Outputs: out is the other coordinate of the intercept with the horizon   */
/*          returns true if result is a valid c-plane coordinate, false if  */
/*          it is offscreen.                                                */

bool VisualCortex::horizonIntercept(int in, int dir, int *out) {
    int max[2] = {CPLANE_WIDTH, CPLANE_HEIGHT};
    double p;
    
    if (hzExists) {
        if (dir == 0) {
            p = TOPLEFT2CENTER_X(in);
            *out = (int)(CENTER2TOPLEFT_Y(hz_m * p + hz_c) + 0.5);
        } else {
            if (hz_m == 0.0) return false;
        
            p = TOPLEFT2CENTER_Y(in);
            *out = (int)(CENTER2TOPLEFT_X((p - hz_c) / hz_m) + 0.5);
        }
        return ((*out) >= 0 && (*out) < max[1 - dir]);
    }
    return false;
}

/* Given a rotated point, unrotate it back to what it originally is in the
 * cplane. */
inline int VisualCortex::unrotatePointReturnX(double x, double y) {
    return (xOrigin - (int) (  ucos_eroll*(xOrigin - x) 
                             - usin_eroll*(yOrigin - y))
           );
}

inline int VisualCortex::unrotatePointReturnY(double x, double y) {
    return (yOrigin - (int) (  usin_eroll*(xOrigin - x) 
                             + ucos_eroll*(yOrigin - y))
           );
}

// FIXME
void VisualCortex::reloadNNMC(){
    img.loadColourCalibration();
}

#if 0
void VisualCortex::changeCalibration(int type) {
    gcd.changeCalibration(type);
}

void VisualCortex::switchFile(int suff) {
    gcd.switchFile(suff);
}

void VisualCortex::updateCalibration(int col, int row, int newcolour) {
    gcd.updateCalibration(col, row, newcolour, yplane);
}

void VisualCortex::freezeImageProcessing(int i) {
}
#endif

void VisualCortex::getHeadingToBestGap(VobEnum target, double* lMin,
                                    double* lMax, double *rMin, double *rMax) {
    if (vob[target].cf > 0) {
        double left, right;
        if (vob[target].leftGapMin != 0.0 || vob[target].leftGapMax != 0.0) {
            left = pointToHeading(vob[target].leftGapMin, vob[target].cy,
                                            sin_eroll, cos_eroll);
            right = pointToHeading(vob[target].leftGapMax,
                                        vob[target].cy, sin_eroll, cos_eroll);
            *lMin = MIN(left, right);
            *lMax = MAX(left, right);
        } else {
            *lMin = *lMax = 0;
        }

        if (vob[target].rightGapMin != 0.0 || vob[target].rightGapMax != 0.0) {
            left = pointToHeading(vob[target].rightGapMin, vob[target].cy,
                                            sin_eroll, cos_eroll);
            right = pointToHeading(vob[target].rightGapMax, vob[target].cy,
                                            sin_eroll, cos_eroll);
            *rMin = MIN(left, right);
            *rMax = MAX(left, right);
        } else {
            *rMin = *rMax = 0;
        }
    } else {
        *lMin = *lMax = *rMin = *rMax = 0;
    }   
//    cerr << "VisualCortex::getHeadingToBestGap: " << *min << " - " << *max << endl;
}

/* Fills in the array of obstacle points detected this frame. Returns the
 * number of points filled in
 * Points are projected to the field with projectedHeight = 0.
 * Note:points have to be the size of 2*nPoints
 */
int VisualCortex::getObstaclePoints(int *points, int nPoints) {
    vector<VisualFeature>& features = subvision.getObstacleFeatures();
    int count = 0;
    double inPoints[features.size()*2];
    double outPoints[features.size()*2];
    
    for (int i = 0; i < (int)features.size(); ++i) {
        if (features[i].type == VF_OBSTACLE) {
            inPoints[i*2] = TOPLEFT2CENTER_X(features[i].x);
            inPoints[i*2+1] = TOPLEFT2CENTER_Y(features[i].y);
            count++;
        }
    }
    
    projectPoints(inPoints, count, outPoints);
    

    //Filtering the unneccesary ones
    count = 0;
    for (int i = 0; i < (int)features.size() && count < nPoints; ++i) {
        
        if (outPoints[i*2] == INT_MIN || 
            outPoints[i*2 + 1] == INT_MIN || 
            abs(outPoints[i*2]) > MAX_VIEW_ACCURATE_OBSTACLE || 
            abs(outPoints[i*2 + 1]) > MAX_VIEW_ACCURATE_OBSTACLE)
            continue;
        points[count*2] = (int)outPoints[i*2];
        points[count*2+1] = (int)outPoints[i*2 +1];
        ++count;
        //cout << __func__ << "[" << count << "] x:" << outPoints[i*2] << " :y: " << outPoints[i*2 +1] <<endl; 
    }
    return count;
}

int VisualCortex::getVisualConfidence(void) {
    return (featureCount > MAX_VISUAL_INFO_FEATURES) ?
        MAX_VISUAL_INFO :
        featureCount * MAX_VISUAL_INFO / MAX_VISUAL_INFO_FEATURES;
}

void VisualCortex::sendPWalkSensors() {
#ifndef OFFLINE
    struct rlnk_pwalksensors data;

    data.utilt = utilt;
    data.upan = upan;
    data.ucrane = ucrane;
    data.range2Obstacle = range2Obstacle;
    data.leading_leg = *pWalkInfo->leading_leg;
    data.currentWalkType = *pWalkInfo->currentWalkType;
    data.currentStep = *pWalkInfo->currentStep;
    data.currentPG = *pWalkInfo->currentPG;
    data.frontShoulderHeight = *pWalkInfo->frontShoulderHeight;
    data.bodyTilt = *pWalkInfo->bodyTilt;
    data.desiredPan = *pWalkInfo->desiredPan;
    data.desiredTilt = *pWalkInfo->desiredTilt;
    data.desiredCrane = *pWalkInfo->desiredCrane;
    Robolink::sendToBase((byte *)&data, RLNK_PWALK, sizeof(data));
#endif // OFFLINE
}

/* Sends VisualFeatures to robolink */
void VisualCortex::sendFeatures() {
    subvision.sendFeatures();
}

/* Sends VisualObjects to robolink */
void VisualCortex::sendObjects() {
    subobject.sendObjects();
}

/* Sends fired insanities to robolink */
void VisualCortex::sendInsanities() {
    SanityChecks::sendInsanities();
}
/* Process the camera image. This is called by Vision every frame. */
#ifdef OFFLINE
void VisualCortex::processImage(uchar *yuv, long ut, long up, long uc,
                                double r2o, struct PWalkInfoStruct* walk) {
  //bodyMoving = false;
  headMovement = 0.0;
  
#else // OFFLINE
  
void VisualCortex::processImage(const ONotifyEvent &event, long ut,
                                long up, long uc, double r2o,
                                struct PWalkInfoStruct *walk) {
    
#endif // OFFLINE

#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex::processImage\n"));
#endif

#ifdef VALGRIND
  // Mark vob memory as accessible but with undefined data
  VALGRIND_MAKE_WRITABLE(vob, VOB_COUNT * sizeof(VisualObject));
#endif	// VALGRIND
  
    ++vFrame;
    this->utilt = ut;
    this->upan = up;
    this->ucrane = uc;
    range2Obstacle = (int) r2o;    // stands for range to obstacle

    Reset();

    this->pWalkInfo = walk;
    TrigInit(utilt, upan, ucrane);
    /*This should be called before project points, this function for tweaking the tilt for projection. */
    tiltCalibrate();
    
#ifdef OFFLINE
    yplane = yuv;
#else
    // Save only the latest image.
    OFbkImageVectorData *imageVec = reinterpret_cast <OFbkImageVectorData*>
    (event.RCData(event.NumOfData() - 1)->Base());

    OFbkImageInfo *imageInfo = imageVec->GetInfo(0);
    cameraFrame = imageInfo->frameNumber;

    if ((((int)imageInfo->width) != CPLANE_WIDTH) ||
    (((int)imageInfo->height) != CPLANE_HEIGHT)) {
      cerr << "Bad Image Size!!" << endl;
    }
      
    if (imageInfo->type != odataFBK_YCrCb_HPF) {
      cerr << "Bad Image Data Type!!" << endl;
    }
    // pointer to start of actual image data
    // medium resolution index=1
    // high resolution index=0
    yplane = imageVec->GetData(RES);
#endif // OFFLINE

    uplane = yplane + CPLANE_WIDTH;
    vplane = uplane + CPLANE_WIDTH;

    img.setYUV(yplane);

#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex running SubVision\n"));
#endif
    /* Run subvision to sample the image */
    Profile::start(PR_SUBVISION);
    int nFeatures = subvision.sampleImage(hz_c, hz_m, ! hzUpIsSky,
                                            hzExists, hzAllAboveHorizon,
                                            isGrabbed,isGrabbing);
    Profile::stop(PR_SUBVISION);

    // update rolling average of points. 0.93 is selected such that it takes
    // 10 frames of zero information to reduce the count to half of its
    // original value
    featureCount = int(featureCount * 0.93) + int(nFeatures * 0.07);

#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex running SubObject\n"));
#endif
    /* Run subobject to recognise objects */
    for (int i = 0; i < VOB_COUNT; i++) {
        vob[i].reset();
    }

    // Update some trig info in subobject.
    subobject.setParameters(hzExists,
                            pan,tilt,crane,
                            roll,sin_eroll,cos_eroll);
    Profile::start(PR_SUBOBJECT);

#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex running SubObject - finding visual non ball objects\n"));
#endif    
    subobject.findVisualNonBallObjects();
    Profile::stop(PR_SUBOBJECT);

#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex running SubObject - drawing extra scanlines\n"));
#endif      
    subvision.sampleImageExtra();
    
#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex running SubObject - finding visual ball object\n"));
#endif  
    subobject.findVisualBallObject();
     

//    GetSystemTime(&subo_end);

#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex filling objects\n"));
#endif
    
    // Calculate further visual information and projections for vobs
    for (int i = 0; i < VOB_COUNT; ++i) {
        
        if (vob[i].cf <= 0) {
            continue;
        }
#ifdef VC_DEBU
        OSYSDEBUG(("VisualCortex filling objects %d --> d%\n",VOB_COUNT));
#endif    
        vob[i].imgHead = pointToHeading(vob[i].cx);
        vob[i].imgElev = pointToElevation(vob[i].cy);

        if (i == vobBall) {
            // Project ball centre to plane one radius above field
            //projectBall(&vob[i].objectPosition[0], &vob[i].objectPosition[2]);
            //vob[i].dist2 = MAX(BALL_RADIUS, vob[i].dist2);
            //vob[i].objectPosition[0] = -vob[i].objectPosition[0]; // why?
            //vob[i].objectPosition[1] = BALL_RADIUS;
            //vob[i].setPolar();
            // Ball position distance variance varies with distance as
            // projection becomes inaccurate
            //vob[i].var = 10 + (vob[i].dist2 / 500) * 400;
            //vob[i].angleVar = 25; // FIXME?

            // alternate: linear interpolation of radius
            vob[i].dist2 = vob[i].dist3 =
                BALL_DIST_CONSTANT / (vob[i].radius - BALL_DIST_OFFSET);
            RobotRelative(vob[i]);
            vob[i].setPolar();


                        
#if 0
            // Ball variances are affected by the head movement speed
            ball.var += get95CF(ball.d * headMovement);
            ball.angleVar = get95CF(PAN_95CF +  RAD2DEG(headMovement));
            ball.var = (ball.var >= VERY_LARGE_INT) ? VERY_LARGE_INT - 1 : ball.var;
#endif
            // Ball position distance variance varies with distance
            vob[i].var = 25 + (vob[i].dist2 / 500) * 400;
            vob[i].angleVar = 25; // FIXME for head motion?
            
        } else if (i >= vobBluePinkBeacon && i <= vobPinkYellowBeacon) {
            // linear interpolation
            //cout << __func__ << " : height " << vob[i].height << endl;
            vob[i].dist2 = vob[i].dist3 =
                (BEACON_DIST_CONSTANT / (vob[i].height - BEACON_DIST_OFFSET));
//            vob[i].cam_dist = sqrt(SQUARE(vob[i].dist2) + SQUARE(NECK_LENGTH))
//                            - FACE_LENGTH;

            /* The beacon can't be further than the diagonal from a corner to
             * three-quarters along the opposite side, so clip to that length.
             */
            /*vob[i].cam_dist = */vob[i].dist2 = vob[i].dist3 =
                MIN(vob[i].dist3, MAX_BEACON_DIST);

            /* Find the elev/heading/distance relative to dog's neck base. */
            RobotRelative(vob[i]);
            vob[i].setPolar();
            vob[i].var = 625; 
            vob[i].angleVar = 30; // FIXME?
        } else if (i == vobBlueGoal || i==vobYellowGoal){ // goals
            vob[i].dist2 = vob[i].dist3 =
                GOAL_DIST_CONSTANT / (vob[i].height - GOAL_DIST_OFFSET);
//            vob[i].cam_dist = sqrt(SQUARE(vob[i].dist2) + SQUARE(NECK_LENGTH))
//                            - FACE_LENGTH;
            RobotRelative(vob[i]);
            vob[i].setPolar();
            vob[i].var = 900; // 100 FIXME
            vob[i].angleVar = 100; // 100
            //vob[i].var = VERY_LARGE_DOUBLE;
            //vob[i].angleVar = VERY_LARGE_DOUBLE;
        } else {
#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex BUG? VOB on dogs\n"));
#endif
        }
    }

#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex running sanities\n"));
#endif

    /* Check that all the vobs make sense together */
    SanityChecks::checkWorldSanity(vob, VOB_COUNT);

    if (flag_LOC_CHALLENGE) 
        FindPinkLocalisationBlobs();
    
    // debug
#if 0
    if (vFrame % 30 == 0) {
        for (int i = vobBall; i <= vobPinkYellowBeacon; ++i) {
            if (vob[i].cf <= 0)
                continue;
            cout << "Visual " << vobName[i] << " x=" << vob[i].objectPosition[0]
                << " z=" << vob[i].objectPosition[2]
                << " y=" << vob[i].objectPosition[1]
                << " head=" << vob[i].head
                << " d2=" << vob[i].dist2 
                << " d3=" << vob[i].dist3
                << " width=" << vob[i].width
                << " height=" << vob[i].height
                << endl;
        }
    }
#endif

    /* Insert code here to send YUV when we lose track of a vob such as
     * the ball
     */

    /* Run field line mapping */
    bool lookForEdges = true;
#ifndef OFFLINE
    // Only run edge detection when not doing a kick (point projection
    // can be bad due to large roll values.)
    if (*(pWalkInfo->currentWalkType) > AlanWalkWT)
        lookForEdges = false;
#endif //OFFLINE
    //If the tilt less than 0.5 more error on the sensor value so don't use it for edge
    if (degTilt < 0.5)
    {
        lookForEdges = false;
    }
#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex running line mapping\n"));
#endif

    
    Profile::start(PR_LINEMAPPING);
    if (lookForEdges)
        runEdgeDetect(); // poorly named, this maps edges to the field lines
    Profile::stop(PR_LINEMAPPING);

    // reset isGrabbed. Behaviours will set it true if we are still grabbing
    isGrabbed = false;
    isGrabbing = false;
    
//    GetSystemTime(&edge_end);
//    GetSystemTime(&pi_end);
//    if (vFrame % 300 == 0) {
//        cerr << "VisualCortex::ProcessImage took "
//            << (pi_end.useconds - pi_start.useconds) << "us: "
//            << "subvision " << (subv_end.useconds - subv_start.useconds)
//            << ", subobject " << (subo_end.useconds - subo_start.useconds)
//            << ", line mapping " << (edge_end.useconds - edge_start.useconds)
//            << endl;
//    }
#ifdef VC_DEBUG
    OSYSDEBUG(("VisualCortex process image done\n"));
#endif
}
 
/*****************************************************************************/
// return if b1 is completely inside b2
// return if b1 is completely inside b2
// offset grows b2
int VisualCortex::VobInVob(VisualObject &b1, VisualObject &b2, int ot, int or_,
        		   int ob, int ol) {
    return (b1.x >= b2.x - ol && b1.y >= b2.y - ot
            && (b1.x + b1.width) <= (b2.x + b2.width + or_)
            && (b1.y + b1.height) <= (b2.y + b2.height + ob));
}

/* This is for tweaking the tilt value, the number somewhat corespond to the graph.*/
void VisualCortex::tiltCalibrate(){
    if (degTilt < 0)
    {
        TILT_ADJUSTMENT = 0.9* -degTilt;
        //Because the deg is negative then tilt adjustment is positive angle
    }
    else if (degTilt < 3)
    {
        TILT_ADJUSTMENT = 0.5* -degTilt;
    }
    else
    {
        TILT_ADJUSTMENT = -0.0777629*degTilt+ -1.25414;
    }
#ifdef TILT_ADJUSTMENT
    TILT_ADJUSTMENT = 0.0;
#endif

}

/*****************************************************************************/
/* Change the cplane co-ordinates into local co-ordinates base on dog's neck.*/
/* inPoints is the camera coordinate with 0,0 in the middle                  */
void VisualCortex::projectPoints(double *inPoints, int numPoints,
                        double *outPoints, double projectHeight, bool debug) {
    
    getMultiPointProjection(inPoints, outPoints, numPoints, degPan,
                                degTilt + degBodyTilt + TILT_ADJUSTMENT, degCrane, 
                               FACE_LENGTH * 10.0, NECK_LENGTH * 10.0,
                               neckBaseHeight - projectHeight * 10.0,
                               APERTURE_DISTANCE, CAMERA_OFFSET, debug);
    for (int i = 0; i < numPoints; i++) {
        outPoints[i * 2] /= 10.0; //Centimeter
        outPoints[i * 2 + 1] /= 10.0;
#ifdef OFFLINE
        double ipx = inPoints[i*2];
        double ipy = inPoints[i*2 + 1];

        double projectedX, projectedY;

         /* Call getPointProjection in PointProjection.h. */
        getPointProjection(&projectedX, &projectedY, degPan,
                                degTilt + degBodyTilt + TILT_ADJUSTMENT, degCrane, ipx, ipy,
                               FACE_LENGTH * 10.0, NECK_LENGTH * 10.0,
                               neckBaseHeight - projectHeight * 10.0,
                               APERTURE_DISTANCE, CAMERA_OFFSET, debug);
        projectedX /= 10.0;
        projectedY /= 10.0;
        /*
        outPoints[i * 2] = projectedX;
        outPoints[i * 2 + 1] = projectedY;
        */
        if ((projectedX - outPoints[i * 2] > EPSILON) || (projectedY - outPoints[i * 2 + 1] > EPSILON))
        {
            cerr << "Projected point different! |" << (projectedX != outPoints[i * 2]) <<"|" << 
            "projectedX:" << projectedX << " outPointsX:" << outPoints[i * 2] << 
            "|"<< (projectedY != outPoints[i * 2 + 1]) <<  "|" << 
            " projectedY:" << projectedY << " outPointsY:" << outPoints[i * 2 + 1] <<endl;
        }
        
#endif        
    }
}

// These take an extra unneccesary step of copying the whole array.  Should
// delete this sometime. 
void VisualCortex::projectPoints(double *inPoints, int numPoints,
                            int *outPoints, double projectHeight, bool debug) {

    double outPointsDouble[numPoints*2];
    projectPoints(inPoints, numPoints, outPointsDouble, projectHeight, debug);
    for (int i = 0; i < 2 * numPoints; i++) {
        outPoints[i] = (int) outPointsDouble[i];
    }
}

void VisualCortex::projectPoints(int *inPoints, int numPoints,
                        double *outPoints, double projectHeight, bool debug) {

    double inPointsDouble[numPoints*2];
    for (int i = 0; i < 2 * numPoints; i++) {
        inPointsDouble[i] = (double)inPoints[i];
    }
    projectPoints(inPointsDouble, numPoints, outPoints,
                    projectHeight, debug);
}

void VisualCortex::projectPoints(int *inPoints, int numPoints,
                        int *outPoints, double projectHeight, bool debug) {

    double inPointsDouble[numPoints*2];
    double outPointsDouble[numPoints*2];
    for (int i = 0; i < 2 * numPoints; i++) {
        inPointsDouble[i] = (double)inPoints[i];
    }
    projectPoints(inPointsDouble, numPoints, outPointsDouble,
                    projectHeight, debug);
    for (int i = 0; i < 2 * numPoints; i++) {
        outPoints[i] = (int) outPointsDouble[i];
    }
}


/* Returns the robot-relative position of the ball calculated by projecting the
 * centre of the ball circle onto the plane one ball-radius above the field.
 */
void VisualCortex::projectBall(double* x, double* z) {
    double inpoints[2] = {
        TOPLEFT2CENTER_X(vob[vobBall].cx), TOPLEFT2CENTER_Y(vob[vobBall].cy)
    };
    double outpoints[2] = {
        -1, -1
    };
    projectPoints(inpoints, 1, outpoints, BALL_RADIUS);
    *x = outpoints[0];
    *z = outpoints[1];
}

/* Returns the elevation of the lowest point of a visual object. */
double VisualCortex::getMinElevation(VisualObject &obj) {

    double elev1 = pointToElevation(obj.x,            obj.y,            sin_eroll, cos_eroll);
    double elev2 = pointToElevation(obj.x+ obj.width, obj.y,            sin_eroll, cos_eroll);
    double elev3 = pointToElevation(obj.x,            obj.y+obj.height, sin_eroll, cos_eroll);
    double elev4 = pointToElevation(obj.x+ obj.width, obj.y+obj.height, sin_eroll, cos_eroll);

    return MIN(elev1,
                MIN(elev2,
                    MIN(elev3, elev4)));
}


/* Convert VisualObject coordinates from relative-to-camera into
 * relative-to-robot (the point on the ground under the base of the neck).
 * Reference for the Math Calucalation obtained from Computer Graphics comp3421
 * lecture.
 *
 * Inputs are object.dist3 estimated earlier, imgElev and imgHead.
 * Sets objectPosition (x,y,z)
 */
void VisualCortex::RobotRelative(VisualObject &object) {

    double ihead = DEG2RAD(object.imgHead);
    double ielev = DEG2RAD(object.imgElev);
    double x1 = 0;
    double y1 = 0;
    double z1 = object.dist3;

    // Rotation for Image Heading
    double x2 = x1 *cos(ihead) + z1 *sin(ihead);
    double y2 = y1;
    double z2 = -x1 *sin(ihead) + z1 *cos(ihead);

    // Rotation for Image Elevation
    double x3 = x2;
    double y3 = y2 *cos(ielev) + z2 *sin(ielev);
    double z3 = -y2 *sin(ielev) + z2 *cos(ielev);

    // Translation for Face Length
    double x4 = x3;
    double y4 = y3;
    double z4 = z3 + FACE_LENGTH;

    // Translation for Camera Offset
    double x4b = x4;
    double y4b = y4 - (CAMERA_OFFSET / 10);
    double z4b = z4;

    // Rotation for Head Crane
    double x5 = x4b;
    double y5 = y4b *cos_crane - z4b *sin_crane;
    double z5 = y4b *sin_crane + z4b *cos_crane;

    // Rotation for Head Pan
    double x5b = x5 *cos_pan + z5 *sin_pan;
    double y5b = y5;
    double z5b = -x5 *sin_pan + z5 *cos_pan;

    // Translation for Neck Length
    double x6 = x5b;
    double y6 = y5b + NECK_LENGTH;
    double z6 = z5b;

    // Rotation for Head Tilt
    double x7 = x6;
    double y7 = y6 *cos_tilt - z6 *sin_tilt;
    double z7 = y6 *sin_tilt + z6 *cos_tilt;

    // Rotation for Body Tilt
    double x8 = x7;
    double y8 = y7 *cos(bodyTilt) - z7 *sin(bodyTilt);
    double z8 = y7 *sin(bodyTilt) + z7 *cos(bodyTilt);

    // Calculate actual distance and robot relative heading and elevation
    // x8, y8, z8 are relative positions
    
    //cout << __func__ << " : " << x8 << ", " << y8 << ", " << z8 << endl;
    object.setPosition(x8, y8, z8);
}

//
// Below here is disabled code from pre-2005 vision for reference
//
//

#if 0
void VisualCortex::Beacon::Set(
        const HalfBeacon &pink,
        const HalfBeacon &closest) {

    valid = true;
    cx = (pink.cx + closest.cx) / 2.0;
    cy = (pink.cy + closest.cy) / 2.0;
    area = pink.area + closest.area;
    halfmass = pink.area > closest.area ? pink.area : closest.area;

    c2c = sqrt(SQUARE(pink.cx - closest.cx) + SQUARE(pink.cy - closest.cy));

    pinkXsum = pink.xsum;
    pinkYsum = pink.ysum;
    pinkarea = pink.area;
    closestXsum = closest.xsum;
    closestYsum = closest.ysum;
    closestarea = closest.area;


    angle = NormalizeAngle_0_360(RAD2DEG(atan2(pink.cy - closest.cy,
                    pink.cx - closest.cx)));

    xmin = MIN(pink.getXMin(), closest.getXMin());
    ymin = MIN(pink.getYMin(), closest.getYMin());

    xmax = MAX(pink.getXMax(), closest.getXMax());
    ymax = MAX(pink.getYMax(), closest.getYMax());
}
#endif


/*****************************************************************************/
/* Converts large pink blobs into visual objects, for use in the             */
/* localisation challenge.                                                   */
void VisualCortex::FindPinkLocalisationBlobs() {
    //double density;
    bool skip;
    int n = 0;

    for (int i = 0; i < subobject.getNoPinkBlob(); i++) {
        skip = false;
    
        // Calculate robot relative heading and elevation
        if (n != i) 
            localisePink[n].reset();
        localisePink[n].cx = localisePink[i].cx;
        localisePink[n].cy = localisePink[i].cy;
        
        localisePink[n].imgHead = pointToHeading(localisePink[i].cx);
        localisePink[n].imgElev = pointToElevation(localisePink[i].cy);
/*     
        localisePink[n].area = localisePink[i].area;
*/        
        localisePink[n].width = localisePink[i].width;
        localisePink[n].height = localisePink[i].height;    
        localisePink[n].x = localisePink[i].cx - localisePink[n].width / 2.0;
        localisePink[n].y = localisePink[i].cy - localisePink[n].height / 2.0;

        // We don't know the distance to a simple blob, so just use a large
        // distance so that interference due to head position / stance is
        // small.
        localisePink[n].dist3 = 500;
        RobotRelative(localisePink[n]);
        localisePink[n].setPolar();
        
        // Some sanity checks for the pink blobs
        
        // Don't use halfbeacons that are very small
        //if (localisePink[n].area < 30) skip = true;
        if (localisePink[n].width < 3) skip = true;
        if (localisePink[n].height < 3) skip = true;

        // We want solid pink blobs, so remove blobs with low density
        /*
        density = localisePink[n].area;
        density /= (localisePink[n].width * localisePink[n].height);
        if (density < 0.85) skip = true;
        */
        // Don't use pink blob that are inside the main beacons
        for (int j = vobBluePinkBeacon; j <= vobPinkYellowBeacon; j++) {
            if (vob[j].cf || vob[j].var != VERY_LARGE_INT) {
                if (VobInVob(localisePink[n], vob[j], 5, 5, 5, 5))
                    skip = true;
            }
        }
#ifdef OFFLINE        
        cout << __func__ << " skip?" << skip <<endl;
        cout << __func__ << " localisePink :" << i << ":cx:" << localisePink[i].cx << " cy:"<< localisePink[i].cy << endl;
        cout << __func__ << " check for size w:"<< localisePink[n].width << "h:"<<localisePink[n].height <<endl;
#endif        
        if (!skip) n++;
    }
    numLocalisePink = n;
}

/* Sends the visual objects recognised this frame to robolink */
void VisualCortex::sendPinkObjects(void) {
#ifndef OFFLINE
    struct rlnk_pinkallobject{
        VisualObject localisePink[MAX_NO_OF_PINK_OBJECTS];
        int numLocalisePink;
        int padding;
    };
    struct rlnk_pinkallobject pp;
    memcpy(pp.localisePink,localisePink, sizeof(localisePink));
    //pp.localisePink = localisePink;
    pp.numLocalisePink = numLocalisePink;
    Robolink::sendToBase((byte*)&pp, RLNK_PINKOBJECT, sizeof(pp));
#endif
}


/*
 * Copies all the beacon information from beacon[] into vob[].
 * Filter beacons that have been marked invalid. 
 */
#if 0
void VisualCortex::copyBeaconsToVobs() {

    const string debugHead = "copyBeaconsToVobs| "; 
    VisualObject *cv = &vob[vobBluePinkBeacon];

    /* Go through all the 6 beacons. */
    for (int i = 0; 
         i < 6;
         i++, cv++) {

		if (!beacon[i].valid) {

			if (debugCopyBeaconsToVobs && flag_OFFLINE && !flag_PRE_SANITY_BLOB) {
                (*outfile) << debugHead
                    << "...classified invalid, beacon[" 
                    << i << "]" << endl;
            }
            cv->reset();
			fireSanity(INVALID_BEACON + i);
			continue;

		} 
        
        cv->x      = beacon[i].xmin;
        cv->y      = beacon[i].ymin;
        cv->width  = beacon[i].xmax - beacon[i].xmin;
        cv->height = beacon[i].ymax - beacon[i].ymin;

		cv->cx = beacon[i].cx;
		cv->cy = beacon[i].cy;

		cv->projected_x = cv->x;
		cv->projected_y = cv->y;
		cv->projected_w = cv->width;
		cv->projected_h = cv->height;

		cv->area = beacon[i].area;
		cv->tilt = beacon[i].angle;
		cv->imgHead = pointToHeading(beacon[i].cx);
		cv->imgElev = pointToElevation(beacon[i].cy);

        cv->cam_dist = cv->dist3 = beaconDistanceConstant / beacon[i].c2c + beaconDistanceOffset;

		/* Write only our interested attribute for calibration. */
        if (calibrateBeaconDist && flag_OFFLINE) {
            (*outfile) << beacon[i].c2c << endl;
        }

		//ERIC these printouts are all for centroid testing
		cv->c2c = beacon[i].c2c;
		cv->pinkXsum = beacon[i].pinkXsum;
		cv->pinkYsum = beacon[i].pinkYsum;
		cv->pinkarea = beacon[i].pinkarea;
		cv->closestXsum = beacon[i].closestXsum;
		cv->closestYsum = beacon[i].closestYsum;
		cv->closestarea = beacon[i].closestarea;

		int halfExpectedArea = (int) SQUARE(beacon[i].c2c);

        /* Avoid divide by 0 below. */
        if (halfExpectedArea == 0)
            halfExpectedArea = 1;
        
        cv->misc = beacon[i].c2c;
        cv->cf = MIN((1000 * beacon[i].area) / (halfExpectedArea * 2), 1000);
        cv->var = exp(2 * BEACON_C + 2 * BEACON_M * cv->dist3)
                  + SQUARE(varConst * (1 - beacon[i].area / (double) (halfExpectedArea * 2)));

        // This is the variance of the beacon angle.
        cv->angleVar = 25 / (beacon[i].area / (double) (halfExpectedArea * 2));


        /* Find the elev/heading/distance relative to dog's neck base. */
        int startBeacon = vobBluePinkBeacon;
        RobotRelative(vob[startBeacon+i]);

        /* The beacon can't be further than the field diagonal length. */
		cv->cam_dist = cv->dist2 = cv->dist3 = MIN(cv->dist3, 530);

	    if (debugCopyBeaconsToVobs && flag_OFFLINE) {

            (*outfile) << debugHead
                << "...Beacon (" << vobName[startBeacon+i] << ") "
                << "identified, with elev " << vob[startBeacon+i].elev
                << ", distance: " << vob[startBeacon+i].dist2
                << endl;
		}
    }
}
#endif

