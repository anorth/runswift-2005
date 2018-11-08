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


# TODO : detect you are stuck and hussle more!!!! pressure the opponent beside you!!


import Action
import Constant
import Debug
import Global
import hMath
import hTrack
import math
import sFindBall
import VisionLink




firedType = 0
fired1 = 0
fired2 = 0
failed1 = 0 
failed2 = 0

CLOSE_DIST = 40          # Slowed down

if Global.penaltyShot or Global.lightingChallenge:
    CLOSE_DIST = 60

REALLY_CLOSE_DIST = 17
GRAB_DIST = 17                  # How far away when we attempt to grab
BALL_AGAINST_CHEST_DIST = 5.6   # Perceived ball dist when touching

# Thresholds for safe grabbing
CLOSE_DIST_SAFE = 34
REALLY_CLOSE_DIST_SAFE = 24
REALLY_CLOSE_SPEED_SAFE = 5
GRAB_DIST_SAFE = 13

# The distance from ball to set time critical
TIME_CRITICAL_DIST = 40

GRAB_FORWARD_TIME = 7
GRAB_NO_CHECK_TIME = 11
GRAB_COMPLETE_TIME = 25 # How many frames it takes to do a grab
OPEN_MOUTH_TIME = 6     # at what grabbingCount do we open mouth

grabbingCount = 0       # How many frames spent doing the grab

isGrabbed = False       # global variable to tell whether the ball is grabbed

gLastGrabSelfLoc = None
gLastGrabTime = (0,0)   # The time when the ball was grabbed.


gLastGrabBallH = 0
gLastGrabWeightedBallH = 0


# Ball under chin related values.
gLastCheckCalledFrame = 0 # The last frame which we called isBallUnderChin()
grabCount = 0             # The number of times isBallUnderChin() is called
gSensors = []             # The last 10 chest sensor values
    
    
    
#if the ball is under chin, this is the max angle the mouth can do
GRABBING_MOUTH_OPEN = -954000


def frameReset(): 
    # Hack for when the robot has grabbed the ball.
    if isGrabbed:  
        # The ball x, y should be a little bit in front from the robot,
        # so that kick selection won't stuff up.
        Global.ballX, Global.ballY = hMath.getPointRelative(Global.selfLoc.getX(),
                                                            Global.selfLoc.getY(),
                                                            Global.selfLoc.getHeading(),
                                                            Constant.BallRadius)        
        Global.ballH = 0
        Global.haveBall = 1
        Global.lostBall = 0
        VisionLink.resetBall(Global.ballX,Global.ballY,0,0)
        VisionLink.setIfGPSVisionUpdate(0)
        VisionLink.setGrabbed()        
    else: 
        VisionLink.setIfGPSVisionUpdate(1)    

pauseCounter = 0
def DecideNextAction(): 
    global pauseCounter
    global fired1, fired2
       
    """
    sFindBall.perform(True) 
    if Action.finalValues[Action.HeadType] == Action.HTAbs_xyz: 
        Action.finalValues[Action.HeadType] = Action.HTAbs_xyz_hack              
    Action.walk(0,0,0,minorWalkType=Action.SkeFastForwardMWT)     
    print "heading : ", Global.ballH
    print "weightedVisBallHead : ", Global.weightedVisBallHead
    
    ballX = math.sin(hMath.DEG2RAD(Global.ballH)) * Global.ballD
    ballY = math.cos(hMath.DEG2RAD(Global.ballH)) * Global.ballD
    print "ballX ", ballX
    print "ballY ", ballY
    
    Action.walk(15,0,0,minorWalkType=Action.SkeFastForwardMWT)
    
    return 
    
    pauseCounter += 1
    if pauseCounter < 300: 
        return    
    elif pauseCounter < 350:
        global grabbingCount
        if pauseCounter == 300:
            grabbingCount = 1
        perform()
    else:
        pauseCounter = 0
        resetPerform()
    
    return 
    """
    
     
    if pauseCounter > 0:
        if pauseCounter > 60: 
            if perform() == Constant.STATE_FAILED:
                resetPerform()
                pauseCounter = 61
            
        else:
            if pauseCounter == 60:
#                 print ""
#                 print "stats!!!!!"
#                 print "fired1 : ", fired1, ", failed1 : ", failed1, ", success rate : ", ((fired1 - failed1 + 0.0) / (fired1 + 0.000001))
#                 print "fired2 : ", fired2, ", failed2 : ", failed2, ", success rate : ", ((fired2 - failed2 + 0.0) / (fired2 + 0.000001))
#                 print "last fired type : ", firedType
#                 print ""
                resetPerform()
            sFindBall.perform()
        pauseCounter -= 1   
        return 
    r = perform() 
    if r == Constant.STATE_SUCCESS: 
        pauseCounter = 120
    elif r == Constant.STATE_FAILED:
        if firedType == 1: 
            fired1 -= 1
        elif firedType == 2: 
            fired2 -= 1
            
    #if Global.haveBall > 1 and abs(Global.vBall.getDistance() - Global.lastVisBall.getDistance()) > 80: 
    #    VisionLink.sendYUVPlane() 


