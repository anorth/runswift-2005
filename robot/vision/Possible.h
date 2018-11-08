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
 * $Id: Possible.h 7536 2005-07-11 09:41:41Z amsi929 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _POSSIBLE_H_
#define _POSSIBLE_H_

#include "../share/minimalMatrix.h"
#include "../share/Common.h"

#ifdef OFFLINE
#define DUPLICATE_DEBUG
static ofstream fp_out;
#endif

static const double normVar = 400.0;    // Measured in cm squared
static const double angVar  = 0.25;     // Measured in radians squared
/* Tight variance observation 
static const double normVar = 350.0;//400.0;//800.0;    // Measured in cm squared
static const double angVar  = 0.1;//0.25;//0.50;     // Measured in radians squared
*/
//------------------------------------------------------------------------------
// This is a list of edge and intersection types (features), for one quadrant
// of the field

struct {
    // The x and y coordinates of a feature. This also includes a heading, which
    // will affect the locations and orientations of the generated symmetries
    // relative to this one.
    double x, y, h;
    
    // The dimensions of the area that the "edge center" (taken from the edge
    // localisation) has to lie in for the symmetries of that feature to be
    // generated. Measured from match position.
    double windowx, windowy;

    // The variance to use for an edge update, if the feature has been used to
    // generate symmetries.
    double varx, vary, varh;
    
    // This number indicates the symmetries for each edge feature.
    // Set to 1 for opposite-x symmetry, 2 for opposite-y symmetry, 3 for both.
    int duplicity;
    
