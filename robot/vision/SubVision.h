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


/* Subvision.h
 * Subsampling visual object recognition
 */

#ifndef _SUBVISION_H_
#define _SUBVISION_H_

#include <utility>
#include <vector>

#include "../share/Common.h"
#include "../share/VisionDef.h"
#include "CorrectedImage.h"

enum visualfeature_type {
    VF_NONE = 0,        // used for pruned features
    VF_FIELDLINE,       // a green/white or white/green transition
    VF_BALL,            // a ball edge (x/orange transition)
    VF_OBSTACLE,        // an obstacle or noise
    VF_FIELD,           // pure field green
    VF_PINK,            // beacon pink run of pixels
    VF_BLUE,            // beacon blue run of pixels
    VF_YELLOW,          // beacon yellow run of pixels
    VF_WALL,            // white wall run of pixels
    VF_WALL_OBJECT,     // wall object hacked as visual feature
    VF_UNKNOWN,         // something we can't classify
};

enum visualfeature_insanity {
    VF_SANE = 0,    
    VF_BELOW_SUM_THRESH,
    VF_BELOW_BALL_DU_THRESH,
    VF_DV_DU_OPPOSITE,
    VF_ABOVE_SPECULAR_THRESH,
    VF_NOT_MUCH_BALL_FEATURE,
    VF_LOOKS_LIKE_RED_ROBOT, 
    VF_LOOKS_LIKE_PINK_BEACON,
};


enum {
    // min pixels of green to find in star to keep field feature
    MIN_GREEN_NEAR_FIELD = 16, // out of 9 + 9 + 9 + 9 - 3 = 33
    // distance to search for expected colours. This could be lowered with a
    // relaxed colour classification
    COLOUR_SEARCH_RADIUS = 4,
};

// data bytes to skip between rows in a y/u/v plane
static const int ROW_SKIP = 6 * CPLANE_WIDTH;
    
using std::pair;
using std::vector;

typedef pair<int, int> point;

/* A visual feature is something recognised by SubVision, usually an edge 
 * or colour transition. Features have information about the colours involved
 * and their location in the frame.
 */
class VisualFeature {
    public:
        /* Create a new VisualFeature with coords x, y */
        VisualFeature(visualfeature_type tt = VF_NONE, double xx = 0.0,
                        double yy = 0.0, int ddy = 0, int ddu = 0,
                        int ddv = 0, int dd = 0)
            : x(xx), y(yy), dy(ddy), du(ddu), dv(ddv), dsum(ddy + ddu + ddv),
                dir(dd), type(tt) {scanline = 0;vertical = 0;}

        ~VisualFeature() {}

        /* Members. These are all public for convenience since this class
         * is only for storing related information. WARNING. The order here is
         * important for alignment when sending to robolink. Put like with
         * like.
         */
        double x, y;        // position in image (subpixel)
        double endx, endy;  // position of the end of a run (beacons and goals)
        double dy, du, dv;  // colour gradients
        double dsum;        // sum of colour gradients
        int scanline;       // index of scanline (beacons/goals)
        // direction of scan line (DIR_UP/DOWN/LEFT/RIGHT). In the case of ball
        // and line features it is the direction of the ball/line.
        int dir;            
        visualfeature_type type;
        int vertical;       // vertical distance from the bottom the scanline
        //int padding;        // for alignment
};


/* An image point fo caching associated colour and classification information.*/
struct classifiedPoint { 
    int x; 
    int y; 
    uchar yy; 
    uchar uu; 
    uchar vv; 
    Color cc;
    
    classifiedPoint(int ax, int ay) { 
        x = ax;
        y = ay;
    }
    
    void set(int ax, int ay, uchar ayy, uchar auu, uchar avv, Color acc) { 
        x = ax; 
        y = ay; 
        yy = ayy; 
        uu = auu; 
        vv = avv; 
        cc = acc;
    }
};

/* SubVision is the image sub-sampling visual engine. It attempts to 
 * recognise "features" in the image by sampling a small collection of
 * pixels, never the whole image. Features are usually "edges" - boundaries
 * between two colours. These features can be used to recognise objects
 */
class SubVision {


    public:
        SubVision(CorrectedImage& i) : img(i) {}
        ~SubVision() {}
        int sampleImage(double hIntercept, double horizonGradient,
                        bool upsideDown, bool horizonExists,
                        bool horizonBelowImage, bool isGrabbed = false, 
                        bool isGrabbing = false);
        int sampleImageExtra();
        
