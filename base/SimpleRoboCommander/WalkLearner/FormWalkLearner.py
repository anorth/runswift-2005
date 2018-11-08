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

# Form implementation generated from reading ui file '/home/kcph007/trunk/base7/work/SimpleRoboCommander/WalkLearner/FormWalkLearner.ui'
#
# Created: Sat May 8 17:41:59 2004
#      by: The PyQt User Interface Compiler (pyuic) 3.11
#
# WARNING! All changes made in this file will be lost!


import sys
from qt import *

class FormWalkLearner(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("FormWalkLearner")


        FormWalkLearnerLayout = QGridLayout(self,1,1,11,6,"FormWalkLearnerLayout")

        self.groupBox1 = QGroupBox(self,"groupBox1")
        self.groupBox1.setColumnLayout(0,Qt.Vertical)
        self.groupBox1.layout().setSpacing(6)
        self.groupBox1.layout().setMargin(11)
        groupBox1Layout = QGridLayout(self.groupBox1.layout())
        groupBox1Layout.setAlignment(Qt.AlignTop)

        self.btNewRobot = QPushButton(self.groupBox1,"btNewRobot")

        groupBox1Layout.addMultiCellWidget(self.btNewRobot,0,0,2,3)

        self.btStartAll = QPushButton(self.groupBox1,"btStartAll")

        groupBox1Layout.addWidget(self.btStartAll,0,4)

        self.btQueryAll = QPushButton(self.groupBox1,"btQueryAll")

        groupBox1Layout.addWidget(self.btQueryAll,1,0)

        self.btStartStopLearning = QPushButton(self.groupBox1,"btStartStopLearning")

        groupBox1Layout.addMultiCellWidget(self.btStartStopLearning,1,1,1,2)

        self.textLabel4 = QLabel(self.groupBox1,"textLabel4")

        groupBox1Layout.addWidget(self.textLabel4,0,0)

        self.ledNumRobot = QLineEdit(self.groupBox1,"ledNumRobot")

        groupBox1Layout.addWidget(self.ledNumRobot,0,1)

        self.btStopAll = QPushButton(self.groupBox1,"btStopAll")

        groupBox1Layout.addMultiCellWidget(self.btStopAll,1,1,3,4)

        FormWalkLearnerLayout.addWidget(self.groupBox1,0,0)

        self.groupBox9 = QGroupBox(self,"groupBox9")
        self.groupBox9.setColumnLayout(0,Qt.Vertical)
        self.groupBox9.layout().setSpacing(6)
        self.groupBox9.layout().setMargin(11)
        groupBox9Layout = QGridLayout(self.groupBox9.layout())
        groupBox9Layout.setAlignment(Qt.AlignTop)

        self.textLabel1 = QLabel(self.groupBox9,"textLabel1")

        groupBox9Layout.addWidget(self.textLabel1,0,0)

        self.ledBestValue = QLineEdit(self.groupBox9,"ledBestValue")
        self.ledBestValue.setEnabled(1)

        groupBox9Layout.addWidget(self.ledBestValue,0,3)

        self.textLabel2 = QLabel(self.groupBox9,"textLabel2")

        groupBox9Layout.addWidget(self.textLabel2,0,2)

        self.ledBestParams = QLineEdit(self.groupBox9,"ledBestParams")
        self.ledBestParams.setEnabled(1)
        self.ledBestParams.setMinimumSize(QSize(310,0))

        groupBox9Layout.addWidget(self.ledBestParams,0,1)

        self.ledRunTimes = QLineEdit(self.groupBox9,"ledRunTimes")

        groupBox9Layout.addMultiCellWidget(self.ledRunTimes,1,1,0,3)

        FormWalkLearnerLayout.addWidget(self.groupBox9,2,0)

        self.groupBox3 = QGroupBox(self,"groupBox3")
        self.groupBox3.setColumnLayout(0,Qt.Vertical)
        self.groupBox3.layout().setSpacing(6)
        self.groupBox3.layout().setMargin(11)
        groupBox3Layout = QGridLayout(self.groupBox3.layout())
        groupBox3Layout.setAlignment(Qt.AlignTop)

        self.tabRobots = QTabWidget(self.groupBox3,"tabRobots")

        self.tab = QWidget(self.tabRobots,"tab")
        self.tabRobots.insertTab(self.tab,QString(""))

        groupBox3Layout.addWidget(self.tabRobots,0,0)

        FormWalkLearnerLayout.addWidget(self.groupBox3,1,0)

        self.languageChange()

        self.resize(QSize(592,474).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.ledNumRobot,SIGNAL("returnPressed()"),self.slotNumRobotChange)
        self.connect(self.btNewRobot,SIGNAL("clicked()"),self.slotAddNewRobot)
        self.connect(self.btQueryAll,SIGNAL("pressed()"),self.slotQueryAllRobots)
        self.connect(self.btStartAll,SIGNAL("pressed()"),self.slotStartAllRobots)
        self.connect(self.btStopAll,SIGNAL("clicked()"),self.slotStopAllRobots)
        self.connect(self.btStartStopLearning,SIGNAL("clicked()"),self.slotStartStopLearning)


    def languageChange(self):
        self.setCaption(self.__tr("Walking Learner Controller"))
        self.groupBox1.setTitle(self.__tr("Control"))
        self.btNewRobot.setText(self.__tr("&New Robot"))
        self.btStartAll.setText(self.__tr("&Start All Robots"))
        self.btQueryAll.setText(self.__tr("&Query All Robots"))
        self.btStartStopLearning.setText(self.__tr("Start Learning"))
        self.textLabel4.setText(self.__tr("Number of robots"))
        self.btStopAll.setText(self.__tr("Sto&p All Robots"))
        self.groupBox9.setTitle(self.__tr("Learning algorithm"))
        self.textLabel1.setText(self.__tr("Best parameters"))
        self.textLabel2.setText(self.__tr("Value"))
        self.groupBox3.setTitle(self.__tr("Robo Info"))
        self.tabRobots.changeTab(self.tab,self.__tr("Robot 1"))


    def slotNumRobotChange(self):
        print "FormWalkLearner.slotNumRobotChange(): Not implemented yet"

    def slotAddNewRobot(self):
        print "FormWalkLearner.slotAddNewRobot(): Not implemented yet"

    def slotQueryAllRobots(self):
        print "FormWalkLearner.slotQueryAllRobots(): Not implemented yet"

    def slotStartAllRobots(self):
        print "FormWalkLearner.slotStartAllRobots(): Not implemented yet"

    def slotStopAllRobots(self):
        print "FormWalkLearner.slotStopAllRobots(): Not implemented yet"

    def slotStartStopLearning(self):
        print "FormWalkLearner.slotStartStopLearning(): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("FormWalkLearner",s,c)

if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    w = FormWalkLearner()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()
