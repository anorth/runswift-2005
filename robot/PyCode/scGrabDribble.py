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


# IDEAS : 
# TODOS : Remove magic number!!!

import Action
import Constant
import Global
import hMath
import hTrack
import hWhere
import Indicator
import sFindBall
import sGrab
import VisionLink


targetPosition = (0,0)

grabCounter = 0
def DecideNextAction():
    global grabCounter 
    grabCounter -= 1
    
    #testKick()
    #return 
    """    
    Action.openMouth()
    sGrab.moveHeadForward()
    Action.walk(0,0,0,walkType=Action.NormalWalkWT,minorWalkType=Action.GrabForwardOnlyMWT)
    
    obsFront = VisionLink.getNoObstacleInBox(-25,100,25,20,Constant.MIN_OBSTACLE_IN_BOX,Constant.OBS_USE_LOCAL)
    obsLeft  = VisionLink.getNoObstacleInBox(-80,80,-40,-80,Constant.MIN_OBSTACLE_IN_BOX,Constant.OBS_USE_LOCAL)
    obsRight = VisionLink.getNoObstacleInBox(40,80,80,-80,Constant.MIN_OBSTACLE_IN_BOX,Constant.OBS_USE_LOCAL)
    
    print "Obstacle Front : ",obsFront, " Left : ", obsLeft, " Right: ", obsRight
    
    """
    #print "Camera Frame : ", Global.cameraFrame
    Indicator.setDefault()
    if not sGrab.isGrabbed or performToTargetGoal() != Constant.STATE_EXECUTING:         
        if grabCounter == 60:
            hTrack.panLow = True
            resetPerform()
            sGrab.resetPerform()
        
        if grabCounter > 0: 
            sFindBall.perform()
        else:
            sGrab.perform()
            
        grabCounter -= 1
            
    else:
        grabCounter = 60
    
    #if Global.lostBall == 1: 
    #    VisionLink.sendYUVPlane()
    

counter = 0
def testKick(): 
    global counter
    Indicator.setDefault()
    if counter < 90:
        Action.openMouth()
        moveHeadForward()
        #Action.walk(Action.MAX_FORWARD,0,0,walkType=Action.NormalWalkWT,minorWalkType=Action.GrabForwardOnlyMWT)
        Action.walk(Action.MAX_FORWARD,0,0,minorWalkType=Action.SkeFastForwardMWT)
        #Action.walk(0,0,0)
    elif Action.shouldIContinueKick(): 
        Action.kick(Action.BuBuFwdKickWT) 
        return
    elif counter < 90 + NU_FWD_DURATION:
        Indicator.showFacePattern([1,1,1,1,1])
        Action.kick(Action.BuBuFwdKickWT)
    else:
        counter = 0  
    
    counter += 1
    
gForceTimeElapse = False
gForceToTargetGoal = False
gTrustGpsGoal = True
gLeft = None

gTurningDir = None
gTurningCnt = 0
MAX_OVER_TURN = 50

# Last frame which used diagonal grab dribble.
gLastDiagFrame = 0

# Don't reset these two variables in resetPerform(), because these are used in sFindBall.
gLastTurnFrame = 0  
gLastTurnDir = 0

# Last seen visual goal information.
gLastVisTGoalFrame = 0
gLastVisTGoal = None
gLastSelfLoc = None

# 
gKickType = 0
gKickHead = 90
gLastKickFrame = 0
gLastBreakFrame = 0 
gLastLocaliseFrame = 0
gKickCounter = 0
gBreakCounter = 0
gLocaliseCounter = 0
gTrustGpsGoalCounter = 0

gLastEdgeFrame = 0
gEdgeCounter = 0

gDodgeCounter = 0

# kick selection type
SEL_OFFENSIVE = 1 # kick selection for offensive
SEL_DEFENSIVE = 2 # kick selection for defensive
SEL_MIDFIELD  = 3 # kick selection for mid field
SEL_BREAK     = 4 # forcing to break out
SEL_GPS       = 5 # just kick without checking

MIN_TIME_TO_DRIBBLE = 2500
MAX_TIME_TO_DRIBBLE = 3000

# Minimum obstacle
MIN_OBS_TO_DODGE = 100
MIN_OBS_TO_GAP   = 100

GAP_MIN = 30           # Min gap size (degrees)
GAP_MIN_INTENSITY = 30 # Min obs intensity to be considered
GAP_MIN_DIST = 10      # Min obs distance to be considered
GAP_MAX_DIST = -1


# Special kickt action type, a
AVOID_OWN_GOAL = 95
GAP_KICK = 96
FWD_STEP_SHOOT = 97
SIDE_STEP_LEFT_SHOOT = 98
SIDE_STEP_RIGHT_SHOOT = 99
JUST_SHOOT = 100



