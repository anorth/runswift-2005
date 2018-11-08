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


/* FieldLineVision.cc
 *
 * The field line vision and localisation is part of VisualCortex since it
 * relies on edge points detected by SubVision, but is implemented here
 * as a logically separate module to reduce clutter in VisualCortex.
 */

//#define FLV_DEBUG
#include "VisualCortex.h"
#ifdef OFFLINE
#include <sstream>
//#define TABLE_MATCH_DEBUG
//#define calcEdgeWeights_DEBUG
#endif
/* Initialisation is called by the constructor of VisualCortex */
void VisualCortex::initEdgeDetect() {
    matchLineTable = new matchValue[ARRAY_WIDTH * ARRAY_HEIGHT];
    matchFieldTable = new matchValue[ARRAY_WIDTH * ARRAY_HEIGHT];
    bodyMoving = true;
    headMovement = 0.0;
    mapInvalid = false;
    numEdgePnts = 0;
    numMapPnts = 0;
    numEdgeLinePnts = 0;
    numEdgeFieldPnts = 0;
    initGradField();
}

// This is called by the destructor of VisualCortex
void VisualCortex::shutdownEdgeDetect() {
    delete[] matchLineTable;
    delete[] matchFieldTable;
}

/*********************************************************************/
/* Initialise the match functions (line & field), for gradient ascent. This is
 * a probability map of observing a field line edge, i.e. a white/green 
 * transition.
 */
