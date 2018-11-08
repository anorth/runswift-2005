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
#  $Id: SupportZidane.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2005 (c) 
#
# Striker.py
#
# Called by pForward.py to make decisions base on being the role of striker.
#
#===============================================================================

"""
(Striker)
"""

import Action
import Constant
import Global
import hFWHead
import hMath
import hTrack
import hWhere
import hStuck
import hTeam
import Indicator
#import math

#import sActiveLocalise
import sBirdOfPrey
import sFindBall
#import sGetBehindBall
import sGetOutOfTheWay
#import sObstacleBackOff
#import VisionLink




#===============================================================================
# Functions belong to this module.
#===============================================================================

# Hysterisis for reaching target
TARGET_PT_ACC_SMALL_CIRCLE = 15
TARGET_PT_ACC_LARGE_CIRCLE = 30
gTargetPointReqAccuracy = TARGET_PT_ACC_SMALL_CIRCLE

# Don't choose a position closer than this to sideline
STAY_INSIDE_X = 40

# Desired position will be moved back by BACKOFF_DIST and wider by RANGEX_MULT
# times if we don't see the ball for BACKOFF_TIME frames
LOST_BALL_BACKOFF_TIME = 5
LOST_BALL_BACKOFF_DIST = 20
LOST_BALL_RANGEX_MULT = 1.2

# Which side of attacker we are on. Constant.LEFT or RIGHT
gSelectedSide = Constant.LEFT

# Histerisis on (attackerX - ballX) for choosing a side
SIDE_HYSTERISIS = 20

# We will bird to striker pos if in front by this much
BIRD_TO_POS_THRESH = 100


GET_OUT_DURATION = 45   # shorter than defender

gLastFrameCalled = 0


#--------------------------------------
# Reset these variables every frame.
def resetEachFrame():
    global gLastFrameCalled
    global gGetOutOfTheShotCounter
    global gGetOutOfTheAttackerCounter   
    global gGetAwayFromTheBallCounter 
    global gGetAwayFromTheAttackerCounter
    
    if gLastFrameCalled != Global.frame - 1: 
        gGetOutOfTheShotCounter = 0
        gGetOutOfTheAttackerCounter = 0        
        #gGetAwayFromTheBallCounter = 0
        gGetAwayFromTheAttackerCounter = 0
    gLastFrameCalled = Global.frame 


def DecideNextAction():
    Global.myLastRole = Constant.STRIKER
    Global.myRole = Constant.STRIKER
    Indicator.setDefault()
    hFWHead.resetEachFrame()
    perform()
    

def perform(params = None):    
    global gBirdToBall
    id(params) # ignore
    resetEachFrame()

    # Back of if stuck
    if hStuck.amIStuckForward():
        hFWHead.DecideNextAction()
        return

    targetX, targetY, _rangeX = hWhere.getStrikerPos()          
    selfX, selfY = Global.selfLoc.getPos()

    hFWHead.DecideNextAction()

    # If the attacker has grabbed, get out of the way
    if hTeam.hasTeammateGrabbedBall():
        if getOutOfEveryonesWay():
            return


    # Continue BOP if one is executing
    if sBirdOfPrey.isBirdOfPreyTriggering():        
        if sBirdOfPrey.birdTo == sBirdOfPrey.BIRD_TO_BALL:
            birdPos = (Global.ballX, Global.ballY)
        else:
            birdPos = (targetX, targetY)
        if sBirdOfPrey.perform(*birdPos) == Constant.STATE_EXECUTING:
            hFWHead.minPosVariance = 1  # look around more often
            hFWHead.DecideNextAction()
            return

    # Stay out of the shot line and away from robots. This is after the bird
    # since the bird is more important than most of these, and DodgyDog in 
    # the bird should keep us clear
    if getOutOfEveryonesWay():
        return
    
    # If I have lost the ball for a while, then find the ball.
    # Don't trust gps for a long time, because after a few frames, gps ball
    # will direct you to no where.  It is better for the robot to go into find
    # ball mode. 
    if Global.ballSource == Constant.GPS_BALL and Global.lostBall >= Constant.LOST_BALL_GPS: 
        Indicator.showFacePattern([5,0,0,0,5])
        findBallStriker()
        return 
    
    # Move to striker position
    supportBall()
    
    # Bird back to position if I am way in front
    if targetY < selfY - BIRD_TO_POS_THRESH:
        sBirdOfPrey.birdTo = sBirdOfPrey.BIRD_TO_POS
        if sBirdOfPrey.perform(targetX, targetY) == Constant.STATE_EXECUTING: 
            return       

    # Bird if we need to (but doesn't if our target is in forward of the ball)
    #DEFENCE_OFFSET = 10.0
    #DEFENCE_ANGLE  = 150.0
    #if targetY < Global.ballY < selfY and \
    #        sBirdOfPrey.areWeAboveTheLine(DEFENCE_OFFSET, DEFENCE_ANGLE, True, \
    #                                        Global.ballX, Global.ballY):
    #    sBirdOfPrey.birdTo = sBirdOfPrey.BIRD_TO_BALL
    #    if sBirdOfPrey.perform(Global.ballX, Global.ballY) == Constant.STATE_EXECUTING: 
    #        return       

