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


#include <iostream>
#include <fstream>
#include <sstream>

#include <qapplication.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qtabwidget.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qfont.h>
#include <qpainter.h>
#include <qsocketdevice.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qapplication.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qframe.h>

 
#include "FormSubvisionImpl.h"
#include "../../robot/share/Common.h"
#include "../robolink/robolink.h"
#include "../../robot/vision/Profile.h"
#define DEBUG 0

void FormSubvisionImpl::resetLineCounter(void){
    for (int x = 0 ; x < ARRAY_SIZE_X ; ++x) 
    {
        for (int y = 0 ; y < ARRAY_SIZE_Y ; ++y)
        {
            lineCounter[x][y] = 0;
        }
    }
}

/* 
 * Tweaking doesn't work if using YUV plane
 */
void FormSubvisionImpl::slot_EC_TEW_Default(){
    if (DEBUG)
        cerr << __func__ << endl;

    TXT_EC_TEW_Reduce_Dist->setText("4900");//70
    TXT_EC_TEW_Cull_Dist->setText("40000");  //350
    TXT_EC_TEW_Min_Dist->setText("4");   //4
    TXT_EC_TEW_Min_Head_Speed->setText("0.09");   //0.09
    TXT_EC_TEW_Max_Sqr_Dist_Field->setText("25.0");   //25
}

/* Calculating all the edge point this is get called by play fast button*/
void FormSubvisionImpl::calculateEdgePointAllFrame() { 
    int frame;
    int counter = 0;
    double totalEdgePoints = 0;
    
    vector<VisualFeature>::iterator itr;
    lineOutsideCounter = 0;
    this->clearData();
    DogEdgeTable detObj;
    //Initiate arrays
    detObj.dogsEdgePointsQuantity.resize(ARRAY_SIZE_X);
    for (int x= 0; x < ARRAY_SIZE_X ; ++x)
        detObj.dogsEdgePointsQuantity[x].resize(ARRAY_SIZE_Y);
        
    detObj.dogsEdgePointsWeight.resize(ARRAY_SIZE_X);
    for (int x= 0; x < ARRAY_SIZE_X ; ++x)
        detObj.dogsEdgePointsWeight[x].resize(ARRAY_SIZE_Y);
    
    if (TXT_EC_TEW_Cull_Dist->text() == "" ||
        TXT_EC_TEW_Reduce_Dist->text() == "" ||
        TXT_EC_TEW_Min_Dist->text() == "" ||
        TXT_EC_TEW_Min_Head_Speed->text() == "" || 
        TXT_EC_TEW_Max_Sqr_Dist_Field->text() == "" 
    )
        slot_EC_TEW_Default();
    
    while ((frame = readFrame()) != -1) {

        if (this->shouldSampleImage)
            this->sampleImage();
        else 
            this->initRobotCondition();        
            
        //Using offline manually
#if 0         
        int maxPoints = 500;
        double inPoints[maxPoints*2];
        int sizeInPoints = 0;
        itr= this->features.begin();
        //cout << "this->features.size():"<<this->features.size() << endl;
        counter = 0;
        //Getting initial field line features points
        for (; itr != this->features.end() && counter < maxPoints; ++itr) {
            if (itr->type != VF_FIELDLINE)
                continue;
            inPoints[(counter*2)] = itr->x;
            inPoints[(counter*2)+1] = itr->y;
            ++counter;
            //cout << __func__ << " found a fieldline" << endl;
        }
        sizeInPoints = counter;
        double afterFiltering[counter*2];
        
        
        /* project it and filter the points */
        int no_of_points = 
            projectAndFilterPoints(inPoints,afterFiltering,sizeInPoints);
#endif
        double afterGlobal[cortex.numEdgePnts*2];
        localToGlobal(
            gps.r[0].mean.pos.x,
            gps.r[0].mean.pos.y,
            DEG2RAD(gps.r[0].mean.h),
            cortex.numEdgePnts, 
            cortex.localEdgePnts, 
            afterGlobal);
        /* Top view display*/
        /* after adjust to top view */
        int xpos, ypos;
        for (counter = 0; counter < cortex.numEdgePnts ; ++counter) 
        {   
            if (cortex.edgePntTypes[counter] != LINE_EDGE) 
                continue;
            xpos = (int) (afterGlobal[counter*2] + VisualCortex::offsetX);
            ypos = (int) (afterGlobal[counter*2+1] + VisualCortex::offsetY);
            if (xpos < 0 || 
                xpos >= ARRAY_SIZE_X || 
                ypos < 0 || 
                ypos >= ARRAY_SIZE_Y)
                ++lineOutsideCounter;
            else  
            {
                ++detObj.dogsEdgePointsQuantity[xpos][ypos];
                detObj.dogsEdgePointsWeight[xpos][ypos] = cortex.edgeWeights[counter];
            }
            ++totalEdgePoints;
        }
        //cout << "no_of_points:" << no_of_points << endl;
        this->clearData();
    }
    tablePerFile.push_back(detObj);
    stringstream ss;
    ss << (((QFile*)this->input)->name()).data() << " EP:" << totalEdgePoints << " OutEP:" << lineOutsideCounter;
    LB_DataLoaded->insertItem( ss.str());
    LB_DataLoaded->setSelected(LB_DataLoaded->count()-1,true);
    //resetLineCounter();
    cout << "totalEdgePoints:" << totalEdgePoints << endl;
    cout << "lineOutsideCounter:" << lineOutsideCounter << endl;
    
    this->slotRedraw();
    //cout << "FIELDLINE sizeInPoints:" << sizeInPoints <<endl;
    //cout << "FIELDLINE after filtering:" << no_of_points <<endl;
}

