#
#   Copyright 2005 The University of New South Wales (UNSW) and National  
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
#  $Id: hTrack.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 

#===============================================================================
#   Python Behaviours : 2005 (c) 
#
# hTrack.py
#
# Different simple behaviours that might be used in any player codes. Compare to
# HelpShort.py, functions in this file is more complex.
#
#===============================================================================

import math
import Global
import Constant
import VisionLink
import Action
#import Debug
import hMath
#import hTeam
import hWhere
import sGrab
import sSelKick


# A global variable used by stationaryLocalise. This variable indicate what
# direction the robot want to pan to when localising.
panDirection = Constant.dCLOCKWISE

# Constants used for timeToReachPoint/Ball. These are best-case estimates
EST_WALK_SPEED = 35.0/1000     # Actual is approx. 41 cm/sec, but we are harsh
EST_TURN_SPEED = 120.0/1000    # Actual is approx. 180 deg/sec but we are harsh

# After this number of lost ball frames, penalty is given to timeToBall
LOST_BALL_FRAME = 15

# Time penalty to timeToReachBall for lost ball frames. Penalty is
# ballLost^2 * penalty. Initial setting of 2 means after half a second
# penalty = 15 * 15 * 2 = 450 ms, approx equal to attacker/supporter hysterisis
LOST_BALL_TIME_PENALTY = 2

# Since we use signed short, we might want to cap the largest and the smallest
# possible value
gCapTime = True

gTimeToReachBall = 0
gTimeToReachBallFrame = -1

# Hysterisis values for timeToBall to give advantage to currently selected
# roles (milliseconds)
ATTACKER_TIME_BONUS = 800       # current attacker over others
SUPPORTER_TIME_BONUS = 800      # current supporter over winger/defender
STRIKER_TIME_BONUS = 800        # current striker
WINGER_TIME_BONUS = 800         # current winger bonus for winger pos
DEFENDER_TIME_BONUS = 1000      # current defender for defender pos
GRABBER_TIME_BONUS = 10000      # grabbing the ball is huge bonus

CLOSE_WALK_RANGE = 30

#---------------------------------------
# Return True if visual ball is observed in cplane.
def canSeeBall():
    return VisionLink.getBallConfidence() > 0
    
#def canSeeTargetGoal():
#    return 

def DecideNextAction():
    scan()


gVisBallPan = 0
gVisBallTilt = 0
gVisBallCrane = 0
#--------------------------------------
# Adjust the pan, tilt and crane to look at the ball, if there is one on scene.
def trackVisualBall(adjX=0,adjY=0,adjZ=0,useVelocity=True,isTurning=False,adjPos=1.0):    
    global gVisBallPan, gVisBallTilt, gVisBallCrane
    
    if not Global.vBall.isVisible():
        Action.setHeadToLastPoint()
        return
    
    vel = VisionLink.getGPSBallVInfo(Constant.CTLocal)        
    velX, velY = vel[0], vel[1]

    
    # adjustment with velocity.
    # look ahead half as much as we should.   
    if useVelocity:
        if abs(velX) > 2:          
            adjX += velX
    
    if abs(velY) > 2:
        adjZ += velY
            
    #if Global.ballD < 50:
    #    trackVisualBallByProj(adjX,adjY,adjZ)
    #else:         
    #    trackVisualBallByTrig(adjZ,adjY,adjZ)
    trackVisualBallByProj(adjX,adjY,adjZ,isTurning,adjPos)
        
    # Make sure the head don't swing all the time.
    clipFireballPan()
    
    gVisBallPan = Action.finalValues[Action.Panx]
    gVisBallTilt = Action.finalValues[Action.Tilty]
    gVisBallCrane = Action.finalValues[Action.Cranez]
            


    
def trackVisualBallByProj(adjX=0,adjY=0,adjZ=0,isTurning=False,adjPos=1.0):       
    ballPos = VisionLink.getProjectedBall()
    
    # If projection returns infinity, then use gps.
    if abs(ballPos[0]) >= Constant.LARGE_VAL\
        or abs(ballPos[1]) >= Constant.LARGE_VAL:
        # We should divide the velocity adjustment by half.
        trackVisualBallByTrig(adjX,adjY,adjZ,adjPos=adjPos)   
        return
        
    ballPosX = ballPos[0] * adjPos
    ballPosY = ballPos[1] * adjPos        

    #print " track by proj ",
    # Note that xyz in getPointProjection is different from xyz to headMotion
    if isTurning and abs(Global.ballH) > 80:              
        d = hMath.getDistanceBetween(0,0,ballPosX,ballPosY) 
        x = d * math.cos(hMath.DEG2RAD(10))
        y = d * math.sin(hMath.DEG2RAD(10))                
        if Global.ballH < 0: 
            x = -x
        Action.setHeadParams(x + adjX, adjY, y + adjZ, Action.HTAbs_xyz)                    
       
    else:
        Action.setHeadParams(ballPosX + adjX, adjY, ballPosY + adjZ, Action.HTAbs_xyz)


