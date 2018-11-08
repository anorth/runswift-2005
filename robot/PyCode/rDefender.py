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
#  $Id: DefenderHomer.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 

#===============================================================================
#   Python Behaviours : 2004 (c) 
#===============================================================================

"Defender"

import math
import Constant
import Global
import Indicator
import Action
import hMath
import hTrack
import hFWHead
import hWhere
import hStuck
import hTeam
import sGetOutOfTheWay
import sBirdOfPrey
import sFindBall

import sBlock


###############################################################
# Defender.
###############################################################
 
TARGET_PT_ACC_SMALL_CIRCLE = 10 # Hysterisis
TARGET_PT_ACC_LARGE_CIRCLE = 25
NORMAL_BACKOFF_YDIST       = 160

gTargetPointReqAccuracy    = TARGET_PT_ACC_SMALL_CIRCLE

gLastFrameCalled = 0

# We will bird to defender pos if in front by this much
BIRD_TO_POS_THRESH = 100

GET_OUT_DURATION = 75

# Reset these variables every frame.
def resetEachFrame():
    global gLastFrameCalled
    global gGetOutOfTheShotCounter
    global gGetOutOfTheAttackerCounter   
    global gGetAwayFromTheBallCounter 
    global gGetAwayFromTheSupporterCounter
    
    if gLastFrameCalled != Global.frame - 1: 
        gGetOutOfTheShotCounter = 0
        gGetOutOfTheAttackerCounter = 0        
        #gGetAwayFromTheBallCounter = 0
        gGetAwayFromTheSupporterCounter = 0
    gLastFrameCalled = Global.frame 


def DecideNextAction():
    Global.myLastRole = Constant.DEFENDER
    Global.myRole = Constant.DEFENDER
    Indicator.setDefault()
    hFWHead.resetEachFrame() 
    perform()   
    #findBallDefender()
    hFWHead.DecideNextAction()
 
def perform(params = None):
    global gTargetPointReqAccuracy
    global gBirdToBall#, gBirdPos
    id(params) # ignore
    
    resetEachFrame() 
    
    # Back off if stuck
    if hStuck.amIStuckForward() and not hWhere.selfInOwnGoalBox(15):
        hFWHead.DecideNextAction()
        return

    targetX, targetY = hWhere.getDefenderPos()          
    selfX, selfY = Global.selfLoc.getPos()

    hFWHead.DecideNextAction()

    # If the attacker has grabbed, get out of the way
    if hTeam.hasTeammateGrabbedBall():
        if getOutOfEveryonesWay():
            return

    # Continue BOP if already doing one
    if sBirdOfPrey.isBirdOfPreyTriggering():        
        if sBirdOfPrey.birdTo == sBirdOfPrey.BIRD_TO_BALL:
            birdPos = (Global.ballX, Global.ballY)
        else: # BIRD_TO_POS
            birdPos = (targetX, targetY)
        # If there are no robots between (x) us and the ball defender can use
        # a tighter curve
        birdAngle = 40
        for i in Global.otherValidForwards:
            if Global.teammatesLoc[i].getY() > Global.ballY \
                    and Global.ballX < Global.teammatesLoc[i].getX() \
                    < Global.selfLoc.getX():
                birdAngle = 45
            elif Global.teammatesLoc[i].getY() > Global.ballY \
                    and Global.selfLoc.getX() < Global.teammatesLoc[i].getX() \
                    < Global.ballX:
                birdAngle = 45
        if sBirdOfPrey.perform(birdPos[0], birdPos[1], birdAngle)\
                == Constant.STATE_EXECUTING:
            # Can we do better than this (supporter too)? Need to see obstacles
            hFWHead.minPosVar = 1   # localise more often
            hFWHead.DecideNextAction()
            return

    # Make sure we're not obstructing teammates. This is less important than
    # the bird, but hopefully DodgyDog in the bird will keep us clear.
    if getOutOfEveryonesWay():
        return
      
        
    # Bird back to position if I am way in front and not close to ball
    if targetY < selfY - BIRD_TO_POS_THRESH and Global.ballD > 70:
        sBirdOfPrey.birdTo = sBirdOfPrey.BIRD_TO_POS
        if sBirdOfPrey.perform(targetX, targetY) == Constant.STATE_EXECUTING: 
            return       
    
    # trigger BOP when I am certain distance in front of the ball, but not if 
    # defender position is in front of the ball anyway
    #DEFENCE_OFFSET = 10.0
    #DEFENCE_ANGLE  = 150.0
    #if targetY < Global.ballY < selfY and \
    #        sBirdOfPrey.areWeAboveTheLine(DEFENCE_OFFSET, DEFENCE_ANGLE, \
    #                                        True, Global.ballX, Global.ballY):
    #    sBirdOfPrey.birdTo = sBirdOfPrey.BIRD_TO_BALL
    #    if sBirdOfPrey.perform(Global.ballX, Global.ballY) == Constant.STATE_EXECUTING: 
    #        return       

    sBirdOfPrey.birdTo = None

    # If I have lost the ball for a while, then find the ball.
    if Global.ballSource == Constant.GPS_BALL and Global.lostBall > 150: 
        Indicator.showFacePattern([5,0,0,0,5])
        findBallDefender()
        return 

    doGetToTargetPoint(targetX, targetY)
        
    # Check if I need to back off, because of obstacle in front of me. 
    """
    left = Action.finalValues[Action.Left]
    if left == 0: 
        turnccw = hMath.CLIP(Action.finalValues[Action.TurnCCW],30)
        sObstacleBackOff.perform(0,Action.MAX_LEFT_NORMAL,turnccw)   
    else:
        left = hMath.CLIP(left,Action.MAX_LEFT_NORMAL)
        turnccw = hMath.CLIP(Action.finalValues[Action.TurnCCW],30)
        sObstacleBackOff.perform(0,left,turnccw,True)        
    """



