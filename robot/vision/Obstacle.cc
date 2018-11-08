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


#include "Obstacle.h"
//#define OBS_DEBUG

Obstacle::Obstacle()
{
    //cerr << __func__ <<endl;
    int i;
    m_nPoints = 0;
    m_nBoxes = 0;
    
    for(i = 0; i < MAX_NUM_BOXES; i++)
    {
        mp_obstacle_box_list[i].m_frameLastSeen = -1;
    }
    
    m_nGPSBoxes = 0;
    
    for(i = 0; i < GPS_MAX_NUM_BOXES; i++)
    {
        mp_obstacle_box_gps[i].m_frameLastSeen = -1;
    }    
    
    setConstBoxes();
    doObstacleUpdate = true;
}

Obstacle::~Obstacle()
{
    //cerr << "delete" << __func__ <<endl;
    m_nPoints = 0;
}


void 
Obstacle::setShareInfo(WMShareObj& share) { 

    qsort(mp_obstacle_box_gps, m_nGPSBoxes, sizeof(ObstacleBox), compareByOwnObstaclePoints); 
    
    // Obstacle sharing 
    ObstacleShareObj obs;
    for (int i = 0; i < NUM_OBSTACLE_SHARE; i++) {
        if (i < m_nGPSBoxes) { 
            ObstacleBox box = mp_obstacle_box_gps[i];
            int x, y; 
            box.getMiddlePoint(&x,&y);
            obs.x = x;
            obs.y = y;
            // floor the obstacle points 
            obs.count = round(box.getNoPoints());

            share.obstacleVals[i] = obs.convert();
        } else { 
            share.obstacleVals[i] = 0;
        }
    } 
}

void 
Obstacle::processShareInfo(const WMShareObj& info) { 
    for (int i = 0; i < NUM_OBSTACLE_SHARE; i++) { 
        ObstacleShareObj obs = (ObstacleShareObj)info.obstacleVals[i];
        // update obstacle
        int count = obs.count;
        if (count <= 0) {
            continue;
        }
        
        // don't add more than 100 obstacles. 
        if (count > 100) { 
            count = 100;
        }
        addSharedPointsToGPS(obs.x,obs.y,count);
    }
    
    return;
}

void
Obstacle::addSharedPointsToGPS(int x, int y, int nSharedPoints)
{
    int i;
    
    for(i = m_nGPSBoxes-1; i >= 0; i--)
    {
        if (mp_obstacle_box_gps[i].pointInBox(x, y))
        {              
            mp_obstacle_box_gps[i].addSharedPoints(nSharedPoints);
            mp_obstacle_box_gps[i].m_frameLastReceived = currentFrame;
            break;
        }

    }  
    //point not in any current gps boxes
    if (i < 0)
    {
        int middleX, middleY;
        getBoxCoordinatePoint(x, y, middleX, middleY);
        
        if (m_nGPSBoxes != GPS_MAX_NUM_BOXES) 
        {            
            mp_obstacle_box_gps[m_nGPSBoxes].resetTo(middleX,middleY,0,0,currentFrame,nSharedPoints);
            ++m_nGPSBoxes;
        }
    }
}

// set an array of obstacle boxes representing known constant
// obstacles on the field.
// NOTE: this will probably need to be updated for new field specs
void
Obstacle::setConstBoxes()
{
    currentFrame = 0;
    m_nConstBoxes = NUM_CONST_BOXES;
    
    int x, y, i = -1;
    int boxX, boxY;
    
    //my goal (6 boxes)
    x = (FIELD_WIDTH/2) + (GOAL_WIDTH/2);
    
    y = BOX_SIZE_Y/2 - GOAL_FRINGE_WIDTH;
    getBoxCoordinatePoint(x, y, boxX, boxY);    
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);

    y += BOX_SIZE_Y;
    getBoxCoordinatePoint(x, y, boxX, boxY);        
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);
    
    y += BOX_SIZE_Y;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);    
    
    x = (FIELD_WIDTH/2) - (GOAL_WIDTH/2);
    
    y = BOX_SIZE_Y/2 - GOAL_FRINGE_WIDTH;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);
    
    y += BOX_SIZE_Y;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);
    
    y += BOX_SIZE_Y;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);    
    
    //opponent goal (6 boxes)
    x = (FIELD_WIDTH/2) + (GOAL_WIDTH/2);
    
    y = FIELD_LENGTH + GOAL_FRINGE_WIDTH - BOX_SIZE_Y/2;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);
    
    y -= BOX_SIZE_Y;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);
    
    y -= BOX_SIZE_Y;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);  
    
    x = (FIELD_WIDTH/2) - (GOAL_WIDTH/2);
    
    y = FIELD_LENGTH + GOAL_FRINGE_WIDTH - BOX_SIZE_Y/2;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);
    
    y -= BOX_SIZE_Y;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);
    
    y -= BOX_SIZE_Y;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);   
    
    //beacons (4 boxes)
    x = 0 - BEACON_EDGE_LINE_OFFSET_X;
    
    y = (FIELD_LENGTH/2) - BEACON_BOTTOM_LINE_OFFSET_Y;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS); 
    
    y = (FIELD_LENGTH/2) + BEACON_BOTTOM_LINE_OFFSET_Y; 
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);     
    
    x = FIELD_WIDTH + BEACON_EDGE_LINE_OFFSET_X;
    
    y = (FIELD_LENGTH/2) - BEACON_BOTTOM_LINE_OFFSET_Y;
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS); 
    
    y = (FIELD_LENGTH/2) + BEACON_BOTTOM_LINE_OFFSET_Y; 
    getBoxCoordinatePoint(x, y, boxX, boxY);   
    mp_obstacle_box_const[++i].resetTo(boxX, boxY, currentFrame, CONST_BOX_POINTS);      

}

int
Obstacle::getNoObstacleInBox(int topleftx, int toplefty, int bottomrightx, int bottomrighty, int minObstacle, int flags) 
{

    bool useGPS = flags & OBS_USE_GPS;
    bool useLocal = flags & OBS_USE_LOCAL;
    
    if ((not(useGPS) && not(useLocal)) || (useGPS && useLocal))
    {
        useGPS = ObstacleGPSValid();
        useLocal = not(useGPS);
    }
    
    if (useGPS)
    {
        return getNoObstacleInBoxGPS(topleftx, toplefty, 
        bottomrightx, bottomrighty, flags, minObstacle); 
    }
    else
    {
        return getNoObstacleInBoxLocal(topleftx, toplefty, 
        bottomrightx, bottomrighty, flags, minObstacle);
    } 
}

/*
 * Get the number of obstacle points in the box according to dog Coordinate
 * Given the points , number of points and box coordinate
 */ 
int 
Obstacle::getNoObstacleInBoxLocal(int topleftx, int toplefty, int bottomrightx, int bottomrighty, int flags, int minObstacle) 
{
    //cerr << __func__ <<endl;
    int middleX, middleY;
    double count = 0;    
    
    int tlx = min(topleftx, bottomrightx);
    int tly = max(toplefty, bottomrighty);
    
    int brx = max(topleftx, bottomrightx);
    int bry = min(toplefty, bottomrighty);    
    
    for (int i = 0; i < m_nBoxes; ++i) 
    {
        mp_obstacle_box_list[i].getMiddlePoint(&middleX,&middleY);
        if (isInside(tlx, tly, brx, bry, middleX, middleY))
        {   
            if (mp_obstacle_box_list[i].getNoTotalPoints() >= minObstacle)
            {
                count += mp_obstacle_box_list[i].getNoTotalPoints();
            }
        }
    }
    
    return round(count);
    
    if(flags){} // get rid of unused variable warning
}

/*
 * Get the number of obstacle in the box according to dog Coordinate
 * Given the points , number of points and box coordinate
 */ 
int 
Obstacle::getNoObstacleInBoxGPS(int topleftx, int toplefty, 
    int bottomrightx, int bottomrighty, int flags, int minObstacle, bool globalCoords) 
{
    double tlx_d, tly_d, brx_d, bry_d;
    bool useShared = flags & OBS_USE_SHARED;
    bool useConst = flags & OBS_USE_CONST;
    
    int tlx, tly, brx, bry;
    if (globalCoords)
    {
        tlx = min(topleftx, bottomrightx);
        tly = max(toplefty, bottomrighty);
        
        brx = max(topleftx, bottomrightx);
        bry = min(toplefty, bottomrighty);
    }
    else
    {
        getGlobalCoordinate(dogX, dogY, dogH, topleftx, toplefty, &tlx_d, &tly_d);
        getGlobalCoordinate(dogX, dogY, dogH, bottomrightx, bottomrighty, &brx_d, &bry_d);
        
        tlx = round(min(tlx_d, brx_d));
        tly = round(max(tly_d, bry_d));
        
        brx = round(max(tlx_d, brx_d));
        bry = round(min(tly_d, bry_d));
    }
    
    int i;
    int middleX, middleY;
    double count = 0;    
    for (i = 0; i < m_nGPSBoxes; ++i) 
    {
        mp_obstacle_box_gps[i].getMiddlePoint(&middleX,&middleY);
        if (isInside(tlx, tly, brx, bry, middleX, middleY))
        {
            if (mp_obstacle_box_gps[i].getPoints(useShared) >= minObstacle) 
            {
                count += mp_obstacle_box_gps[i].getPoints(useShared);
            }
        }
    }
    
    if (useConst)
    {
        for (i = 0; i < m_nConstBoxes; ++i)
        {
            mp_obstacle_box_const[i].getMiddlePoint(&middleX,&middleY);
            if (isInside(tlx, tly, brx, bry, middleX, middleY))
            {
                if (mp_obstacle_box_const[i].getPoints(useShared) >= minObstacle) 
                {
                    count += mp_obstacle_box_const[i].getPoints(useShared);
                }
            }
        }
    }
    
    return round(count);
}
    
