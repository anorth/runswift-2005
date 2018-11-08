##     Copyright 2005 The University of New South Wales (UNSW) and National  
##     ICT Australia (NICTA).
##  
##     This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
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
##     should be included in that publication.
##  
##     This rUNSWift source is distributed in the hope that it will be useful,  
##     but WITHOUT ANY WARRANTY; without even the implied warranty of  
##     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
##     General Public License for more details.
##  
##     You should have received a copy of the GNU General Public License along  
##     with this source code; if not, write to the Free Software Foundation,  
##     Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


import Global
import Constant
import VisionLink
import hMath
import Indicator
import Action
import sGrabDribble
import hTrack
#import math

WITHIN_VISUAL_DISTANCE = 80 # cm
MAX_VISUAL_DISTANCE = WITHIN_VISUAL_DISTANCE + 200 

########################## TARGET defs ##################################
class Target:
    def __init__(self, name, indColor, vobId):
        self.__name, self.__indColor, self.__vInfoId = name, indColor, vobId
        self.__vInfo = None
        self.reset()
    def __str__(self):
        return self.__name
    def frameReset(self): # should be set once in every frame
        self.__vInfo = Global.VisualObject(
            *VisionLink.getVisualObject(self.__vInfoId) )
        if not self.__vInfo.isVisible():
            self.__withInDistanceCount = 0
            self.__outOfDistanceCount = 0
            self.__lostCount += 1
        else:
            self.__lostCount = 0
    def lostCount(self):
        return self.__lostCount
    def vInfo(self):
        return self.__vInfo
    def withInVisualDistance(self, dist):
        if self.__vInfo.isVisible() and self.__vInfo.getDistance() < dist:
            self.__withInDistanceCount += 1
        return self.__withInDistanceCount > 3
    def outOfVisualDistance(self, dist):
        if self.__vInfo.isVisible() and self.__vInfo.getDistance() > dist:
            self.__outOfDistanceCount += 1
        return self.__outOfDistanceCount > 3
    def indicate(self):
        if self.__vInfo.isVisible():
            Indicator.showHeadColor(self.__indColor)
        else:
            Indicator.showHeadColor(Indicator.RGB_NONE)
    def reset(self):
        self.__lostCount = 1000 # long time
        self.__oldHeading = 180 # big heading
        self.__withInDistanceCount = 0
        self.__outOfDistanceCount = 0
        self.__lostCount = 0
    def getRobustHeading(self):
        if self.__vInfo.isVisible():
            self.__lostCount = 0
            self.__oldHeading = self.__vInfo.getHeading()
            return self.__oldHeading
        elif self.__lostCount < 15: # 1/2 seconds
            self.__lostCount += 1
            return self.__oldHeading
        else:
            return self.__oldHeading * 100           

# all possible targets
ball = Target("Ball", Indicator.RGB_WHITE, Constant.vobBall)
blueGoal = Target("BLUE GOAL", Indicator.RGB_BLUE, Constant.vobBlueGoal)
yellowGoal = Target("YELLOW GOAL", Indicator.RGB_YELLOW, Constant.vobYellowGoal)
pinkOnBlue = Target("PINK ON BLUE BEACON", Indicator.RGB_PURPLE,
                    Constant.vobPinkOnBlueBeacon)
blueOnPink = Target("BLUE ON PINK BEACONS", Indicator.RGB_PURPLE,
                    Constant.vobBlueOnPinkBeacon)
pinkOnYellow = Target("PINK ON YELLOW BEACONS", Indicator.RGB_ORANGE,
                      Constant.vobPinkOnYellowBeacon)
yellowOnPink = Target("YELLOW ON PINK BEACONS", Indicator.RGB_ORANGE,
                      Constant.vobYellowOnPinkBeacon)

# the order has to match the order in the
# checkbox in walkBase
objList = [ball, blueOnPink, pinkOnBlue, pinkOnYellow, yellowOnPink, blueGoal, yellowGoal]

######################### END of TARGETS defs ##################################        

def walk(fwd, lft, trn, wt):
    if command == "std":
        Action.rawWalk(0, 0, 0, wt)
        if currentMode == grabTurn:
            grabTurn.look()
            Action.openMouth()
    else:
        Action.rawWalk(fwd, lft, trn, wt) # we use raw walk to gain maximum flexibility
    Global.forceHighGain = forceHighGain

        
