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



/* Subvision.cc
 * Subsampling visual object recognition
 *
 * Note: In all cases, the origin is the top left corner of the image,
 * so "down" is usually positive y
 */

#include <iostream>

#include "SubVision.h"
#include "Profile.h"

#ifndef OFFLINE
#include "Robolink.h"
#endif

//#define TESTPIXEL_DEBUG

#define MAX_NO_FEATURES_TO_SEND 300

//#define DISABLE_BALL_COLOUR_TRANSITION
//#define DISABLE_BALL_EDGE_DETECTION

using namespace std;

// constants
enum {
    // pixels above edge to put fake horizon when it would otherwise be
    // off the bottom
    LOW_HORIZON_LIMIT = 1,
    // pixels from the edge of the image that are too distorted to use
    IMAGE_EDGE = 3,
    // pixels horizontal space between full scan lines (power of 2)
    FULL_SCANLINE_SPACING = 16,
    // max length of the half-size scanlines
    HALF_SCANLINE_LENGTH = 4 * FULL_SCANLINE_SPACING,
    // max length of the quarter-size scanlines
    QTR_SCANLINE_LENGTH = 3 * FULL_SCANLINE_SPACING,
    // minimum scan line length (pixels)
    MIN_SCANLINE = 3,
    // max distance above horizon to put scanlines. We ignore goals 
    // much lower down in sanity checks anyway.
    MAX_HORZ_SCANLINE_GAP = 50, // gives 10 scanlines above horizon
    // Max number of horizontal scanlines (when grabbed)
    MAX_HORZ_SCANLINES = 30,
    // min pixels of expected colour to find in an 8 point star of radius
    // COLOUR_SEARCH_RADIUS to keep feature
    MIN_COLOUR_NEAR_FEATURE = 5, 
    // distance to search for orange inwards from a ball feature
    BALL_COLOUR_SEARCH = 5,
    // min pixels of orange in BALL_COLOUR_SEARCH to classify as ballfeature
    MIN_BALL_COLOUR = 3,
    // max pixels of non-orange in BALL_COLOUR_SEARCH to classify as ballfeature
    MAX_NONBALL_COLOUR = 1,
    // minimum number of ball features we want, else we'll try to find more
    MIN_BALLFEATURES = 7, // = SubObject.MIN_RADII_AVERAGE
    // minimum number of beacon features we want, else we'll try to find more
    //MIN_BEACONFEATURES = 3,
    // length of additional scanlines (length will actually be 1 longer)
    LEN_MORE_SCANLINES = 16, // 18
    // distance of additional scanlines from initial point of interest
    OFFSET_MORE_SCANLINES = FULL_SCANLINE_SPACING/8,
    // max number of obstacle features to find in a scanline
    MAX_OBSTACLE_LINE = 5,
    // max number of line edges to accept in a scanline
    MAX_LINE_FEATURES = 6,
    // number of (not consec) orange pix required for testBallColourTransition
    MIN_ORANGE_TRANSITION = 6,
    // min number of consecutive orange pixels to count as orange found
    // in testBallColourTransition
    MIN_CONSEC_ORANGE_TRANSITION = 3,
    // number of non-orange pix required to reset testBallColourTransition
    MIN_NOTORANGE_TRANSITION = 3,
    // number of maximum allowable white
    MAX_WHITE_COUNT = 3,
    
    MAX_GREEN_ABOVE_OBS = 10,
    TH_GREEN_ABOVE_OBS = 1, 
    
};

// visual feature thresholds and constants
// FIXME: many of these (e.f. TH_FIELDLINE_DY) are dependent on head speed
// and/or distance from robot
enum {
    TH_ANYTHING = 20,       // minimum gradient sum to be at all interesting
    TH_FIELDLINE_DY = 15,   // minimum Y gradient for green/white edge
    TH_FIELDLINE_DUV = 40,  // maximum U/Y gradient for green/white edge
    TH_BALL_DU = 15,        // was 20, min du on orange edge
    TH_BALL_DV = 35,        // max dv on orange edge
    TH_BALL_DUDV = 4,       // max fraction of similar du/dv for ball edge
    
    TH_WALL_HEIGHT = 5,     // min side wall height
    
    TH_OBSTACLE_Y = 35,     // min y of non-obstacle/shadow
    TH_SPECULAR_Y = 180,    // y above this on objects is probably specular
    TH_FUZZY = 4,           // a small delta allowing for noise
#ifdef LOCALISATION_CHALLENGE
    TH_BEACON_PINK = 3,     // min length of pink run for beacon
#else
    TH_BEACON_PINK = 5,     // min length of pink run for beacon
#endif    
    TH_GOAL_BLUE = 12,      // ditto blue for goal
    TH_GOAL_YELLOW = 12,    // ditto yellow for goal
    TH_BEACON_NOTPINK = 1,  // max length not pink in a beacon
    TH_GOAL_NOTBLUE = 1,    // ditto blue
    TH_GOAL_NOTYELLOW = 1,  // ditto yellow
};


/* Static vars for line drawing */
namespace LineDraw {
    int x, y, dx, dy, b;
    double m;
    bool shallow;
    point endpoint;
    point startpoint;
    int POINT_OFFSET = 1;
}

/* Sample the image YUV planes and detect features. The Y, U and V planes
 * are as extracted by VisualCortex. The horizon estimate is given in 
 * point-gradient form. The horizon is that as calculated by VisualCortex
 * with hIntercept the value at the intercept with the y
 * axis, with the origin in the centre of the image and +ve y up (this will
 * be translated to the normal image coordinates).
 * If the horizon lies above or below the image hExists
 * should be false and hBelowImage reflect whether the horizon lies above or
 * below frame. upsideDown should be true if up in the 
 * image is down in the real world. 
 */
int SubVision::sampleImage(double hIntercept, double hGradient,
                            bool hUpsideDown, bool hExists, bool hBelowImage,
                            bool is_grabbed, bool is_grabbing) {

    if (hExists) {
        this->horizonUpsideDown = hUpsideDown;
        this->horizonPoint.first = 0;
        this->horizonPoint.second =
            (int)(-hGradient * (0 - CPLANE_WIDTH/2)
            + (CPLANE_HEIGHT/2 - hIntercept));
        this->horizonGradient = -hGradient;
    } else { // no horizon in view - put in a dummy
        if (hBelowImage) {
            this->horizonPoint = point(0, CPLANE_HEIGHT);
            this->horizonGradient = 0;
            this->horizonUpsideDown = false;
        } else {
            this->horizonPoint = point(0, -1);
            this->horizonGradient = 0;
            this->horizonUpsideDown = false;
        }
    }

    this->isGrabbed = is_grabbed;
    this->isGrabbing = is_grabbing;

    this->scanlines.clear();
    this->leftScanlines.clear(); 
    this->horzScanlines.clear();
    this->features.clear();
//    this->transitionFeatures.clear();
    this->obstacleFeatures.clear();
    this->fieldFeatureCount = 0;

    calculateOtherHorizonPoint();

//    cout << "SubVision::sampleImage horizon from " << horizonPoint.first
//        << "," << horizonPoint.second << " to " << otherHorizonPoint.first
//        << "," << otherHorizonPoint.second << ". Gradient: "
//        << horizonGradient << ", upside down: " << horizonUpsideDown << endl;

    createScanLines();
    
    // Now for each scan line, run along it and find interesting points
//    cout << "SubVision processing scan lines" << endl;
    Profile::start(PR_SV_VERT);
    vector<pair<point, point> >::iterator itr = scanlines.begin();
    int id = 0; // scan line index
    bool findGreen = true;
    while (itr != scanlines.end()) {
        if (length(itr->first, itr->second) > HALF_SCANLINE_LENGTH + 1
                && (findGreen = !findGreen)) // green on every second
            processScanline(itr->first, itr->second, DIR_UP, id, true);
        else
            processScanline(itr->first, itr->second, DIR_UP, id, false);
        ++itr; // next line
        ++id;
    }
    
    itr = leftScanlines.begin();
    while (itr != leftScanlines.end()) {
        if (length(itr->first, itr->second) > HALF_SCANLINE_LENGTH + 1
                && (findGreen = !findGreen))
            processScanline(itr->first, itr->second, DIR_RIGHT, id, true); 
        else 
            processScanline(itr->first, itr->second, DIR_RIGHT, id, false);
        ++itr; 
        ++id;
    }
    
    Profile::stop(PR_SV_VERT);

    Profile::start(PR_SV_HORZ);
    itr = horzScanlines.begin();
    while (itr != horzScanlines.end()) {
        processScanline(itr->first, itr->second, DIR_RIGHT, id, false, true);
        ++itr; // next line
        ++id;
    }
    
        
    Profile::stop(PR_SV_HORZ);

//    features.insert(features.end(), transitionFeatures.begin(),
//                    transitionFeatures.end());
       
    return features.size();
}


