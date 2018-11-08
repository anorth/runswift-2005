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
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 * 
 * The c-plane is forwarded to the blob forming process to allow objects constructed as simple blobs of one colour
 * There are two phases in the algorithm:
 * - For each row, the contiguous pixels of the same colour are combined to form the s-plane (segmented plane).
 * - Form blobs from the s-plane. Blobs are segments of the same colour that are connected in any way.
 *
 **/

#ifndef BLOB_H
#define BLOB_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <cfloat>
#include <queue>
#include <list>
#include <vector>
#include <algorithm>

using std::cout;
using std::endl;
using std::min;

#include "../share/Common.h"
#include "GeneralisedColourDetect.h"

#ifndef OFFLINE
#include <OPENR/OFbkImage.h>
#endif

using std::list;
using std::vector;
using std::iterator;
using std::priority_queue;

static const int MAX_NUM_BLOBS = 550;
static const int MIN_OVERLAP_PIXELS = 5;
static const int LONG_SEGMENT_LENGTH = 10;

class RunLengthInfo {
	public:
		int startIndex, endIndex, length;
		uchar color;
		uchar rawColor;
		int blob_number;
		int xsum, ysum;

		// Disjoint sets algorithm fields.
		//    rank is the the upper bound height of a blob.
		//    nextSeg point to the next segment in the disjoint-set forsets. 
		int rank;
		RunLengthInfo *nextSeg;

		void Set(int y, int s, int e, uchar c) {
			startIndex = s;
			endIndex = e;
			rank = 0;
			length = e - s + 1;
			rawColor = c;
			color = c & COLOR_MASK;
			blob_number = -1;
			nextSeg = NULL;
			xsum = ((2 * s + (length - 1)) * length) / 2;
			ysum = y*length;
		}

	
		bool overlaps(RunLengthInfo *segment) {
			return this->startIndex <= segment->endIndex 
				&& this->endIndex >= segment->startIndex ;
		}

        //8 direction case
		bool overlaps8(RunLengthInfo *segment) {
			return this->startIndex <= segment->endIndex + 1
				&& this->endIndex >= segment->startIndex - 1;
		}
};

/* Segmented Plane is the a two dimensional array runLengthInfo's.  
 * A series of these forms the segments of a cpl  Set(y, x, e,
 ane row
 **/
struct SegmentedPlane {
    RunLengthInfo segment[HEIGHT][WIDTH]; /* Every pixel can be a segment. */
    int segmentCount[HEIGHT];             /* Segments found in each row. */
};

struct ColorListElem {
  uchar color;
  int count;

  ColorListElem(uchar elemColor, int elemCount):
    color(elemColor),
    count(elemCount)
  {}

  bool operator <(const ColorListElem& elem) const {
    return count >= elem.count;
  }

  void operator =(const ColorListElem& elem) {
    color = elem.color;
    count = elem.count;
  }
};

typedef list<ColorListElem> ColorList;

//RunLengthInfo + descendingly sorted list of color count
class eRunLengthInfo : public RunLengthInfo {

 public:

  bool isEdge;
  long ypsum, upsum, vpsum;
  ColorList colorLst;
  eRunLengthInfo *nextSeg;

  //Don't know why this can't be inherited from RunLengthInfo
  void Set(int y, int s, int e, uchar c, bool edge) {
    startIndex = s;
    endIndex = e;
    rank = 0;
    length = e - s + 1;
    rawColor = c;
    color = c & COLOR_MASK;
    blob_number = -1;
    nextSeg = NULL;
    xsum = ((2 * s + (length - 1)) * length) / 2;
    ysum = y*length;
    isEdge = edge;
  }

  void Set(int y, int s, int e, long yp, long up, long vp, uchar c, bool edge) {
    ypsum = yp;
    upsum = up;
    vpsum = vp;
    length = e - s + 1;
    Set(y, s, e, c, edge);
  }

  void Set(int y, int s, int e, ColorList cl, bool edge) {
    colorLst = cl;
    colorLst.sort();
    Set(y, s, e, colorLst.front().color, edge);
  }

  void UpdateLength(int y, int newEnd) {
    length += newEnd - endIndex;
    endIndex = newEnd;
    xsum = ((2 * startIndex + (length - 1)) * length) / 2;
    ysum = y*length;
  }