######################### CALIBRATION MODE defs ##################################  
class CalMode:
    def __init__(self, name, targetList):
        self.__name, self.__targetList = name, targetList
        self.__currentTargetIndex = 0
        self.__target = targetList[self.__currentTargetIndex]
        self.reset()
    def __str__(self):
        return "Calibration Mode: "+self.__name
    def look(self):
        pass
    def walk(self):
        pass
    def shouldStartTiming(self):
        pass
    def shouldEndTiming(self):
        pass
    def reset(self):
        self.__target.reset()
    def target(self):
        return self.__target
    def changeTarget(self):
        if len(self.__targetList) == 1:
            return # if there's only one target
        self.__currentTargetIndex = (self.__currentTargetIndex + 1) % \
                                    len(self.__targetList)
        self.__target = self.__targetList[self.__currentTargetIndex]
        self.__target.reset()


class CalWalkToTarget(CalMode):
    def __init__(self, name, targetList):
        CalMode.__init__(self, name, targetList)        
        self.reset()
        self.count = 0
        self.dist = 0
        self.adist = 0
        self.acount = 0
    def reset(self):
        CalMode.reset(self)
        self.__isWalkingToTarget = False
    def shouldStartTiming(self):
        return self.__isWalkingToTarget
    def shouldEndTiming(self):
        if forward >= 0:
            return self.target().withInVisualDistance(WITHIN_VISUAL_DISTANCE)
        else: # if forward < 0
            return self.target().outOfVisualDistance(MAX_VISUAL_DISTANCE)
    def setWalkingToTarget(self, isWalking):
        self.__isWalkingToTarget = isWalking
    def lookOrig(self):
        vInfo = self.target().vInfo()
        if vInfo.isVisible(): # if we can see the object.
            Action.setHeadParams(vInfo.getHeading(), 0, 25)
        else:
            Action.setHeadParams(self.panOffset(), 0, 25)
    def look(self):
        vInfo = self.target().vInfo()
        if vInfo.isVisible(): # if we can see the object.
            craneChange = vInfo.getElev()
            if 0 < craneChange < 10:
                craneChange = 0
            Action.setHeadParams(vInfo.getHeading(), 0, craneChange, Action.HTRel_z)
##             dist = vInfo.getDistance()            
##             if self.count < 30:
##                 self.dist += dist
##             elif self.count == 30:
##                 #print "BodyTilt:", math.degrees(Global.pWalkInfo.getBodyTilt()),
##                 #print "Elev:", vInfo.getElev(),
##                 meandist = self.dist / self.count
##                 #print "dist", meandist,
##                 self.adist += meandist
##                 self.acount +=1
##                 #print "Mean:", self.adist / self.acount, "count:", self.acount
##                 if self.acount > 20:
##                     self.adist = 0
##                     self.acount = 0
##             self.count += 1
##             if self.count > 30:
##                 self.count = 0
##                 self.dist = 0
        else:
            if Global.desiredCrane > 27:
                craneChange = -3
            elif Global.desiredCrane < 23:
                craneChange = 3
            else:
                craneChange = 0
            Action.setHeadParams(self.panOffset(), 0, craneChange, Action.HTRel_z)
            self.count = 0
            self.dist = 0
            
    def panOffset(self):
        pass
    def walk(self):
        heading = self.target().getRobustHeading() - self.panOffset()
        # act according to heading
        if abs(heading) < 30:
            self.setWalkingToTarget(True)
            if abs(heading) < 5:
                walk(forward, left, 0, walkType)
            else:
                walk(forward, left, hMath.CLIP(heading*turnAdjustment, walkToTargetMaxTurn),
                            walkType)
        else:
            self.searchOnSpot(heading)    
    def searchOnSpot(self, heading):
        self.__isWalkingToTarget = False
        walk(0, 0, hMath.CLIP(heading, walkToTargetMaxTurn), walkType)
        
        
        
class CalForward(CalWalkToTarget):
    def __init__(self, name, targetList):
        CalWalkToTarget.__init__(self, name, targetList)
    def walk(self):
        global left
        left = 0
        CalWalkToTarget.walk(self)
    def panOffset(self):
        return 0

class CalGrabForward(CalForward):
    def __init__(self, name, targetList):
        CalForward.__init__(self, name, targetList)
    def look(self):
        sGrabDribble.moveHeadForward()        

class CalSideWay(CalWalkToTarget):
    def __init__(self, name, targetList):
        CalWalkToTarget.__init__(self, name, targetList)
    def walk(self):
        global forward
        forward = 0
        CalWalkToTarget.walk(self)
    def panOffset(self):
        if left > 0:
            return 90
        elif left < 0:
            return -90
        else: # if left == 0:
            return Global.desiredPan


class CalTurn(CalMode):
    def __init__(self, name, targetList):
        CalMode.__init__(self, name, targetList)
    def shouldStartTiming(self):
        return not self.shouldEndTiming()
    def shouldEndTiming(self):
        vInfo = self.target().vInfo()
        return vInfo.isVisible() and \
               abs(vInfo.getHeading() - self.panOffset()) < 5
    def panOffset(self):
        return 0
    def look(self):
        Action.setHeadParams(0, 0, -20)
    def walk(self):
        walk(0, 0, turn, walkType)

