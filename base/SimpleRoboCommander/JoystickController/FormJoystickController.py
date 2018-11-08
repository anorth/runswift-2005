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

# Form implementation generated from reading ui file '/home/kcph007/trunk/base7/work/SimpleRoboCommander/JoystickController/FormJoystickController.ui'
#
# Created: Wed Sep 1 17:09:37 2004
#      by: The PyQt User Interface Compiler (pyuic) 3.11
#
# WARNING! All changes made in this file will be lost!


import sys
from qt import *
from qttable import QTable

class FormJoystickController(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("FormJoystickController")


        FormJoystickControllerLayout = QGridLayout(self,1,1,11,6,"FormJoystickControllerLayout")

        self.groupBox12 = QGroupBox(self,"groupBox12")

        self.ledVertAxis = QLineEdit(self.groupBox12,"ledVertAxis")
        self.ledVertAxis.setGeometry(QRect(210,25,207,23))

        self.ledHoriAxis = QLineEdit(self.groupBox12,"ledHoriAxis")
        self.ledHoriAxis.setGeometry(QRect(210,58,207,23))

        self.textLabel1 = QLabel(self.groupBox12,"textLabel1")
        self.textLabel1.setGeometry(QRect(112,22,92,30))

        self.ledRotaAxis = QLineEdit(self.groupBox12,"ledRotaAxis")
        self.ledRotaAxis.setGeometry(QRect(210,87,207,23))

        self.ledButtonClicked = QLineEdit(self.groupBox12,"ledButtonClicked")
        self.ledButtonClicked.setGeometry(QRect(210,116,207,23))

        self.textLabel1_3 = QLabel(self.groupBox12,"textLabel1_3")
        self.textLabel1_3.setGeometry(QRect(112,87,92,23))

        self.btStart = QPushButton(self.groupBox12,"btStart")
        self.btStart.setGeometry(QRect(11,22,95,30))

        self.textLabel1_2 = QLabel(self.groupBox12,"textLabel1_2")
        self.textLabel1_2.setGeometry(QRect(112,58,92,23))

        self.btCalibrate = QPushButton(self.groupBox12,"btCalibrate")
        self.btCalibrate.setGeometry(QRect(11,69,95,30))

        self.textLabel1_3_2 = QLabel(self.groupBox12,"textLabel1_3_2")
        self.textLabel1_3_2.setGeometry(QRect(112,116,92,23))

        self.cbxCalibration = QComboBox(0,self.groupBox12,"cbxCalibration")
        self.cbxCalibration.setGeometry(QRect(10,110,92,27))

        FormJoystickControllerLayout.addWidget(self.groupBox12,0,0)

        self.groupBox2 = QGroupBox(self,"groupBox2")
        self.groupBox2.setColumnLayout(0,Qt.Vertical)
        self.groupBox2.layout().setSpacing(6)
        self.groupBox2.layout().setMargin(11)
        groupBox2Layout = QGridLayout(self.groupBox2.layout())
        groupBox2Layout.setAlignment(Qt.AlignTop)

        self.ledCommand = QLineEdit(self.groupBox2,"ledCommand")

        groupBox2Layout.addMultiCellWidget(self.ledCommand,1,1,1,2)

        self.textLabel1_4 = QLabel(self.groupBox2,"textLabel1_4")

        groupBox2Layout.addMultiCellWidget(self.textLabel1_4,0,0,1,2)

        self.textLabel1_4_2 = QLabel(self.groupBox2,"textLabel1_4_2")

        groupBox2Layout.addWidget(self.textLabel1_4_2,0,0)

        self.tblConfig = QTable(self.groupBox2,"tblConfig")
        self.tblConfig.setNumCols(self.tblConfig.numCols() + 1)
        self.tblConfig.horizontalHeader().setLabel(self.tblConfig.numCols() - 1,self.__tr("Action"))
        self.tblConfig.setNumCols(self.tblConfig.numCols() + 1)
        self.tblConfig.horizontalHeader().setLabel(self.tblConfig.numCols() - 1,self.__tr("Input"))
        self.tblConfig.setNumCols(self.tblConfig.numCols() + 1)
        self.tblConfig.horizontalHeader().setLabel(self.tblConfig.numCols() - 1,self.__tr("Command"))
        self.tblConfig.setNumCols(self.tblConfig.numCols() + 1)
        self.tblConfig.horizontalHeader().setLabel(self.tblConfig.numCols() - 1,self.__tr("Description"))
        self.tblConfig.setNumRows(0)
        self.tblConfig.setNumCols(4)

        groupBox2Layout.addMultiCellWidget(self.tblConfig,1,4,0,0)

        self.btSave = QPushButton(self.groupBox2,"btSave")

        groupBox2Layout.addMultiCellWidget(self.btSave,4,4,1,2)

        self.btNew = QPushButton(self.groupBox2,"btNew")

        groupBox2Layout.addMultiCellWidget(self.btNew,3,3,1,2)

        self.textLabel1_5 = QLabel(self.groupBox2,"textLabel1_5")

        groupBox2Layout.addWidget(self.textLabel1_5,2,1)

        self.spbDogNumber = QSpinBox(self.groupBox2,"spbDogNumber")

        groupBox2Layout.addWidget(self.spbDogNumber,2,2)

        FormJoystickControllerLayout.addWidget(self.groupBox2,1,0)

        self.languageChange()

        self.resize(QSize(450,367).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.btStart,SIGNAL("clicked()"),self.slotStartStopJoystick)
        self.connect(self.btNew,SIGNAL("clicked()"),self.slotNewAction)
        self.connect(self.btSave,SIGNAL("clicked()"),self.slotSaveConfiguration)


    def languageChange(self):
        self.setCaption(self.__tr("Joystick Controller"))
        self.groupBox12.setTitle(self.__tr("Input"))
        self.ledVertAxis.setText(self.__tr("0.0"))
        self.ledHoriAxis.setText(self.__tr("0.0"))
        self.textLabel1.setText(self.__tr("Verical Axis"))
        self.ledRotaAxis.setText(self.__tr("0.0"))
        self.textLabel1_3.setText(self.__tr("Rotation Axis"))
        self.btStart.setText(self.__tr("Start Joystick"))
        self.textLabel1_2.setText(self.__tr("Horizontal Axis"))
        self.btCalibrate.setText(self.__tr("Calibrate"))
        self.textLabel1_3_2.setText(self.__tr("Button Clicked"))
        self.cbxCalibration.clear()
        self.cbxCalibration.insertItem(self.__tr("1.0"))
        self.cbxCalibration.insertItem(self.__tr("63.998046875043258"))
        self.groupBox2.setTitle(self.__tr("Output"))
        self.textLabel1_4.setText(self.__tr("Command send"))
        self.textLabel1_4_2.setText(self.__tr("Configuration"))
        self.tblConfig.horizontalHeader().setLabel(0,self.__tr("Action"))
        self.tblConfig.horizontalHeader().setLabel(1,self.__tr("Input"))
        self.tblConfig.horizontalHeader().setLabel(2,self.__tr("Command"))
        self.tblConfig.horizontalHeader().setLabel(3,self.__tr("Description"))
        self.btSave.setText(self.__tr("Save"))
        self.btNew.setText(self.__tr("New"))
        self.textLabel1_5.setText(self.__tr("Dog No"))


    def slotStartStopJoystick(self):
        print "FormJoystickController.slotStartStopJoystick(): Not implemented yet"

    def slotNewAction(self):
        print "FormJoystickController.slotNewAction(): Not implemented yet"

    def slotSaveConfiguration(self):
        print "FormJoystickController.slotSaveConfiguration(): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("FormJoystickController",s,c)

if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    w = FormJoystickController()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()
