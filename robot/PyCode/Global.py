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
#  $Id: Global.py 4690 2005-01-21 00:51:43Z weiming $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# Global.py
#
# Contains visual and gps information, and also commands to be sent to
# actuators. Every player modules import this to use these info.
#
#===============================================================================
import math
import hMath
import Constant
import VisionLink

# Ok so I know this is poor style. I may even be responsible for a rant on
# the subject. However this will prevent people committing Global with the
# challenge flags turned on.
from Challenge import * # put these in the Global namespace

print "loading Global"

       
#===============================================================================
# Sub-classes of this module
#===============================================================================

class WirelessTeammateInfo:
    # Wireless information from teammates.
        
    # counter: Whenever wireless info arrive, counter is set (set by gps.cc)
    #          If nothing is received, it'll be decreased by 1 unless already 0.
    #          Tell you how recent the information is.
    # timeToReachBall : Time for this teammate to reach the ball. 
    #                   Unfortunately, 2004 ppl hacked this value and in ready 
    #                   state this variable is used to specify the kick off 
    #                   position of this teammate 
    # ballX : The global X of the ball which this teammate thinks
    # ballY : The global Y of the ball which this teammate thinks
    # ballVar : The variance of the ball which this teammate thinks
    # hasGrabbedBall : Boolean to signal this teammate has grabbed the ball 
    # hasSeenBall : Boolean to signal this teammate has seen the ball
    # kickedOff : NEVER USED! it was there from 2004.. WILL BE REMOVED!
    # role : The role which this teammate is currently assigned.
    # roleCounter : The number of frames which this teammate is assigned to 
    #               the current role for.

    def __init__(self, arg):
        self.__playerNum,\
        self.__counter,\
        self.__timeToReachBall,\
        self.__ballX,\
        self.__ballY,\
        self.__ballVar,\
        self.__hasGrabbedBall,\
        self.__hasSeenBall,\
        self.__hasLostBall,\
        self.__role,\
        self.__roleCounter = arg
        
        self.__ballDistValid = False 

    def setVals(self, arg):
        self.__playerNum,\
        self.__counter,\
        self.__timeToReachBall,\
        self.__ballX,\
        self.__ballY,\
        self.__ballVar,\
        self.__hasGrabbedBall,\
        self.__hasSeenBall,\
        self.__hasLostBall,\
        self.__role,\
        self.__roleCounter = arg
        
        self.__ballDistValid = False                 

    def getCopy(self):
        return WirelessTeammateInfo([self.__playerNum,\
        self.__counter,\
        self.__timeToReachBall,\
        self.__ballX,\
        self.__ballY,\
        self.__ballVar,\
        self.__hasGrabbedBall,\
        self.__hasSeenBall,\
        self.__hasLostBall,\
        self.__role,\
        self.__roleCounter])
                                    
    def isValid(self):
        return self.__counter > 0
    def getCounter(self):
        return self.__counter
    def getTimeToReachBall(self): 
        return self.__timeToReachBall
    def getBallDist(self):
    
        if not(self.__ballDistValid):
            if self.__playerNum != None:
                self.__ballDist = hMath.getDistanceBetween(\
                                    teammatesLoc[self.__playerNum].getX(),\
                                    teammatesLoc[self.__playerNum].getY(),\
                                    self.__ballX,\
                                    self.__ballY)
            self.__ballDistValid = True
            
        return self.__ballDist
        
    def getBallX(self): 
        return self.__ballX 
    def getBallY(self): 
        return self.__ballY
    def getBallVar(self):
        return self.__ballVar
    def hasGrabbedBall(self): 
        return self.__hasGrabbedBall
    def hasSeenBall(self): 
        return self.__hasSeenBall
    def hasLostBall(self):
        return self.__hasLostBall
    def getRole(self): 
        return self.__role
    def getRoleCounter(self): 
        return self.__roleCounter + 12 - self.__counter 
               
    #hacked value for kick off state
    def getKickedOff(self):
        return self.__timeToReachBall

    def isAttacker(self):
        return self.__role == Constant.ATTACKER
    def isSupporter(self):
        return self.__role == Constant.SUPPORTER
    def isStriker(self):
        return self.__role == Constant.STRIKER        
    def isWinger(self): 
        return self.__role == Constant.WINGER
    def isDefender(self):
        return self.__role == Constant.DEFENDER
    def isBackOff(self): 
        return self.__role == Constant.BACKOFF
    def isGoalie(self): 
        return self.__role == Constant.GOALIE
            
    #def getTuple(self):
    #    return (self.__counter,self.__isStriker, self.__ballDist, self.__isGoalie, self.__hasGrabbedBall,
    # self.__isBackOff, self.__kickedOff,self.__isSupporter,)
        
