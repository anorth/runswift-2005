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


/* An interface to the YUV image which performs on the fly image correction:
 * ring correction and linear shift colour correction. Colour classification
 * is also performed (on the fly) here. This is optimised for a sparsely
 * accessed image, so results are not cached.
 */

#ifndef _CORRECTED_IMAGE_H_
#define _CORRECTED_IMAGE_H_

#include <iostream>

#include "../share/VisionDef.h"
#include "ring.h"

// chose zero or one of these three
//#define RING_CORRECTION
//#define LINEAR_CORRECTION
#define RING_AND_LINEAR_CORRECTION

// define this when testing to see an image access map in offline subvision
//#define PROFILE_ACCESS

#ifndef OFFLINE
#undef PROFILE_ACCESS
#endif

typedef enum {
	cBALL = 0,
	cBEACON_BLUE = 1,
	cBEACON_GREEN = 2,
	cBEACON_YELLOW = 3,
	cBEACON_PINK = 4,
	cROBOT_BLUE = 5,
	cROBOT_RED = 6,
	cFIELD_GREEN = 7,
	cROBOT_GREY = 8,
	cWHITE = 9,
	cBLACK = 10,
	cFIELD_LINE = 11,       // marker, not a real colour
	cFIELD_BORDER = 12,     // ditto
	cNUM_COLOR = 13,
	cNONE = 127
}
Color;

static const char* ColourNames[] = {
    "orange",
    "beacon blue",
    "beacon green",
    "beacon yellow",
    "beacon pink",
    "robot blue",
    "robot red",
    "field green",
    "robot grey",
    "white",
    "black",
    "field line",
    "field border"
};

static const Color BACKGROUND = cROBOT_GREY;
static const Color MAX_COLOR = cBLACK;
static const Color LAST_USABLE_COLOR = cROBOT_RED;
static const int NUM_USABLE_COLORS = LAST_USABLE_COLOR+1;

static const int MAXY = 128,
                 MAXU = 128,
                 MAXV = 128;
static const int DCD_SIZE = 256;

static const int COLOR_MASK = 0x0F;
static const int MAYBE_BIT = 0x10;
static const int RES = 0;
static const int FACTOR = 2;

// Linear colour space shift parameters.
struct DogColourDistortion {
	double My;	// coefficient
	double Cy;	// constant
	double Mu;
	double Cu;
	double Mv;
	double Cv;
};

class CorrectedImage
{
    public:
        CorrectedImage();
        ~CorrectedImage() { (void)ColourNames; /* avoid warning */ }

        /* Set a new YUV image as the image data. This gets called each frame */
        static inline void setYUV(const uchar* newyuv) {
            yuv = newyuv;
            yplane = yuv;
            uplane = yplane + CPLANE_WIDTH;
            vplane = uplane + CPLANE_WIDTH;
#ifdef PROFILE_ACCESS
            memset(cplane, 127, CPLANE_WIDTH*CPLANE_HEIGHT);
#endif
        }

        /* Initialise the linear colour distortion table */
        static void initDCD(const DogColourDistortion& dcd);

        /* Returns a corrected y, u or v value for the specified image
         * coordinates.
         */
        static inline uchar y(int x, int y) {
//            if (x < 0 || x >= CPLANE_WIDTH || y < 0 || y >= CPLANE_HEIGHT) {
//                std::cerr << "ERROR: referenced img "
//                    << x << ", " << y << std::endl;
//                return 0;
//            }
#ifdef PROFILE_ACCESS
            if (cplane[y*CPLANE_WIDTH + x] == cNONE) { 
                cplane[y*CPLANE_WIDTH + x] = 0;    
            } else if (cplane[y*CPLANE_WIDTH + x] != cBLACK) {
                cplane[y*CPLANE_WIDTH + x] += 1;
            }            
#endif

#ifdef RING_CORRECTION
            return sliceY[yplane[y*ROW_SKIP + x]][(xLUT[y][abs(x-cyx)])];
#endif
#ifdef LINEAR_CORRECTION
            return dcdY[yplane[y*ROW_SKIP + x]];
#endif
#ifdef RING_AND_LINEAR_CORRECTION
            return dcdY[sliceY[yplane[y*ROW_SKIP + x]][(xLUT[y][abs(x-cyx)])]];
#endif
            return yplane[y*ROW_SKIP + x];
        }
        
        
        static inline uchar u(int x, int y) {
//            if (x < 0 || x >= CPLANE_WIDTH || y < 0 || y >= CPLANE_HEIGHT) {
//                std::cerr << "ERROR: referenced img "
//                    << x << ", " << y << std::endl;
//                return 0;
//            }
#ifdef PROFILE_ACCESS
            if (cplane[y*CPLANE_WIDTH + x] == cNONE) { 
                cplane[y*CPLANE_WIDTH + x] = 0;    
            } else if (cplane[y*CPLANE_WIDTH + x] != cBLACK) {
                cplane[y*CPLANE_WIDTH + x] += 1;
            } 
#endif
            
#ifdef RING_CORRECTION
            return sliceU[uplane[y*ROW_SKIP + x]][(xLUT[y][abs(x-cux)])];
#endif
#ifdef LINEAR_CORRECTION
            return dcdU[uplane[y*ROW_SKIP + x]];
#endif
#ifdef RING_AND_LINEAR_CORRECTION
            return dcdU[sliceU[uplane[y*ROW_SKIP + x]][(xLUT[y][abs(x-cux)])]];
#endif
            return uplane[y*ROW_SKIP + x];
        }