# Kick type duration
AVOID_OWN_GOAL_DURATION = 30
BREAK_DURATION = 15
BUBU_FWD_DURATION = 25
NU_FWD_DURATION = 16
HEADTAP_FWD_DURATION = 16
SOFT_TAP_DURATION = 16
FWD_STEP_SHOOT_DURATION = NU_FWD_DURATION + 15
SIDE_STEP_SHOOT_DURATION = FWD_STEP_SHOOT_DURATION + 15
UPENN_DURATION = 15
CHEST_PUSH_DURATION = 22

# Other durations
EDGE_DURATION = 15
TURN_DURATION = 15
DIAG_DURATION = 15
DODGE_DURATION = 23
LOCALISE_DURATION = 10
TRUST_GPS_GOAL_DURATION = 5
TRUST_VIS_GOAL_DURATION = 10


def resetPerform(): 
    global gForceTimeElapse
    global gForceToTargetGoal
    global gTrustGpsGoal
    
    global gTurningDir
    global gTurningCnt
    
    global gLastEdgeFrame
    global gEdgeCounter
    
    global gLastVisTGoalFrame
    global gLastVisTGoal
    global gLastSelfLoc 
    
    global gKickType
    global gKickHead
    
    global gLastKickFrame
    global gLastBreakFrame
    global gLastLocaliseFrame
    global gDodgeCounter 
    global gKickCounter 
    global gBreakCounter
    global gLocaliseCounter
    
    global gTrustGpsGoalCounter
    
    global gDodgeCounter
    
    gForceTimeElapse = False
    gForceToTargetGoal = False
    gTrustGpsGoal = True
    
    gTurningDir = None
    gTurningCnt = 0
     
    gEdgeCounter = 0 
    gDodgeCounter = 0   
        
    gLastVisTGoalFrame = 0
    gLastVisTGoal = None
    gLastSelfLoc = None
    
    gKickType = 0
    gKickHead = 90
    gLastKickFrame = 0
    gLastBreakFrame = 0
    gLastLocaliseFrame = 0
    
    gKickCounter = 0
    gBreakCounter = 0
    gLocaliseCounter = 0
    gTrustGpsGoalCounter = 0


def moveHeadForwardTight(): 
    Action.setHeadParams(0,-55,40,Action.HTAbs_h)

def moveHeadForward(): 
    Action.setHeadParams(0,-45,50,Action.HTAbs_h)  

    
def perform(forward,left,turnCCW,walkType=None,minorWalkType=None,checkForBallUnderChin=True): 
    if not sGrab.isGrabbed:    
        resetPerform()
        return Constant.STATE_FAILED 
    elif checkForBallUnderChin and not sGrab.isBallUnderChin():
        sGrab.resetPerform()
        resetPerform()
        return Constant.STATE_FAILED        

    Action.openMouth()
    moveHeadForward()
        
    wt = Action.SkellipticalWalkWT
    if forward != 0 and left != 0 and turnCCW != 0: 
        mwt = Action.GrabDribbleMWT
        
    elif forward != 0 and left != 0: 
        mwt = Action.GrabDribbleMWT
        
    elif forward != 0 and turnCCW != 0: 
        if abs(turnCCW) < 10:
            mwt = Action.SkeFastForwardMWT
        else:
            mwt = Action.SkeGrabDribbleMWT
                
    elif left != 0 and turnCCW != 0:       
        mwt = Action.SkeGrabDribbleMWT
        
    elif forward != 0: 
        mwt = Action.SkeFastForwardMWT        
        
    elif left != 0: 
        mwt = Action.SkeGrabDribbleMWT
        
    elif turnCCW != 0: 
        mwt = Action.SkeGrabDribbleMWT
        
    else: 
        mwt = Action.SkeGrabDribbleMWT
    
    if walkType != None: 
        wt = walkType
    if minorWalkType != None: 
        mwt = minorWalkType        
        
    Action.walk(forward,left,turnCCW,"sdd",wt,mwt)
    return Constant.STATE_EXECUTING
    


# A little complex function which tries dribble towards the goal
# and shoots if appropriate.
def performAttack(): 
    performToTargetGoal()
    
