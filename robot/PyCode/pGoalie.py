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
#  $Id: pGoalie.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# Goalie.py
#
# Our rUNSWift 2004 Goal keeper
#
#===============================================================================

"""
Goal keeper
"""

import Action
import Constant
import Debug
import Global
import hGoalieHead
import hMath
import hTeam
import hTrack
import hWhere
import Indicator
import math
#import sGetBehindBall
import sGreenBackOff
#import sPawKick
import sUpennKick
import VisionLink
import sFindBall
import sBlock

# Run this on import to set that this is a Goalie
Global.isGoalieBehaviour = True



#===============================================================================
# Global variables of this module.
#===============================================================================

amAttacking = 0
amBlocking = 0
amLocalising = 0
WIRELESS_BALL_VAR = 50

# Coordinates of where goalie should stand.
FORWARD_OFFSET = 5
defendPos = [Constant.FIELD_WIDTH / 2, FORWARD_OFFSET, 90]

# Grace period for goalie actions.
# e.g. goalie still considers itself to be doing a
# given action MAX-1 frames after action has fired
AM_ATTACKING_MAX = 10
AM_BLOCKING_MAX = 1
AM_LOCALISING_MAX = 3

# Variables used when turning toward the field.
getOutOfGoalCounter   = 0
getOutOfGoalDirection = None

# Where to stand to defend the goal
defendPt = 0
defendAngle = 0

# Direction to move/reach to block the ball
blockDir = None
# Frames until ball arrives
timeToBlock = 0
# Ball speed towards dog
ballSpeed = 0
# Frame number of last time we blocked
lastBlockTime = 0
#Y offset to avoid goal wall when moving back to position
avoidWallOffset = 0

gKickType = None
gSideAttackOffset = 0
gFrontAttackOffset = 0

#===============================================================================
# Functions belong to this module.
#===============================================================================
def DecideNextAction():
    global amAttacking, amBlocking, amLocalising
    global gSideAttackOffset, gFrontAttackOffset
    global gKickType

    #if Global.haveBall == 1:
    #    VisionLink.sendYUVPlane()
    #elif Global.lostBall == 1:
    #    VisionLink.sendYUVPlane()
    

    
    Action.standStill()
    Action.closeMouth()
        
    if amAttacking > 0:
        amAttacking = amAttacking - 1
        
    if amBlocking > 0:
        amBlocking = amBlocking - 1
        
    if amLocalising > 0:
        amLocalising = amLocalising - 1

    gSideAttackOffset = 10 + amAttacking * 0.5  
    gFrontAttackOffset = 5 + amAttacking * 1.5
    
    if Global.penaltyShot:
        gSideAttackOffset = 30 + amAttacking * 0.5
        gFrontAttackOffset = 25 + amAttacking * 1.5        
        
    Indicator.showFacePattern([0, 0, 0, 0, 0])
    
    bTurnOnSuperDebug = True
    
    # Activate green back off, if we need to. 
    if sGreenBackOff.perform(False) == Constant.STATE_EXECUTING:
        if Debug.goalieDebug:
            print "DecideNextAction: No green, backing off"
        Action.setHeadParams(0,0,0,Action.HTAbs_h)
        Indicator.showFacePattern([1,1,3,1,1])

    # Need to move toward the field ?
    elif shouldITurnTowardTheField():
        if Debug.goalieDebug:
            print "DecideNextAction: Turning toward the field!"
        Action.walk(0, 0, getOutOfGoalDirection, minorWalkType = Action.SkeFastForwardMWT)
        moveAngleToMiddle()
        moveAngleToMiddle()
     
    # Should I sidestep or block a shot?
    elif checkThenBlock():
        amBlocking = AM_BLOCKING_MAX
        if Debug.goalieDebug:
            print "DecideNextAction: Blocking Ball"
        bTurnOnSuperDebug = False
        
    # Should I kick now?
    elif shouldIKickBall():
        if Debug.goalieDebug:
            print "DecideNextAction: Kicking", gKickType
        findDefendPtAngle(Global.ballX, Global.ballY)
        Action.kick(gKickType)

    #ball behind me?
    elif ballCloserToGoalThanMe():
        emergencyGetBack()
        if Debug.goalieDebug:
            print "DecideNextAction: Emergency Get Behind"
            
    # Want to approach the ball?
    elif shouldIAttackBall():
        amAttacking = AM_ATTACKING_MAX
        if Debug.goalieDebug:
            print "DecideNextAction: Attacking"
        findDefendPtAngle(Global.ballX, Global.ballY)
        attackBall()

    # Want to move back and defend.
    else:
        if Debug.goalieDebug:
            print "DecideNextAction: Move back and defend"
        defendGoal()

    if hGoalieHead.DecideNextAction(amAttacking or amBlocking):
        amLocalising = AM_LOCALISING_MAX

    if bTurnOnSuperDebug:
        # Show debug information using head lights.
        Indicator.superDebug()

