/* Copyright 2003 The University of New South Wales (UNSW) and National  
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
 * $Id: VisualCortex.h 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Object identification from blobs
 * 
 **/


#ifndef VisualCortex_h_DEFINED
#define VisualCortex_h_DEFINED


#include <iostream>
#include <fstream>
#include <cstdio>
#include "../share/Common.h"
#include "../share/VisionDef.h"
#include "../share/minimalMatrix.h"
#include "../share/SwitchBoard.h"
#include "CorrectedImage.h"
#include "VisualObject.h"
#include "SubVision.h"
#include "SubObject.h"

using namespace std;

class ONotifyEvent;
struct WMObj;

struct matchValue {
    float match;
    float dx;
    float dy;
};

struct PWalkInfoStruct {
    int *leading_leg;
    int *currentWalkType;
    int *currentMinorWalkType;
    int *currentStep;
    int *currentPG;
    double *frontShoulderHeight;
    double *bodyTilt;
    double *desiredPan;
    double *desiredTilt;
    double *desiredCrane;
    bool *isHighGain;
  unsigned int* stepID;
};

/* Edge types */
static const int LINE_EDGE   = 0;
//static const int BORDER_EDGE = 1;
static const int FIELD       = 2;

#ifdef SHAPE_CHALL
/******************************/
// defines for Shape Challenge

typedef enum {
    Triangle,
    Rectangle,
    Square,
    TShape,
    LShape
}
ShapeEnum;

//static const int SHAPE_COUNT = 5;
//static const int BACKGROUND = 8;  //BACKGROUND
//static const int FILL_COLOUR = 6; //cROBOT_RED
//static const int BORDER_COLOUR = 5; //cROBOT_BLUE

/******************************/
#endif

typedef enum {
    vobBall             = 0,
    vobBlueGoal,
    vobYellowGoal,
    vobBluePinkBeacon,
    vobPinkBlueBeacon,
    vobYellowPinkBeacon,
    vobPinkYellowBeacon,
    vobRedDog,
    vobRedDog2,
    vobRedDog3,
    vobRedDog4,
    vobBlueDog,
    vobBlueDog2,
    vobBlueDog3,
    vobBlueDog4,
    VOB_COUNT,
}
VobEnum;


static const char *vobName[] = {
    "ball",
    "blue goal",
    "yellow goal",
    "blue on pink beacon",
    "pink on blue beacon",
    "yellow on pink beacon",
    "pink on yellow beacon",
    "red dog",
    "red dog",
    "red dog",
    "red dog",
    "blue dog",
    "blue dog",
    "blue dog",
    "blue dog",
};


class VisualCortex {
public:

    VisualCortex();
    ~VisualCortex() {
        (void)vobName; // silence compiler warning
        shutdownEdgeDetect();
    }


    void setDogParams(unsigned long mac);

#ifdef OFFLINE
    void processImage(uchar *yuv,
                      long upan,
                      long utilt,
                      long ucrane,
                      double r2o,
                      struct PWalkInfoStruct *walk);
#else // OFFLINE
    void processImage(const ONotifyEvent &event,
                      long upan,
                      long utilt,
                      long ucrane,
                      double r2o,
                      struct PWalkInfoStruct *walk);
#endif // OFFLINE

    /* Check if a given point in cplane is below horizon. */
    /* Format of cplane: (0,0) in top-left corner. */
    bool isBelowHorizon(int x, int y, int canGoOverBy = 0);
    inline bool isBelowHorizon(double x, double y, int canGoOverBy = 0) {
        return isBelowHorizon((int)x, (int)y, canGoOverBy);
    }

    /* Returns elevation of bottom of object */
    double getMinElevation(VisualObject &obj);
    
    /* Finds the intersection of a vertical or horizontal line with horizon. */
    bool horizonIntercept(int in, int dir, int *out);

    /* Sets min and max to the heading range of the best gap in the
     * goal vob, or zero if the goal is not in sight
     */
    void getHeadingToBestGap(VobEnum target, double* lMin, double* lMax,
                                            double* rMin, double* rMax);
    
    /* Fills in an array of int with the odd index is y and the even index is x 
     * with obstacle points detected this frame. The array must be of size
     * at least 2*nPoints. Returns the number of points actually filled in.
     */
    int getObstaclePoints(int *points, int nPoints);

    /* Returns a number in the range 0 to MAX_VISUAL_INFO indicating how 
     * much visual information has been received (i.e. processed from the
     * image) recently. If it's MAX_VISUAL_INFO then we have plenty
     * to go on
     */
    int getVisualConfidence(void);
    
    /* Returns a number of field features */ 
    int getFieldFeatureCount() { return subvision.getFieldFeatureCount(); }
    int getBallFeatureCount() { return subobject.getBallFeatures().size(); }

