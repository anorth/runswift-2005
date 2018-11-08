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


#ifndef CHALLENGE_H
#define CHALLENGE_H


//#define LOCALISATION_CHALLENGE
//#define LIGHTING_CHALLENGE

#ifdef LOCALISATION_CHALLENGE

//uncomment this to use blobing box instead intersection.
#define BY_INTERSECTION

#ifndef BY_INTERSECTION
    #define BY_BLOBBING
#endif

#include <vector>
#define INTENSISVE_CHECK

//EDITABLE
static const int maxPink = 20;
static const int OUTER_BARRIER_X = 100;
static const int OUTER_BARRIER_Y = 100;

static const int BOXSIZE_X = 5;
static const int BOXSIZE_Y = 5;

static const int ANGLE_BOXSIZE_X = 10;
static const int ANGLE_BOXSIZE_Y = 10;

//UNDER THIS LINE IS NOT TWEAKABLE

// Flag indicating whether dog should use localisation challenge
// pink detection stuff in VisualCortex
static const bool flag_LOC_CHALLENGE = true;

static const int PINK_ARRAY_SIZE_X = FIELD_WIDTH+2*OUTER_BARRIER_X;
static const int PINK_ARRAY_SIZE_Y = FIELD_LENGTH+2*OUTER_BARRIER_Y;

static const int PINK_ARRAY_BOX_SIZE_X = PINK_ARRAY_SIZE_X/BOXSIZE_X;
static const int PINK_ARRAY_BOX_SIZE_Y = PINK_ARRAY_SIZE_Y/BOXSIZE_Y;

static const int OFFSET_BOX_X = OUTER_BARRIER_X/BOXSIZE_X;
static const int OFFSET_BOX_Y = OUTER_BARRIER_Y/BOXSIZE_Y;

static const int ANGLE_ARRAY_SIZE_X = FIELD_WIDTH/ANGLE_BOXSIZE_X;
static const int ANGLE_ARRAY_SIZE_Y = FIELD_LENGTH/ANGLE_BOXSIZE_Y;

struct PinkEQ 
{
    double m;
    double c;
    int type;
};

struct PinkBlob
{
    int maxX;
    int minX;
    int maxY;
    int minY;
};

struct PinkAngleBox
{
    int posX;
    int posY;
    int angles[maxPink];
    int anglesToPink[maxPink];
};

typedef enum {
    FLUp,
    FLDown,
    FLLeft,
    FLRight
} FieldLineEnum;

#else
static const bool flag_LOC_CHALLENGE = false;
#endif

#define MAX_NO_OF_PINK_OBJECTS 5


// End define Localisation Challenge


#endif //CHALLENGELOC_H