int SubVision::sampleImageExtra() {

    // if only very few ball features were found, run some more short 
    // scanlines in their vicinity
    Profile::start(PR_SV_EXTRABALL);
    int nBallFeatures = 0;
    unsigned int i;
    for (i = 0; i < features.size(); ++i) {
        if (features[i].type == VF_BALL)
            ++nBallFeatures;
    }
    
    unsigned int startIndex = 0;
    unsigned int featureIndex = 0;
    while (nBallFeatures < MIN_BALLFEATURES) {  
        startIndex = featureIndex;
        featureIndex = features.size();      
        for (i = startIndex; i < featureIndex; ++i) {
            if (features[i].type == VF_BALL) {
                makeMoreScanlines((int)features[i].x, (int)features[i].y,
                                    DIR_UP);
                makeMoreScanlines((int)features[i].x, (int)features[i].y,
                                    DIR_RIGHT);
            }
        }
        
        int nNewBallFeatures = 0;
        unsigned int newFeatureIndex = features.size();
        for (i = featureIndex; i < newFeatureIndex; ++i) { 
            if (features[i].type == VF_BALL) { 
                ++nNewBallFeatures;
            } 
        }
        if (nNewBallFeatures == 0) { 
            break;
        }
        nBallFeatures += nNewBallFeatures;
    }    
    
    Profile::stop(PR_SV_EXTRABALL);

    // if only very few beacon features were found, run some more 
    // horizontal scanlines in their vicinity
    // FIXME
    
    return features.size();
}

/* Sends the visual features recognised this frame to robolink */
void SubVision::sendFeatures(void) {
#ifndef OFFLINE
/*
   cerr << "SubVision::sendFeatures " << this->features.size()
        << " features = " << (this->features.size() * sizeof(VisualFeature))
        << " bytes at " << sizeof(VisualFeature) << endl;
*/        
    static VisualFeature f[MAX_NO_FEATURES_TO_SEND]; // FIXME
    unsigned i, j;
    for (i = 0; i < this->features.size() && i < MAX_NO_FEATURES_TO_SEND; ++i) {
        f[i] = this->features[i];
    }
    for (j = 0; j < obstacleFeatures.size() && i+j < MAX_NO_FEATURES_TO_SEND ; ++j) {
        f[i+j] = this->obstacleFeatures[j];
    }
    
    if (i + j > 0) {
//        cerr << "SubVision sending to Robolink" << endl;
        Robolink::sendToBase((byte*)f, RLNK_SUBVISION, (i+j)*sizeof(VisualFeature));
//        cerr << "SubVision features sent" << endl;
    }
#endif
}

/* Calculates the point where the horizon intercepts the image border
 * given the leftmost horizon intercept and gradient.
 */
void SubVision::calculateOtherHorizonPoint(void) {
    // Check for horizon outside the image. If it's above the image we'll
    // put a horizon across the top (still looking for ball and lines.
    // If it's below we'll put one really low down (just looking for beacons)
    // Of course there are degenerate cases where these will miss objects
    //
    // We have one horizon point (maybe outside the image), calculate the other.
    // Solve for x = right border. If that's within the image, that's our
    // point.  y = m*(x - x1) + y1
    otherHorizonPoint.first = CPLANE_WIDTH - 1;
    otherHorizonPoint.second = solveForX(horizonPoint, horizonGradient,
                                            CPLANE_WIDTH - 1);
    if (horizonPoint.second >= CPLANE_HEIGHT
            && otherHorizonPoint.second >= CPLANE_HEIGHT) {
        // BELOW the image (inverse y, remember?)
        // looking up high we're unlikely to see anything much, so just run
        // a horizon close to the bottom edge
//        cout << "SubVision: horizon below image" << endl;
        horizonPoint = point(0, CPLANE_HEIGHT - 1 - LOW_HORIZON_LIMIT);
        otherHorizonPoint = point(CPLANE_WIDTH - 1,
                                CPLANE_HEIGHT - 1 - LOW_HORIZON_LIMIT);
        horizonGradient = 0;
    } else if (horizonPoint.second < 0 && otherHorizonPoint.second < 0) {
        // ABOVE the image
        // looking down like this everything will be really close and huge
        // so the orientation doesn't matter too much
//        cout << "SubVision: horizon above image" << endl;
        horizonPoint = point(0, 0);
        otherHorizonPoint = point(CPLANE_WIDTH - 1, 0);
        horizonGradient = 0;
    } else {
        // The horizon lies at least partially in the image. Either one or the
        // other horizon intercepts are within the image, or one is above and
        // one below. Clip each point to an image border.
        if (horizonPoint.second < 0) {
            // intercepts the top edge; solve for y = 0
//            cout << "SubVision: horizon left intercepts top" << endl;
            horizonPoint.second = 0;
            horizonPoint.first = solveForY(otherHorizonPoint,
                                            horizonGradient, 0);
        } else if (horizonPoint.second >= CPLANE_HEIGHT) {
            // intercepts the bottom edge
//            cout << "SubVision: horizon left intercepts bottom" << endl;
            horizonPoint.second = CPLANE_HEIGHT - 1;
            horizonPoint.first = solveForY(otherHorizonPoint, horizonGradient,
                                                CPLANE_HEIGHT - 1);
        }

        if (otherHorizonPoint.second < 0) {
            // intercepts the top edge; solve for y = 0
//            cout << "SubVision: horizon right intercepts top" << endl;
            otherHorizonPoint.second = 0;
            otherHorizonPoint.first = solveForY(horizonPoint,
                                                horizonGradient, 0);
        } else if (otherHorizonPoint.second >= CPLANE_HEIGHT) {
            // intercepts the bottom edge
//            cout << "SubVision: horizon right intercepts bottom" << endl;
            otherHorizonPoint.second = CPLANE_HEIGHT - 1;
            otherHorizonPoint.first = solveForY(horizonPoint, horizonGradient,
                                                CPLANE_HEIGHT - 1);
        }
    } 

    // Check horizon has length at least 1, otherwise call it outside the image
//    cout << "horizon at " << horizonPoint.first << "," << horizonPoint.second
//        << " to " << otherHorizonPoint.first << "," << otherHorizonPoint.second
//        << endl;
    if (horizonPoint == otherHorizonPoint) {
        if (horizonPoint.second >= CPLANE_HEIGHT) {
//            cout << "SubVision: horizon below image" << endl;
            horizonPoint = point(0, CPLANE_HEIGHT - 1 - LOW_HORIZON_LIMIT);
            otherHorizonPoint = point(CPLANE_WIDTH - 1,
                                    CPLANE_HEIGHT - 1 - LOW_HORIZON_LIMIT);
            horizonGradient = 0;
        } else {
//            cout << "SubVision: horizon above image" << endl;
            horizonPoint = point(0, 0);
            otherHorizonPoint = point(CPLANE_WIDTH - 1, 0);
            horizonGradient = 0;
        }
    }
}