gGetOutOfTheShotCounter = 0
gGetOutOfTheAttackerCounter = 0
#gGetAwayFromTheBallCounter = 0
gGetAwayFromTheSupporterCounter = 0

# Checks that the robot is not obstructing a teammate and gets out of the
# way if so. Returns True if evasive action was taken, False otherwise.
def getOutOfEveryonesWay():
    global gGetOutOfTheShotCounter   
    global gGetOutOfTheAttackerCounter 
#    global gGetAwayFromTheBallCounter
    global gGetAwayFromTheSupporterCounter

    # ---------------------------------------------------------------
    # Work out who is in what role. We cater for at most one attacker
    # and supporter
    attacker = None
    supporter = None
    for i in Global.otherValidTeammates: 
        mate = Global.teamPlayers[i]
        mateLoc = Global.teammatesLoc[i]
        # Get out from between the attacker and the ball
        if mate.isAttacker():
            attacker = (mate, mateLoc)
        elif mate.isSupporter():
            supporter = (mate, mateLoc)

    # ---------------------------------------------------------------
    # Continue any already-running get-out-of-the-ways
    targetH = hMath.normalizeAngle_0_360(Global.selfLoc.getHeading()\
                    + Global.ballH)

    # Away from the goal-shoot line
    if gGetOutOfTheShotCounter > 0:
        r = sGetOutOfTheWay.perform(Global.ballX,Global.ballY,\
                                Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y,\
                                targetH,True)
        if r == Constant.STATE_EXECUTING:
            gGetOutOfTheShotCounter -= 1
            return True
        gGetOutOfTheShotCounter = 0

    # Away from the attacker->ball line   
    if attacker != None and gGetOutOfTheAttackerCounter > 0: 
        r = sGetOutOfTheWay.perform(Global.ballX, Global.ballY,\
                                    attacker[1].getX(), attacker[1].getY(),\
                                    targetH,True)
        if r == Constant.STATE_EXECUTING:
            gGetOutOfTheAttackerCounter -= 1
            return True
        gGetOutOfTheAttackerCounter = 0                   
    else:
        gGetOutOfTheAttackerCounter = 0
                    
    