##     if Global.vYellowPink.isVisible() or Global.vPinkYellow.isVisible():
##         Indicator.showHeadColor(Indicator.RGB_YELLOW)
##     else:
##         Indicator.showHeadColor(Indicator.RGB_BLUE)

##     selfH = Global.selfLoc.getHeading()
##     if 225 <= selfH < 315:
##         Indicator.showHeadColor(Indicator.RGB_PURPLE)
##     elif 135 <= selfH < 225:
##         Indicator.showHeadColor(Indicator.RGB_GREEN)
##     elif (selfH >= 315 or selfH < 45):
##         Indicator.showHeadColor(Indicator.RGB_YELLOW)
##     else:
##         Indicator.showHeadColor(Indicator.RGB_WHITE)
        
    sendGoalieWirelessInfo()

    
def ballCloserToGoalThanMe():

    #I can see my own goal and can see ball? 
    
    return False
    return Global.ballY < Global.selfLoc.getPos()[0] - 5
    
def emergencyGetBack():
    #left = 0
    #if Global.BallX <  
    Action.walk(-Action.MAX_FORWARD,0,0)
        
def moveAngleToMiddle(iteration = 1):
    global defendAngle, defendPt
    for _ in xrange(iteration):
        if defendAngle < 0:
            defendAngle += 0.5
        else:
            defendAngle -= 0.5
        if defendPt < 0:
            defendPt += 0.1
        else:
            defendPt -= 0.1


# Defending when not seeing the ball.
def defendGoal():
    global WIRELESS_BALL_VAR

    # Defending when not see vision ball = green.
    Global.myRole = Constant.GOALIEDEFENDNOTS
    
    # Last known vision position.
    if Global.lostBall < 5:
        if Debug.goalieDefendDebug:
            print "lostBall < 5 case!"
        if Global.lostBall == 0:
            Global.myRole = Constant.GOALIEDEFENDSEEB
        
        findDefendPtAngle(Global.ballX, Global.ballY)
    
    # Wireless case.
    #elif Global.sharedBallVar < hMath.get95var(WIRELESS_BALL_VAR):
    #    if Debug.goalieDefendDebug:
    #        print "Wireless ball case!"
    #    findDefendPtAngle(Global.sharedBallX, Global.sharedBallX)
        
    # GPS ball now. 
    elif (Global.lostBall <= Constant.VBALL_LOST
            and abs(Global.gpsLocalBall.getHeading()) < 90):
        if Debug.goalieDefendDebug:
            print "GPS ball case!"
        findDefendPtAngle(Global.ballX, Global.ballY)
        
    # Default position.
    else:
        if Debug.goalieDefendDebug:
            print "I am confused, moving back to the original position!"
        # use previous angles, but decay back to looking straight out over time
        moveAngleToMiddle()
    
    findDefendPosn()
    moveToDefendPos()
    
