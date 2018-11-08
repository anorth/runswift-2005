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


# this module handles the common extreme cases that happen in the game
# and provides common resolution methods (other module could use their own handling methods)

import Global
import Constant
import Action

##########################################################################
# CASE: The robot is in the goal and face into the goal
# API functions: amIInAndFacingGoal() and inAndFacingGoalResolution()
##########################################################################
# this function detects whether the robot is in the goal and facing into the goal
# return False/Constant.dCLOCKWISE/Constant.dANTICLOCKWISE
def amIInAndFacingGoal():
    goals = [Global.vOGoal, Global.vTGoal]
    # If we can see either goal real big
    for goal in goals:
        if goal.isVisible() and \
               (goal.getWidth() > Constant.WIDTH_MOVE_THRESHOLD and \
                goal.getHeight() > Constant.HEIGHT_MOVE_THRESHOLD):
            if goal.getHeading() >= 0:
                return Constant.dCLOCKWISE
            else:
                return Constant.dANTICLOCKWISE
    else:
        return None


# resolution method in the dog is in the goal: turn on spot
def inAndFacingGoalResolution():
    direction = amIInAndFacingGoal()
    if direction == None:
        return
    print "InAndFacingGoal Resolution!"
    if direction == Constant.dCLOCKWISE:
        Action.walk(0, 0, -Action.MAX_TURN)
    elif direction == Constant.dANTICLOCKWISE:
        Action.walk(0, 0, Action.MAX_TURN)



##########################################################################
# CASE: One of the robot leg is stuck
# API functions: amIStuck()
##########################################################################
import VisionLink

#which leg gets stuck
LEG_FRONT_RIGHT = 1
LEG_FRONT_LEFT  = 2
LEG_BACK_RIGHT  = 3
LEG_BACK_LEFT   = 4

#a history of max PWM Duty values
STUCK_NUMBER_OF_FRAMES = 30 # 1 seconds
lastMaxPWMValues = [(0,0)] * STUCK_NUMBER_OF_FRAMES

def amIStuck():
    (maxPWM, leg) = max(lastMaxPWMValues)
    print maxPWM, leg
    if maxPWM >= 1300: # and Kick.framesSinceLastKicking >= 150:
        return leg
    else:
        return 0

def stuckFrameReset():
    global lastMaxPWMValues
    lastMaxPWMValues.pop(0)
    lastMaxPWMValues.append(getMaxPWMDuty())

#--------------------------------------
# Return numeric value.
# Get the maximum PWM values of current joints on the legs.
# When motor is jamed (something blocks the dog's way),
# the value should be much higher.
def getMaxPWMDuty():
    maxVal = 0
    whichLeg = None
    for i in range(3, Constant.NUM_OF_EFFECTORS): #ignore head pwm
        thisJointVal = abs(VisionLink.getJointPWMDuty(i+1))
        if (maxVal < thisJointVal):
            maxVal = thisJointVal
            whichLeg = i / 3
    return (maxVal, whichLeg)

import Indicator

# testing function
# this functions should NOT be called directly in any behaviours,
# amIStuck() should be called instead
def DecideNextAction():
    stuckFrameReset()
    Action.walk(Action.MAX_FORWARD, 0, 0)    
    leg = amIStuck()
    if leg == 0:
        Indicator.showHeadColor(Indicator.RGB_NONE)
        Indicator.showFacePattern([3,2,2,2,3])
    else:
        Indicator.showHeadColor(Indicator.RGB_YELLOW)
        Indicator.showFacePattern([0,0,0,0,0])        
        print "leg: ", leg
        if leg == LEG_FRONT_RIGHT or leg == LEG_BACK_RIGHT:
            Action.walk(0, Action.MAX_LEFT, 0)
        else:
            Action.walk(0, -Action.MAX_LEFT, 0)


##########################################################################
# CASE: The robot couldn't see any landmarks or ball for a long time
# API functions: amIBlind() & blindResolution()
##########################################################################
lastFrameSeenObjects = Global.frame

def blindFrameReset():
    global lastFrameSeenObjects
    if Global.seenBeacon:
        lastFrameSeenObjects = Global.frame
        return
    
    #Visual robots are left out,
    #because they are too easy to be misrecognised.    
    for obj in [Global.vBall, Global.vTGoal, Global.vOGoal]:
        if obj.isVisible():
            lastFrameSeenObjects = Global.frame
            return

def amIBlind():
    return Global.frame - lastFrameSeenObjects > \
           Constant.FRAME_RATE * 5    #5 seconds

def blindResolution():
    # well, turn around
    Action.walk(0, 0, Action.MAX_TURN)


##################################################################
# this function should get called in hFrameReset.py
def frameReset():
    blindFrameReset()
    stuckFrameReset()
