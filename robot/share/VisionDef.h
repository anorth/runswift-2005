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
 * @author UNSW 2003 Robocup (Will Uther)
 *
 * Last modification background information
 * $Id: VisionDef.h 5168 2005-03-22 05:30:13Z alexn $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/


#ifndef VisionDef_h_DEFINED
#define VisionDef_h_DEFINED

#include "SanityConstant.h"
#include "Common.h"

// size of primitive types
static const int INT_BYTES = 4;
static const int LONG_BYTES = 4;
static const int INT_BITS = INT_BYTES * 8;

// direction constants
static const int DIR_UP = 0;
static const int DIR_LEFT = 1;
static const int DIR_DOWN = 2;
static const int DIR_RIGHT = 3;

// size of the cplane
static const int CPLANE_WIDTH = 208;
static const int CPLANE_HEIGHT = 160;
static const int CPLANE_SIZE = CPLANE_WIDTH *CPLANE_HEIGHT;  // 25344
static const int COMPRESSED_CPLANE_SIZE = CPLANE_SIZE * 2; // the maximum size of the compressed cplane using run-length encoding
static const int SUB_CPLANE_SAMPLE = 2; // sample every n'th pixel in subplane
static const int SUB_CPLANE_SIZE = COMPRESSED_CPLANE_SIZE /
                                    (SUB_CPLANE_SAMPLE * SUB_CPLANE_SAMPLE);

// size of the sensors
static const int NUM_SENSOR_COORDS = 3;
static const int SENSOR_SIZE = NUM_SENSOR_COORDS *LONG_BYTES;

// size of the sanity values
static const int NUM_SANITY_COORDS = (INSANITY_COUNT - 1) / INT_BITS + 1;
static const int SANITY_SIZE = NUM_SANITY_COORDS *INT_BYTES;

// size of objects
static const int NUM_OBJ = 15;
static const int NUM_INT_COORDS = 6;
static const int NUM_ROBOT_DATA = 2;
// Tried to add hz data as well, not successful so comment out for now.
//static const int NUM_ROBOT_DATA = 7;  

static const int ONE_OBJ_SIZE = NUM_INT_COORDS *INT_BYTES;
static const int OBJ_SIZE = NUM_OBJ *ONE_OBJ_SIZE
+ INT_BYTES *NUM_ROBOT_DATA; // use integer to represent the double values

static const int YUVPLANE_SCALE = 6;
static const int YUVPLANE_SIZE = CPLANE_SIZE *YUVPLANE_SCALE;  // 76032 

static const int DEBUG_PLANES = 3;
static const int DEBUG_SIZE = CPLANE_SIZE *DEBUG_PLANES;    // 76032 

// size of gps
#ifdef ONLY_ONE_DOG
static const int GPS_SIZE = 288; //TODO: derive GPS_SIZE from other constants
#else
static const int GPS_SIZE = 360;
#endif //ONLY_ONE_DOG



/* According to Sony model info, 
   vertical angle of view is 45.2 degree.
   horizontal angle of view is 56.9 degree. */
static const double HORI_VIEWABLE_DEG = 56.9;
static const double HORI_VIEWABLE = DEG2RAD(HORI_VIEWABLE_DEG);

static const double VERT_VIEWABLE_DEG = 45.2;
static const double VERT_VIEWABLE = DEG2RAD(VERT_VIEWABLE_DEG);

/* Angle per pixel. This is the average over the image: horizontal is
 * slightly smaller than vertical and there are more degrees/pixel near the 
 * centre of the image than at the edges. Accurate to 3% though.
 */
static const double ANGLE_PER_PIXEL = (HORI_VIEWABLE/ CPLANE_WIDTH 
                                    + VERT_VIEWABLE/ CPLANE_HEIGHT) / 2;
static const double DEGREES_PER_PIXEL = (HORI_VIEWABLE_DEG / CPLANE_WIDTH 
                                    + VERT_VIEWABLE_DEG / CPLANE_HEIGHT) / 2;

static const double CAMERA_CPLANE_HORI_CONST = ((CPLANE_WIDTH/2) / 
                                                 tan(HORI_VIEWABLE/2));
static const double CAMERA_CPLANE_VERT_CONST = ((CPLANE_HEIGHT/2) / 
                                                 tan(VERT_VIEWABLE/2));


//-------------------------------------- Some will be thrown away. 

// Horizontal viewable angle in radians
// ERS210 value for xField was 1.012290966
static const double xField = 0.9939092344;

// Vertical viewable angle in radians
// ERS210 value = 0.837758041 (48.0 degree vertical)
static const double yField = 0.788888821;
// yField was set to 0.77603490013023746 before (not sure why)

//probably outdated value
static const double offsetValue = 0.052359877;

//TODO : check this
const double yFieldDegree = 44.463524;

//doesn't really matter in projection formulea
static const double APERTURE_DISTANCE = CPLANE_HEIGHT
/ (2 * tan(yField / 2)) ;

//-------------------------------------- 

/* Other visual utility functions */

// Calculates Image Heading using cplane x value.
inline double pointToHeading(double x) {
    return RAD2DEG(atan(((CPLANE_WIDTH / 2.0) - x) * 2.0
                * tan(xField / 2.0) / CPLANE_WIDTH));
}

// Calculates Image Elevation using cplane y value.
inline double pointToElevation(double y) {
    return RAD2DEG(atan(((CPLANE_HEIGHT / 2.0) - y) * 2.0
                * tan(yField / 2.0) / CPLANE_HEIGHT));
}

// Calculates Rotated Image Heading.
inline double pointToHeading(double x, double y, double sin_eroll,
                                double cos_eroll) {
    return RAD2DEG(
            atan(
                (
                      (CPLANE_WIDTH  / 2.0 - x) * cos_eroll
                    - (CPLANE_HEIGHT / 2.0 - y) * sin_eroll) * 2.0 * tan(xField / 2.0)
                / CPLANE_WIDTH)
            );
}

// Calculates Rotated Image Elevation
inline double pointToElevation(double x, double y, double sin_eroll,
                                double cos_eroll) {
    return RAD2DEG(
            atan(
                (
                    (CPLANE_WIDTH  / 2.0 - x) * sin_eroll
                  + (CPLANE_HEIGHT / 2.0 - y) * cos_eroll) * 2.0 * tan(yField / 2.0)
                / CPLANE_HEIGHT)
            );
}

#endif // VisionDef_h_DEFINED

