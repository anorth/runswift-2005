/* Copyright 2005 The University of New South Wales (UNSW) and National  
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
 * $Id: $
 *
 * Copyright (c) 2005 UNSW
 * All Rights Reserved.
 *
 * Obstacle functions
 * 
 **/
 
#ifndef OBSTACLE_H
#define OBSTACLE_H
#include <cstdio>
#include <cstring>
#include <cfloat>
#include <iostream>
#include <map>
#include <utility>
#include "VisualCortex.h"
#include "gps.h"
#include "../share/Common.h"
#include "../share/FieldDef.h"
#include "../share/SharedMemoryDef.h"
#define NOOBSTACLEPOINTS 200
//assumed number of box maximum is one point each box
#define MAX_NUM_BOXES NOOBSTACLEPOINTS
#define GPS_MAX_NUM_BOXES (MAX_NUM_BOXES * 2)
#define NUM_CONST_BOXES 16
#define CONST_BOX_POINTS 40
#define BOX_SIZE_X 10
#define BOX_SIZE_Y 10
#define NUMBER_OF_BOX_X FIELD_WIDTH/BOX_SIZE_X
#define NUMBER_OF_BOX_Y FIELD_LENGTH/BOX_SIZE_Y
#define BOX_LIFETIME 90
#define BOX_OWN_DECAY 0.98
#define BOX_SHARED_DECAY 0.80
#define MAX_POS_VAR_TO_UPDATE_OBSTACLE get95CF(200)
#define MAX_HEADING_VAR_TO_UPDATE_OBSTACLE get95CF(HALF_CIRCLE)

#define CORRIDOR_WIDTH 30
#define MIN_OBSTACLE 10
#define MIN_DIST 50

//following for use in getBestGap
#define MIN_GAP_ANGLE 20
#define NO_GAP 666
#define NO_HEADING 666
//calculated dynamically
#define GAP_MAX_DIST -1
#define GPS_MIN_DIST 50

// user function flags
// -------------------

/* 
 * OBS_USE_GPS = False && OBS_USE_LOCAL = False
 * use gps obstacles if gps is valid this
 * frame, else use local obstacles
 * most cases should use this
 * OBS_USE_GPS = True && OBS_USE_LOCAL = True
 * same as above
 * OBS_USE_GPS = True && OBS_USE_LOCAL = False
 * use gps obstacles
 * OBS_USE_GPS = False && OBS_USE_LOCAL = True
 * use local obstacles
 */
#define OBS_USE_GPS 1
#define OBS_USE_LOCAL 2 

/*
 * use constant obstacles (beacons, goals, etc)
 * won't work in local
 */
#define OBS_USE_CONST 4

/*
 * use shared obstacles
 * won't work in local
 */
#define OBS_USE_SHARED 8

//use no extra obstacles

#define OBS_USE_NONE 0

//add more flags as next powers of 2, i.e. 16,32,64,128

#define OBS_USE_DEFAULT (OBS_USE_CONST + OBS_USE_SHARED)

// -------------------

static const int MIN_ANGLE = -110;
static const int MAX_ANGLE = 110; 
static const int ANGLE_RANGE = 10;
static const int MAX_RANGES = 22;
static const int OUTSIDE_FIELD_OFFSET = 20;

using namespace std;


class ObstacleBox
{
    public:
        ObstacleBox();
        ~ObstacleBox();
        
        int getX() const; 
        int getY() const;
        
        void setX(int x) { m_middleX = x; headingValid = false;}
        void setY(int y) { m_middleY = y; headingValid = false;}
        
        double getNoTotalPoints() const;
        double getNoPoints() const;
        double getNoSharedPoints() const ;
        double getPoints(bool useShared); 
        void addPoint();
        void addPoints(int npoints);
        void addSharedPoints(int npoints);
        