# Moves the robot to the defend position. Stop the robot if it is in position.
def moveToDefendPos():
    global defendPos, avoidWallOffset, amLocalising
    
    #don't adjust position while localising
    if amLocalising:
        return
    
    #don't adjust position in a penalty shot
    #if the ball is just sitting in own goalbox
    #.... you might nudge it in!     
    #if Global.penaltyShot and hWhere.ballInOwnGoalBox():
    #    return
    
    myPos = Global.selfLoc.getPos()
    
    diffX = defendPos[0] - myPos[0]
    diffY = defendPos[1] - myPos[1]
    diffH = defendPos[2] - Global.selfLoc.getHeading()
          
    # offset y when coming from side of goal
    # NOTE: this is a hard point change, don't 
    # think it will cause odd behaviour in current 
    # context but maybe look here first if odd 
    # stuff starts happening    
    # DISABLING TILL GOALIE LOCALISATION BETTER
#     if avoidWallOffset > 0:
#         avoidWallOffset -= 2
#     if avoidWallOffset < 0:
#         avoidWallOffset = 0
#     if (myPos[0] < Constant.LEFT_GOAL_POST \
#       or myPos[0] > Constant.RIGHT_GOAL_POST) \
#       and (myPos[1] < Constant.GOAL_LENGTH + 5):
#         if Debug.goalieDebug:
#             print "Altering course to avoid goal wall."
#         avoidWallOffset = Constant.GOALBOX_DEPTH
        
    if not (abs(diffX) <= 3 and abs(diffY) <= 3 and abs(diffH) <= 5):
        saGoToTargetFacingHeading(defendPos[0], defendPos[1] + avoidWallOffset, defendPos[2])


# Return true if I want to kick the ball.    
def shouldIKickBall():
    global gKickType
    global gSideAttackOffset
    
    selfH = Global.selfLoc.getHeading()
    selfX = Global.selfLoc.getX()

    # Facing backwards, be careful
    if 225 <= selfH < 315:        
        gKickType = None
        return False
##         # Don't kick if we are facing our own goal
##         # FIXME: we need a safe grab/turn here        
##         goalboxLeft = (Constant.FIELD_WIDTH - Constant.GOALBOX_WIDTH)/2
##         goalboxRight = (Constant.FIELD_WIDTH + Constant.GOALBOX_WIDTH)/2
##         if goalboxLeft-gSideAttackOffset <= selfX <= goalboxRight + gSideAttackOffset:
##             gKickType = None
##             return False
##         # If facing backwards but not in front of the goal, UPenn to sideline
##         elif selfX < goalboxLeft-gSideAttackOffset:
##             gKickType = Action.UpennLeftWT
##             return True
##         else:
##             gKickType = Action.UpennRightWT
##             return True
    
    # If facing across the field UPenn forwards
    elif 135 <= selfH < 225 and sUpennKick.isUpennLeftOk():       
        if Debug.goalieDebug:
            print "ShouldIKickBall(): Left UPenn."
        gKickType = Action.UpennLeftWT
        return True
 
    # ... and the other side
    elif (selfH >= 315 or selfH < 45) and sUpennKick.isUpennRightOk():
        if Debug.goalieDebug:
            print "ShouldIKickBall(): Right UPenn."
        gKickType = Action.UpennRightWT
        return True

    # If facing forwards then UPenn away from the centreline
    elif 45 <= selfH < 135:
        if selfX < Constant.FIELD_WIDTH/2 and sUpennKick.isUpennRightOk():
            if Debug.goalieDebug:
                print "ShouldIKickBall(): Right UPenn."
            gKickType = Action.UpennRightWT
            return True
        # ... and the other side
        elif sUpennKick.isUpennLeftOk():
            if Debug.goalieDebug:
                print "ShouldIKickBall(): Left UPenn."
            gKickType = Action.UpennLeftWT
            return True
    else:
        gKickType = None
        return False
    


