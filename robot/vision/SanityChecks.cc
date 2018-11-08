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


/* SanityChecks.cc */

#include "../share/SanityConstant.h"

#include "SanityChecks.h"
#include "SubVision.h"
#include "VisualCortex.h"
#ifndef OFFLINE
#include "InfoManager.h"
#include "Robolink.h"
#endif

//#undef ENABLE_SANITIES
#define ENABLE_SANITIES


// string names for sanities. This must match the enum Insanity in
// SanityChecks.h
const char* insanityNames[] = {
    "sane",
    "RDR insanity",

    "ball above horizon",
    "ball infrared obstacle",
    "ball very large radius",
    "ball not much orange",
    "ball too much red",
    "ball too much pink",
    "ball too much yellow",
    "ball not much green around",
    
    "goal too wide",
    "goal not wide enough",
    "goal floating", 
    "goal above horizon", 
    "goal might be a beacon",
    
    "beacon below horizon",

    "two goals - chose highest cf",
    "two goals - chose lowest elevation",
    "goal in beacon",
    "goal above beacon", 
    "beacon in goal",
    "ball above goal",
    "ball above beacon",
    "diagonally opposite beacons",
    "end beacons too close",
};

/*** Thresholds, limits, factors and other magic numbers ***/

// pixels above the horizon to tolerate the ball centroid
static const int BALL_OVER_HORIZON_LIMIT = 10;
// pixels below horizon to tolerate beacon centroid
static const int BEACON_BELOW_HORIZON_LIMIT = 25;
// max pixels above goal centroid to tolerate top of ball
static const int BALL_ABOVE_GOAL_MAX = 0;

// minimum goal height (pixels)
static const int MIN_GOAL_HEIGHT = 3;
// tolerated error for goal that is too wide
static const double GOAL_ASPECT_TOLERANCE = 1.7;
// pixels above horizon for bottom of goal
static const int GOAL_ABOVE_HORIZON_LIMIT = 20;

// near infrared sensor used sanity out a ball.
static const long BALL_NEAR_SENSOR = 70000;

static const int FIELD_FEATURE_THRESHOLD = 2;

// minimum ball radius which some specific ball sanity to be executed. 
static const int BALL_MINIMUM_RADIUS = 10;
static const int BALL_MAXIMUM_RADIUS = 2000;

static const int ELEV_THRESHOLD = 10;

VisualCortex* SanityChecks::cortex = 0;
unsigned char* SanityChecks::fired = 0;

/* Initialises the sanity checks module using the specified VisualCortex */
void SanityChecks::init(VisualCortex* vc) {
    SanityChecks::cortex = vc;
    fired = new unsigned char[INSANITY_COUNT/8 + 1];
}

/* Clears the record of all fired insanities */
void SanityChecks::clear() {
    memset(fired, 0, INSANITY_COUNT/8 + 1);
}

/* Sends fired insanities to Robolink */
void SanityChecks::sendInsanities() {
#ifndef OFFLINE
    Robolink::sendToBase(fired, RLNK_INSANITY, INSANITY_COUNT/8 + 1);
#endif
}

