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
#  $Id: rAttacker.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 





#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# cVariableChallenge.py
#
# Called by Forward.py to make decisions base on being the role of attacker.


# Changes: 
# 1. Narrow dkd when lined up, so paw kick, dont call dir kick
# 2. Get behind when in the own half
# 3. Get hehind upenn when in target half, if grab and upenn is not ok (that is when opp near by)
#     to replace fast upenn
# 4. Comment out the goal edge turn kick
# 
#
#===============================================================================
"""
Variable Lighting challenge
"""
import Action
import Constant
import Global

import hMath
import hPWalk
#import hTeam
import hTrack
#import hWhere
import Indicator
#import math
import sActiveLocalise
import sAvoidOwnGoal

import sDodgyDog
import sDribble
import scFindBall
import sGetBehindBall
import sGrab
import sGrabDribble
import sGrabTurnKick
import sPawKick
import sSelKick
import sTurnKick
import sUpennKick

import VisionLink

#===============================================================================
# Functions belong to this module.
#===============================================================================

LOCALISE_ACTIVATE_BALL_RANGE = 80
LOCALISE_DEACTIVATE_BALL_RANGE = 60 
LOCALISE_TIMER = 8
OBSTACLE_THRESHOLD = 20
gLastActivelyLocalisedFrame = 0
gActiveLocaliseTimer = 0

gLastFrameCalled = 0
gIsApproaching = False
gSelectedKick = None
gSelectedGetBehind = None # (desiredGetBehindDirection,direction)

gUseGetBehind = False
gUseDodgyDog = True
playMode = False
isGoal = False

lastTrackBall = 0
startTrackBall = 0
lastGoToCenter = 0  
startGoToCenter = 0

def ObsMapping():
    global playMode
    for pressSensor in range(1,4,1): # Used to react to back sensors
        if VisionLink.getPressSensorCount(pressSensor) > 8:
            print "Stop mapping and play the game."
            VisionLink.setIfObstacleUpdate(False)
            playMode = True
            VisionLink.setIfObstacleUpdate(False)
            return
    VisionLink.setIfObstacleUpdate(True)            
    hTrack.stationaryLocalise()
    
def DecideNextAction(): 
    global isGoal
    #print "rAttacker: Global.selfLoc at", ("%x" % id(Global.selfLoc)),\
    #       "=", Global.selfLoc
    Indicator.setDefault()  
    #VisionLink.setGPSPaused(0)
    if playMode:
        if Global.vBall.isVisible(): 
            isGoal = False
        
        for pressSensor in range(1,4,1): # Used to react to back sensors
            if VisionLink.getPressSensorCount(pressSensor) > 8:
                print "It's a goal!!"
                isGoal = True
        
        if isGoal:
            VisionLink.resetBall(Constant.FIELD_WIDTH/2.0,Constant.FIELD_LENGTH/2.0,0.0,0.0)
            pos = Global.selfLoc.getPos()
            if (hMath.getDistanceBetween(pos[0],pos[1],Constant.FIELD_WIDTH/2.0,Constant.FIELD_LENGTH/2.0) < 50):
                scFindBall.perform()
            else:
                hTrack.stationaryLocalise()
                hTrack.saGoToTarget(Constant.FIELD_WIDTH/2.0,Constant.FIELD_LENGTH/2.0)
        else:
            perform()
    else:
        ObsMapping()

def resetPerform(): 
    global gSelectedKick
    global gActiveLocaliseTimer 
        
    gSelectedKick = None
    gActiveLocaliseTimer = 0
    
def doGotoCenter():
    global lastGoToCenter, startGoToCenter
    if Global.frame - lastGoToCenter > 1:
        startGoToCenter = Global.frame
    lastGoToCenter = Global.frame
    
    hTrack.stationaryLocalise()
    hTrack.saGoToTarget(Constant.FIELD_WIDTH/2.0,Constant.FIELD_LENGTH/2.0+100)
    
def shouldIGoToCenter():
    pos = Global.selfLoc.getPos()
    if (Global.frame - lastTrackBall > 1):
        return False
    elif ((Global.frame - lastTrackBall < 2) and (lastGoToCenter - startGoToCenter > 60)):
        return False
    elif (lastTrackBall - startTrackBall > Constant.LOST_BALL_SPIN):
        return True
    elif (hMath.getDistanceBetween(pos[0],pos[1],Constant.FIELD_WIDTH/2.0,Constant.FIELD_LENGTH/2.0+50) < 50):
        return False
    return False
          
