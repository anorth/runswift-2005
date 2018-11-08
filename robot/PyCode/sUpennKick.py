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


import Action
import Constant
import Global 
import hMath 
import math
import sFindBall
import sGrab


DIST_TOO_FAR_TO_KICK = 40
UPENN_OFFSET_ANGLE = 50

gLastFrameCalled = 0


gUseUpennLeft = True

gKickCounter = 0
gIsKickTriggering = False
gIsGetBehindNeeded = False
gTargetAngle = None

EDGE_OFFSET = 80

GRAB_PAUSE_TIME = 2
UPENN_DURATION = 24

def DecideNextAction():  
    if isUpennLeftOk():
        sFindBall.perform(True)
        Action.kick(Action.UpennLeftWT)
    else:
        sFindBall.perform()
    
    #perform(90)  


def resetPerform(): 
    global gKickCounter
    global gIsKickTriggering     
    global gUseUpennLeft    
    global gTargetAngle    
 
    gKickCounter = 0
    gIsKickTriggering = False
    gUseUpennLeft = True   
    gTargetAngle = None

  
def perform(dkd,useUpennLeft=None):
    global gLastFrameCalled 
    global gUseUpennLeft    
    
    id(dkd)
    
    if gLastFrameCalled != Global.frame - 1: 
        resetPerform()
    gLastFrameCalled = Global.frame       
        
    if useUpennLeft != None: 
        gUseUpennLeft = useUpennLeft

    if sGrab.isGrabbed: 
        return performWithGrab()    
    else:
        return performWithoutGrab(dkd)
    
    
def performWithGrab(): 
    global gKickCounter 
    
    if gKickCounter < GRAB_PAUSE_TIME: 
        Action.walk(Action.MAX_FORWARD,0,0,walkType=Action.NormalWalkWT,minorWalkType=Action.GrabTurnOnlyMWT)
    
    elif gKickCounter == GRAB_PAUSE_TIME: 
        if gUseUpennLeft:
            Action.kick(Action.UpennLeftWT)
        else:
            Action.kick(Action.UpennRightWT)
    
    elif Action.shouldIContinueKick():
        sFindBall.perform(True)
        Action.continueKick()
        return Constant.STATE_EXECUTING
        
    elif gKickCounter < UPENN_DURATION:
        sFindBall.perform(True)    
        Action.continueKick() 
    
    else: 
        resetPerform()
        return Constant.STATE_SUCCESS        
    
    gKickCounter += 1
        
    return Constant.STATE_EXECUTING
    
    
def performWithoutGrab(dkd):
    global gKickCounter
    global gIsKickTriggering
    
    id(dkd)

    if (gUseUpennLeft and isUpennLeftOk())\
        or (not gUseUpennLeft and isUpennRightOk()):
        if gUseUpennLeft:
            Action.kick(Action.UpennLeftWT)
        else:
            Action.kick(Action.UpennRightWT)                 
        sFindBall.perform(True)
        gIsKickTriggering = True
        
    elif gIsKickTriggering:
        if Action.shouldIContinueKick(): 
            sFindBall.perform(True)
            Action.continueKick()
    
        elif gKickCounter < UPENN_DURATION:        
            sFindBall.perform(True)
            Action.continueKick()
            gKickCounter += 1
            
        else:                 
            sFindBall.perform(doGetBehind = sFindBall.GET_BEHIND_PRIORITY)
            resetPerform()
            return Constant.STATE_SUCCESS
    
    else:         
        sFindBall.perform()        
        gIsKickTriggering = False
        gKickCounter = 0

    return Constant.STATE_EXECUTING          


def setGetBehind(dkd): 
    global gTargetAngle
    global gIsGetBehindNeeded     
    
    if gUseUpennLeft:
        gTargetAngle = dkd + UPENN_OFFSET_ANGLE
    else:
        gTargetAngle = dkd - UPENN_OFFSET_ANGLE                             

    gTargetAngle = hMath.normalizeAngle_0_360(gTargetAngle)    
    diff = hMath.normalizeAngle_0_360(Global.selfLoc.getHeading() - gTargetAngle)
    if abs(diff) < 20:
        gIsGetBehindNeeded = False    
    elif 0 <= Global.selfLoc.getHeading() <= 180:
        gIsGetBehindNeeded = False
    else:
        gIsGetBehindNeeded = True
    
        
# --------------------------------------------------------------------
# condition for upenn suitability
def isUpennLeftOk():
    if not Global.vBall.isVisible():
        return False

    ballx = math.sin (hMath.DEG2RAD(Global.ballH)) * Global.ballD
    bally = math.cos (hMath.DEG2RAD(Global.ballH)) * Global.ballD

    maxBallRadiiAway = 4.5
    
    return -3.0 < ballx < 5.0\
        and bally < maxBallRadiiAway * Constant.BallRadius\
        and abs(Global.ballH) < 30
        
def isUpennRightOk():
    if not Global.vBall.isVisible():
        return False        

    ballx = math.sin (hMath.DEG2RAD(Global.ballH)) * Global.ballD
    bally = math.cos (hMath.DEG2RAD(Global.ballH)) * Global.ballD

    maxBallRadiiAway = 4.5
        
    return -5.0 < ballx < 3.0\
        and bally < maxBallRadiiAway * Constant.BallRadius\
        and abs(Global.ballH) < 30
            
