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


#===============================================================================
#   Python Behaviours : 2005 (c) 
#
#   Chest dribble
#===============================================================================

import Action
import Constant
import Global
import hMath
import hWhere
import hStuck
import Indicator
import math
import sFindBall
import sGetBehindBall
import sUpennKick
import VisionLink


CLOSE_DIST = 25
REALLY_CLOSE_DIST = 15

PAW_OFFSET_X = 7.0


NO_GET_BEHIND_DURATION = 60
gNoGetBehindCounter = 0


UPENN_DURATION = 30

gKickType = 0
gKickCounter = 0

# Dribble related kick types and durations
CHARGE = 1
TURN_LEFT_KICK = 2
TURN_RIGHT_KICK = 3

CHARGE_DURATION = 14
TURN_KICK_DURATION = 15



gLastKickedFrame = 0

def DecideNextAction():  
    print ""
    print "Camera Frame : ", Global.cameraFrame
    Indicator.setDefault() 
    Action.closeMouth()
    perform()      
    Indicator.showTrackingIndicator()
    print ""

    
    
def resetPerform(): 
    pass



# Performs the dribble. This does a getBehind first to line up if the robot
# is not already suitably positioned, then dribbles if the heading is within
# accuracy of the dkd
def perform(dkd = None, accuracy = 30):
    global gNoGetBehindCounter
    global gKickType
    global gKickCounter
    global gLastKickedFrame
    
    id(accuracy)
    
    gNoGetBehindCounter = max(gNoGetBehindCounter-1,0)
    
    if Action.shouldIContinueKick():
        sFindBall.perform(True) 
        Action.continueKick()
        return Constant.STATE_EXECUTING
        
    elif gKickCounter > 0: 
        sFindBall.perform(True)
        if gKickType == Action.UpennLeftWT or gKickType == Action.UpennRightWT:             
            Action.continueKick()
                  
        elif gKickType == CHARGE: 
            performCharge()
        elif gKickType == TURN_LEFT_KICK or gKickType == TURN_RIGHT_KICK: 
            performTurnKick(gKickType == TURN_LEFT_KICK)            
        else:
            sFindBall.perform()
            print "sDribble.perform : Unknown kick type : ", gKickType
        
        gLastKickedFrame = Global.frame        
        gKickCounter -= 1
        return Constant.STATE_EXECUTING 
    
    if Global.lostBall > Constant.LOST_BALL_GPS: 
        sFindBall.perform()
        return Constant.STATE_FAILED    

    if dkd == None:
        dkd = hMath.getHeadingBetween(Global.ballX,Global.ballY,\
                                      Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y+20)  
    selfH2BallH = Global.selfLoc.getHeading() + Global.ballH                         
    selfH2dkd = hMath.normalizeAngle_180(selfH2BallH - dkd)

    # We are close to the ball, be careful about what you are doing.
    ballD = Global.ballD 
    ballH = Global.ballH
    bx = ballD * math.sin(hMath.DEG2RAD(ballH))
    by = ballD * math.cos(hMath.DEG2RAD(ballH))
    
    # Sorry I'm lazy to call this function in every if statement,
    # So, I'll just call it once here.
    sFindBall.perform()    
    
    if Global.ballD < CLOSE_DIST\
        or Global.frame - gLastApproachFrame < 5:           
    
        if Global.frame - gLastKickedFrame > 30:   
            # The ball is contested.
            if hStuck.isBallContested():            
                # Use upenn to swipe the ball.
                if Global.vBall.isVisible() and 40 <= abs(selfH2dkd) <= 75: 
                    if selfH2dkd < 0 and sUpennKick.isUpennRightOk(): 
                        gKickType = Action.UpennRightWT
                        Action.kick(gKickType)
                        gKickCounter = UPENN_DURATION
                        return Constant.STATE_EXECUTING 
                    elif selfH2dkd > 0 and sUpennKick.isUpennLeftOk():
                        gKickType = Action.UpennLeftWT
                        Action.kick(gKickType)
                        gKickCounter = UPENN_DURATION
                        return Constant.STATE_EXECUTING                                                     
            else:                        
                # Use upenn to swipe the ball.
                if Global.vBall.isVisible() and 45 <= abs(selfH2dkd) <= 60: 
                    if selfH2dkd < 0 and sUpennKick.isUpennRightOk(): 
                        gKickType = Action.UpennRightWT
                        Action.kick(gKickType)
                        gKickCounter = UPENN_DURATION                    
                        return Constant.STATE_EXECUTING
                    elif selfH2dkd > 0 and sUpennKick.isUpennLeftOk():
                        gKickType = Action.UpennLeftWT
                        Action.kick(gKickType)
                        gKickCounter = UPENN_DURATION
                        return Constant.STATE_EXECUTING

                # Are we ok to charge?
                if abs(selfH2dkd) <= 30 and ballD < 15 and abs(bx) < 2 and by < 14:
                    gKickType = CHARGE 
                    gKickCounter = CHARGE_DURATION                
                    performCharge()
                    return Constant.STATE_EXECUTING

                # Are we ok to do turn kick?
                if False and abs(selfH2dkd) > 30 and ballD < 10 and abs(bx) < 2 and by < 5:                 
                    if selfH2dkd < 0:
                        gKickType = TURN_LEFT_KICK 
                    else:
                        gKickType = TURN_RIGHT_KICK
                    gKickCounter = TURN_KICK_DURATION
                    performTurnKick(gKickType == TURN_LEFT_KICK)                
                    return Constant.STATE_EXECUTING
        
        # Otherwise approach to ball.                                
        approachBall()            
                        
    else:                               
        
        if hStuck.isBallContested():
            if hWhere.ballOnEdge(65):                 
                r = sGetBehindBall.performBall(dkd)
                if r == Constant.STATE_SUCCESS:
                    sFindBall.perform(doGetBehind = sFindBall.GET_BEHIND_NONE) 
                    gNoGetBehindCounter = NO_GET_BEHIND_DURATION
            else:
                sFindBall.perform()        
        
        else:
            if gNoGetBehindCounter > 0:             
                sFindBall.perform(doGetBehind = sFindBall.GET_BEHIND_NONE)
            else:
                # Call sGetBehind explicitly.
                r = sGetBehindBall.performBall(dkd)
                if r == Constant.STATE_SUCCESS:
                    sFindBall.perform(doGetBehind = sFindBall.GET_BEHIND_NONE) 
                    gNoGetBehindCounter = NO_GET_BEHIND_DURATION               

      
    if Global.ballD < 15: 
        Action.openMouth()
        if Action.finalValues[Action.HeadType] == Action.HTAbs_xyz: 
            Action.finalValues[Action.HeadType] = Action.HTAbs_xyz_hack   
        
    return Constant.STATE_EXECUTING


