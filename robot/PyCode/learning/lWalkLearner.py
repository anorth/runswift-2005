#
#   Copyright 2004 The University of New South Wales (UNSW) and National  
#   ICT Australia (NICTA).
#
#   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
#   redistribute it and/or modify it under the terms of the GNU General  
#   Public License as published by the Free Software Foundation; either  
#   version 2 of the License, or (at your option) any later version as  
#   modified below.  As the original licensors, we add the following  
#   conditions to that license:
#
#   In paragraph 2.b), the phrase "distribute or publish" should be  
#   interpreted to include entry into a competition, and hence the source  
#   of any derived work entered into a competition must be made available  
#   to all parties involved in that competition under the terms of this  
#   license.
#
#   In addition, if the authors of a derived work publish any conference  
#   proceedings, journal articles or other academic papers describing that  
#   derived work, then appropriate academic citations to the original work  
#   must be included in that publication.
#
#   This rUNSWift source is distributed in the hope that it will be useful,  
#   but WITHOUT ANY WARRANTY; without even the implied warranty of  
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along  
#   with this source code; if not, write to the Free Software Foundation,  
#   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


# 
#  Last modification background information
#  $Id: WalkLearner.py 4620 2005-01-06 23:58:24Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# WalkLearner.py
#
# A port of WalkingLearner.cc
#
#===============================================================================
from SysPath import *

import Constant
import Debug
import Global
import HeadAction
import HelpGlobal
import HelpMath
import HelpTrack
import IndicatorAction
import ReadyPlayer
import VisionLink
import WalkAction 


from lStateBasedAction import StateBasedAction
#
#===============================================================================



myLowBeacon     = Constant.vobYellowLeftBeacon
myHighBeacon    = Constant.vobYellowRightBeacon

TURNING_STEP    = 10
STAYING_STILL_PERIOD = 30
LEARNING_WALK = Constant.EllipseWalkWT
##~ LEARNING_WALK = Constant.SkellipticalWalkWT

def initPlayer():
    global myLowBeacon,myHighBeacon
    if Global.myPlayerNum == 1:
        myLowBeacon  = Constant.vobYellowLeftBeacon
        myHighBeacon = Constant.vobYellowRightBeacon
    elif Global.myPlayerNum == 2:
        myLowBeacon  = Constant.vobBlueLeftBeacon
        myHighBeacon = Constant.vobBlueRightBeacon
    

def connectStates(pipelineStates):
    #connect states into a pipeline
    for i in range(len(pipelineStates)-1):
        pipelineStates[i].setOutState(pipelineStates[i+1])
    
irSensorValueFar = None
irSensorValueNear = None

def resetEachFrame():
    global irSensorValueFar,irSensorValueNear
    irSensorValueFar = VisionLink.getAnySensor(Constant.ssINFRARED_FAR)
    irSensorValueNear = VisionLink.getAnySensor(Constant.ssINFRARED_NEAR)

def setTurningTowardVisualObject(vObject):
    if LEARNING_WALK == Constant.EllipseWalkWT:
        turnCCW = HelpMath.CLIP(vObject.getHeading() , 20)
        if abs(turnCCW) < 5:
            turnCCW = 0
    else:
        turnCCW = HelpMath.CLIP(vObject.getHeading()/2.0, 30)

    WalkAction.setForwardLeftTurn(None,None,-turnCCW)
    
MAX_FORWARD_NORMAL = 7
def setLearningWalkType():
    if LEARNING_WALK == Constant.EllipseWalkWT:
        WalkAction.setEllipticalWalk(0,0,0)
    elif LEARNING_WALK == Constant.NormalWalkWT:
        WalkAction.setNormalWalk(MAX_FORWARD_NORMAL,0,0)
    elif LEARNING_WALK == Constant.SkellipticalWalkWT:
        WalkAction.setSkellipticalWalk(MAX_FORWARD_NORMAL,None,None) #just to set walktype, no need for forward,left,turn
    else:
        print "Unknown learning walktype"
    
TRACK_DELTA_CRANEZ = 1.0
TRACK_THRESHOLD = 15.0 #degree
def setHeadTrackingBeacon(vBeacon):
    if vBeacon.getImgElevation()  < 0:
        cranez = - TRACK_DELTA_CRANEZ
    elif vBeacon.getImgElevation()  > TRACK_THRESHOLD:
        cranez = TRACK_DELTA_CRANEZ
    else:
        cranez = 0
        
    panx = vBeacon.getHeading()
    HeadAction.setHeadParams(panx,0,cranez, Constant.HTRel_z)
    
