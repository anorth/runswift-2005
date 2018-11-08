/*
   Copyright 2005 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
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
   should be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


/* SubObject.h
 * Object recognition for SubVision
 */

#ifndef _SUBOBJECT_H_
#define _SUBOBJECT_H_

#include <utility>
#include <vector>
#include <list>

#include "../share/XYZ_Coord.h"
#include "SubVision.h"
#include "VisualObject.h"

#define MAX_NO_PINK_BEACON_BEFORE_FILTER 20

using std::pair;
using std::vector;
using std::list;

// indicies into vBeacons array. Order must match VisualCortex.h
enum beacons {
    svBlueOnPink = 0,
    svPinkOnBlue,
    svYellowOnPink,
    svPinkOnYellow,
    NUM_BEACONS,
};

// indicies into vGoals array. Order must match VisualCortex.h
enum goals {
    svBlueGoal = 0,
    svYellowGoal,
    NUM_GOALS,
};

/* SubObject is the object recognition for the subsampling vision system, 
 * SubVision. It uses VisualFeatures detected by SubVision to form 
 * VisualObjects for use by GPS and behaviours.
 */
class SubObject {

    public:
        SubObject(SubVision& sv, 
                  VisualObject* ball, 
                  VisualObject* beacons,
                  VisualObject* goals,
                  VisualObject* challengePinkBeacon = NULL,
                  int maxChallengePinkBeacon = 0)
            : vBall(ball), 
              vBeacons(beacons), 
              vGoals(goals),
              subvision(sv), 
              //vPinkBeacons(pinkBeacon),
              features(sv.getFeatures()),
              obstacleFeatures(sv.getObstacleFeatures()),
              mMaxChallengePinkBeacon(maxChallengePinkBeacon)
                 {mNoPinkBeacon = 0;vChallengePinkBeacon = challengePinkBeacon;}

        ~SubObject() {}

        void setParameters(bool hzExists,
                           double pan, double tilt, double crane,
                           double roll, double sinEroll,double cosEroll);
        int findVisualNonBallObjects();
        int findVisualBallObject();
        list<int>& getBallFeatures() { return this->ballFeatures; }
         
        void sendObjects();
        
        //Loc Challenge
        int getNoPinkBlob();
        
        // pointers to visual cortex vobs
        VisualObject* vBall;
        VisualObject* vBeacons;    // NUM_BEACONS
        VisualObject* vGoals;      // NUM_GOALS
        VisualObject* vChallengePinkBeacon;  //
        
        // detected wall object feature
        VisualFeature vWall; 
        
        //Check if wall detected.
        bool isWallExist();

    private:
        friend class FormSubvisionImpl; // allow offline tools private access
        
        int pruneFeatures(int start = 0);
        void pruneInsaneBallFeatures();
        void pruneInsaneObstacleFeatures();
        void pruneInsaneLineFeatures();
        int findBall();
        int findBallDirection();
        int findBeacons();
        int findGoals();
        int findWalls();
        
        bool isAboveWall(double x, double y);
        bool ransacWalls();
        bool medianWalls(); 
        bool medianWalls2();
        bool medianBall();
        //bool chordBall();
        //bool blobBall();

        int groupFeatures(list<int>& features, list<list<int> >& groups,
                            int nScanLines = 1, bool groupHoriz = false);
        bool classifyBeacon(list<list<int> >::iterator groupItr);
        bool classifyPinkBeacon(list<list<int> >::iterator groupItr);
        
        bool lineAbove(const VisualFeature& vf1, const VisualFeature& vf2,
                        int nScanLines);
        int localBlob(int x, int y, Color colour, int tol,
                        int* cy, int* cy, int* width, int* height);
        
        SubVision& subvision;
        vector<VisualFeature>& features;
        vector<VisualFeature>& obstacleFeatures;
        list<int> ballFeatures;
        list<int> beaconFeatures;
        list<int> goalFeatures;
        vector<int> wallFeatures;
        vector<int> fieldLineFeatures;

        
        // wall gradient and y-intercept
        bool wallExists;
        double wall_m; 
        double wall_b;
        bool pruneAboveWall; 
        
        bool hzExists;

        double mRoll;
        double mSinEroll; 
        double mCosEroll;
        
        // these are assigned by Visual Cortex and in radians
        double pan; 
        double tilt; 
        double crane;
        
        /****************/
        /*Loc challenge */
        int mMaxChallengePinkBeacon;
        int mNoPinkBeacon;
        VisualObject mChallengePinkBeacon[MAX_NO_PINK_BEACON_BEFORE_FILTER];
        
};



#endif
