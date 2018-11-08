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


# This kick is a port from 2003.
# This turn kick is calibrated for pg 25.

import Action
import Constant
import Global
import hMath
import sFindBall
import sGrab

# Fairly constant 
gApplyForwardUntilThisFromEnd = 8
gErrorMargin = 3
gForwardVector = 1
gHeadLiftTime = 17

gIsInStartPosition = False

# Head related global
gIsHeadLifting   = False
gHeadLiftCounter = 0

# Kick related global 
gKickDuration = 0
gKickCounter  = 0

gIsSlow = False
gDirection = None
gTurnAmount = None
gTurnDir = None

gLastFrameCalled = 0

# Step related global
gStep      = 0
gStartStep = 0



gCounter = 0
def DecideNextAction():
    global gCounter

    import Indicator
    Indicator.setDefault()    
    if sGrab.isGrabbed: 
        if perform(90,True) == Constant.STATE_SUCCESS:
            gCounter = 0

    elif gCounter < 50: 
        gCounter += 1 
        sFindBall.perform()
    else:     
        sGrab.perform()    


def resetPerform(): 
    global gIsInStartPosition
    global gDirection
    global gKickDuration
    global gKickCounter
    global gIsHeadLifting 
    global gHeadLiftCounter
    
    gIsInStartPosition = False
    gDirection = None  
    gKickDuration = 0  
    gKickCounter = 0
    gIsHeadLifting = False
    gHeadLiftCounter = 0

def isBallUnderChin():
    return gIsHeadLifting\
        or sGrab.isBallUnderChin()

            

def perform(dkdOrDirection=90,isDkd=True,isSlow=False,isClockwise=None):
    global gIsSlow
    gIsSlow = isSlow
        
    if isDkd: 
        return performDKD(dkdOrDirection,isClockwise)
    else:
        return performDirection(dkdOrDirection,isClockwise)   

# dkd must be global angle.
def performDKD(dkd,isClockwise):   
    selfH = Global.selfLoc.getHeading()
    direction = hMath.normalizeAngle_180(dkd-selfH) 
    return performDirection(direction,isClockwise)    

# direction must be relative angle.
def performDirection(direction,isClockwise):
    global gLastFrameCalled     
    global gDirection

    if not sGrab.isGrabbed:
        resetPerform()
        return Constant.STATE_FAILED      
    elif not isBallUnderChin():
        sGrab.resetPerform()
        resetPerform()
        return Constant.STATE_FAILED
    
    if gLastFrameCalled != Global.frame - 1:         
        resetPerform()
    gLastFrameCalled = Global.frame

    if gDirection == None:
        gDirection = hMath.normalizeAngle_180(direction)
        if isClockwise == None: 
            gDirection = direction
        elif isClockwise and gDirection > 0:
            gDirection = gDirection - 360
        elif not isClockwise and gDirection < 0: 
            gDirection = 360 + gDirection    
           
        print "TURNKICK Direction!: ", gDirection
    
    if gKickCounter == 0: 
        setTurnKick()
      
    doTurnKick()  
    
    if Global.vBall.isVisible() or gHeadLiftCounter > 12:
        resetPerform()
        sGrab.resetPerform()
        if (gDirection < 0):
            sFindBall.setHint(60, 60)
        else:
            sFindBall.setHint(60, 120)
        return Constant.STATE_SUCCESS   
    else:
        return Constant.STATE_EXECUTING       

def doTurnKick(): 
    global gIsInStartPosition
    global gIsHeadLifting
    global gHeadLiftCounter 
    global gKickCounter
    currentStep = Global.pWalkInfo.getCurrentStep()

    # Check if I am ready to execute the kick
    if not gIsInStartPosition: 
        if gStartStep - gErrorMargin <= currentStep <= gStartStep + (gErrorMargin+2):
            #print "StartStep : ", gStartStep, "  currentStep : ", currentStep            
            gIsInStartPosition = True
        else: 
            setTurnKickParams(forward=Action.MAX_FORWARD,left=-gTurnDir)
            return

    setTurnKickParams(left=-gTurnDir,turnccw=gTurnAmount*gTurnDir)    
    gKickCounter += 1      
    
    
    # Do head related business here
    if gKickCounter >= gKickDuration - gHeadLiftTime\
        and gStep - gErrorMargin <= currentStep <= gStep + (gErrorMargin+2):
        #print "Step : ", gStep, "  currentStep : ", currentStep
        gIsHeadLifting = True
    
    if gIsHeadLifting: 
        Action.setHeadParams(0,0,-10,Action.HTAbs_h)
        gHeadLiftCounter += 1   
        Action.closeMouth() 
   
    if gHeadLiftCounter < gApplyForwardUntilThisFromEnd: 
        Action.finalValues[Action.Forward] = gForwardVector
   
        
def setTurnKickParams(forward=0,left=0,turnccw=0):
    Action.openMouth()
    sGrab.moveHeadForward()
    Action.finalValues[Action.Tilty] = -55
    #Action.finalValues[Action.Panx] = -gTurnDir*20
    Action.walk(forward,left,turnccw,walkType=Action.NormalWalkWT,minorWalkType=Action.TurnKickMWT)
   