void FormSubvisionImpl::calibrateEdge() { 
    if (DEBUG)
        cerr << __func__ << endl;
    if (tabDisplayInfo->currentPage()->name() != QString("tabEdgeCalibration"))
    { return; }
    slotDPixmapRedraw();
}

void FormSubvisionImpl::DPixmapDrawProbability(QPainter &p, double convert, double offsetDisplayX, double offsetDisplayY){
    if (DEBUG)
        cerr << __func__ << endl;
    
    double pointX1,pointY1;

    if (this->CB_EC_Display_ShowLineMatch->isChecked()) {
        for (int yInd = -VisualCortex::offsetY; yInd <= FIELD_LENGTH + VisualCortex::offsetY; yInd++) {
            for (int xInd = -VisualCortex::offsetX; xInd <= FIELD_WIDTH + VisualCortex::offsetX; xInd++) {
                double gx,gy;
                double match = cortex.calcPointMatch(xInd,yInd,LINE_EDGE,&gx,&gy);
                int colordiff = (int) (match * 255);
                if (colordiff > 255) 
                    cout << __func__ << " xInd:"<<xInd << " yInd:"<<yInd << " match:" << match << endl;
                pointY1 = xInd * convert + offsetDisplayY;
                pointX1 = yInd * convert + offsetDisplayX;
                
                p.setPen(QPen(QColor(CLIP(colordiff,255),255,CLIP(colordiff,255)),1));
                p.drawPoint((int)pointX1,(int)pointY1);
            }   
        }
    }
    // FIELD match table    
    if (CB_EC_Display_ShowFieldMatch->isChecked()) {
        for (int yInd = -VisualCortex::offsetY; yInd <= FIELD_LENGTH + VisualCortex::offsetY; yInd++) {
            for (int xInd = -VisualCortex::offsetX; xInd <= FIELD_WIDTH + VisualCortex::offsetX; xInd++) {
                double gx,gy;
                double match = cortex.calcPointMatch(xInd,yInd,FIELD,&gx,&gy);
                int colordiff = (int) (match * 255);
                if (colordiff > 255) 
                    cout << __func__ << " xInd:"<<xInd << " yInd:"<<yInd << " match:" << match << endl;
                pointY1 = xInd * convert + offsetDisplayY;
                pointX1 = yInd * convert + offsetDisplayX;
                
                p.setPen(QPen(QColor(CLIP(colordiff,255),255,CLIP(colordiff,255)),1));
                p.drawPoint((int)pointX1,(int)pointY1);
            }   
        }
    }

}