        long getFrameLastSeen();
        long getFrameLastReceived();
        int getDistance(double dogX = 0, double dogY = 0);
        int getDistanceSquared(double dogX = 0, double dogY = 0);
        double getHeading(double dogX = 0, double dogY = 0, double dogH = 0);
        double getHeadingForQSort(double dogX = 0, double dogY = 0, double dogH = 0) const;
        double getOffset(double heading = NO_HEADING);
        void getMiddlePoint(int *middleX, int *middleY) const;
        void resetTo(int middleX, int middleY,
                     long frameLastSeen, double nPoints = 1,
                     long frameLastReceived = 0, int nSharedPoints = 0);
        bool pointInBox(int x, int y);
        
        void decayPoints();
        
        inline int round(double n) {return int(rint(n));}        
                        
        long m_frameLastSeen;
        long m_frameLastReceived; 
        bool headingValid;
        void setHeading(double h);
          
    private:
        double m_npoints;
        double m_nSharedPoints;
        int m_middleX;
        int m_middleY;
        double m_heading;  
};

class Obstacle 
{
    public:
        Obstacle();
        ~Obstacle();
        
        
        // user obstacle functions
        //--------------------------------------------------------------------
        
        /*
         * Given a rectangle in local coordinates find the
         * number of obstacle points inside
         * do not consider obstacle boxes with less than minObstacle points.
         */
        int getNoObstacleInBox(
            int topleftx, 
            int toplefty, 
            int bottomrightx, 
            int bottomrighty, 
            int minObstacle = MIN_OBSTACLE, 
            int flags = OBS_USE_DEFAULT);
                
        /*
         * Given two headings get number of obstacle points
         * between them.
         * do not consider obstacle boxes not between minDist and maxDist.
         * do not consider obstacle boxes with less than minObstacle points.
         */    
        int getNoObstacleInHeading(
            int* dist,
            int minHeading, 
            int maxHeading,
            int minDist,
            int maxDist, 
            int minObstacle = MIN_OBSTACLE,
            int flags = OBS_USE_DEFAULT);    

        /*
         * Given two points in local coordinates draw a rectangle
         * around them with width equal to corridorWidth.
         * return number of obstacle points in the rectangle.
         * do not consider obstacle boxes closer than minDist.
         * do not consider obstacle boxes with less than minObstacle points.
         */          
        int getNoObstacleBetween(
            int p1x, 
            int p1y, 
            int p2x, 
            int p2y, 
            int corridorWidth = CORRIDOR_WIDTH,
            int minDist = MIN_DIST,
            int minObstacle = MIN_OBSTACLE,
            int flags = OBS_USE_DEFAULT);              

        /*
         * Given a destination in local coordinates get the best gap 
         * to that destination with a specified minGapAngle.  
         * Gap is returned as bestHeading (best heading to destination in gap),
         * gapLeft & gapRight (heading to left and right side of gap),
         * gapAngle (size of gap as angle).
         * headings are in degrees with -90 as right and 90 as left.
         * do not consider obstacle boxes not between minDist and maxDist.
         * default maxDist is dynamically calculated as distance to destination.
         * do not consider obstacle boxes with less than minObstacle points.
         * return false if no best gap found.
         */
        bool getBestGap(
            double destX, 
            double destY, 
            int *gapLeft, 
            int *gapRight, 
            int *bestHeading, 
            int *gapAngle,
            int maxdist = GAP_MAX_DIST, 
            int mindist = MIN_DIST, 
            int minGapAngle = MIN_GAP_ANGLE, 
            int minObstacle = MIN_OBSTACLE,
            int flags = OBS_USE_DEFAULT);  
            