    /* Send camera orientation stuff to robolink */
    void sendPWalkSensors();
    void sendFeatures();
    void sendObjects();
    void sendInsanities();
    void sendPinkObjects();
    
    void RobotRelative(VisualObject &object);  // ANdres make this private!

    // get methods for the sensors (used in method storeObject in Vision.cc)
    long getSensor(int i);

    int countColouredPixels(int colour, int minX, int minY, int maxX, int maxY);
   

    //// Data ////
    // Vision frame, incremented manually every camera frame we process.
    // Approx 30/sec
    int vFrame;
    
    struct PWalkInfoStruct *pWalkInfo;
    
    VisualObject vob[VOB_COUNT];

    bool lookForBeaconsAndGoals;

    /* Declare at all time so that codes can be compiled at all times. */
    ofstream *outfile; // but it's for OFFLINE only.

    // Andrew: This is for the localisation challenge, stores largest
    // pink blob as a VisualObject.
    VisualObject localisePink[MAX_NO_OF_PINK_OBJECTS];
    
    int numLocalisePink;

    CorrectedImage img;
    uchar *yplane, *uplane, *vplane, *cplane; // FIXME: remove this
//    SegmentedPlane* splane;
//    eSegmentedPlane* esplane;
    uchar* edgePlane;

    // Frame ID reported by OPEN-R. This actually counts actuator frames
    // (8ms), of which we get four per vision frame
    long cameraFrame;


    /*------------------------------*/
    double shoulderHeight, hipHeight;   // mm's
    double neckBaseHeight;              // mm's

    double crane, degCrane;             // +ive up, -ive down.
    double pan,   degPan;               // +ive left, -ive right.
    double tilt,  degTilt;              // +ive up, -ive down (original,
                                        // but in TrigInit we reversed it)

    double bodyTilt, degBodyTilt;       // +ive when back is higher. If offline,
                                        // set to 0 because already included in
                                        // tilt. If online, get from PWalkInfo.

    double roll,  degRoll;              // The rotation of cplane,
                                        // combined effect by pan+tilt.

    double effective_tilt, degETilt;    // For ERS7, effective tilt is the
                                        // combine effect of tilt and
                                        // crane, +ive down, -ive up.

    double sin_eroll, cos_eroll;        // values for unrotating the image
    double usin_eroll, ucos_eroll;      // values for rotating image back to normal

    double sin_tilt,  cos_tilt;
    double sin_pan,   cos_pan;          // All rad's, to save duplicated
    double sin_crane, cos_crane;        // calls.
    double sin_etilt, cos_etilt;

    double hzArray[4];      // Define the horizon using two points.
    bool   hzExists;        // If a horizon line exists in cplane.
    bool   hzUpIsSky;       // When this is true, this means cplane region
                            // "above" horizon line is the sky.

    double hz_m, hz_c;      // The gradient and offset of the horizon.

    double hz_roll;         // Should be similar to degRoll except this
                            // one is calculated from horizon. Should be
                            // more accurate because of adjusted bodyTilt.

    bool   hzAllAboveHorizon;   // When no hz is found, either all
                                // above or all (whole screen) below horizon.

    /*------------------------------*/

    // set by behaviours when we are grabbed. Resets to false each frame
    bool isGrabbed;
    bool isGrabbing;

    // pointer to the current walk type the dog is performing
    const int *currentWalkType;

    int player;

#if 0
    // Keep these information relative to the camera as
    // visual objects get changed in remembrate to robot relative
    // new variables added in VisualObject eliminates the need for these
    //long ballHead, ballElev, bGoalHead, bGoalElev, yGoalHead, yGoalElev;

    void changeCalibration(int type);
    void switchFile(int suff);

    // Update the calibration dynamically via wireless.
    void updateCalibration(int col, int row, int newcolour);

    // Subroutine to determine whether to process the image or not.
    void freezeImageProcessing(int i);
#endif

    void projectPoints(double *inPoints,
                       int numPoints,
                       double *outPoints,
                       double projectHeight = 0, bool debug = false);
    void projectPoints(double *inPoints,
                       int numPoints,
                       int *outPoints,
                       double projectHeight = 0, bool debug = false);
    void projectPoints(int *inPoints,
                        int numPoints,
                        double *outPoints,
                        double projectHeight = 0,
                        bool debug = false);
    void projectPoints(int *inPoints, int numPoints,
                        int *outPoints, double projectHeight = 0,
                        bool debug = false);

    static const int MAX_EDGE_PIXELS = 256;

    // Edge detection output
    int numEdgePnts;
    int numEdgeLinePnts;
    int numEdgeFieldPnts;
    double totalLineWeight, totalFieldWeight;
    