int Obstacle::getRectPoints(
    int p1x, int p1y, int p2x, int p2y, int corridorWidth,
    int *rectPt1X, int *rectPt1Y,
    int *rectPt2X, int *rectPt2Y,
    int *rectPt3X, int *rectPt3Y,
    int *rectPt4X, int *rectPt4Y)
{
    if (corridorWidth < 1) return -1;
    
    //we will treat point 1 as being (0,0)
    //shift is the amount we have minused to make this so
    int shiftX = p1x;
    int shiftY = p1y;
    //shifted is the second point if the first point is (0,0)
    double x = p2x - shiftX;
    double y = p2y - shiftY;
    
    double hCW = corridorWidth/2.0;
    
    double heading;
    if (x == 0) heading = 0;
    if (x > 0) heading = atan(y/x) - M_PI/2;
    else heading = atan(y/x) + M_PI/2;
    
    double len = cos(-heading) * y + sin(-heading) * x;
    
    if (len < 0.5) return -1;    

    double cosA = cos(heading);
    double sinA = sin(heading);    

    *rectPt2X = round(hCW * cosA - len * sinA) + shiftX;
    *rectPt2Y = round(len * cosA + hCW * sinA) + shiftY;
    
    *rectPt1X = round(-hCW * cosA - len * sinA) + shiftX;
    *rectPt1Y = round(len * cosA - hCW * sinA) + shiftY;
    
    *rectPt3X = round(hCW * cosA) + shiftX;
    *rectPt3Y = round(hCW * sinA) + shiftY;
    
    *rectPt4X = round(-hCW * cosA) + shiftX;
    *rectPt4Y = round(-hCW * sinA) + shiftY;    

#ifdef OFFLINE    
#ifdef OBS_DEBUG 
    FCOUT << "rectPts" 
    << " (" << *rectPt1X - shiftX<< "," << *rectPt1Y - shiftY<< ")" 
    << " (" << *rectPt2X - shiftX<< "," << *rectPt2Y - shiftY<< ")" 
    << " (" << *rectPt3X - shiftX<< "," << *rectPt3Y - shiftY<< ")" 
    << " (" << *rectPt4X - shiftX<< "," << *rectPt4Y - shiftY<< ")" 
    << endl;
#endif   
#endif
    
    return 0;
}
 
int 
Obstacle::getNoObstacleInHeading(int *dist,
    int minHeading, int maxHeading,
    int minDist, int maxDist, 
    int minObstacle, int flags) {

    bool useGPS = flags & OBS_USE_GPS;
    bool useLocal = flags & OBS_USE_LOCAL;
    
    if ((not(useGPS) && not(useLocal)) || (useGPS && useLocal))
    {
        useGPS = ObstacleGPSValid();
        useLocal = not(useGPS);
    }
    
    if (useGPS)
    {
        return getNoObstacleInHeadingGPS(dist, minHeading, maxHeading, 
        minDist, maxDist, flags, minObstacle);
    }
    else
    {
        return getNoObstacleInHeadingLocal(dist, minHeading, maxHeading, 
        minDist, maxDist, flags, minObstacle);
    }
}

int 
Obstacle::getNoObstacleInHeadingLocal(int *dist,
    int minHeading, int maxHeading,
    int minDist, int maxDist, 
    int flags, int minObstacle) { 
    
    double count = 0; 
    
    *dist = -1;
    
    int i;
    double heading, distance;
    
    for (i = 0; i < m_nBoxes; i++) { 

        heading = mp_obstacle_box_list[i].getHeading();
          
        if (minHeading <= heading && heading <= maxHeading) {
            distance = mp_obstacle_box_list[i].getDistance(); 
            if (minDist <= distance && distance <= maxDist) {
                if (*dist == -1 || *dist > distance) { 
                    *dist = round(distance);
                }            
                
                if (mp_obstacle_box_list[i].getNoPoints() >= minObstacle) { 
                    count += mp_obstacle_box_list[i].getNoPoints();    
                }        
            }
        }
    }  

    return round(count);
    
    if(flags){} // get rid of unused variable warning
}

int 
Obstacle::getNoObstacleInHeadingGPS(int *dist,
    int minHeading, int maxHeading,
    int minDist, int maxDist, 
    int flags, int minObstacle) { 
    
    bool useShared = flags & OBS_USE_SHARED; 
    bool useConst = flags & OBS_USE_CONST;   
    
    double count = 0; 
    
    *dist = -1;
    
    int i;
    double heading, distance;
    
    for (i = 0; i < m_nGPSBoxes; i++) { 

        heading = mp_obstacle_box_gps[i].getHeading(dogX, dogY, dogH);
          
        if (minHeading <= heading && heading <= maxHeading) {
            distance = mp_obstacle_box_gps[i].getDistance(dogX, dogY); 
            if (minDist <= distance && distance <= maxDist) {
                if (*dist == -1 || *dist > distance) { 
                    *dist = round(distance);
                }            
                
                if (mp_obstacle_box_gps[i].getPoints(useShared) >= minObstacle) { 
                    count += mp_obstacle_box_gps[i].getPoints(useShared);    
                }        
            }
        }
    }  
    
    if (useConst)
    {
        for (i = 0; i < m_nConstBoxes; i++) { 
    
            heading = mp_obstacle_box_const[i].getHeading(dogX, dogY, dogH);
            
            if (minHeading <= heading && heading <= maxHeading) {
                distance = mp_obstacle_box_const[i].getDistance(dogX, dogY); 
                if (minDist <= distance && distance <= maxDist) {
                    if (*dist == -1 || *dist > distance) { 
                        *dist = round(distance);
                    }            
                    
                    if (mp_obstacle_box_const[i].getPoints(useShared) >= minObstacle) { 
                        count += mp_obstacle_box_const[i].getPoints(useShared);    
                    }        
                }
            }
        }  
    }
    
    return round(count);
}

    
int Obstacle::getNoObstacleBetween(int p1x, int p1y, int p2x, int p2y, 
int corridorWidth, int minDist, int minObstacle, int flags)
{
//    cerr << "boxes: " << m_nGPSBoxes << endl;
//    for (int i = 0; i < m_nGPSBoxes; ++i) {
//        cerr << "box " << mp_obstacle_box_gps[i].getX() << ", "
//             << mp_obstacle_box_gps[i].getY() << ": " 
//             << mp_obstacle_box_gps[i].getNoPoints() << endl;
//    }
    bool useGPS = flags & OBS_USE_GPS;
    bool useLocal = flags & OBS_USE_LOCAL;
    
    if ((not(useGPS) && not(useLocal)) || (useGPS && useLocal))
    {
        useGPS = ObstacleGPSValid();
        useLocal = not(useGPS);
    }
    
    if (useGPS)
    {
        return getNoObstacleBetweenGPS(p1x, p1y, p2x, p2y, flags, 
                                      corridorWidth, minDist, minObstacle);
    }
    else
    {
        return getNoObstacleBetweenLocal(p1x, p1y, p2x, p2y, flags, 
        corridorWidth, minDist, minObstacle);
    }
}

/*
 * Given two points in local coordinates, return number
 * of obstacle points in the corridor between them
 *
 */
int Obstacle::getNoObstacleBetweenLocal(int p1x, int p1y, int p2x, int p2y, 
int flags, int corridorWidth, int minDist, int minObstacle)
{   

    int rectPt1X, rectPt1Y, rectPt2X, rectPt2Y;
    int rectPt3X, rectPt3Y, rectPt4X, rectPt4Y;
    
    if (getRectPoints(p1x, p1y, p2x, p2y, corridorWidth,
        &rectPt1X, &rectPt1Y, &rectPt2X, &rectPt2Y,
        &rectPt3X, &rectPt3Y, &rectPt4X, &rectPt4Y)) return 0;
    
    int middleX, middleY;
    double count = 0;     
    minDist = SQUARE(minDist);
    int minX = minOfFour(rectPt1X, rectPt2X, rectPt3X, rectPt4X);
    int minY = minOfFour(rectPt1Y, rectPt2Y, rectPt3Y, rectPt4Y);
    int maxX = maxOfFour(rectPt1X, rectPt2X, rectPt3X, rectPt4X);
    int maxY = maxOfFour(rectPt1Y, rectPt2Y, rectPt3Y, rectPt4Y);    
    for (int i = 0; i < m_nBoxes; ++i) 
    {
        if (mp_obstacle_box_list[i].getDistanceSquared() < minDist) continue;
        if (mp_obstacle_box_list[i].getNoTotalPoints() < minObstacle) continue;
        
        mp_obstacle_box_list[i].getMiddlePoint(&middleX,&middleY);
        
        //don't consider a box that's outside the bounding box of the rectangle
        if (not(isInside(minX, maxY, maxX, minY, middleX, middleY))) continue;
                
        if (isInRectangle(middleX, middleY, rectPt1X, rectPt1Y, 
            rectPt2X, rectPt2Y, rectPt3X, rectPt3Y, rectPt4X, rectPt4Y))
        {
#ifdef OBS_DEBUG    
            FCOUT << "obsInRect (" 
            << middleX << "," << middleY << ")" << endl; 
#endif    
            count += mp_obstacle_box_list[i].getNoTotalPoints();
        }
    }
    
#ifdef OBS_DEBUG    
    FCOUT << "Corridor Intensity -> " << count << endl; 
#endif    

    return int(count);
    
    if(flags){} // get rid of unused variable warning
}