  bool overlaps(eRunLengthInfo *segment) {//segment is in previous row
    if(this->isEdge)
      return this->color == segment->color &&
	this->startIndex <= segment->endIndex &&
	this->endIndex >= segment->startIndex;
    else if(!segment->isEdge) //&& !this->isEdge
      return this->startIndex <= segment->endIndex-MIN_OVERLAP_PIXELS &&
	this->endIndex >= segment->startIndex+MIN_OVERLAP_PIXELS;
    else //!this->isEdge && segment->isEdge
      return false;
  }
};

struct eSegmentedPlane {
    eRunLengthInfo segment[HEIGHT][WIDTH]; /* Every pixel can be a segment. */
    int segmentCount[HEIGHT];             /* Segments found in each row. */
};

/* Point structure, used in geometrical calculations */
struct Point {
	int x;
	int y;
};

/* Axis Types are used in a sequence length.
 * It defines which cplane axis the Length intersects with.
 **/
typedef enum {
	X,
	Y
} Axis;

/* LengthState are types that judge the reliability of the information 
 * contained within this sequence length. The quality of the information runs 
 * from Excellent to Useless in the order shown.
 **/
typedef enum {
	READY,
	RELIABLE,
	UNRELIABLE,
	USELESS,
	USED
} LengthState;

class SequenceLength {
	Point myPoint;
	// These functions are boolean functions that define relationships between segments and the picture frame
	bool onFrame() {
		if (axis == X) {
			return (intercept == 0
					|| intercept >= WIDTH - 1
					|| startIndex == 0
					|| endIndex >= HEIGHT - 2);
		}
		else {
			return (intercept == 0
					|| intercept >= HEIGHT - 2
					|| startIndex == 0
					|| endIndex >= WIDTH - 1);
		}
	}
	bool startIndexOnFrame() {
		return startIndex == 0;
	}

	bool endIndexOnFrame() {
		if (axis == X) {
			return endIndex == HEIGHT - 2;
		}
		else {
			return endIndex == WIDTH - 1;
		}
	}

	bool interceptOnFrame() {
		if (axis == X) {
			return intercept == 0 || intercept >= WIDTH - 1;
		}
		else {
			return intercept == 0 || intercept >= HEIGHT - 2;
		}
	}

	public:
	// The axis that this sequence runs intersects with
	Axis axis;
	int intercept;
	int startIndex, endIndex;

	LengthState state;

	void storeYExtreme(int y, int startX, int endX) {
		axis = Y;
		intercept = y;
		startIndex = startX;
		endIndex = endX;
		state = READY;
	}

	// updateYExtreme - store the end of a x range for an y extreme
	void updateYExtreme(int y, int endX) {
		endIndex = endX;
	}

	int midPoint() {
		return (startIndex + endIndex) / 2;
	}

	// storeXExtreme - store an x extreme, and the begining of its y range
	void storeXExtreme(int x, int startY) {
		axis = X;
		intercept = x;
		startIndex = endIndex = startY;
		state = READY;
	}

	// updateXExtreme - store the end of a y range for an x extreme
	void updateXExtreme(int endY) {
		endIndex = endY;
	}

	void updateExtreme(SequenceLength &seq) {
		if (seq.axis != this->axis || seq.intercept != this->intercept) {
			return;
		}
		if (seq.startIndex < this->startIndex) {
			this->startIndex = seq.startIndex;
		}
		if (seq.endIndex > this->endIndex) {
			this->endIndex = seq.endIndex;
		}
	}
              
	bool findPoint(Point &p) {
		if (this->axis == X) {
			myPoint.x = intercept;
			myPoint.y = (endIndex - startIndex) / 2 + startIndex;
		}
		else {
			myPoint.y = intercept;
			myPoint.x = (endIndex - startIndex) / 2 + startIndex;
		}
		p = myPoint;
		return !(this->onFrame());
	}

	bool findPoint2(Point &p) {
		if (this->axis == X) {
			myPoint.x = intercept;
			if (startIndexOnFrame())
				myPoint.y = endIndex;
			else
				myPoint.y = startIndex;
		}
		else {
			myPoint.y = intercept;
			if (startIndexOnFrame())
				myPoint.x = endIndex;
			else
				myPoint.x = startIndex;
		}
		p = myPoint;
		return (!interceptOnFrame());
	}