#
def isBallUnderChin():    
    # Using chin
    #if VisionLink.getAnySensor(Constant.ssMOUTH) > GRABBING_MOUTH_OPEN: 
    #    #print "isBallUnderChin() ", Global.frame, " True ", VisionLink.getAnySensor(Constant.ssMOUTH)
    #    return True
    #else:
    #    #print "isBallUnderChin() ", Global.frame, " False ", VisionLink.getAnySensor(Constant.ssMOUTH)
    #    return False

    # Using chest.
    global failed1, failed2    
    global grabCount, gSensors
    global gLastCheckCalledFrame 
    
    # If we have already called this function in this frame, 
    # then don't add the sensor value.
    if gLastCheckCalledFrame == Global.frame: 
        if len(gSensors) >= 10 and sum(gSensors) / len(gSensors) < 130000:
            return False
        else:
            return True
            
    gLastCheckCalledFrame = Global.frame

    grabCount += 1

    # ignore first 5 frames
    if grabCount > 5: 
        gSensors.insert(0,VisionLink.getAnySensor(Constant.ssCHEST_INFRARED))
        if len(gSensors) > 10: 
            gSensors.pop()
        #print "grabCount : ", grabCount, ", Sensor : ", VisionLink.getAnySensor(Constant.ssCHEST_INFRARED)
        if len(gSensors) >= 10 and sum(gSensors) / len(gSensors) < 130000: # average over 10 frames
            if grabCount < 50:
                if firedType == 1: 
                    failed1 += 1
                elif firedType == 2: 
                    failed2 += 1
                #print "Grab failed early!!"
                                
            sFindBall.setHint(30,90)
            #print "Grab failed : grabCount : ", grabCount, ", cameraFrame : ", Global.cameraFrame
            #print ""
            #print ""
            return False
    return True
    

def moveHeadForwardTight(): 
    Action.setHeadParams(0,-50,40,Action.HTAbs_h)
    
def moveHeadForward():
    #Action.setHeadParams(0,-70,60,Action.HTAbs_h)
    Action.setHeadParams(0,-45,50,Action.HTAbs_h)
    #Action.setHeadParams(0,-55,45,Action.HTAbs_h)


def resetPerform(): 
    global grabbingCount
    global grabCount
    global gSensors
    global isGrabbed
    
    grabbingCount = 0
    grabCount = 0
    gSensors = []
    isGrabbed = False    
    Action.closeMouth()


def perform(safe = False, debugAlone = False, doGetBehindv = None):
    global grabbingCount 
      
    id(safe)
    id(debugAlone)  
      
    if doGetBehindv == None: 
        doGetBehindv = sFindBall.GET_BEHIND_DEFAULT  
      
    #if Debug.grabbingDebug:
    #    print "Ball distance:", Global.ballD
    
    #print ""
    #print "Camera Frame : ", Global.cameraFrame
    
    if isGrabbed:
        
        # If the ball is not 
        if not isBallUnderChin():
            resetPerform() 
            return Constant.STATE_FAILED
            
        grabbingCount = 0
        moveHeadForward()
        Action.openMouth()
        Action.walk(Action.MAX_FORWARD,0,0,minorWalkType=Action.SkeFastForwardMWT)    
        
    elif grabbingCount > 0 or canDoGrab(safe):
        r = grabBall()            
        if r == Constant.STATE_FAILED: 
            return Constant.STATE_FAILED
          
    # if the ball is close and not lost,
    # slow down as we approach the ball ready to grab
    else:
        approachBall(sGrabDoGetBehind = doGetBehindv)
        
    if isGrabbed:
        return Constant.STATE_SUCCESS
    else: 
        return Constant.STATE_EXECUTING