def createNullWirelessTeammateInfo():
    return WirelessTeammateInfo([None,None,None,None,None,None,None,None,None,None,None])
    
#--------------------------------------
# Self information. 
class Vector:
    # x,y: Cartesian coordinates of the vector, usually mean coordinates on field.
    # theta, d: Polar format.
    # head: theta - 90deg (I don't know what for).
    # angleNorm: Angle before wrapping round to 0 again. Not used yet, may del later.
    
    def __init__(self,*arg):
        if len(arg) == 6:
            self.__x , self.__y, self.__d , self.__theta , self.__head , self.__angleNorm = arg
        else:
            print "arg length = ", len(arg)
            raise Exception("Number of argument to Vector() must be 6")
    def setVals(self,*arg):
        if len(arg) == 6:
            self.__x , self.__y, self.__d , self.__theta , self.__head , self.__angleNorm = arg
        else:
            print "arg length = ", len(arg)
            raise Exception("Number of argument to Vector() must be 6")     
    def getCopy(self):
        return Vector(self.__x , self.__y, self.__d , self.__theta , self.__head , self.__angleNorm)              
    def getX(self):
        return (self.__x)
    def getY(self):
        return (self.__y)
    def getPos(self):
        return (self.__x, self.__y)        
    def getDistance(self):
        return self.__d        
    def getTheta(self):
        return self.__theta        
    def getAngleNorm(self):
        return self.__angleNorm
    def getHeading(self):
        return self.__head
    def sub(self,otherVec):
        self.__x -= otherVec.__x
        self.__y -= otherVec.__y
        return self
    def calcTheta(self):
        if (self.__x == 0 or self.__y == 0):
            self.__theta = 0
        else:
            self.__theta = hMath.RAD2DEG(math.atan2(self.__y, self.__x));
    def calcHead(self):
        self.calcTheta()

        # principle() not ported, I think the one below is equivalent - dan.
        # self.__head = principle(self.__theta - HALF_CIRCLE / 2);
        self.__head = hMath.normalizeAngle_0_360(self.__theta - 90);
    def getTuple(self):
        return (self.__x , self.__y, self.__d , self.__theta , self.__head , self.__angleNorm)
    

def createNullVector():
    return Vector(None,None,None,None,None,None)


#--------------------------------------
# PWalk information. 
class PWalkInfo:
    def __init__(self,*arg):
        if len(arg) != 11:
            raise Exception("Number of argument to PWalkInfo() must be 11, but it was %d" % len(arg))
    
        self.__walkType, self.__minorWalkType, self.__step, self.__PG, \
            self.__leadingLeg, self.__frontShoulderHeight, self.__bodyTilt,\
            self.__desiredPan, self.__desiredTilt, self.__desiredCrane, \
            self.__isHighGain = arg
            
    def setVals(self,*arg):
        if len(arg) != 11:
            raise Exception("Number of argument to PWalkInfo() must be 11, but it was %d" % len(arg))
    
        self.__walkType, self.__minorWalkType, self.__step, self.__PG, \
            self.__leadingLeg, self.__frontShoulderHeight, self.__bodyTilt,\
            self.__desiredPan, self.__desiredTilt, self.__desiredCrane, \
            self.__isHighGain = arg       
            
    def getCopy(self):
        return PWalkInfo(self.__walkType, self.__minorWalkType, self.__step, self.__PG, \
            self.__leadingLeg, self.__frontShoulderHeight, self.__bodyTilt,\
            self.__desiredPan, self.__desiredTilt, self.__desiredCrane, \
            self.__isHighGain)        

    def getCurrentWalkType(self):
        return self.__walkType

    def getCurrentMinorWalkType(self):
        return self.__minorWalkType

    def getCurrentStep(self):
        return self.__step

    def getCurrentPG(self):
        return self.__PG        

    def getLeadingLeg(self):
        return self.__leadingLeg

    def getFrontShoulderHeight(self):
        return self.__frontShoulderHeight
        
    def getBodyTilt(self):
        return self.__bodyTilt
        
    def getDesiredHead(self):
        return (self.__desiredPan, self.__desiredTilt, self.__desiredCrane)

    def isUsingHighGain(self):
        return self.__isHighGain
        
    def getTuple(self):
        return (self.__walkType, self.__step, self.__PG, \
            self.__leadingLeg, self.__frontShoulderHeight, self.__bodyTilt,\
            self.__desiredPan, self.__desiredTilt, self.__desiredCrane, \
            self.__isHighGain)