	bool findStartPoint(Point &p) {
		if (this->axis == X) {
			myPoint.x = intercept;
			myPoint.y = startIndex;
		}
		else {
			myPoint.y = intercept;
			myPoint.x = startIndex;
		}
		p = myPoint;
		return (!startIndexOnFrame());
	}

	/* findPoint returns the point, (intercept, end_index)
	 * @return true if the end extreme is not on the border of the 
	 * frame,<br> false otherwise
	 **/
	bool findEndPoint(Point &p) {
		if (this->axis == X) {
			myPoint.x = intercept;
			myPoint.y = endIndex;
		}
		else {
			myPoint.y = intercept;
			myPoint.x = endIndex;
		}
		p = myPoint;
		return (!endIndexOnFrame());
	}

	const Point & getMyPoint() {
		return myPoint;
	}
};

static const int MAX_POINTS = 4;

static const int WEST = 0;
static const int NORTH = 1;
static const int EAST = 2;
static const int SOUTH = 3;

static const int SEARCH_LENGTH = 16;

static const int searchOrder[SEARCH_LENGTH] = {
	EAST,
	SOUTH,
	WEST,
	NORTH,
	EAST,
	WEST,
	NORTH,
	SOUTH,
	EAST,
	WEST,
	NORTH,
	SOUTH,
	EAST,
	SOUTH,
	WEST,
	NORTH
};

// BlobInfo
class BlobInfo {
	public:
	
		int large;

		// Area of the Blob
		int area;
		int maybeArea;

		int blob_number;

		// The Boundary segments
		SequenceLength boundary[MAX_POINTS];

		// True, if the blob has been processed by
		// Blobber::findHalfBeacon
		bool blobProcessed;

		// sum of x and y coor and number of pixels
		int xsum, ysum;

#ifdef SHAPE_CHALL
		// True, if the blob has been included in the shape
		bool enqueued;
#endif

        /* xRmin, yRmin, xRmax, yRmax - are the ROTATION version of their 
         * counter part. (Unrotated can be got from getYMin, getYMax... */
		int xRmin, yRmin, xRmax, yRmax, cRx, cRy;
		int color;

	protected:
		int *priority;

		SegmentedPlane *splane;

	private:
		int chosen[MAX_POINTS];
		int numChosen;

		int sIndex;

		bool southernProblem(int extreme) {
			return (extreme == SOUTH) && boundary[SOUTH].intercept >= HEIGHT - 2;
		}

		bool isNewPoint(Point &p) {
			for (int i = 0; i < numChosen; i++) {
				const Point &curP = boundary[chosen[i]].getMyPoint();

				// Increase the value of that the square of the distance
				//  between the old and new points to dismiss less acurate circles.
				// Currently the new point must be at least 3 pixel's distant
				if (DISTANCE_SQR(curP.x, p.x, curP.y, p.y) <= 9)
					return false;
			}
			return true;
		}

		bool findMin(int min, int y, Point &p) {
			for (int i = 0; i < splane->segmentCount[y]; i++) {
				if (splane->segment[y][i].color == cBALL
						&& splane->segment[y][i].startIndex > min) {
					p.x = splane->segment[y][i].startIndex;
					p.y = y;
					return true;
				}
			}
			return false;
		}

	public:

		void Init(int myColor) {
			color = myColor;
			if (color == cROBOT_BLUE || color == cROBOT_RED) {
				priority = &boundary[WEST].intercept;
			}    // xmin
			else {
				priority = &area;
			}
		}

		/**
		 * Set-up a new blob, with one segment.
		 **/

		void Start(int rawColor, int min, int max, int y, long a, int bn) {
			boundary[WEST].storeXExtreme(min, y);
			boundary[NORTH].storeYExtreme(y, min, max);
			boundary[EAST].storeXExtreme(max, y);
			boundary[SOUTH].storeYExtreme(y, min, max);
			area = a;
			blob_number = bn;
			large = max;
			if (rawColor & MAYBE_BIT) {
				maybeArea = a;
			} else {
				maybeArea = 0;
			}
		}

		void Update(int rawColor, int min, int max, int y, long a, int xs, int ys) {
			Update(min, max, y, a);
			xsum += xs;
			ysum += ys;
			if (rawColor & MAYBE_BIT) {
				maybeArea += a;
			}
		}