int 
Obstacle::getNoObstacleBetweenGPS(int p1x, int p1y, int p2x, int p2y, 
                    int flags, int corridorWidth, int minDist, int minObstacle)
{
    //cerr << "getNoObstacleBetweenGPS" << endl;
    bool useShared = flags & OBS_USE_SHARED;
    bool useConst = flags & OBS_USE_CONST;
    
//     FCOUT << "dog coords -> X:" << dogX << " Y:" << dogY << " H:" << dogH << endl;
    
//     FCOUT << "corridor points (local) -> (" << p1x << "," << p1y 
//     << ") (" << p2x << "," << p2y << ")" << endl;
    
    double gp1x, gp1y, gp2x, gp2y;
    getGlobalCoordinate(dogX, dogY, dogH, p1x, p1y, &gp1x, &gp1y);
    getGlobalCoordinate(dogX, dogY, dogH, p2x, p2y, &gp2x, &gp2y);

/*    FCOUT << "corridor points (global) -> (" << gp1x << "," << gp1y 
    << ") (" << gp2x << "," << gp2y << ")" << endl;   */ 
    
    int rectPt1X, rectPt1Y, rectPt2X, rectPt2Y;
    int rectPt3X, rectPt3Y, rectPt4X, rectPt4Y;
    
    if (getRectPoints(round(gp1x), round(gp1y), 
        round(gp2x), round(gp2y), corridorWidth,
        &rectPt1X, &rectPt1Y, &rectPt2X, &rectPt2Y,
        &rectPt3X, &rectPt3Y, &rectPt4X, &rectPt4Y)) return 0;

//     FCOUT << "rectangle points -> "
//     << "(" << rectPt1X << "," << rectPt1Y << ") "
//     << "(" << rectPt2X << "," << rectPt2Y << ") "    
//     << "(" << rectPt3X << "," << rectPt3Y << ") "    
//     << "(" << rectPt4X << "," << rectPt4Y << ") "    
//     << endl;                
    
    int i; 
    int middleX, middleY;
    double count = 0;    
    minDist = SQUARE(minDist);
    int minX = minOfFour(rectPt1X, rectPt2X, rectPt3X, rectPt4X);
    int minY = minOfFour(rectPt1Y, rectPt2Y, rectPt3Y, rectPt4Y);
    int maxX = maxOfFour(rectPt1X, rectPt2X, rectPt3X, rectPt4X);
    int maxY = maxOfFour(rectPt1Y, rectPt2Y, rectPt3Y, rectPt4Y);
    for (i = 0; i < m_nGPSBoxes; ++i) 
    {
        if (mp_obstacle_box_gps[i].getDistanceSquared(dogX, dogY) < minDist) continue;
        if (mp_obstacle_box_gps[i].getPoints(useShared) < minObstacle) continue;
        
        mp_obstacle_box_gps[i].getMiddlePoint(&middleX,&middleY);
        
        //don't consider a box that's outside the bounding box of the rectangle
        if (not(isInside(minX, maxY, maxX, minY, middleX, middleY))) continue;
        
        if (isInRectangle(middleX, middleY, rectPt1X, rectPt1Y, 
            rectPt2X, rectPt2Y, rectPt3X, rectPt3Y, rectPt4X, rectPt4Y))
        {
            //cerr << "gps box " << middleX << ", " << middleY <<
            //    " is in corridor" << endl;
            count+=mp_obstacle_box_gps[i].getPoints(useShared);
        }
    }
     
    if (useConst)
    {
        for (i = 0; i < m_nConstBoxes; ++i) 
        {
            if (mp_obstacle_box_const[i].getDistanceSquared(dogX, dogY) < minDist) continue;
            
            mp_obstacle_box_const[i].getMiddlePoint(&middleX,&middleY);
            
            //don't consider a box that's outside the bounding box of the rectangle
            if (not(isInside(minX, maxY, maxX, minY, middleX, middleY))) continue;
            
            if (isInRectangle(middleX, middleY, rectPt1X, rectPt1Y, 
                rectPt2X, rectPt2Y, rectPt3X, rectPt3Y, rectPt4X, rectPt4Y))
            {
                //cerr << "constant box " << middleX << ", " << middleY <<
                //    " is in corridor" << endl;
                count+=mp_obstacle_box_const[i].getPoints(useShared);
            }
        }    
    }
    
    return round(count);
}

bool Obstacle::getBestGap(double destX, double destY,
        int *gapLeft, int *gapRight, int *bestHeading, int *gapAngle,
        int maxdist, int mindist, int minGapAngle, int minObstacle, int flags) 
{
    bool useGPS = flags & OBS_USE_GPS;
    bool useLocal = flags & OBS_USE_LOCAL;
    
    if ((not(useGPS) && not(useLocal)) || (useGPS && useLocal))
    {
        useGPS = ObstacleGPSValid();
        useLocal = not(useGPS);
    }
    
    if (useGPS)
    {
        return getBestGapGPS(destX, destY, flags, gapLeft, gapRight, 
        bestHeading, gapAngle, maxdist, mindist, minGapAngle, minObstacle); 
    }
    else
    {
        return getBestGapLocal(destX, destY, flags, gapLeft, gapRight, 
        bestHeading, gapAngle, maxdist, mindist, minGapAngle, minObstacle); 
    }
   
}

/*
 * Given a destination point in local coordinates, 
 * return the best gap to four return value pointers
 * gapLeft = heading to left side of gap
 * gapRight = heading to right side of gap
 * bestHeading = best heading to destination in gap
 * gapAngle = size of gap as angle
 * 
 * all values are in degrees where -90 is right and 90 is left
 * function returns false if no best gap can be found
 */
bool Obstacle::getBestGapLocal(double destX, double destY, int flags,
int *gapLeft, int *gapRight, int *bestHeading, int *gapAngle,
int maxdist, int mindist, int minGapAngle, int minObstacle)   
{
    if ((gapLeft == NULL) or (gapRight == NULL) or 
        (bestHeading == NULL) or (gapAngle == NULL))
    {
#ifdef OBS_DEBUG    
        FCOUT << "ERROR: One or more return value" 
        << " pointers are not defined" << endl;
#endif        
        return false;
    }
 
    //dog is at it's destination, can't get best gap
    if ((destX == 0) and (destY == 0)) return false;

    return getBestGapFromBoxes(
    mp_obstacle_box_list, m_nBoxes, destX, destY, flags, 
    gapLeft, gapRight, bestHeading, gapAngle,
    maxdist, mindist, minGapAngle, minObstacle);
}

/*
 * Given a destination point in local coordinates, 
 * return the best gap to four return value pointers
 * gapLeft = heading to left side of gap
 * gapRight = heading to right side of gap
 * bestHeading = best heading to destination in gap
 * gapAngle = size of gap as angle
 * 
 * all values are in degrees where -90 is right and 90 is left
 * function returns false if no best gap can be found
 */
bool Obstacle::getBestGapGPS(double destX, double destY, int flags,
        int *gapLeft, int *gapRight, int *bestHeading, int *gapAngle, 
        int maxdist, int mindist, int minGapAngle, int minObstacle)   
{
    bool useShared = flags & OBS_USE_SHARED;
    bool useConst = flags & OBS_USE_CONST;

    if ((gapLeft == NULL) or (gapRight == NULL) or 
        (bestHeading == NULL) or (gapAngle == NULL))
    {
#ifdef OBS_DEBUG    
        FCOUT << "ERROR: One or more return value" 
        << " pointers are not defined" << endl;
#endif        
        return false;
    }
 
    //dog is at it's destination, can't get best gap
    if ((destX == 0) and (destY == 0)) return false;

    int i;
    int mx, my;
    double lmx, lmy;
    
    int numLocalGPSBoxes = m_nGPSBoxes;
    if (useConst)
    {
        numLocalGPSBoxes += m_nConstBoxes;
    }

    // create a GPS obstacle box array with local instead
    // of global coordinates
    ObstacleBox localGPSBoxes[numLocalGPSBoxes];
    for (i = 0; i < m_nGPSBoxes; i++)
    {
        mp_obstacle_box_gps[i].getMiddlePoint(&mx, &my);
        getLocalCoordinate(dogX, dogY, dogH, mx, my, &lmx, &lmy);

        localGPSBoxes[i].resetTo(int(lmx), int(lmy), 
            mp_obstacle_box_gps[i].getFrameLastSeen(), 
            mp_obstacle_box_gps[i].getPoints(useShared));
    }
       
    if (useConst)
    {
        for (i = m_nGPSBoxes; i < numLocalGPSBoxes; i++)
        {
            mp_obstacle_box_const[i-m_nGPSBoxes].getMiddlePoint(&mx, &my);
            getLocalCoordinate(dogX, dogY, dogH, mx, my, &lmx, &lmy);
    
            localGPSBoxes[i].resetTo(int(lmx), int(lmy), currentFrame, 
            mp_obstacle_box_const[i].getPoints(useShared));
        }
    }
    
    return getBestGapFromBoxes(
        localGPSBoxes, numLocalGPSBoxes, destX, destY, flags,
        gapLeft, gapRight, bestHeading, gapAngle,
        maxdist, mindist, minGapAngle, minObstacle);
}


