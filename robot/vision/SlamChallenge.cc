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


#include "gps.h"
#include "../share/Common.h"
#define SECOND_WAY
#define TESTING_LANDMARK
#define DEBUG 0
//#define FN_DEBUG

// This determines at what probability a measurement is regarded as
//  noise, and so will be ignored
// Same in the gps.cc
static const double noiseThreshold = 1E-2;

// Next few methods are used for the localisation challenge.
// The next two are called by ResultCamera in Vision.cc.

// This calculates where the given pink blobs would be if they were attached
// to the perimeter walls, and records that position.

static const double mergeDistSq = SQUARE(15.0); //only side x or y depend on where is the point.
static const double mergeAngle = 5.0; //max difference in angle before doing merge
static const int angleRange = 1; //range of threshold angle + and - from the predicted angle.


// This is the criteria for number of pink in the box.
static const int minimumPinkInBox = 4;

void GPS::ResetPinkMapping(void){
    if (DEBUG)
        cerr << __func__ << endl;
    
    numPink = 0;
#ifdef LOCALISATION_CHALLENGE   
    listPinkEQ.clear();
#endif
}


void GPS::MapLocalisePink(VisualObject *pvo, int numPinkVob) {
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
    
    double x, y, dx, dy, distSq, scale;
    bool merged;
    int i, j;
#ifdef LOCALISATION_CHALLENGE   
#ifdef SECOND_WAY
    MapPinkEquations(pvo,numPinkVob);
    return;
#endif
#endif
    
    pinkUpdate = false;
#ifdef OFFLINE    
    cout << __func__ << " started. allowpinkmapping:" << allowPinkMapping << endl;
#endif    
    if (!allowPinkMapping) return;
    
    for (i = 0; i < numPinkVob; i++) {
        if (CalcWallIntercept(&pvo[i], &x, &y)) {
#ifdef OFFLINE        
            cout << __func__ << " x:" << x << " y:"<< y << endl;
#endif            
            merged = false;
            pinkUpdate = true;
            
            // For every valid pink blob, see if it can be merged with one
            // of the mapped pink blobs, by checking distance from each.
            for (j = 0; j < numPink && (!merged); j++) {
                dx = x - pinkX[j]; dy = y - pinkY[j];
                distSq = dx * dx + dy * dy;
                
                // If this pink blob is close enough to be merged with another,
                // then use it in the running average.
                if (merged = (distSq < mergeDistSq)) {
                    scale = 1.0 / (1 + pinkConf[j]);
                    pinkX[j] += dx * scale;
                    pinkY[j] += dy * scale;
                    pinkConf[j]++; // Increase confidence value
                }
            }

            // If the blob couldn't be merged with another, start a new one. 
            if (!merged && numPink < maxPink) {
                pinkX[numPink] = x;
                pinkY[numPink] = y;
                pinkConf[numPink] = 1;
                numPink++;
            }
        }
    }
    
    // Slowly decrease confidence (to hopefully eliminate pink noise)
    pinkDecreaseCounter++;
    if (pinkDecreaseCounter > 120) {
        ReducePinkConfidence(3, 1);
        pinkDecreaseCounter = 0;
    }
}

// This uses the largest valid pink blob seen to localise with the pink blobs
// recorded by MapLocalisePink. Only an angle update is performed.

void GPS::LocalisePinkUpdate(VisualCortex *vision) {
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
    if (!doGPSVisionUpdate)
            return;
    VisualObject *pvo = vision->localisePink;
    int numPinkVob = vision->numLocalisePink;
#ifdef LOCALISATION_CHALLENGE   
#ifdef SECOND_WAY
    StationaryMapAndLocalisePinkAngle(vision);
    return;
#endif
#endif
    
    double jacX, jacY, jacH, iv, var, minWeight;
    int oldNumGaussians, mode, obsMode, dummy;
    int i, vobArea, useVob;
    SingleGaussian temp;
    SlimVisOb obs;
    
    pinkUpdate = false;
        
    if (!landmarksInitialised) {
        // Cull any low confidence pink blobs
#ifdef TESTING_LANDMARK
        pinkX[0] = 260;
        pinkY[0] = -1;
        pinkX[1] = 295;
        pinkY[1] = 550;
        pinkX[2] = -15;
        pinkY[2] = 270;
        numPink = 3;
#else        
        ReducePinkConfidence(3, 3);
#endif        
        cout << "Found " << numPink << " pink landmarks.\n";
        for (i = 0; i < numPink; i++) {
            cout << "Landmark " << i;
            cout << " - x: " << pinkX[i] << "   y: " << pinkY[i];
            cout << "   conf: " << pinkConf[i] << "\n";
        }
        landmarksInitialised = true;
    }    
    
    // Find the largest valid pink blob
    vobArea = 0; useVob = -1;
    for (i = 0; i < numPinkVob; i++) {
        // Do an elevation check on the blob
        //if (pvo->elev < 10.0 && pvo->elev > -2.0) {

            // If this is the largest pink blob, use it
            if (useVob < 0 || (pvo->width * pvo->height) > vobArea) {
                useVob = i;
                vobArea = int(pvo->width * pvo->height);
            }
        //}
    }
    
    // If we found a suitable blob, angle localise off it
    pinkUpdate = (useVob >= 0);

    if (pinkUpdate) {
        // Work from a backup of the old distribution
        minWeight = r[numGaussians - 1].weight;
        oldNumGaussians = numGaussians;
        for (i = 0; i < numGaussians; i++) {
            rCopy[i] = r[i];
            r[i].weight *= noiseThreshold;
        }
        
        mode = 0;
        while (mode < oldNumGaussians && rCopy[mode].weight >= minWeight) {
#ifdef OFFLINE
/*
            cout << __func__ << " ORIGIN rCopy[mode]" << endl;
            rCopy[mode].printOut();
            cout << __func__ << " ENDORIGIN" << endl;
*/            
#endif
        /*
        for (useVob = 0; useVob < numPinkVob; ++useVob)
        {
            //if the diff angle less than 5, skip
            if (ABS(obs.h - pvo[useVob].head) < 5 )
                continue;
        */                
            for (obsMode = 0; obsMode < numPink; obsMode++) {
        
                // Calculate the effect of every observation mode on each of
                // the distribution modes
                dummy = 0;
                temp = rCopy[mode];
                obs.h = pvo[useVob].head;
                obs.angleVar = 400.0;
                addMeasurement(&temp, obs, pinkX[obsMode], pinkY[obsMode],
                    &jacX, &jacY, &jacH, &iv, &var, dummy, false);
                kalmanUpdate<1>(&temp, &jacX, &jacY, &jacH, &iv, &var);
#ifdef OFFLINE          
/*          
                cout << __func__ << " obs.h:" << obs.h << " pinkX:" << pinkX[obsMode] << " pinkY:" << pinkY[obsMode] << endl;
                temp.printOut();
                cout << __func__ << " temp" << endl;
*/                
#endif                
                // Insert the corrected mode into the new distribution
                insertGaussian(temp);
                if (numGaussians == maxGaussians)
                    minWeight = r[numGaussians - 1].weight;
            }
         //}
            mode++;
        }
#ifdef OFFLINE
/*
        cout << __func__ << "BEFORE FORM DISTRIBUTION" << endl;
        GPSOutputSelfData();
        cout << __func__ << "BEFORE FORM DISTRIBUTION END" << endl;
*/        
#endif  
        formDistribution();
#ifdef OFFLINE
/*
        cout << __func__ << "AFTER FORM DISTRIBUTION" << endl;
        GPSOutputSelfData();
        cout << __func__ << "AFTER FORM DISTRIBUTION END" << endl;
        cout << endl << endl;
*/        
#endif
    }
}

// A set of 4 (x, y) gps coordinates that specify the perimeter wall corner
// coordinates. Should to be in CLOCKWISE order. 
static double perimeter[8] = {
-SIDE_FRINGE_WIDTH, -GOAL_FRINGE_WIDTH,
-SIDE_FRINGE_WIDTH, FIELD_LENGTH + GOAL_FRINGE_WIDTH,
FIELD_WIDTH + SIDE_FRINGE_WIDTH, FIELD_LENGTH + GOAL_FRINGE_WIDTH,
FIELD_WIDTH + SIDE_FRINGE_WIDTH, -GOAL_FRINGE_WIDTH};

