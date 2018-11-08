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


import Action
import Constant
import Debug
import Global
import hMath
import Indicator
import sFindBall
import sGrab
import sGrabDribble
import VisionLink



#########################
# DEBUG FLAGS
#########################
headColor = Debug.doNothing
#headColor = Indicator.showHeadColor

SEND_DEBUG_INFO_TO_ROBOLINK = False

#########################
# Global variables
#########################
gLastFrameCalled = 0
gtkGenerator = None
selectedGap = None                  # LEFT or RIGHT
isAimToGoal = True
aimCounter = 0                      # counts frames with good aim
targetX = Constant.TARGET_GOAL_X    # by default, aim to target goal
targetY = Constant.TARGET_GOAL_Y
gKickType = Action.HeadTapWT
frameSinceLastGrabKick = -100
turnSpeed = None

# Degrees to turn past the goal without seeing it before giving up on 
# visual aim. Note that this interplays with the kick we choose afterwards:
# currently turn kick so we need to be a fair bit past the goal for this
# to be good
TURN_PAST_GOAL = 70

# work alone function
def DecideNextAction():
    global frameSinceLastGrabKick 
    if sGrab.isGrabbed: 
        if perform() != Constant.STATE_EXECUTING: 
            sGrab.resetPerform()
            frameSinceLastGrabKick = Global.frame
            resetPerform()
    elif Global.frame - frameSinceLastGrabKick > 60:        
        sGrab.perform()
    else:
        sFindBall.perform()
    
    

#####################################
# Main driver function
#####################################

startTimeStamp = VisionLink.getCurrentTime()

def resetPerform():
    global gtkGenerator, turnSpeed, hasSeenGoal, aimCounter, selectedGap
    gtkGenerator = None
    turnSpeed = None
    headColor(Indicator.RGB_NONE)
    aimCounter = 0
    selectedGap = None


def perform(aimToGoal = True, kickType=Action.HeadTapWT, \
            tX = Constant.TARGET_GOAL_X, tY = Constant.TARGET_GOAL_Y):
    global gLastFrameCalled
    global gtkGenerator
    global isAimToGoal, targetX, targetY, gKickType
    
    if not sGrab.isGrabbed: 
        resetPerform() 
        return Constant.STATE_FAILED
    elif not sGrab.isBallUnderChin():
        sGrab.resetPerform() 
        resetPerform()
        return Constant.STATE_FAILED
    
    if gLastFrameCalled != Global.frame - 1:         
        resetPerform()
    gLastFrameCalled = Global.frame
    
    if gtkGenerator == None:
        isAimToGoal = aimToGoal
        if not isAimToGoal:
            targetX = tX
            targetY = tY            
        gKickType = kickType
        gtkGenerator = gtkGeneratorFunc()
        
    try:
        Global.lostBall = 0
        gtkGenerator.next()
    except StopIteration:
        resetPerform()
        return Constant.STATE_SUCCESS
                
    return Constant.STATE_EXECUTING
        

def gtkGeneratorFunc():
    global frameSinceLastGrabKick, aimCounter

    # Ok, we've grabbed the ball, now spin towards the target 
    aimCounter = 0

    #print "*** GTK"

    # if we have enough time to do turn/side kick
    while hMath.getTimeElapsed(sGrab.gLastGrabTime, VisionLink.getCurrentTime()) < Constant.GRAB_TIMEOUT:
#        print "Vframe", Global.frame,

        isLinedUp = False
        # now decide what to do
        #print "lining up,", 
        if isAimToGoal:
            isLinedUp = performAimToGoal()
        else:
            isLinedUp = performNotAimToGoal() 

        # If visual aim fails (we turn too far) stop now
        if isLinedUp == Constant.STATE_FAILED:
            break
        
        # Im lined up, so kick!
        # DON'T wait for step complete
        if isLinedUp == Constant.STATE_SUCCESS:
            #print "kicking"
            Action.forceStepComplete()
            Action.kick(gKickType)
            while Action.shouldIContinueKick():
                Action.continueKick()
                yield True
            for _ in range(sGrabDribble.NU_FWD_DURATION):
                yield True
                          
            frameSinceLastGrabKick = Global.frame  
            sFindBall.setHint(200, 90)  
            sFindBall.perform()
            Action.closeMouth()
            resetPerform()                  
            return

        # else line up still executing
        if SEND_DEBUG_INFO_TO_ROBOLINK:
            sendDebugInfo()
        
        yield True
    
    #print "timeout or turned too far...",

    ### The line-up has failed
    minH, maxH = selectGap(True)   
    if minH < 0 < maxH:
        #print "shooting"
        Action.forceStepComplete()
        Action.kick(gKickType)
        while Action.shouldIContinueKick():
            Action.continueKick()
            yield True
        for _ in sGrabDribble.NU_FWD_DURATION:
            yield True
                            
        frameSinceLastGrabKick = Global.frame  
        sFindBall.setHint(200, 90)
        sFindBall.perform()
        Action.closeMouth()
        resetPerform()        
        return 
   
    sFindBall.setHint(20, 90)   # directly in front
    sFindBall.perform()
    Action.closeMouth()
    resetPerform()
    sGrab.resetPerform()
    return
        

HEADING_ANGLE_MULTIPLE = 1
WAIT_FOR_AIM = 3

# Grab-turns to aim for the goal, relying on seeing the goal. Returns 
# Constant.STATE_SUCCESS when lined up, STATE_EXECUTING when still going and
# STATE_FAILED if we have turned well past the goal without seeing it.
def performAimToGoal():
    global aimCounter
    if Global.vTGoal.isVisible(): # if we can see the goal

        # Choose the gap, locking it in if we're lining up
        minH, maxH = selectGap(aimCounter > 0)

        gapWidth = abs(maxH - minH)
        padding = gapWidth / 5.0

        #print "vGoal, bestgap", minH, maxH,

        if minH < -padding and maxH > padding:
            aimCounter += 1 # count frames of good aim
            # DON'T reset this to zero if we're outside the gap. reset->deadlock
            #print "++aimcounter to", aimCounter,

            # A forceStepComplete here makes the kick more reliably aim to
            # hit the goal but hinders further lining up to the centre of the
            # gap and any forward walking. Trade-off.
            Action.forceStepComplete()

        
        if aimCounter >= WAIT_FOR_AIM:
            # if inside the best gap, shoot
            #print "lined up good,",
            aimCounter = 0
            headColor(Indicator.RGB_GREEN)
            return Constant.STATE_SUCCESS
        else:
            # turn to the middle of the gap
            turnAmount = (minH + maxH) / 2.0
            #print "turning", turnAmount,
            # TODO: use another kind fast short step walk?
            #if Global.selfLoc.getY() > Constant.FIELD_LENGTH * 3 / 4:
            #    grabSideWalk(turnAmount)
            #else:
            sGrabDribble.perform(0, 0, turnAmount)


    else: # if we can NOT see the goal
        global turnSpeed
        heading = hMath.getHeadingToFaceAt(Global.selfLoc,\
                                           Constant.TARGET_GOAL_X,\
                                           Constant.TARGET_GOAL_Y)
        if not turnSpeed:   # Choose direction to spin
            if heading > 0:
                turnSpeed = 80 
            else: 
                turnSpeed = -80                                            
        #else:               # Check we haven't gone to far
            #if turnSpeed > 0 and heading < -TURN_PAST_GOAL:
            #    return Constant.STATE_FAILED
            #elif turnSpeed < 0 and heading > TURN_PAST_GOAL:
            #    return Constant.STATE_FAILED

        #print "no vGoal, turning", turnSpeed,
        sGrabDribble.perform(0, 0, turnSpeed)

    return Constant.STATE_EXECUTING