        const point getHorizonPoint() { return this->horizonPoint; }
        const point getOtherHorizonPoint() { return this->otherHorizonPoint; }
        const double getHorizonGradient() { return this->horizonGradient; }
        const bool getHorizonUpsideDown() { return this->horizonUpsideDown; }
        vector<VisualFeature>& getFeatures() { return features; }
        vector<VisualFeature>& getObstacleFeatures() { return obstacleFeatures; }
        int getFieldFeatureCount() { return fieldFeatureCount; } 
        bool getIsGrabbed() { return isGrabbed; }
        void sendFeatures(void);
        point searchLine(int x, int y, Color colour, bool up,
                            bool expectColour);

        /* Counts the number of pixels of colour surrounding the specified
         * pixel.  This an 8-point asterisk for efficiency.
         */ 
        inline int nearColour(int x, int y, Color colour, int radius) {
            int colourCount = 0;
            
            char ops[] = {1,1,  // top right diagonal (x-offset,y-offset)
                          1,-1, // bottom right diagonal
                          -1,1, // top left diagonal
                          -1,-1,// bottom left diagonal
                          0,1,  // top vertical
                          0,-1, // bottom vertical
                          1,0,  // right horizontal
                          -1,0};// left horizontal
            for (char i = 0; i < radius; i++) {
                for (char j = 0; j < 8; j++) {
                    int newX = x + ops[2*j]*i; // Multiply radius with x-offset.
                                               // Add to the original x.
                    if (newX < 0 || newX >= CPLANE_WIDTH) { 
                        continue;
                    }
                    int newY = y + ops[2*j+1]*i; // Multiply radius and y-offset
                                                 // Add to the orignal y.  
                    if (newY < 0 || newY >= CPLANE_HEIGHT) { 
                        continue;
                    }
                    if (img.classify(newX,newY) == colour) { 
                        ++colourCount;
                    }   
                }
            }
            
        //    cout << "nearColour " << x << "," << y << " c " << colour << " = "
        //        << colourCount << endl;
            return colourCount;
        }

        /* Returns true if this pixel is surrounded by green */
        inline bool isFieldGreen(const VisualFeature& vf) {
            return nearColour((int)vf.x, (int)vf.y, cFIELD_GREEN,
                                COLOUR_SEARCH_RADIUS) > MIN_GREEN_NEAR_FIELD;
        }


        /* Counts the number of pixels of the specified colour in the specified
         * direction from (x, y), up to length.  FIXME: relative to the
         * horizon?
         */
        inline int dirColour(int x, int y, int dir, Color colour, int length) {
            int i, count = 0;
            if (dir == DIR_UP) {
                length = MIN(length, y + 1);
                for (i = 0; i < length; ++i)
                    if (img.classify(x,y-i) == colour) 
                        ++count;
            } else if (dir == DIR_DOWN) {
                length = MIN(CPLANE_HEIGHT - y, length);
                for (i = 0; i < length; ++i)
                    if (img.classify(x,y+i) == colour)
                        ++count;
            } else if (dir == DIR_LEFT) {
                length = MIN(length, x + 1);
                for (i = 0; i < length; ++i)
                    if (img.classify(x-i,y) == colour)
                        ++count;
            } else if (dir == DIR_RIGHT) {
                length = MIN(CPLANE_WIDTH - x, length);
                for (i = 0; i < length; ++i)
                    if (img.classify(x+i,y) == colour)
                        ++count;
            }
            return count;
        }


    private:
        friend class FormSubvisionImpl; // allow the offline tools priv access

        void calculateOtherHorizonPoint(void);
        void createScanLines(void);
        void makeMoreScanlines(int x, int y, int dir);
        point makeScanline(point hIntercept, int length = INT_MAX);
        pair<point, point> makeHorzScanline(point centrePoint);
        void processScanline(point top, point bottom, int dir, int index,
                            bool lookForGreen = false,
                            bool beaconsAndGoals = false);
        VisualFeature testPixel(point p, uchar yy, uchar uu, uchar vv,
                                Color cc, int dir, bool beaconsAndGoals, 
                                int scanline, bool initialiseOnly, int& insanity);
        void testBallColourTransition(point p, uchar yy, uchar uu, uchar vv,
                                      Color cc, int dir,
                                      int scanline, bool initialiseOnly);
        void testWallColourTransition(point p, uchar yy, uchar uu, uchar vv, 
                                      Color cc, int dir,
                                      int scanline, bool initialiseOnly);
        
        /* The image data */
        CorrectedImage& img;

        // horizon
        point horizonPoint, otherHorizonPoint;
        double horizonGradient;
        bool horizonUpsideDown;
        bool isGrabbed;
        bool isGrabbing;
        
        // scan lines, pairs of two (x,y) points, the first of each pair being
        // on the horizon
        vector<pair<point, point> > scanlines;
        vector<pair<point, point> > leftScanlines;
        
        // scan lines parallel with the horizon
        vector<pair<point, point> > horzScanlines;
        // detected features
        vector<VisualFeature> features;
        //vector<VisualFeature> transitionFeatures;
        vector<VisualFeature> obstacleFeatures;
        
