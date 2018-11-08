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


#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qlistbox.h>
#include <qframe.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qspinbox.h>


#include "../../robot/share/Common.h"
#include "FormSubvisionImpl.h"
#include "../robolink/robolink.h"
#include "../../robot/vision/Profile.h"
//#define DEBUG 1

void FormSubvisionImpl::slotInitPink(){
#ifdef LOCALISATION_CHALLENGE
    gps.CalcIntersections();
#endif    
}

void FormSubvisionImpl::slotGPS_CP_Gaussians() { 
    if (DEBUG) {
        cerr << __func__ << endl;
    }
    if (this->CB_GPS_CP_GAUSSIANS->isChecked())
    {
        if (gotGaussians)
        {
            for (int i = 0; i < dogGaussiansSize && i < gps.maxGaussians; i++)
            {
                //cout << __func__ << " i:" << i << " weight:" << gps.r[i].weight << endl;
                gps.r[i].mean.pos.x = allgaussians[i].x;
                gps.r[i].mean.pos.y = allgaussians[i].y;
                gps.r[i].mean.h = allgaussians[i].h;
                gps.r[i].weight = allgaussians[i].weight;
                for (int x = 0;x < 3; ++x)
                    for (int y = 0;y < 3;++y)
                        gps.r[i].cov(x,y) = allgaussians[i].cov[x][y];                
            }
            gps.numGaussians = dogGaussiansSize;
            cout << "GAUSSIANS COPIED FROM THE DOG" <<endl;
            gps.GPSOutputSelfData();
            cout << "END GAUSSIANS COPIED FROM THE DOG" <<endl;
            //this->slotRedraw();
            slotGaussianDisplay();
        }        
    }
}

void FormSubvisionImpl::GPSOutputSelfData(ofstream &file) {
    if (DEBUG) {
        cerr << __func__ << endl;
    }

    file << "=== GPSOutputSelfData: " << gps.numGaussians << " modes ===\n";

    for (int i = 0; i < gps.numGaussians; i++) {
        file << "Gaussian " << i << " has weight " <<gps.r[i].weight << "\n";
        file << "Mean: x " << gps.r[i].mean.pos.x << "   y " <<gps.r[i].mean.pos.y;
        file << "   h " <<gps.r[i].mean.h << "   Covariance:\n";
        //r[i].cov.printOut();
        for (size_t row = 0 ; row < 3 ; row++) {
            file << "(";
            for (size_t col = 0 ; col < 3 ; col++) {
                file << "\t" << gps.r[i].cov(row,col);
            }
             file << "\t)" << endl;
        }
    }
    
    /*
    // Now output update stastics
    for (int i = 0; i < 10; i++) {
        double mean, stdDev;
    
        if (nX[i] > 0) {
            mean = sumX[i] / nX[i];
            stdDev = sqrt(sumXSquared[i] - SQUARE(mean));
        } else {
            mean = 0.0;
            stdDev = 0.0;
        }
    
        file << nX[i] << " " << i << " dimensional updates. ";
        file << "Mean: " << mean << "   Standard dev: " << stdDev << "\n";
    }
    */
}