bool 
Obstacle::getBestGapFromBoxes(
    ObstacleBox *boxes, int numBoxes,
    double destX, double destY, int flags, 
    int *gapLeft, int *gapRight, 
    int *bestHeading, int *gapAngle,
    int maxdist, int mindist, 
    int minGapAngle, int minObstacle)
{
    
    //get heading to destination
    double headingToDest;
    if (destX == 0) headingToDest = 0;
    else if (destX > 0) headingToDest = 
        (atan(destY/destX) - M_PI_2) * 180 / M_PI;  
    else headingToDest = 
        (atan(destY/destX) + M_PI_2) * 180 / M_PI; 
        
    //no obstacle boxes, return heading to destination 
    if (numBoxes == 0) 
    {
        *gapLeft = round(NormalizeAngle_180(headingToDest + 90));
        *gapRight = round(NormalizeAngle_180(headingToDest - 90));
        *bestHeading = round(headingToDest);
        *gapAngle = 180;
        return true;
    }    
    
    if (maxdist == GAP_MAX_DIST) maxdist = calcMaxDist(destX, destY, mindist);
    
    setLocalHeadings(boxes, numBoxes);
    
    //sort on (local) heading, greatest to smallest (l to r)
    qsort(boxes, numBoxes, sizeof(ObstacleBox), compareByHeading);

    //noObstacles is true for the special case of no valid obstacle boxes
    bool noObstacles = true;   
    
    //create an array to store box offsets so don't need to keep recalculating
    double boxesO[numBoxes];  
    
    //create an array to store whether a box has been deleted 
    //(i.e. the box will not be considered for best gap) 
    bool   boxesDel[numBoxes]; 
    int i, lastValid = -1;
    
    mindist = SQUARE(mindist);
    maxdist = SQUARE(maxdist);
    //mark unusable obstacles as deleted
    for (i = 0; i < numBoxes; i++)
    {

        boxesDel[i] = true;
        
        //discount obstacles with too low intensity
        if (boxes[i].getNoTotalPoints() < minObstacle) {continue;}
                
        //discount obstacles not in given distances
        if (not(isBetween(boxes[i].getDistanceSquared(), mindist, maxdist))) {continue;}
        
        boxesO[i] = boxes[i].getOffset(boxes[i].getHeading());
        
        //if obstacles have same heading, keep one with largest offset
        if ((lastValid != -1) && (boxes[i].getHeading() == boxes[lastValid].getHeading()))
        {
            if(boxesO[i] <= boxesO[lastValid]) {continue;}
            else boxesDel[lastValid] = true;
        }
                 
        lastValid = i;
        boxesDel[i] = false;
        noObstacles = false;
    }     
    
    //no valid obstacle boxes, return heading to destination
    if (noObstacles) 
    {
        *gapLeft = round(NormalizeAngle_180(headingToDest + 90));
        *gapRight = round(NormalizeAngle_180(headingToDest - 90));
        *bestHeading = round(headingToDest);
        *gapAngle = 180;
        return true;
    }    
    
    bool bestGapSet = false; 
    double bestGapHeading = 0;
    double bestGapRight = 0;
    double bestGapLeft = 0;
    double bestGapAngle = 0;
    
    double tmp;
    int gapR = -1;
    int gapL = -1;
    int firstUsedObs = -1;
    int lastUsedObs = -1;
    
    // The boxes are sorted on heading, so loop over them in order
    // (leftest to rightest)
    for (i = 0; i < numBoxes; ++i) 
    {
        if(boxesDel[i]) continue;
        
        lastUsedObs = i;
        if (gapL == -1)
        {
            gapL = i;
            firstUsedObs = i;
            continue;
        }
        gapR = gapL;
        gapL = i;

        tmp = closestHeadingToDestInGap(headingToDest, minGapAngle, 
                boxes[gapR].getHeading(), boxesO[gapR], boxes[gapL].getHeading(), boxesO[gapL]);
        
        if ( (tmp != NO_GAP) && 
                (not(bestGapSet) || (fabs(tmp - headingToDest) 
                                     < fabs(bestGapHeading - headingToDest))))
        {
            bestGapHeading = tmp;
            bestGapRight = offsetRight(boxes[gapR].getHeading(),boxesO[gapR]);
            bestGapLeft = offsetLeft(boxes[gapL].getHeading(),boxesO[gapL]);
            bestGapAngle = fabs(bestGapRight - bestGapLeft);
            bestGapSet = true;
        }
    }
    
    //test gap between last valid obstacle and first valid obstacle
    if(firstUsedObs != lastUsedObs)
    {                
        gapL = firstUsedObs;
        gapR = lastUsedObs;
        
        tmp = closestHeadingToDestInGap(headingToDest, minGapAngle,
        boxes[gapR].getHeading(), boxesO[gapR], boxes[gapL].getHeading(), boxesO[gapL]);
        
        if ( (tmp != NO_GAP) && (not(bestGapSet) || 
        (fabs(tmp - headingToDest) < fabs(bestGapHeading - headingToDest))))
        {
            //NOTE: for this case bestGapAngle is so huge because
            //it's the obtuse angle between the first and last obstacle boxes
            //simalarly at first glance the left heading for this gap 
            //will be to the right of the right heading for this gap
            //but really you're just facing the other way
            //try not to get too confused :)  
            bestGapHeading = tmp;
            bestGapRight = offsetRight(boxes[gapR].getHeading(),boxesO[gapR]);
            bestGapLeft = offsetLeft(boxes[gapL].getHeading(),boxesO[gapL]);
            bestGapAngle = 360 - fabs(bestGapRight - bestGapLeft);
            bestGapSet = true;
        }
    }
    //handle special case: there is only one valid obstacle box.
    //test gap between -180 and obstacle 
    //and gap between obstacle and 180
    else
    {
        gapL = firstUsedObs;
        tmp = closestHeadingToDestInGap(headingToDest, minGapAngle, 
        -180, 0, boxes[gapL].getHeading(), boxesO[gapL]);

        if ( (tmp != NO_GAP) && (not(bestGapSet) || 
        (fabs(tmp - headingToDest) < fabs(bestGapHeading - headingToDest))))
        {   
            bestGapHeading = tmp;
            bestGapRight = -180;
            bestGapLeft = offsetLeft(boxes[gapL].getHeading(),boxesO[gapL]);
            bestGapAngle = fabs(bestGapRight - bestGapLeft);
            bestGapSet = true;
        }
        
        gapR = firstUsedObs;
        tmp = closestHeadingToDestInGap(headingToDest, minGapAngle, 
        boxes[gapR].getHeading(), boxesO[gapR], 180, 0);
        
        if ( (tmp != NO_GAP) && (not(bestGapSet) || 
        (fabs(tmp - headingToDest) < fabs(bestGapHeading - headingToDest))))
        {
            bestGapHeading = tmp;
            bestGapRight = offsetRight(boxes[gapR].getHeading(),boxesO[gapR]);
            bestGapLeft = 180;
            bestGapAngle = fabs(bestGapRight - bestGapLeft);
            bestGapSet = true;
        }
    }
    
    if (bestGapSet) 
    {   
        *gapLeft = round(bestGapLeft);
        *gapRight = round(bestGapRight);
        *bestHeading = round(bestGapHeading);
        *gapAngle = round(bestGapAngle);
        return true;
    }
    
    return false;
    
    if(flags){} // get rid of unused variable warning
}


// qsort functions
// ---------------

// compare two obstacle boxes by their heading from dog
// boxes are assumed to be in local coords
// array ends up sorted as smallest to largest heading
int 
Obstacle::compareByHeading(const void * ob1, const void * ob2) {
    double h1 = (*(const ObstacleBox*)ob1).getHeadingForQSort();
    double h2 = (*(const ObstacleBox*)ob2).getHeadingForQSort();

    if (h1 < h2) return -1;
    if (h1 == h2) return 0;
    return 1;
}

// compare boxes by their x,y coords, used to sort boxes
// before printing them out in an array so it is easier
// to cycle through (see printGPS)
int 
Obstacle::compareByXY(const void * ob1, const void * ob2) {

   int ob1MidX, ob1MidY;
   (*(const ObstacleBox*)ob1).getMiddlePoint(&ob1MidX, &ob1MidY);
   
   int ob2MidX, ob2MidY;
   (*(const ObstacleBox*)ob2).getMiddlePoint(&ob2MidX, &ob2MidY);
   
   if (ob1MidY == ob2MidY) return ob1MidX - ob2MidX;
   else return ob1MidY - ob2MidY;
}