static double beaconPos[8] = {
    BEACON_LEFT_X, 
    BEACON_CLOSE_Y,
    BEACON_LEFT_X, 
    BEACON_FAR_Y,
    BEACON_RIGHT_X, 
    BEACON_FAR_Y,
    BEACON_RIGHT_X, 
    BEACON_CLOSE_Y };

                              
// Given a visual object, this takes the heading and elevation and determines
// the intercept with the perimeter wall, returned in iX, iY. Returns true
// if the VisualObject should be accepted as a legitimate pink blob.

bool GPS::CalcWallIntercept(VisualObject *pvo, double *iX, double *iY) {
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
    
    double wallDX, wallDY, wallA, wallB, wallC;
    double rayHead, rayX, rayY, dist, dotProd;
    double intersectX, intersectY;
    int curPoint, nextPoint;

    // We should make sure self variance is small if we are going to record
    // the position of this blob.
    if (r[0].mean.posVar > 500.0 || r[0].mean.hVar > 300.0)
    {
#ifdef OFFLINE
        cout << __func__ << " Variance is too big" <<endl;
        
#endif        
        return false;
    }
    
#ifdef OFFLINE
        cout << __func__ << " pvo->head:"<<pvo->head <<endl;
#endif        
    // Calculate ray direction parameters
    rayHead = r[0].mean.h + pvo->head;
    rayX = cos(DEG2RAD(rayHead));
    rayY = sin(DEG2RAD(rayHead));

    // Make sure ray doesn't intersect a beacon position
    for (curPoint = 0; curPoint < 8; curPoint += 2) {
        // Make sure beacon is in ray direction
        intersectX = beaconPos[curPoint] - r[0].mean.pos.x;
        intersectY = beaconPos[curPoint + 1] - r[0].mean.pos.y;
#ifdef OFFLINE
        cout << __func__ << " :"<<curPoint<<": intersectX:" <<intersectX<<" intersectY:"<<intersectY <<endl;
#endif        

        if (intersectX * rayX + intersectY * rayY >= 0) {
            // If perpendicular distance from ray to beacon is less than a
            // threshold value, we might have accidentally taken the pink blob
            // from a beacon, so don't use it.
            dotProd = intersectX * rayY - intersectY * rayX;
            if (ABS(dotProd) < 25) 
            {
#ifdef OFFLINE
                cout << __func__ << " Near beacon position" <<endl;
#endif        
                return false;
            }
        }
    }
    
    curPoint = 6;
    for (nextPoint = 0; nextPoint < 8; nextPoint += 2) {
        // Check for an intersection between each wall and a ray from robot
        // position, in the given vob's direction
        wallDX = perimeter[nextPoint] - perimeter[curPoint];
        wallDY = perimeter[nextPoint + 1] - perimeter[curPoint + 1];

        // Calculate wall line paramters
        wallA = wallDY; wallB = -wallDX;
        wallC = perimeter[nextPoint] * perimeter[curPoint + 1];
        wallC -= perimeter[curPoint] * perimeter[nextPoint + 1];
        
        // Find intersection between heading ray and wall line
        dist = -(wallA * r[0].mean.pos.x + wallB * r[0].mean.pos.y + wallC);
        dist /= (wallA * rayX + wallB * rayY);

        // Check that the elevation of the blob is not too high/low.
        /*
        if (pvo->elev > (5.0 + 500.0 / dist) || pvo->elev < -2.0)
        {
#ifdef OFFLINE
            if (pvo->elev > (5.0 + 500.0 / dist))
                cout << __func__ << " Elevation of the blob is too high:"<< pvo->elev << " (5.0 + 500.0 / dist)="<<(5.0 + 500.0 / dist)<< " dist=" << dist <<endl;
            else            
                cout << __func__ << " Elevation of the blob is too low:" << pvo->elev << endl;
#endif                
            return false;
        }
        */
        if (dist >= 0) {
            // Make sure intersection occurs within wall interval
            intersectX = rayX * dist + r[0].mean.pos.x;
            intersectY = rayY * dist + r[0].mean.pos.y;
            
            dotProd = (intersectX - perimeter[curPoint]) * wallDX;
            dotProd += (intersectY - perimeter[curPoint + 1]) * wallDY;
            dist = wallDX * wallDX + wallDY * wallDY;

            if (dotProd >= 0 && dotProd < dist) {
                (*iX) = intersectX;
                (*iY) = intersectY;
#ifdef OFFLINE
                cout << __func__ << " OK! X:" << intersectX << " Y:"<<intersectY <<endl;
#endif                                
                return true;
            }            
        }

        curPoint = nextPoint;
    }
#ifdef OFFLINE
                cout << __func__ << " Last OK! X:" << *iX << " Y:"<<*iY <<endl;
#endif
    return true;
}

void GPS::ReducePinkConfidence(int minConf, int decreaseAmount) {
    if (DEBUG)
        cerr << __func__ << endl;
    
    int newNumPink = 0;
    bool deleted;
    
    for (int i = 0; i < numPink; i++) {
        // Only reduce confidence if it is low to start with
        if (pinkConf[i] <= minConf) {
            pinkConf[i] -= decreaseAmount;

            // Delete any modes whos confidence drops to zero
            deleted = (pinkConf[i] <= 0);

        } else deleted = false;

        // Compact list of pink modes if any modes have been deleted
        if (!deleted) {
            if (newNumPink < i) {
                pinkX[newNumPink] = pinkX[i];
                pinkY[newNumPink] = pinkY[i];
                pinkConf[newNumPink] = pinkConf[i];
            }
        
            newNumPink++;
        }
    }
        
    numPink = newNumPink;
}

#ifdef LOCALISATION_CHALLENGE

void GPS::ResetPinkUpdate(void){
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif
    numStationaryRecordedAngle = 0;
}

void GPS::GPSOffFieldUpdate(VisualCortex *vc)
{
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
    
    if (!doGPSVisionUpdate)
            return;

    if (vc->numEdgeFieldPnts > 3)
        return;
    cout << __func__ << " FACING OFF FIELD " << endl;
    double dist_seeing_offfield = 30;
    double x[4], y[4], h[4], vX[4], vY[4], vH[4], w[4];
    VisualObject vob;
    vob.imgHead = pointToHeading(CPLANE_WIDTH/2);
    vob.imgElev = pointToElevation(CPLANE_HEIGHT/2);
    vc->RobotRelative(vob);
    vob.setPolar();
    
    //Update on right position
    x[0] = FIELD_WIDTH; y[0] = 0; h[0] = (-vob.head); vX[0] = SQUARE(dist_seeing_offfield); vY[0] = VERY_LARGE_INT; vH[0] = SQUARE(90);
    //Update on left position
    x[1] = 0; y[1] = 0 ; h[1] = 180-vob.head; vX[1] = SQUARE(dist_seeing_offfield); vY[1] = VERY_LARGE_INT; vH[1] = SQUARE(90);
    //Update on up position
    x[2] = 0; y[2] = FIELD_LENGTH; h[2] = 90-vob.head; vX[2] = VERY_LARGE_INT; vY[2] = SQUARE(dist_seeing_offfield); vH[2] = SQUARE(90);
    //update on bottom position
    x[3] = 0; y[3] = 0 ; h[3] = 270-vob.head; vX[3] = VERY_LARGE_INT; vY[3] = SQUARE(dist_seeing_offfield); vH[3] = SQUARE(90);
    w[0] = 1; w[1] = 1; w[2] = 1; w[3] = 1;
   
    GPSDirectPositionUpdate(4, x, y, h, vX, vY, vH, w);
     
    formDistribution();
}