void FormSubvisionImpl::DPixmapDrawPointProjection(
    QPainter &p, 
    double convert, 
    double offsetDisplayX, 
    double offsetDisplayY,
    double xpos,
    double ypos,
    double hpos)
{
    if (DEBUG)
        cerr << __func__ << endl;
    
    /*Draw the point projection*/
    double pointX1,pointY1;
    /* Drawing points */
    int maxPoints = 500;
    int counter = 0;
    double inPoints[maxPoints*2];
    int sizeInPoints = 0;
    vector<VisualFeature>::iterator itr;
    
    bool isSelected = false;
    for (unsigned x = 0; x < LB_DataLoaded->count() ; ++x) 
    {
        if (LB_DataLoaded->isSelected(x))
        {
            isSelected = true;
            break;
        }        
    }
    if (isSelected == false) // none selected then use a frame view
    {    
        if (this->CB_EC_Display_Obstacle->isChecked())
        {
            itr= this->obstacleFeatures.begin();
            counter = 0;
            //Getting initial obstacle features points
            for (; itr != this->obstacleFeatures.end() && counter < maxPoints*2 - 1 ; ++itr) {
                inPoints[counter] = itr->x;
                inPoints[counter+1] = itr->y;
                counter +=2;
            }
            sizeInPoints = counter/2;
            double afterFiltering[counter];
            
            /* project it and filter the points */
            int noOfObstaclePoints = 
                projectAndFilterPoints(inPoints,afterFiltering,sizeInPoints, MAX_VIEW_ACCURATE_OBSTACLE, MAX_VIEW_ACCURATE_OBSTACLE);
                
            //set the obstacle objects with obstacle points            
            //obstacle.setObstaclePoints(afterFiltering,noOfObstaclePoints);
            
            /* Convert to global coordinate */
            localToGlobal(xpos,ypos,hpos,  noOfObstaclePoints,afterFiltering, afterFiltering);
            
            /* Top view display*/
            /* after adjust to top view */
            for (counter = 0; counter < noOfObstaclePoints; ++counter) 
            {
                /* adjusting to top view */
                pointY1 = afterFiltering[(counter*2)] * convert + offsetDisplayY;
                pointX1 = afterFiltering[(counter*2)+1] * convert + offsetDisplayX;
                //draw
                p.setPen(QPen("red", 1));
                p.drawPoint((int)pointX1,(int)pointY1);
            }
            //cout << "sizeInPoints:" << sizeInPoints <<endl;
            //cout << "after filtering:" << noOfObstaclePoints <<endl;        
        }
        
        if (this->CB_EC_Display_Fieldline->isChecked())
        {
            itr= this->features.begin();
            counter = 0;
            //Getting initial field line features points
            for (; itr != this->features.end() && counter < maxPoints*2 - 1 ; ++itr) {
                if (itr->type != VF_FIELDLINE)
                    continue;
                inPoints[counter] = itr->x;
                inPoints[counter+1] = itr->y;
                counter +=2;
            }
            sizeInPoints = counter/2;
            double afterFiltering[counter];
            
            /* project it and filter the points */
            int no_of_points = 
                projectAndFilterPoints(inPoints,afterFiltering,sizeInPoints);
            
            /* Convert to global coordinate */
            localToGlobal(xpos,ypos,hpos, no_of_points,afterFiltering, afterFiltering);
                        
            /* Top view display*/
            /* after adjust to top view */
            for (counter = 0; counter < no_of_points ; ++counter) 
            {
                /* adjusting to top view */
                /* adjusting to top view */
                pointY1 = afterFiltering[(counter*2)] * convert + offsetDisplayY;
                pointX1 = afterFiltering[(counter*2)+1] * convert + offsetDisplayX;
                //draw
                p.setPen(QPen("red", 1));
                p.drawPoint((int)pointX1,(int)pointY1);
            }
            //cout << "FIELDLINE sizeInPoints:" << sizeInPoints <<endl;
            //cout << "FIELDLINE camera:" << no_of_points <<endl;        
        }

        if (this->CB_EC_Display_Fieldline_After_Filter->isChecked())
        {
            counter = 0;
            int no_of_points = 0;
            //Getting initial field line features points
            for (counter = 0; counter < cortex.numEdgePnts ; ++counter) 
            {   
                if (cortex.edgePntTypes[counter] != LINE_EDGE) 
                    continue;
                inPoints[no_of_points*2] = cortex.localEdgePnts[counter*2];
                inPoints[no_of_points*2 + 1] = cortex.localEdgePnts[counter*2 + 1];
                ++no_of_points;
            }
            double afterFiltering[counter*2];
                        
            /* Convert to global coordinate */
            localToGlobal(xpos,ypos,hpos, no_of_points,inPoints, afterFiltering);
            /* Top view display*/
            /* after adjust to top view */
            for (counter = 0; counter < no_of_points ; ++counter) 
            {
                /* adjusting to top view */
                pointY1 = afterFiltering[(counter*2)] * convert + offsetDisplayY;
                pointX1 = afterFiltering[(counter*2)+1] * convert + offsetDisplayX;
                //draw
                p.setPen(QPen("green", 1));
                p.drawPoint((int)pointX1,(int)pointY1);
            }
            //cout << "FIELDLINE sizeInPoints:" << sizeInPoints <<endl;
            //cout << "FIELDLINE processed:" << no_of_points <<endl;
        }        

        if (this->CB_EC_Display_Field->isChecked())
        {
            itr= this->features.begin();
            counter = 0;
            //Getting initial field features points
            for (; itr != this->features.end() && counter < maxPoints*2 - 1 ; ++itr) {
                if (itr->type != VF_FIELD)
                    continue;
                inPoints[counter] = itr->x;
                inPoints[counter+1] = itr->y;
                counter +=2;
            }
            sizeInPoints = counter/2;
            double afterFiltering[counter];
            
            /* project it and filter the points */
            int no_of_points = 
                projectAndFilterPoints(inPoints,afterFiltering,sizeInPoints);
    
            /* Convert to global coordinate */
            localToGlobal(xpos,ypos,hpos, no_of_points,afterFiltering, afterFiltering);
                                    
            /* Top view display*/
            /* after adjust to top view */
            for (counter = 0; counter < no_of_points; ++counter) 
            {
                /* adjusting to top view */
                pointY1 = afterFiltering[(counter*2)] * convert + offsetDisplayY;
                pointX1 = afterFiltering[(counter*2)+1] * convert + offsetDisplayX;
                //draw
                p.setPen(QPen("blue", 1));
                p.drawPoint((int)pointX1,(int)pointY1);
            }
            //cout << "FIELD sizeInPoints:" << sizeInPoints <<endl;
            //cout << "FIELD after filtering:" << no_of_points <<endl;        
        }    
    }
    else  // else use the list selected values
    {
        int maximum = 0;
        resetLineCounter();
        /*
        cout << "tablePerFile size:"<< tablePerFile.size() << endl;
        cout << "tablePerFile[0] size:"<< tablePerFile[0].size() << endl;
        cout << "tablePerFile[0][0] size:"<< tablePerFile[0][0].size() << endl;
        */
        for (unsigned tableindex = 0; tableindex < LB_DataLoaded->count();++tableindex)
        {
            if (!LB_DataLoaded->isSelected(tableindex))
                continue;
            for (int yInd = 0; yInd < ARRAY_SIZE_Y; yInd++) {
                for (int xInd = 0; xInd < ARRAY_SIZE_X; xInd++) {
                    if (rb_EC_EPV_Quantity->isChecked())
                    {
                        lineCounter[xInd][yInd] += tablePerFile[tableindex].dogsEdgePointsQuantity[xInd][yInd];
                        if (lineCounter[xInd][yInd] > maximum)
                        {
                            maximum = lineCounter[xInd][yInd];
                        }
                    }
                    else
                    {
                        maximum = 10000;
                        lineCounter[xInd][yInd] = (int)(tablePerFile[tableindex].dogsEdgePointsWeight[xInd][yInd]*maximum);
                    }    
                }
            }           
        }
        
        if (maximum == 0) //remove posibility no points calculated
            maximum = 1;
        
        cout << "maximum:" << maximum << endl;
        for (int yInd = 0; yInd < ARRAY_SIZE_Y; yInd++) {
            for (int xInd = 0; xInd < ARRAY_SIZE_X; xInd++) {
                int match = lineCounter[xInd][yInd];
                if (! match > 0)
                    continue;
                int colordiff = 0;
                colordiff = (int) (255*match/maximum);
                if (colordiff > 255) 
                    cout << __func__ << " xInd:"<<xInd << " yInd:"<<yInd << " match:" << match << endl;
                pointY1 = (xInd-VisualCortex::offsetX) * convert + offsetDisplayY;
                pointX1 = (yInd-VisualCortex::offsetY) * convert + offsetDisplayX;
                
                p.setPen(QPen(QColor(0,CLIP(colordiff,255),255),1));
                p.drawPoint((int)pointX1,(int)pointY1);
            }   
        }
        
    }    
    
    stringstream str;
    str.str("");
    str << xpos;
    this->EC_GPSPosX->setText(str.str());
    str.str("");
    str << ypos;
    this->EC_GPSPosY->setText(str.str());
    str.str("");
    str << RAD2DEG(hpos);
    this->EC_GPSHeading->setText(str.str());
    
}