def checkThenBlock():
    global amAttacking
    
    #if Global.penaltyShot:
    #    #loosen blocking criteria for penalty goalie
    #    return sBlock.checkThenBlock(onlyVisualBall = False, \
    #        minBallSpeed = 0.3, minDist = 10, maxDist = 90)   
    #else:
    
    if amAttacking > 0:
        return False
    
    return sBlock.checkThenBlock(onlyVisualBall = False, maxDist=200)


    
# Returns true if the goalie should attack the ball.
def shouldIAttackBall():
    global amAttacking
    global gSideAttackOffset, gFrontAttackOffset
    
    #if Global.penaltyShot:
    #    #do not attack ball if penalty goalie
    #    return False
       
    ## Immediate danger cases.
    if not hTrack.canSeeBall():
        if Debug.goalieDebug:
            print "No attack because can't see the ball!"
        return False

    if hTeam.hasTeammateGrabbedBall():
        if Debug.goalieDebug:
            print "Not attacking because my teammate is doing it!"
        return False
    
    # if attacker(s) have ball in own goal
    # then lessen criteria for attacking
    if attackerHasBallInOwnGoalbox():
        gFrontAttackOffset += 20 
        gSideAttackOffset += 20 
        
    # If you have gone too far, don't attack.
    if not hWhere.selfInOwnGoalBox(gFrontAttackOffset, gSideAttackOffset):
        if Debug.goalieDebug:
            print "No attack because I am not in the goal box (too far)!"
        return False
          
    # Don't attack unless ball is in own goalbox
    if not hWhere.ballInOwnGoalBox(gFrontAttackOffset, gSideAttackOffset):
        if Debug.goalieDebug:
            print "Ball not in the goal box!"
        return False

    if Debug.goalieDebug:
        print "Attacking the ball!"
        
    return True
    
def attackerHasBallInOwnGoalbox():
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]
        if mate.getRole() == Constant.ATTACKER:
            bx, by = mate.getBallX(), mate.getBallY()
            if hWhere.inGoalBox(bx, by):
                return True
    
    return False


#-------------------------------------------------------------------------------

# Makes the dog approach the ball (will kick if it is very close).
# We decide on the approach once and then execute it until we are no longer
# attacking.
def attackBall():

    # Goalie attack = red.
    Global.myRole = Constant.GOALIEATTACK
    Indicator.showFacePattern([0, 2, 1, 2, 0])

##     if Global.penaltyShot:
##         #i am taking out goalie attacking in penaltyshot... too risky
##         #but i'll leave this in here in case it's turned back on
##         sFindBall.walkToBall(Global.ballD, Global.ballH, sFindBall.GET_BEHIND_ONLY)
##     else:
        
    sFindBall.walkToBall(Global.ballD, Global.ballH, sFindBall.GET_BEHIND_GOALIE)

    return 


        
# Send the data to other forwards.
def sendGoalieWirelessInfo():
    sendBallDistOrKickOff = Constant.MAX_TIME_TO_REACH

    # Does goalie need to send haveBall signal?
    
    hasSeenBall = False
                    
    # If I can see the ball, send the ball distance I calculated. If I can't
    # see the ball, do nothing as sendBallDist is initalised as LOSTBALL_TIME.
    if hTrack.canSeeBall():
        sendBallDistOrKickOff = Global.ballD
        hasSeenBall = True

    if Global.state == Constant.READYSTATE or Debug.mustReady:
        sendBallDistOrKickOff = Global.kickOffState

    hTeam.sendMyBehaviourInfo(sendBallDistOrKickOff,\
                              False,
                              hasSeenBall,
                              False,\
                              Constant.GOALIE)

##############################################                                   
## Turning toward the soccer field.
##############################################                                   

WIDTH_MOVE_THRESHOLD  = 203
HEIGHT_MOVE_THRESHOLD = 155