void GPS::StationaryMapAndLocalisePinkAngle(VisualCortex *vision)
{
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif
    VisualObject *pvo = vision->localisePink;
    int numPinkVob = vision->numLocalisePink;
    bool merged;
    double diffAngle, tempAngle,scale;
    double weight = 0.0;
    double bestweight = 0.0;
    MVec3 pos,bestpos,grad;
    int tempConf, j, index;
    pinkUpdate = false;
#ifdef OFFLINE    
    cout << __func__ << " bodyMoving:" << bodyMoving << " numPinkVob:" << numPinkVob << endl;
#endif    
    if (bodyMoving)
        numStationaryRecordedAngle = 0;
        
    for (index = 0; index < numPinkVob; ++index)
    {
        merged = false;
        pinkUpdate = true;
        //For every angle stored, check if it is with in certain angle then
        //try to merged them.        
        for (j = 0; j < numStationaryRecordedAngle && (!merged); j++) {
            diffAngle = pvo[index].head - stationaryRecordedAngle[j];
            //If angle is close enough then use running average.
            if (merged = (ABS(diffAngle) < mergeAngle)) {
                scale = 1.0 / (1 + stationaryRecordedAngleConf[j]);
                stationaryRecordedAngle[j] += diffAngle * scale;
                stationaryRecordedAngleConf[j]++; // Increase confidence value
            }
        }

        // If the angle couldn't be merged with another, start a new one. 
        if (!merged && numStationaryRecordedAngle < maxPink) {
            stationaryRecordedAngle[numStationaryRecordedAngle] = pvo[index].head;
            stationaryRecordedAngleConf[numStationaryRecordedAngle] = 1;
            ++numStationaryRecordedAngle;
        }
    }
    
        
    //Insertion sort, number of pink only < 20, and it possibly already sorted
    for (index = 1; index < numStationaryRecordedAngle; ++index)
    {
        tempAngle = stationaryRecordedAngle[index];
        tempConf = stationaryRecordedAngleConf[index];
        j = index;
        while ((j > 0) && (stationaryRecordedAngle[j-1] > tempAngle))
        {
            stationaryRecordedAngle[index] = stationaryRecordedAngle[index-1];
            stationaryRecordedAngleConf[index] = stationaryRecordedAngleConf[index-1];
            --j;
        }
        stationaryRecordedAngle[index] = tempAngle;
        stationaryRecordedAngleConf[index] = tempConf;
    }
    
    if (numStationaryRecordedAngle < 1) 
        return;
        
#ifdef OFFLINE    
    for (index =  0; index < numStationaryRecordedAngle; ++index)
    {
        cout << __func__ << " [" << index << "] angle:" << stationaryRecordedAngle[index] << endl;
    }
#endif    
    
    /*    
    if (!bodyMoving)
    {
        for (index = 0; index < numStationaryRecordedAngle; ++index)
        {
            cout << __func__ << " angle:" << stationaryRecordedAngle[index] <<endl;
        }
    }
    */
    
    //if (numStationaryRecordedAngle < 2)
    {
        double jacX, jacY, jacH, iv, var, minWeight;
        int oldNumGaussians, mode, obsMode, dummy;
        int i, vobArea, useVob;
        SingleGaussian temp;
        SlimVisOb obs;
        
        pinkUpdate = false;
        
        
        // Find the largest valid pink blob
        vobArea = 0; useVob = -1;
        
        for (i = 0; i < numPinkVob; i++) {
            // If this is the largest pink blob, use it
            if (useVob < 0 || (pvo->width * pvo->height) > vobArea) {
                useVob = i;
                vobArea = int(pvo->width * pvo->height);
            }
        }
        
        // If we found a suitable blob, angle localise off it
        pinkUpdate = (useVob >= 0);
    
        if (pinkUpdate) {
            // Work from a backup of the old distribution
            minWeight = r[numGaussians - 1].weight;
            oldNumGaussians = numGaussians;
            for (i = 0; i < numGaussians; i++) {
                rCopy[i] = r[i];
                r[i].weight *= noiseThreshold;
            }
            
            mode = 0;
            while (mode < oldNumGaussians && rCopy[mode].weight >= minWeight) {
                for (obsMode = 0; obsMode < numPink; obsMode++) {
            
                    // Calculate the effect of every observation mode on each of
                    // the distribution modes
                    dummy = 0;
                    temp = rCopy[mode];
                    obs.h = pvo[useVob].head;
                    obs.angleVar = 400.0;
                    addMeasurement(&temp, obs, pinkX[obsMode], pinkY[obsMode],
                        &jacX, &jacY, &jacH, &iv, &var, dummy, false);
                    kalmanUpdate<1>(&temp, &jacX, &jacY, &jacH, &iv, &var);
                    // Insert the corrected mode into the new distribution
                    insertGaussian(temp);
                    if (numGaussians == maxGaussians)
                        minWeight = r[numGaussians - 1].weight;
                }
                mode++;
            }
            formDistribution();
        }
    }
    //else
    if (numStationaryRecordedAngle >= 2)
    {
        //For each adjustment angle, look for the coresponding next angle
        PinkAngleBox *pab;
        int h,i,k;
        tempAngle = ABS(stationaryRecordedAngle[0] - stationaryRecordedAngle [1]);
#ifdef OFFLINE        
        cout << __func__ << " tempAngle:" << tempAngle << endl;
#endif        
        for (index = (int)tempAngle-angleRange; index <= (int)tempAngle+angleRange; ++index)
        {
#ifdef OFFLINE                
            cout << __func__ << " trying angle:" << index << " position size:" << anglelist[index].size() << endl;
#endif            
            for (j = 0; j < (int) anglelist[index].size(); ++j)
            {
                pab = anglelist[index][j];
#ifdef OFFLINE                
                //cout << __func__ << " raw possibility x:" << pab->posX << " y:" << pab->posY << endl;
#endif                
                //Find every angles in the box that equal to what we want.
                for (h = 0; h < numPink; ++h)
                {
                    if (pab->angles[h] != index)
                        continue;
                    k = h;
                    //If found one, go through for next angles
                    for (i = 1; i < numStationaryRecordedAngle-1; ++i)
                    {
                        ++k;
                        if (k == numPink || pab->angles[k] == -1)
                        {
                            k = 0;
                            //Should it give indication so we don't interate twice ??
                        }
                        if (ABS(pab->angles[k] - ABS(stationaryRecordedAngle[i] - stationaryRecordedAngle [i+1])) > angleRange)
                        {
                            break;
                        }
                    }
                    if (i == numStationaryRecordedAngle-1)
                    {
                        pos(0, 0) = pab->posX;
                        pos(1, 0) = pab->posY;
                        pos(2, 0) = DEG2RAD(NormalizeAngle_0_360(pab->anglesToPink[h] - stationaryRecordedAngle[0]));

                        weight = vision->calcRobotMatch(pos, grad);
                        if (weight > bestweight)
                        {
                            bestpos = pos;
                            bestweight = weight;
                        }
#ifdef OFFLINE                
                        cout << __func__ << " possibility x:" << pos(0, 0) << " y:" << pos(1, 0) << " h:" << RAD2DEG(pos(2, 0)) << " weight:" << weight << endl;
#endif                
                        
                        //calcRobotMatch();
                        //Do input to gaussians here
                    }    
                    
                }                
            }
        }
    }
    if (bestweight > 0)
    {
        
        cout << __func__ << " Best Possibility x:" << bestpos(0,0) << " y:" << bestpos(1,0) << " h:" <<
            RAD2DEG(bestpos(2,0)) << " weight:" << bestweight << endl;
    }
}