class CalGrabTurn(CalTurn):
    def look(self):
        global grabHeadTilt, grabHeadCrane
        if grabHeadTilt and grabHeadCrane:
            Action.setHeadParams(0, grabHeadTilt, grabHeadCrane)
        else:
            sGrabDribble.moveHeadForward()
    def walk(self):
        walk(forward, left, turn, walkType) 

class CalForwardTurn(CalTurn):
    def look(self):
        Action.setHeadParams(0, 0, 20)
    def walk(self):
        walk(forward, 0, turn, walkType)

class CalSidewayTurn(CalTurn):
    def look(self):
        Action.setHeadParams(self.panOffset(), 0, 20)
    def walk(self):
        walk(0, left, turn, walkType)
    def panOffset(self):
        if left > 0:
            return 90
        elif left < 0:
            return -90
        else: # if left == 0:
            return Global.desiredPan


# All possible calibration modes            
#forwardOnly = CalForward("Forward Only", [yellowGoal, blueGoal])
forwardOnly = CalForward("Forward Only", [pinkOnBlue, blueOnPink])
sideWayOnly = CalSideWay("Sideway Only", [pinkOnYellow, yellowOnPink])
turnOnly = CalTurn("Turn Only", [ball])
grabTurn = CalGrabTurn("Grab Turning", [ball])
forwardTurn = CalForwardTurn("Forward Turning", [pinkOnBlue])
sidewayTurn = CalSidewayTurn("Sideway Turning", [pinkOnBlue])
grabForward = CalGrabForward("Grab Forward", [ball])
backwardOnly = None
grabSideWay = None

#################### END of CALIBRATION MODE defs ##################################


        
###### MAIN DRIVER FUNCTIONS (DecideNextAction & processCommand) ###################


isTiming = False
recvCmdCountDown = 0
def indicateIsTiming():
    if isTiming:
        Indicator.finalValues[Indicator.HeadWhite] = Indicator.LED2_ON
    else:
        Indicator.finalValues[Indicator.HeadWhite] = Indicator.LED2_OFF

def indicateReceivedCommand():
    global recvCmdCountDown
    if recvCmdCountDown > 0:
        recvCmdCountDown -=1
        Indicator.showHeadColor(Indicator.RGB_GREEN)
        

def sendWalkInfo(elapsed):
    walk(forward, left, turn, walkType)
    VisionLink.sendAtomicAction(*tuple(Action.finalValues))    
    VisionLink.sendWalkInfo(int(elapsed))   


def DecideNextAction():
    current = VisionLink.getBatteryLevel()
    if current < 40: # if battery is low
        Action.stopLegs()
        hTrack.stationaryLocalise()
        return

    if command == "std" or command == "adj" or not currentMode:
        adjWalk()
    elif command == "cal" or command == "lrn":
        calWalk()
    else:
        return # for all other commands, do nothing

    # indicate
    indicateReceivedCommand()
    Indicator.showBatteryStatus()
    if shouldOpenMouth or currentMode == grabTurn:
        Action.openMouth()
    else:
        Action.closeMouth()
    
def adjWalk():
    walk(forward, left, turn, walkType)


stopLegsCount = 0
startTimeStamp = VisionLink.getCurrentTime()
def calWalk():    
    # 1. initialise
    target = currentMode.target()
    target.frameReset() # update visual info every frame
  
    # 2. act
    currentMode.look()
    currentMode.walk()
    # 2.1 special case
    if currentMode == forwardOnly and forward > 0 and \
           target.lostCount() > 4 * 30: # lost the target for more than 4 sec.
        currentMode.changeTarget()

    global stopLegsCount
    if stopLegsCount > 0:
        #Action.stopLegs()
        #Action.finalValues[Action.WalkType] = Action.NormalWalkWT
        stopLegsCount -= 1
        if stopLegsCount == 0:
            global lastForward
            if not ( lastForward * forward < 0 ): #if not diff. signs
                currentMode.changeTarget()
            lastForward = forward
            currentMode.reset()
        return

    # 3. time
    global isTiming, startTimeStamp
    if isTiming:
        flc = VisionLink.getSwitchSensorCount(Constant.SWITCH_FL_PALM)
        frc = VisionLink.getSwitchSensorCount(Constant.SWITCH_FR_PALM)
        # if we hit something, or the current mode tell us to stop.
        if flc > 0 or frc > 0 or \
               currentMode.shouldEndTiming():         
            endTimeStamp = VisionLink.getCurrentTime()
            elapsed = hMath.getTimeElapsed(startTimeStamp, endTimeStamp)
            if elapsed > 1300:# 1.3 sec is the minimal time b/t twode detection
                isTiming = False                
                print "--------------------------------"
                print currentMode
                print "timestamp:", endTimeStamp
                print "I'm close to", target
                print "time elapsed: ", elapsed, " milli seconds"
                print
                sendWalkInfo(elapsed)
                Action.forceStepComplete()
                stopLegsCount = 1
            else:
                print "time elapsed is too short"                
    else: # if not isTiming:
        if currentMode.shouldStartTiming():
            isTiming = True
            startTimeStamp = VisionLink.getCurrentTime()

    # 4. indicate (note the order, and latter one can overwrite former)
    Indicator.showDropFrameIndicator()
    target.indicate()
    indicateIsTiming()

        