def shouldITurnTowardTheField():  
    global getOutOfGoalCounter, getOutOfGoalDirection
    
    # If goalie has a visual ball let walkToBall handle it
    #if Global.lostBall < 6:
    #    return False

    ownGoal = Global.vOGoal
    turnCCW = -1
    
    # If we can see our own goal real big or we've got our face in either goal
    # or we're locked into a get-out-of-goal routine.
    if (ownGoal.isVisible() and \
        (ownGoal.getWidth() >  WIDTH_MOVE_THRESHOLD or \
         (ownGoal.getHeight() > HEIGHT_MOVE_THRESHOLD and \
          ownGoal.getWidth() > Constant.IMAGE_WIDTH / 2))) or \
          getOutOfGoalCounter > 0:
        if getOutOfGoalDirection == None:
            if ownGoal.getHeading() >= 0:
                turnCCW = -Action.MAX_TURN
            else:
                turnCCW = Action.MAX_TURN
            getOutOfGoalDirection = turnCCW
        else:
            turnCCW = getOutOfGoalDirection
            
        Action.walk(0, 0, turnCCW, minorWalkType=Action.SkeFastForwardMWT)
        Action.setHeadParams(0, 0, 10, Action.HTAbs_h)
        getOutOfGoalCounter += 1
        
        MAX_INFRA_RED = 90.0
        irFar = VisionLink.getAnySensor(Constant.ssINFRARED_FAR)
        
        if getOutOfGoalCounter > 60 or Global.vTGoal.isVisible() or \
               (irFar / 10000.0) >= MAX_INFRA_RED or Global.seenBeacon:
            getOutOfGoalCounter = 0
        return True
    else:
        getOutOfGoalDirection = None
        return False

##############################################                                   
## Intercept calculating functions
##############################################
# Finds the defense position (relative to an origin in the centre of
# the goal mouth facing forwards).
def findDefendPtAngle(ballX, ballY):
    if ballX <= 0:
        ballX = 0.01
    if ballY <= 0:
        ballY = 0.01   
        
    if Debug.goalieDebug:    
        print "findDefendPtAngle: ball(", ballX, ",", ballY, ")"
        
    ballX = ballX - Constant.OWN_GOAL_X # ballX relative to goal centre

    global defendAngle, defendPt
    # It's not that complex, really. We have a defensive line a fixed distance
    # in front of the goal line. We draw a ray from the ball to the centre of
    # the goal mouth and stand at the intersection of the ray with the
    # line
    defendAngle = math.atan2(ballY, ballX)
    defendAngle = math.degrees(defendAngle) - 90.0
    defendAngle = hMath.normalizeAngle_180(defendAngle)                 


    defendAngle = hMath.CLIP(defendAngle, 90)

    global FORWARD_OFFSET
    if abs(defendAngle) < 45:
        FORWARD_OFFSET = 10
    else:
        FORWARD_OFFSET = 3
    
    defendAngle = hMath.CLIP(defendAngle, 60)

    defendPt = ballX - (ballX * ((ballY - FORWARD_OFFSET) / ballY))
    defendPt = hMath.CLIPTO(defendPt, -Constant.GOAL_WIDTH/2.0+5, Constant.GOAL_WIDTH/2.0-5)


    

def findDefendPosn():
    global defendPos
    global gSideAttackOffset
    
    #penalty goalie loses ball for a while
    #what to do? ... stay where you are!
    #if the ball is near you any move could kick it in
    #if Global.penaltyShot and Global.lostBall > 30:
    #    myLoc = Global.selfLoc
    #    defendPos = (myLoc.getX(), myLoc.getY(), Global.selfLoc.getHeading())
    #    return
    
    ptX = defendPt + Constant.OWN_GOAL_X
    ptY = FORWARD_OFFSET
    
    # Move back if ball is lost near us (might be behind us)
    # but only for a short time, then quit
    myx = Global.selfLoc.getX()
    if Global.lostBall >= 30 and Global.lostBall <= 120 \
        and Global.ballY < Constant.FIELD_LENGTH/4 \
        and Constant.LEFT_GOAL_POST + gSideAttackOffset < myx \
        < Constant.RIGHT_GOAL_POST - gSideAttackOffset:
        ptY = -10
    
    angle = defendAngle + 90.0
    radAngle = math.radians(angle)
    
    ptX += Constant.DOG_LENGTH/2*math.cos(radAngle)
    ptY += Constant.DOG_LENGTH/2*math.sin(radAngle)
    
    defendPos = (ptX, ptY, angle)
    