bool GPS:: CalcLineEquation(VisualObject *pvo, double *m, double *c, int *type)
{
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
    
    double dotProd, rayHead, x, y, rayX, rayY;
    double intersectX,intersectY;
    int curPoint;
    x = 0;
    // We should make sure self variance is small if we are going to record
    // the position of this blob.
    if (r[0].mean.posVar > 500.0 || r[0].mean.hVar > 300.0)
    {
#ifdef OFFLINE
        cout << __func__ << " Variance is too big" <<endl;
#endif        
        return false;
    }
    
#ifdef OFFLINE
    cout << __func__ << " pvo->head:"<<pvo->head <<endl;
#endif        
    // Calculate ray direction parameters
    rayHead = NormalizeAngle_0_360( r[0].mean.h + pvo->head);
    rayX = cos(DEG2RAD(rayHead));
    rayY = sin(DEG2RAD(rayHead));

    // Make sure ray doesn't intersect a beacon position
    for (curPoint = 0; curPoint < 8; curPoint += 2) {
        // Make sure beacon is in ray direction
        intersectX = beaconPos[curPoint] - r[0].mean.pos.x;
        intersectY = beaconPos[curPoint + 1] - r[0].mean.pos.y;
#ifdef OFFLINE
        cout << __func__ << " :"<<curPoint<<": intersectX:" <<intersectX<<" intersectY:"<<intersectY <<endl;
#endif        

        if (intersectX * rayX + intersectY * rayY >= 0) {
            // If perpendicular distance from ray to beacon is less than a
            // threshold value, we might have accidentally taken the pink blob
            // from a beacon, so don't use it.
            dotProd = intersectX * rayY - intersectY * rayX;
            if (ABS(dotProd) < 25) 
            {
#ifdef OFFLINE
                cout << __func__ << " Near beacon position" <<endl;
#endif        
                return false;
            }
        }
    }
    
    *m = tan(DEG2RAD(rayHead));
    *c =r[0].mean.pos.y - (*m) * r[0].mean.pos.x;
    
    if (rayHead <=90)
    {
        //Check if intersect with the top line
        if ( ABS(*m) > EPSILON)
        {
            y = (double) FIELD_LENGTH;
            x = (y - *c) / *m;
        }
        if ( x < FIELD_WIDTH)
            *type = FLUp;
        else
            *type = FLRight;
    }
    else if (rayHead < 180)
    {
        //Check if intersect with the top line
        if ( ABS(*m) > EPSILON)
        {
            y = (double) FIELD_LENGTH;
            x = (y - *c) / *m;
        }
        if ( x > 0)
            *type = FLUp;
        else
            *type = FLLeft;
    }
    else if (rayHead < 270)
    {
        //Check if intersect with the bottom
        if ( ABS(*m) > EPSILON)
        {
            y = 0.0;
            x = (y - *c) / *m;
        }
        if ( x > 0)
            *type = FLDown;
        else
            *type = FLLeft;
    }
    else
    {
        //Check if intersect with the bottom
        if ( ABS(*m) > EPSILON)
        {
            y = 0.0;
            x = (y - *c) / *m;
        }
        if ( x < FIELD_WIDTH)
            *type = FLDown;
        else
            *type = FLRight;
    }
    return true;
}

void GPS::MapPinkEquations(VisualObject *pvo, int numPinkVob) {
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
    
    double m, c;
    int type;
    PinkEQ peq;
    pinkUpdate = false;
#ifdef OFFLINE
    cout << __func__ << " started. allowpinkmapping:" << allowPinkMapping << endl;
#endif    
    if (!allowPinkMapping) return;
    for (int i = 0; i < numPinkVob; ++i) {
        if(CalcLineEquation(&pvo[i], &m, &c,&type))
        {
            cout << __func__ << "["<<listPinkEQ.size() << "] m:" << m << " c:" << c << " type:" << type << endl;
            peq.m = m;
            peq.c = c;
            peq.type = type;
            listPinkEQ.push_back(peq);
            pinkUpdate = true;
        }      
    }
}

void GPS::PinkBoxBlobbing(int wholeField[PINK_ARRAY_BOX_SIZE_X][PINK_ARRAY_BOX_SIZE_Y], int boxPosX, int boxPosY, int minimum, PinkBlob *pb, int from)
{
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
    
    if (pb->maxX < boxPosX)
        pb->maxX = boxPosX;
    if (pb->minX > boxPosX)
        pb->minX = boxPosX;
    if (pb->maxY < boxPosY)
        pb->maxY = boxPosY;
    if (pb->minY > boxPosY)
        pb->minY = boxPosY;
        
    wholeField[boxPosX][boxPosY] = 0;
    
    if (from != FLUp && boxPosY+1<PINK_ARRAY_BOX_SIZE_Y && wholeField[boxPosX][boxPosY+1] > minimum)
        PinkBoxBlobbing(wholeField,boxPosX,boxPosY+1, minimum, pb, FLDown);
        
    if (from != FLDown && boxPosY-1>0 && wholeField[boxPosX][boxPosY-1] > minimum)
        PinkBoxBlobbing(wholeField,boxPosX,boxPosY-1, minimum, pb, FLUp);
        
    if (from != FLLeft && boxPosX-1>0 && wholeField[boxPosX-1][boxPosY] > minimum)
        PinkBoxBlobbing(wholeField,boxPosX-1,boxPosY, minimum, pb, FLRight);
        
    if (from != FLRight && boxPosX+1<PINK_ARRAY_BOX_SIZE_X && wholeField[boxPosX+1][boxPosY] > minimum)
        PinkBoxBlobbing(wholeField,boxPosX+1,boxPosY, minimum, pb, FLLeft);       
}