/* Constructs scan lines over the image based on the current horizon estimate */
void SubVision::createScanLines(void) {
    // Drop a scan line from the centre of the horizon
//    cout << "SubVision calculating scan lines" << endl;
    point horizonCentre;
    if (fabs(horizonGradient) < 1) {
        horizonCentre.first = horizonPoint.first +
                    (otherHorizonPoint.first - horizonPoint.first) / 2;
        horizonCentre.second = solveForX(horizonPoint, horizonGradient,
                                        horizonCentre.first);
    } else {
        horizonCentre.second = horizonPoint.second +
                    (otherHorizonPoint.second - horizonPoint.second) / 2;
        horizonCentre.first = solveForY(horizonPoint, horizonGradient,
                                        horizonCentre.second);
    }
    pair<int, int> scanIntercept = makeScanline(horizonCentre);
    if (length(horizonCentre, scanIntercept) > 2)
        scanlines.push_back(make_pair(horizonCentre, scanIntercept));
    
    // Drop some more scan lines progressively out from the centre,
    // first moving out to the left, then when we've gone too far
    // go back and move out to the right
    int gap;
    int direction = -1; // -1 for left, 1 for right
    bool stop = false; // should we stop dropping in this dir
    for (gap = FULL_SCANLINE_SPACING/4; ; gap += FULL_SCANLINE_SPACING/4) {
        point horizonIntercept;
        if (fabs(horizonGradient) < 1) {
            horizonIntercept.first = horizonCentre.first + direction*gap;
            horizonIntercept.second = solveForX(horizonPoint, horizonGradient,
                                                horizonIntercept.first);
        } else {
            horizonIntercept.second = horizonCentre.second + direction*gap;
            horizonIntercept.first = solveForY(horizonPoint, horizonGradient,
                                                horizonIntercept.second);
        }
        if (gap % FULL_SCANLINE_SPACING == 0)
            scanIntercept = makeScanline(horizonIntercept);
        else if (gap % (FULL_SCANLINE_SPACING/2) == 0)
            scanIntercept = makeScanline(horizonIntercept,HALF_SCANLINE_LENGTH);
        else
            scanIntercept = makeScanline(horizonIntercept,QTR_SCANLINE_LENGTH);

        // if the line's upside down and shouldn't be, stop
        if (! horizonUpsideDown) {
            if (scanIntercept.second < horizonIntercept.second)
                stop = true;
        } else {
            if (horizonIntercept.second < scanIntercept.second)
                stop = true;
        }
        //if (stop)
            //cout << "stopping: scanline upside down" << endl;
        // if the line is too short, stop
        if (length(horizonIntercept, scanIntercept) < MIN_SCANLINE) {
            //cout << "stopping: scanline too short" << endl;
            stop = true;
        }   
        // if both points of a line are on the border or off screen, ignore it.
        if ((horizonIntercept == scanIntercept
                || (horizonIntercept.first < IMAGE_EDGE
                || horizonIntercept.first > CPLANE_WIDTH - 1 - IMAGE_EDGE
                || horizonIntercept.second < IMAGE_EDGE
                || horizonIntercept.second > CPLANE_HEIGHT - 1 - IMAGE_EDGE))
                && (scanIntercept.first < IMAGE_EDGE
                    || scanIntercept.first > CPLANE_WIDTH - 1 - IMAGE_EDGE
                    || scanIntercept.second < IMAGE_EDGE
                    || scanIntercept.second > CPLANE_HEIGHT - 1 - IMAGE_EDGE)) {
            // if it's a long line, stop
           if (gap % FULL_SCANLINE_SPACING == 0) {
                //cout << "stopping: scanline off image" << endl;
                stop = true;
           } else {
               continue;
           }
        }
        if (stop) {
            if (direction == -1) { // back to centre and change direction
                gap = 0;
                direction = 1;
                stop = false;
                continue;
            } else {
                break; // finished
            }
        }
        if (length(horizonIntercept, scanIntercept) > 2)
            scanlines.push_back(make_pair(horizonIntercept, scanIntercept));
    }

    // Create some sparse horizontal scan lines parallel to the horizon below it
    // Note that despite being horizontal these lines are not in the
    // horzScanLines list and are not used for beacon/goal detection; they are
    // for picking up extra ball points and lines that run away from the robot.
    gap = FULL_SCANLINE_SPACING;
    while (true) {
        point centrePoint = horizonCentre; // centre of scanline
        if (fabs(horizonGradient) < 1) {
            centrePoint.second = centrePoint.second + gap;
            centrePoint.first = solveForY(horizonCentre, -1/horizonGradient,
                                            centrePoint.second);
            if (centrePoint.second > CPLANE_HEIGHT - 1)
                break;
        } else if ((horizonGradient < 0 && ! horizonUpsideDown)
                || (horizonGradient > 0 && horizonUpsideDown)) {
            centrePoint.first = centrePoint.first + gap;
            centrePoint.second = solveForX(horizonCentre, -1/horizonGradient,
                                            centrePoint.first);
            if (centrePoint.first > CPLANE_WIDTH - 1)
                break;
        } else {
            centrePoint.first = centrePoint.first - gap;
            centrePoint.second = solveForX(horizonCentre, -1/horizonGradient,
                                            centrePoint.first);
            if (centrePoint.first < 0)
                break;

        }
//        cout << "horzScanLine centred at " << centrePoint.first << ","
//            << centrePoint.second << endl;
        pair<point, point> line = makeHorzScanline(centrePoint);
        if (length(line.first, line.second) > 2)
            //tempScanlines.push_back(line);
            leftScanlines.push_back(line);
        gap *= 2; // exponential spacing
    }
    
    // create some tight horizontal scanlines from just below out to well
    // above the horizon. These are for beacons/goals. If we are currently in
    // a grab make fewer, but really tight together (looking for the goal)

    // start this far below horizon
    if (isGrabbed) {
        gap = (int)(-FULL_SCANLINE_SPACING);
    } else {
        gap = (int)(-FULL_SCANLINE_SPACING * 1.35);
    }
    int nHorzLines = 0;
    while (true) {
        if (nHorzLines++ > MAX_HORZ_SCANLINES)
            break;
        point centrePoint = horizonCentre; // centre of scanline
        if (gap > MAX_HORZ_SCANLINE_GAP) {
            //rarely anything useful above here, so don't process
            break;
        }
        if (fabs(horizonGradient) < 1) {
            centrePoint.second = centrePoint.second - gap;
            centrePoint.first = solveForY(horizonCentre, -1/horizonGradient,
                                            centrePoint.second);
            if (centrePoint.second < 0)
                break;
        } else if ((horizonGradient < 0 && ! horizonUpsideDown)
                || (horizonGradient > 0 && horizonUpsideDown)) {
            centrePoint.first = centrePoint.first - gap;
            centrePoint.second = solveForX(horizonCentre, -1/horizonGradient,
                                            centrePoint.first);
            if (centrePoint.first < 0) 
                break;
            
        } else {
            centrePoint.first = centrePoint.first + gap;
            centrePoint.second = solveForX(horizonCentre, -1/horizonGradient,
                                            centrePoint.first);
            if (centrePoint.first > CPLANE_WIDTH - 1)
                break;
        }
        pair<point, point> line = makeHorzScanline(centrePoint);
        if (length(line.first, line.second) > 2)
            horzScanlines.push_back(line);
        if (isGrabbed) {
            // fixed spacing
            gap += 1;
        } else {
            // slow exponential spacing
            gap += FULL_SCANLINE_SPACING/8 + abs(gap/5);
        }
    }
}

/* Constructs two scanlines of direction dir and length
 * LEN_MORE_SCANLINES each to the left and right of the specified point (x,y)
 * and one each above and below and processes them. This
 * is intended to create additional scanlines in areas of interest.
 */
void SubVision::makeMoreScanlines(int x, int y, int dir) {

    point p, q;
    if (dir == DIR_UP) {       
        int newx = x, newy = y;
        // to the left
        for (int i = 0; i < 2; ++i) {
            newx -= (i+1)*OFFSET_MORE_SCANLINES;
            if (newx <= 0)
                newx = 1;
            newy = y + LEN_MORE_SCANLINES/2; // start at the bottom
            if (newy >= CPLANE_WIDTH - 1)
                newy = CPLANE_WIDTH - 2;
            p = point(newx, newy);
            q = point(p.first, p.second - LEN_MORE_SCANLINES);               
            scanlines.push_back(make_pair(q, p));
            processScanline(q, p, dir, false);
        }
        
        // and right
        newx = x;
        for (int i = 0; i < 2; ++i) {
            newx += (i+1)*OFFSET_MORE_SCANLINES;
            if (newx >= CPLANE_WIDTH - 1)
                newx = CPLANE_WIDTH - 2;
            newy = y + LEN_MORE_SCANLINES/2;
            if (newy >= CPLANE_WIDTH - 1)
                newy = CPLANE_WIDTH - 2;
            p = point(newx, newy);
            q = point(p.first, p.second - LEN_MORE_SCANLINES);      
            scanlines.push_back(make_pair(q, p));
            processScanline(q, p, dir, false);
        }
        
    } else {
        int newx = x, newy = y;
        // above
        newy -= OFFSET_MORE_SCANLINES;
        if (newy <= 0)
            newy = 1;
        newx -= LEN_MORE_SCANLINES/2;
        if (newx <= 0)
            newx = 1;
        p = point(newx, newy);
        q = point(p.first + LEN_MORE_SCANLINES, p.second); 
        scanlines.push_back(make_pair(p, q));
        processScanline(p, q, dir, false);
        
        
        // and below
        newy = y + OFFSET_MORE_SCANLINES;
        if (newy >= CPLANE_HEIGHT - 1)
            newy = CPLANE_HEIGHT - 2;
        p = point(newx, newy);
        q = point(p.first + LEN_MORE_SCANLINES, p.second);
        scanlines.push_back(make_pair(p, q));
        processScanline(p, q, dir, false);
    }
}

/* Constructs a scan line from the specified point on the horizon 
 * "down" perpendicular to the horizon for length pixels, or until it intercepts
 * with an image border. Returns the (x,y) coordinates of the second point.
 */