// compare boxes by own obstacle points
// array ends up sorted most points to least
int 
Obstacle::compareByOwnObstaclePoints(const void * ob1, const void *ob2) { 
    int p1 = int((*(const ObstacleBox*)ob1).getNoPoints());
    int p2 = int((*(const ObstacleBox*)ob2).getNoPoints());
    return p2 - p1; 
}

int Obstacle::calcMaxDist(double destX, double destY, int mindist)
{
    int maxdist = round(DISTANCE(0,0,destX,destY));
    if (maxdist < (mindist + 10)) maxdist = mindist + 10;
    
    return round(maxdist);
}

double Obstacle::closestHeadingToDestInGap(
double destHeading,int minGapAngle,
double gapRHeading, double obsROffset, 
double gapLHeading, double obsLOffset)
{
#ifdef OBS_DEBUG
        FCOUT << "(" << destHeading << "," << gapRHeading << "," << obsROffset << 
        "," << gapLHeading << "," << obsLOffset << "," << minGapAngle << "): ";
#endif    

    double gapAngle = 0;
    if (gapRHeading > gapLHeading) gapAngle = -360;

    
    //handle the special case where after applying offsets left gap
    //heading and right gap heading will cross over each other
    if (fabs(gapAngle + fabs(gapRHeading - gapLHeading)) <= (obsROffset + obsLOffset))
    { 
#ifdef OBS_DEBUG
        cout << NO_GAP << endl;
#endif    
        return NO_GAP;
    }

    gapRHeading = offsetRight(gapRHeading, obsROffset);
    gapLHeading = offsetLeft(gapLHeading, obsLOffset);
        
    gapAngle = fabs(gapAngle + fabs(gapRHeading - gapLHeading));
    
    if (gapAngle < minGapAngle)
    { 
#ifdef OBS_DEBUG
        cout << NO_GAP << endl;
#endif    
        return NO_GAP;
    }
    
    gapRHeading = NormalizeAngle_180(gapRHeading + minGapAngle/2.0);
    gapLHeading = NormalizeAngle_180(gapLHeading - minGapAngle/2.0);
    
    if (isAngleBetween(destHeading, gapRHeading, gapLHeading))
    {
#ifdef OBS_DEBUG
        cout << destHeading << endl;
#endif    
        return destHeading;
    }
    else
    {
        double gapRToDest = fabs(NormalizeAngle_180(gapRHeading - destHeading)); 
        double gapLToDest = fabs(NormalizeAngle_180(gapLHeading - destHeading));         

        if (gapRToDest <= gapLToDest) 
        {
#ifdef OBS_DEBUG
        cout << gapRHeading << endl;
#endif                
            return gapRHeading;
        }
        else
        {        
#ifdef OBS_DEBUG
        cout << gapLHeading << endl;
#endif            
            return gapLHeading;
        }
    }
}

bool Obstacle::isAngleBetween(double destHeading, 
double gapRHeading, double gapLHeading)
{
    if (gapRHeading <= gapLHeading) return isBetween(destHeading, gapRHeading, gapLHeading); 
    return not(isBetween(destHeading, gapRHeading, gapLHeading));
}

bool Obstacle::isBetween(double val, double rangeA, double rangeB)
{
    double start, end;
    
    if (rangeA < rangeB)
    {
        start = rangeA;
        end = rangeB;
    }
    else
    {
        start = rangeB;
        end = rangeA;
    }
    
    if ((val >= start) && (val <= end)) return true;
    
    return false;
}

bool Obstacle::isInside(int topleftx, 
            int toplefty, 
            int bottomrightx, 
            int bottomrighty,
            int x,
            int y)
{
    return  topleftx < x && 
            x < bottomrightx && 
            bottomrighty < y && 
            y < toplefty;            
}

int 
Obstacle::minOfFour(int a, int b, int c, int d)
{
    int min = a;
    if (b < min) min = b;
    if (c < min) min = c;
    if (d < min) min = d;
    
    return min;
}

int 
Obstacle::maxOfFour(int a, int b, int c, int d)
{
    int max = a;
    if (b > max) max = b;
    if (c > max) max = c;
    if (d > max) max = d;
    
    return max;
}

void
Obstacle::ObstacleUpdate(VisualCortex *vision, GPS *gps)
{   
    if (!doObstacleUpdate)
        return;
        
    //cerr << __func__ <<endl;
    if (vision == NULL)
    {
        cerr << __func__ << " : vision is null" <<endl;
        return;
    }
    currentFrame = vision->vFrame;
    
    if (gps == NULL)
    {
        cerr << __func__ << " : gps is null" <<endl;
        return; 
    }
    
    dogX = gps->self().pos.x;
    dogY = gps->self().pos.y;
    dogH = gps->self().h;
    posVar = gps->self().posVar;
    hVar = gps->self().hVar;
    
    
#ifndef OFFLINE
    m_nPoints = vision->getObstaclePoints(mp_points,NOOBSTACLEPOINTS);
    //additional checking 
    if (m_nPoints > NOOBSTACLEPOINTS) 
        m_nPoints = NOOBSTACLEPOINTS;
    //FCOUT << "m_nPoints:"<<m_nPoints <<endl;
    m_nBoxes = 0; //not containing history
    
    putPointsInBox(); 
#endif
    
    //if ((currentFrame % 40) == 0) printGPS();     
}

void Obstacle::clearObstacles(void) {
    m_nPoints = 0;
    m_nBoxes = 0;
    m_nGPSBoxes = 0;
}

void 
Obstacle::printGPS()
{
    int i, j;
    int fieldOffest = 20;

    for (i = 0; i < m_nConstBoxes; i++) 
    {        
        addBoxToGPS(mp_obstacle_box_const[i].getX(), mp_obstacle_box_const[i].getY(),
        round(mp_obstacle_box_const[i].getNoTotalPoints()));
    }

    qsort(mp_obstacle_box_gps, m_nGPSBoxes, sizeof(ObstacleBox), compareByXY); 
    
    int dogBoxX = int((int(dogX) - BOX_SIZE_X/2)/BOX_SIZE_X);
    int dogBoxY = int((int(dogY) - BOX_SIZE_Y/2)/BOX_SIZE_Y);

    int box = 0;
    double nextBoxPts = 0;
    int nextBoxX = 0;
    int nextBoxY = 0;
    if (box < m_nGPSBoxes) 
    {
        mp_obstacle_box_gps[box].getMiddlePoint(&nextBoxX, &nextBoxY);
        nextBoxX = int((nextBoxX - BOX_SIZE_X/2)/BOX_SIZE_X);
        nextBoxY = int((nextBoxY - BOX_SIZE_Y/2)/BOX_SIZE_Y);
        nextBoxPts = mp_obstacle_box_gps[box].getNoTotalPoints();
    }
    else box = -1; 
    
    //show grid
    cout << endl;
    FCOUT << "GPS grid with position variance = " << posVar << endl;
    for (i = -fieldOffest; i < NUMBER_OF_BOX_Y + fieldOffest; i++) 
    {
        for (j = -fieldOffest; j < NUMBER_OF_BOX_X + fieldOffest; j++) 
        {
            if ((dogBoxX == j) && (dogBoxY == i)) cout << "[DG]";
            else if (box != -1) 
            {
                if ((nextBoxX == j) && (nextBoxY == i))
                {
                    if (int(nextBoxPts) < 10) cout << "[0" << int(nextBoxPts) << "]";
                    else cout << "[" << int(nextBoxPts) << "]";
                    
                    box++;
                    if (box < m_nGPSBoxes) 
                    {
                        mp_obstacle_box_gps[box].getMiddlePoint(&nextBoxX, &nextBoxY);
                        nextBoxX = int((nextBoxX - BOX_SIZE_X/2)/BOX_SIZE_X);
                        nextBoxY = int((nextBoxY - BOX_SIZE_Y/2)/BOX_SIZE_Y);
                        nextBoxPts = mp_obstacle_box_gps[box].getNoTotalPoints();
                    }
                    else box = -1; 
                }
                else cout << "[00]";            
            }
            else cout << "[00]";
        }
        cout << endl;
    }  
}

/*
 * Should only be used offline for subvision base.
 * Manually set obstacle points, frame number and dog position
 */
void
Obstacle::setObstaclePoints(int *thepoints, int size, long frameNum,
double myX, double myY, double myH, double myPosVar, double myHVar)
{

    int i;
/*    
    if(frameNum < currentFrame)
    {
        for(i = 0; i < m_nPoints; i++)
        {
            //removeLocalPointFromGPS(dogX, dogY, dogH, 
        }
    }
    strengthenGPS()
*/    
    for (i = 0; i < size; i++)
    {
        mp_points[i*2] = thepoints[i*2];
        mp_points[i*2+1] = thepoints[i*2+1];
    }
    m_nPoints = size;
    m_nBoxes = 0;
    

    currentFrame = frameNum;
    
    dogX = myX;
    dogY = myY;
    dogH = myH;
    posVar = myPosVar;
    hVar = myHVar;
    
    putPointsInBox(); 
    
}