def trackVisualBallByTrig(adjX=0,adjY=0,adjZ=0,isTurning=False,adjPos=1.0):
    #print " track by trig ", 
    ballD, ballH = Global.ballD, Global.ballH 
    ballX = math.sin(hMath.DEG2RAD(ballH)) * ballD;
    ballY = math.cos(hMath.DEG2RAD(ballH)) * ballD;
    
    ballX = ballX * adjPos
    ballY = ballY * adjPos
    
    if isTurning and abs(Global.ballH) > 80: 
        d = Global.ballD 
        x = d * math.cos(hMath.DEG2RAD(10))
        y = d * math.sin(hMath.DEG2RAD(10))                
        if Global.ballH < 0: 
            x = -x
        Action.setHeadParams(x + adjX, Constant.BallDiameter + adjY, y + adjZ, Action.HTAbs_xyz)                      
    else:         
        Action.setHeadParams(ballX + adjX, Constant.BallDiameter + adjY, ballY + adjZ, Action.HTAbs_xyz)
    
    
#--------------------------------------
# Set the head not to have any pan movement when the ball is really close
# This assume the headType is relative (no point to do clip on absolute anyway).
gLastClipFireballFrame = 0
def clipFireballPan():
    global gLastClipFireballFrame
    if Global.vBall.isVisible():
        
        ballx = math.sin(hMath.DEG2RAD(Global.ballH)) * Global.ballD
        bally = math.cos(hMath.DEG2RAD(Global.ballH)) * Global.ballD
        
##~         print "local x %.2f,  y%.2f   ballH %.2f" % (ballx, bally,Global.ballH)
        if abs(ballx) <= 2.5 and 0 <= bally <= 3.5 * Constant.BallRadius:
            Action.finalValues[Action.Panx] = 0
            gLastClipFireballFrame = Global.frame
        # hysterisis
        elif False and gLastClipFireballFrame >= Global.frame - 1 \
            and abs(ballx) <= 3.2 and 0 <= bally <= 3.5 * Constant.BallRadius:
            Action.finalValues[Action.Panx] = 0
            gLastClipFireballFrame = Global.frame
    

def clipFireballPanHack():
    global gLastClipFireballFrame
    if Global.vBall.isVisible():
        
        ballx = math.sin(hMath.DEG2RAD(Global.ballH)) * Global.ballD
        bally = math.cos(hMath.DEG2RAD(Global.ballH)) * Global.ballD
        
##~         print "local x %.2f,  y%.2f   ballH %.2f" % (ballx, bally,Global.ballH)
        if abs(ballx) <= 2.5 and 0 <= bally <= 5.0 * Constant.BallRadius:
            Action.finalValues[Action.Panx] = 0
            gLastClipFireballFrame = Global.frame
        elif False and gLastClipFireballFrame >= Global.frame - 1 \
            and abs(ballx) <= 3.2 and 0 <= bally <= 5.0 * Constant.BallRadius:
            Action.finalValues[Action.Panx] = 0
            gLastClipFireballFrame = Global.frame
            
    

#--------------------------------------
# Given a wireless ball location, just scan around!!!
# hmm.. nothing else better? A narrower scan on that range might be good --weiming
def trackWirelessBall():    
    scan()   
    
#--------------------------------------
# Given a gps ball location, direct your head to it
def trackGpsBall():
    if abs(Global.gpsLocalBall.getHeading()) < 90:
        ballX, ballY = Global.gpsLocalBall.getPos()
        Action.setHeadParams(-ballX,Constant.BallDiameter,ballY,Action.HTAbs_xyz)
        
#--------------------------------------
# Given a location in local coords, direct your head to it        
def trackLocalPoint(localX, localY):
    Action.setHeadParams(-localX,10,localY,Action.HTAbs_xyz)            