void GPS::PinkLineToBox(vector< PinkEQ > &rlistPinkEQ, int wholeField[PINK_ARRAY_BOX_SIZE_X][PINK_ARRAY_BOX_SIZE_Y])
{
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
   
    PinkEQ firstEq;
    double posX, posY, sPosX, sPosY, diffX,diffY;

    for (int indexX = 0; indexX < PINK_ARRAY_BOX_SIZE_X; ++indexX)
        for (int indexY = 0; indexY < PINK_ARRAY_BOX_SIZE_Y; ++indexY)
            wholeField[indexX][indexY] = 0;
    for (int firstIndex = 0; firstIndex < (int)rlistPinkEQ.size(); ++firstIndex)
    {
        firstEq = rlistPinkEQ[firstIndex];
        if (firstEq.type == FLLeft)
        {
            posX = -BOXSIZE_X*1.0;
            posY = sPosY = posX * firstEq.m + firstEq.c;
            diffY = -BOXSIZE_X * firstEq.m;
            //Do the horizontal
            for (; posX > -OUTER_BARRIER_X && posY > -OUTER_BARRIER_Y && posY < FIELD_LENGTH+OUTER_BARRIER_Y; posX -= BOXSIZE_X, posY +=diffY)
            {
#ifdef INTENSISVE_CHECK
                int boxX = (int)(posX/BOXSIZE_X) + OFFSET_BOX_X;
                int boxY = (int)(posY/BOXSIZE_Y)+ OFFSET_BOX_Y;
                if (boxY < 0 || boxY >=PINK_ARRAY_BOX_SIZE_Y || boxX < 0 || boxX >= PINK_ARRAY_BOX_SIZE_X)
                {
#ifdef OFFLINE                
                    cerr << __func__ << " type:" << firstEq.type << " m:" << firstEq.m << " c:" << firstEq.c << endl;
                    cerr << __func__ << " posX:" << posX << " posY:" << posY << endl;
                    cerr << __func__ << " box x:" << boxX << " y:" << boxY << endl;
#endif                    
                    break;
                }
#endif                    
                ++wholeField[(int)(posX/BOXSIZE_X) + OFFSET_BOX_X][(int)(posY/BOXSIZE_Y)+OFFSET_BOX_Y];
            }
            if (firstEq.m < 0) //Going up we start from above line
            {
                posY = ((int)(sPosY / BOXSIZE_Y)) * 1.0 * (BOXSIZE_Y+1);
                diffY = BOXSIZE_Y;
            }
            else //if going down we start from the bottom line
            {
                posY = ((int)(sPosY / BOXSIZE_Y)) * 1.0 * BOXSIZE_Y;
                diffY = -BOXSIZE_Y;
            }
            
            // If it is almost flat, skip checking X
            if (ABS(firstEq.m) < EPSILON) 
                continue;
            
            posX = (posY - firstEq.c) / firstEq.m;
            diffX = diffY/firstEq.m;
            //Do the vertical
            for (; posX > -OUTER_BARRIER_X && (posY+diffY) > -OUTER_BARRIER_Y && (posY+diffY) < FIELD_LENGTH+OUTER_BARRIER_Y; posY += diffY, posX+=diffX)
            {
                int boxX = (int)(posX/BOXSIZE_X) + OFFSET_BOX_X;
                int boxY = (int)((posY+diffY)/BOXSIZE_Y)+ OFFSET_BOX_Y;
#ifdef INTENSISVE_CHECK
                if (boxY < 0 || boxY >=PINK_ARRAY_BOX_SIZE_Y || boxX < 0 || boxX >= PINK_ARRAY_BOX_SIZE_X)
                {
#ifdef OFFLINE                
                    cerr << __func__ << " type:" << firstEq.type << " m:" << firstEq.m << " c:" << firstEq.c << endl;
                    cerr << __func__ << " posX:" << posX << " posY:" << posY << endl;
                    cerr << __func__ << " box x:" << boxX << " y:" << boxY << endl;
#endif                    
                    break;
                }
#endif                    
                ++wholeField[boxX][boxY];
            }            
        }
        else if (firstEq.type == FLRight)
        {
            posX = FIELD_WIDTH+BOXSIZE_X*1.0;
            posY = sPosY = posX * firstEq.m + firstEq.c;
            diffY = BOXSIZE_X * firstEq.m;
            diffX = BOXSIZE_X;
            //Do the horizontal
            for (; posX < FIELD_WIDTH+OUTER_BARRIER_X && posY > -OUTER_BARRIER_Y && posY < FIELD_LENGTH+OUTER_BARRIER_Y;  posY += diffY, posX+=diffX)
            {
                int boxX = (int)(posX/BOXSIZE_X) + OFFSET_BOX_X;
                int boxY = (int)(posY/BOXSIZE_Y) + OFFSET_BOX_Y;
#ifdef INTENSISVE_CHECK
                if (boxY < 0 || boxY >=PINK_ARRAY_BOX_SIZE_Y || boxX < 0 || boxX >= PINK_ARRAY_BOX_SIZE_X)
                {
#ifdef OFFLINE                
                    cerr << __func__ << " type:" << firstEq.type << " m:" << firstEq.m << " c:" << firstEq.c << endl;
                    cerr << __func__ << " posX:" << posX << " posY:" << posY << endl;
                    cerr << __func__ << " box x:" << boxX << " y:" << boxY << endl;
#endif                    
                    break;
                }
#endif                    
                ++wholeField[boxX][boxY];
            }
            if (firstEq.m > 0) //Going up we start from above line
            {
                posY = ((int)(sPosY / BOXSIZE_Y)) * 1.0 * (BOXSIZE_Y+1);
                diffY = BOXSIZE_Y;
            }
            else //if going down we start from the bottom line
            {
                posY = ((int)(sPosY / BOXSIZE_Y)) * 1.0 * BOXSIZE_Y;
                diffY = -BOXSIZE_Y;
            }
            
            // If it is almost flat, skip checking X
            if (ABS(firstEq.m) < EPSILON) 
                continue;
            
            posX = (posY - firstEq.c) / firstEq.m;
            diffX = diffY/firstEq.m;
            //Do the vertical
            for (; posX < FIELD_WIDTH+OUTER_BARRIER_X && (posY+diffY) > -OUTER_BARRIER_Y && (posY+diffY) < FIELD_LENGTH+OUTER_BARRIER_Y; posY += diffY, posX+=diffX)
            {
                int boxX = (int)(posX/BOXSIZE_X) + OFFSET_BOX_X;
                int boxY = (int)((posY+diffY)/BOXSIZE_Y)+ OFFSET_BOX_Y;
#ifdef INTENSISVE_CHECK
                if (boxY < 0 || boxY >=PINK_ARRAY_BOX_SIZE_Y || boxX < 0 || boxX >= PINK_ARRAY_BOX_SIZE_X)
                {
#ifdef OFFLINE                
                    cerr << __func__ << " type:" << firstEq.type << " m:" << firstEq.m << " c:" << firstEq.c << endl;
                    cerr << __func__ << " posX:" << posX << " posY:" << posY << endl;
                    cerr << __func__ << " box x:" << boxX << " y:" << boxY << endl;
#endif                    
                    break;
                }
#endif                    
                ++wholeField[boxX][boxY];
            }        
        }
        else if (firstEq.type == FLUp)
        {
            if (ABS(firstEq.m) < EPSILON)
                continue;
            posY = FIELD_LENGTH+BOXSIZE_Y*1.0;
            posX = sPosX = (posY - firstEq.c) / firstEq.m;
            diffY = BOXSIZE_Y;
            diffX = diffY / firstEq.m;
            /*
            cerr << __func__ << "vertical" << endl;
            cerr << __func__ << "PINK_ARRAY_BOX_SIZE_X" << PINK_ARRAY_BOX_SIZE_X << endl;
            cerr << __func__ << "PINK_ARRAY_BOX_SIZE_Y" << PINK_ARRAY_BOX_SIZE_Y << endl;
            */
            //Do the vertical
            for (;posY < FIELD_LENGTH+OUTER_BARRIER_Y && posX > -OUTER_BARRIER_X && posX < FIELD_WIDTH+OUTER_BARRIER_X; posX += diffX, posY +=diffY)
            {
                int boxX = (int)(posX/BOXSIZE_X) + OFFSET_BOX_X;
                int boxY = (int)(posY/BOXSIZE_Y) + OFFSET_BOX_Y;
                /*
                cerr << __func__ << " type:" << firstEq.type << " m:" << firstEq.m << " c:" << firstEq.c << endl;
                cerr << __func__ << " posX:" << posX << " posY:" << posY << endl;
                cerr << __func__ << " box x:" << boxX << " y:" << boxY << endl;
                */
#ifdef INTENSISVE_CHECK
                if (boxY < 0 || boxY >=PINK_ARRAY_BOX_SIZE_Y || boxX < 0 || boxX >= PINK_ARRAY_BOX_SIZE_X)
                {
#ifdef OFFLINE              
                    cerr << __func__ << " ::ERROR::" << endl;      
                    cerr << __func__ << " type:" << firstEq.type << " m:" << firstEq.m << " c:" << firstEq.c << endl;
                    cerr << __func__ << " PosX:" << posX << " posY:" << posY << endl;
                    cerr << __func__ << " BOX x:" << boxX << " y:" << boxY << endl;
#endif                    
                    break;
                }
#endif                    
                ++wholeField[boxX][boxY];
            }
            if (firstEq.m < 0) //Going left we start from leftline
            {
                posX = ((int)(sPosX / BOXSIZE_X)) * 1.0 * BOXSIZE_X;
                diffX = -BOXSIZE_X;
            }
            else //if going right we start from the right line
            {
                posX = ((int)(sPosX) / BOXSIZE_X) * 1.0 * (BOXSIZE_X+1);
                diffX = BOXSIZE_X;
            }
            
            // If it is almost vertical, skip checking X
            if (ABS(firstEq.m) > PINK_ARRAY_BOX_SIZE_Y) 
                continue;
                       
            posY = posX * firstEq.m + firstEq.c;
            diffY = diffX * firstEq.m;
            
            //Do the horizontal
            for (;posY < FIELD_LENGTH+OUTER_BARRIER_Y && (posX+diffX) > -OUTER_BARRIER_X && (posX+diffX) < FIELD_WIDTH+OUTER_BARRIER_X; posX += diffX, posY +=diffY)
            {
                int boxX = (int)((posX+diffX)/BOXSIZE_X) + OFFSET_BOX_X;
                int boxY = (int)((posY)/BOXSIZE_Y)+ OFFSET_BOX_Y;
                /*                
                cerr << __func__ << " type:" << firstEq.type << " m:" << firstEq.m << " c:" << firstEq.c << endl;
                cerr << __func__ << " posX:" << posX << " posY:" << posY << endl;
                cerr << __func__ << " box x:" << boxX << " y:" << boxY << endl;
                cerr << __func__ << " diffX:" << diffX << " diffY:" << diffY << endl;
                */
#ifdef INTENSISVE_CHECK

                if (boxY < 0 || boxY >=PINK_ARRAY_BOX_SIZE_Y || boxX < 0 || boxX >= PINK_ARRAY_BOX_SIZE_X)
                {
#ifdef OFFLINE                
                    cerr << __func__ << " ::ERROR::" << endl;      
                    cerr << __func__ << " type:" << firstEq.type << " m:" << firstEq.m << " c:" << firstEq.c << endl;
                    cerr << __func__ << " PosX:" << posX << " posY:" << posY << endl;
                    cerr << __func__ << " BOX x:" << boxX << " y:" << boxY << endl;
#endif //OFFLINE
                    break;
                }                
#endif //  INTENSISVE_CHECK        
                ++wholeField[boxX][boxY];
            } 
                       
        }
        else if (firstEq.type == FLDown)
        {
            if (ABS(firstEq.m) < EPSILON)
                continue;
            posY = -BOXSIZE_Y;
            posX = sPosX = (posY - firstEq.c) / firstEq.m;
            diffY = -BOXSIZE_Y;
            diffX = diffY / firstEq.m;
            //Do the vertical
            //cerr << __func__ << "VERTICAL" << endl;
            for (;posY > -OUTER_BARRIER_Y && posX > -OUTER_BARRIER_X && posX < FIELD_WIDTH+OUTER_BARRIER_X; posX += diffX, posY +=diffY)
            {
                int boxX = (int)(posX/BOXSIZE_X) + OFFSET_BOX_X;
                int boxY = (int)(posY/BOXSIZE_Y) + OFFSET_BOX_Y;
#ifdef INTENSISVE_CHECK
                if (boxY < 0 || boxY >=PINK_ARRAY_BOX_SIZE_Y || boxX < 0 || boxX >= PINK_ARRAY_BOX_SIZE_X)
                {
#ifdef OFFLINE                
                    cerr << __func__ << " type:" << firstEq.type << " m:" << firstEq.m << " c:" << firstEq.c << endl;
                    cerr << __func__ << " posX:" << posX << " posY:" << posY << endl;
                    cerr << __func__ << " box x:" << boxX << " y:" << boxY << endl;
#endif                    
                    break;
                }
#endif                                    
                ++wholeField[boxX][boxY];
            }
            if (firstEq.m > 0) //Going left we start from left line
            {
                posX = ((int)(sPosX / BOXSIZE_X)) * 1.0 * BOXSIZE_X;
                diffX = -BOXSIZE_X;
            }
            else //if going right we start from the right line
            {
                posX = ((int)(sPosX) / BOXSIZE_X) * 1.0 * (BOXSIZE_X+1);
                diffX = BOXSIZE_X;
            }
            
            // If it is almost vertical, skip checking X
            if (ABS(firstEq.m) > PINK_ARRAY_BOX_SIZE_Y) 
                continue;
                       
            posY = posX * firstEq.m + firstEq.c;
            diffY = diffX * firstEq.m;
           
            //Do the horizontal            
            for (;posY > -OUTER_BARRIER_Y && (posX+diffX) > -OUTER_BARRIER_X && (posX+diffX) < FIELD_WIDTH+OUTER_BARRIER_X; posX += diffX, posY +=diffY)
            {
                int boxX = (int)((posX+diffX)/BOXSIZE_X) + OFFSET_BOX_X;
                int boxY = (int)((posY)/BOXSIZE_Y)+ OFFSET_BOX_Y;                
#ifdef INTENSISVE_CHECK
                if (boxY < 0 || boxY >=PINK_ARRAY_BOX_SIZE_Y || boxX < 0 || boxX >= PINK_ARRAY_BOX_SIZE_X)
                {
#ifdef OFFLINE                
                    cerr << __func__ << " type:" << firstEq.type << " m:" << firstEq.m << " c:" << firstEq.c << endl;
                    cerr << __func__ << " posX:" << posX << " posY:" << posY << endl;
                    cerr << __func__ << " box x:" << boxX << " y:" << boxY << endl;
#endif                    
                    break;
                }
#endif                    
                ++wholeField[boxX][boxY];
            }
            
        }
#ifdef OFFLINE        
        for (int indexX = OFFSET_BOX_X + 1 ; indexX < PINK_ARRAY_BOX_SIZE_X - OFFSET_BOX_X; ++indexX)
        {
           for (int indexY = OFFSET_BOX_Y + 1; indexY < PINK_ARRAY_BOX_SIZE_Y - OFFSET_BOX_Y; ++indexY)
           {
                if (wholeField[indexX][indexY] != 0)
                {
                    cerr << __func__ << "ERROR PINK ON THE FIELD" << endl;
                    cerr << __func__ << " type:" << firstEq.type << " m:" << firstEq.m << " c:" << firstEq.c << endl;
                    cerr << __func__ << " box x:" << indexX << " y:" << indexY << endl;
                    return;
                }
           }
        }
#endif           
    }
}

