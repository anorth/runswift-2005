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
 * $Id: Common.h 6836 2005-06-23 13:00:40Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * contains basic utilities and functions
 * 
 **/

#ifndef _Common_h_DEFINED
#define _Common_h_DEFINED

#ifdef OFFLINE
#define OFFLINE_CODE(x) x
#define ONLINE_CODE(x) 
#else
#define OFFLINE_CODE(x)
#define ONLINE_CODE(x) x
#endif //OFFLINE

#ifdef OFFLINE
#define OSYSDEBUG(x) fprintf(stderr, x)
#define OSYSLOG1(x) fprintf(stderr, x)
#endif

#define FFCOUT cout << __FILE__ << ": " << __func__ << ": "
#define FFCERR cerr << __FILE__ << ": " << __func__ << ": "
#define FCOUT cout << __func__ << ": "
#define FCERR cerr << __func__ << ": "

#ifdef OFFLINE       
typedef long slongword;
typedef char byte;
typedef unsigned uint32;
#else
#include <Types.h>
#endif //OFFLINE

#ifdef OFFLINE
//If we need tilt calibrartion offline
//*** Used when fitting joint slop
//#define TILT_CALIBRATION
#endif

#include <math.h>

#include "RobotDef.h"
#include "FieldDef.h"
#include "BallDef.h"
#include "SwitchBoard.h"
#include "debuggingMacros.h"
#include "Challenge.h" //Challenge

/* Setting all the flags. */

#ifdef OFFLINE
static const bool flag_OFFLINE = true;
#else
static const bool flag_OFFLINE = false;
#endif 

#ifdef PRE_SANITY_BLOB
static const bool flag_PRE_SANITY_BLOB = true;
#else
static const bool flag_PRE_SANITY_BLOB = false;
#endif

#ifdef MAYBE
static const bool flag_MAYBE = true;
#else
static const bool flag_MAYBE = false;
#endif

#ifdef BAR_CHALL
static const bool flag_BAR_CHALL = true;
#else
static const bool flag_BAR_CHALL = false;
#endif

#ifdef SHAPE_CHALL
static const bool flag_SHAPE_CHALL = true;
#else
static const bool flag_SHAPE_CHALL = false;
#endif

#ifdef FIREBALL
static const bool flag_FIREBALL = true;
#else
static const bool flag_FIREBALL = false;
#endif

// max accuarcy distance of projection in cm
#define MAX_VIEW_ACCURATE_OBSTACLE 250 

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef void *voidPtr;

static const double pi = M_PI;
static const double VERY_LARGE_DOUBLE = 1.0e+30;
static const int VERY_LARGE_INT = 2000000000; //2 billions
static const int VERY_LARGE_SLONGWORD = 2000000000; //2 billions
static const double EPSILON = 0.00001;

static const int GLOBAL = 0;
static const int LOCAL = 1;

static const double FULL_CIRCLE = 360;
static const double HALF_CIRCLE = 180;
static const double QUARTER_CIRCLE = 90;

// these variables to avoid computation
static const double DEG_OVER_RAD = 180 / M_PI;
static const double RAD_OVER_DEG = M_PI / 180;
static const double TWO_PI_INV = 1 / (2 * M_PI);

#ifndef MAX
template <class T>
inline static T MAX(const T &x, const T &y) {
	return (x > y ? x : y);
}
inline static double MAX(const double x, const int y) {
	return (x > y ? x : y);
}
inline static double MAX(const int x, const double y) {
	return (x > y ? x : y);
}
#endif

#ifndef MIN
template <class T>
inline static T MIN(const T &x, const T &y) {
	return (x > y ? y : x);
}
inline static double MIN(const double x, const int y) {
	return (x > y ? y : x);
}
inline static double MIN(const int x, const double y) {
	return (x > y ? y : x);
}
#endif

template <class T>
inline static T ABS(const T &x) {
	return (x > 0 ? x : -x);
}

template <class T>
inline static T SQUARE(const T &x) {
	return x * x;
}
/*
inline static int round(const double x) {
   if (x > 0) {
      return (int)((x) + 0.5);
   } else {
      return (int)((x) - 0.5);
   }
   return 0;
}
*/
inline static double RAD2DEG(const double x) {
	return ((x) * DEG_OVER_RAD);
}
inline static double DEG2RAD(const double x) {
	return ((x) * RAD_OVER_DEG);
}
inline static double DEG2RADSQ(const double x) {
	return (x * RAD_OVER_DEG * RAD_OVER_DEG);
}
inline static slongword RAD2MICRO(const double x) {
	return (slongword) ((x) * 1000000);
}
inline static slongword DEG2MICRO(const double x) {
	return RAD2MICRO(DEG2RAD(x));
}
inline static double MICRO2RAD(const slongword x) {
	return ((x) / 1000000.0);
}
inline static double MICRO2DEG(const slongword x) {
	return RAD2DEG(MICRO2RAD(x));
}