def perform(params = None):
    global gLastFrameCalled
    global gLastDecisionFunction
    id(params) # ignore

    shouldIBeDodgy = gUseDodgyDog

    if gLastFrameCalled != Global.frame - 1: 
        resetPerform()
    gLastFrameCalled = Global.frame

    if shouldIGetBehindBall():
        shouldIBeDodgy = False
        doGetBehindBall()

    elif not shouldIEndActivelyLocalise()\
        or shouldIActivelyLocalise():
        doActivelyLocalise()    

    elif shouldIKick():
        #shouldIBeDodgy = False  #Really?
        doKick()     
    elif shouldIGoToCenter():
        doGotoCenter()
    else:
        doTrackBall()

    # Use dodgy dog if required
    fwdCmd = Action.finalValues[Action.Forward]
    shouldIBeDodgy = shouldIBeDodgy and (not sGrab.isGrabbed) \
                    and sGrab.grabbingCount == 0 \
                    and Global.ballD > sGrab.CLOSE_DIST \
                    and fwdCmd > Action.MAX_SKE_FWD_SPD/2 \
                    and fwdCmd > Action.finalValues[Action.Left] \
                    and sDodgyDog.shouldIBeDodgyToBall()
    if shouldIBeDodgy:
        sDodgyDog.dodgyDogToBall()

gLastBallX = 0 
gLastBallY = 0
def shouldIKick():        
    global gSelectedKick
    global gLastBallX
    global gLastBallY
    
    if gSelectedKick != None\
        and (sGrab.isGrabbed
            or sGrab.grabbingCount > 0
            or Action.shouldIContinueKick()):
        #print "shouldIKick 1", str(sGrab.isGrabbed), "  ", str(gSelectedKick)
        return True
    
    if Global.lostBall < Constant.LOST_BALL_GPS:
        if gSelectedKick != None:
            # If the distance is less than 20 and 
            # the ball hasn't moved far.
            if Global.ballD <= 30\
                and hMath.getDistanceBetween(Global.ballX,Global.ballY,\
                                             gLastBallX,gLastBallY) < 40:
                #print "shouldIKick 2", str(sGrab.isGrabbed), "  ", str(gSelectedKick)                             
                return True

            # If this chosen kick needs to grab and the ball is close,
            # then don't give up the grab.
            if sSelKick.doINeedToGrab(gSelectedKick[0])\
                and Global.ballD <= sGrab.CLOSE_DIST + 5:
                #print "shouldIKick 3", str(sGrab.isGrabbed), "  ", str(gSelectedKick)
                return True       
        
        # If the ball has moved a lot since the last kick selection, reset.
        selectedKick = sSelKick.perform()
        
        # If the selected kick is different, then do kick reset. 
        if gSelectedKick != None and selectedKick[0] != gSelectedKick[0]:
            doKickReset()    
        gSelectedKick = selectedKick     
        
        
        gLastBallX = Global.ballX
        gLastBallY = Global.ballY
        #print "shouldIKick 4", str(sGrab.isGrabbed), "  ", str(gSelectedKick)
        return True
       
    Indicator.showFacePattern([1,1,1,1,1])        
    gSelectedKick = None        
    return False