def performToTargetGoal():
    global gTurningDir
    global gTurningCnt
    
    global gLastVisTGoalFrame
    global gLastVisTGoal
    global gLastSelfLoc
    global gTrustGpsGoal
    global gTrustGpsGoalCounter
    global gForceTimeElapse
    
    Indicator.showFacePattern([0,3,3,3,0])
    
    if not sGrab.isGrabbed:    
        resetPerform()
        return Constant.STATE_FAILED           
    
    # If we are already doing edge behaviour, then continue.
    if gEdgeCounter > 0 and performEdge() == Constant.STATE_EXECUTING: 
        return Constant.STATE_EXECUTING
    
    # If we are already doing dodge behaviour, then continue.
    if gDodgeCounter > 0 and performDodge() == Constant.STATE_EXECUTING: 
        return Constant.STATE_EXECUTING
    
           
    if gForceTimeElapse\
        or (hMath.getTimeElapsed(sGrab.gLastGrabTime,VisionLink.getCurrentTime()) > MIN_TIME_TO_DRIBBLE
            and Global.frame - gLastTurnFrame >= TURN_DURATION):  
        #print "Camera Frame : ", Global.cameraFrame, " : over time!! 2500"                  
        return performTimeElapsedAction(SEL_OFFENSIVE)
    
    if hMath.getTimeElapsed(sGrab.gLastGrabTime,VisionLink.getCurrentTime()) > MAX_TIME_TO_DRIBBLE:
        #print "Camera Frame : ", Global.cameraFrame, " : over time!! 3000" 
        return performTimeElapsedAction(SEL_BREAK)       


    if Global.vTGoal.isVisible():
        gLastVisTGoalFrame = Global.frame
        gLastVisTGoal = Global.vTGoal.getCopy()
        gLastSelfLoc = Global.selfLoc.getCopy()
        gTurningDir = None
        gTurningCnt = 0
        
        minH, maxH = selectGap()
        padding = abs(maxH - minH) / 5.0
        
        # Our heading is within the gap for sure.
        if minH < -padding and padding < maxH\
            and Global.pWalkInfo.getCurrentMinorWalkType() != Action.GrabTurnOnlyMWT\
            and Global.frame - gLastTurnFrame >= TURN_DURATION: 
            
            gForceTimeElapse = True
            #print "Camera Frame : ", Global.cameraFrame, " : over time!! 3000"
            return performTimeElapsedAction(SEL_OFFENSIVE)
        
        # Perform dodge, when there's something in front of you.
        # May be dodge a goalie.
        elif performDodge() == Constant.STATE_EXECUTING:
            return Constant.STATE_EXECUTING   
        
        # Our heading is not within the gap and can detect the gap.        
        elif minH != 0 and maxH != 0:
            turnCCW = (minH + maxH) / 2.0
            
        # Our heading is not within the gap and can't detect the gap.
        else:
            turnCCW = Global.vTGoal.getHeading()
        
    elif Global.frame - gLastVisTGoalFrame < TRUST_VIS_GOAL_DURATION:
        if performDodge() == Constant.STATE_EXECUTING:
            return Constant.STATE_EXECUTING     
        turnCCW = gLastVisTGoal.getHeading()
            
    else: 
        if gTrustGpsGoal and performEdge() == Constant.STATE_EXECUTING: 
            return Constant.STATE_EXECUTING
        
        #print "turning",
        h = hMath.getHeadingToFaceAt(Global.selfLoc,\
                                     Constant.TARGET_GOAL_X,\
                                     Constant.TARGET_GOAL_Y) 
                              
        if abs(h) < 15: 
            #print ", turning with gps",
            
            # If we see many obstacle in front of us, it may explain why we are not seeing any goals.
            # Thus, do some dodging.            
            if gTrustGpsGoal\
                and performDodge() == Constant.STATE_EXECUTING:
                return Constant.STATE_EXECUTING 

            # Gps thinks, we are in front of the goal, but we can't see it. 
            # Thus, Gps is possibly wrong.      
            gTrustGpsGoalCounter += 1
            if gTrustGpsGoalCounter > TRUST_GPS_GOAL_DURATION: 
                gTrustGpsGoal = False
        
        # We still trust gps.  
        if gTrustGpsGoal: 
            turnCCW = h   
            #print ", trust gps goal",
        
        # Otherwise, do something smart.
        else: 
            
            # We tried both directions, we are completely lost...
            # Maybe, do localise?
            # At the moment, we just keep spinning...
            if gTurningCnt >= 2:
                if gTurningDir == Constant.dANTICLOCKWISE: 
                    turnCCW = -Action.MAX_TURN
                else:
                    turnCCW = Action.MAX_TURN

            # Otherwise, turn both directions to find the target goal.
            else:            
                if gTurningDir == None:
                    h1 = hMath.getHeadingToFaceAt(sGrab.gLastGrabSelfLoc,\
                                                  Constant.TARGET_GOAL_X,\
                                                  Constant.TARGET_GOAL_Y)
                    if h1 > 0: 
                        gTurningDir = Constant.dANTICLOCKWISE
                    else: 
                        gTurningDir = Constant.dCLOCKWISE

                
                if gTurningDir == Constant.dANTICLOCKWISE\
                    and h > MAX_OVER_TURN:
                    gTurningDir = Constant.dCLOCKWISE
                    gTurningCnt += 1

                elif gTurningDir == Constant.dCLOCKWISE\
                    and h < -MAX_OVER_TURN:
                    gTurningDir = Constant.dANTICLOCKWISE 
                    gTurningCnt += 1

                if gTurningDir == Constant.dANTICLOCKWISE: 
                    turnCCW = Action.MAX_TURN
                else:
                    turnCCW = -Action.MAX_TURN
         
    # If we are in outside third, use diagonal grab dribbling.
    selfX = Global.selfLoc.getX()
    if selfX < Constant.LEFT_GOAL_POST or selfX > Constant.RIGHT_GOAL_POST:
        left = hMath.CLIP(selfX-Constant.FIELD_WIDTH *0.5,Action.MAX_SKE_LEFT_STP*0.5)
    else:
        left = 0
                                 
    return performDribble(Action.MAX_FORWARD,left,turnCCW)
    
    
    