void FormSubvisionImpl::DPixmapDrawField(QPainter &p, double convert, double offsetDisplayX, double offsetDisplayY){
    if (DEBUG)
        cerr << __func__ << endl;
    
    double pointX1,pointY1,pointX2,pointY2;

    //Draw beacon pink blue
    pointY1 = -BEACON_EDGE_LINE_OFFSET_X;
    pointX1 = BEACON_BOTTOM_LINE_OFFSET_Y-BEACON_RADIUS;
    pointY2 = pointY1/2;
    pointX2 = BEACON_BOTTOM_LINE_OFFSET_Y+BEACON_RADIUS;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("pink"),2));
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    p.setPen(QPen(QColor("blue"),2));
    p.drawRect((int)pointX1,(int)pointY2,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    
    //pink yellow
    pointY1 = -BEACON_EDGE_LINE_OFFSET_X;
    pointX1 = 3*BEACON_BOTTOM_LINE_OFFSET_Y-BEACON_RADIUS;
    pointY2 = pointY1/2;
    pointX2 = 3*BEACON_BOTTOM_LINE_OFFSET_Y+BEACON_RADIUS;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("pink"),2));
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    p.setPen(QPen(QColor("yellow"),2));
    p.drawRect((int)pointX1,(int)pointY2,(int)(pointX2-pointX1),(int)(pointY2-pointY1));       

    //Draw beacon blue pink
    pointY1 = FIELD_WIDTH;
    pointX1 = BEACON_BOTTOM_LINE_OFFSET_Y-BEACON_RADIUS;
    pointY2 = FIELD_WIDTH+BEACON_EDGE_LINE_OFFSET_X/2;
    pointX2 = BEACON_BOTTOM_LINE_OFFSET_Y+BEACON_RADIUS;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("blue"),2));
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    p.setPen(QPen(QColor("pink"),2));
    p.drawRect((int)pointX1,(int)pointY2,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    
    //yellow pink
    pointY1 = FIELD_WIDTH;
    pointX1 = 3*BEACON_BOTTOM_LINE_OFFSET_Y-BEACON_RADIUS;
    pointY2 = FIELD_WIDTH+BEACON_EDGE_LINE_OFFSET_X/2;
    pointX2 = 3*BEACON_BOTTOM_LINE_OFFSET_Y+BEACON_RADIUS;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("yellow"),2));
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    p.setPen(QPen(QColor("pink"),2));
    p.drawRect((int)pointX1,(int)pointY2,(int)(pointX2-pointX1),(int)(pointY2-pointY1));       
        
    //Draw Field
    pointY1 = 0;
    pointX1 = 0;
    pointY2 = FIELD_WIDTH;
    pointX2 = FIELD_LENGTH;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("white"),2));
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    
#ifdef OSAKA
    //Draw WALL
    pointY1 = -SIDE_WALL_FRINGE_WIDTH;
    pointX1 = -GOAL_WALL_FRINGE_WIDTH;
    pointY2 = FIELD_WIDTH+SIDE_WALL_FRINGE_WIDTH;
    pointX2 = FIELD_LENGTH+GOAL_WALL_FRINGE_WIDTH;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("white"),2));
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
#endif
    
    //Draw middle line
    pointY1 = 0;
    pointX1 = FIELD_LENGTH/2;
    pointY2 = FIELD_WIDTH;
    pointX2 = FIELD_LENGTH/2;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("white"),2));
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
    p.drawLine((int)pointX1,(int)pointY1,(int)(pointX2),(int)(pointY2));
    
    //Draw own goalbox line
    pointY1 = FIELD_WIDTH/2 - GOALBOX_WIDTH/2;
    pointX1 = 0;
    pointY2 = FIELD_WIDTH/2 + GOALBOX_WIDTH/2;
    pointX2 = GOALBOX_DEPTH;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("white"),2));
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    
    //Draw enemy goalbox line
    pointY1 = FIELD_WIDTH/2 - GOALBOX_WIDTH/2;
    pointX1 = FIELD_LENGTH - GOALBOX_DEPTH;
    pointY2 = FIELD_WIDTH/2 + GOALBOX_WIDTH/2;
    pointX2 = FIELD_LENGTH;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("white"),2));
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    
    //Draw center circle
    pointY1 = FIELD_WIDTH/2-CIRCLE_DIAMETER/2;
    pointX1 = FIELD_LENGTH/2-CIRCLE_DIAMETER/2;
    pointY2 = FIELD_WIDTH/2+CIRCLE_DIAMETER/2;
    pointX2 = FIELD_LENGTH/2+CIRCLE_DIAMETER/2;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("white"),2));
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
    p.drawEllipse((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    
    //Draw blue goal
    //original point
    pointY1 = (FIELD_WIDTH-GOAL_WIDTH)/2;
    pointX1 = -GOAL_LENGTH;
    pointY2 = (FIELD_WIDTH+GOAL_WIDTH)/2;
    pointX2 = 0;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("blue"),2));
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    
    //Draw yellow goal
    //original point
    pointY1 = (FIELD_WIDTH-GOAL_WIDTH)/2;
    pointX1 = FIELD_LENGTH;
    pointY2 = (FIELD_WIDTH+GOAL_WIDTH)/2;
    pointX2 = FIELD_LENGTH+GOAL_LENGTH;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    p.setPen(QPen(QColor("yellow"),2));
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));    
}