point SubVision::makeScanline(point hIntercept, int len) {
    point p;
    double scanGradient;

    scanGradient = -1.0 / horizonGradient;
    // first try solving for y = CPLANE_HEIGHT (or y = 0 if image is upside
    // down). If that's within the image then it's our intercept
    if (horizonUpsideDown)
        p.second = IMAGE_EDGE;
    else 
        p.second = CPLANE_HEIGHT - 1 - IMAGE_EDGE;

    p.first = solveForY(hIntercept, scanGradient, p.second);
    if (p.first < IMAGE_EDGE) {
        // no, intercept is with left border
        p.first = IMAGE_EDGE;
        p.second = solveForX(hIntercept, scanGradient, p.first);
    } else if (p.first > CPLANE_WIDTH - 1 - IMAGE_EDGE) {
        // no, intercept is with right border
        p.first = CPLANE_WIDTH - 1 - IMAGE_EDGE;
        p.second = solveForX(hIntercept, scanGradient, p.first);
    } else {
        // yes, intercept is with bottom/top edge
    }

    // Clip to desired length
    if (length(hIntercept, p) > len) {
        if (fabs(scanGradient) < 1) {
            p.first = hIntercept.first + SIGN(p.first - hIntercept.first) * len;
            p.second = solveForX(hIntercept, scanGradient, p.first);
        } else {
            p.second = hIntercept.second +
                SIGN(p.second - hIntercept.second) * len;
            p.first = solveForY(hIntercept, scanGradient, p.second);
        }
    }
//    cout << "makeScanline " << hIntercept.first << ", " << hIntercept.second
//        << " to " << p.first << ", " << p.second << endl;
    return p;
}

/* Constructs a scan line from the specified point parallel to the horizon
 * until it intercepts the image borders on either side. Returns the (x,y)
 * coordinates of both points.
 */
pair<point, point> SubVision::makeHorzScanline(point centrePoint) {
    point p, q;

    // p (leftmost) first. Try solving for x = 0
    if (! horizonUpsideDown) {
        p.first = IMAGE_EDGE;
    } else {
        p.first = CPLANE_WIDTH - 1 - IMAGE_EDGE;
    }
    p.second = solveForX(centrePoint, horizonGradient, p.first);
    if (p.second < IMAGE_EDGE) { // intercept with top edge
        p.second = IMAGE_EDGE;
        p.first = solveForY(centrePoint, horizonGradient, p.second);
    } else if (p.second > CPLANE_HEIGHT - 1 - IMAGE_EDGE) { // intercept bottom edge
        p.second = CPLANE_HEIGHT - 1 - IMAGE_EDGE;
        p.first = solveForY(centrePoint, horizonGradient, p.second);
    } // else intercept with left edge
    
    // now q (rightmost). Try x = CPLANE_WIDTH - 1
    if (! horizonUpsideDown) {
        q.first = CPLANE_WIDTH - 1 - IMAGE_EDGE;
    } else {
        q.first = IMAGE_EDGE;
    }
    q.second = solveForX(centrePoint, horizonGradient, q.first);
    if (q.second < IMAGE_EDGE) { // intercept with top edge
        q.second = IMAGE_EDGE;
        q.first = solveForY(centrePoint, horizonGradient, q.second);
    } else if (q.second > CPLANE_HEIGHT - 1 - IMAGE_EDGE) { // intercept bottom edge
        q.second = CPLANE_HEIGHT - 1 - IMAGE_EDGE;
        q.first = solveForY(centrePoint, horizonGradient, q.second);
    } // else intercept with left edge

    return make_pair(p, q);
}

/* Processes a given scan line (bottom to top/left to right) over the YUV
 * planes and extracts features which are stored in this object.
 * dir is DIR_UP (in which case this goes bottom to top) or DIR_RIGHT (left
 * to right). If lookForGreen is true then field green features will be
 * inserted every 2*FULL_SCANLINE_SPACING until an obstacle is detected on the
 * line.
 */
void SubVision::processScanline(point top, point bottom, int dir, int index,
                                bool lookForGreen, bool beaconsAndGoals) {

    //cout << "processScanLine top " << top.first << "," << top.second
    //    << " bottom " << bottom.first << "," << bottom.second
    //    << " dir " << dir << endl;
    int insanity = 0;
    int greenCount = 0;    // count pixels between field green points
    int obstacles = 0;     // how many obstacles so far on this line
    bool obstacle = false; // whether last pix was obstacle
    int lineCount = 0;     // count line edges in this scanline
    int featureIndex = features.size(); // index of first feature we will create
    int lowestObstacle = -1; // vertical index of lowest obstacle
    
    int x, y;
    uchar yy, uu, vv;
    Color cc;
    
    int validObsCount = 0;
    vector<int> obsUnresolved;
    
    int verticalIndex = 0; 
    vector<Color> colourVertical;   // stores each colours 

    if (dir == DIR_UP) {
        swap(top, bottom); // scan verticals from the bottom up
    }
        
    LineDraw::startLine(top, bottom);
   
    // initializing: ignore return
    testPixel(top,0,0,0,cNONE,dir,beaconsAndGoals,index,true,insanity); 
    //transitionFeatures.clear();
   
    x = top.first;
    y = top.second;          
    if (0 <= x && x < CPLANE_WIDTH && 0 <= y && y < CPLANE_HEIGHT) {
        // YUV levels
        yy = img.y(x, y);
        uu = img.u(x, y);
        vv = img.v(x, y);
        cc = img.classify(yy,uu,vv);
        testPixel(top,yy,uu,vv,cc,dir,beaconsAndGoals,index,false,insanity);
    }
    
    point next;
    VisualFeature prev; // prev is previous interesting point
    VisualFeature vf;
    while ((next = LineDraw::nextPoint()) != bottom ) {  
        
        obstacle = false;        
        verticalIndex++;
        
        x = next.first;
        y = next.second;          
        if (x < IMAGE_EDGE || x > CPLANE_WIDTH - 1 - IMAGE_EDGE
                || y < IMAGE_EDGE || y > CPLANE_HEIGHT - 1 - IMAGE_EDGE) {
            continue; 
        }
        //
        // YUV levels
        yy = img.y(x, y);
        uu = img.u(x, y);
        vv = img.v(x, y);
        cc = img.classify(yy,uu,vv);
        vf = testPixel(next,yy,uu,vv,cc,dir,beaconsAndGoals,index,false,insanity);    
        
        vf.vertical = verticalIndex;
        colourVertical.push_back(cc);
        
        //------------------------
        // Obstacle filtering...    
        
  
        if (obsUnresolved.size() > 0) { 
            vector<int>::iterator itr = obsUnresolved.begin(); 
            for (; itr != obsUnresolved.end();) { 

                int start = obstacleFeatures[*itr].vertical;
                if (verticalIndex - start == 20) { 
                    int obsRedCount = 0;      // count red colours above obstacles
                    int obsBlueCount = 0;     // count blue colours above obstacles
                    int obsBallCount = 0;     // count ball colours above obstacles
                    int obsWhiteCount = 0;    // count white colours above obstacles
                    int obsGreenCount = 0;    // count green colours above obstacles 
                    for (int i = start; i < start + 20; i++) { 
                        Color tcc = colourVertical[i]; 
                        if (tcc == cFIELD_GREEN) { 
                            obsGreenCount++; 
                        } else if (tcc == cWHITE) { 
                            obsWhiteCount++;
                        } else if (tcc == cBALL) { 
                            obsBallCount++;
                        } else if (tcc == cROBOT_RED) { 
                            obsRedCount++; 
                        } else if (tcc == cROBOT_BLUE) { 
                            obsBlueCount++;
                        }
                    }
                    if (obsGreenCount > 10
                        || obsBallCount > 5) {                     
                        obstacleFeatures[*itr].type = VF_NONE;
                        validObsCount--;    
                    } else {
                        if (lowestObstacle == -1)
                            lowestObstacle = obstacleFeatures[*itr].vertical;
                    }
                    itr = obsUnresolved.erase(itr);
                    continue;
                    
                }
                itr++; 
            }
        }
        //------------------------
            
        if (vf.type == VF_OBSTACLE) {
            // there's a tradeoff here between rejecting noise that is the
            // obstacle (e.g. robot) and making sure we don't discard too much
            // good information (beacons above robots, ball above shadow...).
            // We can't just stop scanning but fieldline edges above noise
            // are highly spurious, so we'll ignore them.
            // At medium range you only get one dark pixel of robot shadow.
            // Check for non green colours for possible obstacle above.
            if (! obstacle && obstacles < MAX_OBSTACLE_LINE) {
                //&& dirColour(next.first,next.second,dir,cFIELD_GREEN,MAX_GREEN_ABOVE_OBS) < TH_GREEN_ABOVE_OBS) {
                //part of new obstacle
                ++obstacles;
                vf.scanline = index;
                obstacleFeatures.push_back(vf);
                
                validObsCount++;
                obsUnresolved.push_back(obstacleFeatures.size()-1);
            }
            obstacle = true;
        } else if (vf.type != VF_NONE && vf.type != VF_UNKNOWN) {
            //FOR SLAM CHALLENGE, no enemy dogs around, need all points 
            //that possible a field lines

#ifndef LOCALISATION_CHALLENGE
            if (vf.type == VF_FIELDLINE && lineCount > MAX_LINE_FEATURES) {
                // ignore line features after the MAX_LINE_FEATURES'th
                // (probably noise)
                continue;
            }
#endif
            // clump consecutive features, taking the steepest sum of slope as
            // the centre pix, and it's classifcation as the type
            if (adjacent(prev, vf) && SIGN(vf.dsum) == SIGN(prev.dsum)) {
                //cout << "... and similar, merging" << endl;
                if (fabs(vf.dsum) > fabs(prev.dsum)) {
                    features.back() = vf;
                }
                prev = vf;
            } else {
                if (vf.type == VF_FIELDLINE) {
                    ++lineCount;
                }
                vf.scanline = index;
                features.push_back(vf);
                prev = vf;
            }
        }

        // occasionally check for a green area of carpet
        ++greenCount;
        if (lookForGreen && ! obstacle
                && greenCount >= 2 * FULL_SCANLINE_SPACING) {
           if (isFieldGreen(vf)) {
               vf.type = VF_FIELD;
               vf.scanline = index;
               features.push_back(vf);
               fieldFeatureCount++;
           }
           greenCount = 0;
        }
    } // while nextPoint

    // if there were more than two transition features then some of them were
    // wrong, so ignore. Else copy them into features
//    if (transitionFeatures.size() <= 2) {
//        features.insert(features.end(), transitionFeatures.begin(),
//                        transitionFeatures.end());
//    }

    // if this scanline had obstacles discard the field line points
    // we found. Note that this is the wrong thing if the obstacles were
    // above the line points
#ifndef LOCALISATION_CHALLENGE
    if (validObsCount > 0 && lowestObstacle > -1) {
        //cerr << "scanline " << index << ", validObs " << validObsCount
        //    << ", lowest obs " << lowestObstacle
        //    << ". Invalidating line features." << endl;
        for (unsigned i = featureIndex; i < features.size(); ++i) {
            if (features[i].type == VF_FIELDLINE
                    && features[i].vertical > lowestObstacle)
                features[i].type = VF_NONE;
        }
    }
#endif

    // catch any beacon/goal line that runs off the right edge of screen
    if (beaconsAndGoals) {
        x = bottom.first; 
        y = bottom.second;        
        if (0 <= x && x < CPLANE_WIDTH && 0 <= y && y < CPLANE_HEIGHT) {                            
            // YUV levels
            yy = img.y(x, y);
            uu = img.u(x, y);
            vv = img.v(x, y);
            cc = img.classify(yy,uu,vv);

            vf = testPixel(bottom,yy,uu,vv,cc,dir,false,index,false,insanity);
            if (vf.type == VF_PINK || vf.type == VF_BLUE || vf.type == VF_YELLOW) {
                vf.scanline = index;
                features.push_back(vf);
            }        
        }

    }
    

}


