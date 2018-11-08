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
# rAttacker.py
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
rAttacker
"""
import Action
import Constant
import Global

import hMath
import hPWalk
import hTeam
import hTrack
import hWhere
import hFWHead
import hStuck
import Indicator
#import math
import sActiveLocalise
import sAvoidOwnGoal

import sDodgyDog
import sDribble
import sFindBall
import sGetBehindBall
import sGrab
import sGrabDribble
import sGrabTurnKick
import sPawKick
import sSelKick
import sTurnKick
import sUpennKick
import sBlock

import VisionLink

#===============================================================================
# Functions belong to this module.
#===============================================================================

LOCALISE_ACTIVATE_BALL_RANGE = 60
LOCALISE_DEACTIVATE_BALL_RANGE = 50 
LOCALISE_TIMER = 8
gLastActivelyLocalisedFrame = 0
gActiveLocaliseTimer = 0

gLastFrameCalled = 0
gSelectedKick = None
gSelectedGetBehind = None # (desiredGetBehindDirection,direction)

#gUseGetBehind = False
gUseKickIndicator = True
gUseDodgyDog = True

# Variable lighting challenge state
playMode = False
isGoal = False

lastTrackBall = 0
startTrackBall = 0
lastGoToCenter = 0  
startGoToCenter = 0



def DecideNextAction():  
    #print "rAttacker: Global.selfLoc at", ("%x" % id(Global.selfLoc)),\
    #       "=", Global.selfLoc
    Indicator.setDefault()    
    perform()
    
    
def resetPerform(): 
    global gSelectedKick
    global gActiveLocaliseTimer 
        
    gSelectedKick = None
    gActiveLocaliseTimer = 0
    

def perform(params = None):
    VisionLink.startProfile("rAttacker.py")
    global gLastFrameCalled
    global gLastDecisionFunction
    global isGoal
    id(params) # ignore

    shouldIBeDodgy = gUseDodgyDog

    if gLastFrameCalled != Global.frame - 1: 
        resetPerform()
    gLastFrameCalled = Global.frame

    ### Variable lighting challenge ###
    if Global.lightingChallenge and playMode:
        if Global.vBall.isVisible(): 
            isGoal = False
        
        for pressSensor in range(1,4,1): # Used to react to back sensors
            if VisionLink.getPressSensorCount(pressSensor) > 8:
                print "It's a goal!!"
                isGoal = True
        
        if isGoal:
            VisionLink.resetBall(Constant.FIELD_WIDTH/2.0,
                                Constant.FIELD_LENGTH/2.0, 0.0, 0.0)
            pos = Global.selfLoc.getPos()
            if (hMath.getDistanceBetween(pos[0], pos[1],
                    Constant.FIELD_WIDTH/2.0, Constant.FIELD_LENGTH/2.0) < 50):
                sFindBall.perform()
            else:
                hTrack.stationaryLocalise()
                hTrack.saGoToTarget(Constant.FIELD_WIDTH/2.0,
                                    Constant.FIELD_LENGTH/2.0)
            return
        else:
            pass    # perform normal attacker
    elif Global.lightingChallenge:
        ObsMapping()
        return

    ### End variable lighting challenge ###

    ### Penalty Shooter ####
    if Global.penaltyShot:
        if Global.frame - Global.firstPlayFrame < 90:
            hTrack.stationaryLocalise()
            return
        elif Global.frame - Global.firstPlayFrame < 115:
            sFindBall.perform(True)
            return

    #print "rAttacker frame",
    
    # Last man attacker can block
#    if not hTeam.amIFurthestBack(ignoreRoles = []):
#        print "not furthest back"
#    elif sGrab.grabbingCount > 0:
#        print "grabbing count", sGrab.grabbingCount
#    elif sGrab.isGrabbed:
#        print "grabbed"
#    elif Global.frame - sGrabDribble.gLastActionFrame < 60:
#        print "grabdribbleaction", sGrabDribble.gLastActionFrame

##     if hTeam.amIFurthestBack(ignoreRoles = []) \
##            and not sGrab.grabbingCount > 0 \
##            and not sGrab.isGrabbed \
##            and hMath.getTimeElapsed(sGrab.gLastGrabTime, VisionLink.getCurrentTime()) > 7000 \
##            and not Global.penaltyShot \
##            and Global.frame - sGrabDribble.gLastActionFrame > 60 \
##            and Global.selfLoc.getHeading() < 180 \
##            and Global.selfLoc.getY() < Constant.FIELD_LENGTH * 0.7:

##         #print __name__, "candidate block"
##         #_, _, speed, dirDegrees, _, _ = VisionLink.getGPSBallVInfo(Constant.CTLocal)
##         #print "speed", speed

##         # This speed really needs to be 4.5 to remove all noise but then it
##         # nearly never blocks. I'm under pressure to keep it possible in the
##         # face of all the other checks above, hence 4
##         if sBlock.checkThenBlock(onlyVisualBall = True, minBallSpeed = 3,
##                                     minDist = 40, maxDist = 80,
##                                     bothSides = False, dontBlockCentre = True):
##             print __name__,  hMath.getTimeElapsed(sGrab.gLastGrabTime, 
##                                          VisionLink.getCurrentTime())
##             print __name__, Global.frame, "blocking, gLastActionFrame =", sGrabDribble.gLastActionFrame
##             print
##             return


    # Back off if stuck
    if hStuck.amIStuckForward()\
        and Global.ballD > 200\
        and Global.gpsLocalBall.getDistance() > 200\
        and not sGrab.isGrabbed\
        and sGrab.grabbingCount == 0\
        and Global.frame - sGrab.gLastApproachFrame >= 5\
        and (not Global.penaltyShot)\
        and (not Global.lightingChallenge):
        sFindBall.perform(True)
        return

    # Don't go into own goal box
    if hWhere.ballInOwnGoalBox(0)\
            and (not sGrab.isGrabbed)\
            and sGrab.grabbingCount == 0\
            and (not Global.penaltyShot)\
            and (not Global.lightingChallenge):
        #print "Ball in goal box"
        #print "avoiding"
        doAvoidGoalBox()
        hFWHead.DecideNextAction()
        return  # no dodgy
    
    
    if not shouldIEndActivelyLocalise()\
        or shouldIActivelyLocalise():        
        #print "localising"
        doActivelyLocalise()    

    elif shouldIKick():
        #shouldIBeDodgy = False  #Really?
        #print "kicking"
        doKick()     

    
    elif Global.lightingChallenge and shouldIGoToCenter():
        doGotoCenter()
 
    else:
        #print "tracking"
        doTrackBall()

    # Use dodgy dog if required
    fwdCmd = Action.finalValues[Action.Forward]
    shouldIBeDodgy = shouldIBeDodgy \
                    and (not sGrab.isGrabbed) \
                    and sGrab.grabbingCount == 0 \
                    and Global.frame - sGrab.gLastApproachFrame >= 5 \
                    and Global.ballD >= sGrab.CLOSE_DIST \
                    and Global.frame - sFindBall.gLastSpinFrame >= 15 \
                    and Global.frame - sGetBehindBall.gLastCalledFrame >= 5 \
                    and fwdCmd > Action.MAX_SKE_FWD_SPD/2 \
                    and fwdCmd > abs(Action.finalValues[Action.Left]) \
                    and sDodgyDog.shouldIBeDodgyToBall()
    if shouldIBeDodgy:
        #print "and dodging"
        sDodgyDog.dodgyDogToBall()

    VisionLink.stopProfile("rAttacker.py")

gLastBallX = 0 
gLastBallY = 0
gLastCloseFrame = 0
gCloseBallCounter = 0
def shouldIKick():        
    global gSelectedKick
    global gLastBallX
    global gLastBallY
    global gLastCloseFrame
    global gCloseBallCounter
    
    # If the ball distance is closer than this, then don't reselect kick,
    # because the ball heading will be inaccurate when the ball is up close.
    # I *think*
    CLOSE_DIST = 20
    
    if gSelectedKick != None\
        and (sGrab.isGrabbed
            or sGrab.grabbingCount > 0
            or Action.shouldIContinueKick()):
        #print "shouldIKick 1", str(sGrab.isGrabbed), "  ", str(gSelectedKick)
        return True
               
    
    if Global.vBall.isVisible():                      
        
        # Record when was the last close dist frame.
        if Global.ballD < CLOSE_DIST:
            gLastCloseFrame = Global.frame
            gCloseBallCounter += 1
            
        if gSelectedKick != None:                        
            # If the ball is close, then don't re-select kick.
            if Global.frame - gLastCloseFrame < 3:
                # If selected kick is dribble, then check if gCloseBallCounter is less than 1 sec.
                # If it's over, then reselect the kick.
                if gSelectedKick[0] == sSelKick.KT_DRIBBLE: 
                    if gCloseBallCounter < 15:
                        return True
                else:     
                    return True

            # If this chosen kick needs to grab and the ball is close,
            # then don't give up the grab.
            if sSelKick.doINeedToGrab(gSelectedKick[0])\
                and (Global.ballD < sGrab.CLOSE_DIST or Global.frame - sGrab.gLastApproachFrame < 5):
                
                # Check if you need to check for stuck detection.
                # If you are not stuck, don't reset kick selection.
                if not hStuck.gUseContestDetect or not hStuck.isBallContested():                                                    
                    #print "shouldIKick 3", str(sGrab.isGrabbed), "  ", str(gSelectedKick)
                    return True       

                                               
        # resetting kick selection.
        selectedKick = sSelKick.perform()
        
        # If the selected kick is different, then do kick reset. 
        if gSelectedKick != None and selectedKick[0] != gSelectedKick[0]:
            doKickReset()    
        gSelectedKick = selectedKick     
                
        gLastBallX = Global.ballX
        gLastBallY = Global.ballY
        #print "shouldIKick 4", str(sGrab.isGrabbed), "  ", str(gSelectedKick)
        return True
    
    # If we've lost the ball for a little bit, then keep the last kick selection.    
    elif gSelectedKick != None and Global.lostBall < Constant.LOST_BALL_GPS:
        return True
       
    Indicator.showFacePattern([1,1,1,1,1])        
    gSelectedKick = None        
    return False

                
def shouldIActivelyLocalise():     
    global gActiveLocaliseTimer
        
    # 1. The attacker has just span to find the ball.        
    # 2. The attacker has seen the ball for a few frames.
    # 3. The ball is still far away and straight ahead.
    # 4. sFindBall.gLastCloseFrame is not recent.
    # 5. sGrab.gLastApproachFrame is not recent.
    # 6. The attacker has not recently actively localised.
    if Global.frame - sFindBall.gLastSpinFrame > 60\
        and Global.haveBall > 3\
        and Global.ballD > LOCALISE_ACTIVATE_BALL_RANGE and abs(Global.ballH) < 10\
        and Global.frame - sFindBall.gLastCloseFrame >= 5\
        and Global.frame - sGrab.gLastApproachFrame >= 5\
        and Global.frame - gLastActivelyLocalisedFrame > 60:

        # If the ball is moving fast, then don't localise
        vel = VisionLink.getGPSBallVInfo(Constant.CTLocal)        
        velX, velY = vel[0], vel[1]
        if abs(velX) < 2 and abs(velY) < 2:

            # If no obstacle is around the ball.                    
            obs = VisionLink.getNoObstacleBetween(0,0,
                                                  int(Global.gpsLocalBall.getX()),
                                                  int(Global.gpsLocalBall.getY()),
                                                  30,50,Constant.OBS_USE_LOCAL)
            if obs < 100: 
            
                # Now try to do smart set beacon...
                if sActiveLocalise.SmartSetBeacon(70):
                    gActiveLocaliseTimer = LOCALISE_TIMER
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
    global gActiveLocaliseTimer
    
    gLastActivelyLocalisedFrame = Global.frame
       
    r = sActiveLocalise.perform()
    if r == Constant.STATE_FAILED: 
        gActiveLocaliseTimer = 0
        sFindBall.perform()
        return

    turnCCW = 0 
    if Global.vBall.isVisible() and abs(Global.ballH) < 20:
        turnCCW = Global.ballH
    Action.walk(Action.MAX_FORWARD,0,turnCCW,minorWalkType=Action.SkeFastForwardMWT)    
    Global.lostBall = 0 
    sFindBall.setForce(sFindBall.FORCE_LAST_VISUAL)   



def doTrackBall(headOnly=False):

    if Global.lightingChallenge:
        global lastTrackBall,startTrackBall
        if Global.frame - lastTrackBall > 1:
            startTrackBall = Global.frame
        lastTrackBall = Global.frame
 
    sFindBall.perform(headOnly) 
    

"""
def doGetBehindBall(): 
    global gSelectedGetBehind
    dkd = gSelectedGetBehind[0]
    direction = gSelectedGetBehind[1]

    r = sGetBehindBall.perform(dkd=dkd,\
                               direction=direction)    
    if r == Constant.STATE_SUCCESS\
        or r == Constant.STATE_FAILED: 
        gSelectedGetBehind = None
        sFindBall.perform()
    else:
        sFindBall.perform(True)
        if direction == Constant.dCLOCKWISE: 
            Indicator.showFacePattern([1,1,0,0,0])
        else:
            Indicator.showFacePattern([0,0,0,1,1])   