#--------------------------------------
# Move the point under the dog's neck to a location (global) on the field
# Note that turnCCW is relative to the centre of the dog's body, not under its
# neck
def saGoToTargetFacingHeading(targetX, targetY, targetH, \
                            maxSpeed = None, maxTurn = None):

    if maxSpeed == None:             
        maxSpeed = Action.MAX_FORWARD
    if maxTurn == None:
        maxTurn = Action.MAX_TURN

    # Variables relative to self localisation.
    selfPos  = Global.selfLoc.getPos()
    selfH    = Global.selfLoc.getHeading()
    relX     = targetX - selfPos[0]
    relY     = targetY - selfPos[1]
    relH     = hMath.normalizeAngle_180(targetH - selfH)

    # take into account that as we turn, the position of our head will change
    relX  += Constant.DOG_NECK_TO_BODY_CENTER_OFFSET * \
             (math.cos(math.radians(selfH)) - math.cos(math.radians(targetH)))
    relY  += Constant.DOG_NECK_TO_BODY_CENTER_OFFSET * \
             (math.sin(math.radians(selfH)) - math.sin(math.radians(targetH)))
    relD  = hMath.getLength((relX, relY))
    
    if relX == 0 and relY == 0:
        relTheta = 0
    else:
        relTheta = hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(relY, relX)) - selfH)

    forward = hMath.CLIP(relD, maxSpeed) * math.cos(hMath.DEG2RAD(relTheta))
    left    = hMath.CLIP(relD, maxSpeed) * math.sin(hMath.DEG2RAD(relTheta))
    turnccw = hMath.CLIP(relH, maxTurn)    

    Action.walk(forward, left, turnccw,"ssd", minorWalkType = Action.SkeFastForwardMWT)

#--------------------------------------
# Walks to the specified point, turning to face the direction of walk as it
# goes. Appropriate walkParams will be chosen if not specified
def saGoToTarget(targetX, targetY, maxSpeed = None, maxTurn = None):

    if maxSpeed == None:             
        maxSpeed = Action.MAX_FORWARD
    if maxTurn == None:
        maxTurn = Action.MAX_TURN
                                                   
    selfPos = Global.selfLoc.getPos()
    selfH   = Global.selfLoc.getHeading()
    relX    = targetX - selfPos[0]
    relY    = targetY - selfPos[1]
    relD    = hMath.getLength((relX, relY))
    
    if relX == 0 and relY == 0:
        relH = 0
    else:
        relH = hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(relY,relX))-selfH)

    forward = 0
    left = 0
    turnccw = 0
    if relD < CLOSE_WALK_RANGE and abs(relH) > 50:
        forward = relY
        left = relX
        turnccw = relH * 0.8
#        print "close walk and turn",relY,left,turnccw
    else:
        if abs(relH) > 80:      # stop and turn
            turnccw = relH * 0.8
        elif abs(relH) > 25: # walk and turn
            turnccw = relH * 0.8
            forward = hMath.CLIP(relD,maxSpeed) * math.cos(hMath.DEG2RAD(relH))
#            print "walk and turn",forward, 0, turnccw
        else:                   # ellipticalwalk and turn            
            turnccw = hMath.CLIP(relH * 0.8, 25)
            forward = hMath.CLIP(relD,maxSpeed) * math.cos(hMath.DEG2RAD(relH))
    # finally, we walk
    Action.walk(forward, left, turnccw, minorWalkType = Action.SkeFastForwardMWT)

#     forward = hMath.CLIP(relD,maxSpeed) * math.cos(hMath.DEG2RAD(relTheta))
#     left    = hMath.CLIP(relD,maxSpeed) * math.sin(hMath.DEG2RAD(relTheta))
#     turnCCW = hMath.CLIP(relTheta, maxTurn)
#    Action.walk(forward, left, turnCCW, walkParams)

#-----------------------------------------
# Walks in the specified direction (global), turning to face along the way
# Alexn: I'm not sure this works. Test before using
def saGoOnHeading(head, maxSpeed = 10, maxTurn = 30):
    relH = hMath.normalizeAngle_180(head - Global.selfLoc.getHeading()) + 90
    forward = maxSpeed * math.cos(relH)
    left = maxSpeed * math.sin(relH)
#    turnCCW = relH
    turnCCW = 0
    print "globalH", head, "myH", Global.selfLoc.getHeading(), "relH", relH

    Action.walk(hMath.CLIP(forward, maxSpeed), hMath.CLIP(left, maxSpeed), \
                hMath.CLIP(turnCCW, maxTurn))