void FormSubvisionImpl::displayGPSInfo() {
    if (DEBUG) {
        cerr << __func__ << endl;
    }
    
    if (tabDisplayInfo->currentPage()->name() != QString("tabGPSInfo"))
        return;
        
    if (this->CB_GPS_RESET->isChecked())
        gps.resetGaussians(FIELD_WIDTH/2,FIELD_LENGTH/2,90,SQUARE(FIELD_WIDTH), SQUARE(FIELD_LENGTH), SQUARE(360));
    
    if (!gotYUV && !gotSubObject)
    {
        this->CB_Visual_Update->setEnabled(false);
        this->CB_Visual_Update->setChecked(false);
    }
    else
    { 
        this->CB_Visual_Update->setEnabled(true);
    }
    
    if (!gotYUV && !gotSubVision)
    {
        this->CB_Edge_Update->setEnabled(false);
        this->CB_Edge_Update->setChecked(false);
        this->CB_VisualConfidence->setEnabled(false);
        this->CB_VisualConfidence->setChecked(false);
    }
    else 
    {
        this->CB_Edge_Update->setEnabled(true);
        this->CB_VisualConfidence->setEnabled(true);
    }
        
    if (!gotGPSInfo)            
    {
        this->CB_Motion_Update->setEnabled(false);
        this->CB_Motion_Update->setChecked(false);
    }
    else
    {
        this->CB_Motion_Update->setEnabled(true);
    }
    
    if (!gotYUV && !gotPinkObject)
    {
        this->CB_Pink_Mapping->setEnabled(false);
        this->CB_Pink_Update->setEnabled(false);
        this->CB_Pink_Mapping->setChecked(false);
        this->CB_Pink_Update->setChecked(false);        
    }
    else
    {
        this->CB_Pink_Mapping->setEnabled(true);
        this->CB_Pink_Update->setEnabled(true);
    }
#ifdef TILT_CALIBRATION
    this->CB_Visual_Update->setEnabled(false);
    this->CB_Visual_Update->setChecked(false);
    this->CB_Motion_Update->setEnabled(false);
    this->CB_Motion_Update->setChecked(false);
    this->CB_VisualConfidence->setEnabled(false);
    this->CB_VisualConfidence->setChecked(false);
    this->CB_Pink_Mapping->setEnabled(false);
    this->CB_Pink_Update->setEnabled(false);
    this->CB_Pink_Mapping->setChecked(false);
    this->CB_Pink_Update->setChecked(false);
    this->CB_Edge_Update_2->setChecked(false);
    this->CB_GPS_COUT->setChecked(false);
#endif
    gps.SetGoals(false); //blue dog attack yellow goal

    //No YUV, try to get from subobject
    if (!gotYUV && gotSubObject)
        copyVobToCortex();

    if (!gotYUV && gotPinkObject)
    {
        memcpy(this->cortex.localisePink, this->pinkObjects, sizeof(this->pinkObjects));
        this->cortex.numLocalisePink = num_of_pink;
    }    

    if (this->CB_GPS_CP_GAUSSIANS->isChecked())
    {
        slotGPS_CP_Gaussians();
    }
    else {
        if (this->CB_GPS_COUT->isChecked())
        {
            cout <<"Before the Update" <<endl;
            gps.GPSOutputSelfData();
            cout <<"End Before the Update" <<endl <<endl;
        }
        /* 
         * Vision Update
         */
        ofstream fp_out;
        fp_out.open("DebugVisual.txt", ios::out);
        //only if YUV or object is recognised and box is ticked
        if (this->CB_Visual_Update->isChecked())
        {
            /*
            if (this->CB_Visual_Update_2->isChecked())
            {
                gps.outputStream = &fp_out;
                cortex.outputStream = &fp_out;
            } 
            else                
            {
                gps.outputStream = NULL;
                cortex.outputStream = NULL;
            }    
            */
            gps.GPSVisionUpdate(this->cortex.vob, !(ABS(cortex.headMovement) <= EPSILON) );           
            if (this->CB_GPS_COUT->isChecked())
            {
                fp_out <<"After vision update" <<endl;
                GPSOutputSelfData(fp_out);
                fp_out <<"End After vision update" <<endl <<endl;
                /*
                cout <<"After vision update" <<endl;
                gps.GPSOutputSelfData();
                cout <<"End After vision update" <<endl <<endl;
                */
            }
            /*
            gps.outputStream = NULL;
            cortex.outputStream = NULL;            
            */
        }
        fp_out.close();
        
        fp_out.open("DebugPink.txt", ios::out);
        gps.outputStream = &fp_out;
        if (this->CB_Pink_Mapping->isChecked())
        {
            cout << "trying to map pink no:" << this->cortex.numLocalisePink << endl;
            gps.allowPinkMapping = true;
            gps.MapLocalisePink(this->cortex.localisePink, this->cortex.numLocalisePink);
        }   
        #ifdef LOCALISATION_CHALLENGE
        gps.bodyMoving = this->CB_Pink_Update_Body_Move->isChecked();
        #endif
        if (this->CB_Pink_Update->isChecked())
            gps.LocalisePinkUpdate(&this->cortex);
         
        fp_out << __func__ << " AFTER PINK MAPPING " << endl;
        GPSOutputSelfData(fp_out);
        fp_out.close();
        gps.outputStream = NULL;
                
        /* 
         * Edge Update
         */
        //only if YUV or object is recognised and box is ticked
        fp_out.open("DebugEdge.txt", ios::out);
        if (this->CB_Edge_Update->isChecked())
        {
            if (!gotGPSInfo)
            {
                gpsinfo.headSpeed = 0.0;
            }
            if (this->CB_Edge_Update_2->isChecked())
            {
                gps.outputStream = &fp_out;
                cortex.outputStream = &fp_out;
            } 
            else                
            {
                gps.outputStream = NULL;
                cortex.outputStream = NULL;
            }    
            gps.GPSEdgeUpdate(&this->cortex, gpsinfo.headSpeed);
                    
            if (this->CB_GPS_COUT->isChecked())
            {
                fp_out <<"After GPSEdgeUpdate update, headspeed:"<<gpsinfo.headSpeed <<endl;
                GPSOutputSelfData(fp_out);
                fp_out <<"End After GPSEdgeUpdate update" <<endl <<endl;
                /*
                cout <<"After GPSEdgeUpdate update, headspeed:"<<gpsinfo.headSpeed <<endl;
                gps.GPSOutputSelfData();
                cout <<"End After GPSEdgeUpdate update" <<endl <<endl;
                */
            }
            /* Set the gradient values */
            int lastIndex = gps.GA_robotsPos.size() - 1;
            SB_GPS_GradientAscent->setMaxValue(lastIndex);
            SB_GPS_GradientAscent->setValue(lastIndex);
            //Fill the information about gradient ascent
            slotGPSGradientAscent();
            gps.outputStream = NULL;
            cortex.outputStream = NULL;            

        }    
        fp_out.close(); 
        
        /* 
         * Visual Confidence Update
         */
        fp_out.open("DebugVisualConfidence.txt", ios::out);
        if (this->CB_VisualConfidence->isChecked())
        {
            if (!gotGPSInfo)
            {
                gpsinfo.visualConfidence = this->features.size();
            }
            
            gps.GPSVisualConfidenceUpdate(gpsinfo.visualConfidence);
            if (this->CB_GPS_COUT->isChecked())
            {
                fp_out <<"After GPSVisualConfidenceUpdate update" <<endl;
                GPSOutputSelfData(fp_out);
                fp_out <<"End After GPSVisualConfidenceUpdate update" <<endl <<endl;
                /*
                cout <<"After GPSVisualConfidenceUpdate update" <<endl;
                gps.GPSOutputSelfData();
                cout <<"End After GPSVisualConfidenceUpdate update" <<endl <<endl;
                */
            }
        }
        fp_out.close(); 
                
        /* 
         * Motion Update
         */
        fp_out.open("DebugMotion.txt", ios::out);
        if (this->CB_Motion_Update->isChecked())
        {   
            if (gpsinfo.motionFCounter == 0)
                cerr << "motion counter is 0!" <<endl;
            else
                gps.GPSMotionUpdate(gpsinfo.motionForward,gpsinfo.motionLeft,gpsinfo.motionTurn, (long)gpsinfo.motionPWM / gpsinfo.motionFCounter, this->CB_Motion_Update_2->isChecked());
            if (this->CB_GPS_COUT->isChecked())
            {
            
                fp_out <<"After GPSMotionUpdate update" <<endl;
                GPSOutputSelfData(fp_out);
                fp_out <<"End After GPSMotionUpdate update" <<endl <<endl;
                /*
                cout <<"After GPSMotionUpdate update" <<endl;
                gps.GPSOutputSelfData();
                cout <<"End After GPSMotionUpdate update" <<endl <<endl;
                */
            }
        }
        fp_out.close(); 
    }
    
    //Show top view if requested
    if (!RB_GPS_TV_None->isChecked())
        slotDPixmapRedraw();
        
    ostringstream str;
    /* reset every frame */
    str.str("");    
    this->GPSLogPosX->setText(str.str());
    str.str("");
    this->GPSLogPosY->setText(str.str());
    str.str("");
    this->GPSLogHeading->setText(str.str());
    str.str("");
    this->GPSLogMotionF->setText(str.str());
    str.str("");
    this->GPSLogMotionL->setText(str.str());
    str.str(""); 
    this->GPSLogMotionT->setText(str.str());
    str.str("");
    this->GPSLogMotionHS->setText(str.str());
    str.str("");
    this->GPSLogMotionPWM->setText(str.str());
    str.str("");                
    /* end reset every frame */
    if (gotGPSInfo){    
        str.str("");    
        str << gpsinfo.posX;
        this->GPSLogPosX->setText(str.str());
        str.str("");
        str << gpsinfo.posY;
        this->GPSLogPosY->setText(str.str());
        str.str("");
        str << gpsinfo.heading;
        this->GPSLogHeading->setText(str.str());
        str.str("");
        str << gpsinfo.motionForward;
        this->GPSLogMotionF->setText(str.str());
        str.str("");
        str << gpsinfo.motionLeft;
        this->GPSLogMotionL->setText(str.str());
        str.str("");
        str << gpsinfo.motionTurn;
        this->GPSLogMotionT->setText(str.str());
        str.str("");
        str << gpsinfo.headSpeed;
        this->GPSLogMotionHS->setText(str.str());
        str.str("");
        if (gpsinfo.motionFCounter == 0)
        {
            cerr << __func__ << " gpsinfo.motionFCounter is zero" <<endl;
            str << gpsinfo.motionPWM;
        }
        else
            str << gpsinfo.motionPWM / gpsinfo.motionFCounter;
        this->GPSLogMotionPWM->setText(str.str());
        str.str("");                
    }
    str.str("");
    str << gps.self().pos.x;
    this->GPSPosX->setText(str.str());
    str.str("");
    str << (int) gps.self().pos.x;
    this->GPSPosXText->setText(str.str());
    str.str("");
    str << gps.self().pos.y;
    this->GPSPosY->setText(str.str());
    str.str("");
    str << (int) gps.self().pos.y;
    this->GPSPosYText->setText(str.str());
    str.str("");
    str << gps.self().h;
    this->GPSHeading->setText(str.str());
    str.str("");
    str << (int) gps.self().h;
    this->GPSHeadingText->setText(str.str());
    str.str("");

    //Clear the gaussian display list
    this->LB_Gaussian->clear();
    QString qstr;
    for (int i = 0; i < gps.numGaussians ; ++i) {
        qstr = QString( "%1 - %2" ).arg( i ).arg( gps.r[i].weight );
        this->LB_Gaussian->insertItem(qstr);
        if (gps.r[i].weight > 1e-5)
            this->LB_Gaussian->setSelected(i,true);
    }
    
    slotGaussianDisplay();

} 