void VisualCortex::initGradField() {

    double SPREAD = 0.0025;//0.004; // small values => very spread out
    double dist, cur_dist, cur_angle, angle, x, y;
    int index;

    /*
     * Bottom left of the field
     *         
     *        *2*
     *   _____________
     *  |          \__
     *  |           *6* 
     *  |
     *  |*1*
     *  |         *4* 
     *  |        _____
     *  |    *5*|     
     *  |       |
     *  |_______|_____
     *      *3* 
     *          
     */
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
    ofstream fp_out;
    fp_out.open("DebugTableMatch.txt", ios::out);
#endif
#endif
     
    for (int yInd = -offsetY; yInd <= FIELD_LENGTH / 4; yInd++) {
        for (int xInd = -offsetX; xInd <= FIELD_WIDTH / 4; xInd++) {
            index = (yInd + offsetY) * ARRAY_WIDTH + (xInd + offsetX);
            x = static_cast<double>(2 * xInd);
            y = static_cast<double>(2 * yInd);
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
            fp_out << __func__ << " x:" << x <<" y:" << y << endl;
            stringstream ss;
#endif            
#endif            
            /*************
             *           *
             * LINE EDGE *
             *           *
             *************/

            dist = VERY_LARGE_INT;
            angle = VERY_LARGE_INT;
            // line 1 - sideline
            if (y < 0) {
                cur_dist = DISTANCE_SQR(x, y, 0.0, -LINE_WIDTH);
                cur_angle = atan2(y+LINE_WIDTH, x-0.0) + M_PI;
            } else {
                if (SQUARE(x + LINE_WIDTH) < SQUARE(x - LINE_WIDTH)) {
                    cur_dist = SQUARE(x + LINE_WIDTH);
                    cur_angle = 0.0; //Facing Right
                } else {
                    cur_dist = SQUARE(x - LINE_WIDTH);
                    cur_angle = M_PI; //Facing Left
                }
            }

            if (cur_dist < dist) {
                dist = cur_dist;
                angle = cur_angle;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
                ss.str("");
                ss << __func__ << " Sideline distance:" << dist << " angle:" << RAD2DEG(angle);
#endif            
#endif            
            }
            
            // line 2 - mid-line
            if (x < 0)
            {
                //cur_dist = DISTANCE_SQR(x, y, -LINE_WIDTH, FIELD_LENGTH/2.0);
                cur_dist = VERY_LARGE_INT;
                //cur_angle = ....;
            } 
            else
            {
                if (SQUARE(y - FIELD_LENGTH/2.0 - LINE_WIDTH) < SQUARE(y - FIELD_LENGTH/2.0 + LINE_WIDTH)) 
                {
                    cur_dist = SQUARE(y - FIELD_LENGTH/2.0 - LINE_WIDTH);
                    cur_angle = M_PI + M_PI / 2; //Facing Down
                }
                else
                {
                    cur_dist = SQUARE(y - FIELD_LENGTH/2.0 + LINE_WIDTH);
                    cur_angle = M_PI/2; //Facing Up
                }
            }           

            if (cur_dist < dist) {
                dist = cur_dist;
                angle = cur_angle;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
                ss.str("");
                ss << __func__ << " mid-line distance:" << dist << " angle:" << RAD2DEG(angle);
#endif            
#endif            
            }

            // line 3 - goal line horizontal
            if (x < 0)
            {
                cur_dist = DISTANCE_SQR(x, y, -LINE_WIDTH, 0.0);
                cur_angle = atan2(y-0.0, x+LINE_WIDTH) + M_PI;
            } 
            else
            {
                if (SQUARE(y - LINE_WIDTH) < SQUARE(y + LINE_WIDTH)) 
                {
                    cur_dist = SQUARE(y - LINE_WIDTH);
                    cur_angle = M_PI + M_PI / 2; //Facing Down
                }
                else
                {
                    cur_dist = SQUARE(y + LINE_WIDTH);
                    cur_angle = M_PI/2; //Facing Up
                }
            }          
            //cur_dist *= 4; // goal line is tighter. Do we want this?
            if (cur_dist < dist) {
                dist = cur_dist;
                angle = cur_angle;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
                ss.str("");
                ss << __func__ << " goal line distance:" << dist << " angle:" << RAD2DEG(angle);
#endif                            
#endif            
            }

            // line 4 - goalbox line horizontal
            if (x < FIELD_WIDTH/2.0 - GOALBOX_WIDTH/2.0)
            {
                cur_dist = DISTANCE_SQR(x, y,
                            FIELD_WIDTH/2.0 - GOALBOX_WIDTH/2.0 - LINE_WIDTH,
                            (double)GOALBOX_DEPTH);
                cur_angle = atan2(y-(double)GOALBOX_DEPTH, x-FIELD_WIDTH/2.0 - GOALBOX_WIDTH/2.0 - LINE_WIDTH) + M_PI;
            } 
            else
            {
                if (SQUARE(y - GOALBOX_DEPTH - LINE_WIDTH) < SQUARE(y - GOALBOX_DEPTH + LINE_WIDTH)) 
                {
                    cur_dist = SQUARE(y - GOALBOX_DEPTH - LINE_WIDTH);
                    cur_angle = M_PI + M_PI / 2; //Facing Down
                }
                else
                {
                    cur_dist = SQUARE(y - GOALBOX_DEPTH + LINE_WIDTH);
                    cur_angle = M_PI/2; //Facing Up
                }
            
            }
            
            if (cur_dist < dist) {
                dist = cur_dist;
                angle = cur_angle;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
                ss.str("");
                ss << __func__ << " goal box line horizontal distance:" << dist << " angle:" << RAD2DEG(angle);
#endif            
#endif                            
            }

 
            // line 5 - goalbox line vertical
            if (y < 0)    
            {        
                cur_dist = DISTANCE_SQR(x, y,
                        FIELD_WIDTH/2.0 - GOALBOX_WIDTH/2.0, -LINE_WIDTH);
                cur_angle = atan2(y+LINE_WIDTH, x-FIELD_WIDTH/2.0 - GOALBOX_WIDTH/2.0) + M_PI;
            }
            else if (y > GOALBOX_DEPTH)
            {
                cur_dist = DISTANCE_SQR(x, y,
                                        FIELD_WIDTH/2.0 - GOALBOX_WIDTH/2.0,
                                        (double)GOALBOX_DEPTH + LINE_WIDTH);
                cur_angle = atan2(y+(double)GOALBOX_DEPTH + LINE_WIDTH, x-FIELD_WIDTH/2.0 - GOALBOX_WIDTH/2.0) + M_PI;
            }
            else 
            {
                if (SQUARE(FIELD_WIDTH/2.0 - GOALBOX_WIDTH/2.0
                                        - LINE_WIDTH - x) < SQUARE(FIELD_WIDTH/2.0 - GOALBOX_WIDTH/2.0
                                        + LINE_WIDTH - x)) 
                {
                    cur_dist = SQUARE(FIELD_WIDTH/2.0 - GOALBOX_WIDTH/2.0
                                        - LINE_WIDTH - x);
                    cur_angle = 0.0;
                }
                else
                {
                    cur_dist = SQUARE(FIELD_WIDTH/2.0 - GOALBOX_WIDTH/2.0
                                        + LINE_WIDTH - x);
                    cur_angle = M_PI;
                }
            }
                                                    
            if (cur_dist < dist) {
                dist = cur_dist;
                angle = cur_angle;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
                ss.str("");
                ss << __func__ << " goal box line vertical distance:" << dist << " angle:" << RAD2DEG(angle);
#endif            
#endif                            
            }


            // line 6 - quarter circle
            cur_dist = sqrt(DISTANCE_SQR(x, y, FIELD_WIDTH/2.0,
                                        FIELD_LENGTH/2.0));
            if (SQUARE(cur_dist - CIRCLE_DIAMETER/2.0 - LINE_WIDTH) < SQUARE(cur_dist - CIRCLE_DIAMETER/2.0 + LINE_WIDTH))
            {
                cur_angle = atan2(y - FIELD_LENGTH/2.0, x - FIELD_WIDTH/2.0) + M_PI;
                cur_dist = SQUARE(cur_dist - CIRCLE_DIAMETER/2.0 - LINE_WIDTH);
            }
            else
            {
                cur_angle = atan2(FIELD_LENGTH/2.0 - y, FIELD_WIDTH/2.0 - x) + M_PI;
                cur_dist = SQUARE(cur_dist - CIRCLE_DIAMETER/2.0 + LINE_WIDTH);            
            }
            cur_dist *= 4; // quarter circle needs tighter fit
          
            if (cur_dist < dist) {
                dist = cur_dist;
                angle = cur_angle;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
                ss.str("");
                ss << __func__ << " quarter circle distance:" << dist << " angle:" << RAD2DEG(angle);
#endif                            
#endif            
            }
            
            // the fringe. This assumes there is a white border at the outside
            // of the fringe
            if (y > -GOAL_FRINGE_WIDTH)
            {
                cur_dist = SQUARE(x - (-SIDE_FRINGE_WIDTH));
                if (x < -SIDE_FRINGE_WIDTH) 
                {
                    cur_angle = 0;
                } 
                else
                {
                    cur_angle = M_PI;
                }
            }
            else
            {
                cur_dist = DISTANCE_SQR(x, y, (double)-SIDE_FRINGE_WIDTH,
                                        (double)-GOAL_FRINGE_WIDTH);
                cur_angle = atan2(y+(double)GOAL_FRINGE_WIDTH, x+(double)SIDE_FRINGE_WIDTH) + M_PI; 
            }                            
            
            if (cur_dist < dist) {
                dist = cur_dist;
                angle = cur_angle;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
                ss.str("");
                ss << __func__ << " SIDE_FRINGE_WIDTH distance:" << dist << " angle:" << RAD2DEG(angle);
#endif            
#endif                            
            }
            
            if (x > -SIDE_FRINGE_WIDTH)
            {
                cur_dist = SQUARE(y - (-GOAL_FRINGE_WIDTH));
                if (y < -GOAL_FRINGE_WIDTH) 
                {        
                    cur_angle = M_PI /2;
                } 
                else
                {
                    cur_angle = M_PI + M_PI/2;
                }
            }
            
            if (cur_dist < dist) {
                dist = cur_dist;
                angle = cur_angle;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
                ss.str("");
                ss << __func__ << " GOAL_FRINGE_WIDTH distance:" << dist << " angle:" << RAD2DEG(angle);
#endif                            
#endif                            
            }

#if 1 //OSAKA FIELD SETTING
            // WALL outside the field. This is for the wall far outside the field.
            if (y > -GOAL_WALL_FRINGE_WIDTH)
            {
                cur_dist = SQUARE(x - (-SIDE_WALL_FRINGE_WIDTH));
                if (x < -SIDE_WALL_FRINGE_WIDTH) 
                {
                    cur_angle = 0;
                } 
                else
                {
                    cur_angle = M_PI;
                }
            }
            else
            {
                cur_dist = DISTANCE_SQR(x, y, (double)-SIDE_WALL_FRINGE_WIDTH,
                                        (double)-GOAL_WALL_FRINGE_WIDTH);
                cur_angle = atan2(y+(double)GOAL_WALL_FRINGE_WIDTH, x+(double)SIDE_WALL_FRINGE_WIDTH) + M_PI; 
            }                            
            
            if (cur_dist < dist) {
                dist = cur_dist;
                angle = cur_angle;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
                ss.str("");
                ss << __func__ << " SIDE_WALL_FRINGE_WIDTH distance:" << dist << " angle:" << RAD2DEG(angle);
#endif            
#endif                            
            }
            
            if (x > -SIDE_WALL_FRINGE_WIDTH)
            {
                cur_dist = SQUARE(y - (-GOAL_WALL_FRINGE_WIDTH));
                if (y < -GOAL_WALL_FRINGE_WIDTH) 
                {        
                    cur_angle = M_PI /2;
                } 
                else
                {
                    cur_angle = M_PI + M_PI/2;
                }
            }
            
            if (cur_dist < dist) {
                dist = cur_dist;
                angle = cur_angle;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
                ss.str("");
                ss << __func__ << " GOAL_WALL_FRINGE_WIDTH distance:" << dist << " angle:" << RAD2DEG(angle);
#endif                            
#endif                            
            }
                        
#endif
                        
            // the base of the beacon
            cur_dist = DISTANCE_SQR(x, y, (double)BEACON_LEFT_X, 
                                        (double)BEACON_CLOSE_Y);
            cur_dist -= BEACON_RADIUS;
            cur_angle = atan2(y-BEACON_CLOSE_Y, x-BEACON_LEFT_X) + M_PI;

            if (cur_dist < 0) cur_dist = -cur_dist;
            if (cur_dist < dist) {
                dist = cur_dist;
                angle = cur_angle;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
                ss.str("");
                ss << __func__ << " Beacon distance:" << dist << " angle:" << RAD2DEG(angle);
#endif                
#endif            
            }

            matchLineTable[index].match = (float)(1.0 / (1.0 + SPREAD * dist));
            double df = SPREAD  * 2 * sqrt(dist) * (matchLineTable[index].match*matchLineTable[index].match);
            //if (df < 0.0) df = 0.0;
            matchLineTable[index].dx = cos(angle) * df;
            matchLineTable[index].dy = sin(angle) * df;
#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
            fp_out << ss.str() << endl;
            fp_out << __func__ << " match:" << matchLineTable[index].match << " dx:" << matchLineTable[index].dx << " dy:" << matchLineTable[index].dy << endl;
#endif            
#endif                

            /*********
             *       *
             * FIELD *
             *       *
             *********/
            
            /*** Alexn: The 2005 field is now just entirely high probability
             * since there is no field wall. The distance of any outside wall
             * is undefined. For now, I'll put in a border (the fence)
             * at the edge of the 600x400 carpet, but this is not garunteed
             */

          
            /* This is for the fringe area outside the field boundary.
             * This is UNDEFINED but I'll put it in here as not
             * green beyond the 600x400 defined carpet
             */
            dist = 0.0;
            if (x < -SIDE_FRINGE_WIDTH || y < -GOAL_FRINGE_WIDTH) {
                if (x < -SIDE_FRINGE_WIDTH && y < -GOAL_FRINGE_WIDTH)
                {
                    cur_dist = DISTANCE_SQR(x, y,
                                            (double)-SIDE_FRINGE_WIDTH,
                                            (double)-GOAL_FRINGE_WIDTH);
                    cur_angle = atan2(y + (double)GOAL_FRINGE_WIDTH, x + (double)SIDE_FRINGE_WIDTH)+ + M_PI;
                }
                else if (x < -SIDE_FRINGE_WIDTH)
                {
                    cur_dist = min(SQUARE((double)x + SIDE_FRINGE_WIDTH),
                                    cur_dist);
                    cur_angle = M_PI/2;
                }    
                else
                {
                    cur_dist = min(SQUARE((double)y + GOAL_FRINGE_WIDTH),
                                    cur_dist);
                    cur_angle = 0.0;
                }
            } else {
                cur_dist = 0.0;
            }
            
            dist = max(cur_dist, dist);
                    
            
            matchFieldTable[index].match = (float)(1.0 / (1.0 + SPREAD * 0.5 * dist));
            df = SPREAD  * sqrt(dist) * (matchLineTable[index].match*matchLineTable[index].match);
            if (dist > 0.0) {
                matchFieldTable[index].dx = cos(angle) * df;
                matchFieldTable[index].dy = sin(angle) * df;
            } else
            {
                matchFieldTable[index].dx = 0.0;
                matchFieldTable[index].dy = 0.0;
            }
        }
    }
    
    mirrorGradient(matchLineTable);
    mirrorGradient(matchFieldTable);
    
#if 0
    // Derrick: for debugging - this outputs match tables to file (use with
    // offvision). Produces .raw files in offvision directory after process is
    // run. Image sizes are width 346 height 421 (as of 30/6/2004). Use rawtopgm
    // and then ppmtotga / ppmtobmp / ppmtojpeg on each file to view in an
    // image viewer. eg. At shell, type:
    // > rawtopgm 381 471 LineMap.Raw > LineMap.pgm
    // > ppmtotga LineMap.pgm > LineMap.tga
    // or use ImageMagick
    // > convert -depth 8 -size 381x471 gray:FieldMap.Raw FieldMap.jpg
    if (flag_OFFLINE) {
        uchar t;    
        ofstream fout;
    
        fout.open("LineMap.Raw", ios::out);        
        for (int i = 0; i < ARRAY_WIDTH * ARRAY_HEIGHT; i++) {
            t = (uchar)(255 * matchLine[i]);
            fout << t;
        }    
        fout.close();
    
        fout.open("FieldMap.Raw", ios::out);
        for (int i = 0; i < ARRAY_WIDTH * ARRAY_HEIGHT; i++) {
            t = (uchar)(255 * matchField[i]);
            fout << t;
        }
        fout.close();
    }
#endif // 0

#ifdef OFFLINE
#ifdef TABLE_MATCH_DEBUG
    fp_out.close();
#endif
#endif

}