        // Field Green feature counts
        int fieldFeatureCount; 
};

/* Given a straight line in point-gradient form and a y value, returns the
 * x value of that intercept, rounded inwards where necessary.
 */
template<typename T>
inline T solveForY(const pair<T, T>& p, double gradient, T yIntercept) {
    return static_cast<T>((yIntercept - p.second) / gradient + p.first);
}

/* Given a straight line in point-gradient form and an x value, returns the
 * y value of that intercept, rounded inwards where necessary.
 */
template<typename T>
inline T solveForX(const pair<T, T>& p, double gradient, T xIntercept) {
    return static_cast<T>(gradient * (xIntercept - p.first) + p.second);
}

/* Returns true if the specified points differ by no more than 1.5 in both
 * x and y coordinates.
 */
inline bool adjacent(const VisualFeature& v1, const VisualFeature& v2) {
    return fabs(v1.x - v2.x) <= 1.5 && fabs(v1.y - v2.y) <= 1.5;
}

/* Returns the direct distance between two points */
inline double length(const point& p, const point& q) {
    return DISTANCE((double)p.first, (double)p.second,
                    (double)q.first, (double)q.second);
}

/* Rotates one point around another by the specified angle (radians). A positive
 * angle gives anti-clockwise rotation in the standard cartesian plane.
 */
inline pair<double, double> rotatePoint(double px, double py,
                                        double cx, double cy, double angle) {
    pair<double, double> result;
    double dx = px - cx;
    double dy = py - cy;
    result.first = cx + cos(angle) * dx - sin(angle) * dy;
    result.second = cy + sin(angle) * dx + cos(angle) * dy;
    return result; // needs optimisation
}

inline point rotatePoint(int px, int py, int cx, int cy, double angle) {
    pair<double, double> result;
    result = rotatePoint((double)px, (double)py, (double)cx, (double)cy, angle);
    return point((int)result.first, (int)result.second);
}

/* Returns the shortest distance between the specified point p and the given
 * line (in point-gradient form given by l and gradient). Note that the return
 * is signed - you might want to use abs()
 */
// untested
//inline double pointLineDistance(double px, double py, double lx, double ly
//                                  double gradient) {
//    return (py - gradient*px - gradient*lx - ly)/(sqrt(SQUARE(gradient) + 1));
//}

/* Line tracing is used in a number of places so it's implemented here
 * so we get the same lines everywhere.
 */
namespace LineDraw
{
    /* Initialises the start and end points for the next line to trace. */
    inline void startLine(point start, point finish) {
        extern int x, y, dx, dy, b;
        extern double m;
        extern bool shallow;
        extern point endpoint;        
//        extern point startpoint;

//        std::cerr << "startLine: " << start.first << "," << start.second
//            << "  " << finish.first << "," << finish.second << std::endl;
//        startpoint = start;
        endpoint = finish;
        dx = finish.first - start.first;
        dy = finish.second - start.second;

        if (abs(dx) > abs(dy)) { // slope < 1
            shallow = true;
            m = double(dy) / dx;
            b = int(start.second - m*start.first);
            if (dx < 0) {
                dx = -1;
            } else {
                dx = 1;
            }
            x = start.first;
        } else if (dy != 0) {
            shallow = false;
            m = double(dx) / dy;
            b = int(start.first - m*start.second);
            if (dy < 0) {
                dy = -1;
            } else {
                dy = 1;
            }
            y = start.second;
        }
    }

    /* Returns the next point in the line trace from start to finish. The first 
     * point returned is the one after start. The last point returned will equal
     * finish, after which call to nextPoint will become undefined until
     * startLine is called again.
     */
    inline point nextPoint(void) {
        extern int x, y, dx, dy, b;
        extern double m;
        extern bool shallow;
        //extern point startpoint;
        extern point endpoint;
              
        if (shallow) {
            if (x == endpoint.first) 
               return endpoint;
            
            // Trying to scan feature points every point offset.
            //if ((x >= endpoint.first && x > startpoint.first)
            //   || (x <= endpoint.first && x < startpoint.first)) {
            //    return endpoint;
            //}
            x += dx;
            int yy = int(m*x + b);
            //std::cerr << "nextPoint: " << x << ", " << yy << std::endl;
            return point(x, yy);
        } else {
            if (y == endpoint.second) 
                return endpoint;
            
            // Trying to scan feature points every point offset.
            //if ((y >= endpoint.second && y > startpoint.second)
            //    || (y <= endpoint.second && y < startpoint.second)) {
            //    return endpoint;
            //}
            y += dy;
            int xx = int(m*y + b);
            //std::cerr << "nextPoint: " << xx << ", " << y << std::endl;
            return point(xx, y);
        }
    }

} // namespace LineDraw

#endif // _SUBVISION_H_