void FormSubvisionImpl::slotGPSGradientAscent(){
    if (DEBUG) {
        cerr << __func__ << endl;
    }
    stringstream ss1;
    /* Clear data */
    TXT_GPS_GA_Match->setText(ss1.str());
    TXT_GPS_GA_X->setText(ss1.str());
    TXT_GPS_GA_Y->setText(ss1.str());
    TXT_GPS_GA_Heading->setText(ss1.str());
    TXT_GPS_GA_LR->setText(ss1.str());
    
    if (gps.GA_robotsPos.size() == 0)
    {
        return;
    }
    
    int index = SB_GPS_GradientAscent->value();
    if (index < 0 || index >= (int)gps.GA_robotsPos.size()) {
        index = gps.GA_robotsPos.size() -1;
    }
    
    
    /* The match result */
    ss1 << gps.GA_match[index];
    TXT_GPS_GA_Match->setText(ss1.str());
    
    /* SET X Y H to the gradient result*/
    ss1.str("");
    ss1 << gps.GA_robotsPos[index](0,0);
    TXT_GPS_GA_X->setText(ss1.str());
    ss1.str("");
    ss1 << gps.GA_robotsPos[index](1,0);
    TXT_GPS_GA_Y->setText(ss1.str());
    ss1.str("");
    ss1 << RAD2DEG(gps.GA_robotsPos[index](2,0));
    TXT_GPS_GA_Heading->setText(ss1.str());
    
    /* Learning Rate */
    ss1.str("");
    ss1 << gps.GA_LR[index];
    TXT_GPS_GA_LR->setText(ss1.str());
    //Show top view if requested
    if (!RB_GPS_TV_None->isChecked())
        slotDPixmapRedraw();
}