        /*
         * Given a rectangle in local coordinates find the closest   
         * obstacle box position where contain more than minObstacle points.
         * Return the middle position of the box posX, posY.
         * if no box fit the criteria return False, else True.
         */
        bool getPointToNearestObstacleInBox(
            int topleftx, 
            int toplefty, 
            int bottomrightx, 
            int bottomrighty,
            int *posX, 
            int *posY, 
            int minObstacle = MIN_OBSTACLE,
            int flags = OBS_USE_DEFAULT);
            
            
        //--------------------------------------------------------------------
            
                 
        /*
         * Called every frame, update obstacle map from vision
         */    
        void ObstacleUpdate(VisualCortex *visual, GPS *gps);

        /* Clears all obstacle information (except constant obstacles) */
        void clearObstacles(void);
        
        //return true if obstacles seen this frame were
        //added to gps boxes
        bool ObstacleGPSValid();
        
        // make the GPS obstacles valid (for offline)
        void setObstacleGPSValid(bool valid = true);
        
        /*
         * Get from mp_points, put the points in the corresponding box.
         * Moved to public function for OFFLINE checking.
         * Should only be called when OFFLINE.
         */
        void setObstaclePoints(int *thepoints, int size, long frameNum,
        double dogX, double dogY, double dogH, double posVar, double hVar);

        /*
         * Create an obstacle box at the specified x, y (for offline)
         */
        ObstacleBox* addObstacleBox(int x, int y, int frameLastSeen,
                                    int nPoints = MIN_OBSTACLE+1);

        /*
         * for displaying gps in subvision
         */
        int getNumGPSBoxes(){return m_nGPSBoxes;}
        ObstacleBox* getGPSBoxes(){return mp_obstacle_box_gps;}        
                
        /* 
         * sharing obstacles with teammates 
         */
        void setShareInfo(WMShareObj& obj); 
        void processShareInfo(const WMShareObj& info); 
        
        bool doObstacleUpdate;
        
        // allow gps to access getNoObstacleInBox with
        // global coordinates... DON'T put this in visionlink
        int getNoObstacleInBoxGPS(int topleftx, int toplefty, 
            int bottomrightx, int bottomrighty, int flags,
            int minObstacle, bool globalCoords = false);   
                    
    private:
    
        int getNoObstacleInBoxLocal(int topleftx, int toplefty, 
            int bottomrightx, int bottomrighty, int flags,
            int minObstacle);
    
        int getNoObstacleInHeadingLocal(int* dist,
            int midHeading, int maxHeading,
            int minDist, int maxDist, int flags,
            int minObstacle);     
        
        int getNoObstacleInHeadingGPS(int* dist,
            int midHeading, int maxHeading,
            int minDist, int maxDist, int flags,
            int minObstacle);              

        int getNoObstacleBetweenLocal(int p1x, int p1y, int p2x, int p2y, 
            int flags, 
            int corridorWidth,
            int minDist,
            int minObstacle);

        int getNoObstacleBetweenGPS(int p1x, int p1y, int p2x, int p2y, 
            int flags, 
            int corridorWidth,
            int minDist, 
            int minObstacle);    
            
        bool getBestGapLocal(double destX, double destY, int flags,
            int *gapLeft, int *gapRight, 
            int *bestHeading, int *gapAngle,
            int maxdist, 
            int mindist, 
            int minGapAngle, 
            int minObstacle); 
          
        bool getBestGapGPS(double destX, double destY, int flags,
            int *gapLeft, int *gapRight, 
            int *bestHeading, int *gapAngle,
            int maxdist, 
            int mindist, 
            int minGapAngle,
            int minObstacle);       

        bool getPointToNearestObstacleInBoxLocal(
            int topleftx, int toplefty, int bottomrightx, int bottomrighty,
            int flags, int *posX, int *posY, int minObstacle);
                     
        bool getPointToNearestObstacleInBoxGPS(
            int topleftx, int toplefty, int bottomrightx, int bottomrighty,
            int flags, int *posX, int *posY, int minObstacle);
                                         
                                                                                               
        long            currentFrame;   // vision frame number
        bool            localPutInGPS;  
        