#    if gGetAwayFromTheBallCounter > 0:  # Away from the ball
#        r = sGetOutOfTheWay.getOutOfTheCircle(Global.ballX, Global.ballY, \
#                                                targetH, True)   
#        if r == Constant.STATE_EXECUTING:
#            gGetAwayFromTheBallCounter -= 1
#            return True
#        gGetAwayFromTheBallCounter = 0
    
    # Away from the supporter
    if supporter != None and gGetAwayFromTheSupporterCounter > 0:
        r = sGetOutOfTheWay.getOutOfTheCircle(supporter[1].getX(), \
                                            supporter[1].getY(), targetH, True) 
        if r == Constant.STATE_EXECUTING:
            gGetAwayFromTheSupporterCounter -= 1
            return True
        gGetAwayFromTheSupporterCounter = 0
    else:
        gGetAwayFromTheSupporterCounter = 0
    
    ### Check for any get-out-of-the-way I should start. ###
    # Get away from between ball and goal if attacker close to ball or the line
    # is short
    if attacker != None and (attacker[0].getTimeToReachBall() < 1500 \
            or hMath.getDistSquaredBetween(Global.ballX, Global.ballY, \
                       Constant.TARGET_GOAL_X, Constant.TARGET_GOAL_Y) < hMath.SQUARE(150)):
        r = sGetOutOfTheWay.perform(Global.ballX,Global.ballY,\
                            Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y,\
                            targetH)                                        
        if r == Constant.STATE_EXECUTING:  
            gGetOutOfTheShotCounter = GET_OUT_DURATION              
            return  True

    # Get out from between the attacker and the ball
    if attacker != None and attacker[0].getTimeToReachBall() < 3000:
        r = sGetOutOfTheWay.perform(Global.ballX, Global.ballY,\
                                    attacker[1].getX(), attacker[1].getY(),\
                                    targetH)
        if r == Constant.STATE_EXECUTING:  
            gGetOutOfTheAttackerCounter = GET_OUT_DURATION              
            return True
    
    # Stay clear of the supporter
    if supporter != None:
        r = sGetOutOfTheWay.getOutOfTheCircle(supporter[1].getX(), \
                                                supporter[1].getY(),\
                                                targetH)    
        if r == Constant.STATE_EXECUTING: 
            gGetAwayFromTheSupporterCounter = GET_OUT_DURATION
            return True

    # Get away from the ball
#    r = sGetOutOfTheWay.getOutOfTheCircle(Global.ballX, Global.ballY, targetH)  
#    if r == Constant.STATE_EXECUTING: 
#        gGetAwayFromTheBallCounter = GET_OUT_DURATION                
#        return True
            
    # All clear
    return False
    
def doGetToTargetPoint(targetX, targetY):
    global gTargetPointReqAccuracy

    selfX, selfY = Global.selfLoc.getX(), Global.selfLoc.getY()
    ballX, ballY, ballH = Global.ballX, Global.ballY, Global.ballH 
        
    h = hMath.normalizeAngle_0_360(hMath.RAD2DEG(math.atan2(\
                                                ballY - selfY, ballX - selfX)))
   
    #angle = hMath.absAngleBetweenTwoPointsFromPivotPoint(ballX, ballY, \
    #                                                     targetX, targetY, \
    #                                                     selfX, selfY)
        
    distSquared = hMath.getDistSquaredBetween(targetX,targetY,selfX,selfY)

    
##     if dist > 100 and angle < 80: 
##         hTrack.saGoToTarget(targetX,targetY)
##     else:
##         hTrack.saGoToTargetFacingHeading(targetX,targetY,h)
    
    hTrack.saGoToTargetFacingHeading(targetX,targetY,h)

    # Hysterisis for whether or not your at the defender point.
    if distSquared <= hMath.SQUARE(gTargetPointReqAccuracy):
        gTargetPointReqAccuracy = TARGET_PT_ACC_LARGE_CIRCLE        
        if abs(ballH) < 15:
            Action.stopLegs()
    else:
        gTargetPointReqAccuracy = TARGET_PT_ACC_SMALL_CIRCLE

    if abs(targetX - selfX) < 30 and abs(targetY - selfY) < 100:
        checkThenBlock()
    

def checkThenBlock():
    if 20 <= Global.selfLoc.getHeading() <= 160 and not Global.vOGoal.isVisible():
        # only block if my heading is right and I cannot see my own goal
        sBlock.checkThenBlock(onlyVisualBall=True, minBallSpeed = 2, dontBlockCentre = True)

# --------------------------------------------------------------------
# Defender's own way of finding a ball
# 1. Face towards the wireless ball, if we have one, and scan
# 2. Spin if we have no clue
# 3. Move back and repeat
TIMER_1 = 60
TIMER_2 = TIMER_1 + 90
TIMER_3 = TIMER_2 + 90

def findBallDefender():
    time = Global.lostBall % TIMER_3
    if time < TIMER_1: 
        turnccw = hMath.CLIP(Global.ballH,80)    
        Action.walk(0,0,turnccw, minorWalkType = Action.SkeFastForwardMWT)      
        hTrack.scan()

    elif time < TIMER_2:
        hFWHead.compulsoryAction = hFWHead.doNothing 
        sFindBall.findBySpin()
            
    else:
        targetH = Global.selfLoc.getHeading() + Global.ballH
        hTrack.saGoToTargetFacingHeading(Global.selfLoc.getX(), \
                                    Constant.FIELD_LENGTH * 0.25, targetH)  