Insanity SanityChecks::checkBallSanity(VisualObject& vBall)  {
    (void)vBall; // silence warning when sanities disabled
#ifdef ENABLE_SANITIES
    /* Checks about the ball vob's position and size */
    if (!cortex->isBelowHorizon((int)vBall.cx, (int)vBall.cy,
                                BALL_OVER_HORIZON_LIMIT)) {
        return insane(BALL_ABOVE_HORIZON);
    }
    
    
    /* Checks for infra-red sensors. 
     * The robot may have recognised something near as a ball.
     * e.g. the red dog. */ 
    #if 0
    long sNear  = gInfoManager.getAnySensor(ssINFRARED_NEAR);
    // cout << __func__ << " : sNear " << sNear << endl;    
    if (sNear < BALL_NEAR_SENSOR) {
        return insane(BALL_INFRARED_OBSTACLE);
    }    
    #endif     

    /* Checks very large radius */ 
    if (vBall.radius > BALL_MAXIMUM_RADIUS) { 
        return insane(BALL_VERY_LARGE_RADIUS);
    }

    /* Checks about the robot's position */

 

    /* Checks for if there are not much field feature in the image, 
     * then ignore the ball.
     * However, the whole ball must be in the image. 
     */
    int ballRadius = (int)vBall.radius;
    //if (0 <= vBall.cx - ballRadius && vBall.cx + ballRadius <= CPLANE_WIDTH
    //    && 0 <= vBall.cy - ballRadius && vBall.cy + ballRadius <= CPLANE_HEIGHT
    //    && cortex->getFieldFeatureCount() < FIELD_FEATURE_THRESHOLD) {
    //    return insane(BALL_NOT_MUCH_GREEN_AROUND);
    //}

    /* Draws lines in an asterix shape at the centre of 
     * the circle and checks for the orangeness in the ball.
     */
    int totalCount = 0;
    int orangeCount = 0;
    int greenCount = 0; 
    int redCount = 0;
    int pinkCount = 0;
    int tightOrangeCount = 0;
    int tightRedCount = 0;
    int tightPinkCount = 0;
    int tightYellowCount = 0;
    if (ballRadius > BALL_MINIMUM_RADIUS) {
                
        vector<VisualFeature> features = cortex->subvision.getFeatures(); 
        list<int> ballFeatures = cortex->subobject.getBallFeatures();
        vector<int> curBallFeatures; 
        curBallFeatures.assign(ballFeatures.begin(),ballFeatures.end()); 
                
        srand(time(NULL));
        point finish((int)vBall.cx,(int)vBall.cy);
        while (!curBallFeatures.empty() && totalCount < 100) {
        
            int i = rand() % curBallFeatures.size(); 
            int index = curBallFeatures[i];
            curBallFeatures.erase(curBallFeatures.begin()+i);
            
            VisualFeature f = features[index]; 
            double d = DISTANCE(f.x,f.y,vBall.cx,vBall.cy); 
            
            if (d > ballRadius + 10) {
                continue;        
            }
             
            point start((int)f.x,(int)f.y); 
            point next = start;
            LineDraw::startLine(start,finish);            
            while (next != finish) { 
                next = LineDraw::nextPoint(); 
                if (next.first < 0 || next.first >= CPLANE_WIDTH
                    || next.second < 0 || next.second >= CPLANE_HEIGHT) {
                    continue;   
                }    
                // N.B. duplicate pixel access here could be optimised
                int colour = cortex->img.classify(next.first,next.second); 
                int tight = cortex->img.classifyTight(next.first,next.second); 
                
                if (colour == cBALL) {                  
                    orangeCount++;
                } else if (colour == cFIELD_GREEN) {
                    greenCount++;
                } else if (colour == cROBOT_RED) { 
                    redCount++;
                } else if (colour == cBEACON_PINK) { 
                    pinkCount++;
                }
                if (tight == cBALL) {
                    tightOrangeCount++;
                } else if (tight == cROBOT_RED) {
                    tightRedCount++;
                } else if (tight == cBEACON_PINK) {
                    tightPinkCount++;
                } else if (tight == cBEACON_YELLOW) { 
                    tightYellowCount++;
                }
                totalCount++;                              
                
                if (totalCount > 100) { 
                    break;
                }             
            }
        }
    } else { // ball smaller than MIN_BALL_RADIUS
        /* If the ball is too small for the star shape, we just fill a box */
        int minx = int(vBall.cx - (ballRadius)/2);
        int miny = int(vBall.cy - (ballRadius)/2);
        if (minx < 0) minx = 0;
        if (miny < 0) miny = 0;
        int maxx = minx + ballRadius;
        int maxy = miny + ballRadius;
        for (int x = minx; x < maxx; ++x) {
            if (x >= CPLANE_WIDTH)
                break;
            for (int y = miny; y < maxy; ++y) {
                if (y >= CPLANE_HEIGHT)
                    break;
                // duplicate pixel access could be optimised
                int colour = cortex->img.classify(x, y); 
                int tight = cortex->img.classifyTight(x, y); 
                
                if (colour == cBALL) {                  
                    orangeCount++;
                } else if (colour == cFIELD_GREEN) {
                    greenCount++;
                } else if (colour == cROBOT_RED) { 
                    redCount++;
                } else if (colour == cBEACON_PINK) { 
                    pinkCount++;
                }
                if (tight == cBALL) {
                    tightOrangeCount++;
                } else if (tight == cROBOT_RED) {
                    tightRedCount++;
                } else if (tight == cBEACON_PINK) {
                    tightPinkCount++;
                } else if (tight == cBEACON_YELLOW) { 
                    tightYellowCount++;
                }
                totalCount++;                              
            }
        }

    }
    #ifdef OFFLINE
    #ifdef SANITY_DEBUG
    cout << __func__ << " : ballRadius : " << ballRadius << endl;
    cout << __func__ << " : Orange Points Count : " << orangeCount 
        << " tight : " << tightOrangeCount << endl; 
    cout << __func__ << " : Red Points Count : " << redCount 
        << " tight : " << tightRedCount << endl;
    cout << __func__ << " : Pink Points Count : " << pinkCount 
        << " tight : " << tightPinkCount << endl;
    cout << __func__ << " : Green Points Count : " << greenCount << endl; 
    #endif
    #endif
    // If there is not much orange, or less orange than other colours
    // we discard the ball. In reality the last two tests don't work very
    // often but the tighter ones below do. In fact these ones might
    // rule out valid balls occasionally.
    if (ballRadius > BALL_MINIMUM_RADIUS && orangeCount <= 12) { 
        return insane(BALL_NOT_MUCH_ORANGE);
    } else if (orangeCount <= 3) {
        return insane(BALL_NOT_MUCH_ORANGE);
    }/* else if (orangeCount < redCount) { 
        return insane(BALL_TOO_RED);
    } else if (orangeCount < pinkCount) {
        return insane(BALL_TOO_PINK);
    }*/

    // If there is much less orange than other colours under tighter
    // classification we discard the ball. Note that we don't actually
    // require there to be any tight orange, just less than other colours
    //if (tightOrangeCount < tightRedCount/2) {
    //    return insane(BALL_TOO_RED);
    //} else
    if (tightOrangeCount < tightPinkCount/2) {
        return insane(BALL_TOO_PINK);
    } else if (tightOrangeCount < tightYellowCount) { 
        return insane(BALL_TOO_YELLOW);
    }
    
  
    // another possible test: if there is a decent amount of orange but only
    // a really small portion of it is also tight orange then it's not enough
    // (this would fire even when not enough pink/red is found to rule it out)
    

#endif
    return SANE;
}

