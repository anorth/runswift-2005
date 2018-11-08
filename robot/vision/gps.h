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
 * $Id: gps.h 2411 2004-02-14 15:42:34Z tedwong $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Localisation of the robots
 *
**/

#ifndef GPS_h_DEFINED
#define GPS_h_DEFINED


#include "VisualCortex.h"
#include "CommonSense.h"
#include "Vector.h"
#include "../share/Common.h"
#include "../share/FieldDef.h"
#include "../share/minimalMatrix.h"
#include "../share/SwitchBoard.h"
#include "gpsAux.h"
#include "Robolink.h"

#ifdef LOCALISATION_CHALLENGE
#include <map>
#endif

//#include "KalmanInfo2D.h"

//#define NEW_LINE_UPDATE
//#define VISUAL_TRACK_FRIENDS


class KalmanInfo2D;
class KI2DWithVelocity;
struct WMShareObj;

static const double INVALID_FLAG = -1234; //set the hVar of the teammate to this number
//to indicate that he doesnt exist
//WARNING... if you change this number, change it in robocommander too

// these are constants to determine how to deal with a particular vob
// if you ever change these make sure you go through the code and change
// all the places that depend on these
// eg they might do vobMinFixed + 1
// see VisualCortex.h
static const int vobMinFixed = vobBlueGoal; // first fixed-place vob
static const int vobMaxFixed = vobPinkYellowBeacon; // ... and last

// returned by VisualCortex::getVisualConfidence when it has plenty of
// information
static const int MAX_VISUAL_INFO = 1000;

using namespace std;

typedef const long coordArray[6][2];

class GPS {
  public:

    GPS();
    ~GPS();

    bool sendCPlane;
    bool sendDebug;
    int debugCounter;
    int debugParm;

    bool doGPSMotionUpdate;
    bool doGPSVisionUpdate;
    bool doGPSOpponentUpdate;

    // appending gps information 
    int storeGps(unsigned char *add, int base);

    // restoring gps information
    int restoreGps(unsigned char *add, int base);

    // outputs gps self information
    void GPSOutputSelfData();
    
    //Send the whole gaussians to robolink
    void sendGaussians();
    /////////////////////////////////////////////////////////
    // The methods for performing updates on all localisation
    /////////////////////////////////////////////////////////

    //Reset the gaussians with 1 gaussian wiht the given position and heading
    void resetGaussians(double x = FIELD_WIDTH/2, double y = FIELD_LENGTH/2,
                        double h = 90, double covX = 100, double covY = 900,
                        double covH = 900);

    // Reset the ball to the given x, y, velocity
    void resetBall(double x, double y, double dx, double dy);

    // sets the position and covariance of the robot
    void GPSSetVals(double newx,
                    double newy,
                    double newh,
                    const MMatrix3 &newC);

    // performs estimation update according to movement
    void GPSMotionUpdate(double dForward, double dLeft, double dTurn, long PWM, bool debug = false, bool isTurnNeckBase=false);

    // performs correction update according to a set of vobs
    void GPSVisionUpdate(VisualObject *nvo, bool isHeadMoving = false);

    // performs correction update according to field lines seen
    void GPSEdgeUpdate(VisualCortex *vc, double headSpeed);

    // Informs GPS of the amount of visual information being recieved
    void GPSVisualConfidenceUpdate(int);

    // Tries to find the max edge match, and sets gaussian array to possibles
    void GPSGlobalMaxEdgeSet(VisualCortex *vc);

    // performs correction update according to a direct estimation of the
    //  robot's position.
    void GPSDirectPositionUpdate(double x, double y, double h,
        double xvar, double yvar, double hvar);

    // performs correction update according to a multi-modal estimation of the
    //  robot's position.
    void GPSDirectPositionUpdate(int numObs, double *x, double *y, double *h,
        double *xvar, double *yvar, double *hvar, double *weights);

	// performs correction update for an n dimensional non-linear measurement
	// defined by the jacobians, innovation vector and variances
	template <int n> void kalmanUpdate(SingleGaussian *g, double *jacWRTX, double *jacWRTY, double *jacWRTH, double *iv, double *vars);

    // Sets the colour of the robot, and hence the coordinate system
    // long should be mouth position
    void SetGoals(bool);