void FormSubvisionImpl::DPixmapDrawDog(
            QPainter &p, 
            double convert, 
            double offsetDisplayX, 
            double offsetDisplayY,
            double xpos,
            double ypos,
            double hpos){
    if (DEBUG)
        cerr << __func__ << endl;
    
    double pointX1,pointY1,pointX2,pointY2;
    /* change x and y because it is horizontal view. */
    pointY1 = xpos-2;
    pointX1 = ypos-2;
    
    pointY1 = pointY1 * convert + offsetDisplayY;
    pointX1 = pointX1 * convert + offsetDisplayX;
    
    p.setPen(QPen(QColor("red"),1));
    p.drawEllipse((int)pointX1,(int)pointY1, (int)(4*convert), (int)(4*convert));
    
    //Draw heading
    int arrow = 15;
    pointY2 = cos(hpos) * arrow + xpos;
    pointX2 = sin(hpos) * arrow + ypos;
    pointY2 = pointY2 * convert + offsetDisplayY;
    pointX2 = pointX2 * convert + offsetDisplayX;
    
    //cout << "xend:" << xend << " yend:" << yend << endl;
    p.setPen(QPen(QColor("red"),1));
    p.drawLine((int)pointX1,(int)pointY1,(int)(pointX2),(int)(pointY2));
}