#####################################
#Parameters that will be changed from calWalk base station
command = "cal"
lastCommand = command
currentMode = forwardOnly
walkType = Action.SkellipticalWalkWT
forward = 8.2
lastForward = forward
left = 0.0
turn = 0.0
turnAdjustment = 1.0
walkToTargetMaxTurn = 20
walkStr = None
forceHighGain = None
grabHeadTilt = None
grabHeadCrane = None
learningParamsStr = None
#####################################


shouldOpenMouth = False

def processCommand(cmdStr):
    global command, lastCommand, recvCmdCountDown, shouldOpenMouth
    global lastForward
    global forward, left, turn, turnAdjustment, walkToTargetMaxTurn
    global learningParamsStr
    cmdList = cmdStr.strip().split()
    lastCommand = command
    command = cmdList[0]
    parameters = cmdList[1:]    
    # indication of received command
    shouldOpenMouth = not shouldOpenMouth
    recvCmdCountDown = 30 # 1 sec

    if command == "std":  # stand still
        return  # will be handled in walk()
    if command == "rsm":        
        VisionLink.setWalkLearningParameters(learningParamsStr)            
        return
    elif command == "hdp": # setting grab head params
        global grabHeadTilt, grabHeadCrane
        grabHeadTilt = int(parameters[0])
        grabHeadCrane = int(parameters[1])
        command = lastCommand # revert back to last command
    elif command == "cal" or command == "lrn":
        #format of parameters: mode walktype gain turnAdjustment walkToTargetMaxTurn fwd lft trn [...]
        changeWalkType( int(parameters[1]) )
        changeCalMode( int(parameters[0]) )
        changeGain( parameters[2])
        turnAdjustment = float(parameters[3]) / 100.0
        walkToTargetMaxTurn = float(parameters[4])
        lastForward = forward
        forward = float(parameters[5])
        left = float(parameters[6])
        turn = float(parameters[7])
        # need to process more when learning
        if command == "lrn":
            paramsIdx = len(command)
            for i in xrange(8):
                paramsIdx += 1+len(parameters[i])
                learningParamsStr = walkStr+cmdStr[paramsIdx:]
            print __name__, "Sending learning params:", (learningParamsStr)
            VisionLink.setWalkLearningParameters(learningParamsStr)
    elif command == "adj":
        changeWalkType( int(parameters[0]) )
        changeGain( parameters[1])
        forward = float(parameters[2])
        left = float(parameters[3])
        turn = float(parameters[4])
    else:
        print __name__,"Unknown command:", command

        

def changeWalkType(wt):
    global walkType, walkStr
    if wt == 0:
        walkType = Action.NormalWalkWT
        walkStr = "NOR"
    elif wt == 1:
        walkType = Action.EllipticalWalkWT
        walkStr = "ELI"
    elif wt == 2:
        walkType = Action.SkellipticalWalkWT
        walkStr = "SKE"
    else:
        walkType = Action.NormalWalkWT
        walkStr = "NOR"
        print __name__, "Unknown walk type:", walkType


def changeCalMode(mode):
    global currentMode, walkType
    if mode == 0:
        currentMode = forwardOnly
    elif mode == 1:
        currentMode = sideWayOnly
    elif mode == 2:
        currentMode = turnOnly
    elif mode == 3:
        currentMode = grabTurn
    elif mode == 5:
        currentMode = forwardTurn
    elif mode == 6:
        currentMode = sidewayTurn
    elif mode == 8:
        currentMode = None
    if currentMode:
        currentMode.reset()

    
def changeGain(gainStr):
    global forceHighGain
    if gainStr == "d":
        print __name__, "dynamic gain"
        forceHighGain = None
    elif gainStr == "h": # high gain
        print __name__, "high gain"
        forceHighGain = True
    elif gainStr == "l": # low gain
        print __name__, "low gain"
        forceHighGain = False
    else:
        print __name__, "Unknown Gain Command:", gainStr
        print __name__, "Using dynamic gain"
        forceHighGain = None
