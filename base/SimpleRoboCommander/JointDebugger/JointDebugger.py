#!/usr/bin/python

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




import sys
from FormJointDebugger import FormJointDebugger
from qt import *

class JointInfo:
    def __init__(self, msgStr = None):
        
        self.ssHeadTilt = 0
        self.ssHeadPan = 0
        self.ssHeadCrane = 0
        
        self.ssFrontLeftJoint = 0
        self.ssFrontLeftShoulder = 0
        self.ssFrontLeftKnee = 0
        
        self.ssFrontRightJoint = 0
        self.ssFrontRightShoulder = 0
        self.ssFrontRightKnee = 0
        
        self.ssRearLeftJoint = 0
        self.ssRearLeftShoulder = 0
        self.ssRearLeftKnee = 0
        
        self.ssRearRightJoint = 0
        self.ssRearRightShoulder = 0
        self.ssRearRightKnee = 0
        
        self.cmdHeadTilt = 0
        self.cmdHeadPan = 0
        self.cmdHeadCrane = 0
        
        self.cmdFrontLeftJoint = 0
        self.cmdFrontLeftShoulder = 0
        self.cmdFrontLeftKnee = 0
        
        self.cmdFrontRightJoint = 0
        self.cmdFrontRightShoulder = 0
        self.cmdFrontRightKnee = 0
        
        self.cmdRearLeftJoint = 0
        self.cmdRearLeftShoulder = 0
        self.cmdRearLeftKnee = 0
        
        self.cmdRearRightJoint = 0
        self.cmdRearRightShoulder = 0
        self.cmdRearRightKnee = 0
        
        self.isValid = True
        
        self.msgStr = None
        if msgStr is not None:
            self.parseMessageString(str(msgStr))
            
    def parseMessageString(self,msgStr):
##~         print msgStr, type(msgStr)  
        self.msgStr = msgStr
        strArr = msgStr.split()
        print "Got msg contains ", len(strArr), " elements"
        self.isValid = len(strArr) == 30
        
    def toString(self):
        if self.msgStr :
            return self.msgStr
        else:
            return "TO BE APPEARED"
        
class JointDebugger(FormJointDebugger):
    def __init__(self,parent = None,name = None,fl = 0):
        FormJointDebugger.__init__(self,parent,name,fl)
        self.parent = parent
        self.bRecordMessage = False
        self.jointSetList = []
        
    def gotNewMessage(self,jointMessage):
        print jointMessage
        if self.bRecordMessage:
            self.lastJointSet = JointInfo(jointMessage)
            if self.lastJointSet.isValid :
                self.jointSetList.append(self.lastJointSet)
                self.spbNumReceived.stepUp()
    
    def slotStartStopRecording(self):
        if self.bRecordMessage:
            self.btStartStopRecord.setText("Start &Recording")
        else:
            self.btStartStopRecord.setText("Stop &Recording")
            
        msg = "__02jd__020"
        self.parent.slotServerSend(msg)            
        self.bRecordMessage = not self.bRecordMessage

    def slotSaveToFile(self):
        f = open("joints.log","w")
        for i in range(len(self.jointSetList)):
            f.write(self.jointSetList[i].toString() + "\n")
        print "%d lines saved to file \n" % len(self.jointSetList)
        f.close()
        
    def slotClearRecorded(self):
        print "Record cleared"
        self.jointSetList = []
        self.spbNumReceived.setValue(0)
        
if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    w = JointDebugger()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()