#ifdef LOCALISATION_CHALLENGE
void drawPinkMap(
            QPainter &p, 
            double convert, 
            double offsetDisplayX, 
            double offsetDisplayY,
            int wholeField[PINK_ARRAY_BOX_SIZE_X] [PINK_ARRAY_BOX_SIZE_Y], 
            vector < PinkEQ > listPinkEQ,
            double *pinkX,
            double *pinkY,
            int numPink
            )
{
    if (DEBUG)
        cerr << __func__ << endl;
        
    for (int indexX = 0; indexX < PINK_ARRAY_BOX_SIZE_X; ++indexX)
    {
        for (int indexY = 0; indexY < PINK_ARRAY_BOX_SIZE_Y; ++indexY)
        {
            if (wholeField[indexX][indexY] < 1)
                continue;
            double pointX1,pointY1,pointX2,pointY2;
            pointY1 = (indexX-OFFSET_BOX_X) * BOXSIZE_X;
            pointX1 = (indexY-OFFSET_BOX_Y)* BOXSIZE_Y;
            pointY2 = pointY1 + BOXSIZE_X;
            pointX2 = pointX1 + BOXSIZE_Y;
            //cout << __func__ << " before pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
            pointY1 = pointY1 * convert + offsetDisplayY;
            pointX1 = pointX1 * convert + offsetDisplayX;
            pointY2 = pointY2 * convert + offsetDisplayY;
            pointX2 = pointX2 * convert + offsetDisplayX;
            //cout << __func__ << " after pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
            if (wholeField[indexX][indexY] == 1)
                p.setPen(QPen(QColor("pink"),2));
            else if (wholeField[indexX][indexY] == 2)
                p.setPen(QPen(QColor("red"),2));
            else if (wholeField[indexX][indexY] == 3)
                p.setPen(QPen(QColor("blue"),2));
            else if (wholeField[indexX][indexY] == 4)
                p.setPen(QPen(QColor("yellow"),2));
            else if (wholeField[indexX][indexY] > 4)
                p.setPen(QPen(QColor("green"),2));
            
            p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));    
        }
    }
    int counter = 0;
    for (vector < PinkEQ >::iterator peq = listPinkEQ.begin(); peq != listPinkEQ.end(); ++peq, ++counter)
    {
        double pointX1,pointY1,pointX2,pointY2;
        if (peq->type == FLUp)
        {
            pointX1 = FIELD_LENGTH+OUTER_BARRIER_Y;
            pointY1 = (pointX1 - peq->c) / peq->m;
            pointX2 = FIELD_LENGTH;
            pointY2 = (pointX2 - peq->c) / peq->m;
        }        
        else if (peq->type == FLDown)
        {
            pointX1 = -OUTER_BARRIER_Y;
            pointY1 = (pointX1 - peq->c) / peq->m;
            pointX2 = 0.0;
            pointY2 = (pointX2 - peq->c) / peq->m;
        }        
        else if (peq->type == FLLeft)
        {
            pointY1 = -OUTER_BARRIER_X;
            pointX1 = pointY1 * peq->m + peq->c;
            pointY2 = 0.0;
            pointX2 = pointY2 * peq->m + peq->c;
        }        
        else if (peq->type == FLRight)
        {
            pointY1 = FIELD_WIDTH+OUTER_BARRIER_X;
            pointX1 = pointY1 * peq->m + peq->c;
            pointY2 = FIELD_WIDTH;
            pointX2 = pointY2 * peq->m + peq->c;
        }   
        else {
            continue;
        }     
        /*
        if (peq->type == FLLeft)
            cout << __func__ << "["<<counter<<"] before pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
        if (counter < 23 || counter > 25 )
            continue;
        */
        pointY1 = pointY1 * convert + offsetDisplayY;
        pointX1 = pointX1 * convert + offsetDisplayX;
        pointY2 = pointY2 * convert + offsetDisplayY;
        pointX2 = pointX2 * convert + offsetDisplayX;
        //cout << __func__ << " after pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
        p.setPen(QPen(QColor("black"),1));
        p.drawLine((int)pointX1,(int)pointY1,(int)(pointX2),(int)(pointY2));
    }