class IdleState(StateBasedAction):
    def __init__(self,name = None):
        StateBasedAction.__init__(self,name)
    
    def DoAction(self):
        HeadAction.setHeadParams(0,0,20,Constant.HTAbs_h)
        return self
    
class TurnAroundToward(StateBasedAction):
    def __init__(self,target,name = None): #target = vobIndex
        StateBasedAction.__init__(self,name)
        self.target = target
    
    def DoAction(self):
        WalkAction.setNormalWalk(0,0,TURNING_STEP)
        HeadAction.setHeadParams(0,0,10,Constant.HTAbs_h)
        vBeacon = Global.VisualObject(*VisionLink.getVisualObject(self.target))
        if vBeacon.getConfidence() > 0:
##~             HelpLong.setForceStepComplete()
            return self.outState
        else:
            return self

class AligningToward(StateBasedAction):
    OFF_TO_THE_RIGHT = -5
    OFF_TO_THE_LEFT = 5
    def __init__(self,target, name = None):
        StateBasedAction.__init__(self,name)
        self.target = target
        
    def DoAction(self):
        vBeacon = Global.VisualObject(*VisionLink.getVisualObject(self.target))
        if vBeacon.getConfidence() > 0:
            setHeadTrackingBeacon(vBeacon)
        if vBeacon.getHeading() < self.OFF_TO_THE_RIGHT or\
            vBeacon.getHeading() > self.OFF_TO_THE_LEFT:
            turnCCW = HelpMath.CLIP(vBeacon.getHeading() ,30)
            print "Aligning ",vBeacon.getHeading()
        else:
            turnCCW = 0
        WalkAction.setNormalWalk(0,0,turnCCW)
        if turnCCW == 0 :
            return self.outState
        else:
            return self        
            
class StepBackAligningStraight(StateBasedAction):
    OFF_TO_THE_RIGHT = -10
    OFF_TO_THE_LEFT = 10
    STUCK_THRESHOLD = 400
    def __init__(self,target,name = None):
        StateBasedAction.__init__(self,name)
        self.target = target
        
    def DoAction(self):
        WalkAction.setNormalWalk(-3,0,0)
        
        vBeacon = Global.VisualObject(*VisionLink.getVisualObject(self.target))
        if vBeacon.getConfidence() > 0:
            setHeadTrackingBeacon(vBeacon)
        if vBeacon.getHeading() < self.OFF_TO_THE_RIGHT:
            turnCCW = -TURNING_STEP
        elif vBeacon.getHeading() > self.OFF_TO_THE_LEFT:
            turnCCW = TURNING_STEP
        else:
            turnCCW = 0
        WalkAction.setForwardLeftTurn(None,None,turnCCW)
        print "PWM = ",ReadyPlayer.getStuckInfo()[0]
        if turnCCW ==0 and ReadyPlayer.getStuckInfo()[0] > self.STUCK_THRESHOLD: #aligned and stuck to the edge        
            return self.outState
        else:
            return self
        
class MovingToReadyState(StateBasedAction):
    def __init__(self,name = None,lowBeacon = myLowBeacon, 
            highBeacon = myHighBeacon):
        StateBasedAction.__init__(self,name)
        
        self.lowBeacon = lowBeacon
        self.highBeacon = highBeacon
        
        self.turnAroundAtHighBeacon = TurnAroundToward(lowBeacon)
        self.stepBackAligningStraight = StepBackAligningStraight(lowBeacon)
        
        self.turnAroundAtHighBeacon.setOutState(self.stepBackAligningStraight)
        
    def beginAction(self):
        self.doSubState(self.turnAroundAtHighBeacon, immediate = False)
        
    def DoAction(self):
        self.doSubState()
        if self.subState is None: 
            return self.outState
        else: 
            return self
        
class ReadyState(StateBasedAction):
    def __init__(self,name = None):
        StateBasedAction.__init__(self,name)
    
    def DoAction(self):
        WalkAction.setNormalWalk(0,0,0)
        return self
        
class StartLearning(StateBasedAction):
    def __init__(self,name = None):
        StateBasedAction.__init__(self,name)
    
    def DoAction(self):
        return self.outState
        
class PreparationState(StateBasedAction):
    def __init__(self,name = None):
        StateBasedAction.__init__(self,name)
        self.reset()        
        
    def reset(self):
        self.startTime = None
        self.walkingForthTime = -1
        self.walkingBackTime = -1
        self.stopDistance = -1
        
