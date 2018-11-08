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
#  $Id: sHoverToBall.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#===============================================================================


"""
sHoverToBall
"""


import Action
import Constant
import Global
import hMath
import hWhere
import Indicator
import math


#--------------------------------------
# Hover to ball (or another relative point). Approach the ball expecting the
# ball is under chin when the ball is close.
def perform(dist = 0, head = 0, distSquared = False):

    if dist != 0:
        if not(distSquared):
            ballDSquared = hMath.SQUARE(dist)
        else:
            ballDSquared = dist
        ballH = head
    else:
        ballDSquared = hMath.SQUARE(Global.ballD)
        ballH = Global.ballH
    
    if abs(ballH) > 40:
        Action.walk(0, 0, ballH, minorWalkType=Action.SkeFastForwardMWT)
        return
    
    Action.stopLegs()
    
    slowDownDistance = 30

    # If (global) heading is forward then slowdown distance is smaller
    if hWhere.isInRange (5,175):
        slowDownDistance = 20
        
    slowDown = ballDSquared < hMath.SQUARE(slowDownDistance)
        
    headingLarge = abs(ballH) > 15

    # Unless the robot want to grab the ball or control it directly.
    # sHoverToBall shouldn't slow down the robot at all.
    if slowDown:
        if headingLarge:    
            Action.walk(Action.MAX_FORWARD,0,hMath.CLIP(ballH/1.6, 30.0),minorWalkType=Action.SkeFastForwardMWT)
        
        else: 
            Action.walk(Action.MAX_FORWARD,0,hMath.CLIP(ballH/2.0, 30.0),minorWalkType=Action.SkeFastForwardMWT)

    else:
        if headingLarge:
            Action.walk(Action.MAX_FORWARD,0,hMath.CLIP(ballH/1.6, 30.0),minorWalkType=Action.SkeFastForwardMWT)
        
        else:
            Action.walk(Action.MAX_FORWARD,0,hMath.CLIP(ballH/2.0, 30.0),minorWalkType=Action.SkeFastForwardMWT)
        

# Move to specifed position given the global coordinative.
# This version is the "fast" version of the one in the hTrack.
# Turns to face the direction of walking for the first part, then turns
# to face targetH as it arrives
def saGoToTargetFacingHeading(targetX, targetY, targetH, \
                maxSpeed = Action.MAX_FORWARD, maxTurn = Action.MAX_TURN):

    # ariables relative to self localisation.
    selfX           = Global.selfLoc.getX()
    selfY           = Global.selfLoc.getY()
    selfH           = Global.selfLoc.getHeading()
    relX            = targetX - selfX
    relY            = targetY - selfY
    relH            = hMath.normalizeAngle_180(targetH - selfH)
    relX            += Constant.DOG_LENGTH/2.0/10.0*(math.cos(math.radians(selfH)) \
                            - math.cos(math.radians(targetH)))
    relY            += Constant.DOG_LENGTH/2.0/10.0*(math.sin(math.radians(selfH)) \
                            - math.sin(math.radians(targetH)))
    relD            = hMath.getLength((relX, relY))
    distanceSquared = hMath.getDistSquaredBetween(targetX, targetY, selfX, selfY)
    inCircle        = distanceSquared <= hMath.SQUARE(40)
    faceH           = hMath.getHeadingToFaceAt(Global.selfLoc, targetX, targetY)
    
##~     print "faceH: ", faceH
    
    if not inCircle:
        if abs(faceH) >= 30:
            Action.walk(0, 0, faceH)
        else:
            #if sStealthDog.stealthDog(True):
            #    hFWHead.compulsoryAction = hFWHead.mustSeeBall
            #    hTrack.trackVisualBall()
            #    return
            Action.walk(maxSpeed, 0, hMath.CLIP(faceH/1.5,maxTurn))
        
    else:
        if relX == 0 and relY == 0:
            # On the dog, math.atan2(0,0) give "Value Error: math domain error".
            relTheta = 0
        else:
            relTheta = hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(relY, relX)) - selfH)
    
        forward  = hMath.CLIP(relD, maxSpeed) * math.cos(hMath.DEG2RAD(relTheta))
        left     = hMath.CLIP(relD, maxSpeed) * math.sin(hMath.DEG2RAD(relTheta))
        turnCCW  = hMath.CLIP(relH, maxTurn)
        Action.walk(forward, left, turnCCW)

# This version is the "fast" version of the one in the hTrack
def saGoToTarget(targetX, targetY, maxSpeed = Action.MAX_FORWARD, \
                    maxTurn = Action.MAX_TURN):

    selfX, selfY    = Global.selfLoc.getPos()
    selfH           = Global.selfLoc.getHeading()
    relX            = targetX - selfX
    relY            = targetY - selfY
    relD            = hMath.getLength((relX, relY))
    distanceSquared = hMath.getDistSquaredBetween(targetX, targetY, selfX, selfY)
    inCircle        = distanceSquared <= hMath.SQUARE(40)
    faceH           = hMath.getHeadingToFaceAt(Global.selfLoc, targetX, targetY)
    
    if not inCircle and abs(faceH) >= 30:
        Action.walk(0, 0, hMath.CLIP(faceH, maxTurn))
        
    elif not inCircle:
        
        #if sStealthDog.stealthDog(True):
        #    hFWHead.compulsoryAction = hFWHead.mustSeeBall
        #    hTrack.trackVisualBall()
        #    return
            
        Action.walk(maxSpeed,0,hMath.CLIP(faceH/1.5,maxTurn))
        
    else:
        if relX == 0 and relY == 0:
            relTheta = 0
        else:
            relTheta = hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(relY, relX)) - selfH)
        
        forward  = hMath.CLIP(relD, maxSpeed) * math.cos(hMath.DEG2RAD(relTheta))
        left     = hMath.CLIP(relD, maxSpeed) * math.sin(hMath.DEG2RAD(relTheta))
        turnCCW  = hMath.CLIP(relTheta, maxTurn)
        Action.walk(forward, left, turnCCW)