def performNotAimToGoal():
    # if we are not aiming to the goal, use GPS
    heading = hMath.getHeadingToFaceAt(Global.selfLoc, \
                                       targetX, targetY)
    if canIKick(heading, gKickType):
        return Constant.STATE_SUCCESS
    else:
        sGrabDribble.perform(0, 0, heading * HEADING_ANGLE_MULTIPLE )
        return Constant.STATE_EXECUTING


def sendDebugInfo():
    VisionLink.sendAtomicAction(*tuple(Action.finalValues))
    VisionLink.sendWalkInfo(hMath.getTimeElapsed(startTimeStamp, VisionLink.getCurrentTime()))
    VisionLink.sendObjects()
    #VisionLink.sendYUVPlane()    
        


# case 1: if i am lost, don't kick
# case 2: if i am NOT in the right angle range for that kick, don't kick.
# case 3: if i can visually see own goal, DO NOT dive kick.
# NOT IMPLMENTED FULLY YET.
def canIKick(heading, kickType):
    if kickType == Action.UpennRightWT:
        return 70 < heading < 90
    elif kickType == Action.UpennLeftWT:
        return -90 < heading < -70
    else:   # forward kicks
        return abs(heading) < 5 and not Global.vOGoal.isVisible()
        #raise Exception("sGrabTurnKick: canIKick(): I don't know this kickType!!")
   


#########################
# Helper functions
#########################

#MIN_TURN_SPEED = 5
MAX_TURN_SPEED = 90

# Turn on the spot while grabbed. Optionally add a forward component to the walk
    #def grabTurnWalk(speed, forward = 0):
#    Global.forceHighGain = True
#    headColor(Indicator.RGB_PURPLE)    
#    if speed != 0:
#        speed = hMath.CLIP(speed, MAX_TURN_SPEED)
#        #speed = hMath.EXTEND(speed, MIN_TURN_SPEED)
#    
#    Action.walk(forward,0,speed,walkType=Action.NormalWalkWT,minorWalkType=Action.GrabTurnOnlyMWT)
#    sGrab.moveHeadForward()
#    Action.openMouth()
#    
#    # Set findBall hint in case we lose the ball out of the grab-turn
#    if (speed < 0):
#        sFindBall.setHint(25, 20)
#    else:
#        sFindBall.setHint(25, 160)

SIDE_STEP_MAX = 8

def grabSideWalk(direction):
    if direction > 0:
        left = SIDE_STEP_MAX
    else:
        left = -SIDE_STEP_MAX
    headColor(Indicator.RGB_BLUE)        
    Action.walk(0,left,0,walkType=Action.NormalWalkWT,minorWalkType=Action.GrabSideOnlyMWT)
    
    sGrab.moveHeadForward()
    Action.openMouth()
    
    
# Choose which gap to use. If there is only one then that one. 
# If selectedGap is not None the it's that one, else the largest.
# If setSelected is True and selectedGap is None and there is more than
# one gap then sets selectedGap appropriately
def selectGap(setSelected = False):
    global selectedGap
    (lmin, lmax, rmin, rmax) = VisionLink.getHeadingToBestGap()
    if (rmin == rmax == 0):
        if selectedGap != None:
            VisionLink.sendYUVPlane()
            #print "gap selected but only one seen"
        return (lmin, lmax)
    elif (lmin == lmax == 0):
        if selectedGap != None:
            VisionLink.sendYUVPlane()
            #print "gap selected but only one seen"
        return (rmin, rmax)

    if selectedGap == Constant.LEFT:
        return (lmin, lmax)
    elif selectedGap == Constant.RIGHT:
        return (rmin, rmax)
    elif abs(lmax - lmin) > abs(rmax - rmin):
        if setSelected:
            selectedGap = Constant.LEFT
            #print "selectGap locking LEFT"
        return (lmin, lmax)
    else:
        if setSelected:
            selectedGap = Constant.RIGHT
            #print "selectGap locking RIGHT"
        return (rmin, rmax)