		void Update(int min, int max, int y, long a) {
			// update NORTH
			if (y == getYMin()) {
				boundary[NORTH].updateYExtreme(y, max);
			}

			// update WEST
			if (min == getXMin()) {
				boundary[WEST].updateXExtreme(y);
			}
			else if (min < getXMin()) {
				boundary[WEST].storeXExtreme(min, y);
			}

			//update EAST
			if (max == getXMax()) {
				boundary[EAST].updateXExtreme(y);
			}
			else if (max > getXMax()) {
				boundary[EAST].storeXExtreme(max, y);
			}

			//update SOUTH
			if (y == getYMax()) {
				boundary[SOUTH].updateYExtreme(y, max);
			}
			else {
				boundary[SOUTH].storeYExtreme(y, min, max);
			}
			area += a;
			}


			void Update(BlobInfo *otherBlob) {
		
			if (otherBlob->getYMax() == getYMax()) {
				if (otherBlob->large > large)
					large = otherBlob->large;
					
			}
			
			if (otherBlob->getYMax() > getYMax()) {
				large = otherBlob->large;			

			}
					
			for (int i = 0; i < 4; i++) {
				if (otherBlob->boundary[i].intercept == this->boundary[i].intercept)
					boundary[i].updateExtreme(otherBlob->boundary[i]);

				if (i == WEST || i == NORTH) {
					if (otherBlob->boundary[i].intercept < this->boundary[i].intercept)
						boundary[i] = otherBlob->boundary[i];
				}
				else {
					if (otherBlob->boundary[i].intercept > this->boundary[i].intercept)
						boundary[i] = otherBlob->boundary[i];
				}

				boundary[i].updateExtreme(otherBlob->boundary[i]);
			}
			xsum += otherBlob->xsum;
			ysum += otherBlob->ysum;
			area += otherBlob->area;
			maybeArea += otherBlob->maybeArea;
		}

		void initBallBlob(SegmentedPlane *mySplane) {
			splane = mySplane;
		}

		void setXMin(int val) {
			boundary[WEST].intercept = val;
		}

		void setXMax(int val) {
			boundary[EAST].intercept = val;
		}
                
		void setYMin(int val) {
			boundary[NORTH].intercept = val;
		}
                
		void setYMax(int val) {
			boundary[SOUTH].intercept = val;           
		}
         
		int getXMin() const {
			return boundary[WEST].intercept;
		}
		int getXMax() const {
			return boundary[EAST].intercept;
		}
		int getYMin() const {
			return boundary[NORTH].intercept;
		}
		int getYMax() const {
			return boundary[SOUTH].intercept;
		}
		void ResetBlob() {
			area = 0;
			blobProcessed = false;
			sIndex = -1;
			numChosen = 0;
			blob_number = -1;
		}

		BlobInfo(int myColor = -1) {
			ResetBlob();
			Init(myColor);
		}
};               
           
         
/**
 * A Half Beacon is a Blob, that has determined all its extreme     
 * points and/or geometries.               
 * It has other member fields, such as the centroid and perimeter.     
 **/        
class HalfBeacon : public BlobInfo {
	public:         
               
		bool matched;
		double cx, cy;
		//ERIC centroid debugging stuff
        /*
		double cx2, cy2;
		double cx3, cy3;
        */

		int half_perimeter;

		friend bool operator<(const HalfBeacon x, const HalfBeacon y) {
			return *(x.priority) < *(y.priority);
		}
		friend bool operator>(const HalfBeacon x, const HalfBeacon y) {
			return *(x.priority) > *(y.priority);
		}

		void Reset() {
			matched = false;
			cx = cy = 0;
			half_perimeter = 0;
		}

		void formationComplete() {

            cx = (double) xsum / (double) area;
			cy = (double) ysum / (double) area;

			matched = false;
			half_perimeter = getXMax() - getXMin() + getYMax() - getYMin();
			blobProcessed = true;
		}
};


/* Very useful imits... when changing from ERS7 to ERS210, many more red blobs 
** are required to be seen, and red blobs can be much smaller, these changes are
** required in here. */
struct BlobLimits {
	 int color, numBlobs, minArea, maxArea, height, width;
};

