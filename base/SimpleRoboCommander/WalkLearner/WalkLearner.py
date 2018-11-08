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




#TODO
#handle case when there's no more policy in the iteration -> set to DONE, and wait
#review the structure
#play with maximization
import sys
from qt import *
from FormWalkLearner import FormWalkLearner
from RobotInfo import RobotInfo
from GradientDescent import GradientDescent
from LearnError import LearnError
from PowellDescent import PowellDescent, bDebugThread
import traceback

TWO_SECONDS = 5.0 * 1000
NUMBER_OF_REPETITION = 2
##~ EllipticalWalkStartingParams = [80, 90, 130,  45,  15,   -65,    5, 40,  20, 45, 40 , 100]
##~ EllipticalWalkStartingParams = [80, 80, 100,  54,  10,   -54,    4, 38,  18, 47, 38]
    #    [80, 90, 130,  45,  15,   -65,    5, 40,  20, 45, 40]
    #        PG    HF   HB         FFO    FSO      BFO     BSO     FW     FH   BW    BH
##~ EllipticalWalkStartingParams = [76, 94, 126,  43,  13,   -63,    6, 43,  22, 49, 40 , 100]    
EllipticalWalkStartingParams =  [60, 94, 124, 44, 0, -62, 7, 20, 32, 20, 38, 95]
EllipticalWalkDelta =           [ 2,  2,  2,   1 ,  1 ,    1 ,  1,   2,  2,  2,  2  ,  5]

 

SkellipticalWalkStartingParams = [85, 40, 15, 15, 30, 25, 50, 55, 1, 1, 1, 1,  10,  20,  20, 40, 55, 15, 80, -55, 5, 115, 0, 0, 3, 0, 0]
SkellipticalWalkDelta =         [ 1,  2,  1,  1,  1,  1,   5,  5, 0, 0, 0, 0,   2,   5,   5,  5, 1, 1,   1,   1, 1,   1, 0, 0, 0, 0, 0]
##~ 
##~ STARTING_PARAMETERS = SkellipticalWalkStartingParams
##~ DELTA_PARAMETERS = SkellipticalWalkDelta

##~ SkellipticalWalkStartingParams = [6, 50, 15, 25, 30, 40, 0.1, 0.5, 0.1, 0.5, 0.1, 0.5, 0.1, 0.5, 45, 15, 90, -65, 5, 110, 0]
##~ SkellipticalWalkDelta = [ 0.1, 2, 1, 1,  1,  1, 0.05, 0.1, 0.05, 0.1, 0.05, 0.1, 0.05, 0.1, 1, 1, 2, 1, 1, 2, 1]

STARTING_PARAMETERS = EllipticalWalkStartingParams
DELTA_PARAMETERS = EllipticalWalkDelta


#Choice of GradientDescent , PowellDescent, or LearnError (measure error for a particular parameter)
LearningAlgorithm = GradientDescent

bDebugMessaging = False
bDebugVelocity = False

#everything is in mm
def getTrueWalkingVelocity(forth,back,stopDistance1, stopDistance2):
    
    fieldWidth = 2700.0 #mm
    dogLength = 210.0   #mm
    border = 200.0
    if stopDistance1 < 200 or stopDistance1 > 800: 
        stopDistance1 = border
    if stopDistance2 < 200 and stopDistance2 > 800: 
        stopDistance2 = border
        
    walkingDistance1 = fieldWidth - dogLength - (stopDistance1 - border)
    
    forthVel = walkingDistance1 / forth
    
    walkingDistance2 = fieldWidth - dogLength - (stopDistance1 - border) - (stopDistance2 - border) 
     
    backVel = walkingDistance2 / back 
    return (forthVel,backVel)
    
def getRobustMeasurement(listOfEval):
    listOfEval.sort()
    i1 = len(listOfEval) / 2 -1
    i2 = len(listOfEval) / 2
    return ( listOfEval[i1] + listOfEval[i2] ) / 2