    // MapLocalisePink
    // records the location of pink blobs, while LocalisePinkUpdate uses
    // the recorded pink to localise off.
    void MapLocalisePink(VisualObject *pvo, int numPinkVob);
    void LocalisePinkUpdate(VisualCortex * vision);
    bool CalcWallIntercept(VisualObject *pvo, double *iX, double *iY);
    void ReducePinkConfidence(int minConf, int decreaseAmount);
    void ResetPinkMapping(void);
    bool allowPinkMapping, pinkUpdate;
#ifdef LOCALISATION_CHALLENGE
    void MapPinkEquations(VisualObject *pvo, int numPinkVob) ;
    bool CalcLineEquation(VisualObject *pvo, double *m, double *c, int *type);
    void CalcIntersections();
    void GPSOffFieldUpdate(VisualCortex *vc);
    void ResetPinkUpdate();
    bool bodyMoving;
#endif

    //////////////////////////////////
    // Access methods for localisation
    //////////////////////////////////

    // to set whether robot is currently penalised or not
    void setPenalised(bool p);

    // invalidate the ball and move it to a likely drop-in point. outByOpponents
    // is true if the opponents touched the ball over the line. If the ball
    // was seen this frame then this has no effect.
    void setBallOut(bool outByOpponents);

    // access for self localisation
    const WMObj &self();
    const MMatrix3 &selfCov();
    unsigned int &sharedBehaviour();
    const MVec3 &getVisualPull();

    // this is the pull for the ball velocity
    MVec3 vbPull;

    // access for ball localisation
    const Vector &getBall(int context);
    const Vector &getVBall(int context);
    double getBallAngle(); // [0,180], the balls velocity angle (0 at robot)
    const MMatrix2 &getBallCov();
    double getBallMaxVar();
    const MMatrix2 &getVelCov();
    double getVBallMaxVar();
    double getVBIVLength();
    MMatrix2 getVCLocal();
    MMatrix2 getVCBall();

    //const MMatrix4& bCov();

    // access for teammates based on their shared information
    WMObj &teammate(int botNum, char origin = 'l'); //access 1-4
    const WMObj &tmObj(int arrIndex); //access 0-3
    const WMObj &tmBallObj(int arrIndex); //access 0-3
    int vobMinOpponent;
    int vobMaxOpponent;
    int vobMinFriend;
    int vobMaxFriend;

    // access for opponents and teammates based on visual information
    // oppNum is 0..3
    const Vector &getOppPos(int oppNum, int context = LOCAL);
    const MMatrix2 &getOppCov(int oppNum);
    double getOppCovMax(int oppNum);
    const Vector &getClosestOppPos(int context, int *oppNum = NULL);
#ifdef VISUAL_TRACK_FRIENDS
    const Vector &getFriendPos(int friendNum, int context = LOCAL);
#endif // VISUAL_TRACK_FRIENDS

    // access for other objects
    WMObj &oGoal();
    WMObj &tGoal();

    void increaseSelfAngleVariance(double amount);

    //////////////////////////////////
    // methods for world model sharing
    //////////////////////////////////

    // returns the information to be shared with other teammates
    WMShareObj getShareInfo();

    // processes information recevied from a teammate
    void processShareInfo(int sourceBot, const WMShareObj &info);


    // coordinates of non-moving objects
    coordArray *coord;

    // returns true if it can "see" (ie - using for localisation) the
    // given object, according to indexing in visual cortex.
    int canSee(int object);

    // representiation of which ball, goals & beacons it can see
    // really just 16 booleans in order from least sig bit to most sig
    // 7 most significant not used
    // bit according to vob variables in VisualCortex.h
    int view;

    // representation of direct update values
    // will all be -1 if not utilised
    double directx, directy, directh;

    // These hold the last motion update values
    double motionForward, motionLeft, motionTurn;
    int motionFCounter;
    long motionPWM;

    //  These hold the previous motion update values
    double prevMotionForward, prevMotionLeft, prevMotionTurn;
    int prevMotionFCounter;
    long prevMotionPWM;
    
    // ross's debug stuff - old localisation for debug displaying
    double oldx, oldy, oldh;
    double oldvx, oldvy, oldvh, oldcxy, oldcxh, oldcyh;

    int noBallFrames; //number of camera frames since dog last saw the ball

    // best ball position shared by teammates
    double shareBallx;
    double shareBally;
    double shareBallvar;
    int sbRobotNum;

    // true if setGoal has not been called and hence we don't know which
    // direction we are going, false otherwise
    bool goalNotSet;

    // vob numbers for specific objects
    VobEnum targetGoal, ownGoal;    // the goal the robot is kicking towards
    VobEnum backLeftBeacon;
    VobEnum backRightBeacon;
    VobEnum farLeftBeacon;
    VobEnum farRightBeacon;