Insanity SanityChecks::checkBeaconSanity(VisualObject& vBeacon)  {
    (void)vBeacon; // silence warning when sanities disabled
#ifdef ENABLE_SANITIES
    /* Checks about the beacon's position */
    if (cortex->isBelowHorizon(vBeacon.cx, vBeacon.cy,
                                -BEACON_BELOW_HORIZON_LIMIT)) {
        return insane(BEACON_BELOW_HORIZON);
    }
    
    
    
#endif
    return SANE;
}

Insanity SanityChecks::checkGoalSanity(VisualObject& vGoal)  {
    (void)vGoal; // silence warning when sanities disabled
#ifdef ENABLE_SANITIES
#ifdef OFFLINE
#ifdef GOAL_SANITY_DEBUG
    cout << __func__ << " w:" <<vGoal.width << " h:" << vGoal.height << endl;
#endif
#endif
    /* Checks about aspect ratio */
    double aspect = vGoal.width/vGoal.height;
    double desired = (double)GOAL_WIDTH/(double)GOAL_HEIGHT;
    
    // check the goal is not too wide, but only if it's not clipped at the
    // top of the image, as when grabbing
    if ((vGoal.y - vGoal.height/2) > 8 && (vGoal.height < MIN_GOAL_HEIGHT || 
            aspect/desired > GOAL_ASPECT_TOLERANCE)) {
        return insane(GOAL_TOO_WIDE);
    }
    
    // allow it to be not wide enough to the point of being square
    if (vGoal.height > vGoal.width) {
        return insane(GOAL_NOT_WIDE_ENOUGH);
    }

    /* Checks about the goal's position */
    if (!cortex->isBelowHorizon(vGoal.cx, vGoal.cy + vGoal.height/2,
                                GOAL_ABOVE_HORIZON_LIMIT)) {
        return insane(GOAL_ABOVE_HORIZON);
    }
    
    /* Checks whether goal is floating or not.
     * <del>This check is unnecessary when we are grabbing the ball, 
     * because we have very less chance of seeing false goal.</del>
     * It draws lines under the goal to check green points
     * Don't check every pixel, per 10 of goal width 
     */    
    //if (!cortex->isGrabbed) {
    

        int perXPixel = 12;
        int widthUnit = (int)vGoal.width / perXPixel;
        if (widthUnit == 0) {
            widthUnit = 1; //If it less than 10 pix, run all of them.
        }

        point rc = rotatePoint((int)vGoal.cx,(int)vGoal.cy,
                                CPLANE_WIDTH/2,CPLANE_HEIGHT/2,cortex->roll);
        int pointCount = 0;
        int consecutiveGreen = 0;
        int consecutiveWhite = 0;
        int MIN_PIXEL_GREEN = 2; //min pixel height consider as a green
        int MIN_PIXEL_WHITE = (int)vGoal.height; //min pixel height consider as not goal
        int startY = int(rc.second + vGoal.height/2);
        int endYGreen = int(rc.second + MAX(vGoal.height, vGoal.height/2 + 10));
        int endYWhite = int(rc.second + MAX(vGoal.height*2, vGoal.height/2 + 10));
        int index = 0;
        int endY = endYGreen;
        for (int i = int(rc.first - vGoal.width / 2);
                i < int(rc.first + vGoal.width / 2); 
                i += widthUnit, ++index) { 
            consecutiveGreen = 0;
            consecutiveWhite = 0;
    #ifdef OFFLINE
    #ifdef GOAL_SANITY_DEBUG
            cout << __func__ << " i:" << i<< endl;
    #endif
    #endif
            //maximum size of height in beacon looking from the dog standing average < 36 pixel.
            if (index % 3 == 1 && vGoal.height < 40) 
            {
                endY = endYWhite;
            }
            else
            {
                endY = endYGreen;
            }
            point rstart(i,startY);
            point rfinish(i,endY);


            // unrotating points...
            point start = rotatePoint(rstart.first,rstart.second,
                                        CPLANE_WIDTH/2,CPLANE_HEIGHT/2,-cortex->roll);
            point finish = rotatePoint(rfinish.first,rfinish.second,
                                        CPLANE_WIDTH/2,CPLANE_HEIGHT/2,-cortex->roll);

            point next = start;
            LineDraw::startLine(start,finish);

            while (next != finish) { 
                next = LineDraw::nextPoint();

                if (next.first < 0 || next.first >= CPLANE_WIDTH || next.second < 0 || next.second >= CPLANE_HEIGHT) {
                    continue;   
                }
                Color cpixel = cortex->img.classify(next.first,next.second);

                if (consecutiveGreen < MIN_PIXEL_GREEN && cpixel != cFIELD_GREEN)
                    consecutiveGreen = 0;
                else
                {
                    ++consecutiveGreen;
                }                

                if (cpixel != cWHITE && consecutiveWhite < MIN_PIXEL_WHITE) {
                    consecutiveWhite = 0;
                }
                else
                {
                    ++consecutiveWhite;
                }
                if (index % 3 != 1 && consecutiveGreen >= MIN_PIXEL_GREEN)
                    break;
    #ifdef OFFLINE
    #ifdef GOAL_SANITY_DEBUG
                cout << __func__ << " Point Comparing " << next.first << ", " << next.second << endl;
                cout << __func__ << " consecutiveGreen:" << consecutiveGreen << " consecutiveWhite:" << consecutiveWhite << endl;
    #endif
    #endif      
                //If there is a long white line
                if (consecutiveWhite >= MIN_PIXEL_WHITE)
                    break;            
            }    
            if (consecutiveGreen >= MIN_PIXEL_GREEN) 
            {
                ++pointCount;
            }
            if (consecutiveWhite >= MIN_PIXEL_WHITE) 
            {
                return insane(GOAL_IT_MIGHT_BE_BEACON);
            }
        } 
    #ifdef OFFLINE
    #ifdef GOAL_SANITY_DEBUG
        cout << __func__ << " pointCount:" << pointCount<< endl;
    #endif
    #endif

        if (pointCount <= 4) { 
            return insane(GOAL_FLOATING);    
        } 
    //}       
#endif
    return SANE;
}