#ifdef DRAW_ALL_INTERSECTION    
    //Drawing the intersection points
    PinkEQ firstEq,secondEq, tempEq;
    double x, y;
    for ( unsigned firstIndex = 0; firstIndex < listPinkEQ.size() - 1; ++firstIndex)
    {
        firstEq = listPinkEQ[firstIndex];
        for ( unsigned secondIndex = firstIndex+1; secondIndex < listPinkEQ.size(); ++secondIndex)
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
            double pointX1,pointY1;
            pointY1 = x;
            pointX1 = y;
            pointY1 = pointY1 * convert + offsetDisplayY;
            pointX1 = pointX1 * convert + offsetDisplayX;
            p.setPen(QPen(QColor("white"),5));
            /*
            cout << __func__ << " intersection x:" << x << " Y:" << y << " between findex:" << firstIndex << " sindex:" << secondIndex << endl;
            cout << __func__ << " 1stType:" << firstEq.type  << " 2ndType:" << secondEq.type << endl;
            */
            p.drawEllipse((int)pointX1-2,(int)pointY1-2,4,4);
        }
    }        
#else //DRAW_ALL_INTERSECTION
    
    //cout << __func__ << "Start Drawing points, numPink:" << numPink << endl;
    for (int index = 0; index < numPink; ++index)
    {   
        double pointX1,pointY1;
        pointY1 = pinkX[index];
        pointX1 = pinkY[index];
        pointY1 = pointY1 * convert + offsetDisplayY;
        pointX1 = pointX1 * convert + offsetDisplayX;
        p.setPen(QPen(QColor("white"),5));
        //cout << __func__ << " x:" << pinkX[index] << " Y:" << pinkY[index];
        p.drawEllipse((int)pointX1-2,(int)pointY1-2,4,4);
    }
#endif //DRAW_ALL_INTERSECTION
    
}
#endif

