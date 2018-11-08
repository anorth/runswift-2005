#
#   Copyright 2005 The University of New South Wales (UNSW) and National  
#   ICT Australia (NICTA).
#
#   This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
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
#  Copyright (c) 2005 UNSW
#  All Rights Reserved.
# 


#===============================================================================
#   Python Behaviours : 2005 (c) 
#
# sDodgyDog - edited by Joshua Shammay.
#===============================================================================


import Global
import Constant
import VisionLink
import Debug
import Action
import Indicator

import hMath
# import hFWHead
import sFindBall

GAP_MAX_DIST = -1
BEST_GAP_DIST_LIMIT = 80


MIN_BOX_INTENSITY = 30
MIN_DODGY_TARGET_DIST = 25      # Don't dodge if closer to destination than this
CLOSE_DODGY_OBSTACLE_DIST = 50  # Add sidestep when this close to an obstacle
MAX_DODGY_OBSTACLE_DIST = 150   # Don't dodge any obstacle farther than this
MAX_DODGY_DODGE = 60            # Don't divert by more than this (degrees)
CORRIDOR_WIDTH = 40
CLOSE_CORRIDOR_WIDTH = 15
MIN_OBS_IN_CORRIDOR = MIN_BOX_INTENSITY * 2

FIXED_DEST_VAR = 20

SIDESTEP_FOR = 10               #number of frames to sidestep when close to an obstacle

gUseMinGap = 30                      # Min gap size (degrees)
gUseMinIntensity = MIN_BOX_INTENSITY # Min obs intensity to be considered
gUseMinDist = 10                     # Min obs distance to be considered

#hysteris
gNewHeading = True
gLastBestHeading = 0
gBestHeadingInfluence = 3.0
gLastBestHeadingInfluence = 1.0
gSidestepCounter  = 0
gLeft = 0

def DecideNextAction():
    dodgyDogToBall()

# True if there are obstacles between me and ball that require dodging
def shouldIBeDodgyToBall(): 
    if Global.vBall.isVisible(): 
        ball = Global.vBall.getPos()
    else:
        ball = Global.gpsGlobalBall.getPos()

    if Global.lostBall > Constant.LOST_BALL_GPS:
        return False

    return shouldIBeDodgy(ball[0], ball[1])

# True if there are obstacles along heading (local, zero right) that require
# dodging
def shouldIBeDodgyAlongHeading(heading):
    # Pick a point out on the heading and dodgy to that point
    heading = hMath.local2GlobalHeading(Global.selfLoc, heading)
    pos = hMath.polarToCart(100, heading)
    #print "shouldIBeDodgyAlongHeading", heading, pos
    return shouldIBeDodgy(pos[0], pos[1], False)
    
# True if there are obstacles between me and destX/Y (global) that require
# dodging
def shouldIBeDodgy(destX, destY, fixedDest = False,\
                    fixedDestVar = FIXED_DEST_VAR):
    global gUseMinDist, gUseMinIntensity

    #return False #HACK: Make dodgydog off all the time
    #return True #HACK: Make dodgydog on all the time

    selfX, selfY = Global.selfLoc.getPos()  
    selfH = Global.selfLoc.getHeading()
    dist = hMath.getDistanceBetween(selfX, selfY, destX, destY)
    localDest = hMath.getLocalCoordinate(selfX, selfY, selfH, destX, destY)
    localHead = hMath.cartToPolar(*localDest)   # zero right

    if dist <= MIN_DODGY_TARGET_DIST:     
        return False
    
    if fixedDest and dist < fixedDestVar:
        return False

    # clip distance we count obstacles to MAX_DODGY_OBSTACLE_DIST or
    # (dist - MIN_DODGY_TARGET_DIST) so we don't dodge obstacles that are
    # miles away, and don't dodge obstacles that are right next to the
    # target.
    dist -= MIN_DODGY_TARGET_DIST
    if dist >= MAX_DODGY_OBSTACLE_DIST:
        dist = MAX_DODGY_OBSTACLE_DIST
    localDest = hMath.polarToCart(dist, localHead)

    tmp = VisionLink.getNoObstacleBetween(0, 0, 
                                int(localDest[0]), int(localDest[1]), 
                                CORRIDOR_WIDTH, gUseMinDist,
                                gUseMinIntensity, Constant.OBS_USE_NONE)  
    
    if Debug.dodgyDebug:
        print "shouldIBeDodgy: obs in corr lhead, dist (", localHead, \
                ", ", dist, ") = ", tmp,
    
    if tmp > MIN_OBS_IN_CORRIDOR:
        #when get a better sidestep, take out mindist
        #requirement and when close add left component?
        if Debug.dodgyDebug:
            print "-> True"
        return True
            
    if Debug.dodgyDebug:
        print "-> False"
    return False 

#go to ball, dodge obstacles on the way
def dodgyDogToBall():
    global gNewHeading

    if Global.lostBall > Constant.LOST_BALL_GPS:
        if Debug.dodgyDebug:
            print "dodgyDogToBall: cannot see ball, searching"
        gNewHeading = True
        return sFindBall.perform() 
        
    if Global.vBall.isVisible(): 
        ball = Global.vBall.getPos()
        #headAction = hFWHead.mustSeeBall
    else:
        ball = Global.gpsGlobalBall.getPos()
        #headAction = hFWHead.mustSeeGpsBall
    
    rtn = dodgyDogTo(ball[0], ball[1])
    
    #look at ball
    #hFWHead.compulsoryAction = headAction
    #hFWHead.DecideNextAction()
    
    return rtn
    