void GPS::PinkFindBlobBox(vector< PinkBlob > &listPinkBlob, int wholeField[PINK_ARRAY_BOX_SIZE_X][PINK_ARRAY_BOX_SIZE_Y])
{
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
    
    PinkBlob pb;
    //Get the blobbing
    for (int indexX = 0; indexX < PINK_ARRAY_BOX_SIZE_X; ++indexX)
    {
        for (int indexY = 0; indexY < PINK_ARRAY_BOX_SIZE_Y; ++indexY)
        {
            if (wholeField[indexX][indexY] > minimumPinkInBox)
            {
                pb.minX  = pb.maxX  = indexX;
                pb.minY  = pb.maxY  = indexY;
                PinkBoxBlobbing(wholeField,indexX,indexY, minimumPinkInBox, &pb, FLLeft);
                listPinkBlob.push_back(pb);
                indexY = pb.maxX;
            }
        }
    }
}

int GPS::PinkFindPosition(vector< PinkBlob > &listPinkBlob, double *posPinkX, double *posPinkY, int *posPinkConf, int maxPosPink)
{
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
    int numPosPink = 0;
    double x,y,dx,dy,distSq,scale;
    bool merged = false;
    PinkBlob pb;
    //For every blob, calculate the position.
    for(vector < PinkBlob >::iterator iter = listPinkBlob.begin(); iter != listPinkBlob.end() && numPosPink < maxPosPink; ++iter)
    {
        pb = *iter;
        x = (pb.maxX+pb.minX)*BOXSIZE_X/2 - OUTER_BARRIER_X;
        y = (pb.maxY+pb.minY)*BOXSIZE_Y/2 - OUTER_BARRIER_Y;
        merged = false;
#ifdef OFFLINE 
        cout<<__func__ << " Pink Blob x:" << x << " y:" << y << endl;
#endif        
        // For every valid pink blob, see if it can be merged with one
        // of the mapped pink blobs, by checking distance from each.
        for (int j = 0; j < numPosPink && (!merged); j++) {
            dx = x - posPinkX[j]; dy = y - posPinkY[j];
            distSq = dx * dx + dy * dy;
            
            // If this pink blob is close enough to be merged with another,
            // then use it in the running average.
            if (merged = (distSq < mergeDistSq)) {
                scale = 1.0 / (1 + pinkConf[j]);
                posPinkX[j] += dx * scale;
                posPinkY[j] += dy * scale;
                posPinkConf[j]++; // Increase confidence value
            }
        }

        // If the blob couldn't be merged with another, start a new one. 
        if (!merged && numPosPink < maxPosPink) {
            posPinkX[numPosPink] = x;
            posPinkY[numPosPink] = y;
            posPinkConf[numPosPink] = 1;
            numPosPink++;
        }            
    }
    return numPosPink;
}