/* The image (top left) coordinate system is pixel coordinates so the
 * point should really lie in the middle of the pixel. The camera (centre)
 * based coordinate system has origin between pixels, hence the half
 * pixel offset here. The field sizes are hardcoded due to an otherwise
 * circular dependency with VisionDef.h
 * FIXME: this is just an intelligent guess. In reality probably the camera
 * mountings might be more innaccurate than this. It really needs
 * an experiment to determine the right offset. It might be more or less than
 * half a pixel.
 */
inline static double TOPLEFT2CENTER_X(const double x) {
	return ((x) - (208/2) + 0.5); // CPLANE_WIDTH
}
inline static double TOPLEFT2CENTER_Y(const double y) {
	return ((160/2) - (y) - 0.5); // CPLANE_HEIGHT
}

inline static double CENTER2TOPLEFT_X(const double x) {
	return ((x) + (208/2) - 0.5); // CPLANE_WIDTH
}
inline static double CENTER2TOPLEFT_Y(const double y) {
	return ((160/2) - (y) + 0.5); // CPLANE_HEIGHT
}

template <class T>
inline static T DISTANCE_SQR(const T &xA,
		const T &yA,
		const T &xB,
		const T &yB) {
	return SQUARE(xA - xB) + SQUARE(yA - yB);
}

// This is slow. The sqrt probably sucks.
inline static double DISTANCE(const double xA,
		const double yA,
		const double xB,
		const double yB) {
	return sqrt(DISTANCE_SQR(xA, yA, xB, yB));
}

template <class T>
inline static int SIGN(const T &x) {
	return (x < 0 ? -1 : 1);
}

template <class T>
inline static T CLIP(const T &x, const T &bound) {
	/* clips (+/-) x to bound, assume bound is positive */
	return ((x <-bound || x> bound) ? bound * SIGN(x) : x);
}

template <class T>
inline static T CLIP(const T& x, const T& lowerBound, const T& upperBound) {
  // similar with CLIP, but distinguish lower and upper bounds
  if (lowerBound <= upperBound) {
    // if bounds are in order
    if (x < lowerBound)
      return lowerBound;
    else if (x > upperBound)
      return upperBound;
    else
      return x;
  } else {
    // if upperBound < lowerBound
    if (x < upperBound)
      return upperBound;
    else if (x > lowerBound)
      return lowerBound;
    else
      return x;    
  }
}

static inline int DECREMENT(int &x) {
	return (x = (x <= 0 ? 0 : x - 1));
}

//Robot dimension constants, etc.

static const int ROBOTS = 8;
static const int NUM_TEAM_MEMBER = 4;
#ifdef ONLY_ONE_DOG
static const int NUM_OPPONENTS = 1;
#else
static const int NUM_OPPONENTS = 4;
#endif //ONLY_ONE_DOG

static const int NUM_FRIENDS_VIS = 3;
static const int NUM_FORWARDS = 2;
static const int VISION_BALL = 0;
static const int WIRELESS_BALL = 1;
static const int NO_BALL       = 4;
static const int GPS_BALL = 2;
static const int TEAMMATE_HAS_BALL = 3; //used by maverick.. shouldn't be here but otherwise the number 3 might get used by someone else

//given local coordinate, get global
void getGlobalCoordinate(double myX, double myY, double myH, 
double localX, double localY, double * x, double * y);        

//given global coordinate, get local
void getLocalCoordinate(double myX, double myY, double myH,
double globalX, double globalY, double * x, double * y);    

double NormalizeAngle_0_360(double angle);
double NormalizeAngle_180(double angle);
double NormalizeAngle_PI(double angle);
double NormalizeAngle_m90_270(double angle);
double NormalizeAngle_90_450(double angle);
int byte2UInt(byte);
int ReadSignedInt(void);
bool FieldClipping(double &x, double &y);
bool IsInField(double x, double y);

/* 
 * @return true if (cX,cY)-(pX,pY) cuts vertical line with 
 * Xval (Xval,WALL_THICKNESS) - (Xval,FIELD_LENGTH - WALL_THICKNESS) at resY
 * otherwise false.
 **/
bool cutVertical(double cX,
		double cY,
		double pX,
		double pY,
		double Xval,
		double *resY);

/* 
 * @return true if (cX,cY)-(pX,pY) cuts horizontal line with 
 * Yval (WALL_THICKNESS,Yval) - (FIELD_WIDTH-WALL_THICKNESS,Yval) at resX
 * otherwise false.
 **/
bool cutHorizontal(double cX,
		double cY,
		double pX,
		double pY,
		double Yval,
		double *resX);

/* if (pX,pY) is out of the field, it should be clipped but
 * the gradient of the line (pX,pY) and (cX,cY) should not be modified.
 * So check if (pX,pY)-(cX,cY) cuts the 4 bounderies then that is
 * the clipped point.
 * @param cX is the centered X
 **/
void angleClipping(double cX, double cY, double *pX, double *pY);

/* 
 * @return true if (cX,cY)-(pX,pY) cuts vertical line with 
 * Xval (Xval,0) - (Xval,FIELD_LENGTH) at resY
 * otherwise false.
 **/
bool cutVerticalMore(double cX,
		double cY,
		double pX,
		double pY,
		double Xval,
		double *resY);

