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
#  $Id: VisionLink.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 

#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# VisionLink.py
#
# Actual implementation of VisionLink is in PyEmbed.cc in robot/vision.
# This file is here for two reasons
# 1. So pychecker doesn't complain about non-existant functions
# 2. For offline simulation
#
# NOTHING IN HERE IS EVER EXECUTED BY THE ROBOT
# All the methods are executed only when running offline, i.e. they are hacks
#
#===============================================================================

#===============================================================================# Simulation data.
# This data is only ever used offline. Some items assume the robot is on the
# red team
#===============================================================================
import Constant
import hMath
import math

frame = 0
playerNum = 2
teamNum = 1

# WMObjs. Initialise these outside (classes cannot be imported here)
me = None
ball = None
team = [None]*4          # 4 WMObjs (I am player 2)
opponents = [None]* 4    # 4 WMObjs

#===============================================================================
# Functions belong to this module.
#===============================================================================

# Because each function must have something indented as its content, empty 
# function needs to have the keyword "pass".
# Functions listed here should always be the same as the ones in Vision_methods
# table in PyEmbed.cc. 

#---------------------------------------
# General functions
def setCallbacks(*arg):
    pass

# OFFLINE ONLY. Called each frame to set up the C++ vision/gps/behaviours
# to run offline. Arguments are GPS data to push into GPS:
# selfx, selfy, selfh, ballx, bally
def processFrame(*arg):
    pass
    
#---------------------------------------
# Actuator related functions
def sendAtomicAction(*arg):
    pass 

def sendIndicators(*arg):
    pass 

def sendMessageToBase(*arg):
    pass 

#get dog's battery capacity range = [0..90+] ???
def getBatteryLevel(*arg):
    pass 

def getFallenState(*arg):
    pass 

def getPressSensorCount(pressSensorID):
    return 0 # count

def getSwitchSensorCount(switchSensorID):
    return 0 # count

def getJointSensor(*arg):
    pass 

def getAnySensor(*arg):
    return 0

def getLastMaxStepPWM(sensorIndex):
    return (0, 0) # stepID, maxPWMValue

def getJointPWMDuty(*arg):
    return 0

# Gets information regarding the current walk
# Returns a tuple of:
# (currentWalkType, currentStep, currentPG, leading_leg, frontShoulderHeight,
# bodyTilt, desiredPan, desiredTilt, desiredCrane, isHighGain)
# from actuator control
def getPWalkInfo(*arg):
    id(arg)
    return (0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#---------------------------------------
# Communcations functions

# Toggle sending of a particular robolink data type. Arg is an int corresponding
# to the data type required (SEND_DATA in Vision.h)
def sendData(*arg):
    pass

def sendEnvironmentTeamMates(*arg):
    pass

def sendCompressedCPlane(*arg):
    pass

def sendOPlane(*arg):
    pass

def sendEnvironmentBaseStation(*arg):
    pass

def sendYUVPlane():
    pass

def sendWalkInfo(elapsed):
    pass

def sendObjects():
    pass

# don't call this directly
def sendMyBehaviourInfo(*arg):
    pass

# Send a behaviour coordination message. Arg is a string (byte array) message
# which may contain nulls. Don't call this directly - use hTeam.sendTeamTalk
# whichs adds mandatory header info
def sendTeamTalk(*arg):
    pass
    
#---------------------------------------
# GPS Functions
    
def setIfGPSMotionUpdate(*arg):
    pass

def setIfGPSVisionUpdate(*arg):
    pass
# Pass true to keep gps opponent updating on, false to turn it off
def setIfGPSOpponentUpdate(*arg):
    pass
# Resets the edge pixel stationary map
def resetEdgeMap(*arg):
    pass
#For localisation challenge. Set to true if you want to record pink blobs.    
def setAllowPinkMapping(*arg):
    pass
# Sets the gps location to the position where a (hopefully) global maximum
# in field line match occurs. Takes no arguments, returns no arguments.
# You would usually run this after standing still and stationary localising,
# otherwise the line information won't be sufficient to get a good fix on
# your location.
def setGlobalMaxEdge(*arg):
    pass
#Returns true if gps used a pink blob to map / localise off (for localisation
#challenge)
def getUsePinkUpdate(*arg):
    pass

def getIfGPSVisionUpdate(*arg):
    pass

def setGPSPaused(*arg):
    pass

# Returns a WMObj of my location
def getGPSSelfInfo(*arg):
    id(arg)
    return me.getTuple()

def getGPSSelfCovariance(*arg):
    id(arg)
    return ((0,0,0),(0,0,0),(0,0,0))

def getGPSBallInfo(context):
    if context == Constant.CTGlobal:
        (myx, myy, myh) = (0, 0, 0)
    else:
        (myx, myy, myh) = getSelfLocation()
        myh -= 90

    (x, y) = (ball.getX() - myx, ball.getY() - myy)
    ang = hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(y, x)) - myh)
    return (x, y, math.sqrt(x*x + y*y), ang, ang - 90, 0)