/* This function will sort the pink according to angle start from 0 to 360. Angle is calculated from position 0,0. */
void GPS::SortAndCalculatePink()
{
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
    
    if (numPink < 1)
        return;
        
    double angle[numPink];
    double tempx, tempy, tempangle;
    int index, j,tempconf;
    //Get all the angle
    for (index = 0; index < numPink; ++index) 
    {
        if (ABS(pinkX[index] - FIELD_WIDTH/2) < EPSILON)
            pinkX[index] += EPSILON;
        angle[index] = (RAD2DEG(atan((pinkY[index] - FIELD_LENGTH/2) / (pinkX[index] - FIELD_WIDTH/2))));
        if (pinkX[index] - FIELD_WIDTH/2 < 0)
        {   
            angle[index] += FULL_CIRCLE/2;
        }
        else if (pinkY[index] - FIELD_LENGTH/2 < 0)
        {
            angle[index] += FULL_CIRCLE;
        }
    }
#ifdef OFFLINE    
    cout << "BEFORE SORT Found " << numPink << " pink landmarks.\n";
    for (int i = 0; i < numPink; i++) {
        cout << "Landmark " << i;
        cout << " - x: " << pinkX[i] << "   y: " << pinkY[i];
        cout << "   angle: " << angle[i] << "\n";
    }
#endif
    
    //Insertion sort, number of pink only < 20 recursion (merge sort) could be slower due to re-allocate memory.
    for (index = 1; index < numPink; ++index)
    {
        tempangle = angle[index];
        tempx = pinkX[index];
        tempy = pinkY[index];
        tempconf = pinkConf[index];
        j = index;
        while ((j > 0) && (angle[j-1] > tempangle))
        {
            angle[j] = angle[j-1];
            pinkX[j] = pinkX[j-1];
            pinkY[j] = pinkY[j-1];
            pinkConf[j] = pinkConf[j-1];
            j = j - 1;
        }
        angle[j] = tempangle;
        pinkX[j] = tempx;
        pinkY[j] = tempy;
        pinkConf[j] = tempconf;
    }
    //Calculate distance between pink, sorted
    for (index = 0; index < numPink - 1; ++index)
    {
        distanceBetweenPink[index] = SQUARE(pinkX[index] - pinkX[index+1]) + SQUARE(pinkY[index] - pinkY[index+1]);
    }
    //circular include distace of the last and the first one.
    if (numPink > 0)
        distanceBetweenPink[numPink-1] = SQUARE(pinkX[numPink-1] - pinkX[0]) + SQUARE(pinkY[numPink-1] - pinkY[0]);
        
#ifdef OFFLINE            
    cout << "AFTER SORT Found " << numPink << " pink landmarks.\n";
    for (int i = 0; i < numPink; i++) {
        cout << "Landmark " << i;
        cout << " - x: " << pinkX[i] << "   y: " << pinkY[i];
        cout << "   angle: " << angle[i] << "\n";
    }
#endif    
}

void GPS::PinkCalcAngles()
{
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        
    
    int y,x,index, biggest;
    double distA, distB, distFirst, anAngle, biggestAngle, totalAngle, dx, dy;
    PinkAngleBox *pab;
    if (numPink < 2)
        return;
    /*
    cout << __func__ << " numpink:"<< numPink << endl;    
    cout << __func__ << " ANGLE_ARRAY_SIZE_Y:"<< ANGLE_ARRAY_SIZE_Y << endl;    
    cout << __func__ << " ANGLE_ARRAY_SIZE_X:"<< ANGLE_ARRAY_SIZE_X << endl;    
    cout << __func__ << " size of angleBoxPosition:"<< sizeof(angleBoxPosition) << endl;    
    */
    for (y = 0; y < ANGLE_ARRAY_SIZE_Y; ++y)
    {
        for (x = 0; x < ANGLE_ARRAY_SIZE_X; ++x)
        {
            pab = &angleBoxPosition[y*ANGLE_ARRAY_SIZE_X + x];
            pab->posX = x * ANGLE_BOXSIZE_X;
            pab->posY = y * ANGLE_BOXSIZE_Y;
            
            //cout << __func__ << " x:" << x << " y:" << y << " posx:" <<pab->posX << " pab->posY:" << pab->posY << endl;
            
            index = 0;
            biggestAngle = 0;
            biggest = 0;
            totalAngle = 0;
               
            dx = pab->posX - pinkX[index];
            dy = pab->posY - pinkY[index];
            distA = distFirst = SQUARE(dx) + SQUARE(dy);
            for (index = 0; index < numPink-1; ++index)
            {
                
                if (ABS(dx) < EPSILON)
                    dx = EPSILON;
                if (dx < 0)
                    pab->anglesToPink[index] = (int) (RAD2DEG(atan(dy/dx)) + FULL_CIRCLE/2);
                else if (dy < 0)
                    pab->anglesToPink[index] = (int) (RAD2DEG(atan(dy/dx)) + FULL_CIRCLE);
                else
                    pab->anglesToPink[index] = (int) (RAD2DEG(atan(dy/dx)));
                   
                dx = pab->posX - pinkX[index+1];
                dy = pab->posY - pinkY[index+1];
                distB = SQUARE(dx) + SQUARE (dy);
                
                anAngle = RAD2DEG(acos((distA+distB - distanceBetweenPink[index])/(2*sqrt(distA)*sqrt(distB))));
                ANGLE_FIX(anAngle);
                
                //cout << __func__ << " index:" << index << " distA:" << distA << " distB:" << distB << " index:" << index << " distanceBetweenPink[index]:" << distanceBetweenPink[index] << " angle:" <<anAngle<< endl;
                if (biggestAngle < anAngle) 
                {
                    biggestAngle = anAngle;
                    biggest = index;
                }
                distA = distB;
                pab->angles[index] = (int) anAngle;
                
                totalAngle += anAngle;
            }
            if (numPink > 0)
            {
                
                if (ABS(dx) < EPSILON)
                    dx = EPSILON;
                if (dx < 0)
                    pab->anglesToPink[index] = (int) (RAD2DEG(atan(dy/dx)) + FULL_CIRCLE/2);
                else if (dy < 0)
                    pab->anglesToPink[index] = (int) (RAD2DEG(atan(dy/dx)) + FULL_CIRCLE);
                else
                    pab->anglesToPink[index] = (int) (RAD2DEG(atan(dy/dx)));
                
                anAngle = RAD2DEG(acos((distA+distFirst - distanceBetweenPink[numPink-1])/(2*sqrt(distA)*sqrt(distFirst))));
                ANGLE_FIX(anAngle);
                //cout << __func__ << " index:" << index << " distA:" << distA << " distFirst:" << distFirst << " index:" << index << " distanceBetweenPink[index]:" << distanceBetweenPink[index] << " angle:" <<anAngle<< endl;
                if (biggestAngle < anAngle) 
                {
                    biggestAngle = anAngle;
                    biggest = numPink-1;
                }
                pab->angles[numPink-1] = (int) anAngle;
                totalAngle += anAngle;
            }
            
            //Check if there is false angle (the angle between it bigger than 180)
            if (totalAngle < 360 - EPSILON)
            {
                //cout << __func__ << " totalangle map is less than 360:" << totalAngle << " biggest:" << biggest << endl;
                while (biggest < numPink - 1)
                {
                    //cout << __func__ << " biggest:" << biggest << " pab->angles[biggest]:" << pab->angles[biggest]<< " = " << pab->angles[biggest+1] <<endl;
                    pab->angles[biggest] = pab->angles[biggest+1];
                    ++biggest;
                } 
                pab->angles[biggest] = -1;
            } 
            else if (numPink < maxPink)
            {
                pab->angles[numPink] = -1;
            }
            for (index = 0; index < numPink; ++index)
            {
                if (pab->angles[index] == -1)
                    break;
                //cout << __func__ << " x:" << x << " y:" << y << " index:" << index << " pab->angles[index]:" << pab->angles[index] << endl;
                
                if (anglelist.find(pab->angles[index]) != anglelist.end())
                {
                    anglelist[pab->angles[index]].push_back(pab);
                }
                else
                {
                    vector < PinkAngleBox* > vecPab;
                    vecPab.push_back(pab);
                    anglelist[pab->angles[index]] = vecPab;
                }
            }
            
        }
    }
#ifdef OFFLINE    
    for (map < int, vector < PinkAngleBox * > >::iterator  iter =  anglelist.begin(); iter != anglelist.end(); ++iter)
    {
        cout << __func__ << " angle:" << iter->first << " size:" << iter->second.size() << endl;
    }
#endif
#ifdef FN_DEBUG
    cout << __func__ << " end." << endl;
#endif
}