def performCharge():
    Indicator.showFacePattern([0,0,3,0,0])
    if gKickCounter < 8:  
        Action.walk(Action.MAX_FORWARD,0,0,minorWalkType=Action.SkeFastForwardMWT)
    else:
        Action.walk(30,0,0,minorWalkType=Action.SkeFastForwardMWT) 

def performTurnKick(doTurnCCW=True):
    if doTurnCCW:
        Indicator.showFacePattern([3,0,0,0,0])
        Action.walk(25,0,45,minorWalkType=Action.SkeFastForwardMWT)
    else: 
        Indicator.showFacePattern([0,0,0,0,3])
        Action.walk(25,0,-45,minorWalkType=Action.SkeFastForwardMWT)
        
        

gLastApproachFrame = 0
def approachBall():
    global gLastApproachFrame

    sFindBall.perform()
     
    if Global.lostBall < Constant.LOST_BALL_GPS and Global.ballD < CLOSE_DIST:
        gLastApproachFrame = Global.frame    
    
    if Global.frame - gLastApproachFrame < 5:    
        VisionLink.setGrabbing()

        rate = Action.SKE_FF_PG * 2.0 * 8.0 / 1000.0

        fwd = Global.ballD       
        fwd = hMath.EXTEND(fwd,Action.MAX_SKE_FF_FWD_STP * 0.7) 
        # don't walk backward
        if fwd < 0: 
            fwd = 0
            
        turnCCW = Global.ballH   
        left = Global.ballD * math.sin(hMath.DEG2RAD(Global.ballH))
        
        if left > 0: 
            left = hMath.CLIP(left,Action.MAX_SKE_FF_LEFT_STP * 0.8)  
        else:
            left = hMath.CLIP(left,Action.MAX_SKE_FF_RIGHT_STP * 0.8)            
        
        mwt = Action.SkeNeckTurnWalkMWT
        if abs(left) < 2:  
            #print "branch 4"      
            Action.walk(fwd,left,0,"ddd",minorWalkType=mwt)
            Action.finalValues[Action.Forward] = 35

        elif abs(turnCCW) < 20:
            #print "branch 5"
            #left = left * 0.6
            turnCCW = turnCCW * 1.1
            Action.walk(fwd,0,turnCCW,"ddd",minorWalkType=mwt)
            Action.finalValues[Action.Forward] = 35

        else:           
            #print "branch 6"                
            turnCCW = turnCCW * 0.8  
            Action.walk(fwd,0,turnCCW,"ddd",minorWalkType=mwt) 
            
            
                    
