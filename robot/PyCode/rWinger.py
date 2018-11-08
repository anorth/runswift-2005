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
#  $Id: StrikerBecken.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# Striker.py
#
# Called by Forward.py to make decisions base on being the role of striker.
#
#===============================================================================

"""
Winger
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
import sGetOutOfTheWay
#import sObstacleBackOff



TARGET_PT_ACC_SMALL_CIRCLE = 20
TARGET_PT_ACC_LARGE_CIRCLE = 40
NORMAL_BACKOFF_YDIST       = 160

#
gTargetPointReqAccuracy    = TARGET_PT_ACC_SMALL_CIRCLE


gLastFrameCalled = 0

GET_OUT_DURATION = 75
gGetOutOfTheBallCounter = 0
gGetOutOfTheAttackerCounter = 0
gGetOutOfTheCircleCounter = 0
gMateLoc = None
#===============================================================================
# Functions belong to this module.
#===============================================================================

# Reset these variables every frame.
def resetEachFrame():
    global gLastFrameCalled
    global gGetOutOfTheBallCounter
    global gGetOutOfTheAttackerCounter   
    global gGetOutOfTheCircleCounter 
    
    if gLastFrameCalled != Global.frame - 1: 
        gGetOutOfTheBallCounter = 0
        gGetOutOfTheAttackerCounter = 0        
        gGetOutOfTheCircleCounter = 0
    gLastFrameCalled = Global.frame 


def DecideNextAction():
    Global.myLastRole = Constant.WINGER
    Global.myRole = Constant.WINGER
    Indicator.setDefault()
    hFWHead.resetEachFrame() 
    perform()   
     

#--------------------------------------
# Called by DecideNextAction in Forward.py
def perform(params = None):
    global gGetOutOfTheBallCounter   
    global gGetOutOfTheAttackerCounter 
    global gGetOutOfTheCircleCounter
    global gMateLoc
    global gTargetPointReqAccuracy
    id(params) # ignore
    
    resetEachFrame() 

    # Back of if stuck
    if hStuck.amIStuckForward():
        hFWHead.DecideNextAction()
        return

    hFWHead.DecideNextAction()

    # If the attacker has grabbed, get out of the way
#    if hTeam.hasTeammateGrabbedBall():
#        if getOutOfEveryonesWay():
#            return

      
    # ---------------------------------------------------------------
    # Check if I need to get out of the way for the attacker first.
    targetH = hMath.normalizeAngle_0_360(Global.selfLoc.getHeading() + Global.ballH)

    if gGetOutOfTheBallCounter > 0:         
        r = sGetOutOfTheWay.perform(Global.ballX,Global.ballY,\
                                    Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y,\
                                    targetH,True)
        if r == Constant.STATE_EXECUTING:
            gGetOutOfTheBallCounter -= 1
            Indicator.showFacePattern([0,0,2,0,0])                    
            return
        gGetOutOfTheBallCounter = 0
    
    if gGetOutOfTheAttackerCounter > 0:         
        r = sGetOutOfTheWay.perform(Global.ballX,Global.ballY,\
                                    gMateLoc.getX(),gMateLoc.getY(),\
                                    targetH,True)
        if r == Constant.STATE_EXECUTING:
            gGetOutOfTheAttackerCounter -= 1
            Indicator.showFacePattern([0,0,2,0,0])                    
            return
        gGetOutOfTheAttackerCounter = 0                   
                    
    
    if gGetOutOfTheCircleCounter > 0:
        r = sGetOutOfTheWay.getOutOfTheCircle(Global.ballX,Global.ballY,targetH,True)   
        if r == Constant.STATE_EXECUTING:
            gGetOutOfTheCircleCounter -= 1
            Indicator.showFacePattern([0,2,0,2,0])                        
            return
        gGetOutOfTheCircleCounter = 0
    
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]
        if mate.isAttacker()\
            and mate.getTimeToReachBall() < 3000:
                
            r = sGetOutOfTheWay.perform(Global.ballX,Global.ballY,\
                                        Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y,\
                                        targetH)                                        
            if r == Constant.STATE_EXECUTING:  
                gGetOutOfTheBallCounter = GET_OUT_DURATION              
                Indicator.showFacePattern([0,0,2,0,0])
                return  
            
            mateLoc = Global.teammatesLoc[i]
            gMateLoc = mateLoc.getCopy()
            r = sGetOutOfTheWay.perform(Global.ballX,Global.ballY,\
                                        mateLoc.getX(),mateLoc.getY(),\
                                        targetH)                                        
            if r == Constant.STATE_EXECUTING:  
                gGetOutOfTheAttackerCounter = GET_OUT_DURATION              
                Indicator.showFacePattern([0,0,2,0,0])
                return   

        
            r = sGetOutOfTheWay.getOutOfTheCircle(Global.ballX,Global.ballY,targetH)    
            if r == Constant.STATE_EXECUTING: 
                gGetOutOfTheCircleCounter = GET_OUT_DURATION                
                Indicator.showFacePattern([0,2,0,2,0])                        
                return   
    # ---------------------------------------------------------------     

    # trigger BOP if i was doing it.
    if sBirdOfPrey.isBirdOfPreyTriggering():        
        # FIXME: Winger currently not in use so birding not updated
        if sBirdOfPrey.perform(Global.ballX, Global.ballY) == Constant.STATE_EXECUTING:
            return
        
    # trigger BOP when I am certain distance behind from the ball
    DEFENCE_OFFSET = 10.0
    DEFENCE_ANGLE  = 150.0
    if sBirdOfPrey.areWeAboveTheLine(DEFENCE_OFFSET,DEFENCE_ANGLE,True, \
                                    Global.ballX, Global.ballY):
        sBirdOfPrey.perform(Global.ballX, Global.ballY)
        return 

    # If I have lost the ball for a while, then find the ball.
    if (Global.ballSource == Constant.GPS_BALL and Global.lostBall > 210)\
        or (Global.ballSource == Constant.WIRELESS_BALL and Global.lostBall > 240): 

        Indicator.showFacePattern([5,0,0,0,5])
        sFindBall.perform()
        return 

    doGetToTargetPoint()  
     
    
def doGetToTargetPoint():
    global gTargetPointReqAccuracy

    targetX, targetY = hWhere.getWingerPos()          
            
    selfX, selfY = Global.selfLoc.getPos()
    selfH = Global.selfLoc.getHeading()
    ballH = Global.ballH 
        
    h = hMath.normalizeAngle_0_360(selfH + ballH)
   
    hTrack.saGoToTargetFacingHeading(targetX,targetY,h)
    
    # Hysterisis for whether or not your at the striker point.
    distSquared = hMath.getDistSquaredBetween(targetX,targetY,selfX,selfY)

    if distSquared <= hMath.SQUARE(gTargetPointReqAccuracy):
        gTargetPointReqAccuracy = TARGET_PT_ACC_LARGE_CIRCLE        
        if abs(ballH) < 5:
            Action.stopLegs() 
        
    else:
        gTargetPointReqAccuracy = TARGET_PT_ACC_SMALL_CIRCLE