const BlobLimits limits[NUM_USABLE_COLORS] = {

    /* This order is important! Notice how limits array is used. */
    /* The first element co-incident (color) co-incident with its index in
     * the limits array so that the array is easily accessible (cBall = 0,
     * cBEACON_BLUE = 1, cBEACON_GREEN = 2, etc). */ 
	 {cBALL,            10, 15, INT_MAX, INT_MAX, INT_MAX},

     {cBEACON_BLUE,     10, 10, INT_MAX, INT_MAX, INT_MAX},
     {cBEACON_GREEN,    10, 10, INT_MAX, INT_MAX, INT_MAX},
     {cBEACON_YELLOW,   15, 10, INT_MAX, INT_MAX, INT_MAX},
     {cBEACON_PINK,     10, 10, INT_MAX, INT_MAX, INT_MAX},

     {cROBOT_BLUE,      20, 15, INT_MAX, INT_MAX, INT_MAX},
     {cROBOT_RED,       10, 10, INT_MAX, INT_MAX, INT_MAX}
};


class BlobArray {
	public:

	int maxSize;
	int minAreaThreshold;
	int maxAreaThreshold;
	int widthThreshold;
	int heightThreshold;
	int color;

	HalfBeacon *curBottom;
	HalfBeacon *array[MAX_NUM_BLOBS];

	int count;
	int size;

	bool add(HalfBeacon &hb) {
		if (hb.area <minAreaThreshold || hb.area> maxAreaThreshold) {
			return false;
		}

		if (size >= MAX_NUM_BLOBS) {
			cout << "Warning from BlobArray!" << endl;
			return false;
		}

		array[size++] = &hb;       
		count = min(maxSize, ++count);        
		return true;       
		
	}

	void init(int i) {
		color = i;
		maxSize             = limits[i].numBlobs;       
		minAreaThreshold    = limits[i].minArea;        
		maxAreaThreshold    = limits[i].maxArea;          
		widthThreshold      = limits[i].width;            
		heightThreshold     = limits[i].height;        
		clear();
	}         
           
	HalfBeacon & operator[](const int i) {             
		return *(array[i]);         
	}             
           
	int getCount() {          
		return count;            
	}          
           
	int numCandidates() {              
		return size;            
	}              

	void clear() {           
		count = 0;             
		curBottom = 0;        
		size = 0;        
		//doesn't properly reset everything to NULL
	}
};

 class BlobHeap : public priority_queue<HalfBeacon> {
	// useable size of heap
	int maxSize;

	//thesholds to enter this heap
	int minAreaThreshold;
	int maxAreaThreshold;
	int widthThreshold;
	int heightThreshold;

	// colour that uses this heap
	int color;

	// current minimum of the heap - used as an additional threshold to 
	// gain entrance to this heap when it is full
	HalfBeacon *curBottom;

	// counts upto maxSize
	int count;

	public:

	bool add(HalfBeacon &hb) {
		if (hb.area <minAreaThreshold || hb.area> maxAreaThreshold) {
			return false;
		}

		if (curBottom == 0) {
			curBottom = &hb;
		}
		else if (hb < *curBottom) {
			if (count < maxSize) {
				curBottom = &hb;
			}
			else {
				return false;
			}
		}

		this->push(hb);
		count = min(maxSize, ++count);
		return true;
	}

	void next() {
		count--;
		this->pop();
	}

	void init(int i) {
		color = i;
		maxSize = limits[i].numBlobs;
		minAreaThreshold = limits[i].minArea;
		maxAreaThreshold = limits[i].maxArea;
		widthThreshold = limits[i].width;
		heightThreshold = limits[i].height;
		clear();
	}

	int getCount() {
		return count;
	}

	void clear() {
		// clears the heap
		c.clear();
		count = 0;
		curBottom = 0;
	}
};

/*
 * The Blobber class is a new class
 * It encapsulates the frequently used Blob Forming and findHalfBeacon Methods.
 **/
class Blobber {
	private:

		// A structure used temporarily in form blobs
		vector<int> openBlobs[NUM_USABLE_COLORS];

		int nbob[NUM_USABLE_COLORS];

		static uchar dcdY[256];
		static uchar dcdU[256];
		static uchar dcdV[256];

		/**
		 * The array which contains all the blobs found in the splane
		 */

		HalfBeacon blobinfo[NUM_USABLE_COLORS][MAX_NUM_BLOBS];

		BlobHeap blobs[NUM_USABLE_COLORS];

		BlobArray barray[NUM_USABLE_COLORS];


		// called by formblobs depending on whether an array or a heap is being used
		void updateHeap(int y);
		void updateArray(int y);
		
		// a stack that is used to hold the blob numbers that are not currently 
		// being used
		int available_bn[NUM_USABLE_COLORS][MAX_NUM_BLOBS];