/* Tests if this pixel is interesting relative to the previous one. Returns
 * a VisualFeature, with type VF_NONE if it's not interesting. If 
 * beaconsGoals is false interesting things are field line edges, ball edges and
 * obstacles (shadows). If beaconsGoals is true interesting things are runs
 * of (mostly) pink, blue and yellow classified pixels. Call once with
 * beaconsGoals false to reset the internal pink/blue/yellow counters.
 */
VisualFeature SubVision::testPixel(point p, uchar yy, uchar uu, uchar vv, 
                                   Color cc, int dir, bool beaconsGoals, 
                                   int scanline, bool initialiseOnly,
                                   int& insanity) {

    insanity = VF_SANE;

    visualfeature_type type = VF_NONE;
    static int prevY = 0, prevU = 0, prevV = 0; // previous YUV  
        
    // count consecutive pink/blue/yellow and not pink/blue/yellow
    static int countPink = 0, countBlue = 0, countYellow = 0;
    static int countNotPink = 0, countNotBlue = 0, countNotYellow = 0;
    // positions of runs of pink/blue/yellow
    static int startPinkX = -1, startPinkY = -1, endPinkX = -1, endPinkY = -1;
    static int startBlueX = -1, startBlueY = -1, endBlueX = -1, endBlueY = -1;
    static int startYellowX = -1, startYellowY = -1, endYellowX = -1, endYellowY = -1;

    int x = p.first, y = p.second;          
    if (x < 0 || x >= CPLANE_WIDTH || y < 0 || y >= CPLANE_HEIGHT)
        initialiseOnly = true;
   
    // We don't use pix in the corners of the image
    // because it's too noisy
    int cornerDist = 3*IMAGE_EDGE; // pix
    if ((x < cornerDist && y < cornerDist)
            || (x < cornerDist && y > CPLANE_HEIGHT - 1 - cornerDist)
            || (x > CPLANE_WIDTH - 1 - cornerDist && y < cornerDist)
            || (x > CPLANE_WIDTH - 1 - cornerDist 
                    && y > CPLANE_HEIGHT - 1 - cornerDist)) {
        initialiseOnly = true;
    }
    
#if 0
    double sqrCornerDist = SQUARE(cornerDist);
    if (DISTANCE_SQR(x, y, 0, 0) < sqrCornerDist
            || DISTANCE_SQR(x, y, CPLANE_WIDTH -1, 0) < sqrCornerDist
            || DISTANCE_SQR(x, y, 0, CPLANE_HEIGHT -1) < sqrCornerDist
            || DISTANCE_SQR(x, y, CPLANE_WIDTH -1, CPLANE_HEIGHT -1) 
                < sqrCornerDist) {
        initialiseOnly = true;
    }
#endif   

    if (initialiseOnly) {
        prevY = -1; prevU = -1; prevV = -1;
        //prevY = yy; prevU = uu; prevV = vv;
        countPink = 0; countBlue = 0, countYellow = 0;
        countNotPink = 0, countNotBlue = 0, countNotYellow = 0;
        startPinkX = startPinkY = endPinkX = endPinkY = -1;
        startBlueX = startBlueY = endBlueX = endBlueY = -1;
        startYellowX = startYellowY = endYellowX = endYellowY = -1;
        
        testBallColourTransition(p,0,0,0,cNONE,dir,scanline,true);
        testWallColourTransition(p,0,0,0,cNONE,dir,scanline,true);
        return VF_NONE;
    }

    // handle line ending at right edge
    if ((dir == DIR_LEFT || dir == DIR_RIGHT) && ! beaconsGoals) {
        if (startPinkX != -1 && countPink >= TH_BEACON_PINK) {
            type = VF_PINK;
            VisualFeature vf(type, startPinkX, startPinkY);
            vf.endx = x;
            vf.endy = y;
//            cout << "run type " << type << " from (" << startx << "," 
//                << starty << ") to (" << endx << "," << endy << ")" << endl;
            startPinkX = startPinkY = endPinkX = endPinkY = -1;
            countPink = 0;
            return vf;
            
        } else if (startBlueX != -1 && countBlue >= TH_GOAL_BLUE) {
            type = VF_BLUE;
            VisualFeature vf(type, startBlueX, startBlueY);
            vf.endx = x;
            vf.endy = y;
//                cout << "run type " << type << " from (" << startx << "," 
//                    << starty << ") to (startPinkX = startPinkY = endPinkX = endPinkY = -1;
            startBlueX = startBlueY = endBlueX = endBlueY = -1;
            countBlue = 0;
            return vf;
            
         } else if (startYellowX != -1 && countYellow >= TH_GOAL_YELLOW) {
            type = VF_YELLOW;
            VisualFeature vf(type, startYellowX, startYellowY);
            vf.endx = x;
            vf.endy = y;
//                cout << "run type " << type << " from (" << startx << "," 
//                    << starty << ") to (" << endx << "," << endy << ")" << endl;
            startYellowX = startYellowY = endYellowX = endYellowY = -1;
            countYellow = 0;
            return vf;
            
        }
        countPink = countBlue = countYellow = 0;
        countNotPink = countNotBlue = countNotYellow = 0;
    }

    // Ball/line/obstacle detection: look for interesting edges
    if (! beaconsGoals) {
        
        testBallColourTransition(p,yy,uu,vv,cc,dir,scanline,false);
        testWallColourTransition(p,yy,uu,vv,cc,dir,scanline,false);
        
        // simple sum of YUV gradients
        int dy = yy - prevY;
        int du = uu - prevU;
        int dv = vv - prevV;
        int sum = abs(dy) + abs(du) + abs(dv);      
        
        // This unusual control structure functions like a goto without
        // actually using one. The reason is that there is some finalisation
        // that needs to happen at the end of each call in setting the prevX
        // values, regardless of the return value, so we can't return
        // immediately. So once a point has been classified  (or failed to be)
        // we break from the loop. The loop only ever executes once.
        while (true) {
            // The pixel tested is the beginning of the scanline. 
            // So ignore it. Just update prevY, prevU, prevV.
            if (prevY == -1) {
                break;
            }
        
            // for obstacles we search for the low-y level of shadows, feet etc
            if (yy < TH_OBSTACLE_Y) {
                type = VF_OBSTACLE;
                // but we don't break. Allow the point to be classified an edge
                // point below
            }
            
            #ifdef OFFLINE
            if (sum < TH_ANYTHING) {
                insanity = VF_BELOW_SUM_THRESH;
            }
            #endif 
            
            // if the sum of changes is too small, quit now
            if (sum < TH_ANYTHING) {   
                break;
            }
    
            // Ball. A something/orange transition has a large du but not
            // necessarily much dy
            if (abs(du) > TH_BALL_DU            // minimum du
                    && (dv == 0 || abs(du/dv) < TH_BALL_DUDV
                        || SIGN(du) == -SIGN(dv)) // dv opp du
                     // not specular reflection
                    && yy < TH_SPECULAR_Y && prevY < TH_SPECULAR_Y) {
                // if this edge is leaving a ball, reverse dir to point back
                // towards the orange
                if (du < 0) {
                    if (dir == DIR_UP)
                        dir = DIR_DOWN;
                    else if (dir == DIR_DOWN)
                        dir = DIR_UP;
                    if (dir == DIR_LEFT)
                        dir = DIR_RIGHT;
                    else if (dir == DIR_RIGHT)
                        dir = DIR_LEFT;
                } 
                //cerr << " potential balledge on scanline " << scanline
                //    << " at " << x << "," << y
                //    << " orange dir " << dir << endl;
                 
                // check orange towards the ball 
                int ballColour;
                ballColour = dirColour(x, y, dir, cBALL, BALL_COLOUR_SEARCH);
                if (ballColour >= MIN_BALL_COLOUR
                        && dirColour(x, y, dir, cROBOT_RED, BALL_COLOUR_SEARCH)
                            <= MAX_NONBALL_COLOUR
                        && dirColour(x, y, dir, cBEACON_PINK,BALL_COLOUR_SEARCH)
                            <= MAX_NONBALL_COLOUR
                        && dirColour(x, y, dir, cBEACON_YELLOW,
                                    BALL_COLOUR_SEARCH) < MAX_NONBALL_COLOUR) {
                
                //ballColour = nearColour(x, y, cBALL, COLOUR_SEARCH_RADIUS);
                //if (ballColour > MIN_COLOUR_NEAR_FEATURE
                //        && ballColour > nearColour(x, y, cROBOT_RED,
                //                                        COLOUR_SEARCH_RADIUS)
                //        && ballColour > nearColour(x, y, cBEACON_PINK,
                //                                        COLOUR_SEARCH_RADIUS)) {
                    
#ifndef DISABLE_BALL_EDGE_DETECTION
                    //cerr << "Colour confirmed feature" << endl;
                    type = VF_BALL;
#endif
                    break;
                }
            }

            /*
            #ifdef OFFLINE
            if (!(abs(du) > TH_BALL_DU)) {           // minimum du
                insanity = VF_BELOW_BALL_DU_THRESH;
            } else if (!(dv == 0 || abs(du/dv) < TH_BALL_DUDV || SIGN(du) == -SIGN(dv))) { // dv opp du
                insanity = VF_DV_DU_OPPOSITE;
            } else if (!(yy < TH_SPECULAR_Y)) { 
                insanity = VF_ABOVE_SPECULAR_THRESH;
            } else if (!(prevY < TH_SPECULAR_Y)) {
                insanity = VF_ABOVE_SPECULAR_THRESH; 
            } else if (!(ballNearColour > MIN_COLOUR_NEAR_FEATURE)) { 
                insanity = VF_NOT_MUCH_BALL_FEATURE;           
            } else if (!(ballNearColour > nearColour(x, y, cROBOT_RED, COLOUR_SEARCH_RADIUS))) { 
                insanity = VF_LOOKS_LIKE_RED_ROBOT;           
            } else if (!(ballNearColour > nearColour(x, y, cBEACON_PINK, COLOUR_SEARCH_RADIUS))) { 
                insanity = VF_LOOKS_LIKE_PINK_BEACON;
            }
            #endif
            */
                
            // Field lines. A white/green transition has most of its change in Y
            if (abs(dy) > TH_FIELDLINE_DY            // minimum dy
                    && abs(du) < TH_FIELDLINE_DUV     // maximum du and v
                    && abs(dv) < TH_FIELDLINE_DUV 
                    && (abs(du) < TH_FUZZY || SIGN(du) == SIGN(dy)))// du follows dy
                    // alt: dy > FIELDLINE_Y and dy > du, dy > dv
            {
                // if we don't already think this is an obstacle (e.g. robot
                // leg) then it's a line
                if (type != VF_OBSTACLE)
                    type = VF_FIELDLINE;
                // if this edge is leaving a line, reverse dir to point back
                // towards the white
                if (dy < 0) {
                    if (dir == DIR_UP)
                        dir = DIR_DOWN;
                    else if (dir == DIR_DOWN)
                        dir = DIR_UP;
                //} else if (dy > 0) {
                    if (dir == DIR_LEFT)
                        dir = DIR_RIGHT;
                    else if (dir == DIR_RIGHT)
                        dir = DIR_LEFT;
                }
                break;
            }
            type = VF_UNKNOWN;
            break;
        } // while(true)

        prevY = yy;
        prevU = uu;
        prevV = vv;

        // the feature coordinates are in the middle of the edge between pixels
        // that gave the biggest change. This assumes vertical lines are
        // scanned upwards (decreasing y), horizontal scanned left to right
        // (increasing x), and image coords refer to the centre of their pixel
        if (dir == DIR_UP) {
            return VisualFeature(type, x, y + 0.5, dy, du, dv, dir);
        } else {
            return VisualFeature(type, x - 0.5, y, dy, du, dv, dir);
        }
    }

    // beacons/goals detection. Look for runs of pink/blue/yellow
    else {
        // double not-colour tolerance when grabbed
        int notPinkTh = isGrabbed ? 2*TH_BEACON_NOTPINK : TH_BEACON_NOTPINK;
        int notYellowTh = isGrabbed ? 2*TH_GOAL_NOTYELLOW : TH_GOAL_NOTYELLOW;
        int notBlueTh = isGrabbed ? 2*TH_GOAL_NOTBLUE : TH_GOAL_NOTBLUE;
        if (cc == cBEACON_PINK) {
            if (countPink == 0) {
                startPinkX = x;
                startPinkY = y;
            }
            endPinkX = x;
            endPinkY = y;
            ++countPink;
            countNotPink = 0;
            #ifdef OFFLINE
            //cout << __func__ << " : pink detected at point (" << x << "," << y << ") " << endl;
            //cout << __func__ << " : pink count so far " << countPink << endl;
            #endif
        } else {
            ++countNotPink;
            if (countNotPink > notPinkTh) {
                if (countPink >= TH_BEACON_PINK) {
                    type = VF_PINK;
                }
                countPink = 0;
            }
        } 
        if (cc == cBEACON_BLUE) {            
            if (countBlue == 0) {
                startBlueX = x;
                startBlueY = y;
            }
            endBlueX = x;
            endBlueY = y;
            ++countBlue;
            countNotBlue = 0;
            #ifdef OFFLINE
            //cout << __func__ << " : blue detected at point (" << x << "," << y << ") " << endl;
            //cout << __func__ << " : blue count so far " << countBlue << endl;
            #endif
        } else {
            ++countNotBlue;
            if (countNotBlue > notBlueTh) {
                if (countBlue >= TH_GOAL_BLUE) {
                    type = VF_BLUE;
                }
                countBlue = 0;
            }
        } 
        if (cc == cBEACON_YELLOW) {
            //cout << "(" << x << "," << y << ") " << endl;
            if (countYellow == 0) {
                startYellowX = x;
                startYellowY = y;
            }
            endYellowX = x;
            endYellowY = y;
            ++countYellow;
            countNotYellow = 0;
            #ifdef OFFLINE
            //cout << __func__ << " : yellow detected at point (" << x << "," << y << ") " << endl;
            //cout << __func__ << " : yellow count so far " << countYellow << endl;
            #endif
        } else {
            ++countNotYellow;
            if (countNotYellow > notYellowTh) {
                if (countYellow >= TH_GOAL_YELLOW) {
                    type = VF_YELLOW;
                }
                countYellow = 0;
            }
        }

        //TODO: there are some bogus zero-length features coming out of the 
        // above that I filter here, but it would be better to root them out      
        if (type == VF_PINK) {
            if (DISTANCE_SQR(startPinkX, startPinkY, endPinkX, endPinkY) + 1 
                    >= SQUARE((int)TH_BEACON_PINK)) {
                VisualFeature vf(type, startPinkX, startPinkY);
                vf.endx = endPinkX;
                vf.endy = endPinkY;
                startPinkX = startPinkY = endPinkX = endPinkY = -1;
                #ifdef OFFLINE
                //cout << __func__ << " : pink runs detected" << endl;
                #endif
                return vf;
            }

        } else if (type == VF_BLUE) {
            if (DISTANCE_SQR(startBlueX, startBlueY, endBlueX, endBlueY) + 1 
                    >= SQUARE((int)TH_GOAL_BLUE)) {
                VisualFeature vf(type, startBlueX, startBlueY);
                vf.endx = endBlueX;
                vf.endy = endBlueY;
                startBlueX = startBlueY = endBlueX = endBlueY = -1;
                #ifdef OFFLINE
                //cout << __func__ << " : blue runs detected" << endl;
                #endif
                return vf;
            } 

        } else if (type == VF_YELLOW) { 
            if (DISTANCE_SQR(startYellowX, startYellowY, endYellowX, endYellowY) + 1 
                    >= SQUARE((int)TH_GOAL_YELLOW)) {
                VisualFeature vf(type, startYellowX, startYellowY);
                vf.endx = endYellowX;
                vf.endy = endYellowY;
                startYellowX = startYellowY = endYellowX = endYellowY = -1;
                #ifdef OFFLINE
                //cout << __func__ << " : yellow runs detected" << endl;
                #endif
                return vf;
            }

        }


        return VisualFeature(); // VF_NONE
    }

}