def performDefense(): 
    pass    


# This function which tries to dribble to the dkd. 
# minTimeToDribble -> minimum time we are going to grab dribble.
def performToDKD(dkd,minTimeToDribble=1500):     
    global gForceToTargetGoal
    global gForceTimeElapse 
    
    Indicator.showFacePattern([1,0,0,0,1])
    
    if not sGrab.isGrabbed: 
        resetPerform() 
        return Constant.STATE_FAILED            
    
    # If we are already doing edge behaviour, then continue.
    if gEdgeCounter > 0 and performEdge() == Constant.STATE_EXECUTING: 
        return Constant.STATE_EXECUTING
    
    # If we are already doing dodging behaviour, then continue.
    if gDodgeCounter > 0 and performDodge() == Constant.STATE_EXECUTING: 
        return Constant.STATE_EXECUTING        
        
    if gForceTimeElapse\
        or hMath.getTimeElapsed(sGrab.gLastGrabTime, VisionLink.getCurrentTime()) > MAX_TIME_TO_DRIBBLE:
        return performTimeElapsedAction(SEL_GPS)
        
    if performEdge() == Constant.STATE_EXECUTING: 
        return Constant.STATE_EXECUTING
                    
    turnCCW = hMath.normalizeAngle_180(dkd - Global.selfLoc.getHeading())
                 
    # If we are lined up to the dkd, do something smart.
    if abs(turnCCW) <= 10: 
                
        # Dodge, if we need to.
        if performDodge() == Constant.STATE_EXECUTING: 
            return Constant.STATE_EXECUTING

        # If we exceed the minTimeToDribble, then release the ball.
        if Global.frame - gLastTurnFrame >= TURN_DURATION\
            and hMath.getTimeElapsed(sGrab.gLastGrabTime, VisionLink.getCurrentTime()) > minTimeToDribble: 
        
            return performTimeElapsedAction(SEL_GPS)
                        
    # If we are in outside third, use diagonal grab dribbling.
    selfX = Global.selfLoc.getX()
    if selfX < Constant.LEFT_GOAL_POST or selfX > Constant.RIGHT_GOAL_POST:
        if (0 < dkd < 180):
            left = hMath.CLIP(selfX-Constant.FIELD_WIDTH *0.5,Action.MAX_SKE_LEFT_STP*0.5)
        else:
            left = hMath.CLIP(Constant.FIELD_WIDTH *0.5-selfX,Action.MAX_SKE_LEFT_STP*0.5)
    else:
        left = 0
        
    return performDribble(Action.MAX_FORWARD,left,turnCCW)

# This function which tries to dribble to the dkd. 
# minTimeToDribble -> minimum time we are going to grab dribble.
def performToPosition(targetPos = (0,0),minTimeToDribble=1500):     
    global gForceToTargetGoal
    global gForceTimeElapse
    global targetPosition
    pos = Global.selfLoc.getPos() 
    targetPosition = targetPos
    dkd = hMath.getHeadingBetween(pos[0],pos[1],targetPos[0],targetPos[1])
    performToDKD(dkd,minTimeToDribble)
    
# This function is called by performToTargetGoal, performToDKD.
# It is been refactored, because the above two functions run exactly the same code.
# It uses diagonal walk, if there's no obstacle towards the centre of the field.
# This decides what sort of grab dribble we should do.
# One level higher from a function, perform()
def performDribble(fwd,left,turnCCW):
    global gLastDiagFrame 
    global gLastTurnFrame, gLastTurnDir

    if abs(turnCCW) <= 30:                
        if left == 0\
            or hMath.getTimeElapsed(sGrab.gLastGrabTime,VisionLink.getCurrentTime()) > 2000\
            or (left > 0 and VisionLink.getNoObstacleBetween(-30,-20,-30,80) > MIN_OBS_TO_DODGE)\
            or (left < 0 and VisionLink.getNoObstacleBetween(30,-20,30,80) > MIN_OBS_TO_DODGE):
            if Global.frame - gLastTurnFrame < 8:
                r = perform(fwd,0,turnCCW,minorWalkType=Action.SkeGrabDribbleMWT)     
            else:
                r = perform(fwd,0,turnCCW)
        else:    
            gLastDiagFrame = Global.frame
            if Global.frame - gLastTurnFrame < 8:
                r = perform(fwd,left,turnCCW,minorWalkType=Action.SkeGrabDribbleMWT)     
            else:
                r = perform(fwd,left,turnCCW)
    else: 
        r = perform(0,0,turnCCW)
        gLastTurnFrame = Global.frame
        if turnCCW > 0: 
            gLastTurnDir = Constant.dANTICLOCKWISE
        else:
            gLastTurnDir = Constant.dCLOCKWISE
    
    return r


