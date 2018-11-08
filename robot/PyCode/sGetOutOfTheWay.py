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


import Constant
import Indicator
import Global
import hMath
import hTrack
import math


def DecideNextAction(): 
    import hFWHead
    import hWhere
    import Indicator
        
    targetX, targetY = hWhere.getSupporterPos()   
    targetH = Global.selfLoc.getHeading() + Global.ballH
        
    r = perform(Global.ballX,Global.ballY,\
                Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y,\
                targetH)                   
    if r == Constant.STATE_EXECUTING: 
        Indicator.showHeadColor([True,True,True])
    else:         
        Indicator.showHeadColor([False,False,False])
        hTrack.saGoToTargetFacingHeading(targetX,targetY,targetH)

    hFWHead.DecideNextAction()
    

# Gets out of the way of the line between (fromX, fromY) and (toX, toY)
# (the ball and target goal by default). Returns True if any action was
# taken, False if we were not in the way. 
# This function will also try to move the robot down field. 
def perform(fromX, fromY, toX, toY, targetH, doForce=False, getOutOfTheWayDist=60): 

    selfX, selfY = Global.selfLoc.getPos()
    
    # If you are really further away from the "from" cords, then don't do anything. 
    if hMath.getDistSquaredBetween(selfX,selfY,fromX,fromY) > hMath.SQUARE(250):
        return Constant.STATE_SUCCESS
    
    
    dist, head = amIInTheWay(fromX,fromY,toX,toY)
    
    if dist != None\
        and (doForce or abs(dist) < getOutOfTheWayDist):
        rise = fromY - toY
        run = fromX - toX
        if rise == 0: 
            rise = 0.01
        if run == 0:
            run = 0.01
        
        # gradient of the line between from and to
        m1 = rise / (run + 0.0)
        b1 = fromY - m1 * fromX
        
        # perpendicular gradient to the line between from and to
        m2 = - run / (rise + 0.0)
        b2 = selfY - m2 * selfX             
                
        # on the right side
        if dist > 0:
            newX = selfX + 50
            newY = m2 * newX + b2                   
                
        # on the left side       
        else: 
            newX = selfX - 50
            newY = m2 * newX + b2
            
        # Adjust newY, so that the robot would move downwards. 
        temp = m1 * newX + b1
        if newY > temp:
            newY = selfY
        else: 
            newY -= (50.0 / abs(getOutOfTheWayDist)) * abs(dist)
                    
        newX = max(newX,0)
        newX = min(newX,Constant.FIELD_WIDTH)    
        newY = max(newY,0)
        newY = min(newY,Constant.FIELD_LENGTH)
        
        hTrack.saGoToTargetFacingHeading(newX,newY,targetH)
        
        Indicator.showFacePattern([0,0,2,0,0])
        return Constant.STATE_EXECUTING
        
    else:        
        return Constant.STATE_SUCCESS      


# Returns the perpendicular distance away of the current robot location from
# the line between (x, y) and (targetX, targetY) and the heading (global
# degrees) away from the line, or (None, None) if we are not between the
# points. Dist will be negative if we are to the (global) left of the line,
# positive to the right.
# E.g. if (x, y) is the ball and target is the goal then returns the clearance
# from another robot's shot on goal
def amIInTheWay(x, y, targetX, targetY):
    myX, myY = Global.selfLoc.getPos()

    if x == None or y == None or targetX == None or targetY == None:
        return (None, None)

#    if Global.frame % 30 == 0:
#        print "amIInTheWay", x, y, targetX, targetY, "me", myX, myY
    
    # We have a line in two point form, find offset of selfLoc from the line
    dist = ((targetX - x)*(y - myY) - (x - myX)*(targetY - y)) / \
            math.sqrt(hMath.SQUARE(targetX - x) + hMath.SQUARE(targetY - y))

#    if Global.frame % 30 == 0:
#        print "I'm", dist, "away from in the way",
        
    # If we're not between the points return None
    dx = x - targetX
    dy = y - targetY
    if abs(dx) > abs(dy) and \
            ((myX > x and myX > targetX) or (myX < x and myX < targetX)):
#        if Global.frame % 30 == 0:
#            print "but not between the points (X)"
        return (None, None)
    elif abs(dy) > abs(dx) and \
            ((myY > y and myY > targetY) or (myY < y and myY < targetY)):
#        if Global.frame % 30 == 0:
#            print "but not between the points (Y)"
        return (None, None)

    # The heading away is perpendicular to the target line
    if dy == 0:
        dy += 0.1     # prevent division by zero
    head = - dx / dy 
    head = hMath.RAD2DEG(math.atan(head))
 
    # Which way along the line is away? If dist is positive we are to the left
    # of the line and want heading in the left half
    if (dist > 0):
        head += 180
    head = hMath.normalizeAngle_180(head)

    if Global.frame % 30 == 0:
        print "dx =", dx, "dy =", dy, "head =", head
        
#    if Global.frame % 30 == 0:
#        print "and between the points. Head", head, "to escape"

    return (dist, head)

def getOutOfTheCircle(centerX,centerY,targetH,doForce=False,radius=40):
    selfX, selfY = Global.selfLoc.getPos()        
    dist = hMath.getDistanceBetween(centerX,centerY,selfX,selfY)
    heading = hMath.getHeadingBetween(centerX,centerY,selfX,selfY)     
    
    if amIInTheCircle(centerX,centerY,radius)\
        or doForce: 
        
        if dist > radius:
            relX = dist * 2 * math.cos(hMath.DEG2RAD(heading))
            relY = dist * 2 * math.sin(hMath.DEG2RAD(heading))
            
        else:
            relX = radius * 2 * math.cos(hMath.DEG2RAD(heading))
            relY = radius * 2 * math.sin(hMath.DEG2RAD(heading))
        
        destX = centerX + relX
        destY = centerY + relY 
        
        hTrack.saGoToTargetFacingHeading(destX,destY,targetH)

        Indicator.showFacePattern([0,2,0,2,0])
        return Constant.STATE_EXECUTING
        
    return Constant.STATE_SUCCESS


def amIInTheCircle(targetX,targetY,radius):
    selfX, selfY = Global.selfLoc.getPos()    
    distSquared = hMath.getDistSquaredBetween(targetX,targetY,selfX,selfY) 
    
    return distSquared < hMath.SQUARE(radius)
            