#--------------------------------------
# There are two dkd functions for some reason.
# Note: this returns a global heading between 0 and 360 (like the gps self heading)
# returns a tuple of 5 numbers (avgDir, narrowMin, narrowMax, wideMin, wideMax)
def getDKDRange():

    nearEdgeDist = 40
    debugDKD     = False
    length       = Constant.FIELD_LENGTH
    width        = Constant.FIELD_WIDTH

    # Make sure determine ballSource is called
    if Global.ballSource is None:
        print "Warning: in getDKDRange() havent determined the ball source before getDKD()"
        return None
        
    X_ = Global.ballX
    Y_ = Global.ballY
    if debugDKD:
        print "Ball Source: ", Global.ballSource, "   X_ ", X_, "  Y_ " , Y_ 
    
    # If you don't know where the ball is determine dkd based on where you are.
    if Global.ballSource == Constant.GPS_BALL\
        and VisionLink.getGPSBallMaxVar() >= hMath.get95var(35):
        
        X_, Y_ = Global.selfLoc.getPos()
        if debugDKD:
            print "Warning: in getDKDRange(): I dont trust GPS ball right now: use my own pos is X_ ", X_, "  Y_ " , Y_ 

    Y_ = hMath.CLIPTO(Y_,0,length)
    X_ = hMath.CLIPTO(X_,0,width)
    
    # If the ball is near one of the side edges and not in corners then the direction becomes up the field.
    if Y_ > nearEdgeDist:
        if X_ < nearEdgeDist: # on the left edge
            if debugDKD:
                print "On the left edge"
            return (90, 70, 100, 10, 110)
        elif X_ > (width - nearEdgeDist): #on the right edge
            if debugDKD:
                print "On the right edge"
            return (90, 80, 110, 70, 170)

    # upper 60%, can I use voak?

    if Y_ > 0.6*length:
    
        # If target goal seen and is close and not many obstacles, it has a good heading, get the best heading like VOAK
        if (Global.vTGoal.getConfidence() > 3 and 
            Global.ballSource == Constant.VISION_BALL):
            toY       = length - Y_
            toX       = width/2.0 - X_
            xRange    = Constant.GOAL_WIDTH/2.0 + (toY * math.tan(hMath.DEG2RAD(30)))
            inVOAKcal = abs(toX) < xRange
            
            if inVOAKcal:
            
                (lmin, lmax, rmin, rmax) = VisionLink.getHeadingToBestGap()
                lmin = hMath.normalizeAngle_0_360 (lmin+Global.selfLoc.getHeading())
                lmax = hMath.normalizeAngle_0_360 (lmax+Global.selfLoc.getHeading())
                rmin = hMath.normalizeAngle_0_360 (rmin+Global.selfLoc.getHeading())
                rmax = hMath.normalizeAngle_0_360 (rmax+Global.selfLoc.getHeading())
                if abs(lmax - lmin) > abs(rmax - rmin):
                    tdkdmin, tdkdmax = lmin, lmax
                    dkd = (lmin+ lmax) / 2.0
                else:
                    tdkdmin, tdkdmax = rmin, rmax
                    dkd = (rmin+ rmax) / 2.0
                if (dkd < 150 and dkd > 30):
                    return (dkd,tdkdmin,tdkdmax,10,170)
    
    # Find the dkd fron the center of the target and own goals
    dkdup     = hMath.getHeadingBetween(X_, Y_, (width / 2.0), length)
    dkdbottom = hMath.getHeadingBetween((width/2.0),0,X_,Y_)
    
    # if i cant use voak, then the heading to the center of target goal is used as dkd
    if Y_ > 0.6*length:
        
        tdkdmin = hMath.getHeadingBetween (X_,Y_,Constant.RIGHT_GOAL_POST,length)
        tdkdmax = hMath.getHeadingBetween (X_,Y_,Constant.LEFT_GOAL_POST,length)
        if debugDKD:
            print "close to target goal, dkd directed to the target goal", Global.DKD[0]
        return (dkdup,tdkdmin,tdkdmax,10,170)
         
    # if close to own goal, direted out of own goal
    elif Y_ < 0.4*length:
    
        if debugDKD:
            print "close to own goal, dkd directed out of the own goal"
        return (dkdbottom, 20, 160, 10 , 170)
        
    # Now, the middle 20% of the field should have a continous change of direction.   
    else:
        myy   = Y_ - 0.4*length
        ratio = myy/(0.2*length)
        dkd   = dkdup * ratio + dkdbottom * (1-ratio) 
        
        if debugDKD:
            print "middle 20%, linear interpolation"
        return (dkd, 40, 140, 20, 160)
        

