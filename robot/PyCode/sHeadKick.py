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
import sGetBehindBall
import sGrab


HEAD_OFFSET_ANGLE = 80

gLastFrameCalled = 0
gUseHeadLeft = True
gIsKickTriggering = False
gKickCounter = 0
gIsGetBehindNeeded = False
gTargetAngle = None

def DecideNextAction(): 
    import hFWHead
    if sGrab.perform() == Constant.STATE_SUCCESS:
        hFWHead.DecideNextAction()
        perform(90) 
    
    


def resetPerform(): 
    global gUseHeadLeft 
    global gIsKickTriggering
    global gKickCounter 
    
    gUseHeadLeft = True
    gIsKickTriggering = False
    gKickCounter = 0

def perform(dkd,useHeadLeft=None): 
    global gLastFrameCalled 
    global gUseHeadLeft    
    
    if gLastFrameCalled != Global.frame - 1: 
        resetPerform()
    gLastFrameCalled = Global.frame     
    
    if useHeadLeft != None: 
        gUseHeadLeft = useHeadLeft
    
    if sGrab.isGrabbed: 
        return performWithGrab() 
    else: 
        return performWithoutGrab(dkd)

def performWithGrab(): 
    global gKickCounter 
    
    if gKickCounter < 2: 
        Action.walk(0,0,0,walkType=Action.NormalWalkWT,minorWalkType=Action.GrabTurnOnlyMWT)
    
    elif gKickCounter < 20:
        if gUseHeadLeft:
            Action.kick(Action.HeadLeftWT)
        else:
            Action.kick(Action.HeadRightWT)
    
    else: 
        resetPerform()
        return Constant.STATE_SUCCESS        
    
    gKickCounter += 1
        
    return Constant.STATE_EXECUTING
    
def performWithoutGrab(dkd):
    global gKickCounter
    global gIsKickTriggering
 
    if (gUseHeadLeft and isHeadLeftOk())\
        or (not gUseHeadLeft and isHeadRightOk()): 
        gIsKickTriggering = True
    
    if gIsKickTriggering: 
        if gKickCounter < 15: 
            if gUseHeadLeft:
                Action.kick(Action.HeadLeftWT) 
            else:
                Action.kick(Action.HeadRightWT)
        else: 
            resetPerform() 
            return Constant.STATE_SUCCESS
        gKickCounter += 1

        return Constant.STATE_EXECUTING
        
    if gTargetAngle == None: 
        setGetBehind(dkd)
        
    if gIsGetBehindNeeded:
        sGetBehindBall.perform(gTargetAngle) 

    else: 
        sFindBall.perform()        
   
    gIsKickTriggering = False
    gKickCounter = 0

    return Constant.STATE_EXECUTING
    


def setGetBehind(dkd): 
    global gTargetAngle
    global gIsGetBehindNeeded     
    
    if gUseHeadLeft:
        gTargetAngle = dkd + HEAD_OFFSET_ANGLE
    else:
        gTargetAngle = dkd - HEAD_OFFSET_ANGLE                             

    gTargetAngle = hMath.normalizeAngle_0_360(gTargetAngle)    
    diff = hMath.normalizeAngle_0_360(Global.selfLoc.getHeading() - gTargetAngle)
    if abs(diff) < 20:
        gIsGetBehindNeeded = False    
    elif 0 <= Global.selfLoc.getHeading() <= 180:
        gIsGetBehindNeeded = False
    else:
        gIsGetBehindNeeded = True


def isHeadLeftOk(): 
    if not Global.vBall.isVisible():
        return False
        
    ballx = math.sin(hMath.DEG2RAD(Global.ballH)) * Global.ballD
    bally = math.cos(hMath.DEG2RAD(Global.ballH)) * Global.ballD        
    return -3.2 < ballx < 2.0\
           and bally < 3.4*Constant.BallRadius\
           and Global.ballD < 12\
           and abs(Global.pan) < 30

def isHeadRightOk(): 
    if not Global.vBall.isVisible():
        return False
    
    ballx = math.sin (hMath.DEG2RAD(Global.ballH)) * Global.ballD
    bally = math.cos (hMath.DEG2RAD(Global.ballH)) * Global.ballD        
    return -2.0 < ballx < 3.2\
        and bally < 3.4*Constant.BallRadius\
        and Global.ballD < 12\
        and abs(Global.pan) < 30