"""                 

# Walks to ball if far away. Close to the goal box this stands out of the way
# to leave open for the goalie to clear
def doAvoidGoalBox(ownGoalBox = True):    
    headToBall = hMath.normalizeAngle_0_360(Global.ballH + \
                                            Global.selfLoc.getHeading())

    ballH2OGoalH = hMath.getHeadingBetween(Global.ballX,Global.ballY,\
                                           Constant.OWN_GOAL_X,Constant.OWN_GOAL_Y)
                                           
    selfH2OGoalH = hMath.normalizeAngle_180(headToBall - ballH2OGoalH)

    xOffset = 50
    if (Global.selfLoc.getX() > Constant.FIELD_WIDTH/2 \
            and Global.ballX < Constant.FIELD_WIDTH/2) \
            or (Global.selfLoc.getX() < Constant.FIELD_WIDTH/2 \
            and Global.ballX > Constant.FIELD_WIDTH/2):
        xOffset += abs(Global.ballX - Constant.FIELD_WIDTH/2)/2

    # Far away? Just go as normal
    if not hWhere.inGoalBox(Global.selfLoc.getX(), Global.selfLoc.getY(),
                            ownGoalBox, Constant.GOALBOX_WIDTH):
        doTrackBall()

    # If next to the goal box walk to the corner of it
    elif ownGoalBox and Global.selfLoc.getY() < Constant.GOALBOX_DEPTH - 10:
        if Global.selfLoc.getX() < Constant.FIELD_WIDTH/2:
            hTrack.saGoToTargetFacingHeading(Constant.MIN_GOALBOX_EDGE_X,
                                            Constant.OWN_GOALBOX_EDGE_Y + 25,
                                            headToBall)
        else:
            hTrack.saGoToTargetFacingHeading(Constant.MAX_GOALBOX_EDGE_X,
                                            Constant.OWN_GOALBOX_EDGE_Y + 25,
                                            headToBall)

    elif (not ownGoalBox) and Global.selfLoc.getY() > Constant.TOP_GOALBOX_EDGE_Y + 10:
        if Global.selfLoc.getX() < Constant.FIELD_WIDTH/2:
            hTrack.saGoToTargetFacingHeading(Constant.MIN_GOALBOX_EDGE_X,
                                            Constant.TOP_GOALBOX_EDGE_Y - 25,
                                            headToBall)
        else:
            hTrack.saGoToTargetFacingHeading(Constant.MAX_GOALBOX_EDGE_X,
                                            Constant.TOP_GOALBOX_EDGE_Y - 25,
                                            headToBall)
    # Else if we are in front of goalbox walk to line up ouselves with
    # the ball and goal
    elif ownGoalBox:        
        # Move to the side of the ball, so we can localise as well.
        # May need hysterisis here... not to switch sides frequently.        
        if selfH2OGoalH < 0:
            adjX = xOffset
        else: 
            adjX = -xOffset
            
        hTrack.saGoToTargetFacingHeading(Global.ballX + adjX,
                                        Constant.OWN_GOALBOX_EDGE_Y + 20,
                                        headToBall)
    elif not ownGoalBox:
        # Move to the side of the ball, so we can localise as well.
        # May need hysterisis here... not to switch sides frequently.
        if selfH2OGoalH < 0:
            adjX = xOffset
        else: 
            adjX = -xOffset
            
        hTrack.saGoToTargetFacingHeading(Global.ballX + adjX,
                                        Constant.TOP_GOALBOX_EDGE_Y - 20,
                                        headToBall)
    

gIsKickTriggering = 0
gLastKickedFrame = 0
def doKick():
    global gSelectedKick
    global gIsKickTriggering
    global gLastKickedFrame
         
    # After you released the ball, don't grab for a second.
    if Global.frame - gLastKickedFrame < 75:
        sFindBall.perform(doGetBehind = sFindBall.GET_BEHIND_LOTS) 
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
        
        if kickType != sSelKick.KT_AVOID_OWN_GOAL\
            and r == Constant.STATE_SUCCESS:        
            gLastKickedFrame = Global.frame
            
        gIsKickTriggering = False
        gSelectedKick = None
        
        if kickType == sSelKick.KT_GRAB_DRIBBLE:
            pass        
        elif kickType == sSelKick.KT_DRIBBLE\
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

    if not sGrab.isGrabbed:   
        # If the ball is in the edge, do more get behind.
        if hWhere.ballOnEdge():
            r = sGrab.perform(sFindBall.GET_BEHIND_PRIORITY)
        else:            
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
            #print "kick reselected"
            # If the selected kick is different, then do kick reset. 
            if gSelectedKick != None and selectedKick[0] != gSelectedKick[0]: 
                doKickReset()    
            gSelectedKick = selectedKick

            # If kick selection returns paw kick, change it to 
            # grab dribble.
            # Because we can't paw kick or dribble from grab.
            kickType = gSelectedKick[0]
            if kickType == sSelKick.KT_DRIBBLE\
                or kickType == sSelKick.KT_PAW_KICK\
                or kickType == sSelKick.KT_AVOID_OWN_GOAL:        
                gSelectedKick = (sSelKick.KT_GRAB_DRIBBLE,gSelectedKick[1],gSelectedKick[2])
    
    r = Constant.STATE_EXECUTING
    
    
    if sGrab.isGrabbed:  
    
        if not sGrab.isBallUnderChin(): 
            return Constant.STATE_FAILED    

        kickType, dkd, direction = gSelectedKick    

        # Find a global co-ordinate on the dkd
        tx, ty = hMath.getPointRelative(Global.ballX, Global.ballY, dkd, 200)

        # Setting isClockwise
        isClockwise = None 
        if direction == Constant.dCLOCKWISE: 
            isClockwise = True
        elif direction == Constant.dANTICLOCKWISE: 
            isClockwise = False

        if kickType == sSelKick.KT_GRAB_TURN_KICK:                            
            r = sGrabTurnKick.perform(False, Action.HeadTapWT, tx, ty)
            #r = sGrabDribble.performToDKD(dkd, 0)
            
        elif kickType == sSelKick.KT_GRAB_TURN_SHOOT:                      
            r = sGrabTurnKick.perform()

        elif kickType == sSelKick.KT_GRAB_DRIBBLE_GOAL:
            r = sGrabDribble.performToTargetGoal()

        elif kickType == sSelKick.KT_GRAB_DRIBBLE_STOP:
            r = sGrabDribble.performToDKD(dkd, minTimeToDribble=2500,
                                          dontKick = True)

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
            print "rAttacker.py : No appropriate kick for grab chosen??? ", kickType
            r = Constant.STATE_FAILED       
    
    
    return r





def doKickWithoutGrab():
    global gIsKickTriggering
    
    kickType, dkd, _ = gSelectedKick    
    
    r = Constant.STATE_EXECUTING
    
    if kickType == sSelKick.KT_PAW_KICK:                                 
        r = sPawKick.perform()

    elif kickType == sSelKick.KT_DRIBBLE: 
        r = sDribble.perform(dkd)

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

    elif kickType == sSelKick.KT_AVOID_OWN_GOAL: 
        r = sAvoidOwnGoal.perform()       

    # else paw kick through    
    else:                                             
        r = sPawKick.perform()

    return r


def showKickTypeIndicator(kickType): 
    # Do indicator business here.
    if not gUseKickIndicator: 
        return
    
    if kickType == sSelKick.KT_AVOID_OWN_GOAL:
        Indicator.showFacePattern([3,0,0,0,3])
    elif kickType == sSelKick.KT_UPENN_RIGHT: 
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
    elif kickType == sSelKick.KT_GRAB_DRIBBLE and not sGrab.isGrabbed:
        Indicator.showFacePattern([0,1,2,1,0])


########################## Variable lighting challenge methods ###############
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
    
def doGotoCenter():
    global lastGoToCenter, startGoToCenter
    if Global.frame - lastGoToCenter > 1:
        startGoToCenter = Global.frame
    lastGoToCenter = Global.frame
    
    hTrack.stationaryLocalise()
    hTrack.saGoToTarget(Constant.FIELD_WIDTH/2.0,Constant.FIELD_LENGTH/2.0+80)
 
def shouldIGoToCenter():
    pos = Global.selfLoc.getPos()
    if (Global.frame - lastTrackBall > 1):
        return False
    elif ((Global.frame - lastTrackBall < 2) and \
            (lastGoToCenter - startGoToCenter > 60)):
        return False
    elif (lastTrackBall - startTrackBall > Constant.LOST_BALL_SPIN):
        return True
    elif (hMath.getDistanceBetween(pos[0], pos[1], Constant.FIELD_WIDTH/2.0, 
            Constant.FIELD_LENGTH/2.0+50) < 50):
        return False
    return False
 