/* 
 * @return true if (cX,cY)-(pX,pY) cuts horizontal line with 
 * Yval (0,Yval) - (FIELD_WIDTH,Yval) at resX
 * otherwise false.
 **/
bool cutHorizontalMore(double cX,
		double cY,
		double pX,
		double pY,
		double Yval,
		double *resX);

/* if (pX,pY) is out of the field, it should be clipped but
 * the gradient of the line (pX,pY) and (cX,cY) should not be modified.
 * So check if (pX,pY)-(cX,cY) cuts the 4 bounderies then that is
 * the clipped point.
 * @param cX is centered X
 **/
void angleClippingMore(double cX, double cY, double *pX, double *pY);

inline int determinant(int a1,
		int b1,
		int c1,
		int a2,
		int b2,
		int c2,
		int a3,
		int b3,
		int c3) {
	return a1 * b2 * c3
		- a1 * b3 * c2
		+ a2 * b3 * c1
		- a2 * b1 * c3
		+ a3 * b1 * c2
		- a3 * b2 * c1;
}

/* 
 * @return radian
 **/
double getHeadingBetween(double fromX, double fromY, double toX, double toY);

double getDistanceBetween(double fromX, double fromY, double toX, double toY);

/*
 * spits out the 95% confidence factor of the variance of
 * of a given value
 **/
inline double get95CF(double standardDeviation) {
	return SQUARE(standardDeviation) / 4;
}

//floating point comparison 
inline int doubleCMP(double a,double b){
    if (a < b - EPSILON) return -1;
    else if (a > b + EPSILON) return 1;
    else return 0;
}

/*
 * calculate the offset position(pX,pY) which are distance d away at an angle ang from the pivot position (x, y, h)
 * @param x the current x loc
 * @param y the current y loc
 * @param h the current heading (absolute)
 * @param d the distance to walk
 * @param ang the angle to walk, relative to the current heading (ccw is +ve)
 * @image html calPosition.gif
 **/
void calPosition(double x,
		double y,
		double h,
		double d,
		double ang,
		double *pX,
		double *pY);

/*
 *Takes inNumPoints points as local co-ordinate pairs in inLocal and transforms
 *them into global co-ordinate pairs in outGlobal based on inX, inY and inT.
 *NOTE: outGlobal must be twice inNumPoints in size (as must inLocal) or we'll
 *have problems. This routine works even if inLocal and outGlobal are the same. 
 *note, dog heading in Radian.
 */
void localToGlobal(double dogX, double dogY, double dogHeading, int inNumPoints, double *inLocal, double *outGlobal);

//====================================================================
//quicksort template function
//====================================================================
struct sortee {
	double id;
	double weight;
	bool opponent;

	bool operator>=(sortee s) {
		return this->weight >= s.weight;
	}

	bool operator<=(sortee s) {
		return this->weight <= s.weight;
	}
};

template <class T>
void swap(int i, int j, T *array) {
	T temp = array[i];
	array[i] = array[j];
	array[j] = temp;
}


template <class T>
int partition(int first, int last, T *array) {
	int left = first;
	int right = last;

	while (left < right) {
		while ((array[left] <= array[first]) && (left < last)) {
			left++;
		}
		while ((array[right] >= array[first]) && (right > first)) {
			right--;
		}
		if (left < right)
			swap(left, right, array);
	}
	swap(right, first, array);
	return right;
}


template <class T>
void qSort(int first, int last, T *array) {
	int middle;
	if (first < last) {
		middle = partition(first, last, array);
		qSort(first, middle - 1, array);
		qSort(middle + 1, last, array);
	}
}

template <class T>
void quickSort(T *array, int arraySize) {
	int last = arraySize - 1;
	qSort(0, last, array);
}


/** 
  * CUSTOMIZED ROBOTICS QUICK-SORT 
  */

template<class T>
void swap(int i, int j, T *array1, T *array2) {
   T temp = array1[i];
   array1[i] = array1[j];
   array1[j] = temp;

   temp = array2[i];
   array2[i] = array2[j];
   array2[j] = temp;   
}


template<class T>
int partition(int first, int last, T *array, T *array2) {

   int left  = first;
   int right = last;

   while (left < right) {
      while ((array[left] <= array[first]) && (left<last)) {
         left++;
      }
      while ((array[right] >= array[first]) && (right>first)) {
         right--;
      }
      if (left < right)
         swap(left, right, array, array2);
   }
   swap(right, first, array, array2);
   return right;
}


template<class T>
void qSort (int first, int last, T *array1, T *array2) {
   int middle;
   if (first < last){ 
       middle = partition(first,last,array1, array2);
       qSort(first, middle-1, array1, array2);
       qSort(middle+1, last, array1, array2);
   }
}

template<class T>
void quickSort(T *array1, T *array2, int arraySize) {
   int last = arraySize - 1;
   qSort (0,last,array1,array2);
}

// Call with d = 0 to crash the robot. Put an informative message
// in msg for viewing in gdb/ddd
inline int die(const char* msg = 0, int d = 0) {
    (void)msg;
    return 1/d;
}

#endif // _Common_h_DEFINED