class Preparation1(PreparationState):
    def __init__(self,name = None):
        PreparationState.__init__(self,name)
        
    def DoAction(self):
        WalkAction.setNormalWalk(0,0,0)
        if self.counter == STAYING_STILL_PERIOD:
            self.startTime = HelpGlobal.getCurrentSimpleTime()
            return self.outState
        else:
            return self
        
    def getStartTime(self):
        return self.startTime
        
        
class WalkingForth(StateBasedAction):
    CLOSE_DISTANCE_FORTH = 400000
    
    def __init__(self,target = myLowBeacon,name = None):
        StateBasedAction.__init__(self,name)
        self.target = target
    
    def DoAction(self):
        setLearningWalkType()
        vBeacon = Global.VisualObject(*VisionLink.getVisualObject(self.target))
        if vBeacon.getConfidence() > 0:
            setTurningTowardVisualObject(vBeacon)
            setHeadTrackingBeacon(vBeacon)
        else:
            HeadAction.setHeadParams(0,0,10,Constant.HTAbs_h)
        
        if irSensorValueFar < self.CLOSE_DISTANCE_FORTH:
            return self.outState
        else:
            return self
            
class Preparation2(PreparationState):
    def __init__(self,name = None):
        PreparationState.__init__(self,name)
        self.sum = 0.0
        self.minIR = 10000000000
    def beginAction(self):
        WalkAction.setForceStepComplete()
        self.walkingForthTime = HelpGlobal.getCurrentSimpleTime() - preparation1.getStartTime()
        self.sum = 0.0
        self.minIR = 10000000000
        
    def DoAction(self):
        WalkAction.setNormalWalk(0,0,0)
        HelpTrack.spinningLocalise(True,20,20,0,3,-30,30)
##~         HelpLong.setHeadParams(0,0,20,Constant.HTAbs_h)
        if self.counter > STAYING_STILL_PERIOD:
            self.sum += irSensorValueFar
            if irSensorValueFar < self.minIR:
                self.minIR = irSensorValueFar
        if self.counter == 2*STAYING_STILL_PERIOD:
            self.stopDistance = self.minIR
            return self.outState
        else:
            return self
            
    def getWalkingTime(self):
        return self.walkingForthTime
    
    def getDistanceFromBeacon(self):
        return self.stopDistance
        

class Turning180(StateBasedAction):
    def __init__(self,name = None):
        StateBasedAction.__init__(self,name)
        self.turningAround = TurnAroundToward(myHighBeacon)
        self.aligningToward = AligningToward(myHighBeacon)
        
        self.turningAround.setOutState(self.aligningToward)
        
    def beginAction(self):
        self.doSubState(self.turningAround,immediate = False)
        
    def DoAction(self):
        self.doSubState()
        
        if self.subState is None:
            return self.outState
        else:
            return self
                
class Preparation3(PreparationState):
    STILL_ENOUGH = 15
    def __init__(self,name = None):
        PreparationState.__init__(self,name)
        
    def DoAction(self):
        WalkAction.setNormalWalk(0,0,0)
        if self.counter == self.STILL_ENOUGH:
            self.startTime = HelpGlobal.getCurrentSimpleTime()
            return self.outState
        else:
            return self
    
    def getStartTime(self):
        return self.startTime
        
class WalkingBack(StateBasedAction):
    CLOSE_DISTANCE_BACK = 350000
    def __init__(self,target = myHighBeacon,name = None):
        StateBasedAction.__init__(self,name)
        self.target = target
    
    def DoAction(self):
        setLearningWalkType()
        vBeacon = Global.VisualObject(*VisionLink.getVisualObject(self.target))
        if vBeacon.getConfidence() > 0:
            setTurningTowardVisualObject(vBeacon)
            setHeadTrackingBeacon(vBeacon)
        else:
            HeadAction.setHeadParams(0,0,10,Constant.HTAbs_h)
        
        if irSensorValueFar < self.CLOSE_DISTANCE_BACK:
            return self.outState
        else:
            return self
        
class Preparation4(PreparationState):
    def __init__(self,name = None):
        PreparationState.__init__(self,name)
        
        self.sum = 0.0
        self.minIR = 10000000000
        
    def beginAction(self):
        self.walkingBackTime = HelpGlobal.getCurrentSimpleTime() - preparation3.getStartTime()
        WalkAction.setForceStepComplete()
        self.sum = 0.0
        self.minIR = 10000000000
        
    def DoAction(self):
        WalkAction.setNormalWalk(0,0,0)
        HelpTrack.spinningLocalise(True,20,20,0,3,-30,30)
