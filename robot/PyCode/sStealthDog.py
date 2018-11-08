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
#  $Id: ssStealthDog 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# sStealthDog - edited by Ted Wong.
#===============================================================================

"""
Stealth Dog.
"""




import Action
import Constant
import Global
import hMath
import Indicator
import sFindBall
import VisionLink


def DecideNextAction(): 
    isNeeded, leftAngle, rightAngle = isStealthDogNeededForBall()
    Indicator.setDefault()
    if isNeeded: 
        print "sStealthDog : leftAngle = ", leftAngle, " rightAngle = ", rightAngle
        performToBall(leftAngle,rightAngle) 
    else:
        if Global.ballD < 25:
            resetPerform()
        sFindBall.perform()
        
        
        
gLastStealth = 0

def resetPerform():
    global gLastStealth 
    gLastStealth = 0

def performToBall(leftAngle,rightAngle):
    sFindBall.perform(True)
    perform(leftAngle,rightAngle,Global.ballH)    
   
def perform(leftAngle,rightAngle,heading):
    global gLastStealth 

    forward = Action.MAX_FORWARD_NORMAL
    left = 0 
    turn = heading
    
    badHeading = (leftAngle + rightAngle) / 2.0
    trueHeading = hMath.normalizeAngle_180(Global.selfLoc.getHeading() + heading)
        
    if heading < badHeading + 10 * gLastStealth:  
        if True or not (trueHeading < -90 or trueHeading > 120): 
            stealthTurn = rightAngle - 45
            if stealthTurn < turn: 
                turn = stealthTurn 
            gLastStealth = 1
            Indicator.showFacePattern([3,3,0,0,0])
    else: 
        if True or not (trueHeading > -90 and trueHeading < 60):
            stealthTurn = leftAngle + 45; 
	    if stealthTurn > turn:
                turn = stealthTurn
	    gLastStealth = -1
            Indicator.showFacePattern([0,0,0,3,3])

    turnccw = hMath.CLIP(turn / 2.0, Action.MAX_TURN_NORMAL)
    Action.walk(forward,left,turnccw)                


# returns (boolean,leftAngle,rightAngle)
def isStealthDogNeededForBall(): 
    # if not using vision ball don't bother
    if Global.ballSource != Constant.VISION_BALL:
        return (False,0,0);

    return isStealthDogNeeded(Global.ballX, Global.ballY, Global.ballD);


def isStealthDogNeeded(targetX,targetY,targetD):
    id(targetX)
    id(targetY) 
    # if the target is close, then don't bother
    if targetD < 30: 
        return (False,0,0)
    
    leftAngle = None
    leftDist = None
    rightAngle = None
    rightDist = None
    maxDist = targetD#,50)
    for i in range(-90,90,100): 
        count, dist = VisionLink.getNoObstacleInHeading(i-5,i+5,50,int(maxDist),5)
        if count >= 10:
            if leftAngle == None or i > leftAngle: 
                leftAngle = i
                leftDist = dist
            if rightAngle == None or i < rightAngle:          
                rightAngle = i 
                rightDist = dist
    
    if leftAngle != None: 
        if abs(leftDist - rightDist) > 20\
            and leftDist > rightDist:
            leftAngle, rightAngle = rightAngle, leftAngle
        return (True,leftAngle,rightAngle)   

    return (False,0,0)