/* Detects transitions from runs of orange classified points to runs of
 * non-orange classified points. These are probably ball edges that are
 * possibly too blury or red to pick up in edge detection
 */
void SubVision::testBallColourTransition(point p, uchar yy, uchar uu, uchar vv, 
                                         Color cc, int dir, 
                                         int scanline, bool initialiseOnly) {  
#ifdef DISABLE_BALL_COLOUR_TRANSITION
    return; 
#endif
    
    // point start non-orange
    static classifiedPoint startNotOrange = classifiedPoint(-1,-1); 
    static classifiedPoint endNotOrange = classifiedPoint(-1,-1); 
    // point start and end orange 
    static classifiedPoint startOrange = classifiedPoint(-1,-1);
    static classifiedPoint endOrange = classifiedPoint(-1,-1);
    // point start and end maybe-orange 
    static classifiedPoint startMaybeOrange = classifiedPoint(-1,-1);
    static classifiedPoint endMaybeOrange = classifiedPoint(-1,-1);
    
    // number of detected transitions in this scanline
    static int countDetected = 0;
    // pointer to last transition feature
    static VisualFeature* lastTransitionFeature = 0;

    // count of white pix
    static int countWhite = 0;
    // count of orange pix
    static int countOrange = 0;
    // count of non-orange pix
    static int countNotOrange = 0;
    // count of maybe-orange pix
    static int countMaybeOrange = 0;
    // count of consecutive orange pix
    static int countConsecOrange = 0;

    // count since the last white and green pix
    static int sinceGreen = 0, sinceWhite = -1;
    
    // true when we get a run of orange (possibly separated by maybeorange)
    static bool orangeFound = false;
    // true when we get a run of non-orange
    static bool notOrangeFound = false;
    // true when we get a run of maybe-orange
    static bool maybeOrangeFound = false;
    // true when we get a run of consecutive orange
    static bool consecOrangeFound = false;
    // true when we find a transition
    static bool transitionFound = false;
    
    // true if orange was found after non-orange, else false
    static bool transitionToOrange = false;

    if (initialiseOnly) {
        countDetected = 0;
        lastTransitionFeature = 0;

        orangeFound = false; 
        maybeOrangeFound = false;
        notOrangeFound = false;
        consecOrangeFound = false;
        transitionFound = false;

        countWhite = 0;
        countOrange = 0;
        countNotOrange = 0;
        countMaybeOrange = 0;
        countConsecOrange = 0;
        sinceGreen = 0;
        sinceWhite = -1;
        return;
    }
    
    // Don't find more than 2 points in a scanline
    // TODO: throw them all out if we do
    //if (countDetected > 2) { 
    //    return;
    //}

    int x = p.first, y = p.second; 

    ++sinceWhite;
    ++sinceGreen;

    if (cc == cBALL) {
        if (countOrange == 0) { 
            startOrange.set(x,y,yy,uu,vv,cc);     
        }

        endOrange.set(x,y,yy,uu,vv,cc);
        countOrange++; 
        countConsecOrange++;
        if (countConsecOrange >= MIN_CONSEC_ORANGE_TRANSITION) {
            consecOrangeFound = true;
        }
        if (countOrange >= MIN_ORANGE_TRANSITION && consecOrangeFound) { 
            orangeFound = true; 
            if (notOrangeFound) { // a transition *into* orange
                transitionFound = true;
                transitionToOrange = true;
                notOrangeFound = false;
            }
            // on a transition into ball (but not out of) we use the start of
            // the maybe-orange, which will be earlier than the orange
            if (countMaybeOrange >= countOrange && dir == DIR_UP) {
                maybeOrangeFound = true;
            }
        }
        // if we found orange and already have two points then at least the
        // last one is decidedly dodgy
        if (countOrange >= MIN_ORANGE_TRANSITION 
//                && transitionFeatures.size() >= 2) {
                && countDetected >= 2 && lastTransitionFeature != 0) {
//            transitionFeatures.back().type = VF_NONE;
            lastTransitionFeature->type = VF_NONE;
            --countDetected;
        }

        countNotOrange = countWhite = 0; 
        // don't reset countMaybeOrange
    } else if (cc == cROBOT_RED || cc == cBEACON_PINK
            || cc == cBEACON_YELLOW) {
        if (countMaybeOrange == 0) {
            startMaybeOrange.set(x,y,yy,uu,vv,cc);     
        }
        endMaybeOrange.set(x, y, yy, uu, vv, cc);
        countMaybeOrange++;
        countConsecOrange = 0;
        countNotOrange = countWhite = 0; 
        // don't reset countOrange
    } else { 
        //cBEACON_BLUE cROBOT_BLUE cFIELD_GREEN cWHITE cBLACK cNONE cROBOT_GREY?
        if (cc == cWHITE) {
            countWhite++;
            if (countWhite > 1)
                sinceWhite = 0;
        } else {
            countWhite = 0;
        }
        if (cc == cFIELD_GREEN) sinceGreen = 0;
        // FIXME: this reset causes sinceGreen to be small later....

        if (countNotOrange == 0) {
            startNotOrange.set(x, y, yy, uu, vv, cc);
        }
        endNotOrange.set(x, y, yy, uu, vv, cc);

        countNotOrange++; 
        countConsecOrange = 0;
        if (countNotOrange >= MIN_NOTORANGE_TRANSITION) {
            //if (countOrange > 0) {
            //    cerr << "notOrangeFound. Scanline " << scanline
            //        << " CountNotOrange = " << countNotOrange
            //        << " countOrange = " << countOrange
            //        << " countMaybeOrange = " << countMaybeOrange << endl;
            //}
            // if this is the end of and orange run remember as such
            notOrangeFound = true;
            if (orangeFound) {  // transition *out of* orange
                transitionFound = true;
                transitionToOrange = false;
                orangeFound = false;
            }
            // we don't use maybeOrange on a transition out of the ball
            // since that is frequently confused with red robot etc
            
            // reset maybe/orange counts
            countOrange = countMaybeOrange = 0;
            // Need to find another consec orange to get another point
            consecOrangeFound = false;
        }
    }

    // If the transition distance was less than 6 pixels, then consider mid
    // point as a ball edge.
    if (transitionFound) {
        classifiedPoint start(-1, -1), end(-1, -1);

        if (transitionToOrange) {
            start = endNotOrange;
            if (maybeOrangeFound) {
                end = startMaybeOrange;
            } else {
                end = startOrange;
            }
        } else {
            start = endOrange;
            end = startNotOrange;
        }
        double distsqr = DISTANCE_SQR(start.x, start.y, end.x, end.y); 

        if (distsqr >= SQUARE(6)) {
            return;
        }

        int dy, du, dv;
        double midx, midy;

        dy = end.yy - start.yy;
        du = end.uu - start.uu;
        dv = end.vv - start.vv; 
        midx = (end.x + start.x) / 2.0;
        midy = (end.y + start.y) / 2.0;

        // reverse dir to point towards orange if necessary
        if (! transitionToOrange) {
            if (dir == DIR_UP)
                dir = DIR_DOWN;
            else if (dir == DIR_DOWN)
                dir = DIR_UP;
            if (dir == DIR_LEFT)
                dir = DIR_RIGHT;
            else if (dir == DIR_RIGHT)
                dir = DIR_LEFT;
        }            
                            
        // Transitions above white without intervening green are highly
        // spurious and likely robots or beacons. Ignore this check when
        // grabbing
        if (!isGrabbing && sinceWhite < sinceGreen /*&& sinceWhite != -1*/) {
            // add a sanitied edge point
            VisualFeature vf = VisualFeature(VF_NONE,midx,midy,dy,du,dv,dir);
            features.push_back(vf);
            countDetected++;
#ifdef TESTPIXEL_DEBUG
            cerr << "Ignored a point above white at " << midx << ", "
                << midy << endl;
#endif
        } else {
            VisualFeature vf = VisualFeature(VF_BALL,midx,midy,dy,du,dv,dir);
            vf.scanline = scanline;
            features.push_back(vf);
            lastTransitionFeature = &features.back();
            // push temporarily onto transitionFeatures
//            transitionFeatures.push_back(vf);
            countDetected++;
#ifdef TESTPIXEL_DEBUG
            cout << endl;
            cout << __func__ << " : scanline " << scanline << endl;
    //        cout << __func__ << " : startOrange at (" 
    //            << startOrange.x << ", " << startOrange.y << ")" 
    //            << " u : " << int(startOrange.uu) << endl;
    //        cout << __func__ << " : endOrange at (" 
    //            << endOrange.x << ", " << endOrange.y << ")" 
    //            << " u : " << int(endOrange.uu) << endl;
    //        cout << __func__ << " : startNotOrange at (" 
    //            << startNotOrange.x << ", " << startNotOrange.y << ")" 
    //            << " u : " << int(startNotOrange.uu) << endl;
    //        cout << __func__ << " : endNotOrange at (" 
    //            << endNotOrange.x << ", " << endNotOrange.y << ")" 
    //            << " u : " << int(endNotOrange.uu) << endl;
    //        cout << __func__ << " : startMaybeOrange at (" 
    //            << startMaybeOrange.x << ", " << startMaybeOrange.y << ")" 
    //            << " u : " << int(startMaybeOrange.uu) << endl;
    //        cout << __func__ << " : endMaybeOrange at (" 
    //            << endMaybeOrange.x << ", " << endMaybeOrange.y << ")" 
    //            << " u : " << int(endMaybeOrange.uu) << endl;
            cout << __func__ << " : transitionToOrange : "
                << transitionToOrange << endl;
            cout << __func__ << " : start at (" 
                << start.x << ", " << start.y << ")" 
                << " u : " << int(start.uu) << endl;
            cout << __func__ << " : end at (" 
                << end.x << ", " << end.y << ")" 
                << " u : " << int(end.uu) << endl;
            cout << __func__ << " : sinceWhite = " << sinceWhite
                << " sinceGreen = " << sinceGreen << endl;
            cout << __func__ << " : marked a point at (" << midx
                << ", " << midy << ")" << " dir " << dir << endl;            
#endif 
        }

        
        // reset the bools.
        orangeFound = false; 
        maybeOrangeFound = false;
        notOrangeFound = false;
        consecOrangeFound = false;
        transitionFound = false;

        countOrange = 0;
        countMaybeOrange = 0;
        countNotOrange = 0;
        countWhite = 0;
        countConsecOrange = 0;
        sinceGreen = 0;
        sinceWhite = -1;
    }
}


