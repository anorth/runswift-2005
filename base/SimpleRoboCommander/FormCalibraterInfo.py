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

# Form implementation generated from reading ui file '/home/kcph007/trunk/base7/work/SimpleRoboCommander/FormCalibraterInfo.ui'
#
# Created: Fri Jun 11 23:36:44 2004
#      by: The PyQt User Interface Compiler (pyuic) 3.11
#
# WARNING! All changes made in this file will be lost!


import sys
from qt import *

class FormCalibraterInfo(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("FormCalibraterInfo")


        FormCalibraterInfoLayout = QGridLayout(self,1,1,11,6,"FormCalibraterInfoLayout")

        self.btStartStop = QPushButton(self,"btStartStop")

        FormCalibraterInfoLayout.addWidget(self.btStartStop,3,0)

        self.textLabel2 = QLabel(self,"textLabel2")

        FormCalibraterInfoLayout.addWidget(self.textLabel2,0,0)

        self.textLabel1 = QLabel(self,"textLabel1")

        FormCalibraterInfoLayout.addWidget(self.textLabel1,2,0)

        self.textLabel1_2 = QLabel(self,"textLabel1_2")

        FormCalibraterInfoLayout.addWidget(self.textLabel1_2,1,0)

        self.ledFileName = QLineEdit(self,"ledFileName")

        FormCalibraterInfoLayout.addWidget(self.ledFileName,0,1)

        self.ledDistance = QLineEdit(self,"ledDistance")

        FormCalibraterInfoLayout.addWidget(self.ledDistance,1,1)

        self.ledBlobArea = QLineEdit(self,"ledBlobArea")

        FormCalibraterInfoLayout.addWidget(self.ledBlobArea,2,1)

        self.btReset = QPushButton(self,"btReset")

        FormCalibraterInfoLayout.addWidget(self.btReset,3,1)

        self.languageChange()

        self.resize(QSize(229,192).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.btStartStop,SIGNAL("clicked()"),self.slotStartStopSending)
        self.connect(self.btReset,SIGNAL("clicked()"),self.slotReset)


    def languageChange(self):
        self.setCaption(self.__tr("Calibrater Info"))
        self.btStartStop.setText(self.__tr("Send"))
        self.textLabel2.setText(self.__tr("FileName"))
        self.textLabel1.setText(self.__tr("Robot Blob Area"))
        self.textLabel1_2.setText(self.__tr("Dog Distance"))
        self.ledFileName.setText(self.__tr("calibrater.log"))
        self.ledDistance.setText(self.__tr("100"))
        self.btReset.setText(self.__tr("Reset"))


    def slotStartStopSending(self):
        print "FormCalibraterInfo.slotStartStopSending(): Not implemented yet"

    def slotReset(self):
        print "FormCalibraterInfo.slotReset(): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("FormCalibraterInfo",s,c)

if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    w = FormCalibraterInfo()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()