#--------------------------------------
# Move the point under the dog's neck to a location on the field
# Note that turnCCW is relative to the centre of the dog's body, not under its
# neck. This is a modified version of the standard function, it
# stops goalie from continually crashing into the edge of the goal.
DOG_RECT_LENGTH = 20.0
DOG_RECT_WIDTH  = 13.0

def saGoToTargetFacingHeading(targetX, targetY, targetH, \
        maxSpeed = Action.MAX_FORWARD, maxTurn = Action.MAX_TURN):

    # Variables relative to self localisation.
    selfPos  = Global.selfLoc.getPos()
    selfH    = Global.selfLoc.getHeading()
    cosSelf  = math.cos(math.radians(selfH))
    sinSelf  = math.sin(math.radians(selfH))
    
    relX     = targetX - selfPos[0]
    relY     = targetY - selfPos[1]
    relH     = hMath.normalizeAngle_180(targetH - selfH)
    # now take into account that as we turn, the position of our head will change
    relX     += Constant.DOG_LENGTH/2.0*(cosSelf - math.cos(math.radians(targetH)))
    relY     += Constant.DOG_LENGTH/2.0*(sinSelf - math.sin(math.radians(targetH)))
    relD     = hMath.getLength((relX, relY))

    # Calculate approximate coordinates of all the dog's legs
    legs = []
    for legNum in range(4):
        legPos = list(selfPos)
        
        if legNum >= 2:
            legPos[0] = legPos[0] - DOG_RECT_LENGTH * cosSelf
            legPos[1] = legPos[1] - DOG_RECT_LENGTH * sinSelf
            
        if (legNum % 2) == 0:
            legPos[0] = legPos[0] - DOG_RECT_WIDTH * sinSelf
            legPos[1] = legPos[1] + DOG_RECT_WIDTH * cosSelf 
        else:
            legPos[0] = legPos[0] + DOG_RECT_WIDTH * sinSelf
            legPos[1] = legPos[1] - DOG_RECT_WIDTH * cosSelf
            
        legs.append(tuple(legPos))
    
    # If a leg coordinate lies within the rear wall, clip the Y movement
    clipMove = False
    for legPos in legs:
        if abs(legPos[0] - Constant.FIELD_WIDTH / 2) > Constant.GOAL_WIDTH and\
            legPos[1] < 2:
            clipMove = True
            
    if clipMove and (relY < 0):
        relY = 0
    
    if relX == 0 and relY == 0:
        #on the dog, math.atan2(0,0) give "Value Error: math domain error"
        relTheta = 0
    else:
        relTheta = hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(relY, relX)) - selfH)
    
    forward  = hMath.CLIP(relD, maxSpeed) * math.cos(hMath.DEG2RAD(relTheta))
    left     = hMath.CLIP(relD, maxSpeed) * math.sin(hMath.DEG2RAD(relTheta))
    turnCCW  = hMath.CLIP(relH, maxTurn)


    Action.walk(forward, left, turnCCW, minorWalkType = Action.SkeFastForwardMWT)







#################################################################################################
# the below functions are currently obsolete
#################################################################################################

###------------------------------------------------------###
### Ball velocity calculation. Currently Goalie specific ###