void SanityChecks::checkWorldSanity(VisualObject vobs[], int nVobs) {
    (void)vobs, void(nVobs); // silence compiler warning when sanities disabled
#ifdef ENABLE_SANITIES
    VisualObject* vBall = &vobs[vobBall];
    VisualObject* vGoal = 0;
    VisualObject* vPinkOnBlue = &vobs[vobPinkBlueBeacon],
                * vBlueOnPink = &vobs[vobBluePinkBeacon],
                * vPinkOnYellow = &vobs[vobPinkYellowBeacon],
                * vYellowOnPink = &vobs[vobYellowPinkBeacon];

    /***** Checks that rule out a goal. *****/
    /* We resolve these first so that we haveinsane(TWO_GOALS_CHOSE_LOWEST_ELEV)
     * one valid goal against which to compare other objects
     */
    if (vobs[vobBlueGoal].cf > 0 && vobs[vobYellowGoal].cf > 0) { // two goals
        // Rule out vob with lower cf
        if (vobs[vobBlueGoal].cf > vobs[vobYellowGoal].cf) {
            vobs[vobYellowGoal].invalidate();
            vGoal = &vobs[vobBlueGoal];
            insane(TWO_GOALS_CHOSE_HIGHEST_CF);
        } else if (vobs[vobYellowGoal].cf > vobs[vobBlueGoal].cf) {
            vobs[vobBlueGoal].invalidate();
            vGoal = &vobs[vobYellowGoal];
            insane(TWO_GOALS_CHOSE_HIGHEST_CF);
        } else { // both the same cf, accept lowest elevation
            double blueElev = pointToElevation(
                    vobs[vobBlueGoal].cx, vobs[vobBlueGoal].cy,
                    cortex->sin_eroll, cortex->cos_eroll);
            double yellowElev = pointToElevation(
                    vobs[vobYellowGoal].cx, vobs[vobYellowGoal].cy,
                    cortex->sin_eroll, cortex->cos_eroll);
            if (blueElev > yellowElev) {
                vobs[vobBlueGoal].invalidate();
                vGoal = &vobs[vobYellowGoal];
                insane(TWO_GOALS_CHOSE_LOWEST_ELEV);
            } else {
                vobs[vobYellowGoal].invalidate();
                vGoal = &vobs[vobBlueGoal];
                insane(TWO_GOALS_CHOSE_LOWEST_ELEV);
            }
        }
    } else if (vobs[vobBlueGoal].cf > 0) {
        vGoal = &vobs[vobBlueGoal];
    } else if (vobs[vobYellowGoal].cf > 0) {
        vGoal = &vobs[vobYellowGoal];
    }
    
    // goal cannot be in beacons
    VisualObject* vBeacon;
    if (vGoal == &vobs[vobYellowGoal] && vobs[vobYellowPinkBeacon].cf > 0) {
        vBeacon = &vobs[vobYellowPinkBeacon];
        if (vGoal->cx > vBeacon->x
                && vGoal->cx < vBeacon->x + vBeacon->width
                && vGoal->cy > vBeacon->y
                && vGoal->cy < vBeacon->y + vBeacon->height) {
            vGoal->invalidate();
            vGoal = 0;
            insane(GOAL_IN_BEACON);
        }
    }
    if (vGoal == &vobs[vobYellowGoal] && vobs[vobPinkYellowBeacon].cf > 0) {
        vBeacon = &vobs[vobPinkYellowBeacon];
        if (vGoal->cx > vBeacon->x
                && vGoal->cx < vBeacon->x + vBeacon->width
                && vGoal->cy > vBeacon->y
                && vGoal->cy < vBeacon->y + vBeacon->height) {
            vGoal->invalidate();
            vGoal = 0;
            insane(GOAL_IN_BEACON);
        }
    }
    if (vGoal == &vobs[vobBlueGoal] && vobs[vobPinkBlueBeacon].cf > 0) {
        vBeacon = &vobs[vobPinkBlueBeacon];
        if (vGoal->cx > vBeacon->x
                && vGoal->cx < vBeacon->x + vBeacon->width
                && vGoal->cy > vBeacon->y
                && vGoal->cy < vBeacon->y + vBeacon->height) {
            vGoal->invalidate();
            vGoal = 0;
            insane(GOAL_IN_BEACON);
        }
    }
    if (vGoal == &vobs[vobBlueGoal] && vobs[vobBluePinkBeacon].cf > 0) {
        vBeacon = &vobs[vobBluePinkBeacon];
        if (vGoal->cx > vBeacon->x
                && vGoal->cx < vBeacon->x + vBeacon->width
                && vGoal->cy > vBeacon->y
                && vGoal->cy < vBeacon->y + vBeacon->height) {
            vGoal->invalidate();
            vGoal = 0;
            insane(GOAL_IN_BEACON);
        }
    }
    
    // beacons cannot be in a goal
    // a ball sometimes gets seen as pink. 
    if (vGoal == &vobs[vobYellowGoal] && vYellowOnPink->cf > 0) { 
        vBeacon = vYellowOnPink;
        if (vBeacon->cx > vGoal->x
                && vBeacon->cx < vGoal->x + vGoal->width
                && vBeacon->cy > vGoal->y
                && vBeacon->cy < vGoal->y + vGoal->height) {
            vBeacon->invalidate();
            vBeacon = 0;
            insane(BEACON_IN_GOAL);
        }       
    }
    if (vGoal == &vobs[vobYellowGoal] && vPinkOnYellow->cf > 0) { 
        vBeacon = vPinkOnYellow;
        if (vBeacon->cx > vGoal->x
                && vBeacon->cx < vGoal->x + vGoal->width
                && vBeacon->cy > vGoal->y
                && vBeacon->cy < vGoal->y + vGoal->height) {
            vBeacon->invalidate();
            vBeacon = 0;
            insane(BEACON_IN_GOAL);
        }               
    }
    if (vGoal == &vobs[vobBlueGoal] && vBlueOnPink->cf > 0) { 
        vBeacon = vBlueOnPink;
        if (vBeacon->cx > vGoal->x
                && vBeacon->cx < vGoal->x + vGoal->width
                && vBeacon->cy > vGoal->y
                && vBeacon->cy < vGoal->y + vGoal->height) {
            vBeacon->invalidate();
            vBeacon = 0;
            insane(BEACON_IN_GOAL);
        }           
    }
    if (vGoal == &vobs[vobBlueGoal] && vPinkOnBlue->cf > 0) { 
        vBeacon = vPinkOnBlue;
        if (vBeacon->cx > vGoal->x
                && vBeacon->cx < vGoal->x + vGoal->width
                && vBeacon->cy > vGoal->y
                && vBeacon->cy < vGoal->y + vGoal->height) {
            vBeacon->invalidate();
            vBeacon = 0;
            insane(BEACON_IN_GOAL);
        }           
    }
    
    
    // goals cannot be above beacons
    if (vGoal) { 
        double goalElev = pointToElevation(vGoal->cx,vGoal->cy,
                                           cortex->sin_eroll,cortex->cos_eroll);
        if (vYellowOnPink->cf > 0) {
            double beaconElev = pointToElevation(vYellowOnPink->cx,vYellowOnPink->cy,
                                                 cortex->sin_eroll,cortex->cos_eroll);
            if (goalElev > beaconElev + ELEV_THRESHOLD) {                                     
                vGoal->invalidate(); 
                vGoal = 0;
                insane(GOAL_ABOVE_BEACON);
            }
        }
        if (vPinkOnYellow->cf > 0) {
            double beaconElev = pointToElevation(vPinkOnYellow->cx,vPinkOnYellow->cy,
                                                 cortex->sin_eroll,cortex->cos_eroll);
            if (goalElev > beaconElev + ELEV_THRESHOLD) {                                     
                vGoal->invalidate(); 
                vGoal = 0;
                insane(GOAL_ABOVE_BEACON);
            }
        }
        if (vBlueOnPink->cf > 0) { 
            double beaconElev = pointToElevation(vBlueOnPink->cx,vBlueOnPink->cy,
                                                 cortex->sin_eroll,cortex->cos_eroll);
            if (goalElev > beaconElev + ELEV_THRESHOLD) {                                     
                vGoal->invalidate(); 
                vGoal = 0;
                insane(GOAL_ABOVE_BEACON);
            }
        }
        if (vPinkOnBlue->cf > 0) {
            double beaconElev = pointToElevation(vPinkOnBlue->cx,vPinkOnBlue->cy,
                                                 cortex->sin_eroll,cortex->cos_eroll);
            if (goalElev > beaconElev + ELEV_THRESHOLD) {                                     
                vGoal->invalidate(); 
                vGoal = 0;
                insane(GOAL_ABOVE_BEACON);
            }
        }
    }
    
    /***** Checks that rule out a beacon *****/
    // We can't see diagonally opposite beacons at the same time
    // FIXME: rather than ruling out both, can we reliably pick one?
    if (vobs[vobPinkBlueBeacon].cf > 0 && vobs[vobYellowPinkBeacon].cf > 0) {
        vPinkOnBlue->invalidate();
        vYellowOnPink->invalidate();
        insane(DIAGONALLY_OPPOSITE_BEACONS);
    }
    if (vobs[vobBluePinkBeacon].cf > 0 && vobs[vobPinkYellowBeacon].cf > 0) {
        vBlueOnPink->invalidate();
        vPinkOnYellow->invalidate();
        insane(DIAGONALLY_OPPOSITE_BEACONS);
    }
    // Beacons at the same end of the field can't be seen within 30 degress
    // of each other.
    if (vPinkOnBlue->cf > 0 && vBlueOnPink->cf > 0
            && fabs(vPinkOnBlue->head - vBlueOnPink->head) < 30) {
        if (vPinkOnBlue->cf > vBlueOnPink->cf)
            vBlueOnPink->invalidate();
        else if (vBlueOnPink->cf > vPinkOnBlue->cf)
            vPinkOnBlue->invalidate();
        else {
            vBlueOnPink->invalidate();
            vPinkOnBlue->invalidate();
        }
        insane(END_BEACONS_TOO_CLOSE);
    }
    if (vPinkOnYellow->cf > 0 && vYellowOnPink->cf > 0
            && fabs(vPinkOnYellow->head - vYellowOnPink->head) < 30) {
        if (vPinkOnYellow->cf > vYellowOnPink->cf)
            vYellowOnPink->invalidate();
        else if (vYellowOnPink->cf > vPinkOnYellow->cf)
            vPinkOnYellow->invalidate();
        else {
            vYellowOnPink->invalidate();
            vPinkOnYellow->invalidate();
        }
        insane(END_BEACONS_TOO_CLOSE);
    }
    

    /***** Checks that rule out the ball *****/
    if (vBall->cf > 0) { 

        //OSYSDEBUG(("CheckWorldSanity : checking ball and goal\n"));

        // ball cannot be above goal (temp). N.B. elev positive down  
        if (vGoal != 0 && vBall->elev > vGoal->elev + ELEV_THRESHOLD) {
            vBall->invalidate();
            insane(BALL_ABOVE_GOAL);
        }
        
        // ball cannot be above beacon 
        // TODO: should make this to consider the position of both ball and beacon.
        if (vobs[vobYellowPinkBeacon].cf > 0
            && vBall->elev > vobs[vobYellowPinkBeacon].elev) {
            vBall->invalidate();
            insane(BALL_ABOVE_BEACON);
        }
        if (vobs[vobPinkYellowBeacon].cf > 0
            && vBall->elev > vobs[vobPinkYellowBeacon].elev) {
            vBall->invalidate();
            insane(BALL_ABOVE_BEACON);                   
        }
        if (vobs[vobBluePinkBeacon].cf > 0
            && vBall->elev > vobs[vobBluePinkBeacon].elev) {
            vBall->invalidate();
            insane(BALL_ABOVE_BEACON);
        }
        if (vobs[vobPinkBlueBeacon].cf > 0
            && vBall->elev > vobs[vobPinkBlueBeacon].elev) {
            vBall->invalidate();
            insane(BALL_ABOVE_BEACON);
        }
    }
    
    
    // top of ball cannot be equal with or above the goal centroid
//    if (vGoal && pointToElevation(vBall->cx, vBall->y,
//                        cortex->sin_eroll, cortex->cos_eroll)
//            > pointToElevation(vGoal->cx, vGoal->cy,
//                        cortex->sin_eroll, cortex->cos_eroll)
//                + BALL_ABOVE_GOAL_MAX) {
//        cout << "top of ball elev: " << pointToElevation(vBall->cx, vBall->y,
//                                        cortex->s`in_eroll, cortex->cos_eroll)
//            << endl;
//        cout << "goal centroid elev: " << pointToElevation(vGoal->cx, vGoal->cy,
//                                        cortex->sin_eroll, cortex->cos_eroll)
//            << endl;

        //vBall->reset();
//        vBall->cf = 0; // Don't reset so we keep vob info for debugging
//        insane(BALL_ABOVE_GOAL);
//    }

#endif
    return;
}