BACKOFF_BALL_BOTTOM_EDGE_DISTANCE = 60
def shouldIGetBehindBall(): 
    global gSelectedGetBehind 

    if not gUseGetBehind:
        return False
    
    """
    if Global.lostBall >= Constant.LOST_BALL_GPS: 
        return False

    ballX, ballY = Global.ballX, Global.ballY
    ballD, ballH = Global.ballD, Global.ballH
    selfX, selfY = Global.selfLoc.getX(), Global.selfLoc.getY()
    selfH = Global.selfLoc.getHeading()
    
    do_it = False
    
    # default desired get behind direction
    dkd = sSelKick.perform()[1]
    
    # if the ball is on one of the side edges and your not facing the right way
    if Global.ballSource == Constant.VISION_BALL\
        and ballD < 50\
        and (hWhere.ballOnLEdge() or hWhere.ballOnREdge())\
        and 195 <= selfH <= 345:       
        do_it = True

    # if ball is on bottom edge, get behind ball
    if Global.ballSource == Constant.VISION_BALL\
        and ballD < 50\
        and Global.ballY < BACKOFF_BALL_BOTTOM_EDGE_DISTANCE\
        and 200 < selfH < 340:
        #(gps->getOppCovMax(oppNum) < get95CF(75) && opp.d < 100)) {
        do_it = True
    
    
    # if ball is in front of own goal
    ballD2OwnGoal = hMath.getDistanceBetween(ballX,ballY,\
                                             Constant.FIELD_WIDTH/2,0)
    towardsOwnGoalH = hMath.normalizeAngle_0_360(hMath.RAD2DEG(\
                        math.atan2(0 - selfY,Constant.FIELD_WIDTH/2.0 -selfX)))
    
    if Global.ballSource == Constant.VISION_BALL\
        and ballD < 40\
        and ballD2OwnGoal <= 60\
        and towardsOwnGoalH - 60 < selfH < towardsOwnGoalH + 60: 
        
        dkd = hMath.RAD2DEG(math.atan2(ballY - 0, ballX - Constant.FIELD_WIDTH/2.0))          
        do_it = True

    # if ball on top right edge and you're facing wrong way, get behind ball
    if Global.ballSource == Constant.VISION_BALL\
        and hWhere.ballOnTEdge()\
        and ballX > Constant.FIELD_WIDTH/2.0\
        and 7 < ballD < 50\
        and (0 < selfH < 80 or selfH > 330):

        dkd = 100
        do_it = True
    
    # if ball on top left edge and you're facing wrong way, get behind ball
    if Global.ballSource == Constant.VISION_BALL\
        and hWhere.ballOnTEdge()\
        and ballX < Constant.FIELD_WIDTH/2.0\
        and 7 < ballD < 50\
        and 100 < selfH < 210:

        dkd = 80
        do_it = True

    # if ball on top left corner and you're facing the wrong way, get behind ball
    if Global.ballSource == Constant.VISION_BALL\
        and hWhere.ballInTLCorner()\
        and 7 < ballD < 50\
        and 110 < selfH < 210:
        
        dkd = 80
        do_it = True    
    
    # if ball on top right corner and you're facing the wrong way, get behind ball
    if Global.ballSource == Constant.VISION_BALL\
        and hWhere.ballInTRCorner()\
        and 7 < ballD < 50\
        and (0 < selfH < 70 or selfH > 330):
        
        dkd = 100
        do_it = True
    
    
    if not do_it: 
        return False
        
    
    # decide direction the robot should take
    # left right edge cases
    if hWhere.ballOnLEdge():
        direction = Constant.dCLOCKWISE
    elif hWhere.ballOnREdge():
        direction = Constant.dANTICLOCKWISE           

    # top edge cases
    elif hWhere.ballOnTEdge() and ballX < Constant.FIELD_WIDTH/2.0:
        direction = Constant.dCLOCKWISE
    elif hWhere.ballOnTEdge() and ballX >= Constant.FIELD_WIDTH/2.0:
        direction = Constant.dANTICLOCKWISE

    #bottom edge cases
    elif ballY < BACKOFF_BALL_BOTTOM_EDGE_DISTANCE\
        and ballX < Constant.FIELD_WIDTH/2.0:
        direction = Constant.dCLOCKWISE
    elif ballY < BACKOFF_BALL_BOTTOM_EDGE_DISTANCE\
        and ballX >= Constant.FIELD_WIDTH/2.0:
        direction = Constant.dANTICLOCKWISE
    
    # 
    else:
        direction = None
    
    gSelectedGetBehind = (dkd,direction)
    """    
    return True
    
    
                
def shouldIActivelyLocalise():     
    global gActiveLocaliseTimer
        
    # The attacker has just span to find the ball.        
    if Global.frame - scFindBall.gLastSpinFrame > 60:
    
        # The attacker can see the ball.
        if Global.haveBall > 3:

            # The ball is still far away and straight ahead.
            #ballD, ballH = Global.ballD, Global.ballH
            ballD, ballH = Global.ballD, Global.ballH
            if ballD > LOCALISE_ACTIVATE_BALL_RANGE and abs(ballH) < 10:

                # If the ball is moving fast, then don't localise
                vel = VisionLink.getGPSBallVInfo(Constant.CTLocal)        
                velX, velY = vel[0], vel[1]
                if abs(velX) < 2 and abs(velY) < 2:

                    # If no obstacle is around the ball.
                    obs = VisionLink.getNoObstacleInHeading(int(ballH-5),int(ballH+5),0,int(ballD))
                    # The attacker has not recently actively localised.
                    if Global.frame - gLastActivelyLocalisedFrame > 60:
                        gActiveLocaliseTimer = LOCALISE_TIMER
                        sActiveLocalise.SmartSetBeacon(70)
                        return True

    gActiveLocaliseTimer = 0
    return False