# Returns the estimated time (milliseconds) to reach the ball from
# the robot's current (or specified) position. Coordinates are global.
# Credit for the idea to the GermanTeam 2004 report, pp74-5.
def timeToReachBall():
    global gTimeToReachBall, gTimeToReachBallFrame
    
    if gTimeToReachBallFrame == Global.frame:
        return gTimeToReachBall
    
    # 1. Calculate time to reach ball
    # 2. Add time if the ball is unsure
    # 3. Reduce time for current attacker for hysterisis
    # 4. Reduce time a lot if we are grabbing
    # 5. timeToReachPoint adds time for turning to face and turning to dkd

    bonus = 0 # larger numbers are better

    # Penalty for lost ball - quadratic
    if Global.lostBall > LOST_BALL_FRAME:
        bonus -= int(hMath.SQUARE(Global.lostBall - LOST_BALL_FRAME) * LOST_BALL_TIME_PENALTY)

    # Bonus for role
    if Global.myLastRole == Constant.ATTACKER: 
        bonus += ATTACKER_TIME_BONUS # attacker bonus

    if sGrab.isGrabbed: 
        bonus += GRABBER_TIME_BONUS      # big bonus for grabbing

    #dkd = 90    # DKD is initialised in Forward so for ready/set use upfield
    #if Global.DKD != None:
    #    dkd = Global.DKD[0]
    dkd = sSelKick.perform()[1]
    
    t = timeToReachPoint(Global.ballX, Global.ballY, dkd) - bonus
    if gCapTime:
        rtn = hMath.CLIP(t,Constant.MAX_TIME_TO_REACH)
    else:
        rtn = t
        
    gTimeToReachBall = rtn
    gTimeToReachBallFrame = Global.frame
    return rtn

# Returns the estimated time (milliseconds) to reach the supporter position from
# the robot's current (or specified) position. Coordinates are global.
def timeToReachSupporterPos(myX, myY, myH, isSupporter):
    # 1. Calculate time to reach pos
    # 2. Reduce time for current supporter for hysterisis

    bonus = 0 # larger numbers are better

    # Bonus for role
    if isSupporter: 
        bonus += SUPPORTER_TIME_BONUS
    
    (x, y, _) = hWhere.getSupporterPos(False)
    dkd = 90
        
    t = timeToReachPoint(x, y, dkd, myX, myY, myH) - bonus
    return t
    
# Returns the estimated time (milliseconds) to reach the striker position from
# the robot's current (or specified) position. Coordinates are global.
def timeToReachStrikerPos(myX, myY, myH, isStriker):
    # 1. Calculate time to reach pos
    # 2. Reduce time for current striker for hysterisis

    bonus = 0 # larger numbers are better

    # Bonus for role
    if isStriker: 
        bonus += STRIKER_TIME_BONUS
    
    (x, y, _) = hWhere.getStrikerPos()
    dkd = 90
        
    t = timeToReachPoint(x, y, dkd, myX, myY, myH) - bonus
    return t    
    
# Returns the estimated time (milliseconds) to reach the supporter position from
# the robot's current (or specified) position. Coordinates are global.
def timeToReachWingerPos(myX, myY, myH, isWinger):
    # 1. Calculate time to reach pos
    # 2. Reduce time for current Winger for hysterisis

    bonus = 0 # larger numbers are better

    # Bonus for role
    if isWinger: 
        bonus += WINGER_TIME_BONUS
    
    (x, y) = hWhere.getWingerPos2()
    dkd = 90
        
    t = timeToReachPoint(x, y, dkd, myX, myY, myH) - bonus
    return t

# Returns the estimated time (milliseconds) to reach the defending position from
# the robot's current (or specified) position. Coordinates are global.
def timeToReachDefenderPos(myX, myY, myH, isDefender):
    # 1. Calculate time to reach pos
    # 2. Reduce time for current defender for hysterisis

    bonus = 0 # larger numbers are better

    # Bonus for role
    if isDefender: 
        bonus += DEFENDER_TIME_BONUS
    
    (x, y) = hWhere.getDefenderPos()
    dkd = 90
        
    t = timeToReachPoint(x, y, dkd, myX, myY, myH) - bonus
    return t

# Returns the estimated time (milliseconds) to reach the specified position on
# the field from the robot's current position. Coordinates are
# global.
def timeToReachPoint(x, y, h, myX=None, myY=None, myH=None,\
                    doTurn=True, doDKD=True, doObs=True):
    if myX == None:
        myX = Global.selfLoc.getX()
        myY = Global.selfLoc.getY()
        myH = Global.selfLoc.getHeading()

    relH = hMath.getHeadingToMe(myX, myY, myH, x, y)
    absH = hMath.getHeadingBetween(myX, myY, x, y)
    (lX, lY) = hMath.getLocalCoordinate(myX, myY, myH, x, y)
    # 1. Estimate the straight line time to get there
    # 2. Add time to turn and face (or slow walk due to turning on the run)
    # 3. Add time to face desired heading when we arrive (or getBehind)
    # 4. Add time for obstacles if there are any in the way

    #    if (Global.frame % 10 == 0):
    #        print "dist:", hMath.getDistanceBetween(myX, myY, x, y) /EST_WALK_SPEED
    #        print "turn:", abs(relH) /EST_TURN_SPEED 
    #        print "dkd:", abs(hMath.normalizeAngle_180(absH - h)) /EST_TURN_SPEED
    
    time = hMath.getDistanceBetween(myX, myY, x, y) / EST_WALK_SPEED
    if doTurn:
        time += abs(relH) / EST_TURN_SPEED
    if doDKD:
        # Time to turn to dkd when we get there is penalised more harshly
        time += (abs(hMath.normalizeAngle_180(absH - h)) / EST_TURN_SPEED) * 1.3
    if doObs:
        nobs = VisionLink.getNoObstacleBetween(0, 0, int(lX), int(lY), 30,
                                            30, 0, Constant.OBS_USE_NONE)
        if nobs > 100:
            nobs = 50
        #if x == Global.ballX:
        #    print "Nobs=", nobs
        if nobs >= 5:
            # 10ms per obstacle point. A typical half-obstructed path might
            # count 60 obs -> 600ms. Max at nobs 100 -> 1 sec
            time += nobs * 10

    return int(time)
        