def createNullPWalkInfo():
    return PWalkInfo(None,None,None,None,None,None,None,None,None,None,None)           
            
#--------------------------------------
# World model objects information.

class WMObj:
    def __init__(self,*arg):
        if len(arg) != 5:
            raise Exception("Number of argument to WMObj() must be 5")
        self.__x, self.__y, self.__posVar, self.__h, self.__hVar = arg
    def setVals(self,*arg):
        if len(arg) != 5:
            raise Exception("Number of argument to WMObj() must be 5")
        self.__x, self.__y, self.__posVar, self.__h, self.__hVar = arg
    def getCopy(self):
        return WMObj(self.__x, self.__y, self.__posVar, self.__h, self.__hVar)
    def getX(self):
        return (self.__x)
    def getY(self):
        return (self.__y)
    def getPos(self):
        return (self.__x, self.__y)        
    def getPosVar(self):
        return self.__posVar
    def getHeading(self):
        return self.__h
    def getHeadingVar(self):
        return self.__hVar
    def getTuple(self):
        return (self.__x, self.__y, self.__posVar, self.__h, self.__hVar)
        
def createNullWMObj():
    return WMObj(None,None,None,None,None)        
        
#--------------------------------------
# Visual objects information.

class VisualObject:
    def __init__(self,*arg):
        if len(arg) != 15:
            raise Exception("Number of argument to VisualObject() must be 15, while it's %d" % len(arg))
        self.__cf, self.__head, self.__d, self.__var, \
            self.__x, self.__y,  self.__cx, self.__cy, \
            self.__height, self.__width, \
            self.__imgHead, self.__imgElev, self.__imgDir, \
            self.__vobDist, self.__elev = arg
            
    def setVals(self,*arg):
        if len(arg) != 15:
            raise Exception("Number of argument to VisualObject() must be 15, while it's %d" % len(arg))
        self.__cf, self.__head, self.__d, self.__var, \
            self.__x, self.__y,  self.__cx, self.__cy, \
            self.__height, self.__width, \
            self.__imgHead, self.__imgElev, self.__imgDir, \
            self.__vobDist, self.__elev = arg
            
    def getCopy(self):
        return VisualObject(self.__cf, self.__head, self.__d, self.__var, \
            self.__x, self.__y,  self.__cx, self.__cy, \
            self.__height, self.__width, \
            self.__imgHead, self.__imgElev, self.__imgDir, \
            self.__vobDist, self.__elev)   
                 
    def getConfidence(self):
        return self.__cf
    def isVisible(self):
        if self.__cf != None:
            return self.__cf > 0
        else:
            return False
    def getVariance(self):
        return self.__var
    def getCentroid(self):
        return (self.__cx, self.__cy)
    def getX(self):
        return self.__x
    def getY(self):
        return self.__y
    def getPos(self):
        return (self.__x, self.__y) # note it is the global x,y position on field
    def getHeight(self):
        return self.__height
    def getWidth(self):
        return self.__width
    def getImgHeading(self):
        return self.__imgHead
    def getImgElevation(self):
        return self.__imgElev
    def getImgDirection(self):
        return self.__imgDir
    def getHeading(self):         # the heading of the object relative tot the robot's neckbase
        return self.__head
    def getDistance(self):        # the flat distance from the robot's neck-base to the vob
        return self.__d
    def getVobDistance(self):
        return self.__vobDist
    def setPos(self,x,y):
        self.__x = x
        self.__y = y
    def getElev(self):
        return self.__elev
    def getTuple(self):
        return (self.__cf, self.__head, self.__d, self.__var, \
            self.__x, self.__y,  self.__cx, self.__cy, \
            self.__height, self.__width, \
            self.__imgHead, self.__imgElev, self.__imgDir, \
            self.__vobDist,self.__elev)    
    
def createNullVisualObject():
    return VisualObject(None,None,None,None,None,None,None,None, \
        None,None,None,None,None,None,None)                 
        
        