        // Obstacle points as received from vision. [x1, y1, x2, y2, ...]
        int             m_nPoints;  // number of points
        int             mp_points[NOOBSTACLEPOINTS*2];
        
        // Local obstacle boxes
        int             m_nBoxes;
        ObstacleBox     mp_obstacle_box_list[MAX_NUM_BOXES];
        ObstacleBox     mpObstacleAngles[MAX_RANGES];
        
        // GPS (global) obstacle boxes
        int             m_nGPSBoxes;
        ObstacleBox     mp_obstacle_box_gps[GPS_MAX_NUM_BOXES];
        
        // Constant obstacle boxes (landmarks)
        int             m_nConstBoxes;
        ObstacleBox     mp_obstacle_box_const[NUM_CONST_BOXES];
        
        static int compareByHeading(const void * ob1, const void * ob2);
        static int compareByXY(const void * ob1, const void * ob2);
        static int compareByOwnObstaclePoints(const void * ob1, const void * ob2);
        
        double dogX;
        double dogY;
        double dogH;
        double posVar;
        double hVar;
        
        bool gpsBoxValid(int i);
        
        void addPointToAngle(int x, int y);
        void addBoxToLocal(int x, int y, int npoints); 
        void addBoxToGPS(int x, int y, int npoints);
        void addSharedPointsToGPS(int x, int y, int npoints);
        void setConstBoxes();
        
        void decayGPS();              
        
        /*
         * Check if point x,y is inside the area.
         */
        bool isInside(
            int topleftx, int toplefty, 
            int bottomrightx, int bottomrighty,
            int x, int y);
        
        //given an array of obstacle boxes in local
        //coordinates, return bestgap attributes    
        bool getBestGapFromBoxes(
            ObstacleBox *boxes, int numBoxes,
            double destX, double destY,  int flags,
            int *gapLeft, int *gapRight, 
            int *bestHeading, int *gapAngle,
            int maxdist, int mindist, 
            int minGapAngle, int gapObstacle);             
            
        // check if a value is between two other values    
        bool isBetween(double val, double rangeA, double rangeB);
        
        /* Given a gap that is large enough to encompass one or more gaps,
         * i.e. have multiple gap headings, return the gap heading for the
         * gap closest to the destination   
         */ 
        double closestHeadingToDestInGap(
        double destHeading, int minGapAngle,
        double gapRHeading, double obsROffset, 
        double gapLHeading, double obsLOffset); 
        
        bool isAngleBetween(double destHeading, double gapRHeading, double gapLHeading);
        
        int getRectPoints(int p1x, int p1y, int p2x, int p2y,
                            int corridorWidth,
                            int *rectp1x, int *rectp1y,
                            int *rectp2x, int *rectp2y,
                            int *rectp3x, int *rectp3y,            
                            int *rectp4x, int *rectp4y);
        
        bool intersectsRay(int rx, int ry, int p1x, int p1y, int p2x, int p2y);
        
        bool isInRectangle(int pX, int pY, int rectPt1X, int rectPt1Y, int rectPt2X,
        int rectPt2Y, int rectPt3X, int rectPt3Y, int rectPt4X, int rectPt4Y);
        
        int calcMaxDist(double destX, double destY, int mindist);
                
        void localBoxesToGlobalBoxes();   
                
        /* 
         * Put collected points to the boxes, gps and local coordinate
         */         
        void putPointsInBox();
        
        /* 
         * given a point, return the coordinate of the box 
         */
        void getBoxCoordinatePoint(int x, int y, int &boxX, int &boxY);      
        
        
        void printGPS();
        
        double offsetRight(double heading, double offset);
        double offsetLeft(double heading, double offset);
        
        inline int round(double n) {return int(rint(n));}
        
        void setLocalHeadings(ObstacleBox *boxes, int numBoxes);
        
        int minOfFour(int a, int b, int c, int d);
        
        int maxOfFour(int a, int b, int c, int d);
        
};

#endif