# This function executes behaviour which handles if we grab the ball at the field edges. 
def performEdge(): 
    global gLastEdgeFrame
    global gEdgeCounter 
    
    selfH = Global.selfLoc.getHeading()
        
    if gEdgeCounter > 0: 
        #print "edge behaviour executing"
        gEdgeCounter -= 1      
        gLastEdgeFrame = Global.frame              
        perform(-Action.MAX_FORWARD,0,0)
    
    elif Global.frame - gLastEdgeFrame < Constant.GRAB_TIMEOUT: 
        return Constant.STATE_FAILED
    
    elif (hWhere.isOnLEdge() and 120 <= selfH <= 240)\
        or (hWhere.isOnREdge() and (0 <= selfH <= 60 or selfH >= 300))\
        or (hWhere.isOnBEdge() and 210 <= selfH <= 330)\
        or (hWhere.isOnTEdge() and 30 <= selfH <= 150 and not Global.vTGoal.isVisible()): 
        #print "edge behaviour triggered"
        gEdgeCounter = EDGE_DURATION
        gLastEdgeFrame = Global.frame
        perform(-Action.MAX_FORWARD,0,0)

    else: 
        return Constant.STATE_FAILED
    
    return Constant.STATE_EXECUTING


# This function will try to dodge obstacles and shoot between the gaps.
def performGap(): 
    pass

# This function will try to dodge the nearby obstacle.
# If you are near the goal, dodge towards the goal. 
# If you are not, dodge towards the clear view. 
def performDodge():
    global gLastDiagFrame, gDodgeCounter, gLeft 
          
    obsFront = VisionLink.getNoObstacleInBox(-25,100,25,20,Constant.MIN_VIS_OBSTACLE_BOX,Constant.OBS_USE_LOCAL)
    
    if gDodgeCounter > 0: 
        gLastDiagFrame = Global.frame
        
        gDodgeCounter -= 1
        Indicator.showFacePattern([1,1,1,1,1])
        
        turnCCW = 0
        if Global.vTGoal.isVisible(): 
            turnCCW = Global.vTGoal.getHeading()
        elif Global.frame - gLastVisTGoalFrame < TRUST_VIS_GOAL_DURATION: 
            turnCCW = gLastVisTGoal.getHeading()
        turnCCW = hMath.CLIP(turnCCW,30)
        
        return perform(5,gLeft,turnCCW)
    
    elif obsFront > MIN_OBS_TO_DODGE: 
        gLastDiagFrame = Global.frame
        
        Indicator.showFacePattern([1,1,1,1,1])
        gDodgeCounter = DODGE_DURATION
                
        left = 0
        
        # Decide which way to dodge.
        # If you are in offensive half, 
        # then always dodge towards the center of the field.
        selfX, selfY = Global.selfLoc.getPos()
        if selfY > Constant.FIELD_LENGTH / 2.0:            
            if selfX > Constant.FIELD_WIDTH / 2.0: 
                left = Action.MAX_LEFT
            else:
                left = -Action.MAX_LEFT        
        
        # Otherwise, dodge towards outside?
        else:            
            if selfX > Constant.FIELD_WIDTH / 2.0: 
                left = hMath.getSign(Action.MAX_LEFT,selfX-Constant.FIELD_WIDTH/3.0)
            else: 
                left = hMath.getSign(Action.MAX_LEFT,selfX-2.0*Constant.FIELD_WIDTH/3.0)
                
        gLeft = left        
        return perform(5,left,0)        
    
    else:        
        return Constant.STATE_SUCCESS
        




    
