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

# Form implementation generated from reading ui file '/home/kcph007/trunk/base7/work/SimpleRoboCommander/JointDebugger/FormJointDebugger.ui'
#
# Created: Sat May 8 17:41:59 2004
#      by: The PyQt User Interface Compiler (pyuic) 3.11
#
# WARNING! All changes made in this file will be lost!


import sys
from qt import *

class FormJointDebugger(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("FormJointDebugger")


        FormJointDebuggerLayout = QGridLayout(self,1,1,11,6,"FormJointDebuggerLayout")

        self.groupBox1 = QGroupBox(self,"groupBox1")
        self.groupBox1.setColumnLayout(0,Qt.Vertical)
        self.groupBox1.layout().setSpacing(6)
        self.groupBox1.layout().setMargin(11)
        groupBox1Layout = QGridLayout(self.groupBox1.layout())
        groupBox1Layout.setAlignment(Qt.AlignTop)

        self.textLabel1_2 = QLabel(self.groupBox1,"textLabel1_2")

        groupBox1Layout.addWidget(self.textLabel1_2,0,0)

        self.lviewFrontRight = QListView(self.groupBox1,"lviewFrontRight")
        self.lviewFrontRight.addColumn(self.__tr("Joint"))
        self.lviewFrontRight.addColumn(self.__tr("Shoulder"))
        self.lviewFrontRight.addColumn(self.__tr("Knee"))

        groupBox1Layout.addWidget(self.lviewFrontRight,3,1)

        self.lviewHead = QListView(self.groupBox1,"lviewHead")
        self.lviewHead.addColumn(self.__tr("Tilt"))
        self.lviewHead.addColumn(self.__tr("Pan"))
        self.lviewHead.addColumn(self.__tr("Crane"))

        groupBox1Layout.addWidget(self.lviewHead,1,0)

        self.lviewFrontLeft = QListView(self.groupBox1,"lviewFrontLeft")
        self.lviewFrontLeft.addColumn(self.__tr("Joint"))
        self.lviewFrontLeft.addColumn(self.__tr("Shoulder"))
        self.lviewFrontLeft.addColumn(self.__tr("Knee"))

        groupBox1Layout.addWidget(self.lviewFrontLeft,3,0)

        self.textLabel1_2_2_2 = QLabel(self.groupBox1,"textLabel1_2_2_2")

        groupBox1Layout.addWidget(self.textLabel1_2_2_2,2,1)

        self.textLabel1_2_2 = QLabel(self.groupBox1,"textLabel1_2_2")

        groupBox1Layout.addWidget(self.textLabel1_2_2,2,0)

        self.textLabel1_2_2_3 = QLabel(self.groupBox1,"textLabel1_2_2_3")

        groupBox1Layout.addWidget(self.textLabel1_2_2_3,4,0)

        self.lviewRearLeft = QListView(self.groupBox1,"lviewRearLeft")
        self.lviewRearLeft.addColumn(self.__tr("Joint"))
        self.lviewRearLeft.addColumn(self.__tr("Shoulder"))
        self.lviewRearLeft.addColumn(self.__tr("Knee"))

        groupBox1Layout.addWidget(self.lviewRearLeft,5,0)

        self.lviewRearRight = QListView(self.groupBox1,"lviewRearRight")
        self.lviewRearRight.addColumn(self.__tr("Joint"))
        self.lviewRearRight.addColumn(self.__tr("Shoulder"))
        self.lviewRearRight.addColumn(self.__tr("Knee"))

        groupBox1Layout.addWidget(self.lviewRearRight,5,1)

        self.textLabel1_2_2_4 = QLabel(self.groupBox1,"textLabel1_2_2_4")

        groupBox1Layout.addWidget(self.textLabel1_2_2_4,4,1)

        self.groupBox4 = QGroupBox(self.groupBox1,"groupBox4")
        self.groupBox4.setColumnLayout(0,Qt.Vertical)
        self.groupBox4.layout().setSpacing(6)
        self.groupBox4.layout().setMargin(11)
        groupBox4Layout = QGridLayout(self.groupBox4.layout())
        groupBox4Layout.setAlignment(Qt.AlignTop)

        self.btStartStopRecord = QPushButton(self.groupBox4,"btStartStopRecord")

        groupBox4Layout.addWidget(self.btStartStopRecord,0,0)
        spacer3 = QSpacerItem(31,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        groupBox4Layout.addItem(spacer3,0,1)
        spacer4 = QSpacerItem(31,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        groupBox4Layout.addItem(spacer4,1,1)

        self.btSaveToFile = QPushButton(self.groupBox4,"btSaveToFile")

        groupBox4Layout.addWidget(self.btSaveToFile,1,0)

        self.btClearRecorded = QPushButton(self.groupBox4,"btClearRecorded")

        groupBox4Layout.addWidget(self.btClearRecorded,2,0)

        groupBox1Layout.addMultiCellWidget(self.groupBox4,0,1,1,1)

        FormJointDebuggerLayout.addMultiCellWidget(self.groupBox1,1,1,0,1)

        self.spbNumReceived = QSpinBox(self,"spbNumReceived")
        self.spbNumReceived.setMaxValue(999999)

        FormJointDebuggerLayout.addWidget(self.spbNumReceived,0,1)

        self.textLabel1 = QLabel(self,"textLabel1")

        FormJointDebuggerLayout.addWidget(self.textLabel1,0,0)

        self.languageChange()

        self.resize(QSize(472,466).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.btStartStopRecord,SIGNAL("pressed()"),self.slotStartStopRecording)
        self.connect(self.btSaveToFile,SIGNAL("pressed()"),self.slotSaveToFile)
        self.connect(self.btClearRecorded,SIGNAL("pressed()"),self.slotClearRecorded)


    def languageChange(self):
        self.setCaption(self.__tr("Joint Debugger"))
        self.groupBox1.setTitle(self.__tr("Current Value"))
        self.textLabel1_2.setText(self.__tr("Head"))
        self.lviewFrontRight.header().setLabel(0,self.__tr("Joint"))
        self.lviewFrontRight.header().setLabel(1,self.__tr("Shoulder"))
        self.lviewFrontRight.header().setLabel(2,self.__tr("Knee"))
        self.lviewHead.header().setLabel(0,self.__tr("Tilt"))
        self.lviewHead.header().setLabel(1,self.__tr("Pan"))
        self.lviewHead.header().setLabel(2,self.__tr("Crane"))
        self.lviewFrontLeft.header().setLabel(0,self.__tr("Joint"))
        self.lviewFrontLeft.header().setLabel(1,self.__tr("Shoulder"))
        self.lviewFrontLeft.header().setLabel(2,self.__tr("Knee"))
        self.textLabel1_2_2_2.setText(self.__tr("Front Right Leg"))
        self.textLabel1_2_2.setText(self.__tr("Front Left Leg"))
        self.textLabel1_2_2_3.setText(self.__tr("Rear Left Leg"))
        self.lviewRearLeft.header().setLabel(0,self.__tr("Joint"))
        self.lviewRearLeft.header().setLabel(1,self.__tr("Shoulder"))
        self.lviewRearLeft.header().setLabel(2,self.__tr("Knee"))
        self.lviewRearRight.header().setLabel(0,self.__tr("Joint"))
        self.lviewRearRight.header().setLabel(1,self.__tr("Shoulder"))
        self.lviewRearRight.header().setLabel(2,self.__tr("Knee"))
        self.textLabel1_2_2_4.setText(self.__tr("Rear Right Leg"))
        self.groupBox4.setTitle(self.__tr("Logging"))
        self.btStartStopRecord.setText(self.__tr("Start &Recording"))
        self.btStartStopRecord.setAccel(self.__tr("Alt+R"))
        self.btSaveToFile.setText(self.__tr("&Save to file"))
        self.btSaveToFile.setAccel(self.__tr("Alt+S"))
        self.btClearRecorded.setText(self.__tr("&Clear Recorded"))
        self.btClearRecorded.setAccel(self.__tr("Alt+C"))
        self.textLabel1.setText(self.__tr("Number of command received"))


    def slotStartStopRecording(self):
        print "FormJointDebugger.slotStartStopRecording(): Not implemented yet"

    def slotSaveToFile(self):
        print "FormJointDebugger.slotSaveToFile(): Not implemented yet"

    def slotClearRecorded(self):
        print "FormJointDebugger.slotClearRecorded(): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("FormJointDebugger",s,c)

if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    w = FormJointDebugger()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()