#===============================================================================
# Global variables of this module.
#===============================================================================

#-----------------------------------------------------------------------------
#Set-once variables (i.e. Constants)
#-----------------------------------------------------------------------------

# Ready state kickoff position.
kickOffState = Constant.NOTDECIDED
kickOffTeam = None
kickOffPos = None       # x, y position chosen


finalWirelessInfo = [-1,False,False,False,False,False,False] 

# The current frame number, set incremented by Behaviou.processFrame()

cameraFrame = 0
frame    = 0 
firstPlayFrame = 0  # First frame after set state
lastFrameReset = -1 #to prevent frameReset() more than twice in the same frame 

# Robot state.
state     = None
penalised = None


# The player number I am assigned, default is 1
myPlayerNum = None
# Indexes for teammates other than myself
otherTeammates = []
# Indexes for valid teammates other than myself
otherValidTeammates = []
# Indexes for valid forwards other than myself'
otherValidForwards = []
# My team color
teamColor = None
# True if we are running a goalie behaviour
isGoalieBehaviour = False


desiredHead     = (0,0,0) # (Pan, Tilt, Crane)
desiredPan,desiredTilt,desiredCrane = desiredHead


# Is time critical mode set
isTimeCritical = False

# GPS (self) information.
gpsGlobalBall = createNullVector()
gpsLocalBall  = createNullVector()
selfLoc       = createNullWMObj()  # WMObj
teammatesLoc  = [createNullWMObj(),createNullWMObj(),createNullWMObj(),createNullWMObj()]  # Tuple of WMObj.

# GPS (shared - wireless) information
teamPlayers                 = [createNullWirelessTeammateInfo(),\
                                createNullWirelessTeammateInfo(),\
                                createNullWirelessTeammateInfo(),\
                                createNullWirelessTeammateInfo()]
                                
framesSince3ForwardsPresent = 0
framesSince2ForwardsPresent = 0

# Wireless ball from teammate with ok-low-variance
# It does not have a class, but simply a tuple of (x,y, variance, source robot)
# from getSharedBallInfo.
sharedBall            = None
sharedBallX           = None      # = sharedBall[0], for convenience
sharedBallY           = None      # = sharedBall[1], for convenience
sharedBallVar         = None      # = sharedBall[2], for convenience
sharedBallSourceRobot = None      # = sharedBall[3], for convenience

# Number of attackers this frame (and number who don't have lostBall)
attackerCount = 0
attackerWithBallCount = 0

# Vision information stored as visual objects
vBall       = createNullVisualObject()
vTGoal      = createNullVisualObject()
vOGoal      = createNullVisualObject()
vBluePink   = createNullVisualObject()
vPinkBlue   = createNullVisualObject()
vYellowPink = createNullVisualObject()
vPinkYellow = createNullVisualObject()


lastVisBall = createNullVisualObject()
fstVisBallDist = 0
fstVisBallHead = 0
sndVisBallDist = 0
sndVisBallHead = 0
thdVisBallDist = 0
thdVisBallHead = 0
weightedVisBallDist = 0
weightedVisBallHead = 0        
        

# For convenience, got set by determineBallSource()
#--------------------------------------
# NOTE: ballX and ballY are global coordinate
# BUT:    ballH and ballD are relative to the robot neck base
# Want a local one? Go to HelpShort.getLocalCoordinate

ballX       = None
ballY       = None
ballD       = None
ballH       = None
ballSource  = None

#PWalkInfo
pWalkInfo   = createNullPWalkInfo()

seenBeacon  = None

# How many frames vision ball lost
lostBall    = 0

# How many frames vision has ball 
haveBall    = 0

lostGoal    = 0
haveGoal    = 0

# Role switching variables.
formation     = None
myLastRole    = Constant.NOROLE
myRole        = Constant.NOROLE
myRoleCounter = 0
debugBehaviour_line = False
bDebugForwardLeftTurn = False

# DKDRange (dkd, narrowMin, narrowMax, broadMin, broadMax)
DKD     = None

# The number of forwards
forwardAlive = 1


# Motor gain - is it high gain
isHighGain      = False
forceHighGain   = None #True if forced to be High Gain, False if forced to be low gain

# Stuck detection. 1 second = 30 frames.
STUCK_NUMBER_OF_FRAMES = 60
lastMaxPWMValues = [(0,0)] * STUCK_NUMBER_OF_FRAMES