    // This indicates whether the next edge feature in the list should also be
    // used to generate more symmetries.
    bool sameAsNext;
    
} edgeType[] = {

    // --- Straight edges ---
    // Goal line
    {FIELD_WIDTH / 4, 0, DEG2RAD(270),
        FIELD_WIDTH / 2 - 20, 20, VERY_LARGE_INT, normVar, angVar, 2, true},
    
    
    // Goal line opposite facing the field 
    {FIELD_WIDTH / 4, 0, DEG2RAD(90),
        FIELD_WIDTH / 2 - 20, 20, VERY_LARGE_INT, normVar, angVar, 2, true},
    
    
    // Outer goalbox line
    {FIELD_WIDTH / 2, GOALBOX_DEPTH, DEG2RAD(90),
        GOALBOX_WIDTH / 2 - 20, 20, VERY_LARGE_INT, normVar, angVar, 2, true},

   // Outer goalbox line opposite
    {FIELD_WIDTH / 2, GOALBOX_DEPTH, DEG2RAD(270),
        GOALBOX_WIDTH / 2 - 20, 20, VERY_LARGE_INT, normVar, angVar, 2, true},
        
    // Halfway line
    {FIELD_WIDTH / 4, FIELD_LENGTH / 2, DEG2RAD(90),
        FIELD_WIDTH / 4 - 20, 20, VERY_LARGE_INT, normVar, angVar, 2, true},
    
    // Sideline
    {0, FIELD_LENGTH / 4, DEG2RAD(180),
        20, FIELD_LENGTH / 4 - 20, normVar, VERY_LARGE_INT, angVar, 1, true},
    
    
    // Sideline opposite Facing the field
    {0, FIELD_LENGTH / 4, DEG2RAD(0),
        20, FIELD_LENGTH / 4 - 20, normVar, VERY_LARGE_INT, angVar, 1, true},
    
    
    // Goalbox sideline
    {(FIELD_WIDTH - GOALBOX_WIDTH) / 2, GOALBOX_DEPTH / 2, DEG2RAD(0),
        20, GOALBOX_DEPTH / 2 - 20, normVar, VERY_LARGE_INT, angVar, 1, true},

    // Goalbox sideline opposite
    {(FIELD_WIDTH - GOALBOX_WIDTH) / 2, GOALBOX_DEPTH / 2, DEG2RAD(180),
        20, GOALBOX_DEPTH / 2 - 20, normVar, VERY_LARGE_INT, angVar, 1, true},
            
    // Fringe behind goal
    {FIELD_WIDTH / 4, -GOAL_FRINGE_WIDTH, DEG2RAD(270),
        FIELD_WIDTH / 2 - 20, 20, VERY_LARGE_INT, normVar, angVar, 2, true},
    
    
    // Fringe behind goal opposite Facing the field
    {FIELD_WIDTH / 4, -GOAL_FRINGE_WIDTH, DEG2RAD(90),
        FIELD_WIDTH / 2 - 20, 20, VERY_LARGE_INT, normVar, angVar, 2, true},
    
    
    // Side Fringe Facing the field
    {-SIDE_FRINGE_WIDTH, FIELD_LENGTH / 4, DEG2RAD(0),
        20, FIELD_LENGTH / 4 - 20, normVar, VERY_LARGE_INT, angVar, 1, true},
#ifdef OSAKA
    // Side Wall Facing the field
    {-SIDE_WALL_FRINGE_WIDTH, SIDE_WALL_FRINGE_CENTER_Y, DEG2RAD(0),
        20, SIDE_WALL_FRINGE_CENTER_Y - 20, normVar, VERY_LARGE_INT, angVar, 1, true},
    
    // Side Wall Facing outside
    {-SIDE_WALL_FRINGE_WIDTH, SIDE_WALL_FRINGE_CENTER_Y, DEG2RAD(180),
        20, SIDE_WALL_FRINGE_CENTER_Y - 20, normVar, VERY_LARGE_INT, angVar, 1, true},
   
    // Wall behind goal
    {GOAL_WALL_FRINGE_CENTER_X, -GOAL_WALL_FRINGE_WIDTH, DEG2RAD(270),
        FIELD_WIDTH / 2 - 20, 20, VERY_LARGE_INT, normVar, angVar, 2, true},
    
    // Wall behind goal opposite Facing the field
    {GOAL_WALL_FRINGE_CENTER_X, -GOAL_WALL_FRINGE_WIDTH, DEG2RAD(90),
        FIELD_WIDTH / 2 - 20, 20, VERY_LARGE_INT, normVar, angVar, 2, true},
     
#endif    
    // Side Fringe
    {-SIDE_FRINGE_WIDTH, FIELD_LENGTH / 4, DEG2RAD(180),
        20, FIELD_LENGTH / 4 - 20, normVar, VERY_LARGE_INT, angVar, 1, false},
   
    // --- Edge intersections ---
    // Alexn: We're treating all intersections as equivalent now without
    // the field boundary, so all have x/y symmetry
    // Goalbox corner
    {(FIELD_WIDTH - GOALBOX_WIDTH) / 2, GOALBOX_DEPTH, DEG2RAD(315),
        20, 20, normVar, normVar, angVar, 3, true},

    // Intersection of goalbox and goal line -- inside goal box
    {(FIELD_WIDTH - GOALBOX_WIDTH) / 2, 0, DEG2RAD(45),
        20, 30, normVar, normVar, angVar, 3, true},
    
    // Intersection of goalbox and goal line -- outside goal box
    {(FIELD_WIDTH - GOALBOX_WIDTH) / 2, 0, DEG2RAD(135),
        20, 30, normVar, normVar, angVar, 3, true},

    // Intersection of halfway line and side line
    {0, FIELD_LENGTH / 2, DEG2RAD(315),
        20, 20, normVar, normVar, angVar, 3, true},

    // Intersection of goal and goal line 
    {(FIELD_WIDTH - GOAL_WIDTH) / 2, 0, DEG2RAD(225),
        20, 30, normVar, normVar, angVar, 3, true},
#ifndef OSAKA    
    /* This should be commented if the wall is far from halfline */
    // Intersection of halfway line and side wall
    {-SIDE_FRINGE_WIDTH, FIELD_LENGTH / 2, DEG2RAD(315),
        20, 20, normVar, normVar, angVar, 3, true},
#endif //OSAKA      
#ifdef OSAKA
    //WALL corner
    {-SIDE_WALL_FRINGE_WIDTH, -GOAL_WALL_FRINGE_CENTER_X, DEG2RAD(45),
        20, 20, normVar, normVar, angVar, 3, true},
#endif //OSAKA 

    //FRINGE corner
    {-SIDE_FRINGE_WIDTH, -GOAL_FRINGE_WIDTH, DEG2RAD(45),
        20, 20, normVar, normVar, angVar, 3, true},
    
    // Field corner
    {0, 0, DEG2RAD(45),
        20, 20, normVar, normVar, angVar, 3, true},

    // The center circle
    {FIELD_WIDTH / 2, FIELD_LENGTH / 2, DEG2RAD(225),
        CIRCLE_DIAMETER / 2, CIRCLE_DIAMETER / 2,
        normVar, normVar, angVar, 3, false}
};