# Gets GPS ball velocity, need to pass a Constant.CTGlobal or Constant.CTLocal
# argument.  Returns a tuple of (vel x[cm/frame], vel y[cm/frame],
# speed[cm/frame], vel angle [degrees], vel angle - 90[degrees], normalized
# angle [doesn't work])
def getGPSBallVInfo(*arg):
    id(arg)
    return (0, 0, 0, 0, -90, 0)

def getGPSBallMaxVar(*arg):
    return 0

#def getGPSVBallMaxVar(*arg):
#    return 0

def getGPSOpponentInfo(*arg):
    id(arg)
    return (0, 0)

def getGPSOpponentCovMax(*arg):
    return 0

def getGPSTeammateInfo(*arg):
    return (0, 0, 2000000000, 0, -1234)   # initial GPS values
#    (context, i) = arg
#    if context != Constant.CTGlobal:
#        print "Warning: Local context hack not yet implemented"
#        return None
#    return team[i-1].getTuple()

def getOppGoalInfo(*arg):
    #return WMObj(Constant.FIELD_WIDTH/2, Constant.FIELD_LENGTH, 0, 0, 0)
    pass

def getOwnGoalInfo(*arg):
    #return WMObj(Constant.FIELD_WIDTH/2, 0, 0, 0, 0)
    pass

def getGPSCoordArray():
    # assume red based
    return ((Constant.FIELD_WIDTH / 2, Constant.FIELD_LENGTH),\
        (Constant.FIELD_WIDTH / 2, 0),\
        (Constant.BEACON_LEFT_X, Constant.BEACON_FAR_Y),\
        (Constant.BEACON_RIGHT_X, Constant.BEACON_FAR_Y),\
        (Constant.BEACON_LEFT_X, Constant.BEACON_NEAR_Y),\
        (Constant.BEACON_RIGHT_X, Constant.BEACON_NEAR_Y)) 


def getSelfLocation():
    return (me.getX(), me.getY(), me.getHeading())#(me[0], me[1], me[3])

def getBallLocation():
    return (ball.getX(), ball.getY(), ball.getHeading())

def getTeamColor():     # Returns 0 for blue team, 1 for red
    return Constant.RED_TEAM

def getGPSTeammateBallInfo(*arg):
    pass

#---------------------------------------
# Vision functions
def getVisualObject(index):
    id(index)
    return [0]*15

def getColouredPixelCount(*arg):    # Not implemented
    pass

def getBallDistance(*arg):
    pass 

def getBallHeading(*arg):
    pass 

def getBallConfidence(*arg):
    pass 

# Set to false to indicate visualcortex should NOT look for beacons and goals
def setUseBeacons(*arg):
    pass 

# Call when grabbing so vision can use grab-specific vision code
# Automatically resets to false every frame if you don't call it
def setGrabbed(*arg):
    pass
    
def setGrabbing(*arg): 
    pass

#---------------------------------------
# Wireless functions
def getSharedBallInfo(*arg):
    id(arg)
    return (0, 0, Constant.LARGE_VAL, 0) # no shared ball

#Get teammates behaviours values, like isrAttacker, isDefender...
def getWirelessTeammateInfo(*arg):
    id(arg)
    return [0]*40 # FIXME
    
#---------------------------------------
# GameController functions
def getTheCurrentMode(*arg):        # see Constants.py
    pass
    
def getPenalised(*arg):             # get the penalised state
    pass  
    
def getDogPenalised(*arg):          # get the penalised state of a specified dog
    pass      
    
def getKickOffTeam(*arg):          # which team is kicking off
    pass
    