def setTurnKick(): 
    global gKickDuration
    global gTurnDir
    global gTurnAmount
    
    global gStep
    global gStartStep
        
    gTurnAmount = Action.MAX_TURN_NORMAL

    if gIsSlow:
        setSlowTurnKick()
        return 

    if gDirection < 0:
        gTurnDir = -1
        gStep = 30
        gStartStep = 5
        
        d = abs(gDirection)             
        if d <= 45: # calibrated for 45
            gKickDuration = 13
            gTurnAmount = 10
            
        elif 45 <= d <= 60: # calibrated for 60: 
            gKickDuration = 15    
            gTurnAmount = 10
            
        elif 60 <= d <= 110: # calibrated for 90:
            gKickDuration = 13

        elif 110 <= d <= 130: # calibrated for 120
            gKickDuration = 17
            
        elif 130 <= d <= 160: # calibrated for 145 
            gKickDuration = 20
            
        elif 160 <= d <= 200: # calibrated for 180
            gKickDuration = 30
            
        elif 200 <= d <= 225: 
            gKickDuration = 38
            
        elif 225 <= d <= 250:
            gKickDuration = 45
            
        else:
            gKickDuration = 55  

    else: 
        gTurnDir = 1
        gStep = 18
        gStartStep = 45
        
        d = abs(gDirection)
        if d <= 45: # calibrated for 45
            gKickDuration = 13
            gTurnAmount = 10
            
        elif 45 <= d <= 60: # calibrated for 60: 
            gKickDuration = 15    
            gTurnAmount = 10
            
        elif 60 <= d <= 110: # calibrated for 90:
            gKickDuration = 13

        elif 110 <= d <= 130: # calibrated for 120
            gKickDuration = 17
            
        elif 130 <= d <= 160: # calibrated for 145 
            gKickDuration = 20
            
        elif 160 <= d <= 200: # calibrated for 180
            gKickDuration = 30
            
        elif 200 <= d <= 225: 
            gKickDuration = 38
            
        elif 225 <= d <= 250:
            gKickDuration = 45
            
        else:
            gKickDuration = 55          

def setSlowTurnKick(): 
    global gKickDuration
    global gTurnDir
    global gTurnAmount
    
    global gStep
    global gStartStep
        
    gTurnAmount = Action.MAX_TURN_NORMAL

    if gDirection < 0:
        gTurnDir = -1
        gStep = 5
        gStartStep = 55
        
        d = abs(gDirection)             
        if d <= 45: # calibrated for 45
            gKickDuration = 10
            
        elif 45 <= d <= 60: # calibrated for 60: 
            gKickDuration = 12    
            
        elif 60 <= d <= 110: # calibrated for 90:
            gKickDuration = 13

        elif 110 <= d <= 130: # calibrated for 120
            gKickDuration = 17
            
        elif 130 <= d <= 160: # calibrated for 145 
            gKickDuration = 20
            
        elif 160 <= d <= 200: # calibrated for 180
            gKickDuration = 30
            
        elif 200 <= d <= 250: # calibrated for 225
            gKickDuration = 40
            
        else:
            gKickDuration = 50  

    else: 
        gTurnDir = 1
        gStep = 35
        gStartStep = 15
        
        d = abs(gDirection)
        if d <= 45: # calibrated for 45
            gKickDuration = 10
            
        elif 45 <= d <= 60: # calibrated for 60: 
            gKickDuration = 12    
            
        elif 60 <= d <= 110: # calibrated for 90:
            gKickDuration = 13

        elif 110 <= d <= 130: # calibrated for 120
            gKickDuration = 17
            
        elif 130 <= d <= 160: # calibrated for 145 
            gKickDuration = 20
            
        elif 160 <= d <= 200: # calibrated for 180
            gKickDuration = 30
            
        elif 200 <= d <= 250: # calibrated for 225
            gKickDuration = 40
            
        else:
            gKickDuration = 50      

"""
# This kick should not be called when grabbed.
STK_BALL_DIST = 13
STK_BALL_HEAD = 15
STK_BALL_PAN  = 10
STK_DURATION = 15 
gSTKTurnccw = 0
gSTKCounter = 0 
def performSmallTurnKick(isClockwise): 
    global gSTKTurnccw
    global gSTKCounter 
    
    if sGrab.isGrabbed: 
        return Constant.STATE_FAILED
     
    if Action.shouldIContinueKick():
        Action.continueKick()  
          
    #if gSTKCounter > 0: 
    #    sFindBall.perform(True)
    #    Action.walk(0,0,gSTKTurnccw)
    #    gSTKCounter -= 1
    #    if gSTKCounter == 0:
    #        return Constant.STATE_SUCCESS
    #    else:
    #        return Constant.STATE_EXECUTING
       
        
    elif Global.vBall.isVisible()\
        and Global.ballD < STK_BALL_DIST\
        and abs(Global.weightedVisBallHead) < STK_BALL_HEAD\
        and abs(Global.desiredPan) < STK_BALL_PAN: 
        if isClockwise:
            gSTKTurnccw = -65
        else:
            gSTKTurnccw = 65
        Action.forceStepComplete()
        Action.kick(Action.TestKickWT)
        gSTKCounter = STK_DURATION
        
    else:
        sFindBall.perform()

    return Constant.STATE_EXECUTING
"""    
    
    
    
    
    
    
    
    
    
    
    
    
            
