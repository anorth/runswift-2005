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


#ifndef VisualObject_h_DEFINED
#define VisualObject_h_DEFINED

#include <iostream>
#include "../share/Common.h"

class VisualObject {
	public:

		int cf;                 // from 0 to 1000 (0 => not seen)
        unsigned nFeatures;     // number of features forming this object
	double cx, cy;          // centroid
	double x, y, height, width;  // actual object bounding box
//        double gapLeft, gapRight;    // x co-ords of best gap for goals
        double leftGapMin, leftGapMax,
                rightGapMin, rightGapMax;
        double radius;
		double imgHead;         // Heading and elevation relative to the centre
		double imgElev;         // of the camera image
                int imgDir;             // Direction of the ball from the centre of the image.
                int pad;                // for 64-bit alignment

		double head;            // Robot relative heading
		double elev;            // Robot relative elevation, positive DOWN
		double dist3;           // From the neck to edge of the vob [cm]
		double dist2;           // Two Dimensional Distance from to neck to 
                                // vob edge (parallel to the plane of the field)
		double objectPosition[3];   // positiion in three dimensional space
                                    // relative to the neck. (x, y, z) where x
                                    // is sideways, y is up, and z is away
                                    // from the robot
		double var;             // variance of object determined from equation.
		double angleVar;        // variance of the angle of the object
//		HalfBeacon *blob;
        

//		int area;        // area of blob(s)
//		double lhead;    // Heading rotation of vob (XZ) [degrees]
//		double rhead;    // Heading rotation of vob (XZ) [degrees]
//		double h;
        // Only used by robots (NOTE: goal cx/cy/x/y/h/w is actually
        // rotated like this as well).
//		double xRmin, xRmax, yRmin, yRmax;  // bounding box drawn using rotated blobs. 
//		double cRx, cRy;                    // its centroid

		// Added by Ted Wong. These variables are unmodified version
		// from the blob algorithm. "o" stands for original.
//		double ox, oy, oheight, owidth, ocx, ocy;

//		double projected_x;  // projection of bounding box onto image due to occlusion, or limited view
//		double projected_y;
//		double projected_h;
//		double projected_w;
//		double c2c; 
//		double pinkXsum;
//		double pinkYsum;
//		double pinkarea;
//		double closestXsum;
//		double closestYsum;
//		double closestarea;

		/* Distance from the camera to the centroid of the vob in cm.
		 * It is a manipulated value to give accurate distances. It is unsuitable for ball tracking
		 **/
//		double cam_centroid_dist;   

//		double misc;                // beacon: c2c, ball: raw y co-ordinate of blob
//		double f;                   // the absolute direction the visual object is facing in world model.
//		double orient;              // orientation of robots seen 0 - facing away chest, 
                                    // 180 - facing towards chest, -90 left, 90 right

		VisualObject() : var(VERY_LARGE_DOUBLE), angleVar(VERY_LARGE_DOUBLE)
		{
            // var must be set, else division by 0 occurs
			reset();
		}               

        inline void invalidate() { 
            cf = 0; 
        }
        

        /* Resets the confidence/variances so as to "forget" this vob */
        inline void reset() {
            cf = 0;
            var = angleVar = VERY_LARGE_DOUBLE;
            cx = cy = x = y = height = width = radius /*= gapLeft = gapRight*/ = 0.0;
            leftGapMin = leftGapMax = rightGapMin = rightGapMax = 0.0;
            dist3 = dist2 = 0.0;
            imgHead = imgElev = elev = head = 0.0;
            objectPosition[0] = objectPosition[1] = objectPosition[2] = 0.0;
        }

        /* Sets the x, y, z object position to the specified values */
        inline void setPosition(double inx, double iny, double inz) {
            objectPosition[0] = inx;
            objectPosition[1] = iny;
            objectPosition[2] = inz;
        }

        /*
         * Sets dist2, dist3, elev and heading from the cartesian coordinates
         * in objectPosition coordinates (relative to the base of the neck).
         **/
        inline void setPolar(void) {
            dist3 = sqrt(SQUARE(objectPosition[0]) + SQUARE(objectPosition[1])
                        + SQUARE(objectPosition[2]));
            //dist2 = sqrt(SQUARE(objectPosition[0]) + SQUARE(objectPosition[2]));
            head = RAD2DEG(atan2(objectPosition[0], objectPosition[2]));

            double elev_r = acos(sqrt(SQUARE(objectPosition[0])
                                    + SQUARE(objectPosition[2])) / dist3);
            elev = RAD2DEG(elev_r) * ((objectPosition[1] >= 0) ? 1 : -1) ;
        }

		/*
         * @return 1 dimensional distance from robot's neck, on a plane
         * parallel to the field and perpendicular to the body's chest.  (the z
         * axis)
		 **/
		inline double getPerpDistance() const {
			return objectPosition[2];
		}

		/*
         * @return 1 dimensional distance from robot's neck on a plane parallel
         * to the field and parallel to the body's chest.  (the x axis)
		 **/
		inline double getLateralOffset() const {
			return objectPosition[0];
		}

		/*
         * @return 1 dimensional distance from robot's neck on a plane
         * perpendicular to the field plane.  (the y axis)
         **/
		inline double getVerticalOffset() const {
			return objectPosition[1];
		}

		void Print() {
            if (!cf) {
                std::cout << "\tVOB is invalid" << std::endl;
            } else {
                std::cout << "\tConfidence: " << cf << std::endl;
                std::cout << "\tCentroid:   (" << cx << "," << cy << ")"
                                        << std::endl;
                std::cout << "\tImage Head: " << imgHead << std::endl;
                std::cout << "\tImage Elev: " << imgElev << std::endl;
                std::cout << "\tHeading:    " << head << std::endl;
                std::cout << "\tElevation:  " << elev << std::endl;
                std::cout << "\tDistance:   " << dist3 << std::endl;
                std::cout << "\tBBox:       (" << x << "," << y << ")-("
                                        << x + width << ","
                                        << y + height << ")" << std::endl;
                std::cout << "\theight:     " << height << std::endl;
                std::cout << "\twidth:     " << width << std::endl;
            }
        }

};

#endif //VisualObject_h_DEFINED