#--------------------------------------
lastPan = 0
def stationaryLocalise(speed = 5, leftLimit = 80, rightLimit = -80):
    global panDirection, lastPan
    
    if Global.pan >= leftLimit and panDirection == Constant.dANTICLOCKWISE:
        panDirection = Constant.dCLOCKWISE
    elif Global.pan <= rightLimit and panDirection == Constant.dCLOCKWISE:
        panDirection = Constant.dANTICLOCKWISE

    if panDirection == Constant.dCLOCKWISE:
        lastPan = lastPan - speed
    else:
        lastPan = lastPan + speed

    cranez = -25
    tilty = 0 
            
    Action.finalValues[Action.HeadType] = Action.HTAbs_pt
    Action.finalValues[Action.Panx]     = lastPan
    Action.finalValues[Action.Tilty]    = tilty
    Action.finalValues[Action.Cranez]   = cranez

#--------------------------------------
panningCounter = 0
PANNING_TOO_LONG = 60
def spinningLocalise(clockwise = True,  lowCrane = -20, highCrane = 20, tilty = -20,speed = 5, minPan = -80, maxPan = 80):
    global panDirection,currentPanx, panningCounter
    
    pan = Global.desiredPan
    
    if (panningCounter > PANNING_TOO_LONG or pan >= maxPan)\
        and panDirection == Constant.dANTICLOCKWISE:
        
        panDirection = Constant.dCLOCKWISE
        panningCounter = 0
        
    elif (panningCounter > PANNING_TOO_LONG or pan <= minPan)\
        and panDirection == Constant.dCLOCKWISE:
        
        panDirection = Constant.dANTICLOCKWISE
        panningCounter = 0
    
    panningCounter += 1
    
    if hMath.xor(clockwise,panDirection == Constant.dCLOCKWISE):
        cranez = lowCrane
    else:
        cranez = highCrane
        
    if panDirection == Constant.dCLOCKWISE:
        pan += -speed
    else:
        pan += speed
        
    Action.setHeadParams(pan,tilty,cranez,Action.HTAbs_h)

scanListPosition = []
def createScanningPosition(panDir = Constant.dCLOCKWISE,noOfFrame = 120, speed = 5, leftLimit = 80, rightLimit = -80):
    global scanListPosition
    panx   = Global.pan
    cranez = Global.crane
    tilty  = Global.tilt
    scanListPosition = []  
    for _ in range(noOfFrame):
        #print "panDir:",panDir == Constant.dCLOCKWISE
        #print "panx =", panx, " tilty=",tilty," cranez=",cranez
        if panx >= leftLimit and panDir == Constant.dANTICLOCKWISE:
            panDir = Constant.dCLOCKWISE
        elif panx <= rightLimit and panDir == Constant.dCLOCKWISE:
            panDir = Constant.dANTICLOCKWISE
    
        if panDir == Constant.dCLOCKWISE:
            panx += -speed
        else:
            panx += speed
    
        if abs(cranez) > 5:
            cranez += -cranez / 3
        else:
            cranez += 0
        
        if abs(tilty - 10) > 5:
            tilty += 10 - tilty / 3
        else:
            tilty += 0
        scanListPosition+=[(panx,tilty,cranez)]