void SubVision::testWallColourTransition(point p, uchar yy, uchar uu, uchar vv, 
                                         Color cc, int dir,
                                         int scanline, bool initialiseOnly) { 

    (void)yy;
    (void)uu;
    (void)vv;
    (void)scanline;
   
    static const int GREEN_TOLERANCE = 1;
    static const int WHITE_TOLERANCE = 0; 
    static const int OTHER_TOLERANCE = 2; 

    static bool wallFound = true;
    static bool greenFound = false; 
    static bool whiteFound = false; 
    static bool greyFound = false;
    static bool otherFound = false;

    static int countGreen = 0;
    static int countNotGreen = 0;     

    static int countWhite = 0;
    static int countGrey = 0;
    static int countNotWhite = 0;
    
    static int countOther = 0; 
    static int countNotOther = 0;
    
    static int startx = 0, starty = 0;
    static int endx = 0, endy = 0;
         
    
    if (initialiseOnly) {
    
        wallFound = false;
        greenFound = false; 
        whiteFound = false; 
        greyFound = false;
        otherFound = false; 
        countGreen = 0; 
        countNotGreen = 0;
        countWhite = 0;
        countGrey = 0;
        countNotWhite = 0;
        countOther = 0; 
        countNotOther = 0;
        
        return;
    }
    
    // If it's not vertical scanline, then ignore it.
    if (dir != DIR_UP) { 
        return;
    }
    
    // If a wall feature is found on this scanline already, then ignore. 
    if (wallFound) { 
        return;
    }

    int x = p.first, y = p.second; 

    // If green is not found yet.
    if (!greenFound) { 
        if (cc == cFIELD_GREEN) { 
            countGreen++; 
            countNotGreen = 0;
        } else { 
            countNotGreen++; 
            if (countNotGreen > GREEN_TOLERANCE) { 
                if (countGreen >= 4) {  
                    greenFound = true;
                }
                countGreen = 0;
            }
        }
    }
    
    // If green is found, but not white or grey yet.
    if (greenFound && !whiteFound) {
        if (cc == cWHITE || cc == cROBOT_GREY) {
            if (countWhite == 0) { 
                startx = x; 
                starty = y;
            }   
            endx = x; 
            endy = y;
            countWhite++;
            if (cc == cROBOT_GREY) { 
                countGrey++;
            }    
            countNotWhite = 0; 
        } else { 
            countNotWhite++; 
            if (countNotWhite > WHITE_TOLERANCE) { 
                if (countWhite >= 5) {
                    whiteFound = true;
                    if (countGrey >= 5) {
                        greyFound = true;
                    }
                } else { 
                    greenFound = false;
                }
                countWhite = 0;
            } 
        }
    } 
    
    // If green and white are found and other is not found. 
    if (greenFound && whiteFound && !otherFound) {    
        if (cc != cWHITE && cc != cROBOT_GREY && cc != cFIELD_GREEN && cc != cBEACON_BLUE && cc < cNUM_COLOR) {             
            countOther++; 
            countNotOther = 0; 
            if (countOther >= 2) {
                otherFound = true;
            }        
        } else { 
            countNotOther++; 
            if (countNotOther > OTHER_TOLERANCE) { 
                if (countOther >= 2) {
                    otherFound = true;
                } else {
                    greenFound = false; 
                    whiteFound = false;
                }
                countOther = 0;
            }
        }
    } 
    
    // a possible wall feature found
    if (greenFound && whiteFound && otherFound) { 
        double midx = (startx + endx) / 2; 
        double midy = (starty + endy) / 2;
        VisualFeature vf = VisualFeature(VF_WALL,midx,midy,0,0,0,0);
        features.push_back(vf);    
    
        wallFound = true;
        greenFound = false; 
        whiteFound = false;
        greyFound = false;
        otherFound = false; 
        countGreen = 0; 
        countNotGreen = 0;
        countWhite = 0;
        countGrey = 0;
        countNotWhite = 0;
        countOther = 0; 
        countNotOther = 0;
    }                        
}



