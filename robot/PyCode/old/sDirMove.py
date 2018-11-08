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
#  $Id: sDirMove.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
#   Directional paw kick skill, created by Kevin.
#===============================================================================

import math
import Global
import Constant
import sPawKick
import sHoverToBall

import Indicator
import hMath
import Action

# ===============================================
# Globals to be used in this module
# ===============================================

sum_reqTurn               = 0
last_reqTurn              = 0
lastBx                    = 0
lastBy                    = 0
lastFrame                 = 0

# ===============================================
# Directional move using Elliptical Walk at max speed (7.0)
# max normal walk reverse forward is -10
# ===============================================


# Note: I have added something to determine the direction already, so turndir as input has no effects
def DecideNextAction (radius, aa, turndir, taOff, raOff, kp, ki, kd ):

    global lastBx, lastBy, sum_reqTurn, last_reqTurn, lastFrame, lastDir
    
    skippedFrames = Global.frame-(lastFrame+1)

    # Reset the running sum if the routine is not called. 
    lastFrame = Global.frame       # record the last frame

    Indicator.showFacePattern (Constant.FP_DIR_KICK)

    # No need to showFacePattern, because rAttacker has called already before entering this.
    
    myx, myy = Global.selfLoc.getPos()               # My (the dog) position and its heading
    myh      = Global.selfLoc.getHeading()

    if Global.vBall.getConfidence() > 2:
        bx, by         = Global.vBall.getPos()       # ball position
        lastBx, lastBy = Global.vBall.getPos()
    else:
        bx = lastBx
        by = lastBy


    # Find the lp, given the ball position, taOff and the attack angle
    # aa+180 because u want the tagental offset extended to behind the ball
    # the point where the dog leaves the locus (leaving point)

    lpx,lpy = hMath.getPointRelative(bx, by, (aa + 180), taOff)

    
    # Find the center of the circle, given the lp, the radius and attack angle, and where u are.
    
    angleFromBall       = hMath.getHeadingBetween(bx,by,myx,myy)
    angleRelativeToAA   = hMath.normalizeAngle_180 ( angleFromBall - aa )

    if angleRelativeToAA >= 0 and angleRelativeToAA <= 180:
        turndir = Constant.dANTICLOCKWISE
    else:
        turndir = Constant.dCLOCKWISE
 
    if turndir == Constant.dCLOCKWISE:
        cx,cy = hMath.getPointRelative (lpx, lpy, (aa - 90), (radius + raOff))
    else:
        cx,cy = hMath.getPointRelative (lpx, lpy, (aa + 90), (radius + raOff))

    c2my = hMath.getDistanceBetween (cx, cy, myx, myy)         # distance btw me and center
    errL = radius - c2my        # Indicates inside or outside of the circle. Used as adjustment in vector field.

    
    # If not outside of the circle, no point to use alpha(Angle between center and cloest tangent point)
    if c2my <= radius:
        alpha = 0
    else:
        alpha = hMath.RAD2DEG(math.asin(radius / c2my))  # 0 <= alpha <= 90

    beta = hMath.getHeadingBetween(myx,myy,cx,cy)    # Angle between global x-axis and center

    # Fing the current vector
    if turndir == Constant.dANTICLOCKWISE: 
        vectorArr = beta - 90               # Find the tangent vector perp to the c2my line first
        if errL > 0:
            vectorArr = vectorArr - (errL / radius * 45.0)          # linear discrepency
        elif errL < 0:                                              # When outside of the circle, the vector is
            vectorArr = beta - alpha                                # the tangent closest to the circle.
    else:
        vectorArr = beta + 90
        if errL > 0:
            vectorArr = vectorArr + (errL / radius * 45.0)
        elif errL < 0:
            vectorArr = beta + alpha
            
    b2my               = hMath.getDistanceBetween(bx,by,myx,myy)
    angleFromCenToMe   = hMath.normalizeAngle_180(hMath.getHeadingBetween(cx,cy,myx,myy)-aa)
    distanceToAALine   = b2my * math.sin(hMath.DEG2RAD(angleRelativeToAA))
##~     verticalDistToBall = b2my * math.cos(hMath.DEG2RAD(angleRelativeToAA))
    
    if (raOff > 0 and abs(distanceToAALine) < raOff):
        vectorArr = aa
##~         leftAdjust = -hMath.CLIP(distanceToAALine,3)

    elif (turndir==Constant.dANTICLOCKWISE and 
            angleFromCenToMe < 0 and angleFromCenToMe > -90 and
            abs(distanceToAALine) < (raOff + radius) ):
        sHoverToBall.DecideNextAction()
        Indicator.showFacePattern([1,2,1,2,0])
##~         reverse(aa,turndir,taOff,verticalDistToBall)
        return
            
    elif (turndir==Constant.dCLOCKWISE and
            angleFromCenToMe < 90 and angleFromCenToMe > 0 and 
            abs(distanceToAALine) < (raOff + radius) ):
        sHoverToBall.DecideNextAction()
        Indicator.showFacePattern([1,2,1,2,0])