static const int EDGE_TYPES = sizeof(edgeType)/sizeof(edgeType[0]);

// This must be >= the longest run of similar intersections above
// (sameAsNext == true) times four (the number of symmetries)
static const int MAX_EDGE_DUPLICITY = 36;


//------------------------------------------------------------------------------
// Moves a point to a different field quadrant, returns the quadrant it was in
//  before.

int changeQuadrant(int quad, MVec3 &p)
{
    int switchQuad, oldQuad = 0;

    if (p(0, 0) > (FIELD_WIDTH * 0.5)) oldQuad |= 1;
    if (p(1, 0) > (FIELD_LENGTH * 0.5)) oldQuad |= 2;

    switchQuad = oldQuad ^ quad;
    if (switchQuad & 1) {
        p(0, 0) = FIELD_WIDTH - p(0, 0);
        p(2, 0) = pi - p(2, 0);
        if (p(2, 0) < 0.0) p(2, 0) += pi * 2;
    }

    if (switchQuad & 2) {
        p(1, 0) = FIELD_LENGTH - p(1, 0);
        p(2, 0) = -p(2, 0);
        if (p(2, 0) < 0.0) p(2, 0) += pi * 2;
    }

    return oldQuad;
}

//------------------------------------------------------------------------------
// Same as above, except for a (x, y) vector.

inline void changeQuadrantVec(int switchQuad, MVec2 &p)
{
    if (switchQuad & 2) p(1, 0) = -p(1, 0);
    if (switchQuad & 1) p(0, 0) = -p(0, 0);
}

//------------------------------------------------------------------------------
// Calculates all duplicate locations for a given edge intersection type

