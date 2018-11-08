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

# Form implementation generated from reading ui file '/home/kcph007/trunk/base7/work/SimpleRoboCommander/DlgPyChecker.ui'
#
# Created: Mon May 10 02:33:20 2004
#      by: The PyQt User Interface Compiler (pyuic) 3.11
#
# WARNING! All changes made in this file will be lost!


import sys
from qt import *

class DlgPyChecker(QDialog):
    def __init__(self,parent = None,name = None,modal = 0,fl = 0):
        QDialog.__init__(self,parent,name,modal,fl)

        if not name:
            self.setName("DlgPyChecker")


        DlgPyCheckerLayout = QGridLayout(self,1,1,11,6,"DlgPyCheckerLayout")

        self.btFix = QPushButton(self,"btFix")

        DlgPyCheckerLayout.addWidget(self.btFix,1,0)

        self.btIgnore = QPushButton(self,"btIgnore")

        DlgPyCheckerLayout.addWidget(self.btIgnore,1,1)

        self.tedResult = QTextEdit(self,"tedResult")

        DlgPyCheckerLayout.addMultiCellWidget(self.tedResult,0,0,0,1)

        self.languageChange()

        self.resize(QSize(503,480).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.btFix,SIGNAL("clicked()"),self,SLOT("reject()"))
        self.connect(self.btIgnore,SIGNAL("clicked()"),self,SLOT("accept()"))


    def languageChange(self):
        self.setCaption(self.__tr("Pychecker result"))
        self.btFix.setText(self.__tr("Damn, let me fix it"))
        self.btIgnore.setText(self.__tr("I know, just upload, please"))


    def __tr(self,s,c = None):
        return qApp.translate("DlgPyChecker",s,c)

if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    w = DlgPyChecker()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()