/* Searches a vertical (rel. horizon) line from (x,y) up or down for the
 * presence or absence of the specified colour and returns the pixel
 * coordinates of the last pixel satisfying the given criterial. Searches "up"
 * if up is true, else searches down. If expectColour is true then the return
 * value is the last pixel found of the given colour, else it is the pixel
 * before the first. (x,y) is returned if the test fails immediately.
 */
point SubVision::searchLine(int x, int y, Color colour, bool up,
                            bool expectColour) {
    point p(x, y);

    if (x < 0 || x > CPLANE_WIDTH - 1 || y < 0 || y > CPLANE_HEIGHT -1)
        return p;

    // test if this first pixel gives us the answer anyway
    if (expectColour && (img.classify(x, y) != colour))
        return p;
    else if (! expectColour && (img.classify(x, y) == colour))
        return p;

    point q;
    double gradient = -1/horizonGradient;
    if (! horizonUpsideDown) {
        q.second = up ? 0 : CPLANE_HEIGHT -1;
    } else {
        q.second = up ? CPLANE_HEIGHT -1 : 0;
    }
    
    q.first = solveForY(p, gradient, q.second);
    
    if (q.first < 0) {
        q.first = 0;
        q.second = solveForX(p, gradient, q.first);
    } else if (q.first > CPLANE_WIDTH -1) {
        q.first = CPLANE_WIDTH -1;
        q.second = solveForX(q, gradient, q.first);
    }
    if (p == q) return p;

    point next, prev = p;
    LineDraw::startLine(p, q);
    while ((next = LineDraw::nextPoint()) != q) {
        Color c = img.classify(next.first, next.second);
        if (expectColour && (c != colour))
            return prev;
        else if (! expectColour && (c == colour))
            return prev;
        prev = next;
    }
    
    return q;
}