def performTimeElapsedAction(selType):
    global gForceTimeElapse    
    global gKickType
    global gLastKickFrame
    global gLastBreakFrame
    global gLastLocaliseFrame
    global gKickCounter
    global gBreakCounter
    global gLocaliseCounter

    gForceTimeElapse = True

    if Action.shouldIContinueKick(): 
        #print "Waiting for Kick to activate ...", gKickCounter
        Action.continueKick()
        moveHeadForwardTight()
        
    elif gKickCounter > 0:   
        #print "Kicking ...", gKickCounter
        
        if gKickType == AVOID_OWN_GOAL:   
            if gKickCounter > 10:          
                perform(0,0,Action.MAX_TURN)
            elif gKickCounter > 8:
                perform(0,0,0)
            else: 
                perform(Action.MAX_FORWARD,0,0,minorWalkType=Action.SkeGrabDribbleMWT)
        
        elif gKickType == GAP_KICK:
            selfH = Global.selfLoc.getHeading()
            if abs(gKickHead - selfH) > 10:
                turnCCW = gKickHead - selfH
                perform(0,0,turnCCW)
                
                # resetting until we are lined up to the gap.
                gKickCounter = SOFT_TAP_DURATION
                                
            else: 
                gKickType = Action.SoftTapWT
                
                Action.kick(gKickType)
                perform(0,0,0)
                moveHeadForwardTight()            
                        
        elif gKickType == SIDE_STEP_LEFT_SHOOT\
            or gKickType == SIDE_STEP_RIGHT_SHOOT\
            or gKickType == FWD_STEP_SHOOT:

            # Do side step shoot...
            if gKickCounter > FWD_STEP_SHOOT_DURATION:
                left = Action.MAX_LEFT
                if gKickType == SIDE_STEP_RIGHT_SHOOT: 
                    left = -Action.MAX_LEFT 
                turnCCW = 0    
                if Global.vTGoal.isVisible(): 
                    turnCCW = Global.vTGoal.getHeading() / 2
                elif Global.frame - gLastVisTGoalFrame < TRUST_VIS_GOAL_DURATION: 
                    turnCCW = gLastVisTGoal.getHeading() / 2
                turnCCW = hMath.CLIP(turnCCW,30)   
                                     
                perform(5,left,turnCCW)               
            
            # Do forward step shoot...     
            elif gKickCounter > NU_FWD_DURATION:
                fwd = Action.MAX_FORWARD
                turnCCW = 0
                if Global.vTGoal.isVisible(): 
                    turnCCW = Global.vTGoal.getHeading() / 2
                elif Global.frame - gLastVisTGoalFrame < TRUST_VIS_GOAL_DURATION: 
                    turnCCW = gLastVisTGoal.getHeading() / 2
                turnCCW = hMath.CLIP(turnCCW,15)
                
                perform(fwd,0,turnCCW,minorWalkType=Action.SkeFastForwardMWT)
                                                                    
            # Do kick!    
            else:
                gKickType = Action.NUFwdKickWT
                
                Action.kick(gKickType)
                perform(0,0,0)
                moveHeadForwardTight()
        elif gKickType == JUST_SHOOT:
            selfH = Global.selfLoc.getHeading()
            if gKickHead - selfH > 5:
                turnCCW = (gKickHead - selfH) / 2
#                 if (turnCCW > 0):
#                     left = Action.MAX_LEFT
#                 else:
#                     left = -Action.MAX_LEFT
#                     
#                 perform(5,left,hMath.CLIP(turnCCW,30))
                perform(0,0,turnCCW)
                # resetting until we are lined up to the gap.
                gKickCounter = SOFT_TAP_DURATION
            else: 
                gKickType = Action.HandKickLeftWT
                
                Action.kick(gKickType)
                perform(0,0,0)
                moveHeadForwardTight()            
               
                
        else:      
    
            # Reset gps ball coordinates.
            # So when it goes back to find ball, it won't turn around and walk straight.        
            x, y = hMath.getPointRelative(Global.selfLoc.getX(),
                                          Global.selfLoc.getY(),
                                          Global.selfLoc.getHeading(),
                                          160)
            VisionLink.resetBall(x,y,0,0)        
        
            Action.continueKick()
            Action.openMouth()
     
        hTrack.panLow = False
        gKickCounter -= 1
        gLastKickFrame = Global.frame

    elif gBreakCounter > 0:                        
                
        # If we cannot see the ball, then reset gps ball coordinates.
        # So when it goes back to find ball, it won't turn around and walk straight.        
        if not Global.vBall.isVisible():
            x, y = hMath.getPointRelative(Global.selfLoc.getX(),
                                          Global.selfLoc.getY(),
                                          Global.selfLoc.getHeading(),
                                          50)
            VisionLink.resetBall(x,y,0,0)                  

        if gBreakCounter > 10:
            Action.setHeadParams(0,0,0,Action.HTAbs_h)
            Action.walk(Action.MAX_FORWARD,0,0,minorWalkType=Action.SkeFastForwardMWT)
        elif gBreakCounter > 5: 
            Global.lostBall = Constant.LOST_BALL_LAST_VISUAL
            sFindBall.perform(True)    
            rate = Action.SKE_FF_PG * 8.0 * 2.0 / 1000.0
            fwd = Action.MAX_SKE_FF_FWD_STP * 0.8 / rate                     
            Action.walk(fwd,0,0,minorWalkType=Action.SkeFastForwardMWT)   
            Action.openMouth()
        else:
            Global.lostBall = 8 - gBreakCounter + Constant.LOST_BALL_LAST_VISUAL
            sFindBall.perform() 
            Action.openMouth()                   
            
        hTrack.panLow = True    
        gBreakCounter -= 1
        gLastBreakFrame = Global.frame


    elif Global.frame - gLastKickFrame > 1\
        and Global.frame - gLastBreakFrame > 1: 
        
        #print "Kick!!!!"
        selectKick(selType)

    else:
        resetPerform()
        sGrab.resetPerform()
        return Constant.STATE_SUCCESS

    return Constant.STATE_EXECUTING 
        
 