    // if true, will set a value in the information sent to other robots
    // such that the latency test will be done
    bool doLatTest;

    // Whether field line data was used to update self gps
    bool useLineData;
    
    // For localisation challenge
    bool landmarksInitialised;

  private:  
    friend class FormSubvisionImpl; // allow offline tools private access
  
    // If true this means that the robot is penalised and as such shouldn't
    // be doing any updates at all
    bool isPenalised;
  
    // For localisation challenge
#ifndef LOCALISATION_CHALLENGE
    static const int maxPink = 10;
#endif    
    double pinkX[maxPink], pinkY[maxPink];
    int pinkConf[maxPink], numPink, pinkDecreaseCounter;
#ifdef LOCALISATION_CHALLENGE
    vector< PinkEQ > listPinkEQ;
    void PinkLineToBox(vector< PinkEQ > &listPinkEQ, int wholeField[PINK_ARRAY_BOX_SIZE_X][PINK_ARRAY_BOX_SIZE_Y]);
    void PinkBoxBlobbing(int wholeField[PINK_ARRAY_BOX_SIZE_X][PINK_ARRAY_BOX_SIZE_Y], int boxPosX, int boxPosY, int minimum, PinkBlob *pb, int from);
    void PinkFindBlobBox(vector< PinkBlob > &listPinkBlob, int wholeField[PINK_ARRAY_BOX_SIZE_X][PINK_ARRAY_BOX_SIZE_Y]);
    int PinkFindPosition(vector< PinkBlob > &listPinkBlob, double *posPinkX, double *posPinkY, int *posPinkConf, int maxPosPink);
    void SortAndCalculatePink();
    void PinkCalcAngles();
    void StationaryMapAndLocalisePinkAngle(VisualCortex *vision);
    double distanceBetweenPink[maxPink];
    //Store all the box position and store the info in each box.
    PinkAngleBox angleBoxPosition[ ANGLE_ARRAY_SIZE_X * ANGLE_ARRAY_SIZE_Y];
    //To hold 180 angle from 0 to 180
    //Each angle have corresponding box that have that angle.
    std::map < int, vector < PinkAngleBox * > >  anglelist; 
    double stationaryRecordedAngle[maxPink];
    int stationaryRecordedAngleConf[maxPink];
    int numStationaryRecordedAngle;
#endif

    // The robot position is determined by a weighted sum of gaussians,
    // This is the maximum number of gaussians in that sum
    static const int maxGaussians = 16;

    // Current number of modes in the distribution
    int numGaussians;

    // These track weight scaler statistics
    int nX[10];
    double sumX[10], sumXSquared[10];

	// This contains the probability distribution of the robot's position and
	//  heading. Each array element is a single gaussian with the WMObj and
	//  MMatrix3 representing the mean values and covariance matrix (x, y, h)
	//  respectively. The distribution is the weighted sum of all array
	//  elements. See gpsAux.h for definition of 'SingleGaussian'.
    SingleGaussian r[maxGaussians];
    
    // This is a copy of r (above), used when there are multiple observation
    //  modes. See the "GPSDirectPositionUpdate" method in gps.cc.
    SingleGaussian rCopy[maxGaussians];

    KI2DWithVelocity *newball;

    // vector that represents which way vision is pulling the localisation
    MVec3 visPull;
    MVec3 *vpHistory;
    int vpIndex;

    //stores where each of the teammates thinks the ball
    WMObj tmBall[NUM_TEAM_MEMBER];

    // location of teammates according to wireless communication
    WMObj wmTeammate[NUM_TEAM_MEMBER]; // Array containing location of teammates.
    //entered into the array according to
    //the teammates player number.
    //The position in the array that corresponds
    //to your own player number is left blank.

    // information form kalman filters tracking opponent dogs
    KalmanInfo2D *opponents[NUM_OPPONENTS];

#ifdef VISUAL_TRACK_FRIENDS
    // information form kalman filters tracking friendly dogs
    KalmanInfo2D *friends[NUM_FRIENDS_VIS];
#endif // VISUAL_TRACK_FRIENDS
    
    //containers to store values returned by the interface functions
    //They are deliberate not local variables to allow caching
    WMObj retSelf;      //self
    WMObj retBall;      //ball
    WMObj retTeammate;  //the teammate your after
    WMObj retOGoal;     //own goal
    WMObj retTGoal;     //target goal
    WMObj retVBall;     //velocity of ball

