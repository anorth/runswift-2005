##  
##     Copyright 2004 The University of New South Wales (UNSW) and National  
##     ICT Australia (NICTA).
##  
##     This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
##     redistribute it and/or modify it under the terms of the GNU General  
##     Public License as published by the Free Software Foundation; either  
##     version 2 of the License, or (at your option) any later version as  
##     modified below.  As the original licensors, we add the following  
##     conditions to that license:
##  
##     In paragraph 2.b), the phrase "distribute or publish" should be  
##     interpreted to include entry into a competition, and hence the source  
##     of any derived work entered into a competition must be made available  
##     to all parties involved in that competition under the terms of this  
##     license.
##  
##     In addition, if the authors of a derived work publish any conference  
##     proceedings, journal articles or other academic papers describing that  
##     derived work, then appropriate academic citations to the original work  
##     must be included in that publication.
##  
##     This rUNSWift source is distributed in the hope that it will be useful,  
##     but WITHOUT ANY WARRANTY; without even the implied warranty of  
##     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
##     General Public License for more details.
##  
##     You should have received a copy of the GNU General Public License along  
##     with this source code; if not, write to the Free Software Foundation,  
##     Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##  
##  
##  
##   Last modification background information
##   $Id$
##  
##   Copyright (c) 2004 UNSW
##   All Rights Reserved.
##  
##  



# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/home/kcph007/trunk/base7/work/SimpleRoboCommander/WalkLearner/WdgRobotInfo.ui'
#
# Created: Sat May 8 17:41:59 2004
#      by: The PyQt User Interface Compiler (pyuic) 3.11
#
# WARNING! All changes made in this file will be lost!


import sys
from qt import *

class WdgRobotInfo(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("WdgRobotInfo")


        WdgRobotInfoLayout = QGridLayout(self,1,1,11,6,"WdgRobotInfoLayout")

        self.textLabel3_2_2 = QLabel(self,"textLabel3_2_2")

        WdgRobotInfoLayout.addMultiCellWidget(self.textLabel3_2_2,1,1,4,6)

        self.ledBack = QLineEdit(self,"ledBack")

        WdgRobotInfoLayout.addWidget(self.ledBack,1,7)

        self.textLabel1 = QLabel(self,"textLabel1")

        WdgRobotInfoLayout.addMultiCellWidget(self.textLabel1,2,2,0,1)

        self.ledLearningState = QLineEdit(self,"ledLearningState")

        WdgRobotInfoLayout.addMultiCellWidget(self.ledLearningState,1,1,1,3)

        self.lcdRobotID = QLCDNumber(self,"lcdRobotID")

        WdgRobotInfoLayout.addMultiCellWidget(self.lcdRobotID,0,0,1,2)
        spacer5 = QSpacerItem(52,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        WdgRobotInfoLayout.addItem(spacer5,0,3)

        self.textLabel3_3 = QLabel(self,"textLabel3_3")

        WdgRobotInfoLayout.addMultiCellWidget(self.textLabel3_3,0,0,4,6)

        self.textLabel1_2 = QLabel(self,"textLabel1_2")

        WdgRobotInfoLayout.addWidget(self.textLabel1_2,1,0)

        self.textLabel1_2_2 = QLabel(self,"textLabel1_2_2")

        WdgRobotInfoLayout.addWidget(self.textLabel1_2_2,0,0)

        self.ledParams = QLineEdit(self,"ledParams")

        WdgRobotInfoLayout.addMultiCellWidget(self.ledParams,2,2,2,7)

        self.btLearnThis = QPushButton(self,"btLearnThis")

        WdgRobotInfoLayout.addMultiCellWidget(self.btLearnThis,3,3,0,2)

        self.btStartStop = QPushButton(self,"btStartStop")

        WdgRobotInfoLayout.addWidget(self.btStartStop,3,5)

        self.btBeReady = QPushButton(self,"btBeReady")

        WdgRobotInfoLayout.addMultiCellWidget(self.btBeReady,3,3,6,7)

        self.btSetDefault = QPushButton(self,"btSetDefault")

        WdgRobotInfoLayout.addMultiCellWidget(self.btSetDefault,3,3,3,4)

        self.btSkipThisRun = QPushButton(self,"btSkipThisRun")

        WdgRobotInfoLayout.addWidget(self.btSkipThisRun,4,0)

        self.ledForth = QLineEdit(self,"ledForth")

        WdgRobotInfoLayout.addWidget(self.ledForth,0,7)

        self.btRunTooSlow = QPushButton(self,"btRunTooSlow")

        WdgRobotInfoLayout.addMultiCellWidget(self.btRunTooSlow,4,4,1,3)

        self.btDogCrash = QPushButton(self,"btDogCrash")

        WdgRobotInfoLayout.addMultiCellWidget(self.btDogCrash,4,4,4,5)
        spacer2 = QSpacerItem(90,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        WdgRobotInfoLayout.addMultiCell(spacer2,4,4,6,7)

        self.languageChange()

        self.resize(QSize(465,241).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.btLearnThis,SIGNAL("clicked()"),self.slotLearnThis)
        self.connect(self.btStartStop,SIGNAL("clicked()"),self.slotStartStop)
        self.connect(self.btBeReady,SIGNAL("clicked()"),self.slotBeReady)
        self.connect(self.btSetDefault,SIGNAL("clicked()"),self.slotSetDefault)
        self.connect(self.btSkipThisRun,SIGNAL("clicked()"),self.slotSkipThisRun)
        self.connect(self.btRunTooSlow,SIGNAL("clicked()"),self.slotRunTooSlow)
        self.connect(self.btDogCrash,SIGNAL("clicked()"),self.slotDogCrash)


    def languageChange(self):
        self.setCaption(self.__tr("Robot "))
        self.textLabel3_2_2.setText(self.__tr("Last walking time (leg 2)"))
        self.textLabel1.setText(self.__tr("Manual Learning"))
        self.textLabel3_3.setText(self.__tr("Last walking time (leg 1)"))
        self.textLabel1_2.setText(self.__tr("Learning state"))
        self.textLabel1_2_2.setText(self.__tr("Robot Number"))
        self.ledParams.setText(self.__tr("60  90 120  55 15 -55 5  30 20 50 30"))
        self.btLearnThis.setText(self.__tr("&Learn there parameters"))
        self.btStartStop.setText(self.__tr("&GO !"))
        self.btBeReady.setText(self.__tr("Be &Ready !"))
        self.btSetDefault.setText(self.__tr("Set &Default"))
        self.btSkipThisRun.setText(self.__tr("S&kip this run"))
        self.btRunTooSlow.setText(self.__tr("Run too slow"))
        self.btDogCrash.setText(self.__tr("This dog crashed"))


    def slotSkipThisRun(self):
        print "WdgRobotInfo.slotSkipThisRun(): Not implemented yet"

    def slotStartStop(self):
        print "WdgRobotInfo.slotStartStop(): Not implemented yet"

    def slotBeReady(self):
        print "WdgRobotInfo.slotBeReady(): Not implemented yet"

    def slotSetDefault(self):
        print "WdgRobotInfo.slotSetDefault(): Not implemented yet"

    def slotLearnThis(self):
        print "WdgRobotInfo.slotLearnThis(): Not implemented yet"

    def slotRunTooSlow(self):
        print "WdgRobotInfo.slotRunTooSlow(): Not implemented yet"

    def slotDogCrash(self):
        print "WdgRobotInfo.slotDogCrash(): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("WdgRobotInfo",s,c)

if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    w = WdgRobotInfo()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()
