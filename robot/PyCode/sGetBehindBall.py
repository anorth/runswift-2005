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
import hTrack
import Indicator
import math
import sFindBall


gLastCalledFrame = 0

### Constants for get-behind direction. Specify INSIDE to move around
# towards the middle of the field, OUTSIDE to move around closer to
# the sidelines. They will only have effect when the robot is actually
# forward of the ball; from behind the robot just walks directly to its
# position
INSIDE = 0
OUTSIDE = 1


def DecideNextAction():
    Indicator.setDefault()
    if Global.lostBall < Constant.LOST_BALL_GPS:
        r = performBall()
        sFindBall.perform(True)
            
        if r == Constant.STATE_SUCCESS:
            Indicator.showFacePattern([5,3,3,3,5])
            resetPerform()
            Action.stopLegs()    
    else:
        resetPerform()
        sFindBall.perform()
        
    

def resetPerform():
    global gDirection
    gDirection = None    

# The degrees round the circle from the robot's direction that we put the
# circling point. 2003 used 70. Use 90 to get a tangent to the circle.
CIRCLE_DEGREES = 60

# Move to a specified distance behind the ball (or specified target point)
# facing dkd, staying at least dist cm away from the ball.
# Returns true when the dog is behind the ball and facing the ball to
# within +/- accuracy degrees.
# The side argument is not yet implemented.
gDirection = None
def perform(dkd = 90, dist = 20, direction = None, bx = None, by = None, accuracy = 20):
    global gDirection 
    
    #if side != None:
    #    print "Warning: sGetBehindBall.perform: side is not yet implemented"

    if bx == None or by == None:
        (bx, by) = Global.gpsGlobalBall.getPos()
    (myx, myy) = Global.selfLoc.getPos()
    myh = Global.selfLoc.getHeading()


    # Work out if we are to the left or right of the ball (relative to DKD as 0)
    lRobotH = hMath.getHeadingToMe(bx, by, dkd, myx, myy)      
    
    if direction == None and gDirection == None:        
        if lRobotH < 0:  # robot to the left
            #print "robot to left of ball",
            gDirection = Constant.dANTICLOCKWISE
        else:           # robot to the right
            #print "robot to right of ball",
            gDirection = Constant.dCLOCKWISE
            
    elif direction != None: 
        gDirection = direction
            
    # The circling point can be calculated as looking from the ball
    # towards the robot at CIRCLE_DEGREES to the left/right, and distance
    # dist. CircleAng is from the ball facing the robot with positive x
    # at zero degrees
#    if robotH > 180 - CIRCLE_DEGREES:
#        # If we are within CIRCLE_DEGREES of the target point don't overshoot
#        circleAng = 90 + (180 - robotH)
#    elif robotH < -180 + CIRCLE_DEGREES:
#        circleAng = 90 - (-180 + robotH)
#    else:
    if gDirection == Constant.dANTICLOCKWISE:
        circleAng = 90 + CIRCLE_DEGREES
    else:
        circleAng = 90 - CIRCLE_DEGREES
    circleAng = hMath.normalizeAngle_180(circleAng)

    #print ""  
    #print "local robot H ", lRobotH
    # relative to target facing robot
    
    # This factor is used to adjust the dodgyness of the sidewards and 
    # backwards ability of fast skelliptical walk.
    factor = 1 #max(min(abs(180-lRobotH)/60.0,2),1)
    lcx = math.cos(hMath.DEG2RAD(circleAng)) * dist * factor
    lcy = math.sin(hMath.DEG2RAD(circleAng)) * dist * factor
    
    #print "circleAng", circleAng, "rel circle pos", circleRelX, circleRelY
    
    robotH = hMath.normalizeAngle_0_360(lRobotH + dkd) # now global
    cx, cy = hMath.getGlobalCoordinate(bx, by, robotH, lcx, lcy)                                       
    
    #print " local circle pos : (", lcx ,",",lcy,")"
    #print "my pos : (", myx, ",", myy, ",",myh,")"                                                
    #print "global robotH ", robotH
    #print "global ball :(", bx, ",", by, ") global circle pos : (", cx ,",",cy,")"
    # circleX/Y now is the global coords of the circle point, so walk there.

    bh = hMath.getHeadingBetween(myx,myy,bx,by)   # global
    lbh = hMath.normalizeAngle_180(bh - myh)
        
    lcx, lcy = hMath.getLocalCoordinate(myx,myy,myh,cx,cy)
    #lcdSquared = hMath.getDistSquaredBetween(0,0,lcx,lcy)
    Action.walk(lcy,lcx,lbh,"ddd",minorWalkType=Action.SkeFastForwardMWT)

    if abs(hMath.normalizeAngle_180(bh - dkd)) < accuracy and abs(lbh) < accuracy:
        resetPerform()
        return Constant.STATE_SUCCESS
    
    return Constant.STATE_EXECUTING




gLastCalledFrame = 0
def performBall(dkd = 90, dist = 20, direction = None, accuracy = 20):
    global gLastCalledFrame
    global gDirection 
    global gLastCalledFrame
    
    
    
    bx, by = Global.ballX, Global.ballY
    myx, myy = Global.selfLoc.getPos()
    myh = Global.selfLoc.getHeading()

    # If this function wasn't called in previous frame, then reset the direction.
    if Global.frame == gLastCalledFrame: 
        gDirection = None
    gLastCalledFrame = Global.frame

    # Work out if we are to the left or right of the ball (relative to DKD as 0)
    lRobotH = hMath.getHeadingToMe(bx, by, dkd, myx, myy)
    if abs(lRobotH) > 70:
        gDirection = None
    
    if direction == None and gDirection == None:        
        if lRobotH < 0:  # robot to the left
            #print "robot to left of ball",
            gDirection = Constant.dANTICLOCKWISE
        else:           # robot to the right
            #print "robot to right of ball",
            gDirection = Constant.dCLOCKWISE            
            
    elif direction != None: 
        gDirection = direction

    if gDirection == Constant.dANTICLOCKWISE:
        circleAng = 90 + CIRCLE_DEGREES
    else:
        circleAng = 90 - CIRCLE_DEGREES
    circleAng = hMath.normalizeAngle_180(circleAng)
    
    # This factor is used to adjust the dodgyness of the sidewards and 
    factor = max(min(abs(180-lRobotH)/90.0,1),0)
    lcx = math.cos(hMath.DEG2RAD(circleAng)) * dist * factor
    lcy = math.sin(hMath.DEG2RAD(circleAng)) * dist * factor
    
    robotH = hMath.normalizeAngle_0_360(lRobotH + dkd) # now global
    cx, cy = hMath.getGlobalCoordinate(bx, by, robotH, lcx, lcy)                                       
    
    bh = Global.ballH
        
    lcx, lcy = hMath.getLocalCoordinate(myx,myy,myh,cx,cy)
    
    
    if abs(bh) > 30 and Global.ballD > dist:
        Action.walk(0,0,bh,minorWalkType=Action.SkeFastForwardMWT)    
    else:
        Action.walk(lcy,lcx,bh*0.75,"ddd",minorWalkType=Action.SkeFastForwardMWT)


    if abs(hMath.normalizeAngle_180(myh + bh - dkd)) < accuracy and abs(bh) < accuracy:
        resetPerform()
        return Constant.STATE_SUCCESS
    
    gLastCalledFrame = Global.frame
    
    return Constant.STATE_EXECUTING
    
