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
#include <qfile.h>


#include "../../robot/share/Common.h"
#include "FormSubvisionImpl.h"
#include "../robolink/robolink.h"
#include "../../robot/vision/Profile.h"
/* Check flag checking the value stored and the vision confidence difference
 * errorMap will contain information of the error will fill add 1 if there is error
 */

bool FormSubvisionImpl::checkFlag(FlagObject &errorMap){
    bool isSame = true;
    //BALL
    if (this->vBall.cf > 0 && !CB_Flag_Ball->isChecked() || 
        this->vBall.cf <= 0 && CB_Flag_Ball->isChecked()
        )
    {
        CB_Flag_Ball->setPaletteBackgroundColor(QColor("red"));
        isSame = false;
        ++errorMap.flags[vobBall];
    }
    else
    {
        CB_Flag_Ball->setPaletteBackgroundColor(QColor(239,239,239));
    }
    
    //PinkBlue
    if (this->vBeacons[svPinkOnBlue].cf > 0 && !CB_Flag_PinkBlue->isChecked() || 
        this->vBeacons[svPinkOnBlue].cf <= 0 && CB_Flag_PinkBlue->isChecked()
        ) 
    {
        CB_Flag_PinkBlue->setPaletteBackgroundColor(QColor("red"));
        isSame = false;
        ++errorMap.flags[vobPinkBlueBeacon];
    }
    else
    {
        CB_Flag_PinkBlue->setPaletteBackgroundColor(QColor(239,239,239));
    }
    
    //BluePink
    if (this->vBeacons[svBlueOnPink].cf > 0 && !CB_Flag_BluePink->isChecked() || 
        this->vBeacons[svBlueOnPink].cf <= 0 && CB_Flag_BluePink->isChecked()
        ) 
    {
        CB_Flag_BluePink->setPaletteBackgroundColor(QColor("red"));
        isSame = false;
        ++errorMap.flags[vobBluePinkBeacon];
    }
    else
    {
        CB_Flag_BluePink->setPaletteBackgroundColor(QColor(239,239,239));
    }
            
    if (this->vBeacons[svPinkOnYellow].cf > 0 && !CB_Flag_PinkYellow->isChecked() || 
        this->vBeacons[svPinkOnYellow].cf <= 0 && CB_Flag_PinkYellow->isChecked()
        ) 
    {
        CB_Flag_PinkYellow->setPaletteBackgroundColor(QColor("red"));
        isSame = false;
        ++errorMap.flags[vobPinkYellowBeacon];
    }
    else
    {
        CB_Flag_PinkYellow->setPaletteBackgroundColor(QColor(239,239,239));
    }
    
    if (this->vBeacons[svYellowOnPink].cf > 0 && !CB_Flag_YellowPink->isChecked() || 
        this->vBeacons[svYellowOnPink].cf <= 0 && CB_Flag_YellowPink->isChecked()
        ) 
    {
        CB_Flag_YellowPink->setPaletteBackgroundColor(QColor("red"));
        isSame = false;
        ++errorMap.flags[vobYellowPinkBeacon];
    }
    else
    {
        CB_Flag_YellowPink->setPaletteBackgroundColor(QColor(239,239,239));
    }
    
    if (this->vGoals[svBlueGoal].cf > 0 && !CB_Flag_BlueGoal->isChecked() || 
        this->vGoals[svBlueGoal].cf <= 0 && CB_Flag_BlueGoal->isChecked()
        ) 
    {
        CB_Flag_BlueGoal->setPaletteBackgroundColor(QColor("red"));
        isSame = false;
        ++errorMap.flags[vobBlueGoal];
    }
    else
    {
        CB_Flag_BlueGoal->setPaletteBackgroundColor(QColor(239,239,239));
    }
    
    if (this->vGoals[svYellowGoal].cf > 0 && !CB_Flag_YellowGoal->isChecked() || 
        this->vGoals[svYellowGoal].cf <= 0 && CB_Flag_YellowGoal->isChecked()
        ) 
    {
        CB_Flag_YellowGoal->setPaletteBackgroundColor(QColor("red"));
        isSame = false;
        ++errorMap.flags[vobYellowGoal];
    }
    else
    {
        CB_Flag_YellowGoal->setPaletteBackgroundColor(QColor(239,239,239));
    }
    return isSame;
}

void FormSubvisionImpl::slotDisplayFlag()
{
    if (flagobjects.find(currentFrame) != flagobjects.end() && flagobjects[currentFrame].flags[vobBlueGoal] == 1)
        CB_Flag_BlueGoal->setChecked(true);
    else        
        CB_Flag_BlueGoal->setChecked(false);
        
    if (flagobjects.find(currentFrame) != flagobjects.end() && flagobjects[currentFrame].flags[vobYellowGoal] == 1)
        CB_Flag_YellowGoal->setChecked(true);
    else        
        CB_Flag_YellowGoal->setChecked(false);
        
    if (flagobjects.find(currentFrame) != flagobjects.end() && flagobjects[currentFrame].flags[vobBluePinkBeacon] == 1)
        CB_Flag_BluePink->setChecked(true);
    else        
        CB_Flag_BluePink->setChecked(false);
    
    if (flagobjects.find(currentFrame) != flagobjects.end() && flagobjects[currentFrame].flags[vobPinkBlueBeacon] == 1)
        CB_Flag_PinkBlue->setChecked(true);
    else        
        CB_Flag_PinkBlue->setChecked(false);
    
    if (flagobjects.find(currentFrame) != flagobjects.end() && flagobjects[currentFrame].flags[vobYellowPinkBeacon] == 1)
        CB_Flag_YellowPink->setChecked(true);
    else        
        CB_Flag_YellowPink->setChecked(false);
       
    if (flagobjects.find(currentFrame) != flagobjects.end() && flagobjects[currentFrame].flags[vobPinkYellowBeacon] == 1)
        CB_Flag_PinkYellow->setChecked(true);
    else        
        CB_Flag_PinkYellow->setChecked(false);
    
    if (flagobjects.find(currentFrame) != flagobjects.end() && flagobjects[currentFrame].flags[vobBall] == 1)
        CB_Flag_Ball->setChecked(true);
    else        
        CB_Flag_Ball->setChecked(false);
    FlagObject errorMap;
    checkFlag(errorMap);
}