# Move to the striker position, and return that target position
def supportBall(): 
    global gTargetPointReqAccuracy

    targetX, targetY, rangeX = hWhere.getStrikerPos() 
    targetX, targetY = getAdjustedTarget(targetX, targetY, rangeX)  

    selfX, selfY = Global.selfLoc.getPos() 
    selfH = Global.selfLoc.getHeading()
    ballH = Global.ballH                
     
    h = hMath.normalizeAngle_0_360(selfH + ballH)
    distSquared = hMath.getDistSquaredBetween(targetX,targetY,selfX,selfY)
        
    # From outside a metre walk fast, else turn to face ball
    #if dist > 100:
    #    hTrack.saGoToTarget(targetX, targetY)
    #else:
    #    hTrack.saGoToTargetFacingHeading(targetX, targetY, h)
    hTrack.saGoToTargetFacingHeading(targetX, targetY, h)
    
    # Hysterisis for whether or not you are at the striker point.

    if distSquared <= hMath.SQUARE(gTargetPointReqAccuracy):
        gTargetPointReqAccuracy = TARGET_PT_ACC_LARGE_CIRCLE
        if abs(ballH) < 5:
            Action.stopLegs()                   
    else:
        gTargetPointReqAccuracy = TARGET_PT_ACC_SMALL_CIRCLE 

    return targetX, targetY


# Using the rangeX, redetermine the striker's targetX (to left or right
# of specified targetX) using attacker position etc. Also adjusts targetX,
# targetY to be further out if ball is not seen
def getAdjustedTarget(targetX, targetY, rangeX):
    global gSelectedSide

    # If we can't see the ball adjust the position a little back and wider
    # This must be done before we choose side
    if Global.lostBall > LOST_BALL_BACKOFF_TIME:
        targetY -= LOST_BALL_BACKOFF_DIST
        rangeX *= LOST_BALL_RANGEX_MULT

    # Near the edges of the field always choose an inside pos
    if targetX - rangeX < STAY_INSIDE_X:
        gSelectedSide = Constant.RIGHT
    elif targetX + rangeX > Constant.FIELD_WIDTH - STAY_INSIDE_X:
        gSelectedSide = Constant.LEFT

    # In the attacking quarter always choose inside
    if Global.ballY > Constant.FIELD_LENGTH * 0.75:
        if Global.ballX > Constant.FIELD_WIDTH/2:
            gSelectedSide = Constant.LEFT
        else:
            gSelectedSide = Constant.RIGHT
    
    # If the attacker has grabbed choose the inside.
    # If the attacker is much to one side of the ball then choose the other
    # side. Otherwise stay on the side we are already on
    else:
        for i in Global.otherValidForwards:      
            mate = Global.teamPlayers[i]
            if mate.isAttacker():
                mateLoc = Global.teammatesLoc[i]
                
                if mate.hasGrabbedBall(): 
                    if targetX > Constant.FIELD_WIDTH/2:
                        gSelectedSide = Constant.LEFT
                    else:
                        gSelectedSide = Constant.RIGHT
                
                elif mateLoc.getX() < mate.getBallX() - SIDE_HYSTERISIS: 
                    gSelectedSide = Constant.RIGHT
                elif mateLoc.getX() > mate.getBallX() + SIDE_HYSTERISIS:
                    gSelectedSide = Constant.LEFT
                # else don't change
                                       
    # If facing forward choose the side with more obstacles
    # FIXME: use GPS obs and hysterisis
#    elif 45 < Global.selfLoc.getHeading() < 135:
#        numObstacleLeft = VisionLink.getNoObstacleInBox(-50,80,0,50)
#        numObstacleRight = VisionLink.getNoObstacleInBox(0,80,50,50)
#        if numObstacleLeft > numObstacleRight: 
#            targetX += rangeX
#        else:
#            targetX -= rangeX

    if gSelectedSide == Constant.LEFT:
        Indicator.showFacePattern([0, 0, 0, 2, 3])
    else:
        Indicator.showFacePattern([3, 2, 0, 0, 0])

    return (targetX + (-gSelectedSide * rangeX), targetY)

