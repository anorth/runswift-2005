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
#include <qpainter.h>
#include <qpixmap.h>

#include <qlineedit.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qlistbox.h>
#include <qframe.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qfile.h>

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

#include "../../robot/share/Common.h"
#include "FormSubvisionImpl.h"
#include "hough.h"

void FormSubvisionImpl::slotDisplayHough()
{
    if (DEBUG)
        cerr << __func__ <<endl;
    HoughLine *hl = new HoughLine();
    double points[this->cortex.subvision.getFeatures().size() *2];
    int index = 0;
    vector<VisualFeature>::iterator itr = this->cortex.subvision.getFeatures().begin();
    for (; itr != this->cortex.subvision.getFeatures().end(); ++itr) {
        if (itr->type == VF_FIELDLINE) {
            if (index == 0)
                cout << __func__ << " itr x:" << itr->x << " y:" << itr->y << endl;
            points[index*2] = itr->x;
            points[index*2 + 1] = itr->y;
            ++index;
        }
     }
     int rt[10*2];
     int numlines = hl->DetectLines(points,index,rt,10);
     delete hl;
     if (this->pixmap == NULL)
        return;
        
     QPainter p(this->pixmap);
     p.setPen(QPen(QColor("white"),1));
     cout << __func__ << " numlines:" << numlines << endl;
     for (int x = 0; x < numlines ; ++x)
     {
        double xPos[2],yPos[2];
        int index = 0;
        xPos[index] = TOPLEFT2CENTER_X(0.0);
        yPos[index] = TOPLEFT2CENTER_Y(0.0);
        
        double sinT = sin(DEG2RAD(rt[x*2+1]*1.0));
        double cosT = cos(DEG2RAD(rt[x*2+1]*1.0));
        //check if it touch left cplane
        
        if (ABS(sinT) > EPSILON)
        { 
            yPos[index] = (rt[x*2] - xPos[index] * cosT ) / sinT;
            if (yPos[index] > TOPLEFT2CENTER_Y(0.0) && yPos[index] < TOPLEFT2CENTER_Y(CPLANE_HEIGHT*1.0))
                ++index;
        }
        //check if it touch top cplane
        if (index == 0)
        {   
            if (ABS(cosT) < EPSILON)
                cerr << __func__ << " cosT is less than EPSILON " << endl;
            yPos[index] = TOPLEFT2CENTER_Y(0.0);
            xPos[index] = (rt[x*2] - yPos[index] * sinT) / cosT;
            ++index;
        }
        //Check if it touch right CPLANE
        if (ABS(sinT) > EPSILON)
        { 
            xPos[index] = TOPLEFT2CENTER_X(CPLANE_WIDTH*1.0);
            yPos[index] = (rt[x*2] - xPos[index] * cosT ) / sinT;
            if (yPos[index] > TOPLEFT2CENTER_Y(0.0) && yPos[index] < TOPLEFT2CENTER_Y(CPLANE_HEIGHT*1.0))
                ++index;
        }
        //check if it touch top cplane
        if (index == 1)
        {   
            if (ABS(cosT) < EPSILON)
                cerr << __func__ << " cosT is less than EPSILON " << endl;
            yPos[index] = TOPLEFT2CENTER_Y(CPLANE_HEIGHT*1.0);
            xPos[index] = (rt[x*2] - yPos[index] * sinT) / cosT;
            ++index;
        }
        p.drawLine((int)xPos[0],(int)yPos[0],(int)xPos[1],(int)yPos[1]);
        cout << __func__ << " x1:" << xPos[0] << " y1:" << yPos[0] << " x2:" << xPos[1] << " y2:" << yPos[1] << endl;
     }
     
     bitBlt(this->pixmapImage, 0, 0, this->pixmap);
     cout << __func__ << " End." << endl;
}