def framePreset():
    global frame
    global cameraFrame
    frame += 1
    cameraFrame = VisionLink.getCameraFrame()

#return number of milliseconds of the current time
# Returns the number of millisecond from within an interval of 1000 seconds
# so that range = 0...10e6
def getCurrentSimpleTime():
    (seconds, microSeconds) = VisionLink.getCurrentTime()
    return (seconds % 1000 ) * 1000 + microSeconds / 1000


# Returns a random number that will be the same random number for each robot
# at any given time. We use the game score to seed this.
def getSharedRandom():
    score = VisionLink.getScore()
    seed = score[0] * 31 + score[1] * 487
    VisionLink.srand(seed)
    return VisionLink.random()


pan = tilt = crane = 0    

# following variables no longer create
# a new class instance every frame
# --------------------------------
# gpsGlobalBall *
# gpsLocalBall *
# selfLoc *
# teammatesLoc *
# teamPlayers *
# vBall *
# vTGoal *
# vOGoal *
# vBluePink *
# vPinkBlue *
# vYellowPink *
# vPinkYellow *
# lastVisBall *
# pWalkInfo *

#--------------------------------------
# Called by processFrame() in Behaviou.py, update ball/self location to latest
# information for each frame.
def frameReset():
    VisionLink.startProfile("GlobalFrameReset")
    global lastFrameReset, forceHighGain, isTimeCritical
    global myRole, myLastRole
    global pan, tilt, crane
    global selfLoc, teammatesLoc, gpsLocalBall, gpsGlobalBall
    global sharedBall, sharedBallX, sharedBallY, sharedBallVar, sharedBallSourceRobot    
    global pWalkInfo,desiredHead,desiredPan,desiredTilt,desiredCrane,isHighGain
    global myPlayerNum, otherTeammates, otherValidTeammates, otherValidForwards, teamColor
    global lastVisBall
    global fstVisBallDist, fstVisBallHead, sndVisBallDist, sndVisBallHead, thdVisBallDist, thdVisBallHead
    global weightedVisBallDist, weightedVisBallHead
    global attackerCount, attackerWithBallCount
    
    global vBall, vTGoal, vOGoal
    global vBluePink, vPinkBlue, vYellowPink, vPinkYellow, seenBeacon
    
    if frame == lastFrameReset:
        return #already reset in this frame, quit
    else:
        lastFrameReset = frame
        
    # Need to be reset, otherwise it over written by dynamic_gain,
    # which is very dangerous
    forceHighGain = None
    
    #Kick.frameReset()
    
    myLastRole = myRole

    isTimeCritical = VisionLink.isTimeCritical()
    
    #-----------------------------------
    # Get the head motion info    
    pan   = VisionLink.getJointSensor(Constant.jHeadPan)
    tilt  = VisionLink.getJointSensor(Constant.jHeadTilt)
    crane = VisionLink.getJointSensor(Constant.jHeadCrane)
    
    #-----------------------------------
    # Get the GPS info    
    gpsGlobalBall.setVals(*VisionLink.getGPSBallInfo(Constant.CTGlobal))
    gpsLocalBall.setVals(*VisionLink.getGPSBallInfo(Constant.CTLocal))
    selfLoc.setVals(*VisionLink.getGPSSelfInfo())
    #print "frameReset setting selfLoc at", ("%x" % id(selfLoc)) ,"=", selfLoc
    
    for i in range(Constant.NUM_TEAM_MEMBER):
        teammatesLoc[i].setVals( \
            *VisionLink.getGPSTeammateInfo(Constant.CTGlobal, i+1))
                                    
    #--------------------------------------
    # Get the Shared ball
    sharedBall = VisionLink.getSharedBallInfo();
    sharedBallX, sharedBallY, sharedBallVar, sharedBallSourceRobot = sharedBall

    #-----------------------------------
    # Get pWalkInfo    
    pWalkInfo.setVals(*VisionLink.getPWalkInfo())
    desiredHead   = pWalkInfo.getDesiredHead()
    desiredPan,desiredTilt,desiredCrane = desiredHead
    isHighGain = pWalkInfo.isUsingHighGain()
    

    #-----------------------------------
    # Get team related info
    myPlayerNum  = VisionLink.getMyPlayerNum()
    
    # VisionLink.setGPSGoal(Constant.BLUE_TEAM)
    # VisionLink.setGPSGoal(Constant.RED_TEAM)
    teamColor = VisionLink.getTeamColor()