# using the rangeX, redetermine the striker's targetX by winger's position
def getAdjustedTargetX2(targetX,rangeX):    
    for i in Global.otherValidForwards:      
        mate = Global.teamPlayers[i]
        if not mate.isAttacker():
            mateLoc = Global.teammatesLoc[i]
            if mateLoc.getPosVar() < hMath.get95var(30):
                if mateLoc.getX() < Constant.FIELD_WIDTH/2: 
                    targetX += rangeX
                else:
                    targetX -= rangeX 
                        
    targetX = max(min(targetX,Constant.FIELD_WIDTH-100),100)
    return targetX   


gGetOutOfTheShotCounter = 0
gGetOutOfTheAttackerCounter = 0
#gGetAwayFromTheBallCounter = 0
gGetAwayFromTheAttackerCounter = 0

# Checks that the robot is not obstructing a teammate and gets out of the
# way if so. Returns True if evasive action was taken, False otherwise.
# Similar to the one in rDefender.
def getOutOfEveryonesWay():
    global gGetOutOfTheShotCounter   
    global gGetOutOfTheAttackerCounter 
#    global gGetAwayFromTheBallCounter
    global gGetAwayFromTheAttackerCounter

    # ---------------------------------------------------------------
    # Work out who is in what role. We cater for at most one attacker
    attacker = None
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]
        mateLoc = Global.teammatesLoc[i]
        if mate.isAttacker():
            attacker = (mate, mateLoc)

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
    
    # Away from the attacker robot
    if attacker != None and gGetAwayFromTheAttackerCounter > 0:
        r = sGetOutOfTheWay.getOutOfTheCircle(attacker[1].getX(),\
                                              attacker[1].getY(), targetH, True) 
        if r == Constant.STATE_EXECUTING:
            gGetAwayFromTheAttackerCounter -= 1
            return True
        gGetAwayFromTheAttackerCounter = 0
    else:
        gGetAwayFromTheAttackerCounter = 0
    
    #------- Check for any get-out-of-the-way I should start ----------------
    # Get away from between ball and goal if attacker close to ball or the line
    # is short
    if attacker != None\
        and (attacker[0].getTimeToReachBall() < 1500\
            or hMath.getDistSquaredBetween(Global.ballX, Global.ballY,\
                                        Constant.TARGET_GOAL_X, Constant.TARGET_GOAL_Y) < hMath.SQUARE(150))\
        and not (Global.ballSource == Constant.GPS_BALL and Global.lostBall >= Constant.LOST_BALL_GPS):
                
        r = sGetOutOfTheWay.perform(Global.ballX,Global.ballY,\
                            Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y,\
                            targetH)                                        
        if r == Constant.STATE_EXECUTING:  
            gGetOutOfTheShotCounter = GET_OUT_DURATION              
            return  True

    # Get out from between the attacker and the ball
    if attacker != None and attacker[0].getTimeToReachBall() < 3000\
        and not (Global.ballSource == Constant.GPS_BALL and Global.lostBall >= Constant.LOST_BALL_GPS):
        r = sGetOutOfTheWay.perform(Global.ballX, Global.ballY,\
                                    attacker[1].getX(), attacker[1].getY(),\
                                    targetH)
        if r == Constant.STATE_EXECUTING:  
            gGetOutOfTheAttackerCounter = GET_OUT_DURATION              
            return True
    
    # Stay clear of the attacker robot
    if attacker != None:
        r = sGetOutOfTheWay.getOutOfTheCircle(attacker[1].getX(), \
                                              attacker[1].getY(),\
                                              targetH, radius = 30)    
        if r == Constant.STATE_EXECUTING: 
            gGetAwayFromTheAttackerCounter = GET_OUT_DURATION
            return True

    # Get away from the ball
#    if not (Global.ballSource == Constant.GPS_BALL and Global.lostBall >= Constant.LOST_BALL_GPS):
#        r = sGetOutOfTheWay.getOutOfTheCircle(Global.ballX, Global.ballY, targetH)  
#        if r == Constant.STATE_EXECUTING: 
#            gGetAwayFromTheBallCounter = GET_OUT_DURATION                
#            return True
            
    # All clear
    return False
    
# --------------------------------------------------------------------
# Striker's own way of finding a ball (copied from Defender)
# 1. Face towards the wireless ball, if we have one, and scan
# 2. Spin if we have no clue
# 3. Move and repeat
TIMER_1 = 60
TIMER_2 = TIMER_1 + 90
TIMER_3 = TIMER_2 + 90

def findBallStriker():
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
        targetX, targetY, _rangeX = hWhere.getStrikerPos(True) 
        
        hTrack.saGoToTargetFacingHeading(targetX, targetY, targetH)      
    