    //frame id for the last time the accessor functions were used.
    //needed for accessor function caching
    //caching only last for 1 frame because the variance of most things should get updated after 1 frame
    int selfAccessId;
    int ballAccessId;
    char lastBallOrigin;  
    int teammateAccessId;
    int lastTeammateAccessed;
    char lastTeammateOrigin;
    int oGoalAccessId;
    int tGoalAccessId;
    int vBallAccessId;
    char lastvBallOrigin;

    int frameId; //a number to represent what frame this is
    //it is used so we can implement caching of
    //the values returned from the accessor functions
    //(of the ball teammate goals etc)

    SlimVisOb observedVobs[VOB_COUNT]; //stores a cut down version of the
    //array of VisualObjects given to us
    //through GPSVisionUpdate.
    //theres an entry for every visual object

    int edgeDebugCounter; // Used for edge detection debugging output

    // cached for easy use as they are needed in kalman filters
    MMatrix3 *unitMatrix3; //3 by 3 unit matrix
    MMatrix4 *unitMatrix4; //4 by 4 unit matrix

	// set the observedVobs data from data passed from vision
    void UpdateObservedVobs(VisualObject * nvo);

    // Helper function for GPSEdgeUpdate
    double gradientAscent(MVec3 &oldRobot, MVec3 &newRobot, VisualCortex *vc);
    
#ifdef NEW_LINE_UPDATE   
    // Helper functions for the untested form of GPSEdgeUpdate
    int reformGaussian(bool output = false);
    int findSampleMaximums(int *indices);
    void calcSamplePoint(int index, MVec3 &out);
    int calcNeighbourIndices(int dir, int index, int *outIndices);

    // Variables used by the untested form of GPSEdgeUpdate
    static const int NUM_SAMPLES = 27;
    double samples[NUM_SAMPLES], prob[NUM_SAMPLES];
    MVec3 mean[NUM_SAMPLES], var[NUM_SAMPLES];
    MVec3 curMean, curDev;
#endif    
    
    // Performs inserting, culling, merging, normalisation and sorting
    //  of probability distribution.
    void insertGaussian(SingleGaussian&);
    void formDistribution();
    void normaliseDistribution();
    void mergeGaussians(int, int);
       
    void testKalman(); // DERRICK DEBUG
    
	// adds measurements to the given arrays
    void addMeasurement(SingleGaussian *g, SlimVisOb &vi, double bx, double by,
            double *jacWRTX, double *jacWRTY, double *jacWRTH,
            double *iv, double *vars, int curNum, bool dist);

    inline void GPS::addMeasurement(SingleGaussian *g, SlimVisOb &vi,
                    double *jacWRTX, double *jacWRTY, double *jacWRTH,
                    double *iv, double *vars, int curNum, bool dist) {
        // coordinates of object
        double bx = (*coord)[vi.vobType - 1][0];
        double by = (*coord)[vi.vobType - 1][1];
        addMeasurement(g, vi, bx, by, jacWRTX, jacWRTY, jacWRTH,
                        iv, vars, curNum, dist);
    }
    
    // utility functions
    // converts the given angle to [0,FULL_CIRCLE (360))
    inline void GPS::ANGLE_FIX(double& x) {
        //x=x%(2*M_PI);
        while(x>=FULL_CIRCLE)
            x-=FULL_CIRCLE;
        while(x<0)
            x+=FULL_CIRCLE;
    }
    
    // makes the given vector (x,y,z) length 1
    inline void GPS::NORMALISE(double& x, double& y, double& z) {
        double l = sqrt(SQUARE(x)+SQUARE(y)+SQUARE(z));
        if (l < 1E-5 && l > -1E-5) {
          cout << "WARNING: GPS->NORMALISE is trying to divide by "
              << l << endl;
          return;
        }
        x /= l;
        y /= l;
        z /= l;
    }

    // makes the given vector (x,y) length 1
    inline void GPS::NORMALISE(double& x, double& y) {
        double l = sqrt(SQUARE(x)+SQUARE(y));
        if (l < 1E-5 && l > -1E-5) {
          cout << "WARNING: GPS->NORMALISE is trying to divide by "
              << l << endl;
          return;
        }
        x /= l;
        y /= l;
    }

    void beaconDistanceCal();
#ifdef OFFLINE
    //pointer to a stream output for debugging offline
    ostream *outputStream;
    ostream *tiltCalibrationOutputStream;
    //Remembering the robot positions doing the last gradient ascent function 
    vector <MVec3> GA_robotsPos;
    vector <MVec3> GA_gradient;
    vector <double> GA_match;
    vector <double> GA_LR;
    
    vector <MVec3> GA_Duplicates;
#endif //OFFLINE

};

#endif // GPS_h_DEFINED