def approachBall1():
    sFindBall.perform(True)

    ballD = Global.ballD 
    ballH = Global.ballH 
    
    bx = ballD * math.sin(hMath.DEG2RAD(ballH))
    by = ballD * math.cos(hMath.DEG2RAD(ballH))

    
    ballH2TGoalH = hMath.getHeadingBetween(Global.ballX,Global.ballY,
                                           Constant.TARGET_GOAL_X,
                                           Constant.TARGET_GOAL_Y)
                                         
    offsetH = hMath.normalizeAngle_0_360(ballH2TGoalH + 180)
    offsetD = 5   
    tx, ty = hMath.getPointRelative(bx,by,offsetH,offsetD)                             
    
    rate = Action.SKE_FF_PG * 2.0 * 8.0 / 1000.0
    fwd = ty / rate
    left = tx / rate 
    
    Action.walk(fwd,left,0,"ssd")




# This function and performPawKick() looks very similar...
# I should refactor this function
def performChestBump(): 

    if Global.lostBall > Constant.LOST_BALL_GPS:    
        sFindBall.perform()
        return Constant.STATE_FAILED
              
    ballH = Global.ballH
    ballD = Global.ballD
    ballX = ballD * math.sin(hMath.DEG2RAD(ballH))
    if ballD < 15:        
        sFindBall.perform(True)
        if abs(ballH) < 15:                
            
            # It's the right time to charge.                    
            if abs(ballX) < 2:
                Action.walk(Action.MAX_FORWARD,0,0,"ddd",minorWalkType=Action.SkeFastForwardMWT)                
            
            # Adjust our heading to the ball.    
            else:
                ballY = ballD * math.cos(hMath.DEG2RAD(ballH))                         
                x = hMath.CLIP(ballX,Action.MAX_SKE_LEFT_STP * 0.7)
                y = hMath.EXTEND(ballY,Action.MAX_SKE_FF_FWD_STP * 0.7)                            
                turnCCW = 0
                if Global.vTGoal.isVisible(): 
                    turnCCW = hMath.CLIP(Global.vTGoal.getHeading() / 2,30)
                    
                Action.walk(y,x,turnCCW,"ddd",minorWalkType=Action.SkeFastForwardMWT) 
        
        else:            
            turnCCW = hMath.CLIP(ballH,60)
            Action.walk(0,0,turnCCW,"ddd",minorWalkType=Action.SkeFastForwardMWT)
            
    else:
        sFindBall.perform(doGetBehind = sFindBall.GET_BEHIND_NONE)

    return Constant.STATE_EXECUTING        
        

def performPawKick():   
    global gKickType 
    global gKickCounter

    if Global.lostBall > Constant.LOST_BALL_GPS: 
        sFindBall.perform()
        return Constant.STATE_FAILED
    
    ballH = Global.ballH
    ballD = Global.ballD
    ballX = ballD * math.sin(hMath.DEG2RAD(ballH))
    if ballD < 15:        
        sFindBall.perform(True)
        if abs(ballH) < 30:                
            x1 = ballX - PAW_OFFSET_X
            x2 = ballX + PAW_OFFSET_X
            if abs(x1) < abs(x2): 
                x = x1
            else:
                x = x2
            
            if abs(x) < 2:
                Action.walk(Action.MAX_FORWARD,0,0,"ddd",minorWalkType=Action.SkeFastForwardMWT)
                                
            else:
                ballY = ballD * math.cos(hMath.DEG2RAD(ballH))                         
                x = hMath.CLIP(x,Action.MAX_SKE_LEFT_STP * 0.7)
                y = hMath.EXTEND(ballY,Action.MAX_SKE_FF_FWD_STP * 0.7)                            
                turnCCW = 0
                if Global.vTGoal.isVisible(): 
                    turnCCW = hMath.CLIP(Global.vTGoal.getHeading() / 2,30)
                    
                Action.walk(y,x,turnCCW,"ddd",minorWalkType=Action.SkeFastForwardMWT) 
        
        else:            
            turnCCW = hMath.CLIP(ballH,60)
            Action.walk(0,0,turnCCW,"ddd",minorWalkType=Action.SkeFastForwardMWT)
            
    else:
        sFindBall.perform(doGetBehind = sFindBall.GET_BEHIND_NONE)

    return Constant.STATE_EXECUTING
    

    
                           
        
    