##~         HelpLong.setHeadParams(0,0,20,Constant.HTAbs_h)
        if self.counter > STAYING_STILL_PERIOD:
            self.sum += irSensorValueFar
            if irSensorValueFar < self.minIR:
                self.minIR = irSensorValueFar
        if self.counter == 2*STAYING_STILL_PERIOD:
            self.stopDistance = self.minIR
            return self.outState
        else:
            return self
              
    def getWalkingTime(self):
        return self.walkingBackTime
    
    def getDistanceFromBeacon(self):
        return self.stopDistance
        
class DoneState(StateBasedAction):
    def __init__(self,name = None):
        StateBasedAction.__init__(self,name)
    
    def DoAction(self):
        return self
              
class WalkingLearner(StateBasedAction):
    def __init__(self,name = None, learningWaklType = Constant.EllipseWalkWT):
        StateBasedAction.__init__(self,name)
        self.learningWaklType = learningWaklType
        
    def beginAction(self):
        self.doSubState(idleState,immediate = False)
        
    def DoAction(self):
        self.doSubState()
        if self.subState is None: return None
        else: return self
        
    def replyQuery(self):
        
        if self.subState is not None:
            currentState = self.subState.getName()
        else:
            currentState = "lsUnknown"
                
        message = "&&&&%d %s %.3f %.3f %d %d" % \
            (Global.myPlayerNum, 
            currentState,
            preparation2.getWalkingTime()/1000.0,
            preparation4.getWalkingTime()/1000.0,
            preparation2.getDistanceFromBeacon(),
            preparation4.getDistanceFromBeacon() )
        if Debug.bDebugOn:
            print "Sending : ",message
        VisionLink.sendMessageToBase(message)
    
    def setLearningParameters(self,parameters):
        if self.learningWaklType == Constant.EllipseWalkWT:
            prefix = "ELI "
        elif self.learningWaklType == Constant.SkellipticalWalkWT:
            prefix = "SKE "
            #HACK: SkellipticalWalk.cc should handle this
            global MAX_FORWARD_NORMAL
            MAX_FORWARD_NORMAL = float(parameters.split()[0])/10
        VisionLink.setWalkLearningParameters(prefix+parameters)

##################################################################
##              START OF THE PROGRAM
##
        
initPlayer()

idleState           = IdleState("lsIDLE") 
movingToReadyState  = MovingToReadyState("lsMOVING_TO_READY_STATE",myLowBeacon,myHighBeacon) 
readyState          = ReadyState("lsREADY") 
startLearning       = StartLearning("lsSTART_LEARNING")  
preparation1        = Preparation1("lsPREPARATION1")
walkingForth        = WalkingForth(myLowBeacon,"lsWALKING_FORTH")  
preparation2        = Preparation2("lsPREPARATION2")
turning180          = Turning180("lsTURNING180")
preparation3        = Preparation3("lsPREPARATION3")
walkingBack         = WalkingBack(myHighBeacon,"lsWALKING_BACK")
preparation4        = Preparation4("lsPREPARATION4") 
doneState           = DoneState("lsDONE")



#big parent state
walkingLearner      = WalkingLearner("WalkingLearner",LEARNING_WALK)
allStates           = [idleState, #0
    movingToReadyState,     #1
    readyState,
    startLearning,  #3
    preparation1,
    walkingForth,   #5
    preparation2,
    turning180,    
    preparation3,   #8
    walkingBack,    
    preparation4,   #10
    doneState]

connectStates(allStates)

def DecideNextAction():
    Global.frameReset()
    IndicatorAction.superDebug()
    resetEachFrame()
    walkingLearner.DecideNextAction()
    
def processCommand(cmdStr):
    cmdStr = cmdStr.strip()
    command = cmdStr.split()[0]
    parameters = cmdStr[len(command):]
    print "WalkLearner got [%s]%s." % (command,parameters)
    if command == "start":
        walkingLearner.doSubState(startLearning,immediate = False)
    elif command == "query":
        walkingLearner.replyQuery()
    elif command == "stop":
        walkingLearner.doSubState(idleState,immediate = False)
        preparation1.reset()
        preparation2.reset()
        preparation3.reset()
        preparation4.reset()
    elif command == "set":
        walkingLearner.doSubState(allStates[int(parameters.split()[0])],immediate = False)
    elif command == "slp":
        walkingLearner.setLearningParameters(parameters)
    elif command == "ready":
        walkingLearner.doSubState(movingToReadyState,immediate = False)
        