def grabBall(): 
    global grabbingCount 
    global isGrabbed
    global gLastGrabSelfLoc
    global gLastGrabTime
    
    debug("Grabbing!")

    # Hack lost ball here.
    Global.lostBall = 0
    
    # Force high gain!!
    Global.forceHighGain = True
          
    turnCCW = gLastGrabBallH * 1.2             
    Action.walk(Action.MAX_FORWARD,0,turnCCW,minorWalkType=Action.SkeNeckTurnWalkMWT)

    grabbingCount += 1
    
    if grabbingCount < GRAB_FORWARD_TIME:

        # If we can see the ball still and is lined up, delay the grabbingCount.
        # Experimenting!
        if Global.vBall.isVisible():
            ballX = math.sin(hMath.DEG2RAD(Global.ballH)) * Global.ballD
            ballY = math.cos(hMath.DEG2RAD(Global.ballH)) * Global.ballD
            if abs(ballX) < 0.5 and ballY < 8:
                grabbingCount = hMath.EXTEND(grabbingCount-1,1)

        Action.setHeadParams(0,-25,30,Action.HTAbs_h) 
        Action.closeMouth()  
        debug("Going for the ball")     
    
    elif grabbingCount < GRAB_NO_CHECK_TIME:
        moveHeadForward()
        Action.closeMouth()
        
    elif grabbingCount < GRAB_COMPLETE_TIME:
        moveHeadForwardTight()
        #Action.closeMouth()
        Action.openMouth()
        
        if Global.vBall.isVisible(): 
            #print "wait i can see the ball still.." 
            VisionLink.clearTimeCritical()
            resetPerform()
            sFindBall.setHint(25,90)
            return Constant.STATE_FAILED

    else:
        moveHeadForward() 
        Action.openMouth()
                
        isGrabbed = True
        grabbingCount = 0
        gLastGrabSelfLoc = Global.selfLoc.getCopy()
        gLastGrabTime = VisionLink.getCurrentTime()            

        frameReset() 
        VisionLink.clearTimeCritical()
        
    return Constant.STATE_EXECUTING         

gLastApproachFrame = 0
gLastReallyCloseFrame = 0
gLastTurnFrame = 0
def approachBall(sGrabDoGetBehind = None):
    global gLastApproachFrame
    global gLastReallyCloseFrame
    global gLastTurnFrame

    resetPerform()        

    if sGrabDoGetBehind == None: 
        sGrabDoGetBehind = sFindBall.GET_BEHIND_DEFAULT

    # Setting default action values with sFindBall.
    # Action values are overwritten by the approach ball code below.
    sFindBall.perform(doGetBehind = sGrabDoGetBehind)
    
    if Global.ballD < TIME_CRITICAL_DIST and Global.lostBall <= 3:
        VisionLink.setTimeCritical()
    else:
        VisionLink.clearTimeCritical()

    if Global.lostBall < Constant.LOST_BALL_GPS and Global.ballD < CLOSE_DIST:
        gLastApproachFrame = Global.frame
    
    if Global.frame - gLastApproachFrame < 5:
        VisionLink.setGrabbing()

        # Try to step just the right amount, but limit the minimum step
        # length. Reduce max to shorten step time and improve reaction time
        #vel = VisionLink.getGPSBallVInfo(Constant.CTLocal)        
        #velX, velY = vel[0], vel[1]
        
        rate = Action.SKE_FF_PG * 2.0 * 8.0 / 1000.0
        
        fwd = Global.ballD - BALL_AGAINST_CHEST_DIST        
        fwd = hMath.EXTEND(fwd,Action.MAX_SKE_FF_FWD_STP * 0.7) 
        # don't walk backward
        if fwd < 0: 
            fwd = 0
                
        turnCCW = Global.ballH   
        left = Global.ballD * math.sin(hMath.DEG2RAD(Global.ballH))
        
        #if left > 0: 
        #    left = hMath.CLIP(left,Action.MAX_SKE_FF_LEFT_STP * 0.8)  
        #else:
        #    left = hMath.CLIP(left,Action.MAX_SKE_FF_RIGHT_STP * 0.8)
        
        
        if Global.ballD < REALLY_CLOSE_DIST: 
            gLastReallyCloseFrame = Global.frame
        
        
        mwt = Action.SkeNeckTurnWalkMWT
        if Global.frame - gLastReallyCloseFrame < 5: 
            turnCCW = hMath.CLIP(turnCCW,30)
                          
            if abs(left) < 2:
                #print "branch 1"
                fwd = fwd / rate
                left = left / rate
                #turnCCW = hMath.CLIP(turnCCW * 0.4,30) / rate                            
                Action.walk(fwd,left,turnCCW,"sss",minorWalkType=mwt) 
            elif abs(left) < 8: 
                #print "branch 2"
                left = left * 1.2 / rate
                #turnCCW = hMath.CLIP(turnCCW * 0.4,30) / rate
                Action.walk(0,left,0,"sss",minorWalkType=mwt)
            else:
                #print "branch 3"
                #turnCCW = hMath.CLIP(turnCCW * 0.5,30) / rate                                                 
                turnCCW = turnCCW * 0.7                                                               
                Action.walk(0,0,turnCCW,minorWalkType=mwt)            
                gLastTurnFrame = Global.frame
                
        else:       
            if abs(turnCCW) < 30:
                #print "branch 5"
                #left = left * 0.6
                turnCCW = turnCCW * 1.1
                Action.walk(fwd,0,turnCCW,"ddd",minorWalkType=mwt)
                
            else:   
                if Global.ballD < 30:                   
                    #print "branch 6" 
                    turnCCW = turnCCW * 0.8
                    Action.walk(0,0,turnCCW,minorWalkType=mwt)                           
                else: 
                    Action.walk(fwd,0,turnCCW,minorWalkType=mwt)
                gLastTurnFrame = Global.frame 

                                       
        # Use the hacked version of HTAbs_xyz
        if Action.finalValues[Action.HeadType] == Action.HTAbs_xyz: 
            Action.finalValues[Action.HeadType] = Action.HTAbs_xyz_hack              
                    
        # Force high gain!!
        Global.forceHighGain = True
        
        # setting hTrack.panLow variable to true.
        hTrack.panLow = True

        #print ""
        #print "Camera Frame : ", Global.cameraFrame, ", Final Action Values : ", Action.finalValues
        #print ""
    