def shouldIEndActivelyLocalise():
    global gActiveLocaliseTimer
    gActiveLocaliseTimer -= 1
    if gActiveLocaliseTimer <= 0:
        return True
    if Global.ballD < LOCALISE_DEACTIVATE_BALL_RANGE:
        return True
    return False


##--------------------------------------------------------------------                 
                    
def doActivelyLocalise(): 
    global gLastActivelyLocalisedFrame
    gLastActivelyLocalisedFrame = Global.frame
       
    sActiveLocalise.DecideNextAction()
    Action.walk(Action.MAX_FORWARD,0,0,minorWalkType=Action.SkeFastForwardMWT)    
    Global.lostBall = 0


def doTrackBall(headOnly=False):
    global lastTrackBall,startTrackBall
    if Global.frame - lastTrackBall > 1:
        startTrackBall = Global.frame
    lastTrackBall = Global.frame
    scFindBall.perform(headOnly) 
    

def doGetBehindBall(): 
    global gSelectedGetBehind
    dkd = gSelectedGetBehind[0]
    direction = gSelectedGetBehind[1]

    r = sGetBehindBall.perform(dkd=dkd,\
                               direction=direction)    
    if r == Constant.STATE_SUCCESS\
        or r == Constant.STATE_FAILED: 
        gSelectedGetBehind = None
        scFindBall.perform()
    else:
        scFindBall.perform(True)
        if direction == Constant.dCLOCKWISE: 
            Indicator.showFacePattern([1,1,0,0,0])
        else:
            Indicator.showFacePattern([0,0,0,1,1])        
    

gIsKickTriggering = 0
gLastKickedFrame = 0
def doKick():
    global gSelectedKick
    global gIsKickTriggering
    global gLastKickedFrame
         
    # After you released the ball, don't grab for a second.
    if Global.frame - gLastKickedFrame < 30:
        doTrackBall()
        return    
    
    
    kickType = gSelectedKick[0]
     
    # grab actions
    if sSelKick.doINeedToGrab(kickType)\
        or sGrab.isGrabbed:
        r = doKickWithGrab()                      
    # non grab actions     
    else:                                        
        r = doKickWithoutGrab()

            
    # When return value from skill is not executing, 
    # Do some resetting business       
    if r != Constant.STATE_EXECUTING:
        # Resetting grab and all other kick modules
        sGrab.resetPerform()
        doKickReset()
                
        gLastKickedFrame = Global.frame
            
        gIsKickTriggering = False
        gSelectedKick = None
        
        if kickType == sSelKick.KT_DRIBBLE\
            or kickType == sSelKick.KT_PAW_KICK:
            hTrack.panLow = True
        else:
            hTrack.panLow = False
        
        doTrackBall()

    showKickTypeIndicator(kickType)

def doKickReset(): 
    # Resetting all the kicks    
    sAvoidOwnGoal.resetPerform()
    sDribble.resetPerform()
    sGrabDribble.resetPerform()
    sGrabTurnKick.resetPerform()
    sPawKick.resetPerform()
    sTurnKick.resetPerform()
    sUpennKick.resetPerform()     