def getDropInTeam(*arg):            # the team that kicked the ball out
    pass

def getScore(*arg):                 # Score as (us, them) tuple
    pass

def getDropInTime(*arg):            # time passed since that happened
    pass    

def gameDataIsValid(*arg):          # 1 if game data receieved recently
    pass

# Call this to skip stationary localising, and
# disable robot demobilisation using head button
# (useful for challenges)
#def setMinimalMode(*arg):
#    pass
#---------------------------------------
# Other functions
def doBasicBehaviour(*arg):
    pass 

#return Behaviour::PLAYER
def getMyPlayerNum(*arg):
    return playerNum

#Get the project for ball. This returns the point on the field that is the
#projection of the top of the ball onto the field plane. It assumes
#that the robot can see the ball.
def getProjectedBall(*arg):
    pass
    
#what is it?
def printCPlusPlus(*arg):
    pass
    


def getBatteryCurrent(*arg):
    pass
    
#get dog's temperature    
def getTemperature(*arg):
    pass
#what is it?
def getCPUUsage(*arg):
    pass
    
def getCameraFrame(*arg): 
    pass

#what is it?
def getCurrentPreviousFrameID(*arg):
    id(arg)
    return (frame * 4, (frame-1) * 4)

# Sets time critical mode: some expensive computations will not be performed
def setTimeCritical():
    pass

# Clears time critical mode
def clearTimeCritical():
    pass

# Returns 1 if currently in time-critical mode, else 0
def isTimeCritical():
    pass
   
#---------------------------------------
# Wrapping functions that are using the old C++ codes.        

# Returns (min, max) heading to the best gap in the visual goal,
# or (0, 0) if none.
def getHeadingToBestGap():
    pass # return (min, max)

#equivalent to SystemTime in C++
def getCurrentTime():
    return (int(frame/30), (frame % 30) * (1.0/30.0)) 

#what is it?
def getProjectedPoint(*arg):
    pass

#set team color, 0 = blue, 1 = red
def setGPSGoal(*arg):
    pass
    
#send parameter in learned walk.
#EllipticalWalk --> "ELI ..."
#SkellipticalWalk --> "SKE ..."
def setWalkLearningParameters(*arg):
    pass
    
#def getRedBlobInfo(*arg):
#    pass
    
def PointToHeading(*arg):
    pass
    
def PointToElevation(*arg):
    pass 

def getTestingInfo(*arg):
    pass
    
def reloadNNMC():
    pass

def srand(*arg): 
    pass

def random(): 
    pass
    
def startProfile(*arg):
    pass
 
def stopProfile(*arg):
    pass
    
def profileTimeSoFar(*arg):
    pass

##### OBSTACLE FUNCTIONS #####

# return true (1) if local obstacles were transfered to GPS this frame
def ObstacleGPSValid():    
    return 1
    
# return the number of obstacle in box area. Args are topleftx, toplefty,
# botrightx, botrighty [, minobstacle, flags]
def getNoObstacleInBox(*arg):
    pass

# return the number of obstacles between two headings
def getNoObstacleInHeading(*arg):
    pass

# return the nearest point obstacles in dog coordinate
def getPointToNearestObstacleInBox(*arg):
    pass        

# return tuple (gapLeft, gapRight, bestHeading, gapAngle)
# where: 
#  gapLeft = heading to left side of best gap
#  gapRight = heading to right side of best gap
#  bestHeading = best heading to destination in gap
#  gapAngle = size of gap as angle
# or None if there is no gap.
# All values are in degrees where left is 90 and right is -90.
# Arguments are destX, destY - the desired kick location (local) -
# then optionally maxDist, minDist, minGapAngle, obstacleThreshhold, flags
# where only obstacles in (maxDist, minDist) are considered, the returned
# gap must be at least minGapAngle degrees and only obstacle boxes with at
# least obstacleThreshold obstacles will be considered.
def getBestGap(*arg):
    pass    
    
#return number of obstacle points in the corridor
#between two points    
def getNoObstacleBetween(*arg):
    pass
    
# return a number of green field features
def getFieldFeatureCount(): 
    return 10

def resetPinkMapping():
    pass

def initPinkObject():  
    pass
    
def resetGPS():
    pass

def setIfObstacleUpdate(*arg):
    pass

def resetBall(*arg):
    pass

def resetPinkUpdate(*arg):
    pass