# A decision tree that selects the kick from dribble type 
def selectKick(selType):
    global gKickType
    global gKickHead
    global gKickCounter
    global gBreakCounter
    
    #gBreakCounter = BREAK_DURATION    
    #gKickCounter = 0
    #return
        
    gBreakCounter = 0    
        
    # Choosing ball releasing approach by dribbleType
    # Selecting kick approach for offensive.
    if selType == SEL_OFFENSIVE:
    	    	
        if Global.vTGoal.isVisible():

            cx = Global.vTGoal.getCentroid()[0]
            width = Global.vTGoal.getWidth()
            minH, maxH = selectGap()
            padding = abs(maxH - minH) / 5.0            
                    
            # Within the gap
            if minH < -padding and padding < maxH:                      
                #gKickType = Action.NUFwdKickWT        
                #gKickCounter = NU_FWD_DURATION 
                gKickType = Action.HeadTapWT        
                gKickCounter = HEADTAP_FWD_DURATION 
                #print "within gap"               
            
            # Take side step and shoot
            elif minH < 0 < maxH:         
                if abs(minH) > abs(maxH):       
                    gKickType = SIDE_STEP_LEFT_SHOOT
                else:
                    gKickType = SIDE_STEP_RIGHT_SHOOT
                gKickCounter = SIDE_STEP_SHOOT_DURATION
                #print "side step shoot"
            
            # Check if the goal is in the view.           
	    # If so have a crack.
            elif cx - width / 2 <= Constant.IMAGE_WIDTH * 0.4\
                and Constant.IMAGE_WIDTH * 0.6 <= cx + width / 2\
                and Global.selfLoc.getY() < Constant.FIELD_LENGTH * 0.6:                            
                #gKickType = Action.NUFwdKickWT
                #gKickCounter = NU_FWD_DURATION 
                gKickType = Action.HeadTapWT        
                gKickCounter = HEADTAP_FWD_DURATION 
                #print "have a crack" 
            
            # gap is in right hand side, do Upenn left?
            elif minH < maxH < -45:
                gKickType = Action.UpennLeftWT
                gKickCounter = UPENN_DURATION
            
            # gap is in left hand side, do Upenn right?
            elif 45 < minH < maxH: 
                gKickType = Action.UpennRightWT
                gKickCounter = UPENN_DURATION    
               
            else:
		gBreakCounter = BREAK_DURATION
                #print "break out by visual"
                
        elif Global.frame - gLastVisTGoalFrame < TRUST_VIS_GOAL_DURATION:             
            cx = gLastVisTGoal.getCentroid()[0]
            width = gLastVisTGoal.getWidth()
            
            if cx - width / 2 <= Constant.IMAGE_WIDTH * 0.4\
                and Constant.IMAGE_WIDTH * 0.6 <= cx + width / 2\
                and Global.selfLoc.getY() < Constant.FIELD_LENGTH * 0.6:         
                #gKickType = Action.NUFwdKickWT        
                #gKickCounter = NU_FWD_DURATION   
                gKickType = Action.HeadTapWT        
                gKickCounter = HEADTAP_FWD_DURATION 
                #print "have a crack last visual goal"   
            else:
                gBreakCounter = BREAK_DURATION
                #print "break out by last visual"

        else:          
            gBreakCounter = BREAK_DURATION
            #print "break out by nothing!!!" 
    
    # Choose defensive kick strategy for grab dribble.
    elif selType == SEL_DEFENSIVE:
        
        # gap heading must be between the headings of the offensive field edges.
        bestGap = VisionLink.getBestGap(0,100, 
                                        GAP_MAX_DIST,GAP_MIN_DIST,
                                        GAP_MIN,GAP_MIN_INTENSITY)    
        bestH = 0
        if bestH != None:
            bestH = bestGap[2]
        selfX, selfY = Global.selfLoc.getPos() 
        selfH = Global.selfLoc.getHeading()        
        maxH = hMath.getHeadingToMe(selfX,selfY,selfH,0,Constant.FIELD_LENGTH*0.6)
        minH = hMath.getHeadingToMe(selfX,selfY,selfH,Constant.FIELD_WIDTH,Constant.FIELD_LENGTH*0.6)
        
        obsFront = VisionLink.getNoObstacleBetween(0,0,0,100)    
        
        #print "defensive: "
        #print "minH: ",minH
        #print "bestH: ", bestH
        #print "maxH: ",maxH
        #print "obsFront: ", obsFront
             
        if Global.vOGoal.isVisible(): 
            gKickType = AVOID_OWN_GOAL
            gKickCounter = AVOID_OWN_GOAL_DURATION
            gBreakCounter = BREAK_DURATION
        elif obsFront > MIN_OBS_TO_GAP\
            and bestGap != None and minH < bestH < maxH:
            #print "gap : selected!!!!!!!!" 
            gKickHead = selfH + bestH
            gKickType = GAP_KICK
            gKickCounter = SOFT_TAP_DURATION
            
        else:
            gKickType = Action.NUFwdKickWT
            gKickCounter = NU_FWD_DURATION
    
    # Choose mid-field kick strategy for grab dribble.
    elif selType == SEL_MIDFIELD: 
        
        # gap heading must be between the headings of the two goal box edges.
        bestGap = VisionLink.getBestGap(0,100, 
                                        GAP_MAX_DIST,GAP_MIN_DIST,
                                        GAP_MIN,GAP_MIN_INTENSITY)    
        bestH = 0
        if bestGap != None:
            bestH = bestGap[2]
        selfX, selfY = Global.selfLoc.getPos() 
        selfH = Global.selfLoc.getHeading()        
        maxH = hMath.getHeadingToMe(selfX,selfY,selfH,Constant.MIN_GOALBOX_EDGE_X,Constant.FIELD_LENGTH)
        minH = hMath.getHeadingToMe(selfX,selfY,selfH,Constant.MAX_GOALBOX_EDGE_X,Constant.FIELD_LENGTH)            
    
        obsFront = VisionLink.getNoObstacleBetween(0,0,0,100)     
         
        #print "midfield: "
        #print "minH: ",minH
        #print "bestH: ", bestH
        #print "maxH: ",maxH
        #print "obsFront: ", obsFront
                         
        if Global.vOGoal.isVisible():
            gKickType = AVOID_OWN_GOAL
            gKickCounter = AVOID_OWN_GOAL_DURATION
            gBreakCounter = BREAK_DURATION            
        elif obsFront > MIN_OBS_TO_GAP\
            and bestGap != None and minH < bestH < maxH: 
            #print "gap : selected!!!!!!!!!!!"
            gKickHead = selfH + bestH
            gKickType = GAP_KICK
            gKickCounter = SOFT_TAP_DURATION

        else:
            gBreakCounter = BREAK_DURATION
            
    elif selType == SEL_GPS:
        selfH = Global.selfLoc.getHeading() 
        selfX, selfY = Global.selfLoc.getPos()
        #hMath.getDistSquaredBetween(pos[0],pos[1],targetPos[0],targetPos[1])
        gKickHead = selfH #+ hMath.getHeadingToMe(selfX, selfY, selfH,targetPosition[0],targetPosition[1])
        gKickType = JUST_SHOOT
        gKickCounter = SOFT_TAP_DURATION
        
                    
    # Otherwise, just release the ball.    
    else:        
        gBreakCounter = BREAK_DURATION


    # If we were doing diagonal dribble recently, 
    # then step forward a little before we shoot. 
    if Global.frame - gLastDiagFrame < DIAG_DURATION\
        and gKickType == Action.NUFwdKickWT: 
        gKickType = FWD_STEP_SHOOT
        gKickCounter = FWD_STEP_SHOOT_DURATION


    # If we are kicking, then hold the ball tight.
    # Make sure you check for appropriate kick type to be called by Action.kick().
    # ..            
    if gBreakCounter == 0\
        and gKickType != AVOID_OWN_GOAL\
        and gKickType != FWD_STEP_SHOOT\
        and gKickType != SIDE_STEP_LEFT_SHOOT\
        and gKickType != SIDE_STEP_RIGHT_SHOOT\
        and gKickType != JUST_SHOOT\
        and gKickType != GAP_KICK: 
        Action.kick(gKickType)
        perform(0,0,0)
        moveHeadForwardTight()

    #print ""
    #print "gKickType : ", gKickType
    #print "gBreakCounter : ", gBreakCounter
    #print "" 
                                    