int calcDuplicates(int eType, MVec3 &approach, MVec3 *possibles, MVec3 *varList)
{
    int i, index, lastIndex, locCount, newQuad;
    MVec2 intersectDir, dir;
    MVec3 intersect, var;
    double tempX, tempY;
    
    locCount = 0;
    index = eType;
    lastIndex = index;
    while (index > 0 && edgeType[index - 1].sameAsNext) index--;
    while (lastIndex < (EDGE_TYPES - 1) &&
           edgeType[lastIndex].sameAsNext) lastIndex++;
#ifdef OFFLINE
#ifdef DUPLICATE_DEBUG
//    if (index >= 7)
    if (fp_out.good())
        fp_out << endl << __func__ << " edge type:" << eType << " index:"<<index<<" lastIndex:"<<lastIndex<<endl;
#endif
#endif
    for (; index <= lastIndex; index++) {
        intersect(0, 0) = edgeType[index].x;
        intersect(1, 0) = edgeType[index].y;
        intersect(2, 0) = edgeType[index].h;
        intersectDir(0, 0) = cos(intersect(2, 0));
        intersectDir(1, 0) = sin(intersect(2, 0));

        var(0, 0) = edgeType[index].varx;
        var(1, 0) = edgeType[index].vary;
        var(2, 0) = edgeType[index].varh;

        for (i = 0; i < 4; i++) {
            newQuad = edgeType[index].duplicity & i;

            if (newQuad == i) {
                // Generate the edge point
                possibles[locCount] = intersect;
                dir = intersectDir;

                changeQuadrant(i, possibles[locCount]);
                changeQuadrantVec(i, dir);
#ifdef OFFLINE
#ifdef DUPLICATE_DEBUG
//                if (index >= 7){
                if (fp_out.good())
                {
                    fp_out << __func__ << " edgeType[" << index << "]" << " quardrant[" << newQuad << "]" << " possibles:"<< locCount <<" possiblesX:"<< possibles[locCount](0, 0) << " possiblesY:" << possibles[locCount](1, 0) <<" possiblesH:" << RAD2DEG(possibles[locCount](2, 0)) << " approachX:" << approach(0, 0) << " dirX:"<< dir(0, 0) << " approachY:" << approach(1, 0) << " dirY:"<< dir(1, 0)<< " approachH:" << RAD2DEG(approach(2, 0)) <<endl;
                }
//                }
#endif    
#endif
                // Calculate new position
                possibles[locCount](0, 0) += approach(0, 0) * dir(0, 0)
                                          -  approach(1, 0) * dir(1, 0);
                possibles[locCount](1, 0) += approach(0, 0) * dir(1, 0)
                                          +  approach(1, 0) * dir(0, 0);
                possibles[locCount](2, 0) += approach(2, 0);
                
                if (possibles[locCount](2, 0) >= pi * 2)
                    possibles[locCount](2, 0) -= pi * 2;
    
                if (possibles[locCount](2, 0) < 0.0)
                    possibles[locCount](2, 0) += pi * 2;
                
                // Set new variance
                varList[locCount] = var;
                
#ifdef OFFLINE
#ifdef DUPLICATE_DEBUG
                if (fp_out.good())
                {
                    fp_out << __func__ << " edgeType[" << index << "]" << " quardrant[" << newQuad << "]" << " possibles:"<<locCount<<": x:"<<possibles[locCount](0, 0)<<" y:"<<possibles[locCount](1, 0)<<" h:"<<RAD2DEG(possibles[locCount](2, 0))<<endl << endl;
                }
#endif
#endif                
                /* remove duplicate that outside the Field */
                // checking in the front because we want to give the original calculation
                tempX = possibles[locCount](0, 0);
                tempY = possibles[locCount](1, 0);
                if (eType != index && FieldClipping(tempX,tempY))
                    continue;
                
                ++locCount;
            }
        }
    }

    return locCount;
}

//------------------------------------------------------------------------------
// Calculates a list of alternate locations the robot could be at, given
//  its current position (in gps x, y, h global coordinates) and the center
//  of edges seen (in gps x, y global coordinates). The possible locations and
//  orientations are output to the "possibles" array, and the number of
//  possible locations returned. This assumes "possibles" array is large enough
//  (MAX_EDGE_DUPLICITY) to hold points (will seg fault if not). 