/* Copies mirror gradient info from bot-left? corner of match info
 * to other quadrants
 */
void VisualCortex::mirrorGradient(matchValue *gradient)
{
    int x, y;
    float val, dx, dy;
    
    for (y = 0; y <= ARRAY_HEIGHT / 2; y++) {
        for (x = 0; x <= ARRAY_WIDTH / 2; x++) {

            val = gradient[y * ARRAY_WIDTH + x].match;
            dx = gradient[y * ARRAY_WIDTH + x].dx;
            dy = gradient[y * ARRAY_WIDTH + x].dy;
            
            // top left corner
            gradient[(ARRAY_HEIGHT - 1 - y) * ARRAY_WIDTH + x].match = val;
            gradient[(ARRAY_HEIGHT - 1 - y) * ARRAY_WIDTH + x].dx = dx;
            gradient[(ARRAY_HEIGHT - 1 - y) * ARRAY_WIDTH + x].dy = -dy;
            
            // top right corner
            gradient[(ARRAY_HEIGHT - 1 - y) * ARRAY_WIDTH + (ARRAY_WIDTH - 1 - x)].match = val;
            gradient[(ARRAY_HEIGHT - 1 - y) * ARRAY_WIDTH + (ARRAY_WIDTH - 1 - x)].dx = -dx;
            gradient[(ARRAY_HEIGHT - 1 - y) * ARRAY_WIDTH + (ARRAY_WIDTH - 1 - x)].dy = -dy;

            // bottom left corner
            gradient[y * ARRAY_WIDTH + (ARRAY_WIDTH - 1 - x)].match = val;
            gradient[y * ARRAY_WIDTH + (ARRAY_WIDTH - 1 - x)].dx = -dx;
            gradient[y * ARRAY_WIDTH + (ARRAY_WIDTH - 1 - x)].dy = dy;
        }
    }
}