void FormSubvisionImpl::slotGaussianDisplay()
{
    if (DEBUG) {
        cerr << __func__ << endl;
    }
    QPixmap temppixmap(this->FieldMapPM->width(),
                                                this->FieldMapPM->height());
    temppixmap.fill(Qt::green);                                          
    //bitBlt(this->FieldMapPM, 0, 0, temppixmap);
    
    //this->FieldMapPM->fill(Qt::green);
    QPainter p(&temppixmap);
    p.setPen(QPen(QColor("black"),1));   
    //cout << "h:"<<this->FieldMapPM->height() << endl;
    //cout << "w:"<<this->FieldMapPM->width() << endl;
    //Finding best resolution
    //based on drawing horizontal
    double widthNeeded = FIELD_LENGTH + 2 * GOAL_FRINGE_WIDTH;
    double heightNeeded = FIELD_WIDTH + 2 * SIDE_FRINGE_WIDTH;
    double widthImage = this->FieldMapPM->width();
    double heightImage = this->FieldMapPM->height();
    double convert;
    double offsetX, offsetY;
    if ( widthImage/widthNeeded < heightImage/heightNeeded)
    {
        convert = widthImage/widthNeeded;
    }
    else 
    {
        convert = heightImage/heightNeeded;
    }
    offsetX = GOAL_FRINGE_WIDTH * convert;
    offsetY = SIDE_FRINGE_WIDTH * convert;
    double pointX1,pointY1,pointX2,pointY2;
    
    //Draw beacon pink blue
    pointY1 = -BEACON_EDGE_LINE_OFFSET_X;
    pointX1 = BEACON_BOTTOM_LINE_OFFSET_Y-BEACON_RADIUS;
    pointY2 = pointY1/2;
    pointX2 = BEACON_BOTTOM_LINE_OFFSET_Y+BEACON_RADIUS;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetY;
    pointX1 = pointX1 * convert + offsetX;
    pointY2 = pointY2 * convert + offsetY;
    pointX2 = pointX2 * convert + offsetX;
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
    pointY1 = pointY1 * convert + offsetY;
    pointX1 = pointX1 * convert + offsetX;
    pointY2 = pointY2 * convert + offsetY;
    pointX2 = pointX2 * convert + offsetX;
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
    pointY1 = pointY1 * convert + offsetY;
    pointX1 = pointX1 * convert + offsetX;
    pointY2 = pointY2 * convert + offsetY;
    pointX2 = pointX2 * convert + offsetX;
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
    pointY1 = pointY1 * convert + offsetY;
    pointX1 = pointX1 * convert + offsetX;
    pointY2 = pointY2 * convert + offsetY;
    pointX2 = pointX2 * convert + offsetX;
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
    pointY1 = pointY1 * convert + offsetY;
    pointX1 = pointX1 * convert + offsetX;
    pointY2 = pointY2 * convert + offsetY;
    pointX2 = pointX2 * convert + offsetX;
    p.setPen(QPen(QColor("white"),2));
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    
    //Draw middle line
    pointY1 = 0;
    pointX1 = FIELD_LENGTH/2;
    pointY2 = FIELD_WIDTH;
    pointX2 = FIELD_LENGTH/2;
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
    //converted
    pointY1 = pointY1 * convert + offsetY;
    pointX1 = pointX1 * convert + offsetX;
    pointY2 = pointY2 * convert + offsetY;
    pointX2 = pointX2 * convert + offsetX;
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
    pointY1 = pointY1 * convert + offsetY;
    pointX1 = pointX1 * convert + offsetX;
    pointY2 = pointY2 * convert + offsetY;
    pointX2 = pointX2 * convert + offsetX;
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
    pointY1 = pointY1 * convert + offsetY;
    pointX1 = pointX1 * convert + offsetX;
    pointY2 = pointY2 * convert + offsetY;
    pointX2 = pointX2 * convert + offsetX;
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
    pointY1 = pointY1 * convert + offsetY;
    pointX1 = pointX1 * convert + offsetX;
    pointY2 = pointY2 * convert + offsetY;
    pointX2 = pointX2 * convert + offsetX;
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
    pointY1 = pointY1 * convert + offsetY;
    pointX1 = pointX1 * convert + offsetX;
    pointY2 = pointY2 * convert + offsetY;
    pointX2 = pointX2 * convert + offsetX;
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
    pointY1 = pointY1 * convert + offsetY;
    pointX1 = pointX1 * convert + offsetX;
    pointY2 = pointY2 * convert + offsetY;
    pointX2 = pointX2 * convert + offsetX;
    p.setPen(QPen(QColor("yellow"),2));
    //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
    p.drawRect((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
    
    //Draw dog
    int colordiff = 12;
    if (gps.numGaussians > 0) colordiff = 255/gps.numGaussians;
    for (int i = gps.numGaussians; i >= 0 ; --i) {
        if (!this->LB_Gaussian->isSelected(i))
        {
            continue;
        }
        p.setPen(QPen(QColor(CLIP(colordiff*i,255),CLIP(colordiff*i,255),255),1));
        double y = gps.r[i].mean.pos.x;
        double x = gps.r[i].mean.pos.y;
        //cout << "gps: y:" << x << " x:" << y << " degh:" << (gps.r[i].mean.h) << " radh:" << DEG2RAD(gps.r[i].mean.h) << " covh:"<< sqrt(gps.r[i].cov(2,2)) << endl;
        x = convert * x + offsetX;
        y = convert * y + offsetY;
        
        p.drawRect((int)x,(int)y,2,2);
        //cout << "x:" << x << " y:" << y << endl;
        
        //Draw heading
        int arrow = 10;
        double angle = DEG2RAD(gps.r[i].mean.h);
        double xend = sin(angle) * arrow + x;
        double yend = cos(angle) * arrow + y;
        //cout << "xend:" << xend << " yend:" << yend << endl;
        p.drawLine((int)x,(int)y,(int)xend,(int)yend);
        //p.drawArc( x-15, y, 30,30,16*( -90+(gps.r[i].mean.h-sqrt(gps.r[i].cov(2,2)))),-90+16*(gps.r[i].mean.h+sqrt(gps.r[i].cov(2,2))));//(gps.r[i].mean.h-sqrt(gps.r[i].cov(2,2)))*16, (gps.r[i].mean.h+sqrt(gps.r[i].cov(2,2)))*16 ); 
        
        //Draw variance
        pointY1 = gps.r[i].mean.pos.x-sqrt(gps.r[i].cov(1,1));
        pointX1 = gps.r[i].mean.pos.y-sqrt(gps.r[i].cov(0,0));
        pointY2 = gps.r[i].mean.pos.x+sqrt(gps.r[i].cov(1,1));
        pointX2 = gps.r[i].mean.pos.y+sqrt(gps.r[i].cov(0,0));
        //cout << "pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;    
        //converted
        pointY1 = pointY1 * convert + offsetY;
        pointX1 = pointX1 * convert + offsetX;
        pointY2 = pointY2 * convert + offsetY;
        pointX2 = pointX2 * convert + offsetX;
        //p.setPen(QPen(QColor("black"),1));
        //cout << "Converted pointX1:" << pointX1 << " pointY1:" << pointY1 << " pointX2:" << pointX2 << " pointY2:" << pointY2 << endl;
        p.drawEllipse((int)pointX1,(int)pointY1,(int)(pointX2-pointX1),(int)(pointY2-pointY1));
        
        //Draw the heading variance
        //p.drawRect( x-15, y-15, 30,30);
        p.setPen(QPen(QColor(CLIP(colordiff*i,255),CLIP(colordiff*i,255),255),1));
        int doubleArc = 30;
        p.drawArc( (int)x-doubleArc/2, (int)y-doubleArc/2, doubleArc,doubleArc, (int)(16*(-90+(gps.r[i].mean.h-sqrt(gps.r[i].cov(2,2))))), (int)(sqrt(gps.r[i].cov(2,2))*16*2));
        doubleArc -= 5;
        p.drawArc( (int)x-doubleArc/2, (int)y-doubleArc/2, doubleArc,doubleArc, (int)(16*(-90+(gps.r[i].mean.h-sqrt(gps.r[i].cov(2,2))))), (int)(sqrt(gps.r[i].cov(2,2))*16*2));
    }

    this->FieldMapPM->setPixmap(temppixmap);
}
/* not yet implemented, forgot what to put */
void FormSubvisionImpl::slotPopup() {
    if (DEBUG) {
        cerr << __func__ << endl;
    }
}