int calcAlternateLocations(MVec3 &robot, MVec2 &edgeCenter,
                           MVec3 *possibles, MVec3 *varList)
{
#ifdef OFFLINE
#ifdef DUPLICATE_DEBUG
    fp_out.open("DebugPossible.txt", ios::out);
#endif
#endif
    MVec3 testPoint, approach, finalApp;
    MVec2 dir;
    int i, index, ecQuad;
    double dx, dy, dist;

    // Find the intersection point that the dog is looking at, by finding the
    // edge feature window that the edge center resides in.
    testPoint(0, 0) = edgeCenter(0, 0);
    testPoint(1, 0) = edgeCenter(1, 0);
    testPoint(2, 0) = 0.0;
    index = -1; dist = 0;
    ecQuad = changeQuadrant(0, testPoint);
#ifdef OFFLINE
#ifdef DUPLICATE_DEBUG
        if (fp_out.good())
        {
            fp_out << __func__ << " Test point before change quadrant x:" << edgeCenter(0, 0) << " y:" << edgeCenter(1, 0) << endl;
            fp_out << __func__ << " Test point after change quadrant x:" << testPoint(0, 0) << " y:" << testPoint(1, 0) << endl;
        }
#endif
#endif        
    for (i = 0; i < EDGE_TYPES; i++) {
        dx = fabs(testPoint(0, 0) - edgeType[i].x);
        dy = fabs(testPoint(1, 0) - edgeType[i].y);
#ifdef OFFLINE
#ifdef DUPLICATE_DEBUG
        if (fp_out.good())
        {
            fp_out << endl << __func__ << " Center point with type:" << i << " x:" << edgeType[i].x << " y:" << edgeType[i].y << endl;            
            fp_out << "dx:"<< dx << " dy:" << dy <<endl;
            fp_out << __func__ << " wx:" << edgeType[i].windowx << " wy:" << edgeType[i].windowy <<endl;
        }        
#endif
#endif        
        if (dx < edgeType[i].windowx && dy < edgeType[i].windowy) {
            if (dx > 0.0) dx /= edgeType[i].windowx;
            if (dy > 0.0) dy /= edgeType[i].windowy;
#ifdef OFFLINE
#ifdef DUPLICATE_DEBUG
            if (fp_out.good())
            {
                fp_out << __func__ << " div by window dx:"<< dx << " dy:" << dy <<endl;
                fp_out << __func__ << " dx+dy:" << dx+dy << " dist:" << dist <<endl;
            }        
#endif
#endif                                
            if (index == -1 || (index != -1 && (dx + dy) < dist)) {
                dist = dx + dy;
                index = i;
            }
        }
    }
    
    // If no match was found, just use a basic "opposite side of the field"
    // symmetry.
    if (index == -1) {
        possibles[0] = robot;
        possibles[1](0, 0) = FIELD_WIDTH - robot(0, 0);
        possibles[1](1, 0) = FIELD_LENGTH - robot(1, 0);
        possibles[1](2, 0) = robot(2, 0) + pi;
        if (possibles[1](2, 0) >= 2 * pi)
            possibles[1](2, 0) -= 2 * pi;

        varList[0](0, 0) = normVar;
        varList[0](1, 0) = normVar;
        varList[0](2, 0) = angVar;
        varList[1] = varList[0];
        
        return 2;
    }

    // Calculate the approach (vector from edge intersection to the robot).
    testPoint(0, 0) = edgeType[index].x;
    testPoint(1, 0) = edgeType[index].y;
    testPoint(2, 0) = edgeType[index].h;
    dir(0, 0) = cos(testPoint(2, 0));
    dir(1, 0) = sin(testPoint(2, 0));
    changeQuadrant(ecQuad, testPoint);
    approach = robot;
    approach -= testPoint;
#ifdef OFFLINE
#ifdef DUPLICATE_DEBUG
    if (fp_out.good())
    {
        fp_out << __func__ << "Before! approach(x):"<< approach(0, 0) << 
            " approach(y):" << approach(1, 0) << 
            " approach(a):" << RAD2DEG(approach(2, 0)) <<endl;       
    }
#endif                            
#endif

    // Resolve approach in terms of edge direction & orthogonal to edge
    // direction.
    changeQuadrantVec(ecQuad, dir);
    //dir(0,0) close to 0 if angle 90
    finalApp(2, 0) = approach(2, 0);
    finalApp(0, 0) = approach(0, 0) * dir(0, 0) + approach(1, 0) * dir(1, 0);
    finalApp(1, 0) = approach(1, 0) * dir(0, 0) - approach(0, 0) * dir(1, 0);
    
#ifdef OFFLINE
#ifdef DUPLICATE_DEBUG
    if (fp_out.good())
    {
        fp_out <<  __func__ <<" fa(x):"<< finalApp(0, 0) << 
                " fa(y):" << finalApp(1, 0) << 
                " fa(a):" << RAD2DEG(finalApp(2, 0)) <<endl;
                
        fp_out <<  __func__ <<" approach(x):"<< approach(0, 0) << 
                " approach(y):" << approach(1, 0) << 
                " approach(a):" << RAD2DEG(approach(2, 0)) <<endl;
    
        fp_out <<  __func__ <<" dir(x):"<< RAD2DEG(dir(0, 0)) << 
                " dir(y):" << RAD2DEG(dir(1, 0)) << endl;
    }
#endif                        
#endif
    int result = calcDuplicates(index, finalApp, possibles, varList);

#ifdef OFFLINE
#ifdef DUPLICATE_DEBUG
    fp_out.close();
#endif
#endif
    return result;
}


#endif // _POSSIBLE_H_