#efficient slide scan, only scanning where the place haven't been seen.
#If the frame is not next to the other, set reset to False
#It is depend on the speed and the time pause,
#see tEfficientScan for example
lastFrame = 0
def efficientScan(reset = True, frameList = 120,speed=50*80/90,leftLimit = 80, rightLimit = -80, offsetFrame = 15):
    global scanListPosition, lastFrame
    #if current frame not after the last frame in some offset(1 or 2), reset.
    #if it set to reset, then reset
    #if it not reset then not reset except the scanListPosition is empty
    if reset or (Global.frame - lastFrame > offsetFrame) or scanListPosition == []:
        #print "reseted, reset", reset, " Global.frame - lastFrame:",Global.frame - lastFrame, " scanListPosition:", scanListPosition == []
        createScanningPosition(Constant.dCLOCKWISE, frameList, speed, leftLimit, rightLimit)
        #print scanListPosition
    if scanListPosition == []:
        print "scan list empty"
        return
    panx,tilty,cranez = scanListPosition[0]
    del scanListPosition[0]
    Action.setHeadParams(panx,tilty,cranez,Action.HTAbs_h)    
    print Global.frame, ":set:", panx,":", tilty,":",cranez
    print Global.frame, ":", Global.pan,":", Global.tilt,":",Global.crane
    lastFrame = Global.frame

gScanWaitCounter = 0
## Tried to mimic the German head scan (come back later)
def scanLikeGermanTeam(lowCrane=0,highCrane=0,tilty=0,speed=10,minPan=-90,maxPan=90):
    global panDirection
    global gScanWaitCounter
        
    if not scanLikeGermanTeamHorizontal(highCrane,tilty,speed,minPan,maxPan):
        if gScanWaitCounter < 3:
            gScanWaitCounter += 1
        elif not scanLikeGermanTeamVertical():
            gScanWaitCounter = 0
            panx = Global.desiredPan
            gScanWaitCounter = 0
            if panx == 0\
                and panDirection == Constant.dCLOCKWISE:        
                panx = -1
                panDirection = Constant.dCLOCKWISE
                gScanWaitCounter = 0

            elif panx == 0\
                and panDirection == Constant.dANTICLOCKWISE: 
                panx = 1
                panDirection = Constant.dANTICLOCKWISE
                

            Action.setHeadParams(panx,tilty,lowCrane,Action.HTAbs_h)
    else: 
        gScanWaitCounter = 0                

                

def scanLikeGermanTeamHorizontal(cranez=0,tilty=0,speed=10,minPan=-90,maxPan=90):
    global panDirection
    
    panx = Global.desiredPan
    
    if 0 < panx <= maxPan\
        and panDirection == Constant.dCLOCKWISE:        
        panx -= speed
        if panx <= 0: 
            panx = 0
         
    elif 0 < panx <= maxPan\
        and panDirection == Constant.dANTICLOCKWISE:
        panx += speed
        if panx >= maxPan:
            panx = maxPan
            panDirection = Constant.dCLOCKWISE
            
        
    elif minPan <= panx < 0\
        and panDirection == Constant.dCLOCKWISE: 
        panx -= speed
        if panx <= minPan: 
            panx = minPan
            panDirection = Constant.dANTICLOCKWISE
    
    elif minPan <= panx < 0\
        and panDirection == Constant.dANTICLOCKWISE:
        panx += speed
        if panx >= 0:
            panx = 0
    
    else: 
        return False
    
    Action.setHeadParams(panx,tilty,cranez,Action.HTAbs_h)    
    return True        
    
    
dDOWN = 0
dUP   = 1
tiltDirection = dDOWN
def scanLikeGermanTeamVertical(): 
    global tiltDirection
    tilty = Global.desiredTilt
    

    if tiltDirection == dDOWN: 
        tilty -= 3
    else: 
        tilty += 3
        
    if tilty > 0:
        tiltDirection = dDOWN
        tilty = 0
        Action.setHeadParams(0,tilty,-20,Action.HTAbs_h)
        return False
               
    elif tilty < -35: 
        tiltDirection = dUP
        tilty = -35
                       
    Action.setHeadParams(0,tilty,-20,Action.HTAbs_h)
    return True