#    if penaltyShot:  # Force penalty shooter colour
#        teamColor = Constant.RED_TEAM
#    elif lightingChallenge:  # Force challenge colour
#        teamColor = Constant.BLUE_TEAM

    if teamColor == Constant.RED_TEAM: # RED
        ownGoal       = Constant.vobYellowGoal
        targetGoal    = Constant.vobBlueGoal
    elif teamColor == Constant.BLUE_TEAM: # BLUE
        ownGoal       = Constant.vobBlueGoal
        targetGoal    = Constant.vobYellowGoal
    else:
        print "Warning: teamColor is None, the dog's coordinate is wrong"


    #-----------------------------------
    # Get the VisualObjects  
     
    vBluePink.setVals(*VisionLink.getVisualObject(Constant.vobBlueOnPinkBeacon))
    vPinkBlue.setVals(*VisionLink.getVisualObject(Constant.vobPinkOnBlueBeacon))
    vYellowPink.setVals(*VisionLink.getVisualObject(Constant.vobYellowOnPinkBeacon)) 
    vPinkYellow.setVals(*VisionLink.getVisualObject(Constant.vobPinkOnYellowBeacon))

    if vBluePink.getConfidence() > 0\
        or vPinkBlue.getConfidence() > 0\
        or vYellowPink.getConfidence() > 0\
        or vPinkYellow.getConfidence() > 0:
        seenBeacon = True
    else:
        seenBeacon = False
    
    if vBall.isVisible():
        lastVisBall.setVals(*vBall.getTuple())     
    
    vBall.setVals(*VisionLink.getVisualObject(Constant.vobBall))
    
    # visual ball smoothing for ball grabbing
    if vBall.isVisible():
        thdVisBallDist = sndVisBallDist
        thdVisBallHead = sndVisBallHead
        sndVisBallDist = fstVisBallDist
        sndVisBallHead = fstVisBallHead
        fstVisBallDist = vBall.getDistance()
        fstVisBallHead = vBall.getHeading()
        weightedVisBallDist = fstVisBallDist*0.5\
                                + sndVisBallDist*0.3\
                                + thdVisBallDist*0.2
        weightedVisBallHead = fstVisBallHead*0.5\
                                + sndVisBallHead*0.3\
                                + thdVisBallHead*0.2
        #weightedVisBallHead = vBall.getHeading() * 0.2\
        #                        + weightedVisBallHead * 0.8

    vOGoal.setVals(*VisionLink.getVisualObject(ownGoal))
    vTGoal.setVals(*VisionLink.getVisualObject(targetGoal))
    
    #-----------------------------------
    # Goals doesnt change their global positions
    vOGoal.setPos(Constant.OWN_GOAL_X   ,Constant.OWN_GOAL_Y   )
    vTGoal.setPos(Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y)

    #-----------------------------------
    # Set default final actions/indicators/wireless info.
    global finalWirelessInfo
    finalWirelessInfo = [-1,False,False,False,False,False,False]
    
    #-----------------------------------
    # Other variable to be reset
    global lostBall, haveBall
    global lostGoal, haveGoal

    # Increase the lostBall counter if can't see a ball.
    if vBall.isVisible():        
        haveBall += 1
        lostBall = 0            
    else:        
        lostBall += 1
        if True or lostBall >= 3:
            haveBall = 0
        
    # Cap lostBall if ball was just out so GPS ball is used. GPS ball is set
    # to the drop-in points. This also allows role switching on the replaced
    # ball
    # (NOT doing this to the goalie, as it may cause unexpected behaviours.)
    if VisionLink.gameDataIsValid() and \
           0 <= VisionLink.getDropInTime() < Constant.DROP_IN_REACT_TIME:
        #print "lostBall hack: dropInTime =", VisionLink.getDropInTime()
        lostBall = hMath.CLIP(lostBall, Constant.LOST_BALL_LAST_VISUAL)
    
    if vTGoal.isVisible(): 
        lostGoal = 0
        haveGoal += 1
    else: 
        lostGoal += 1
        haveGoal = 0
    
    determineBallSource()
    
    #-----------------------------------
    # Set the wireless teammate communication variables.
    global teamPlayers, forwardAlive, framesSince3ForwardsPresent, framesSince2ForwardsPresent
    wirelessInfo = VisionLink.getWirelessTeammateInfo()

    for i in range(Constant.NUM_TEAM_MEMBER):
        teamPlayers[i].setVals(\
                 [i,\
                 wirelessInfo[i],\
                 wirelessInfo[i+4],\
                 wirelessInfo[i+8],\
                 wirelessInfo[i+12],\
                 wirelessInfo[i+16],\
                 wirelessInfo[i+20],\
                 wirelessInfo[i+24],\
                 wirelessInfo[i+28],\
                 wirelessInfo[i+32],\
                 wirelessInfo[i+36]])
    
           
    otherTeammates = []
    otherValidTeammates = []     
    otherValidForwards = []  
    attackerCount = 0
    attackerWithBallCount = 0            
    for i in range(Constant.NUM_TEAM_MEMBER):
    
        if i != myPlayerNum - 1:   
            otherTeammates.append(i)        
                                       
            if teamPlayers[i].isValid():
                otherValidTeammates.append(i)   
                                         
                if not teamPlayers[i].isGoalie():
                    otherValidForwards.append(i)
                                                       
                    if teamPlayers[i].isAttacker():
                        attackerCount = attackerCount + 1
                        
                        if not teamPlayers[i].hasLostBall():
                            attackerWithBallCount = attackerWithBallCount + 1
                                      
                                      
    #------------------------------------------------
    # This is the RoboCup Game state, see the rule book    
    global state, lastMaxPWMValues, penalised 
    state = VisionLink.getTheCurrentMode()
    penalised = VisionLink.getPenalised()

    lastMaxPWMValues.pop(0)
    lastMaxPWMValues.append(getMaxPWMDuty())
    
    VisionLink.stopProfile("GlobalFrameReset")


