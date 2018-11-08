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
from qt import *
from WdgRobotInfo import WdgRobotInfo
import WalkLearner

def learnParamToStr(para):
    return str(int(round(para)))

VERY_SLOW_SPEED = 0 #0cm/sec
LOG_FILE_NAME = "walking_robot.log"

class RobotInfo(WdgRobotInfo):
    def __init__(self,robotID = 0, parent = None,name = None,fl = 0):
        WdgRobotInfo.__init__(self,parent,name,fl)
        self.robotID = robotID
        self.parent = parent
        self.learningState = None
        self.forthTime = -1
        self.backTime = -1
        
        self.lcdRobotID.display( self.robotID )
        self.bStarted = False
        self.learningParameters = None  # this will be set in slotGetNewCommand()
        self.policyID = -1
        self.slotGetNewCommand()
        
        
    def setStatus(self, learningState, forth, back):
        self.learningState = learningState
        self.forthTime = forth
        self.backTime = back
        self.ledLearningState.setText( str(learningState) )
        self.ledForth.setText( "%.2f"%forth )
        self.ledBack.setText( "%.2f"%back )
        if self.parent.bStartLearning:
            self.continueLearning()
            
    def saveToLogFile(self,policy, value):
        f = open(LOG_FILE_NAME,"a")
        f.write("%s %f\n" % (policy,value))
        f.close()
        
    def continueLearning(self):
        if self.learningState == "lsDONE" :
            if self.forthTime + self.backTime > 0:
                self.saveToLogFile(self.learningParameters, self.forthTime + self.backTime)
            self.slotGetNewCommand()
            #if there's no policy for this iteration, just wait
            if self.policyID != -1:
                self.slotBeReady()
            
        elif self.learningState == "lsREADY":
            if self.learningParameters is None:
                self.slotGetNewCommand()
            if self.policyID != -1:
                self.ledParams.setText( " ".join( map(learnParamToStr,self.learningParameters)  ) ) # [1,2,3] => "1 2 3"
                self.slotLearnThis()
                self.slotStart()
            
    def slotGetNewCommand(self):
        print "RobotInfo.slotGetNewCommand"
        if not self.btSkipThisRun.isEnabled():
            self.btSkipThisRun.setEnabled(True)
            print "Skipped the result" 
            return #skipped, don't ask for new policy, just use old one
            
        nextPolicy = self.parent.requestNewCommand(self.policyID, self.forthTime, self.backTime)
        if nextPolicy is not None and nextPolicy != (None,None):
            (self.policyID, self.learningParameters) = nextPolicy
        else:
            print "++++++++++++++++++++++++++++++++++++++++ Got None", 
            self.policyID = -1

    def isDone(self):
        return self.learningState == "lsDone"
     
    def slotLearnThis(self):
        print "RobotInfo.slotLearnThis(): "
        learningParams = str(self.ledParams.text())
        if len(learningParams) < 94:
            msg = WalkLearner.WrlCommand("slp",self.robotID,learningParams)
        else:
            raise "learningParams too long"        
##~         print self.parentWidget() , type(self.parentWidget())
##~         print self.parentWidget().parentWidget() , type(self.parentWidget().parentWidget())
        print "learn this ", learningParams
        self.parent.parent.slotServerSend(msg)
        
    def slotSkipThisRun(self):
        self.slotStop()
##~         self.btSkipThisRun.setEnabled(False)
##~         print self.btSkipThisRun.isEnabled()
##~         if self.policyID != -1:
##~             self.parent.cancelThisRun(self.policyID)
##~             self.slotStop()
##~             self.policyID = -1
##~         self.btSkipThisRun.setEnabled(True)
##~         print self.btSkipThisRun.isEnabled()
        
    def slotStop(self):
        msg = WalkLearner.WrlCommand("stop",self.robotID) 
        self.btStartStop.setText("&GO !")
        self.parent.parent.slotServerSend(msg)
        
    def slotStart(self):
        msg = WalkLearner.WrlCommand("start" , self.robotID) 
        self.btStartStop.setText("&Stop !")
        self.parent.parent.slotServerSend(msg)
        
    def slotStartStop(self):
        print "RobotInfo.slotStartStop():"
        if self.bStarted:
            self.slotStop()
        else:
            self.slotStart()
        self.bStarted = not self.bStarted
        
    def slotBeReady(self):
        print "RobotInfo.slotBeReady(): "
        msg = WalkLearner.WrlCommand("ready",self.robotID)
        self.parent.parent.slotServerSend(msg)
        
    def slotSetDefault(self):
        print "RobotInfo.slotSetDefault():"
        self.ledParams.setText(" ".join( map(learnParamToStr,WalkLearner.STARTING_PARAMETERS) ) )
            
    def slotSetDone(self):
        print "RobotInfo.slotSetDone():"
        msg = WalkLearner.WrlCommand("set",self.robotID,"11") #11 = lsDONE
        self.parent.parent.slotServerSend(msg)
        
    def slotRunTooSlow(self):
        self.forthTime = self.backTime = VERY_SLOW_SPEED
        self.slotGetNewCommand()
        self.slotStop()
    
    def slotDogCrash(self):
        self.slotStop()
        self.parent.cancelThisRun(self.policyID)
        
if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    w = RobotInfo()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()