		// the top of the stack pointer for each color
		int top_bn[NUM_USABLE_COLORS];

		// creates a new blob, used by blob former
		void createBlob(RunLengthInfo *segment, int y);

		// updates the blob numbers for each segment
		void updateBlobNums(RunLengthInfo segment[],
				int start,
				int max,
				int oldnum,
				int newnum,
				int colorMatch,
				int newcolor);

		//edge functions
		int getCrossVal(uchar* p, long row);
		void findEdge(uchar *yp, uchar *up, uchar *cp, long skip);
		void findVerticalEdge(uchar *yp, uchar *up, long skip);
		void findHorizontalEdge(uchar* yp, uchar *up, uchar*vp, long skip);

		void parseSegment(int column, int start, int end, 
				  uchar* yp, uchar* up, uchar* cp, 
				  int colorArray[], 
				  long sumY, long sumYSqr, long sumU, long sumUSqr);
		void parseVerticalSegment(int start, int end, int row, uchar* yp, uchar* up, long skip);
		void parseHorizontalSegment(int column, int start, int end, uchar* yp, uchar* up, uchar* vp, long skip);
		void saveColorSegment(int column, int start, int end, uchar color);
		void saveEdgeSegment(int column, int start, int end, ColorList cl, bool edge);
		void saveEdgeSegment(int column, int start, int end, long sumY, long sumU, long sumV, bool edge);
		void updateSegmentLength(int column, int end);

		void jointSegments(int row,RunLengthInfo *cs, RunLengthInfo *ps);


	public:

		/**
		 * The colour plane of the last camera frame
		 **/
		uchar *cplane;
		unsigned char *nn_multi_colour;
    void reloadNNMC(GeneralisedColourDetect &gcd);
	void test();
	void updateBlobNums(RunLengthInfo segment[]);
	void createOriginalBlob(RunLengthInfo *segment, int y);

		void setCPlane(uchar *newcplane) {
			cplane = newcplane;
		}

		bool usingHeap;
		bool usingAlt;
		SegmentedPlane splane;
		eSegmentedPlane esplane;
		uchar edgePlane[HEIGHT][WIDTH];
		Blobber(GeneralisedColourDetect &gcd);

		void formDisjoint();
		void formBlobs();
		void formOriginalBlobs();
		

		/**
		 * Returns the Blob at desired index's of the 2-dim array: blobinfo
		 */
		HalfBeacon &getBlob(int i, int j);
		HalfBeacon &getHalfBeacon(int i, int j);
		int findHalfBeacons(int colorIndex, HalfBeacon **hbArray, int hbArraySize);
		void reset();	
		int count(int colour);

		// Boolean flag to indicate whether to use multi_colour pixels or not.
		int multi_colour;

		void qsort(int colour);
		void orderMin(int colour, int lo0, int hi0);
		void rotate(int colour, double sin_eroll, double cos_eroll);
		void add(int colour, HalfBeacon &hb);

#ifndef OFFLINE
		void replaceCplane(OFbkImageInfo *info, byte *data);
#endif //OFFLINE

		// Do segmentation and run length encoding in one run.
		void replaceCplane(uchar *yp, uchar *up, uchar *vp, long skip);
		
		void initDCD(const DogColourDistortion &dcd);
};

inline void Blobber::reset() {

    #ifndef MAYBE
	for (int i = 0; i < NUM_USABLE_COLORS; i++) {
		barray[i].clear();
		for (int j = 0; j < nbob[i]; j++) {
			blobinfo[i][j].ResetBlob();
			blobinfo[i][j].Reset();
		}
		top_bn[i] = -1;
	}

	for (int k = 0; k < NUM_USABLE_COLORS; k++) {
		nbob[k] = 0;
	}
	#endif
}

inline void Blobber::qsort(int colour) {
	orderMin(colour, 0, barray[colour].numCandidates() - 1);
}        
         
inline int Blobber::count(int colour) {         
return barray[colour].getCount();
}       
    
inline HalfBeacon & Blobber::getBlob(int i, int j) {     
	return barray[i][j];       
}      
     
inline HalfBeacon & Blobber::getHalfBeacon(int i, int j) {
	return barray[i][j];        
}        
       
inline void Blobber::add(int color, HalfBeacon &hb) {
	barray[color].add(hb);       
}           
            
#endif //BLOB_H