panLow = False
def scan(lowCrane=-10,highCrane=25,lowTilt=0,highTilt=0,lowSpeed=10,highSpeed=12,minPan=-90,maxPan=90,lookDown=True): 
    global panDirection
    global panLow
           
    minPan = max(minPan,-90)
    maxPan = min(maxPan,90)
    lowCrane = max(lowCrane,-20) 
    highCrane = min(highCrane,10)
    
    pan = max(min(Global.desiredPan,maxPan),minPan)
        
    pan_sensor = hMath.MICRORAD2DEG(VisionLink.getAnySensor(Constant.ssHEAD_PAN))
    #tilt_sensor = hMath.MICRORAD2DEG(VisionLink.getAnySensor(Constant.ssHEAD_TILT1))
    #crane_sensor = hMath.MICRORAD2DEG(VisionLink.getAnySensor(Constant.ssHEAD_TILT2))
    
    
    # Checking if the pan reached either maximum or minimum
    if pan >= maxPan and panDirection == Constant.dANTICLOCKWISE\
        and abs(pan - pan_sensor) <= 20:
        panDirection = Constant.dCLOCKWISE
        panLow = not panLow
        if panLow: 
            tilt = lowTilt
            crane = lowCrane
        else: 
            tilt = highTilt
            crane = highCrane        
                
    elif pan <= minPan and panDirection == Constant.dCLOCKWISE\
        and abs(pan - pan_sensor) <= 20: 
        panDirection = Constant.dANTICLOCKWISE
        panLow = not panLow
        if panLow: 
            tilt = lowTilt
            crane = lowCrane
        else: 
            tilt = highTilt
            crane = highCrane
    else:
        # Setting head parameters for next frame    
        if panDirection == Constant.dCLOCKWISE:
            d = -1
        else:
            d = 1
            
        # Turning super ball detection on when we are looking down.
        if panLow and abs(pan_sensor) < 40:
            VisionLink.setGrabbing()

        if panLow:            
            if lookDown and abs(pan) < 40: 
                tilt = -30
            else:                
                tilt = lowTilt
            pan += d * lowSpeed                
            crane = lowCrane

        else:
            pan += d * highSpeed
            tilt = highTilt
            crane = highCrane
    
    #print "pan : ", pan, " tilt : ", tilt, " crane : ", crane              
    Action.setHeadParams(pan,tilt,crane,Action.HTAbs_h) 

def utilIsUp(DKD):
    (_, tdkdmin, tdkdmax, wdkdmin, wdkdmax) = DKD
    return hWhere.isInRange( (tdkdmin+wdkdmin)/2.0 , (tdkdmax+wdkdmax)/2.0)
    
"""

def amIFurtherestXToBall():
    for i in Global.otherValidForwards:
        if Global.teamPlayers[i].getCounter() <= 0: 
            continue

        if abs(Global.selfLoc.getPos()[0] - Global.ballX) - \
           abs(Global.teammatesLoc[i].getPos()[0] - Global.ballX) < 0:
            return False
            
    return True
    
def amIClosestXToBall():
    for i in Global.otherValidForwards:
        if Global.teamPlayers[i].getCounter() <= 0: 
            continue
            
        if abs(Global.selfLoc.getX() - Global.ballX) - \
           abs(Global.teammatesLoc[i].getX() - Global.ballX) > 0:
            return False
            
    return True


def amIFurtherestToBall(): 
    for i in Global.otherValidForwards: 
        if Global.teamPlayers[i].getCounter() <= 0: 
            continue
        
        ballD1 = Global.ballD
        ballD2 = Global.teamPlayers[i].getBallDist()
    
        if ballD1 < ballD2:
            return False 
            
    return True


## checks if you are the closest to the ball. 
## Using distance
def amIClosestToBall(offset=0):
    for i in Global.otherValidForwards: 
        if Global.teamPlayers[i].getCounter() <= 0: 
            continue
        
        ballD1 = Global.ballD
        ballD2 = Global.teamPlayers[i].getBallDist()
    
        if ballD1 - ballD2 > offset:
            return False 
            
    return True
"""   

"""
lookDir = Constant.dANTICLOCKWISE;
# num frames in 1/2 cycle of head finding ball
HEAD_PERIOD = 16 # was 22 --weiming /7/2/05
BL_LOOK_GPS     = Constant.VBALL_LOST + 1   # Start looking at gps ball.


def headFindBall(tiltLow, tiltHigh):
    global lookDir
    global HEAD_PERIOD, BL_LOOK_GPS

    lowfirst = False

    # If first time in calling this function, set lookDir.
    if (Global.lostBall <= (BL_LOOK_GPS + 1)):

        lowfirst = True

        h = Global.gpsLocalBall.getHeading()
        if  ((h >= 0 and lowfirst) or \
             (h < 0  and not lowfirst)):
            lookDir = Constant.dCLOCKWISE
        else:
            lookDir = Constant.dANTICLOCKWISE

    headStep = Global.lostBall % (HEAD_PERIOD)
            
    # Every head period alternate between low first and high first
    # 1 == True, 0 == False.
    if (((Global.lostBall / HEAD_PERIOD) % 2) == lowfirst):

        # look down first
        panx = -90 + (180 / (HEAD_PERIOD)) * headStep
        tilty = tiltLow + 30
        cranez = -15

    else:
        # look up first
        panx = 90 - (180 / (HEAD_PERIOD)) * headStep
        tilty = tiltHigh
        cranez = 5

    # Set pan direction.
    if (lookDir == Constant.dCLOCKWISE):
        panx *= 1
    else:
        panx *= -1
    Action.setHeadParams(panx,tilty,cranez,Action.HTAbs_h)
"""