/* Add an obstacle box at the specified location */
ObstacleBox* 
Obstacle::addObstacleBox(int cx, int cy, int frameLastSeen, int nPoints) {
    ObstacleBox *box = &mp_obstacle_box_gps[m_nGPSBoxes];
    m_nGPSBoxes++;
    box->resetTo(cx, cy, frameLastSeen, nPoints);
    return box;
}

void 
Obstacle::getBoxCoordinatePoint(int x, int y, int &boxX, int &boxY)
{
    /* Find out the midlle position of a box */
    
    /* checking negative and postive will make it positive modulus */
    /* -6 % 20 return -6 we need to return 14 */
    int temp = (x % BOX_SIZE_X);
    if (temp < 0)
        temp += BOX_SIZE_X;
    boxX = x - temp + BOX_SIZE_X/2;
    temp = (y % BOX_SIZE_Y);
    if (temp < 0)
        temp += BOX_SIZE_Y;
    boxY = y - temp + BOX_SIZE_Y/2;
}


void 
Obstacle::putPointsInBox()
{   
    
    int i;
    // Reset an array of obstacle boxes for filtering. 
    for (i = 0; i < MAX_RANGES; i++) { 
        mpObstacleAngles[i].resetTo(0,0,0,0);
    }
    
    for (i = 0; i < m_nPoints; ++i) {
        addPointToAngle(mp_points[i*2], mp_points[i*2+1]);        
    }
    
    for (i = 0; i < MAX_RANGES; i++) { 
        if (mpObstacleAngles[i].getNoTotalPoints() > 0) { 
            addBoxToLocal(mpObstacleAngles[i].getX(),
                          mpObstacleAngles[i].getY(),
                          round(mpObstacleAngles[i].getNoTotalPoints()));
        }  
    }

    decayGPS();
    
    localPutInGPS = false;
    if (posVar <= MAX_POS_VAR_TO_UPDATE_OBSTACLE
        && hVar <= MAX_HEADING_VAR_TO_UPDATE_OBSTACLE) {   
        localBoxesToGlobalBoxes();

        for (i = 0; i < MAX_RANGES; i++) {
            if (mpObstacleAngles[i].getNoTotalPoints() > 0) {
                addBoxToGPS(mpObstacleAngles[i].getX(),
                            mpObstacleAngles[i].getY(),
                            round(mpObstacleAngles[i].getNoTotalPoints()));
            }
        }
        localPutInGPS = true;
    } 
}

bool
Obstacle::ObstacleGPSValid()
{
    return localPutInGPS;
}

void Obstacle::setObstacleGPSValid(bool valid) {
    localPutInGPS = valid;
}

bool 
Obstacle::gpsBoxValid(int i)
{
    ObstacleBox box = mp_obstacle_box_gps[i];
    return (((currentFrame - box.getFrameLastSeen() < BOX_LIFETIME)
                || (currentFrame - box.getFrameLastReceived() < BOX_LIFETIME))
            && box.getNoTotalPoints() > 0);
}

void 
Obstacle::decayGPS()
{
    int i;

    for(i = m_nGPSBoxes-1; i >= 0; i--)
    {
        mp_obstacle_box_gps[i].decayPoints();
        
        // box has expired, delete from array
        if (!gpsBoxValid(i))
        {               
            if (i != m_nGPSBoxes - 1)
            {               
                ObstacleBox lastBox = mp_obstacle_box_gps[m_nGPSBoxes - 1];
                mp_obstacle_box_gps[i] = lastBox;
            }
            --m_nGPSBoxes;
        }
    }
}

void 
Obstacle::addPointToAngle(int x, int y)
{
    // Filter out an obstacle that is within 50 cm. 
    if (DISTANCE_SQR(0,0,x,y) < SQUARE(GPS_MIN_DIST)) { 
        return;
    }

    // Filter out any obstabcles that are behind a closer obstacle.
    // Currently, obstacles that are behind are added to the closer obstacle box.   
    int middleX, middleY; 
    getBoxCoordinatePoint(x,y,middleX,middleY);
    
    ObstacleBox box;
    box.resetTo(middleX,middleY,currentFrame);
    double angle = box.getHeading();
    int angleIndex = round((angle - MIN_ANGLE) / ANGLE_RANGE);   
    if (angleIndex < 0) {
        angleIndex = 0;
    } else if (angleIndex >= MAX_RANGES) { 
        angleIndex = MAX_RANGES - 1;
    }
    
    ObstacleBox& angleBox = mpObstacleAngles[angleIndex]; 
    if (angleBox.getNoTotalPoints() > 0) {        
        if (box.getDistanceSquared() < angleBox.getDistanceSquared()) {        
            angleBox.resetTo(middleX,middleY,currentFrame);
            // angleBox.resetTo(middleX,middleY,currentFrame,angleBox.getNoTotalPoints());                       
        } 
        angleBox.addPoint();        
    } else {        
        angleBox.resetTo(middleX,middleY,currentFrame);
    }
}

void
Obstacle::addBoxToLocal(int x, int y, int npoints) 
{
    int i;
    for (i = m_nBoxes-1; i >= 0 ; i--) {
        if (mp_obstacle_box_list[i].pointInBox(x,y)) {              
            mp_obstacle_box_list[i].addPoints(npoints);
            break;
        }
    }
    if (i < 0) { // no box satisfy the point
        if (m_nBoxes == MAX_NUM_BOXES) { 
            cerr <<__func__ << "Need more boxes to be created!"<<endl;
            return;
        } else {            
            mp_obstacle_box_list[m_nBoxes].resetTo(x,y,currentFrame,npoints);
            i = m_nBoxes;
            ++m_nBoxes;
        }
    }    
}

/*
void
Obstacle::addPointToLocal(int x, int y)
{
    int middleX, middleY;
    getBoxCoordinatePoint(x,y,middleX,middleY);

    int i;
    for (i = m_nBoxes-1; i >= 0 ; i--) {
        if (mp_obstacle_box_list[i].pointInBox(middleX,middleY)) {              
            mp_obstacle_box_list[i].addPoint();
            break;
        }
    }
    if (i < 0) { // no box satisfy the point
        if (m_nBoxes == MAX_NUM_BOXES) { 
            cerr <<__func__ << "Need more boxes to be created!"<<endl;
            return;
        } else {            
            mp_obstacle_box_list[m_nBoxes].resetTo(middleX,middleY,currentFrame);
            i = m_nBoxes;
            ++m_nBoxes;
        }
    }
    
    
    // Filter out any obstabcles that are behind a closer obstacle.
    // Currently, obstacles that are behind are added to the closer obstacle box.   
    ObstacleBox box = mp_obstacle_box_list[i];
    double angle = box.getHeading();
    int angleIndex = int((angle - MIN_ANGLE) / ANGLE_RANGE);   
    if (angleIndex < 0) {
        angleIndex = 0;
    } else if (angleIndex >= MAX_RANGES) { 
        angleIndex = MAX_RANGES - 1;
    }
    
    if (mpObstacleAngles[angleIndex].getNoTotalPoints() > 0) {
        ObstacleBox& angleBox = mpObstacleAngles[angleIndex]; 
        if (box.getDistance() < angleBox.getDistance()) {
            box.addPoints(round(angleBox.getNoTotalPoints()));            
            mpObstacleAngles[angleIndex] = box;                       
        } else { 
            angleBox.addPoint();   
        }
    } else {        
        mpObstacleAngles[angleIndex] = box;
    }
    
}
*/

void 
Obstacle::addBoxToGPS(int x, int y, int npoints) 
{
    int i;
    int leastPtsIndex = -1;
          
    for(i = m_nGPSBoxes-1; i >= 0; i--)
    {
        if (mp_obstacle_box_gps[i].pointInBox(x, y))
        {              
            mp_obstacle_box_gps[i].addPoints(npoints);
            mp_obstacle_box_gps[i].m_frameLastSeen = currentFrame;            
            break;
        }

        if (leastPtsIndex == -1 
            || mp_obstacle_box_gps[i].getNoTotalPoints() < mp_obstacle_box_gps[leastPtsIndex].getNoTotalPoints())
        {
            leastPtsIndex = i;
        }
    }  
    //point not in any current gps boxes
    if (i < 0)
    {        
        //all gps boxes are full, replace one with least points
        if (m_nGPSBoxes == GPS_MAX_NUM_BOXES) 
        {
            mp_obstacle_box_gps[leastPtsIndex].resetTo(x,y,currentFrame,npoints);
            i = leastPtsIndex;
        }
        //still empty gps boxes, fill the next empty one
        else
        {   
            mp_obstacle_box_gps[m_nGPSBoxes].resetTo(x,y,currentFrame,npoints);
            i = m_nGPSBoxes;
            ++m_nGPSBoxes;
        }
    }
}



