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


# A skill to run at the ball from side-on, facing the ball. This can be used
# when you are running in to grab or UPenn and don't need to do a full
# get behind, but don't what to attemp to grab when facing opposite to the
# desired kick direction
#
#           DKD
#    R       ^ 
#    |       |
#    |
#     \
#      \
#        --->O

import math
import Constant
import Global
import Action
import Indicator
import hMath
import hTrack

### Constants for get-beside direction. Specify INSIDE to move around
# towards the middle of the field, OUTSIDE to move around closer to
# the sidelines. They will only have effect when the robot is actually
# forward of the ball; from behind the robot just walks directly to 
# the ball
# NOT YET IMPLEMENTED
INSIDE = 0
OUTSIDE = 1


def DecideNextAction():
    import sFindBall
    import hFWHead
    Indicator.setDefault()
    hFWHead.resetEachFrame()
    if Global.lostBall < 30:
        hFWHead.DecideNextAction()
        if perform():
            Indicator.showFacePattern([5,3,3,3,5])
    else:
        sFindBall.perform()
    
# The degrees round the circle from the robot's direction that we put the
# circling point. GetBehind uses 70.
CIRCLE_DEGREES = 70

# The radius of the circle, i.e. the distance from the ball to the point that
# the robot circles about. Smaller means the robot must turn more as it
# walks forward
CIRCLE_RADIUS = 50

# Global that tracks whether we are outside the circle and moving to it (False)
# or on the circle and tracing it (True)
onCircle = False

# Move to beside the ball, facing the ball, so that dkd is about 90 degrees to
# the left or right. Keep calling this to run at the ball.
# The side argument is not yet implemented.
def perform(dkd = 90, side = None, bx = None, by = None):
    # This implementation is very similar to sGetBehindBall (based on 2003)
    # but the ball is on the bottom edge of the circle, not the centre.
    global onCircle

    if side != None:
        print "Warning: sGetBesideBall.perform: side is not yet implemented"

    if bx == None or by == None:
        (bx, by) = Global.gpsGlobalBall.getPos()
    (myx, myy) = Global.selfLoc.getPos()
    myh = Global.selfLoc.getHeading()

    # Determine the centre of the circle, which is CIRCLE_RADIUS towards
    # dkd from the ball. Global coords.
    cx = bx + math.cos(hMath.DEG2RAD(dkd)) * CIRCLE_RADIUS
    cy = by + math.sin(hMath.DEG2RAD(dkd)) * CIRCLE_RADIUS

    # If we are backward of the ball or really close just run at it
    ballRobotH = hMath.getHeadingToMe(bx, by, dkd, myx, myy)
    ballDSquared = hMath.getDistSquaredBetween(myx, myy, bx, by)
    if (abs(ballRobotH) > 90 or ballDSquared < hMath.SQUARE(20)):
        Indicator.showHeadColor(Indicator.RGB_PURPLE)
        ballH = hMath.getHeadingBetween(myx, myy, bx, by)
        hTrack.saGoToTargetFacingHeading(bx, by, ballH)
        return

    # Work out if we are left or right of the centre (relative to DKD as 0)
    robotH = hMath.getHeadingToMe(cx, cy, dkd, myx, myy)
    # FIXME: allow choice of direction
    if (robotH > 0):  # robot to the left
        #print "robot to left of ball",
        direction = Constant.dANTICLOCKWISE
    else:           # robot to the right
        #print "robot to right of ball",
        direction = Constant.dCLOCKWISE
    
    # The circling point can be calculated as looking from the centre
    # towards the robot at CIRCLE_DEGREES to the left/right, and distance
    # CIRCLE_RADIUS. CircleAng is from the centre facing the robot with
    # positive x at zero degrees.
    # There are two modes here. In the first we are well outside the and
    # running to make a tangent with the circle. In the second we are close
    # to or inside the circle and tracing the circumference
    centreDSquared = hMath.getDistSquaredBetween(myx, myy, cx, cy)
    if (centreDSquared > hMath.SQUARE(CIRCLE_RADIUS + 20)):
        #print "Outside circle, running to tangent"
        onCircle = False
        Indicator.showHeadColor(Indicator.RGB_GREEN)
        if direction == Constant.dANTICLOCKWISE:
            circleAng = 90 + CIRCLE_DEGREES
        else:
            circleAng = 90 - CIRCLE_DEGREES
        circleAng = hMath.normalizeAngle_180(circleAng)
    else:
        #print "On circle, tracing circumference"
        onCircle = True
        Indicator.showHeadColor(Indicator.RGB_YELLOW)
        if direction == Constant.dANTICLOCKWISE:
            circleAng = 110
        else:
            circleAng = 70
        
#    print "me", int(myx), int(myy), "ball", int(bx), int(by), \
#        "centre", int(cx), int(cy), "robotH", int(robotH),
    # relative to centre facing robot
    circleRelX = math.cos(hMath.DEG2RAD(circleAng)) * CIRCLE_RADIUS
    circleRelY = math.sin(hMath.DEG2RAD(circleAng)) * CIRCLE_RADIUS
    #print "circleAng", circleAng, "rel circle pos", circleRelX, circleRelY
    robotH = hMath.normalizeAngle_180(robotH + dkd) # now global
    (circleX, circleY) = hMath.getGlobalCoordinate(cx, cy, robotH, \
                                                    circleRelX, circleRelY)
#    print "gRobotH", robotH, "circle pos", int(circleX), int(circleY)
    # circleX/Y now is the global coords of the circle point, so walk there.

#    ballH = hMath.getHeadingBetween(myx, myy, bx, by)   # global
    
    if onCircle:
        # Calls the walk directly to ensure smoothness: no stopping to turn
        relX = circleX - myx
        relY = circleY - myy
        relD = hMath.getLength((relX, relY))
        relTheta = hMath.RAD2DEG(hMath.getHeadingToRelative(relX, relY))

        # Don't turn outwards much, even if we are inside the circle. Walking
        # forward will put us back on it. Nobu can you fix this?
#        print "relTheta", relTheta, "=>",
#        if direction == Constant.dANTICLOCKWISE and relTheta < 0:
#            #relTheta = hMath.CLIP(relTheta, 15)
#            relTheta = 0
#        elif direction == Constant.dCLOCKWISE and relTheta > 0:
#            #relTheta = hMath.CLIP(relTheta, 15)
#            relTheta = 0
#        print relTheta
        left = 0
        if abs(relTheta) < 30:
            Action.walk(Action.MAX_FORWARD, left, relTheta)
        else:
            Action.walk(Action.MAX_FORWARD, left, relTheta)
    else:
        hTrack.saGoToTarget(circleX, circleY)


"""
def performNose(): 
    
    selfX, selfY = Global.selfLoc.getPos()
    selfH = Global.selfLoc.getHeading()
    
    ballX, ballY, ballH = Global.ballX, Global.ballY, Global.ballH

    # If we are getting closer to the ball and also we are behind the 
    # ball. 
    if Global.ballD < 30\
        and 180 <= selfH <= 360: 
        
        # Still not beside the ball...
        if abs(ballH) < 90:                                                       
            if ballH < 0: 
            
            else:             

                    
        # beside the ball
        else:
            
        
    else:     
        sFindBall.perform()
    
                
"""