void FormSubvisionImpl::slotDPixmapRedraw(){
    if (DEBUG)
        cerr << __func__ << endl;
    
    int drawingHeight = dpixmap.height();
    int drawingWidth = dpixmap.width();
    //return;
    
    QPixmap temppixmap(drawingWidth,drawingHeight);
    temppixmap.fill(Qt::darkGray);                                          
    
    
    //Finding best resolution
    //based on drawing horizontal
    int extraX = VisualCortex::offsetX;
    int extraY = VisualCortex::offsetY;
    double widthNeeded = FIELD_LENGTH + 2 * GOAL_FRINGE_WIDTH + 2 * extraX;
    double heightNeeded = FIELD_WIDTH + 2 * SIDE_FRINGE_WIDTH + 2 * extraY;
    double widthImage = drawingWidth;
    double heightImage = drawingHeight;
    double convert;
    double offsetDisplayX, offsetDisplayY;
    
    //Calculate which one is the most good match
    if ( widthImage/widthNeeded < heightImage/heightNeeded)
    {
        convert = widthImage/widthNeeded;
    }
    else 
    {
        convert = heightImage/heightNeeded;
    }
    
    //Additional offset to the side of the field
    offsetDisplayX = (extraX + GOAL_FRINGE_WIDTH) * convert;
    offsetDisplayY = (extraY + SIDE_FRINGE_WIDTH) * convert;
    
    QPainter p(&temppixmap);
    DPixmapDrawField(p, convert, offsetDisplayX, offsetDisplayY);
    if (RB_GPS_TV_PinkMap->isChecked())
    {
#ifdef LOCALISATION_CHALLENGE    
        int wholeField[PINK_ARRAY_BOX_SIZE_X][PINK_ARRAY_BOX_SIZE_Y];
        vector < PinkEQ > listPinkEQ;
        PinkEQ peq;
        ifstream fp_in;
        fp_in.open("DebugPinkMapping.txt", ifstream::in);
        if (! fp_in.good())
        {
            cerr << __func__ << " File is not exist" << endl;
            return;
        }
        while (fp_in.good())
        {
            fp_in >> peq.m;
            if (!fp_in.good())
            {
                break;
            }
            fp_in >> peq.c;
            fp_in >> peq.type;
            
            listPinkEQ.push_back(peq);
            //cout << __func__ << " " << peq.m << "\t" << peq.c << "\t" << peq.type << endl;
        }
        fp_in.close();
        gps.listPinkEQ = listPinkEQ;
        gps.CalcIntersections();
        gps.PinkLineToBox(listPinkEQ,wholeField);
        drawPinkMap(p, convert, offsetDisplayX, offsetDisplayY,wholeField,listPinkEQ, gps.pinkX, gps.pinkY, gps.numPink);
#endif        
    }
    else 
    {
        DPixmapDrawProbability(p, convert, offsetDisplayX, offsetDisplayY);
        double xpos, ypos, hpos;
        /* Drawing the duplicates of Possible.h */
        if (RB_GPS_TV_Duplicate->isChecked()){
            for (int d = 0; d < (int)gps.GA_Duplicates.size() ; ++d)
            {
                xpos = gps.GA_Duplicates[d](0,0);
                ypos = gps.GA_Duplicates[d](1,0);
                hpos = gps.GA_Duplicates[d](2,0);            
                DPixmapDrawPointProjection(p, convert, offsetDisplayX, offsetDisplayY,xpos,ypos,hpos);
                DPixmapDrawDog(p, convert, offsetDisplayX, offsetDisplayY,xpos,ypos,hpos);
            }
        }
        else
        {

            if (RB_GPS_TV_GA->isChecked() && gps.GA_robotsPos.size() != 0)
            {
                int index = SB_GPS_GradientAscent->value();
                if (index < 0 || index >= (int)gps.GA_robotsPos.size())
                {
                    cerr << __func__ << " Index out of range"<<endl;
                    index = gps.GA_robotsPos.size() - 1;
                }
                xpos = gps.GA_robotsPos[index](0,0);
                ypos = gps.GA_robotsPos[index](1,0);
                hpos = gps.GA_robotsPos[index](2,0);
            } 
            else
            {
                xpos = gps.r[0].mean.pos.x;
                ypos = gps.r[0].mean.pos.y;
                hpos = DEG2RAD(gps.r[0].mean.h);
            }
            
            DPixmapDrawPointProjection(p, convert, offsetDisplayX, offsetDisplayY,xpos,ypos,hpos);
            DPixmapDrawDog(p, convert, offsetDisplayX, offsetDisplayY,xpos,ypos,hpos);    
        }
    }     
    dpixmap.formPixmap->resize(drawingWidth,drawingHeight);
    dpixmap.formPixmap->setPixmap(temppixmap);
    dpixmap.show();
    if (DEBUG)
        cerr << __func__ << " End." << endl;
}