    double calcRobotMatch(MVec3 &, MVec3 &);
    void calcEdgeCentre(MVec2 &);

    double globalEdgePnts[MAX_EDGE_PIXELS * 2];
    double localEdgePnts[MAX_EDGE_PIXELS * 2];

    // Edge detection inputs
    double headMovement;
    bool bodyMoving;
    void invalidateEdgeMap();

    //Localisation Challenge
    void FindPinkLocalisationBlobs();
    
#ifdef OFFLINE
    ostream *outputStream;
#endif    

private:
    /* Edge detection prototypes */
    void initEdgeDetect();
    void shutdownEdgeDetect();
    void initGradField();
    void mirrorGradient(matchValue *);
    double distanceSq(int, int, int, int);
    double calcPointMatch(double, double, int, double *, double *);
#ifdef OFFLINE        
    void calcEdgeWeights(double &reduceDist, double &cullDist, int &minPoints, double &minFieldDist);
    void runEdgeDetect(double reduceDist = 4900.0, double cullDist = 40000.0, int minPoints = 4, double maxheadspeed = 0.09, double minFieldDist = 25.0);
#else
    void calcEdgeWeights();
    void runEdgeDetect();
#endif //OFFLINE    
    void stationaryMap();
    bool shouldMerge(double, double, int, int, int *);
    bool tryAddEdge(double, double, int, int *, int);
    int findFieldEdges();
    //moved to common.h
    //void localToGlobal(MVec3 &robot, int, double *, double *);

    // Edge detection constants
    static const int offsetX = 120;  // How far match function extends
    static const int offsetY = 120;  // beyond beacons
    static const int ARRAY_WIDTH = FIELD_WIDTH / 2 + 2 * offsetX + 1;
    static const int ARRAY_HEIGHT = FIELD_LENGTH / 2 + 2 * offsetY + 1;
    static const double INFINITY_GRADIENT_VALUE = 0;
    static const double groundPlane = 0.0;

    // Edge detection variables
    double edgeList[MAX_EDGE_PIXELS * 2];
    int edgePntTypes[MAX_EDGE_PIXELS];
    int cplaneEdgePnts[MAX_EDGE_PIXELS * 2];
    int mapPntTypes[MAX_EDGE_PIXELS];
    int mapPntConfidence[MAX_EDGE_PIXELS];
    double mapPnts[MAX_EDGE_PIXELS * 2];
    double edgeWeights[MAX_EDGE_PIXELS];
    matchValue *matchLineTable;
    matchValue *matchFieldTable;
    
    int numMapPnts;
    bool mapInvalid;
    
    friend class FormSubvisionImpl; // allow offline tools private access
    friend class SanityChecks;
    
    SubVision subvision;
    SubObject subobject;
    int featureCount; // rolling average of detected features

public:

    /*
     * allow dynamic reloading of NNMC
     */
    void reloadNNMC();

    // empty function for GDB debugging
    void VisualCortex::test() {}
    
private:
    int VobInVob(VisualObject &b1,
                 VisualObject &b2,
                 int ot,
                 int or_,
                 int ob,
                 int ol);

    void TrigInit(const long &micro_tilt,
                  const long &micro_pan,
                  const long &micro_crane);

    void Reset();

    bool InYellowNoGreen(VisualObject &ball);

    void projectBall(double* x, double* z);
    inline int unrotatePointReturnX(double x, double y);
    inline int unrotatePointReturnY(double x, double y);

    long upan;
    long utilt;
    long ucrane;
    int range2Obstacle;
    double TILT_ADJUSTMENT;
    void tiltCalibrate();
//    DistanceCalibrator *ballCalibrator;



#if 0
    // Struct for a beacon type
    class Beacon {
    public:

        bool valid;
        double c2c;  // c2c distance of the 2 halfbeacons making up the beacon
          
        double pinkXsum;
        double pinkYsum;
        double pinkarea;

        double closestXsum;
        double closestYsum;
        double closestarea;

        double cx, cy;
        int    xmin, xmax, ymin, ymax;
        int    area;

        double angle;
        int halfmass;              // area of larger halfbeacon

        void Set(
            const HalfBeacon &pink,
            const HalfBeacon &closest
        );


        Beacon()
        {
            xmin = -1;
            xmax = -1;
            ymin = -1;
            ymax = -1;
            cx = -1;
            cy = -1;
        }

    }; // class Beacons
    int FindVisualObjects();

    
    

    void FindBeacons();
    void BeaconsSanities();
    void copyBeaconsToVobs();
    bool NoGreenNearBy(int xmin, int xmax, int ymin, int ymax);
    bool NoColourXNearBy(int colour,
                         int xmin,
                         int xmax,
                         int ymin,
                         int ymax,
                         int deltaX,
                         int deltaY,
                         int threshHold);