def doKickWithGrab(): 
    global gSelectedKick
    global gIsKickTriggering
    

    if not sGrab.isGrabbed:   
            
        r = sGrab.perform()
        if r == Constant.STATE_FAILED: 
            hTrack.panLow = True
            return r  
        elif r == Constant.STATE_EXECUTING:
            return r    
        # Grab success    
        else:                      
            # override kick selection
            selectedKick = sSelKick.perform()

            # If the selected kick is different, then do kick reset. 
            if gSelectedKick != None and selectedKick[0] != gSelectedKick[0]: 
                doKickReset()    
            gSelectedKick = selectedKick

            # If kick selection returns paw kick, change it to 
            # grab dribble.
            # Because we can't paw kick or dribble from grab.
            kickType = gSelectedKick[0]
            if kickType == sSelKick.KT_DRIBBLE\
                or kickType == sSelKick.KT_PAW_KICK:        
                gSelectedKick = (sSelKick.KT_GRAB_DRIBBLE,gSelectedKick[1],gSelectedKick[2])

            gIsKickTriggering = True                        

    
    r = Constant.STATE_EXECUTING
    
    if gIsKickTriggering:  

        kickType, dkd, direction = gSelectedKick    

        # Setting isClockwise
        isClockwise = None 
        if direction == Constant.dCLOCKWISE: 
            isClockwise = True
        elif direction == Constant.dANTICLOCKWISE: 
            isClockwise = False


        if kickType == sSelKick.KT_GRAB_TURN_KICK:                            
            r = sGrabTurnKick.perform(False,\
                                      Action.DiveKickWT,\
                                      Constant.TARGET_GOAL_X,\
                                      Constant.FIELD_LENGTH)

        elif kickType == sSelKick.KT_GRAB_TURN_SHOOT:                      
            r = sGrabTurnKick.perform()

        elif kickType == sSelKick.KT_GRAB_DRIBBLE_GOAL:
            r = sGrabDribble.performToTargetGoal()

        elif kickType == sSelKick.KT_GRAB_DRIBBLE: 
            r = sGrabDribble.performToDKD(dkd)

        elif kickType == sSelKick.KT_TURN_KICK:                
            r = sTurnKick.perform(dkd,isClockwise=isClockwise)

        elif kickType == sSelKick.KT_UPENN_LEFT: 
            #print "grab upeen left"
            r = sUpennKick.perform(dkd,True)

        elif kickType == sSelKick.KT_UPENN_RIGHT:
            #print "grab upeen right" 
            r = sUpennKick.perform(dkd,False)    
        
        else:
            print "rAttacker.py : No appropriate kick for grab chosen???"
            r = Constant.STATE_FAILED       
    
    
    return r





def doKickWithoutGrab():
    global gIsKickTriggering
    
    kickType, dkd, _ = gSelectedKick    
    
    r = Constant.STATE_EXECUTING
    
    if kickType == sSelKick.KT_PAW_KICK:                                 
        r = sPawKick.perform()

    elif kickType == sSelKick.KT_UPENN_LEFT:            
        
        #print "non-grab upenn left"
        
        if gIsKickTriggering\
            or (hPWalk.isStepComplete() and sUpennKick.isUpennLeftOk()): 
            gIsKickTriggering = True                
            r = sUpennKick.perform(dkd,True)
        else: 
            doTrackBall()

    elif kickType == sSelKick.KT_UPENN_RIGHT:     
        
        #print "non-grab upenn right"       
        
        if gIsKickTriggering\
            or (hPWalk.isStepComplete() and sUpennKick.isUpennRightOk()):
            gIsKickTriggering = True 
            r = sUpennKick.perform(dkd,False)
        else:
            doTrackBall()

    elif kickType == sSelKick.KT_DRIBBLE: 
        r = sDribble.perform(dkd)

    # else paw kick through    
    else:                                             
        r = sPawKick.perform()

    return r


def showKickTypeIndicator(kickType): 
    # Do indicator business here. 
    if kickType == sSelKick.KT_UPENN_RIGHT: 
        Indicator.showFacePattern([2,0,0,0,0])  
    elif kickType == sSelKick.KT_UPENN_LEFT: 
        Indicator.showFacePattern([0,0,0,0,2]) 
    elif kickType == sSelKick.KT_PAW_KICK: 
        Indicator.showFacePattern([0,2,0,2,0]) 
    elif kickType == sSelKick.KT_DRIBBLE: 
        Indicator.showFacePattern([0,1,0,1,0]) 
    elif kickType == sSelKick.KT_TURN_KICK:
        Indicator.showFacePattern([1,0,1,0,1])    
    elif kickType == sSelKick.KT_GRAB_TURN_SHOOT: 
        Indicator.showFacePattern([2,0,2,0,2]) 
    elif kickType == sSelKick.KT_GRAB_TURN_KICK:
        Indicator.showFacePattern([0,2,2,2,0])
    elif kickType == sSelKick.KT_GRAB_DRIBBLE\
        or kickType == sSelKick.KT_GRAB_DRIBBLE_GOAL:
        Indicator.showFacePattern([0,1,1,1,0])     