        static inline uchar v(int x, int y) {
//            if (x < 0 || x >= CPLANE_WIDTH || y < 0 || y >= CPLANE_HEIGHT) {
//                std::cerr << "ERROR: referenced img "
//                    << x << ", " << y << std::endl;
//                return 0;
//            }
#ifdef PROFILE_ACCESS
            if (cplane[y*CPLANE_WIDTH + x] == cNONE) { 
                cplane[y*CPLANE_WIDTH + x] = 0;    
            } else if (cplane[y*CPLANE_WIDTH + x] != cBLACK) {
                cplane[y*CPLANE_WIDTH + x] += 1;
            } 
#endif

#ifdef RING_CORRECTION
            return sliceV[vplane[y*ROW_SKIP + x]][(xLUT[y][abs(x-cvx)])];
#endif
#ifdef LINEAR_CORRECTION
            return dcdV[vplane[y*ROW_SKIP + x]];
#endif
#ifdef RING_AND_LINEAR_CORRECTION
            return dcdV[sliceV[vplane[y*ROW_SKIP + x]][(xLUT[y][abs(x-cvx)])]];
#endif
            return vplane[y*ROW_SKIP + x];
        }

        /* Returns the classified colour for the specified image coordinates */
        static inline Color classify(int x, int y) {
            return (Color)(nnmc[(CorrectedImage::y(x,y) / FACTOR) * MAXU * MAXV
                             + (CorrectedImage::u(x,y) / FACTOR) * MAXV
                             + (CorrectedImage::v(x,y) / FACTOR)]
                           & ~MAYBE_BIT); // discard maybe bit
        }

        /* Returns the classified colour for the specified YUV */
        static inline Color classify(uchar y, uchar u, uchar v) {
            return (Color)(nnmc[(y / FACTOR) * MAXU * MAXV
                            + (u / FACTOR) * MAXV
                            + (v / FACTOR)]
                          & ~MAYBE_BIT); // discard maybe bit
        }
        
        /* Returns a tightly classified colour for the specified image
         * coordinates. This is much more likely to return cNONE than the above
         * and gives better assurance that this is indeed the returned colour.
         */
        static inline Color classifyTight(int x, int y) {
            Color col =
                (Color)nnmc[(CorrectedImage::y(x,y) / FACTOR) * MAXU * MAXV
                             + (CorrectedImage::u(x,y) / FACTOR) * MAXV
                             + (CorrectedImage::v(x,y) / FACTOR)];
            return (col & MAYBE_BIT) ? cNONE : col;
        }

        /* Returns the tight classification for the given YUV */
        static inline Color classifyTight(uchar y, uchar u, uchar v) {
            Color col = (Color)nnmc[(y / FACTOR) * MAXU * MAXV
                                    + (u / FACTOR) * MAXV
                                    + (v / FACTOR)];
            return (col & MAYBE_BIT) ? cNONE : col;
        }

        /* Loads the specified calibration file, or the default */
        static void loadColourCalibration(const char *filename = 0);

        /* Calculates the entire CPlane (for debugging colour classification */
		static void fillCPlane(bool tight = false);
        
        /* Sends CPlane to robolink */
        static void sendCPlane(void);

        /* Calculates a sub-sampled CPlane (for debugging) */
        static void fillSubCPlane(void);
        
        /* Sends subsampled CPlane to robolink */
        static void sendSubCPlane(void);

    private:
        friend class FormSubvisionImpl; // allow offline tools private access

        static const int ROW_SKIP = 6 * CPLANE_WIDTH;
        
        // Image data
        static const uchar* yuv;
        static const uchar* yplane, *uplane, *vplane;
		
        // Classified image (debugging only
        static uchar cplane[CPLANE_WIDTH * CPLANE_HEIGHT];

        // lookup table for colour classification
		static unsigned char nnmc[MAXY*MAXU*MAXV];

        // dog colour distortion table
        static uchar dcdY[DCD_SIZE];
        static uchar dcdU[DCD_SIZE];
        static uchar dcdV[DCD_SIZE];
};

#endif // _CORRECTED_IMAGE_H_