##~         reverse(aa,turndir,taOff,verticalDistToBall)
        return

    
    # After the vector is found, the PID controller will handle the job.
    
    # Note:
    # When u tune, first tune the KP to improve the rise time, then the KD to improve
    # the overshoot, finally the KI for better steady state.
    # better see http:##www.engin.umich.edu/group/ctm/PID/PID.html before u touch them
    
    # thisTurn - The vector that brings the dog heading to the vector arrow
    
    thisTurn = hMath.CLIP ( hMath.normalizeAngle_180 (vectorArr - myh) , 30.0 )

    # For normal walk , kp = 40 ki = 8 kd = 5
            
    KP            = kp/100.0             #0.5   ## Less Than 1
    KD            = kd/100.0             #0.1
    KI            = ki/100.0             #0.1  ## Has to be very small
 
    if skippedFrames > 0:
        sum_reqTurn  = sum_reqTurn * math.pow(0.8,skippedFrames)
        last_reqTurn = 0

    # clip the running sum to be as most causing a 8 degree change.
    if abs(sum_reqTurn) > (3.0/KI):
        sum_reqTurn = hMath.CLIP (sum_reqTurn, 3.0/KI)

    # if far away, there is no I and D effects, only the P.

    if c2my > (4.0*radius):
        sum_reqTurn  = 0
        last_reqTurn = thisTurn

    # The real PID calculations
    #print thisTurn
    #print last_reqTurn
    #print sum_reqTurn
    
##~     print "==========new frame"
##~     print "ball pos %.2f , %.2f and my h %.2f" % (bx,by,myh)
##~     print "lp pos %.2f , %.2f and cx pos %.2f , %.2f" % (lpx,lpy,cx,cy)
##~     print "vectorArr %.2f   and thisTurn %.2f " % (vectorArr,thisTurn)
##~     print "last  %.2f and sum_reqTurn  %.2f) " % (last_reqTurn,sum_reqTurn)
##~     print "P %.5f   I %.5f    D %.5f " % (KP * thisTurn,KI * sum_reqTurn,\
##~         KD * (thisTurn - last_reqTurn))
##~        
    reqTurn = (KP * thisTurn) + (KI * sum_reqTurn) + (KD * (thisTurn - last_reqTurn))
##~     print "the reqTurn is %.2f" % (reqTurn)

    sum_reqTurn  = sum_reqTurn + thisTurn           # The integral part
    last_reqTurn = thisTurn                         # Used in the derivative part

    # Print the debugging info
    Action.walk(7, 0, hMath.CLIP(reqTurn, 30), Action.EllipticalWalk)

##~     print "Direction: " , turndir , "vectorArr %.2f" % (vectorArr)

    if ( (angleRelativeToAA > 160 or angleRelativeToAA < -160) and
         abs(hMath.normalizeAngle_180(myh-aa)) < 20 ):
        sPawKick.firesPawKick(sPawKick.FIRE_PAWKICK_AUTO)
        Indicator.showFacePattern([1,1,1,1,0])


# --------------------------------------
# Note:
# Here I assume taOff > 2*ball diameter = 16.8,
# that is the center of the locus is behind the ball relative to the aa line
# and also assume that the quarter circle region boundary are the stright
# line between the 2 centers

##~ def reverse(radius,aa,turndir,taOff,raOff,verticalDistToBall,distanceToAALine):

def reverse(aa,turndir,taOff,verticalDistToBall):

    adjustTurn      = hMath.CLIP(hMath.normalizeAngle_180(Global.selfLoc.getHeading()-aa),10)
    adjustTurn     *= -1
##~     correctionLen   = max (0,taOff-2*Constant.BallDiameter)

    if (verticalDistToBall > -Constant.BallDiameter):
    
        Action.walk(-6,0,adjustTurn)
##~         print "uppest"
        
##~     elif (verticalDistToBall > (Constant.BallDiameter-taOff) ):

    elif (verticalDistToBall > -taOff / 2.0 ):
    
##~         correctAmp   = correctionLen-abs(Constant.BallDiameter+verticalDistToBall)
##~         reverseSpeed = -6.0 * correctAmp / correctionLen
        
        if (turndir == Constant.dANTICLOCKWISE):
            Action.walk(-3,-4,adjustTurn)
        else:
            Action.walk(-3,4,adjustTurn)
            
##~         print "middle"
            
    else:
    
        if (turndir == Constant.dANTICLOCKWISE):    
            Action.walk(-2,-4,adjustTurn)
        else:
            Action.walk(-2,4,adjustTurn)
        
##~         print "lowest"
##~     elif (verticalDistToBall > -(taOff - radius/2.0) ):
##~         if (turndir == Constant.dANTICLOCKWISE):
##~             HelpLong.walk(-4,-3,adjustTurn)
##~         else:
##~             HelpLong.walk(-4,3,adjustTurn)
##~             
##~     else:
##~         if (turndir == Constant.dANTICLOCKWISE):
##~             HelpLong.walk(0,-4,adjustTurn)
##~         else:
##~             HelpLong.walk(0,4,adjustTurn)