bPythonPlayer = True
def WrlCommand(command,robotID = 0,parameters = ""):
    if bPythonPlayer:
        return "__03pyc__99%d %s %s" % (robotID,command,parameters)
    else:
        if command == "ready": command = "red"
        if robotID == 9: robotID = 0 #"all" in C++
        return "__03wrl__99%s %d %s" % (command,robotID,parameters)

class WalkLearner(FormWalkLearner):
    
    def __init__(self,parent = None,bContinueFromFile = False, name = None,fl = 0):
        FormWalkLearner.__init__(self,parent,name,fl)
        self.parent = parent
        
        self.tabRobots.removePage(self.tabRobots.currentPage() )
        self.robotInfoList = []
        #store evaluation of each repetition = policyID -> ([values] , policy)
        self.repetitionMap = {}
        #free runID, run that's not done 
        self.freePolicy = []
        
        
        if bContinueFromFile:
            self.gradDescent = LearningAlgorithm(False)
        else:
            self.gradDescent = LearningAlgorithm(True)
            self.gradDescent.setParameters(STARTING_PARAMETERS)
            self.gradDescent.setDeltaParameters(DELTA_PARAMETERS)
        
        self.bStartLearning = False
        
    def runToPolicy(self,runID):
        return runID / NUMBER_OF_REPETITION
    def policyToRun(self,policyID, nth):
        return policyID * NUMBER_OF_REPETITION + nth
        
    def checkHasPolicyRunInFreePool(self,policyID):
        for i in self.freePolicy:
            if self.runToPolicy(i) == policyID:
                return True
        return False
        
    def addToThePool(self,runID, value):
        policyID = self.runToPolicy(runID)
        if self.repetitionMap.has_key(policyID) :
            pool = self.repetitionMap[policyID][0]
            pool.append(value) # add to values list
            
            #if this policyID is done:
            if len(pool) == NUMBER_OF_REPETITION:
                if self.checkHasPolicyRunInFreePool(policyID):
                    raise "Fatal error: Free pool has not released all runs, policyID = %d" % policyID
                policyValue = getRobustMeasurement(pool)
                del(self.repetitionMap[policyID]) #remove from the pool
                (policyID,policy) = self.requestNewOptimization(policyID,policyValue)
                if (policyID,policy) != (None, None) :
                    self.repetitionMap[policyID] = ([],policy)
                    for i in range(NUMBER_OF_REPETITION):
                        self.freePolicy.append(self.policyToRun(policyID,i))
                    
                
    def findRunInThePool(self):
        if len(self.freePolicy) == 0:
            (policyID,policy) = self.requestNewOptimization(-1,-1)
            if (policyID,policy) != (None, None) :
                    self.repetitionMap[policyID] = ([],policy)
                    for i in range(NUMBER_OF_REPETITION):
                        self.freePolicy.append(self.policyToRun(policyID,i))
                            
        if len(self.freePolicy) > 0:
            runID = self.freePolicy.pop(0)
            policy = self.repetitionMap[self.runToPolicy(runID)][1]
            return (runID,policy)
        else:
            print "No more policy, probably optimization is done"
            return (-1,-1)
        
    #return (id, policy ) 
    def requestNewCommand(self, runID, forthTime, backTime):
        if runID > -1 and  forthTime > 0 and backTime > 0:
            self.addToThePool(runID, forthTime+backTime)
            
        (runID,policy) = self.findRunInThePool()
        self.showPoolStatus()
        return (runID,policy)
        
    def requestNewOptimization(self,policyID,value):
        if policyID > -1 and  value > 0:
            self.gradDescent.setEvaluation(policyID, value)
            (params, bestValue) = self.gradDescent.getBestParameter()
            self.ledBestParams.setText(str(params))
            self.ledBestValue.setText(str(bestValue))
        return self.gradDescent.getNextEvaluation()
        
    def cancelThisRun(self, runID):        
        #self.gradDescent.cancelEvaluation(policyID)
        if runID in self.freePolicy:
            print " WARNING : runID %d is already in free pool"%runID
        else:
            self.freePolicy.append(runID)
            
    def showPoolStatus(self):
        s = ""
        for key in self.repetitionMap.keys():
            s += " ~ %d : %s " % (key,self.repetitionMap[key][0])
                
        s += " Free: %s" % self.freePolicy
        self.ledRunTimes.setText(s)
            
    def slotNumRobotChange(self):
        print "WalkLearner.slotNumRobotChange(): "
        numRobot = int( str( self.ledNumRobot.text() ) )
        if numRobot < len(self.robotInfoList):
            for i in range(len(self.robotInfoList) ,numRobot , -1):
                self.tabRobots.removePage(self.robotInfoList[i-1])
                del(self.robotInfoList[i-1])
        elif numRobot > len(self.robotInfoList):
            for i in range(len(self.robotInfoList), numRobot ):
                self.slotAddNewRobot()
            
    def slotAddNewRobot(self):
        robotID = len(self.robotInfoList) + 1
        currWid = RobotInfo(robotID, self)
        self.robotInfoList.append(currWid)
        self.tabRobots.addTab(currWid,"Robot " + str(len(self.robotInfoList)) )
        currWid.show()
        self.ledNumRobot.setText( str(len( self.robotInfoList) )) 
        
    def slotGotNewMessage(self,msg):
        if bDebugMessaging:
            print "Getting msg " , str(msg)
        #msg = [robotID]   [learningState]   [forth]  [back]
        
        try:
            arr = msg.split()
            robotID = int(arr[0])
            learningState = str(arr[1])
            forth = float(arr[2])
            back = float(arr[3])
            stopDistance1 = int(float(arr[4]) / 1000.0)  #mm
            stopDistance2 = int( float(arr[5]) / 1000.0)
            
            if bDebugVelocity:
                print "Velocity %f %f" % ( 2460.0 / forth , 2460.0 / back )
                print "IR Distance %d %d adjusted velocity %f %f" % \
                ( (stopDistance1, stopDistance2) + getTrueWalkingVelocity(forth,back,stopDistance1,stopDistance2) )
                
            forth,back = getTrueWalkingVelocity(forth,back,stopDistance1,stopDistance2)
            
            if robotID > 0 and robotID <= len(self.robotInfoList):
                self.robotInfoList[robotID-1].setStatus(learningState, forth, back)
            elif robotID > len(self.robotInfoList) :
                for i in range(len(self.robotInfoList) , robotID):
                    self.slotAddNewRobot()
                self.robotInfoList[robotID-1].setStatus(learningState, forth, back)
            else:
                print "Unknown robot" 
        except:
            traceback.print_exc()
            print "Probably corrupted wireless messages. No worries!"
            
    def slotQueryAllRobots(self):
        print "WalkLearner.slotQueryAllRobots(): "
        msg = WrlCommand("query",9)
        self.parent.slotServerSend(msg)
    
    def slotStartAllRobots(self):
        print "WalkLearner.slotStartAllRobots(): "
        msg = WrlCommand("start",9)
        self.parent.slotServerSend(msg)
   
   
    def slotStopAllRobots(self):
        print "WalkLearner.slotStopAllRobots(): "
        msg = WrlCommand("stop",9)
        self.parent.slotServerSend(msg)
        
    def slotStartStopLearning(self):
        if self.bStartLearning:
            self.bStartLearning = False
            self.btStartStopLearning.setText( "Start Learning" )
            self.timer.stop()
        else:
            self.bStartLearning = True
            self.btStartStopLearning.setText( "Stop Learning" )
            self.timer = QTimer()
            QObject.connect(self.timer,SIGNAL("timeout()"), self.slotQueryAllRobots)
            self.timer.start( TWO_SECONDS )
        

    def hide(self):
        print "HAHAHA"
        return FormWalkLearner.hide(self)
        
if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    w = WalkLearner()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()