void GPS::CalcIntersections()
{
#ifdef OFFLINE
    if (DEBUG)
        cerr << __func__ << endl;
#endif        

#ifdef SECOND_WAY

#if 1
    PinkEQ firstEq,secondEq;
    double x,dx,dy,distSq,scale;
    double y;
    bool merged;
    numPink = 0;
#ifdef OFFLINE    
    cout << __func__ << " size:" << listPinkEQ.size() << endl;    
#endif    
    for ( int firstIndex = 0; firstIndex < ((int)listPinkEQ.size() - 1); ++firstIndex)
    {
        firstEq = listPinkEQ[firstIndex];
        for ( int secondIndex = firstIndex+1; secondIndex < (int)listPinkEQ.size(); ++secondIndex)
        {
            secondEq = listPinkEQ[secondIndex];
            
            //Check if opposite direction
            if ((firstEq.type == FLUp && secondEq.type == FLDown) || 
                (firstEq.type == FLDown && secondEq.type == FLUp) || 
                (firstEq.type == FLLeft && secondEq.type == FLRight) ||
                (firstEq.type == FLRight && secondEq.type == FLLeft) ||
                (ABS(firstEq.m - secondEq.m) < EPSILON)
                )
                continue;
                
            //Check if adjacent side but line pass the field
            if (firstEq.type == FLUp && secondEq.type != FLUp)
            {
                if (ABS(firstEq.m) < EPSILON)
                    continue;
                x = (FIELD_LENGTH-firstEq.c) / firstEq.m;
                if (x > 0 && x < FIELD_WIDTH)
                    continue;
            }
            else if (firstEq.type != FLUp && secondEq.type == FLUp)
            {
                if (ABS(secondEq.m) < EPSILON)
                    continue;
                x = (FIELD_LENGTH-secondEq.c) / secondEq.m;
                if (x > 0 && x < FIELD_WIDTH)
                    continue;            
            }
            else if (firstEq.type == FLDown && secondEq.type != FLDown)
            {
                if (ABS(secondEq.m) < EPSILON)
                    continue;
                x = (-secondEq.c) / secondEq.m;
                if (x > 0 && x < FIELD_WIDTH)
                    continue;            
            }
            else if (firstEq.type != FLDown && secondEq.type == FLDown)
            {
                if (ABS(secondEq.m) < EPSILON)
                    continue;
                x = (-secondEq.c) / secondEq.m;
                if (x > 0 && x < FIELD_WIDTH)
                    continue;            
            }
            
            x = (secondEq.c - firstEq.c)/(firstEq.m - secondEq.m);
            y = firstEq.m * x + firstEq.c;
            if (firstEq.type == FLLeft)
            {
                if (
                    x > 0 || 
                    x < -OUTER_BARRIER_X || 
                    y < -OUTER_BARRIER_Y || 
                    y > FIELD_LENGTH+OUTER_BARRIER_Y
                    )
                    continue;
            }
            else if (firstEq.type == FLRight)
            {
                if (
                    x > FIELD_WIDTH+OUTER_BARRIER_X || 
                    x < FIELD_WIDTH || 
                    y < -OUTER_BARRIER_Y || 
                    y > FIELD_LENGTH+OUTER_BARRIER_Y
                    )
                    continue;
            }    
            else if (firstEq.type == FLUp)
            {
                if (
                    x > FIELD_WIDTH+OUTER_BARRIER_X || 
                    x < -OUTER_BARRIER_X || 
                    y < FIELD_LENGTH || 
                    y > FIELD_LENGTH+OUTER_BARRIER_Y
                    )
                    continue;
            }    
            else if (firstEq.type == FLDown)
            {
                if (
                    x > FIELD_WIDTH+OUTER_BARRIER_X || 
                    x < -OUTER_BARRIER_X || 
                    y < -OUTER_BARRIER_Y || 
                    y > 0
                    )
                    continue;
            }    
            //cout << __func__ << " intersection x:" << x << " y:" << y << endl;
            merged = false;
            
            // For every valid pink blob, see if it can be merged with one
            // of the mapped pink blobs, by checking distance from each.
            for (int j = 0; j < numPink && (!merged); j++) {
                dx = x - pinkX[j]; dy = y - pinkY[j];
                //Only consider the side or up distance accordingly
                if (firstEq.type == FLDown || firstEq.type == FLUp)
                    distSq = dx * dx;
                else
                    distSq = dy * dy;
                
                // If this pink blob is close enough to be merged with another,
                // then use it in the running average.
                if (merged = (distSq < mergeDistSq)) {
                    scale = 1.0 / (1 + pinkConf[j]);
                    
                    //Merging while finding the nearest to the field               
                    if (firstEq.type == FLDown || firstEq.type == FLUp)
                    {
                        pinkX[j] += dx * scale;
                        //Find the nearest to the field
                        if (firstEq.type == FLDown)
                        {
                            if (pinkY[j] < y)
                                pinkY[j] = y;
                        } 
                        else 
                        {
                            if (pinkY[j] > y)
                                pinkY[j] = y;
                        }
                    }
                    else                        
                    {
                        pinkY[j] += dy * scale;
                        //Find the nearest to the field
                        if (firstEq.type == FLLeft)
                        {
                            if (pinkX[j] < x)
                                pinkX[j] = x;
                        } 
                        else 
                        {
                            if (pinkX[j] > x)
                                pinkX[j] = x;
                        }
                    }

                    /*
                    pinkX[j] += dx * scale;
                    pinkY[j] += dy * scale;
                    */
                    pinkConf[j]++; // Increase confidence value
                }
            }

            // If the blob couldn't be merged with another, start a new one. 
            if (!merged && numPink < maxPink) {
                pinkX[numPink] = x;
                pinkY[numPink] = y;
                pinkConf[numPink] = 1;
                numPink++;
            } 
        }
    }
        
    
#ifdef OFFLINE    
    cout << "BEFORE CUT LOW CONFIDENCE Found " << numPink << " pink landmarks.\n";
    for (int i = 0; i < numPink; i++) {
        cout << "Landmark " << i;
        cout << " - x: " << pinkX[i] << "   y: " << pinkY[i];
        cout << "   conf: " << pinkConf[i] << "\n";
    }
#endif //OFFLINE        
    // Cull any low confidence pink blobs
    ReducePinkConfidence(3, 3);

    #ifdef TESTING_LANDMARK
    pinkX[0] = 260;
    pinkY[0] = -1;
    pinkConf[0] = 100;
    pinkX[1] = 295;
    pinkY[1] = 550;
    pinkConf[1] = 100;
    pinkX[2] = -15;
    pinkY[2] = 270;
    pinkConf[2] = 100;
    numPink = 3;
    #endif //  TESTING_LANDMARK 
    
    
    SortAndCalculatePink();
    
    PinkCalcAngles();
    cout << "Found " << numPink << " pink landmarks.\n";
    for (int i = 0; i < numPink; i++) {
        cout << "Landmark " << i;
        cout << " - x: " << pinkX[i] << "   y: " << pinkY[i];
        cout << "   conf: " << pinkConf[i] << "\n";
    }
    landmarksInitialised = true;
        
#else    
    int wholeField[PINK_ARRAY_BOX_SIZE_X][PINK_ARRAY_BOX_SIZE_Y];
    PinkLineToBox(listPinkEQ,wholeField);
    
    vector < PinkBlob > listPinkBlob;
    PinkFindBlobBox(listPinkBlob,wholeField);
    
    numPink = PinkFindPosition(listPinkBlob, pinkX, pinkY, pinkConf,maxPink);
    cout << "BEFORE BEFORE SORT Found " << numPink << " pink landmarks.\n";
    for (int i = 0; i < numPink; i++) {
        cout << "Landmark " << i;
        cout << " - x: " << pinkX[i] << "   y: " << pinkY[i];
        cout << "   conf: " << pinkConf[i] << "\n";
    }
    SortAndCalculatePink();
    PinkCalcAngles();
    if (!landmarksInitialised) {
        #ifdef TESTING_LANDMARK
        pinkX[0] = 260;
        pinkY[0] = -1;
        pinkConf[0] = 100;
        pinkX[1] = 295;
        pinkY[1] = 550;
        pinkConf[1] = 100;
        pinkX[2] = -15;
        pinkY[2] = 270;
        pinkConf[2] = 100;
        numPink = 3;
        #endif

    
        cout << "Found " << numPink << " pink landmarks.\n";
        for (int i = 0; i < numPink; i++) {
            cout << "Landmark " << i;
            cout << " - x: " << pinkX[i] << "   y: " << pinkY[i];
            cout << "   conf: " << pinkConf[i] << "\n";
        }
        landmarksInitialised = true;
    }    
#endif //0
#endif // SECOND_WAY
#ifdef FN_DEBUG
    cout << __func__ << " end." << endl;
#endif
}

#endif //Localisation Challenge