/*
void
Obstacle::addPointToGPS(int x, int y)
{
    int i;
    int leastPtsIndex = -1;
        
    for(i = m_nGPSBoxes-1; i >= 0; i--)
    {
        if (mp_obstacle_box_gps[i].pointInBox(x, y))
        {              
            mp_obstacle_box_gps[i].addPoint();
            mp_obstacle_box_gps[i].m_frameLastSeen = currentFrame;
            break;
        }

        if (leastPtsIndex == -1 
            || mp_obstacle_box_gps[i].getNoTotalPoints() < mp_obstacle_box_gps[leastPtsIndex].getNoTotalPoints())
        {
            leastPtsIndex = i;
        }
    }  
    //point not in any current gps boxes
    if (i < 0)
    {
        int middleX, middleY;
        getBoxCoordinatePoint(x, y, middleX, middleY);
        
        //all gps boxes are full, replace one with least points
        if (m_nGPSBoxes == GPS_MAX_NUM_BOXES) 
        {
            mp_obstacle_box_gps[leastPtsIndex].resetTo(middleX,middleY,currentFrame);
        }
        //still empty gps boxes, fill the next empty one
        else
        {   
            mp_obstacle_box_gps[m_nGPSBoxes].resetTo(middleX,middleY,currentFrame);
            ++m_nGPSBoxes;
        }
    }
}
*/


/*
void
Obstacle::localPtsToGlobal()
{   
    double x, y;
    for (int i = m_nPoints-1; i >= 0; i--) 
    {
        //get rid of points too close to dog, they aren't trustworthy
        if (DISTANCE(0,0,mp_points[i*2],mp_points[i*2+1]) < GPS_MIN_DIST)
        {
            if (i != (m_nPoints-1))
            {
               mp_points[i*2] = mp_points[(m_nPoints-1)*2];
               mp_points[i*2+1] = mp_points[(m_nPoints-1)*2+1];
            }
            m_nPoints--;
        }
        else
        {
            getGlobalCoordinate(dogX, dogY, dogH, 
            double(mp_points[i*2]), double(mp_points[i*2+1]), &x, &y);
            
            if (not(isBetween(int(x),-OUTSIDE_FIELD_OFFSET,FIELD_WIDTH+OUTSIDE_FIELD_OFFSET)) 
                || not(isBetween(int(y),-OUTSIDE_FIELD_OFFSET,FIELD_LENGTH+OUTSIDE_FIELD_OFFSET)))
            {
                if (i != (m_nPoints-1))
                {
                    mp_points[i*2] = mp_points[(m_nPoints-1)*2];
                    mp_points[i*2+1] = mp_points[(m_nPoints-1)*2+1];
                }
                m_nPoints--;
            }
            else
            {
                mp_points[i*2] = int(x);
                mp_points[i*2+1] = int(y);
            }
        }
    }   
}
*/

void 
Obstacle::localBoxesToGlobalBoxes() 
{
    double x, y;
    double gapMinDistSqr = SQUARE(GPS_MIN_DIST);
    for (int i = 0; i < MAX_RANGES; i++) {
        if (mpObstacleAngles[i].getNoTotalPoints() <= 0) { 
            continue;
        }   
        
        ObstacleBox& angleBox = mpObstacleAngles[i];
        //get rid of points too close to dog, they aren't trustworthy
        if (angleBox.getDistanceSquared() < gapMinDistSqr) {
            angleBox.resetTo(0,0,0,0);
        } else {
            getGlobalCoordinate(dogX, dogY, dogH, 
                                double(angleBox.getX()), double(angleBox.getY()), 
                                &x, &y);
            
            if (not(isBetween(int(x),-OUTSIDE_FIELD_OFFSET,FIELD_WIDTH+OUTSIDE_FIELD_OFFSET)) 
                || not(isBetween(int(y),-OUTSIDE_FIELD_OFFSET,FIELD_LENGTH+OUTSIDE_FIELD_OFFSET))) {
                angleBox.resetTo(0,0,0,0);
            } else {
                int middleX, middleY; 
                getBoxCoordinatePoint((int)x,(int)y,middleX,middleY);
                angleBox.setX(middleX); 
                angleBox.setY(middleY);
            }
        }
    } 
}

bool 
Obstacle::getPointToNearestObstacleInBox(
int topleftx, int toplefty, int bottomrightx, int bottomrighty,
int *posX, int *posY, int minObstacle, int flags)
{
    bool useGPS = flags & OBS_USE_GPS;
    bool useLocal = flags & OBS_USE_LOCAL;
    
    if ((not(useGPS) && not(useLocal)) || (useGPS && useLocal))
    {
        useGPS = ObstacleGPSValid();
        useLocal = not(useGPS);
    }
    
    if (useGPS)
    {
        return getPointToNearestObstacleInBoxGPS(
        topleftx, toplefty, bottomrightx, bottomrighty,
        flags, posX, posY, minObstacle);
    }
    else
    {
        return getPointToNearestObstacleInBoxLocal(
        topleftx, toplefty, bottomrightx, bottomrighty,
        flags, posX, posY, minObstacle);
    }
}

bool 
Obstacle::getPointToNearestObstacleInBoxLocal(
int topleftx, int toplefty, int bottomrightx, int bottomrighty,
int flags, int *posX, int *posY, int minObstacle)
{
    if (posX == NULL || posY == NULL || mp_obstacle_box_list == NULL)
    {
#ifdef OBS_DEBUG    
        FCOUT << 
        "posX == NULL || posY == NULL || mp_obstacle_box_list == NULL return false" <<endl;
#endif        
        return false;
    }
    
    int middleX, middleY;
    int distance = VERY_LARGE_INT;
    int minDistIndex = -1;
    
    int tlx = min(topleftx, bottomrightx);
    int tly = max(toplefty, bottomrighty);
    
    int brx = max(topleftx, bottomrightx);
    int bry = min(toplefty, bottomrighty); 
    
    for (int i = 0; i < m_nBoxes; ++i) 
    {
        mp_obstacle_box_list[i].getMiddlePoint(&middleX,&middleY);
        
        if (isInside(tlx, tly, brx, bry, middleX, middleY) && 
            (mp_obstacle_box_list[i].getNoTotalPoints() >= minObstacle) &&
            (mp_obstacle_box_list[i].getDistance() <= distance))
        {
            distance = mp_obstacle_box_list[i].getDistance();
            minDistIndex = i;
        }
    }
    
    if (minDistIndex < 0)
    {
#ifdef OBS_DEBUG        
        FCOUT << "minDistIndex < 0 return false" <<endl;
#endif        
        return false;
    }
    
    mp_obstacle_box_list[minDistIndex].getMiddlePoint(posX,posY);
    
    return true;
    
    if(flags){} // get rid of unused variable warning
}

bool 
Obstacle::getPointToNearestObstacleInBoxGPS(
int topleftx, int toplefty, int bottomrightx, int bottomrighty,
int flags, int *posX, int *posY, int minObstacle)
{
    bool useShared = flags & OBS_USE_SHARED;
    bool useConst = flags & OBS_USE_CONST;
    if (posX == NULL || posY == NULL || mp_obstacle_box_list == NULL)
    {
#ifdef OBS_DEBUG    
        FCOUT 
        << " posX == NULL || posY == NULL || mp_obstacle_box_list == NULL return false" <<endl;
#endif        
        return false;
    }
    
    int i, j;
    bool constObs = false;
    int middleX, middleY;
    int distance = VERY_LARGE_INT;
    int minDistIndex = -1;
    
    double tlx_d, tly_d, brx_d, bry_d;
    
    getGlobalCoordinate(dogX, dogY, dogH, topleftx, toplefty, &tlx_d, &tly_d);
    getGlobalCoordinate(dogX, dogY, dogH, bottomrightx, bottomrighty, &brx_d, &bry_d);
    
    int tlx = round(min(tlx_d, brx_d));
    int tly = round(max(tly_d, bry_d));
    int brx = round(max(tlx_d, brx_d));
    int bry = round(min(tly_d, bry_d));
    
    for (i = 0; i < m_nGPSBoxes; ++i) 
    {
        mp_obstacle_box_gps[i].getMiddlePoint(&middleX,&middleY);
        
        if (isInside(tlx, tly, brx, bry, middleX, middleY) && 
            (mp_obstacle_box_gps[i].getPoints(useShared) >= minObstacle) &&
            (mp_obstacle_box_gps[i].getDistance(dogX, dogY) <= distance))
        {
            distance = mp_obstacle_box_gps[i].getDistance(dogX, dogY);
            minDistIndex = i;
        }
    }
    
    if (useConst)
    {
        for (j = 0; j < m_nConstBoxes; ++j) 
        {
            mp_obstacle_box_const[j].getMiddlePoint(&middleX,&middleY);
            
            if (isInside(tlx, tly, brx, bry, middleX, middleY) && 
                (mp_obstacle_box_const[j].getPoints(useShared) >= minObstacle) &&
                (mp_obstacle_box_const[j].getDistance(dogX, dogY) <= distance))
            {
                distance = mp_obstacle_box_const[j].getDistance(dogX, dogY);
                minDistIndex = j;
                constObs = true;
            }
        }    
    }
    
    if (minDistIndex < 0)
    {
#ifdef OBS_DEBUG        
        FCOUT << "minDistIndex < 0 return false" <<endl;
#endif        
        return false;
    }
    
    int gposX, gposY;
    double retX, retY;

    if (constObs)
    { 
        mp_obstacle_box_const[minDistIndex].getMiddlePoint(&gposX,&gposY);    
    }
    else
    {
        mp_obstacle_box_gps[minDistIndex].getMiddlePoint(&gposX,&gposY);    
    }
    
    getLocalCoordinate(dogX, dogY, dogH, gposX, gposY, &retX, &retY);
        
    *posX = round(retX);
    *posY = round(retY); 
    
    return true;
}