    void formBeaconFromPink(HalfBeacon *pink,
                            int colorIndex,
                            HalfBeacon **hbArray,
                            int hbCount,
                            HalfBeacon **closest);

    inline bool goalSurroundingCheck(
        int goalX,
        int goalY,
        int goalWidth,
        int goalHeight);


    inline void throwAwayOneBeacon(
        VisualObject *beaconYR,
        VisualObject *beaconBR);


    inline void recalPinkBlobCentroid(
        HalfBeacon *pink,
        HalfBeacon *other);

    inline bool beaconHasWhiteBelow(
        int beaconX,
        int beaconY,
        int oldBeaconMinX,
        int oldBeaconMaxX,
        int oldBeaocnMaxY,
        int beaconWidth,
        int beaconHeight
    );

    inline bool isBeaconPartiallySeen(
        HalfBeacon *pink,
        HalfBeacon *other);

    inline void recalConfusedBeaconBlobCentroid(
        HalfBeacon *pink,
        HalfBeacon *other,
        int        goodBlobColor,
        int        nonPinkBlobColor);

    inline void calBeaconEdgePoint(
        double &outX,
        double &outY,
        int cx,
        int cy,
        double mBeacon,
        bool goodBlobIsUp,
        bool goodBlobIsLeft,
        int goodBlobColour);

    inline bool isBeaconBlobMergedWithBackground(
        HalfBeacon *pink,
        HalfBeacon *other,
        double c2c,
        bool relaxThreshold);

    void FindGoal();
    void LookForColourGoal(HalfBeacon **hbArraye,
                           int hbCount,
                           int beaconLeftOfGoalIndex,
                           int beaconRightOfGoalIndex,
                           int goalIndex);

    void GoalSanities();

    void LandmarkSanities();

    void FindBall();
    bool isBallBeaconNoise(VisualObject& ball);
    bool normalFindBall(HalfBeacon *preball, VisualObject &ball);
    bool andresFindBall(HalfBeacon *preball, VisualObject &ball);

    int FindHalfBeacons(int colorIndex,
                        HalfBeacon **hbArray,
                        int maxHB,
                        int threshold);

    bool BallSanities(VisualObject &,
                      int,
                      int,
                      int,
                      int,
                      double,
                      double,
                      double,
                      double);

    void FindRobot();
    void FindColorRobot(int colour);
    void RobotNoiseFilter();
    bool isRobotTooFarAwayFromGoal(double goalDist, double
                                   robotDist);
    // used to draw lines between centroids of blobs and then returns
    // the number of 'colour's seen on the line
    inline int calDistBaseOnColor(int x1, int y1, int x2, int y2,
                                  HalfBeacon *sourceBlob, uchar colour,
                                  bool skipBoundary = true);

#endif
//    double NewBallDistance();

//    bool DarkBackgroundInBetween(int left, int right, int top, int bottom);
    //void FindColorRobot (int vobNum);

//    void PrintBeaconInfo();

//    void convertGoalBeacon();


public:
    //  int CreateRunLengthInfo(int start, RunLengthInfo& info);
//    Blobber *blobs;

    // This variable is deprecated but used by a few behaviours.
    // Initialised only by C++ default (false)
//    bool facingFieldEdge;


    //bool cmdAdjustDistance;   // distance calibration testing

    // gets rid of recognizing carpet as beacon
//    int beaconSearchPerim;
//    double beaconErrorThreshold;
//    int filterInnerBeacon;

//    double errorScale;   // scale of partially seen obj

//    double BEACON_M;

    // the constant to use for variance adjustance of partially seen objects
//    double varConst;

//    double minGoalDim;
//    int numPink, numGreen, numBlue, numYellow;

    /* These are numbered from left to right, top to bottom with the 'top'
     * of the field being the blue goal, and the 'bottom' being the yellow 
     * goal. ie #0=blue/pink, and #5=pink/yellow. */
#if 0
    Beacon beacon[numBeacons];


    HalfBeacon *hbPink[kMaxPinkBlobs];
    HalfBeacon *hbBlue[kMaxBlueBlobs];
    HalfBeacon *hbYellow[kMaxYellowBlobs];
    HalfBeacon *hbGreen[kMaxGreenBlobs];
    HalfBeacon *hbBall[kMaxOrangeBlobs];
#endif
};



// Calculates the angle (radians) subtended by a run of the given number of
// pixels. This is based on the *average* degrees/pixel over the image but
// should be accurate to about 0.01 per pixel
inline double pixelsToAngle(double nPix) {
    return ANGLE_PER_PIXEL * nPix;
}

#endif // VisualCortex_h_DEFINED