/*****************************************************************************/
/* Run Edge detection. Called by ProcessImage.                               */ 
#ifdef OFFLINE
void VisualCortex::runEdgeDetect(double reduceDist, double cullDist, int minPoints, double maxheadspeed, double minFieldDist) {
#else
void VisualCortex::runEdgeDetect() {
    static const double maxheadspeed = 0.09;
#endif
    numEdgePnts = 0;
/*
#ifdef OFFLINE
    cout << __func__ << " headMovement:" <<  headMovement << endl;
#endif
*/
    // Don't look for edge pixels if the head is moving too fast, since fast
    // head rotations can cause image distortion, such as straight lines
    // looking curved.
    if (headMovement < maxheadspeed) {
        // Edge points pixels will be stored in the edgeList and number of
        //  points will be returned.
        numEdgePnts = findFieldEdges();
        for (int i = 0; i < numEdgePnts; i ++) {
            cplaneEdgePnts[i*2] = (int)edgeList[i*2]
                    - CPLANE_WIDTH / 2;
            cplaneEdgePnts[i*2 + 1] = -((int)edgeList[i*2 + 1]
                    - CPLANE_HEIGHT / 2);
            edgeWeights[i] = 0.0;
        }

        // Project the edge pixels into the ground plane.
        projectPoints(cplaneEdgePnts, numEdgePnts, localEdgePnts, groundPlane);
#ifdef OFFLINE
        calcEdgeWeights(reduceDist,cullDist,minPoints, minFieldDist);
#else
        calcEdgeWeights();
#endif
    }
    
    // If we're not moving, keep track of old points
//#ifndef LOCALISATION_CHALLENGE    
    stationaryMap();    
//#endif    
/*    
#ifdef OFFLINE
    cout << __func__ << " numEdgePnts:" <<  numEdgePnts<< endl;    
#endif    
*/
}

/*****************************************************************************/
/* Calculates the match value and gradient, given a robot position & heading.*/
/* Note: Outputs gradient to 'gradient', returns match value.                */
double VisualCortex::calcRobotMatch(MVec3 &robot, MVec3 &gradient)
{
    double totalLineMatch, totalFieldMatch;
    double localGX, localGY, dX, dY, match;
    MVec3 lineGrad, fieldGrad, grad;

#ifdef OFFLINE
#ifdef TILT_CALIBRATION 
    // *** Used when fitting joint slop
    runEdgeDetect(SQUARE(70.0), SQUARE(200.0), 10, 1.0, 25.0);
#endif    
#endif
    
    localToGlobal(robot(0,0),robot(1,0),robot(2,0), numEdgePnts, localEdgePnts, globalEdgePnts);

    lineGrad(0, 0) = 0.0; fieldGrad(0, 0) = 0.0;
    lineGrad(1, 0) = 0.0; fieldGrad(1, 0) = 0.0;
    lineGrad(2, 0) = 0.0; fieldGrad(2, 0) = 0.0;
    totalLineMatch = 0.0; totalFieldMatch = 0.0;
    localGX = 0.0; localGY = 0.0;
    for (int c = 0; c < numEdgePnts; c++) {
        match = calcPointMatch(globalEdgePnts[c*2], globalEdgePnts[c*2 + 1],
                        edgePntTypes[c], &localGX, &localGY);
        match *= edgeWeights[c];
        localGX *= edgeWeights[c];
        localGY *= edgeWeights[c];
        
        dX = globalEdgePnts[c*2] - robot(0, 0);
        dY = globalEdgePnts[c*2+1] - robot(1, 0);

        grad(0, 0) = localGX;
        grad(1, 0) = localGY;
        grad(2, 0) = dX * localGY - dY * localGX;
        
        if (edgePntTypes[c] == FIELD) {
            totalFieldMatch += match;
            fieldGrad += grad;
        } else {
            totalLineMatch += match;
            lineGrad += grad;
        }
                                   
    }
   
    if (totalLineWeight > 0.0) {
        totalLineMatch /= totalLineWeight;
        lineGrad /= totalLineWeight;
    } else totalLineMatch = 1.0;

    if (totalFieldWeight > 0.0) {
        totalFieldMatch /= totalFieldWeight;
        fieldGrad /= totalFieldWeight;
    } else totalFieldMatch = 1.0;
    
    // The actual match value is the product of the field and line match values
    match = totalLineMatch * totalFieldMatch;
    fieldGrad *= totalLineMatch;
    lineGrad *= totalFieldMatch;
    gradient = fieldGrad;
    gradient += lineGrad;
    
#ifdef OFFLINE
    if (outputStream != NULL)
    {
        *outputStream << __func__ << " robot x:" << robot(0,0) << " y:"<< robot(1,0) << " h:"<< RAD2DEG(robot(2,0)) << " match:" << match << endl;
    }
#endif 
    
    return match;
}

/*****************************************************************************/
/* Calculates the match value and local gradient of a single point           */
/*  (specified by x, y, type) on the field, outputs gradient to gX, gY,      */ 
/*  returns match value.                                                     */
/*inline */double VisualCortex::calcPointMatch(double inX, double inY, int type,
                                           double *gX, double *gY)
{
    // OPTIMISE ME: this gets called a lot and takes too long
    double x, y, match;
    int iX, iY;
    int index;
    matchValue *matchTable = NULL;
    if (type == FIELD) 
    {
        matchTable = matchFieldTable;
    }
    else if (type == LINE_EDGE) 
    {   
        matchTable = matchLineTable;
    }
    
    if (matchTable == NULL || matchTable == NULL)
        return INFINITY_GRADIENT_VALUE;

    x = inX * 0.5; y = inY * 0.5;
    iX = (int)(x + 0.5); iY = (int)(y + 0.5);
    iX += offsetX;       iY += offsetY;

    (*gX) = 0.0; (*gY) = 0.0;
    
    if (iX < 0 || iX > (ARRAY_WIDTH - 1) || iY < -1 || iY > (ARRAY_HEIGHT - 1))
        return INFINITY_GRADIENT_VALUE;

    index = iY * ARRAY_WIDTH + iX;
    match = matchTable[index].match;
    (*gX) = matchTable[index].dx * 0.5;
    (*gY) = matchTable[index].dy * 0.5;
#ifdef OFFLINE    
    if (outputStream != NULL)
    {
        *outputStream <<__func__ << " inX:" << inX << " inY:" << inY << " type:" << type << " match:" << match << " gx:" << (*gX) << " gy:" << (*gY) << endl;
    }
#endif    
    return match;
}

/*****************************************************************************/
/* Weights points according to their distance and culls points that are too  */
/*  far away (usually these are noise)                                       */
#ifdef OFFLINE
void VisualCortex::calcEdgeWeights(double &reduceDist, double &cullDist, int &minPoints, double &minFieldDist)
{
#ifdef calcEdgeWeights_DEBUG
        ofstream fp_out;
        fp_out.open("DebugcalcEdgeWeights.txt", ios::out);
#endif        
#else
void VisualCortex::calcEdgeWeights()
{
    static const double reduceDist = 4900.0; // in cm2 reduce distance squared
#ifdef LOCALISATION_CHALLENGE    
    static const double cullDist = 122500.0;    //in cm2
#else
    static const double cullDist = 40000.0;// in cm2
#endif    
    static const int minPoints = 4;
    static const double minFieldDist = 25.0; //in cm
#endif //OFFLINE    
    double largestSqrDistanceToFieldPoint = 0;
    double dx, dy, d;
    int numPnts, typeCount[3];
    bool useType[3];
    bool nearFieldPoint = false;
    
    // First count the number of points of each type
    typeCount[FIELD] = 0;
    typeCount[LINE_EDGE] = 0;
    double temp = 0;
    double fieldPoints[numEdgePnts * 2];
    numEdgeLinePnts = 0;
    numEdgeFieldPnts = 0;
    numPnts = 0;
    totalLineWeight = 0.0; totalFieldWeight = 0.0;
    
    /* If wall exist then it will removed the ones on top of the wall, then
       We should not trim any field points. */
    if (! subobject.isWallExist()) 
    {
        //Finding the largest square distance of field points
        for (int i = 0; i < numEdgePnts; i++)
        {
            if (edgePntTypes[i] == FIELD) {
                fieldPoints[typeCount[FIELD]*2]  = localEdgePnts[i*2];
                fieldPoints[typeCount[FIELD]*2 + 1] = localEdgePnts[i*2 + 1];
                temp = SQUARE(localEdgePnts[i*2]) + SQUARE(localEdgePnts[i*2 + 1]);
                if (temp < SQUARE(FIELD_DIAGONAL) && largestSqrDistanceToFieldPoint < temp)
                {
                    largestSqrDistanceToFieldPoint = temp;
#ifdef OFFLINE
#ifdef calcEdgeWeights_DEBUG
                    //The farest field point
                    fp_out << __func__ << " ["<< i << "] x:" << localEdgePnts[i*2] << " y:" << localEdgePnts[i*2+1] << " d:" << sqrt(largestSqrDistanceToFieldPoint) << endl;
#endif                    
#endif
                }
            }
            typeCount[edgePntTypes[i]]++;
        }
        
        // If there are too few points of one type, don't use that type
        for (int i = 0; i < 3; i++)
            useType[i] = (typeCount[i] >= minPoints);
        
        
        //Squared the distance
        largestSqrDistanceToFieldPoint = sqrt(largestSqrDistanceToFieldPoint);
        //Add to some offset
        largestSqrDistanceToFieldPoint += minFieldDist;
        largestSqrDistanceToFieldPoint = SQUARE(largestSqrDistanceToFieldPoint);
    } else
        largestSqrDistanceToFieldPoint = SQUARE(FIELD_DIAGONAL);
        
#ifdef OFFLINE
#ifdef calcEdgeWeights_DEBUG
    fp_out << __func__ << " total point FIELD  :" << typeCount[FIELD] << " LINE_EDGE:" << typeCount[LINE_EDGE] << endl;
    fp_out << __func__ << " largestSqrDistanceToFieldPoint  :" << sqrt(largestSqrDistanceToFieldPoint) << endl;
#endif                    
#endif
    for (int i = 0; i < numEdgePnts; i++) {
        
        nearFieldPoint = false;
        
        dx = SQUARE(localEdgePnts[i * 2]);
        dy = SQUARE(localEdgePnts[i * 2 + 1]);
        
        //don't take points that are greater than the distance
        if (ABS(dy) > cullDist || ABS(dx) > cullDist)
        {
#ifdef OFFLINE
#ifdef calcEdgeWeights_DEBUG
            fp_out << __func__ << " localEdgePnts[i * 2]:" << localEdgePnts[i * 2] << " localEdgePnts[i * 2 + 1]:" << localEdgePnts[i * 2 + 1] << " cull dist:" << sqrt(cullDist) << endl;
#endif                    
#endif
            continue;
        }
/*  Calculate the nearest field point and check if if in certain distance */      
/* Not working well because some of the time the field points are not detected
   near the dog where obstacle nearby (the scan line get cut)*/
/*             
        if (edgePntTypes[i] != FIELD)
        {    
            
            //filtering the ones far away from the FIELD points
            for (int field = 0; field < typeCount[FIELD]; ++field)
            {
                d = SQUARE(dx-fieldPoints[field*2]) + SQUARE(dy-fieldPoints[field*2 + 1]);
                
                if (d < minFieldSqrDist) 
                {   
                    nearFieldPoint = true;
                    break;
                }
            }
            
            //Skip if it is not near point
            if (nearFieldPoint != true)
            {
                continue;
            }
        }
*/
        
        // Calculate distance d from robot to point
        d = dx + dy;
        //Only use the point inside the cull distance
        if (d > cullDist)
            continue;
            
        if (edgePntTypes[i] == LINE_EDGE) {
            /* check if the distance is in the certain distance from the farest away of field points */
            if (d > largestSqrDistanceToFieldPoint)
            {
#ifdef OFFLINE            
#ifdef calcEdgeWeights_DEBUG
                fp_out << __func__ << " removed d:" << sqrt(d) << " x:"<< localEdgePnts[i * 2] << " y:" << localEdgePnts[i * 2 + 1] << " biggest:" << sqrt(largestSqrDistanceToFieldPoint) <<  endl;
#endif
#endif
                continue;
            }
            
            // For field lines/borders, the weight for each point decreases as
            // distance increases, since projection accuracy decreases with
            // distance.
            if (d < reduceDist)
                edgeWeights[numPnts] = 1.0;
            else if (d < cullDist)
            {
                edgeWeights[numPnts] = (cullDist - d) / (cullDist - reduceDist);
            }
            
        } else {
            // Field points are constant weight because they don't need to be
            // particalarly accurate.
            edgeWeights[numPnts] = 1.0;
        }
#ifdef OFFLINE      
#ifdef calcEdgeWeights_DEBUG
        fp_out << __func__ << " type:"<< edgePntTypes[i] << " d:" << sqrt(d) << " x:"<< localEdgePnts[i * 2] << " y:" << localEdgePnts[i * 2 + 1] << " got weight:"<<edgeWeights[numPnts] <<endl;
#endif            
#endif
        // Only use point if it is within the culling distance
            
        if (numPnts < i) {
            localEdgePnts[numPnts * 2] = localEdgePnts[i * 2];
            localEdgePnts[numPnts * 2 + 1] = localEdgePnts[i * 2 + 1];
            edgePntTypes[numPnts] = edgePntTypes[i];
        }

        if (edgePntTypes[i] != FIELD)
        {
            totalLineWeight += edgeWeights[numPnts];
            ++numEdgeLinePnts;
        }
        else 
        {
            totalFieldWeight += edgeWeights[numPnts];
            ++numEdgeFieldPnts;
        }
        ++numPnts;
    }
    
#ifdef OFFLINE        
#ifdef calcEdgeWeights_DEBUG
    fp_out << __func__ << " numPnts:"<< numPnts << " totalLineWeight:"<<totalLineWeight<<" :totalFieldWeight:" <<totalFieldWeight <<endl;
#endif
#endif
    numEdgePnts = numPnts;
}

/*****************************************************************************/
/* Calculates the centre of the matched projected points                     */
void VisualCortex::calcEdgeCentre(MVec2 &projEdgeCentre)
{
    double x, y, weight, total, gX, gY, closeDist;
    int t, closest;

    total = 0.0;
    projEdgeCentre(0, 0) = 0.0;
    projEdgeCentre(1, 0) = 0.0;
    gX = 0.0; gY = 0.0;    
    // Calculate average centre point
    for (int i = 0; i < numEdgePnts; i++) {
        t = edgePntTypes[i];
        x = globalEdgePnts[i * 2];
        y = globalEdgePnts[i * 2 + 1];
#ifdef OFFLINE 
        if (outputStream != NULL)
        {
            *outputStream << __func__ << " x:" << x << " y:" << y << " t!=FIELD:" <<(t != FIELD)<<endl; 
        }
#endif        
        if (t != FIELD) {
#ifdef OFFLINE
            ostream *temp = outputStream; //To not include the calcPointMatch debug
            outputStream = NULL; 
#endif        
            weight = calcPointMatch(x, y, t, &gX, &gY) * edgeWeights[i];
            
#ifdef OFFLINE 
            outputStream = temp;
            if (outputStream != NULL)
            {
                *outputStream << __func__ << " weight:" << weight << " calcPM:"<< weight/edgeWeights[i] <<" gX:" << gX << " gY:" << gY << " edgeWeights[i]:" << edgeWeights[i]<<endl; 
            }
#endif        
            projEdgeCentre(0, 0) += weight * x;
            projEdgeCentre(1, 0) += weight * y;
            total += weight;
        }
    }
    
    if (total > 0) projEdgeCentre /= total;
    
    // Find projected point that is closest to the average centre
    closest = -1;
    closeDist = 0.0;
    for (int i = 0; i < numEdgePnts; i++) {
        t = edgePntTypes[i];
        x = globalEdgePnts[i * 2] - projEdgeCentre(0, 0);
        y = globalEdgePnts[i * 2 + 1] - projEdgeCentre(1, 0);
        weight = x * x + y * y;
        
        if ((weight < closeDist || closest == -1) && t != FIELD) {
            closest = i;
            closeDist = weight;
        }
    }

    // Set the centre to closest point
    projEdgeCentre(0, 0) = globalEdgePnts[closest * 2];
    projEdgeCentre(1, 0) = globalEdgePnts[closest * 2 + 1];
}

/*****************************************************************************/
/* If the dog is standing still, instead of throwing away points from        */
/*  previous frames, we build up a map from them                             */
static const int maxMapConf = 100;

void VisualCortex::stationaryMap()
{
    int index, localInd, oldInd, pT, pC, failedAdds;
//    double mergeRad, scaler, pDistSq;
    double pX, pY;
    bool pointUsed;

    // We have to restart map building if the robot moves position
    if (bodyMoving || mapInvalid) {
#ifdef OFFLINE    
#ifdef FLV_DEBUG
        if (bodyMoving)
            cout <<  __func__ <<  " body moving" <<endl;
        if (mapInvalid)
            cout <<  __func__ <<  " mapInvalid" <<endl;
#endif
#endif //OFFLINE        
        numMapPnts = 0;
        mapInvalid = false;
        return;
    }

    failedAdds = 0;    
    numEdgeFieldPnts = 0;
    numEdgeLinePnts = 0;
    // Incorporate edge pixel data into map
    for (localInd = 0; localInd < numEdgePnts; localInd++) {
        pointUsed = false;

        pC = 1;
        pT = edgePntTypes[localInd];
        pX = localEdgePnts[localInd * 2];
        pY = localEdgePnts[localInd * 2 + 1];
        oldInd = -1;
        
        // Check if current point is close to a map point
        while (shouldMerge(pX, pY, pT, oldInd, &index)) {
            // If current point matches a map point, merge it
            pX = pX * pC + mapPnts[index * 2] * mapPntConfidence[index];
            pY = pY * pC + mapPnts[index * 2 + 1] * mapPntConfidence[index];
            pC += mapPntConfidence[index];
            pX /= pC; pY /= pC;
            pC = min(pC, maxMapConf);
            
            mapPnts[index * 2] = pX;
            mapPnts[index * 2 + 1] = pY;
            mapPntConfidence[index] = pC;
                
            if (pointUsed) mapPntConfidence[oldInd] = 0;
            oldInd = index;
            pointUsed = true;
        }
        
        // If point could not be merged, insert it into map array
        if (!pointUsed) {
            // Otherwise add a new map point
            if (index < 0 && numMapPnts < MAX_EDGE_PIXELS) {
                index = numMapPnts;
                numMapPnts++;
            }
            
            if (index >= 0) {
                mapPntConfidence[index] = pC;
                mapPntTypes[index] = pT;
                mapPnts[index * 2] = pX;
                mapPnts[index * 2 + 1] = pY;
            } else failedAdds++;
        }
    }
    
    // If we couldn't add points, then decrease confidence of all map points
    if (failedAdds > 0) {
        for (int i = 0; i < numMapPnts; i++) {
            if (mapPntConfidence[i] <= failedAdds)
                mapPntConfidence[i] = 0;
            else mapPntConfidence[i] -= failedAdds;
        }
    }

    // Copy map points to local points array
    totalLineWeight = 0.0; totalFieldWeight = 0.0;
    numEdgePnts = 0;
    for (int i = 0; i < numMapPnts; i++) {
        if (mapPntConfidence[i] > 0) {
            pX = mapPnts[i * 2];
            pY = mapPnts[i * 2 + 1];
            localEdgePnts[numEdgePnts * 2] = pX;
            localEdgePnts[numEdgePnts * 2 + 1] = pY;
            edgePntTypes[numEdgePnts] = mapPntTypes[i];
            edgeWeights[numEdgePnts] = mapPntConfidence[i];

            if (edgePntTypes[numEdgePnts] == FIELD)
            {
                totalFieldWeight += edgeWeights[numEdgePnts];
                ++numEdgeFieldPnts;
            } 
            else
            {
                totalLineWeight += edgeWeights[numEdgePnts];
                ++numEdgeLinePnts;
            }
            numEdgePnts++;
        }
    }
}

bool VisualCortex::shouldMerge(double x, double y, int t, int dontMergeWith, int *mergeInd)
{
    double mX, mY, mDSq, dX, dY, dSq, dotProd, cosSq, mergeRad;
    int mapInd;

    dSq = x * x + y * y;
    if (t == FIELD) mergeRad = 20.0 + 0.2 * sqrt(dSq);
    else mergeRad = 7.0 + 0.15 * sqrt(dSq);
    mergeRad *= mergeRad;

    (*mergeInd) = -1;
    for (mapInd = 0; mapInd < numMapPnts; mapInd++) {
        if (mapPntConfidence[mapInd] == 0) {
            (*mergeInd) = mapInd;
            
        } else if (t == mapPntTypes[mapInd]) {
            // Calculate square distance between local and map points
            mX = mapPnts[mapInd * 2];
            mY = mapPnts[mapInd * 2 + 1];
            dX = mX - x; dY = mY - y;
                
            if ((dX * dX + dY * dY) <= mergeRad) {
                // Calculate cosine of angle between local and map points
                dotProd = x * mX + y * mY;
                mDSq = mX * mX + mY * mY;
                cosSq = dotProd * dotProd / (dSq * mDSq);

                // Don't want to do angle check for field points
                if ((t == FIELD || cosSq > 0.997) && mapInd != dontMergeWith) {
                    (*mergeInd) = mapInd;
                    return true;
                }
            }
        }
    }
    
    return false;
}

void VisualCortex::invalidateEdgeMap()
{
    mapInvalid = true;
}

/*****************************************************************************/
/* Adds in edge pixel to the edgeList, but only if it passes some checks    
 * Returns true if we can still fit more edge pixels in our arrays
 */
bool VisualCortex::tryAddEdge(double x, double y, int t, int *numEdges, int dir)
{
    (void)dir; // silence compiler warning
//    int odir = 1 - dir;
//    uchar cPixel;

    if (*numEdges >= MAX_EDGE_PIXELS)
        return false;
    
    // Make sure point is not inside a dog or ball vob
    for (int i = vobBall; i <= vobBlueDog4; i++) {
        if (vob[i].cf > 0) {
            if (x >= vob[i].x && x <= (vob[i].x + vob[i].width) &&
                y >= vob[i].y && y <= (vob[i].y + vob[i].height))
                
                return true;
        }
        
        if (i == vobBall) i = vobRedDog - 1;
    }

    // don't add points right on the edge of the image
    if (x < 1 || x > CPLANE_WIDTH - 1)
        return true;
    if (y < 1 || y > CPLANE_HEIGHT - 1)
        return true;

//    int pos[2], greenCount = 0;

    edgeList[(*numEdges) * 2 + 0] = x;
    edgeList[(*numEdges) * 2 + 1] = y;
//    edgeList[(*numEdges) * 3 + 2] = t;
    edgePntTypes[*numEdges] = t;
    (*numEdges)++;

    return ((*numEdges) < MAX_EDGE_PIXELS);
}

/*****************************************************************************/
/* This scans the c-plane in a series of horizontal and vertical lines,      */
/* searching for green to white, or white to green transitions that mark the */
/* field lines or edges. All transition pixels will be stored in the edgeList*/
/* and number in the array  will be returned.                                */
int VisualCortex::findFieldEdges()
{
#if 0 // old CPlane edge detection
    static const int SOLID_GREEN_THRESHOLD = 2;
    static const int NOISE_THRESHOLD = 2;
    static const int MAX_LINE_WIDTH   = 18;

    bool green, white, edgeStarted, fieldStarted, intOffScreen, useFieldPoints;
    int pos[2], max[2], st[2], field[2], dir, odir, numEdges;
    int greenSeen, greenSinceField, othersSeen, eType;
    int stop, intercept, fieldInterval, edgeInterval;
    double in;
    uchar cPixel, previous;

    numEdges = 0;
    max[0] = CPLANE_WIDTH;
    max[1] = CPLANE_HEIGHT;

    // Scan both horizontal and vertical lines
    for (dir = 0; dir < 2; dir++) {
        odir = 1 - dir;

        for (pos[odir] = 0; pos[odir] < max[odir]; pos[odir] += GRID_SPACING) {
            // Only scan as far as horizon permits
            intercept = 0;
            intOffScreen = !horizonIntercept(pos[odir], odir, &intercept);

            pos[dir] = max[dir] - 1;
            stop = 0;

            if (!intOffScreen) {
                if (isBelowHorizon(pos[0], pos[1], 0)) stop = intercept;
                else pos[dir] = intercept;
            } else if (!isBelowHorizon(pos[0], pos[1], 0)) stop = max[dir];
            
            // Record field green points only occasionally
            useFieldPoints = (dir == 1 && pos[odir] % (5 * GRID_SPACING) == 0);
            
            // Scan along line, checking for white/green transitions
            greenSeen = 0;
            othersSeen = 0;
            greenSinceField = 0;
            previous = cNONE;
		    edgeStarted = false;
            edgeInterval = 0;

            for (; pos[dir] >= stop; pos[dir]--) {
                cPixel = cplane[pos[1] * CPLANE_WIDTH + pos[0]] & COLOR_MASK;
                
                // Count number of consecutive green pixels
                if (cPixel == cFIELD_GREEN) greenSeen++;
                else greenSeen = 0;
                
                // Count number of non-green & non-white pixels
                if (cPixel != cFIELD_GREEN
                    && (cPixel & COLOR_MASK) != cWHITE)
                    othersSeen++;
                else othersSeen = 0;
                
                green = (greenSeen >= SOLID_GREEN_THRESHOLD);
                white = (cPixel == cWHITE);
                
                if (othersSeen > NOISE_THRESHOLD) previous = cNONE;
                
                // Check for green -> white transitions
                if (previous == cFIELD_GREEN && white) {
                    st[0] = pos[0]; st[1] = pos[1];
                    edgeStarted = true;
                    edgeInterval = 0;
                }
                
                // Check for white -> green transitions
                if (previous == cWHITE && green) {
                    bool addEdge = true;
                
                    if (edgeStarted) {
                        if (edgeInterval <= MAX_LINE_WIDTH) {
                            // For white lines thin enough to be classified as a
                            // field line, put the edge pixel in the centre of
                            // the line.
                            st[dir] += pos[dir];
                            st[dir] += SOLID_GREEN_THRESHOLD;
                            st[dir] /= 2;
                            eType = LINE_EDGE;
                            /* OLDFIELD    
                        } else if (edgeInterval > MIN_BORDER_WIDTH) {
                            // If the white line was fat enough to be classified
                            // as a field border, we have to decide which side
                            // of the line to put the edge pixel on. When
                            // scanning up the cplane, always take the lower
                            // one. When scanning across take the closer of the
                            // two points to the centre.
                            if (dir == 0) {
                                int endLinePos, startLineDist, endLineDist;
                                endLinePos = pos[dir] + SOLID_GREEN_THRESHOLD;
                                endLineDist = abs(endLinePos - max[dir] / 2);
                                startLineDist = abs(st[dir] - max[dir] / 2); 
                            
                                if (endLineDist < startLineDist) {
                                    st[0] = pos[0];
                                    st[1] = pos[1];
                                    st[dir] += SOLID_GREEN_THRESHOLD;
                                }
                            }

                            eType = BORDER_EDGE;
                        
                       // Don't add points if unsure if it is a line or border.
                       OLDFIELD */
                        } else addEdge = false;

                    // If we are scanning from bottom of cplane up, we will not
                    // be able to tell if the first white->green transition
                    // (without a preceeding green->white transition) is a field
                    // border or line, so don't use it.
                    } else if (dir != 1) {
                        st[0] = pos[0];
                        st[1] = pos[1];
                        st[dir] += SOLID_GREEN_THRESHOLD;
                        eType = BORDER_EDGE;
                    
                    } else addEdge = false;

                    if (addEdge) {
                        if(!tryAddEdge(st[0], st[1], eType, &numEdges, dir))
                            return numEdges;
                    }
                
                    edgeStarted = false;
                }

                // Check for substantial amounts of solid green, and mark them
                // as field green (only do this in one direction, so we don't
                // get too many points)
                if (useFieldPoints) {

                    // Record more field points as we get closer to the horizon
                    // or top of the screen.
                    fieldInterval = min((pos[dir] - stop) >> 2, 25);
                
                    if (greenSeen > fieldInterval) {
                        if (greenSinceField == 0) {
                            field[0] = pos[0]; field[1] = pos[1];

                        } else if (greenSinceField > fieldInterval) {
                            if(!tryAddEdge(field[0], field[1], FIELD, &numEdges, dir))
                                return numEdges;
    
                            field[0] = pos[0]; field[1] = pos[1];
                            greenSinceField = 0;
                        }
                    
                        greenSinceField++;
                    
                    } else greenSinceField = 0;
                }
        
                if (green) previous = cFIELD_GREEN;
                if (white) previous = cWHITE;
                edgeInterval++;
            }
            
            if (edgeStarted) {
                if(!tryAddEdge(st[0], st[1], BORDER_EDGE, &numEdges, dir))
                    return numEdges;            
            }
        }
    }
#endif
    // Extract field lines and field green from SubVision's detected points
    int numEdges = 0;
    vector<VisualFeature>::iterator itr = subvision.getFeatures().begin();
    for (; itr != subvision.getFeatures().end(); ++itr) {
        if (itr->type == VF_FIELDLINE) {
            if (! tryAddEdge(itr->x, itr->y, LINE_EDGE, &numEdges, itr->dir))
                break;
        } else if (itr->type == VF_FIELD) {
            if (! tryAddEdge(itr->x, itr->y, FIELD, &numEdges, itr->dir))
                break;
        }
    }
    //cout << __func__ << " num edges is "<< numEdges <<endl;

    // For each edge pixel found, mark it on the c-plane
    for (int i = 0; i < numEdges; i++) {
        int x = (int)edgeList[2*i]; int y = (int)edgeList[2*i+1];
		int type = edgePntTypes[i];

        if (cplane && type == LINE_EDGE)
            cplane[y * CPLANE_WIDTH + x] = cFIELD_LINE;
        else if (cplane && type == FIELD)
            cplane[y * CPLANE_WIDTH + x] = cROBOT_GREY;
    }
    
    return numEdges;
}
/*
//Takes inNumPoints points as local co-ordinate pairs in inLocal and transforms
//them into global co-ordinate pairs in outGlobal based on inX, inY and inT.
//NOTE: outGlobal must be twice inNumPoints in size (as must inLocal) or we'll
//have problems. This routine works even if inLocal and outGlobal are the same. 
void VisualCortex::localToGlobal(MVec3 &robot, int inNumPoints, double *inLocal, double *outGlobal) {

    //First, we rotate all the points about the origin by an amount equal to
    //inT-90 where positive angle is COUNTERCLOCKWISE. For instance, if we're
    //facing left (inT = 180) we need to rotate all the points counterclockwise
    //by 90 degrees.
    
    double newAng = robot(2, 0) - pi * 0.5;
    double sinT = sin(newAng);
    double cosT = cos(newAng);
    int index = 0;
    
    for (int i = 0; i < inNumPoints; i ++) {
        double in0 = inLocal[index + 0];
        double in1 = inLocal[index + 1];

        outGlobal[index + 0] = (in0 * cosT - in1 * sinT) + robot(0, 0);
        outGlobal[index + 1] = (in0 * sinT + in1 * cosT) + robot(1, 0);
        
        index += 2;
    }
}
*/