void FormSubvisionImpl::slotModifyFlag()
{
    if (DEBUG) {
        cerr << __func__ << endl;
    }
    if (CB_Flag_BlueGoal->isChecked())
        flagobjects[currentFrame].flags[vobBlueGoal] = 1;
    else
        flagobjects[currentFrame].flags[vobBlueGoal] = 0;
    
    if (CB_Flag_YellowGoal->isChecked())
        flagobjects[currentFrame].flags[vobYellowGoal] = 1;
    else
        flagobjects[currentFrame].flags[vobYellowGoal] = 0;
    
    if (CB_Flag_BluePink->isChecked())
        flagobjects[currentFrame].flags[vobBluePinkBeacon] = 1;
    else
        flagobjects[currentFrame].flags[vobBluePinkBeacon] = 0;
    
    if (CB_Flag_PinkBlue->isChecked())
        flagobjects[currentFrame].flags[vobPinkBlueBeacon] = 1;
    else
        flagobjects[currentFrame].flags[vobPinkBlueBeacon] = 0;
    
    if (CB_Flag_YellowPink->isChecked())
        flagobjects[currentFrame].flags[vobYellowPinkBeacon] = 1;
    else
        flagobjects[currentFrame].flags[vobYellowPinkBeacon] = 0;
    
    if (CB_Flag_PinkYellow->isChecked())
        flagobjects[currentFrame].flags[vobPinkYellowBeacon] = 1;
    else
        flagobjects[currentFrame].flags[vobPinkYellowBeacon] = 0;
        
    if (CB_Flag_Ball->isChecked())
        flagobjects[currentFrame].flags[vobBall] = 1;
    else
        flagobjects[currentFrame].flags[vobBall] = 0;       
    FlagObject errorMap;
    checkFlag(errorMap);
}

void FormSubvisionImpl::slotSavingFlag()
{
    if (DEBUG) {
        cerr << __func__ << endl;
    }
    stringstream ss;
    ss << (((QFile*)this->input)->name()).data() << ".flags";
    ofstream fp_out;
    fp_out.open(ss.str().c_str(), ios::out);
    for (map<int, FlagObject>::iterator i=flagobjects.begin(); i != flagobjects.end(); ++i)
    {
        fp_out << i->first;
        WritingAFlag(fp_out,i->second);
        fp_out << endl;
    }
    fp_out.close();
}

void FormSubvisionImpl::WritingAFlag(ostream &fp_out, const FlagObject& fo)
{
    for (int x = 0; x < VOB_COUNT; ++x)
    {
        fp_out << "\t" << fo.flags[x];
    }    
}

void FormSubvisionImpl::slotLoadingFlag()
{
    if (DEBUG) {
        cerr << __func__ << endl;
    }
    if (this->input == NULL) 
        return;
    stringstream ss;
    ss << (((QFile*)this->input)->name()).data() << ".flags";
    flagobjects.clear();
    ifstream fp_in;
    fp_in.open(ss.str().c_str(), ifstream::in);
    if (! fp_in.good())
        cerr << __func__ << " File is not exist" << endl;
    FlagObject fo;
    int framecounter;
    while(fp_in.good()){
        fp_in >> framecounter;
        //cout << "line:" << framecounter;
        for (int x = 0; x < VOB_COUNT; ++x)
        {
            fp_in >> fo.flags[x];
            //cout << "\t" << fo.flags[x];
        }
        flagobjects[framecounter] = fo;
        //cout << endl;
    }
    fp_in.close();
    slotRedraw();
}

void FormSubvisionImpl::slotCopyFlagFromObj()
{
        if (this->vBall.cf > 0) {
            CB_Flag_Ball->setChecked(true);
        }        
        if (this->vBeacons[svPinkOnBlue].cf > 0) {
            CB_Flag_PinkBlue->setChecked(true);
        }        
        if (this->vBeacons[svBlueOnPink].cf > 0) {
            CB_Flag_BluePink->setChecked(true);
        }        
        if (this->vBeacons[svPinkOnYellow].cf > 0) {
            CB_Flag_PinkYellow->setChecked(true);
        }        
        if (this->vBeacons[svYellowOnPink].cf > 0) { 
            CB_Flag_YellowPink->setChecked(true);
        }        
        if (this->vGoals[svBlueGoal].cf > 0) {
            CB_Flag_BlueGoal->setChecked(true);
        }        
        if (this->vGoals[svYellowGoal].cf > 0) {
            CB_Flag_YellowGoal->setChecked(true);
        }   
        slotModifyFlag();           
}