# Copied from sGrabTurnKick
gSelectedGap = None
# Choose which gap to use. If there is only one then that one. 
# If gSelectedGap is not None the it's that one, else the largest.
# If setSelected is True and gSelectedGap is None and there is more than
# one gap then sets selectedGap appropriately
def selectGap(setSelected = False):
    global gSelectedGap
    (lmin, lmax, rmin, rmax) = VisionLink.getHeadingToBestGap()
    if (rmin == rmax == 0):
        if gSelectedGap != None:
            VisionLink.sendYUVPlane()
            #print "gap selected but only one seen"
        return (lmin, lmax)
    elif (lmin == lmax == 0):
        if gSelectedGap != None:
            VisionLink.sendYUVPlane()
            #print "gap selected but only one seen"
        return (rmin, rmax)

    if gSelectedGap == Constant.LEFT:
        return (lmin, lmax)
    elif gSelectedGap == Constant.RIGHT:
        return (rmin, rmax)
    elif abs(lmax - lmin) > abs(rmax - rmin):
        if setSelected:
            gSelectedGap = Constant.LEFT
            #print "selectGap locking LEFT"
        return (lmin, lmax)
    else:
        if setSelected:
            gSelectedGap = Constant.RIGHT
            #print "selectGap locking RIGHT"
        return (rmin, rmax)