// NB at present this is all commented code from the pre-2005 vision.
// Relevant sanity checks should be enabled when the vision is stable

/* Find robots of all colour from CPlane. */
#if 0
void VisualCortex::FindRobot() {


            // SANITY CHECK (FOR BALL) ----------------------------------------

            // A ball beside a red robot, if ball is very small compare to it, 
            // ball is probably a noise.
            if (ball2dogRatio < BALL2DOG_RATIO_THRESHOLD
                    && ball.ocx > dog.x - BALL_NEAR_DOG_THRESHOLD
                    && ball.ocx < ((dog.x+dog.width)  + BALL_NEAR_DOG_THRESHOLD)
                    && ball.ocy > dog.y - BALL_NEAR_DOG_THRESHOLD
                    && ball.ocy < ((dog.y+dog.height) + BALL_NEAR_DOG_THRESHOLD)
                ) {

                ball.reset();
                fireSanity(LAZY_SANITY_MESSAGE2);


            // SANITY CHECK (FOR BALL) ----------------------------------------

            // If the ball is inside a red dog, and not in the lower part of
            // it, it's probably some phantom orange of its body.
            } else if (ball.ox >= dogMinX
                    && ball.ox + ball.owidth <= dogMaxX
                    && ball.oy >= dogMinY
                    && ball.oy + ball.oheight <= dogMaxY) {

                // Check if ball is not around the lower part of the dog. 
                double ballElev        = PointToElevation(ball.ocx, ball.ocy, sin_eroll, cos_eroll);
                double dogCentroidElev = PointToElevation(dog.cx, dog.cy, sin_eroll, cos_eroll);
                double dogMinElev      = getMinElevation(dog);

                if (debugFindRobot && flag_OFFLINE) {
                    (*outfile) << debugHead 
                        << "    Checking ball inside robot..."
                        << endl;
                    (*outfile) 
                        << debugHead << "    ballElev: "   << ballElev << endl 
                        << debugHead << "    dogCenElev: " << dogCentroidElev << endl 
                        << debugHead << "    dogMinElev: " << dogMinElev << endl 
                        << endl;
                }


                if (ballElev > (dogCentroidElev + dogMinElev)/2) {
                    ball.reset();
                    fireSanity(LAZY_SANITY_MESSAGE2);

                    if (debugFindRobot && flag_OFFLINE) {
                        (*outfile) 
                            << debugHead << "...ball reset because it is totally inside the bounding box" 
                                << " and not lower than the dog."
                                << endl;
                    }
                }
            }
        }
}




   /* If the dog is looking really down or really up, there should not be 
      any goal. */
   if (!hzExists && goalIndex == vobYellowGoal) {
     
     if (debugLookForColourGoal && flag_OFFLINE) {
       (*outfile) << debugHead 
		  << "... looking straight down, no yellow goal could be observed!"
		  << endl;
     }
     return;
   }

     // SANITY CHECK ------------------------------------------------------

     /* If the blob is too small, forget it. */
     if (goalBlob.area < 30) {
       
       if (debugLookForColourGoal && flag_OFFLINE) {
	 (*outfile) << debugHead << "... area under 30 -> filtered" << endl;
       }
       
       fireSanity(BLOB_AREA_LT30);
       continue;
     }

     
     // SANITY CHECK ------------------------------------------------------
     
     /* Check if goal is too much above horizon. Compare using the lower 
      * edge so that it won't be affected by whether it's far/near. */

     // SANITY CHECK ------------------------------------------------------
     /* Check elevation and heading comparing to beacons. */
     
     // SANITY CHECK ------------------------------------------------------
     
     // A goal blob cannot be to the left of the left goal beacon
     // so we conclude it cannot be the goal
     
     // SANITY CHECK ------------------------------------------------------
     
     // A goal blob cannot be to the right of the right beacon
     // so we conclude it cannot be the goal
     
    // SANITY CHECK ------------------------------------------------------
    /* Take out goals of extreme bounding box ratio. */

    // SANITY CHECK ------------------------------------------------------

	/* A goal with not enough green below - throw away. */

    // SANITY CHECK ----------------------------------------------------------
	// Check elev of ball against that of goals.
    //
    // SANITY CHECK ----------------------------------------------------------
	// Check elev of ball against that of beacons.
    // beacon vob IDs starts from left top to right bottom.

    // SANITY CHECK ----------------------------------------------------------

    // too far away outside the field, cant be the ball
    if (ball.dist3 > FIELD_DIAGONAL) {
        fireSanity(BALL_OUTSIDE_FIELD);

        if (debugBallSanities && flag_OFFLINE && !flag_PRE_SANITY_BLOB) {
            (*outfile) << debugHead
                 << "...ball is too far - outside field distance." << endl;
        }
        return false;
    }

    // SANITY CHECK ----------------------------------------------------------
	// If yellow goal is inside ball, reject the goal.

        /* Quick check - can't be a ball if not occupying enough orange. */

        /* Check ----------------------------------------------------*/

        /* Check if any corner of the ball is higher than the horizon. */
        int BALL_OVER_HORIZON_THRESHOLD = 30; 

        /* For very large ball, the horizon may be just touching the ball
        ** edge, so relax the threshold a bit. */
        if (blobArea > 8000) {
            BALL_OVER_HORIZON_THRESHOLD = 50; 
        }

            /* Check ------------------------------------------------------- */
            double maxDimension = MAX(preball->getXMax() - preball->getXMin(),
                                    preball->getYMax() - preball->getYMin());

            /* If the radius of FireBall found is shorter than the real 
             * dimension of orange blob, then something is wrong, most likely 
             * the edges of ball were detected falsely due to noise around ball. 
             * */

            // Check whether the ball covers the entire screen.
            // however cant check for full orange because when ball close 
            // enough there'll be alot of red as well on the other hand, it's 
            // possible that you are looking straight into red dog, 
            // so if ball is reasonable size, but rest of screen is covered 
            // by red dog, then fire.

            // WARNING: 21000 is the constant for old dogs.
            // Using the same ratio (21000/176*144), new camera should 
            // use 27575.

            /* Check ------------------------------------------------------- */

            /* Now already said no fireball, but check again if the ball 
             * actually covered the entire screen, foundBall=true again if yes. */
            static const int BALL_ON_ENTIRE_SCREEN_CONST = 27575;

            if (totalOrangeArea + totalRedArea >= BALL_ON_ENTIRE_SCREEN_CONST && !fbFound) {

            /* Check ----------------------------------------------------*/

            // If ball is small but vision says it's very close then likely it 
            // is reflection from looking down at other robot
            if (currentRDist < 50 && ball.area < 50 && ball.radius <= 5) {

    /* CHECK ----------------------------------------------------------------*/
    /* Check yellow goal elevation. */
    if (vob[vobYellowGoal].cf && vob[vobYellowGoal].width < WIDTH - 5 
            && vob[vobYellowGoal].elev > 25.0) {
        vob[vobYellowGoal].reset();

        if (debugGoalSanities && flag_OFFLINE && !flag_PRE_SANITY_BLOB) {
            (*outfile) << debugHead
                << "...Yellow goal too high, sanitied out." << endl;
        }
        fireSanity(YELLOW_GOAL_TOO_HIGH);
    }

    /* CHECK ----------------------------------------------------------------*/
    /* Check blue goal elevation. */
    if (vob[vobBlueGoal].cf && vob[vobBlueGoal].width < WIDTH - 5 && vob[vobBlueGoal].elev> 25.0) {
        vob[vobBlueGoal].reset();

        if (debugGoalSanities && flag_OFFLINE && !flag_PRE_SANITY_BLOB) {
            (*outfile) << debugHead
                << "...blue goal too high, sanitied out" << endl;
        }
        fireSanity(BLUE_GOAL_TOO_HIGH);
    }

    /* CHECK ----------------------------------------------------------------*/
	// We can't see both goals at the same time.

    /*---------------------------------------------------*/
    /* If there're 3 beacons, one definitely is insane. */

        /*---------------------------------------------------*/
        /* The right beacon appears in the left of left beacon? */

        /*---------------------------------------------------*/
        /* One beacon's elevation is much higher than the other? */

        /*---------------------------------------------------*/
        /* Two beacons are too close? */
        if (beaconBL->cf > 0 && beaconBR->cf > 0
                && abs(beaconBL->h - beaconBR->h) < 35) {
            throwAwayOneBeacon(beaconBL, beaconBR);

        /*---------------------------------------------------*/
        /* Their size differ a lot? */
        if (beaconBL->cf > 0 && beaconBR->cf > 0) {

            double areaBL = beaconBL->width * beaconBL->height; 
            double areaBR = beaconBR->width * beaconBR->height; 

            if (abs(areaBL-areaBR) > 500) {

/*********************************************************/
/* Check if the beacon has enough white colour below it. */

    // SANITY CHECK 1 ----------------------------------------------------------

    /* If the dog is looking really down, there should not be any beacon. */
    if (!hzExists && !hzAllAboveHorizon) {

    // SANITY CHECK 2 ----------------------------------------------------------

    // Check if the pink blob is too high above horizon (80 pixels buffer).
    if (hzExists && !isBelowHorizon((int)pink->cx, (int)pink->cy, 80, false)) {

    // SANITY CHECK 3 ----------------------------------------------------------

    // Check if the pink blob is too low below horizon (80 pixels buffer).

    // SANITY CHECK 5 ------------------------------------------------------

    // Check if pink solid ratio is ok.

        // Check if the beacon is vertical to the horizon.

        // SANITY CHECK 22 ------------------------------------------------------

        /* Check if the elevation and distance make sense together. */

        // New height check using elevation angle from camera. 
        // Very relaxed thershold because of bodyTilt error.




        

#endif