void Obstacle::setLocalHeadings(ObstacleBox *boxes, int numBoxes)
{
    for(int i = 0; i < numBoxes; i++)
    {
        boxes[i].setHeading(boxes[i].getHeading());
    }
}

ObstacleBox::ObstacleBox()
{
    m_middleX = 0;
    m_middleY = 0;
    m_npoints = 0;
    m_nSharedPoints = 0;
    headingValid = false;
}

ObstacleBox::~ObstacleBox()
{}


int ObstacleBox::getX() const
{
    return m_middleX;
}

int ObstacleBox::getY() const
{
    return m_middleY;
}

void ObstacleBox::resetTo(int middleX, int middleY, long lastFrameSeen, double nPoints, long lastFrameReceived, int nSharedPoints)
{
    m_middleX = middleX;
    m_middleY = middleY;
    m_npoints = nPoints;
    m_frameLastSeen = lastFrameSeen;
    
    m_nSharedPoints = nSharedPoints;    
    m_frameLastReceived = lastFrameReceived;
    
    headingValid = false;
}


void ObstacleBox::getMiddlePoint(int *middleX, int *middleY) const 
{
    if (middleX == NULL || middleY == NULL)
        return;
    *middleX = m_middleX;
    *middleY = m_middleY;
}

int ObstacleBox::getDistance(double dogX, double dogY)
{
    return round(DISTANCE(dogX, dogY, m_middleX, m_middleY));
}

int ObstacleBox::getDistanceSquared(double dogX, double dogY)
{
    return int(DISTANCE_SQR(dogX, dogY, (double)m_middleX, (double)m_middleY));
}

double ObstacleBox::getNoTotalPoints() const
{
    return m_npoints + m_nSharedPoints;
}

double ObstacleBox::getNoPoints() const
{
    return m_npoints;
}

double ObstacleBox::getNoSharedPoints() const 
{
    return m_nSharedPoints;
}

double ObstacleBox::getPoints(bool useShared)
{
    if (useShared)
    {
        return getNoTotalPoints();
    }
    else
    {
        return getNoPoints();
    }
}

void ObstacleBox::addPoint()
{
    m_npoints++;
}

void ObstacleBox::addPoints(int npoints)
{
    m_npoints += npoints;
}

void ObstacleBox::addSharedPoints(int npoints)
{
    m_nSharedPoints += npoints;
}

long ObstacleBox::getFrameLastSeen()
{
    return m_frameLastSeen;
}

long ObstacleBox::getFrameLastReceived()
{
    return m_frameLastReceived; 
}

//get heading for obstacle box from dog, if using
//local coordinates return cached heading or cache
//calculated heading if heading not previously
//calculated this frame. 
double ObstacleBox::getHeading(double dogX, double dogY, double dogH)
{
    bool localCoords = false;
    if ((dogX == 0) && (dogY == 0) && (dogH == 0)) localCoords = true;
    
    if (headingValid && localCoords) 
    {
        return m_heading;
    }
    else
    {
        double x, y;
        if(!localCoords)
        {
            getLocalCoordinate(dogX, dogY, dogH, m_middleX, m_middleY, &x, &y);
        }
        else
        {
            x = m_middleX;
            y = m_middleY;
        }
        
        double headingToDest;
        if (x == 0) headingToDest = 0;
        else if (x > 0)
            headingToDest = (atan(y/x) - M_PI_2) * 180 / M_PI;  
        else 
            headingToDest = (atan(y/x) + M_PI_2) * 180 / M_PI; 
            
        headingToDest = NormalizeAngle_180(headingToDest);
        
        if(localCoords)
        {
            setHeading(headingToDest);
        }
                  
        return headingToDest;
    }  
}

//same as getHeading but can't cache because is a const function
//should call setLocalHeadings to pre-calculate before calling 
//qsort with comparebyheading
double ObstacleBox::getHeadingForQSort(double dogX, double dogY, double dogH) const
{
    bool localCoords = false;
    if ((dogX == 0) && (dogY == 0) && (dogH == 0)) localCoords = true;
    
    if (headingValid && localCoords) 
    {
        return m_heading;
    }
    else
    {
        double x, y;
        if(!localCoords)
        {
            getLocalCoordinate(dogX, dogY, dogH, m_middleX, m_middleY, &x, &y);
        }
        else
        {
            x = m_middleX;
            y = m_middleY;
        }
        
        double headingToDest;
        if (x == 0) headingToDest = 0;
        else if (x > 0)
            headingToDest = (atan(y/x) - M_PI_2) * 180 / M_PI;  
        else 
            headingToDest = (atan(y/x) + M_PI_2) * 180 / M_PI; 
            
        headingToDest = NormalizeAngle_180(headingToDest);
                  
        return headingToDest;
    }  
}

void ObstacleBox::setHeading(double h)
{
    m_heading = h;
    headingValid = true;
}

void ObstacleBox::decayPoints()
{
    m_npoints *= BOX_OWN_DECAY;
    m_nSharedPoints *= BOX_SHARED_DECAY; 
}

//tests whether a given point appears in the box
bool ObstacleBox::pointInBox(int x, int y)
{
    if ((abs(m_middleX - x) <= (BOX_SIZE_X/2))
        &&  (abs(m_middleY - y) <= (BOX_SIZE_Y/2))) 
    {
        return true;
    }

    return false;  
}

// use getOffset to return the angle t 
// in degrees between A and B
// where A is the line from the dog to
// the edge of the obstacle box and 
// B is the line from the dog to the
// center of the obstacle box 
// 
//      -----------
//      |         |
//      |    OB   |
//      |         |
//      -----------
//      \    |
//      A\   |B 
//        \  |
//         \t|
//          \|
//          dog
//
// makes getBestGap more accurate
//
// getOffset requires box heading
// if you've already calculated this
// you can pass it as an argument
// to save on processing
double ObstacleBox::getOffset(double heading)
{    
    double boxH;
    if (heading == NO_HEADING)
    { 
        boxH = getHeading();
    }
    else
    {
        boxH = heading;
    }
        

    double sideX = m_middleX - BOX_SIZE_X/2;
    double sideY = m_middleY;
    double sideH;
    if (sideX == 0) sideH = 0;
    else if (sideX > 0)
        sideH = (atan(sideY/sideX) - M_PI_2) * 180 / M_PI;  
    else 
        sideH = (atan(sideY/sideX) + M_PI_2) * 180 / M_PI; 
        
    return fabs(NormalizeAngle_180(sideH - boxH));    
    
}


// offset the right heading leftways by offset
// i.e. make the gap smaller by the box offset 
// of the right heading
double 
Obstacle::offsetRight(double heading, double offset)
{
    double result = heading + offset;
    if(result > 180) result = 180;
    return result;   
}

// offset the left heading rightways by offset
// i.e. make the gap smaller by the box offset 
// of the left heading
double 
Obstacle::offsetLeft(double heading, double offset)
{
    double result = heading - offset;
    if(result < -180) result = -180;
    return result;    
}

// True if p is inside the rectangle. 
//NOTE: rect points should go around the rectangle, i.e. successive
//rectangle points should NOT be diagonally opposite
bool Obstacle::isInRectangle(int pX, int pY, int rectPt1X, int rectPt1Y,
                            int rectPt2X, int rectPt2Y, int rectPt3X,
                            int rectPt3Y, int rectPt4X, int rectPt4Y)
{
    // Draw a ray from (pX, pY) and count the number of intersections with
    // the rectangle. We draw the ray horizontally from p for easy math
    int crossovers = 0;
    
    if (intersectsRay(pX, pY, rectPt1X, rectPt1Y, rectPt2X, rectPt2Y))
        crossovers++;
    if (intersectsRay(pX, pY, rectPt2X, rectPt2Y, rectPt3X, rectPt3Y))
        crossovers++;
    if (intersectsRay(pX, pY, rectPt3X, rectPt3Y, rectPt4X, rectPt4Y))
        crossovers++;
    if (intersectsRay(pX, pY, rectPt4X, rectPt4Y, rectPt1X, rectPt1Y))
        crossovers++;   

    // If there is an even number of intersections then the point is 
    // not inside the rectangle
    if ((crossovers % 2) == 0) {
        return false;
    }
    //cerr << "point " << px << ", " << py << " inside rect";
    return true;   
}
        

/* True if the line segment (p1, p2) intersects the ray from (rx, ry) with
 * angle zero (i.e. horizontal to the right)
 */
bool
Obstacle::intersectsRay(int rx, int ry, int p1x, int p1y, int p2x, int p2y) {
    // A horizontal line isn't intercepting any horizontal ray
    if (p2y - p1y == 0)
        return false;

    // Representing the line (segment) in two point form calculate the
    // intercept with the horizontal line through (rx, ry)
    double rayXInt = p1x + ((ry - p1y)*(p2x - p1x) / (p2y - p1y));
    
    // If intercept is to the left of r then no intersection. If it's to
    // the right of r then check that the intercept is within the segment
    if (rayXInt > rx) {
        if ((p1y > ry && p2y < ry)
                || (p1y < ry && p2y > ry))
            return true;
    }
    return false;
}