def canDoGrab(safe):
    global firedType, fired1, fired2
    global gLastGrabBallH
    global gLastGrabWeightedBallH
    id(safe)

    # Ball must be visible
    if Global.vBall.isVisible():
        ballD = Global.vBall.getDistance()
        ballH = Global.vBall.getHeading()
    else:
        return False
                
    can = True            
    
    # you are close to the ball
    can = can and 0 <= ballD <= GRAB_DIST
    
    # you are not turning right now!
    can = can and Global.frame - gLastTurnFrame > 6 # strictly speaking it's 11, because 23 * 8 * 2 / 1000 * 30
     
    # Check if the ball is really close (i.e. under the robot's chin.
    # Copied from hTrack.clipFireballPan()
    ballX = math.sin(hMath.DEG2RAD(ballH)) * ballD        
    ballY = math.cos(hMath.DEG2RAD(ballH)) * ballD
    
    ballX1 = math.sin(hMath.DEG2RAD(Global.weightedVisBallHead)) * Global.weightedVisBallDist
    #ballY1 = math.cos(hMath.DEG2RAD(Global.weightedVisBallHead)) * Global.weightedVisBallDist
    
    #can = can and abs(ballX) < 2.5        
    can = can and ballY < 15
    
    #can = can and abs(ballX1) < 2.5
    #can = can and ballY1 < 15
    
    if abs(ballX) < 0.5 and ballY < 8: 
        gLastGrabBallH = 0
        gLastGrabWeightedBallH = 0
        if can:
            firedType = 1
            fired1 += 1        
    else:
        can = can and abs(Global.weightedVisBallHead) < 8        
        can = can and abs(Global.ballH) < 8
        can = can and abs(Global.weightedVisBallHead - Global.ballH) < 6
        #can = can and abs(Global.fstVisBallHead - Global.thdVisBallHead) < 6 
        
        gLastGrabBallH = Global.ballH
        gLastGrabWeightedBallH = Global.weightedVisBallHead    

        if can: 
            firedType = 2
            fired2 += 1 
    
          
    # Force step complete, if all our grabbing criteria satisfies.
    # But, does this work?   
    if can: 
        Action.forceStepComplete()
   
    if False and can:
        vel = VisionLink.getGPSBallVInfo(Constant.CTLocal)        
        velx, vely = vel[0], vel[1]
    
        print ""
        print "Camera Frame : ",  Global.cameraFrame
        print "ballX : ", ballX, "  ballY : ", math.cos(hMath.DEG2RAD(ballH)) * ballD
        print "gps ball d : ", Global.gpsLocalBall.getDistance() 
        print "fst vis ball d : ", Global.fstVisBallDist, ", h : ", Global.fstVisBallHead
        print "snd vis ball d : ", Global.sndVisBallDist, ", h : ", Global.sndVisBallHead
        print "thd vis ball d : ", Global.thdVisBallDist, ", h : ", Global.thdVisBallHead
        print "Global.weightedVisBallHead : ", Global.weightedVisBallHead
        print "Global.ballH : ", Global.ballH       
        print "vel : ", velx, ",", vely 
        print "Since turned : ", Global.frame - gLastTurnFrame
        print "lostball : ", Global.lostBall
        print "Grab decision true : ", can 
        print "fired Type : ", firedType
        print ""
       
    return can

    


# debugging output
def debug(*toks):
    if Debug.grabbingDebug:
        for tok in toks:
            print tok,
        print