def determineBallSource():
    global ballSource, ballX, ballY, ballD, ballH
    
    #--------------------------------------
    # NOTE: ballX and ballY are global coordinate
    # BUT:    ballH and ballD are relative to the robot neck base
    # Want a local one? Go to hMath.getLocalCoordinate
    (selfX, selfY) = selfLoc.getPos()
    selfH = selfLoc.getHeading()
    
    # If vision ball is available, use it since it is the most accurate.
    if vBall.isVisible():
        ballSource          = Constant.VISION_BALL
        ballX, ballY        = vBall.getPos()
        ballH               = vBall.getHeading()
        ballD               = vBall.getVobDistance()

    # Check if wireless ball is accurate.
    # Maybe teammate got lost, then their "reliable" ball's gps coord will
    # be wrong.
    elif (sharedBallVar < (VisionLink.getGPSBallMaxVar() + \
            hMath.get95var(Constant.USE_WIRELESS_BALL_VAR))
            and lostBall > Constant.VBALL_LOST):
        ballSource = Constant.WIRELESS_BALL
        ballX      = sharedBallX
        ballY      = sharedBallY
        ballH      = hMath.normalizeAngle_180(hMath.getHeadingBetween\
        (selfX, selfY, ballX, ballY) - selfH)
        ballD      = hMath.getDistanceBetween(ballX, ballY, selfX, selfY)

    # Use gps ball if nothing better.
    else:
        ballSource = Constant.GPS_BALL
        ballX, ballY = gpsGlobalBall.getPos()
        ballH      = gpsLocalBall.getHeading()
        ballD      = gpsLocalBall.getDistance()
        
    # Stop divides by zero
    if ballX == 0: ballX = 0.01;
    if ballY == 0: ballY = 0.01;
    if ballD == 0: ballD = 0.01;
    if ballH == 0: ballH = 0.01;
    
# This is for pReady.py, which figures out when a dog is
# stuck...it requires the lastMaxPWMValues
def getLastMaxPWMValues():
    return lastMaxPWMValues    

    
#--------------------------------------
# Return numeric value.
# Get the maximum PWM values of current joints. When motor is jamed (something
# blocks the dog's way), it should be much higher.
def getMaxPWMDuty():
    maxVal = 0
    whichLeg = 0 #see Constant.legFRONT_RIGHT, ....
    for i in range(3,Constant.NUM_OF_EFFECTORS): #ignore head pwm
        thisJointVal = abs(VisionLink.getJointPWMDuty(i+1))

        if (maxVal < thisJointVal):
            maxVal = thisJointVal
            whichLeg = i / 3

    return (maxVal,whichLeg)
        
    