# Go along specified heading (local, zero right), dodging obstacles on the way
def dodgyDogAlongHeading(heading):
    # Pick a point out on the heading and dodgy to that point
    heading = hMath.local2GlobalHeading(Global.selfLoc, heading)
    pos = hMath.polarToCart(100, heading)
    return dodgyDogTo(pos[0], pos[1], False)

#go to destination, dodge obstacles on the way
#destination is specified in global co-ordinates
#set fixedDest to true if you are moving towards
#a set point on the field (as opposed to a moving point/object)
def dodgyDogTo(destX, destY, fixedDest = False, fixedDestVar = FIXED_DEST_VAR):

    global gLastBestHeading, gNewHeading
    global gBestHeadingInfluence, gLastBestHeadingInfluence
    global gUseMinGap, gUseMinIntensity, gUseMinDist
    global gSidestepCounter, gLeft
    
    myPos = Global.selfLoc.getPos() 
    myHeading = Global.selfLoc.getHeading()        
    localDest = hMath.getLocalCoordinate(myPos[0], myPos[1], myHeading,
                                         destX, destY)  
    localHead = hMath.cartToPolar(*localDest) - 90  # zero forward
    distToDest = hMath.getDistanceBetween(myPos[0],myPos[1],destX,destY)    
       
    if fixedDest and distToDest < fixedDestVar:
        return Constant.STATE_SUCCESS
    
    # bestGap = (hLeft, hRight, hBest, gapsize) degrees
    bestGap = VisionLink.getBestGap(localDest[0], localDest[1], 
                                    GAP_MAX_DIST, gUseMinDist, gUseMinGap,
                                    gUseMinIntensity, Constant.OBS_USE_NONE)    

    if bestGap != None:
        if Debug.dodgyDebug:
            print "Dodgy: localxy_dest", localDest, "dist", distToDest
            print "bestgap (left, right, best, size) =", bestGap
        
        thisBestHeading = bestGap[2]

        # Don't try to head more than x away from direct heading
        if abs(localHead - thisBestHeading) > MAX_DODGY_DODGE:
            if thisBestHeading > localHead:
                thisBestHeading = localHead + MAX_DODGY_DODGE
            elif thisBestHeading < localHead:
                thisBestHeading = localHead - MAX_DODGY_DODGE
            
            
        if gNewHeading:
            gLastBestHeading = thisBestHeading
            gNewHeading = False
        
        bestHeading  = thisBestHeading * gBestHeadingInfluence
        bestHeading += gLastBestHeading * gLastBestHeadingInfluence
        bestHeading /= gBestHeadingInfluence + gLastBestHeadingInfluence
        
        gLastBestHeading = bestHeading

        # Don't try to turn more than 30 in one go
        bestHeading = hMath.CLIP(bestHeading, 30)
            
        if Debug.dodgyDebug:
            if VisionLink.ObstacleGPSValid():
                print "dodgyDog: heading ", thisBestHeading, "->", \
                       bestHeading, " (GPS)" 
            else:
                print "dodgyDog: heading ", thisBestHeading, "->", \
                       bestHeading, " (Local)" 

            if abs(bestHeading - Global.ballH) < 10:
                Indicator.showFacePattern([0,2,2,2,0])
            elif bestHeading < Global.ballH - 60:     
                Indicator.showFacePattern([3,0,0,0,0])       
            elif bestHeading < Global.ballH:
                Indicator.showFacePattern([3,2,0,0,0])
            elif bestHeading > Global.ballH + 60:
                Indicator.showFacePattern([0,0,0,0,3])
            elif bestHeading > Global.ballH:
                Indicator.showFacePattern([0,0,0,2,3])
        
        # Wag tail in direction of dodge. 
        tailH = Indicator.TAIL_H_CENTRED
        if bestHeading > localHead + 10:
            tailH = Indicator.TAIL_LEFT * 3/4
        elif bestHeading < localHead - 10:
            tailH = Indicator.TAIL_RIGHT * 3/4         

        Indicator.finalValues[Indicator.TailH] = tailH
        #Indicator.finalValues[Indicator.TailV] = tailV

        fwd = Action.MAX_FORWARD
        turnccw = bestHeading

        closeDist = CLOSE_DODGY_OBSTACLE_DIST
        if closeDist > distToDest - MIN_DODGY_TARGET_DIST:
            closeDist = distToDest - MIN_DODGY_TARGET_DIST
        closePoint = hMath.polarToCart(closeDist, localHead)           
        closeObsIntensity = VisionLink.getNoObstacleBetween(0, 0, 
                                int(closePoint[0]), int(closePoint[1]),
                                CLOSE_CORRIDOR_WIDTH, gUseMinDist,
                                gUseMinIntensity, Constant.OBS_USE_NONE) 
                     
        if closeObsIntensity > MIN_OBS_IN_CORRIDOR: 
            if bestHeading > localHead:
                gLeft = Action.MAX_LEFT
            else:
                gLeft = -Action.MAX_LEFT
                
            #turnccw /= 2.0
            gSidestepCounter = SIDESTEP_FOR  
        else:
            if gSidestepCounter > 0:
                gSidestepCounter -= 1
            else:
                gLeft = 0
                
        if Debug.dodgyDebug and gLeft != 0:
            if bestHeading > localHead:
                print "sDodgyDog: Sidestepping LEFT (", gSidestepCounter, ")"
            else:    
                print "sDodgyDog: Sidestepping RIGHT (", gSidestepCounter, ")"    
        
        Action.walk(fwd, gLeft, turnccw, minorWalkType=Action.SkeFastForwardMWT)
        
        return Constant.STATE_EXECUTING    

    else:        
        if Debug.dodgyDebug:
            print "getBestGap failed - can't be dodgy"
        return Constant.STATE_FAILED   
        