lasty = 0       # last ball y pos
lastx = 0       # last ball x pos
lastpan = 0     # last head pan command
BALL_HIST = 10
balldy = [0] * BALL_HIST # a list of 10 previous ball pos deltas
balldx = [0] * BALL_HIST

def updateBallV():
    global lastx, lasty, lastpan
    balldy.pop()
    balldx.pop()
    if Global.haveBall > 1 and abs(lastpan - Global.pan) < 10:
        balldy.insert(0, Global.gpsLocalBall.getY() - lasty)
        balldx.insert(0, Global.gpsLocalBall.getX() - lastx)
    else:
        balldy.insert(0, 0)
        balldx.insert(0, 0)
    if hTrack.canSeeBall():
        lasty = Global.gpsLocalBall.getY()
        lastx = Global.gpsLocalBall.getX()
    lastpan = Global.pan
    
def getBallV():
    dy = sum(balldy)
    dx = sum(balldx)
    return (dx / BALL_HIST, dy / BALL_HIST)


def isGoalieUPrightOk():
    if hTrack.canSeeBall():
        ballx = math.sin (hMath.DEG2RAD(Global.ballH)) * Global.ballD
        bally = math.cos (hMath.DEG2RAD(Global.ballH)) * Global.ballD
                
        if (ballx < 3.5 and ballx > -5.5 and
            bally < 4.4 * Constant.BallRadius and
            Global.ballD < 18 and
            Global.tilt < -9 and abs(Global.pan) < 30):
            return True            
            
    return False
    
def isGoalieUPleftOk():
    if hTrack.canSeeBall():
        ballx = math.sin (hMath.DEG2RAD(Global.ballH)) * Global.ballD
        bally = math.cos (hMath.DEG2RAD(Global.ballH)) * Global.ballD
        
        if (ballx < 3.0 and ballx > -5.0 and
            bally < 4.4 * Constant.BallRadius and
            Global.ballD < 18 and
            Global.tilt < -9 and abs(Global.pan) < 30):
            return True
            
    return False


# Returns true if the ball is approaching and the goalie should take immediate
# action to block it
def shouldIBlockBallOrig():
    global blockDir, timeToBlock, ballSpeed, lastBlockTime

    # Don't block if we can't see the ball
    if Global.haveBall < 3:
        return False

    # Don't block immediately after dive-blocking
    if (Global.frame - lastBlockTime) < 90:
        return False

    # Don't block a ball that is really close. If it's moving, then it's too
    # late. If it's still we should attack.
    if Global.ballD < 25:
        return False

    (dx, dy, speed, _e, _e, _e) = VisionLink.getGPSBallVInfo(Constant.CTLocal)
    #updateBallV()
    #(dx, dy) = getBallV()
#    if Global.frame % 3 == 0:
#        print "dx", ("%.2f" % dx), "dy", ("%.2f" % dy)

    # If heading away or too slow then ignore
    if dy >= -1.2:
        return False

    # Don't try to block a ball going across our front
    if abs(dx) > abs(dy):
        return False

    # Stop dividing zero.
    if dx == 0:
        dx = 0.001
        
    # Plot ball local path as y = mx + b and get local x intercept
    #m = dy / dx
    #b = Global.gpsLocalBall.getY() - Global.gpsLocalBall.getX() * m
    #xInt = -b / m
    
    xInt = Global.gpsLocalBall.getX()
    
    if not (-10 > xInt > -50 or 10 < xInt < 50):
        #print "Ball intercepting too wide (", xInt, ")"
        return False

    print "Ball approaching", dy, "cm/frame"
    
    # If ball will arrive soon return true
    if Global.ballD <= abs(dy) * 60:
        if xInt < 0:
            blockDir = Constant.LEFT
            print "Ball coming to left",
        else:
            blockDir = Constant.RIGHT
            print "Ball coming to right",
        ballSpeed = abs(dy)
        timeToBlock = Global.gpsLocalBall.getY() / ballSpeed
        print "in", timeToBlock, "frames"
        return True
    else:
        return False


# Block the ball either by strafing sideways or reaching out
def blockBall():
    global lastBlockTime
    if timeToBlock < 25 and ballSpeed > 3.0:
        lastBlockTime = Global.frame
        if blockDir == Constant.LEFT:
            Indicator.showFacePattern([0, 0, 0, 0, 3]);
            Action.kick(Action.BlockLeftWT)
            Action.forceStepComplete()
        else:
            Indicator.showFacePattern([3, 0, 0, 0, 0]);
            Action.kick(Action.BlockRightWT)
            Action.forceStepComplete()
    else:
        if blockDir == Constant.LEFT:
            Indicator.showFacePattern([0, 0, 0, 0, 2]);
            Action.walk(0,Action.MAX_LEFT_NORMAL,0,minorWalkType = Action.SkePG22MWT)
        else:
            Indicator.showFacePattern([2, 0, 0, 0, 0]);
            Action.walk(0,-Action.MAX_LEFT_NORMAL,0,minorWalkType = Action.SkePG22MWT)
            
# Move sideways to block the ball. FIXME - get behind ball
def sideBlockBall():
    pass
#    if 10 <= Global.ballH < 160:
        # None as not overwriting forward/turnCCW/walkParams
#        Action.walk(None, 3, None, None) 
        #Indicator.showHeadColor(Indicator.RGB_YELLOW)
        #print Global.frame, " side left blocking",
#    elif -160 < Global.ballH <= -10:
#        Action.walk(None, -3, None, None)
        #Indicator.showHeadColor(Indicator.RGB_YELLOW)
        #print Global.frame, " side right blocking"    


def findOpenAngle(ptX, ptY, ballX, ballY, headPoint = False):

    # print "pX, pY, bX, bY: " , ptX, ptY, ballX, ballY

    if ballY < 0:
        ballY = 0
    
    if ptY > ballY: # we don't want to choose a point on the wrong side of the ball
        return Constant.LARGE_VAL
    
    ptX -= Constant.OWN_GOAL_X
    ballX -= Constant.OWN_GOAL_X
    
    relX = ballX - ptX
    relY = ballY - ptY
    dist = math.sqrt(relX*relX + relY*relY)
    
    # print "relX, relY, dist: " , relX, relY, dist
    
    if not headPoint:
        dist -= Constant.DOG_LENGTH/10.0
    
    GoalPostLeft = -Constant.GOAL_WIDTH / 2.0
    GoalPostRight = Constant.GOAL_WIDTH / 2.0
    
    GoalY = -Constant.BallRadius
    
    # if the ball is outside the line of the posts
    # then leave a small gap (ball radious) to the goal post
    if ballX < GoalPostLeft:
        GoalPostLeft += Constant.BallRadius
    elif ballX > GoalPostRight:
        GoalPostRight -= Constant.BallRadius
    
    # print "leftGw, rightGw, GoalY" , GoalPostLeft, GoalPostRight, GoalY
    
    blockAngle = math.atan2(Constant.DOG_WIDTH/10.0, dist)
    ballAngle = math.atan2(relX, relY)
    
    leftSize = ballX - GoalPostLeft
    rightSize = ballX - GoalPostRight
    
    leftAngle = math.atan2(leftSize, ballY - GoalY)
    rightAngle = math.atan2(rightSize, ballY - GoalY)
    
    # print "ballX, leftSz, rightSz" , ballX, leftSize, rightSize
    
    # print "ball, block, leftA, rightA: " , math.degrees(ballAngle), math.degrees(blockAngle), math.degrees(leftAngle), math.degrees(rightAngle)
    
    thetaL = leftAngle - ballAngle - blockAngle
    thetaR = ballAngle - rightAngle - blockAngle
    
    # print "L, R: " , math.degrees(thetaL), math.degrees(thetaR)
    
    if thetaL > thetaR:
        return thetaL
    else:
        return thetaR
