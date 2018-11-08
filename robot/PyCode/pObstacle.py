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


# DoNothing.py
#
# DoNothing is a simple high-level Behaviour/Role that just stands there.

import Indicator
import VisionLink
import Constant
import Action
#import Debug
import Global
import hMath
import sGrab
#import sGrabSideWalk
#import sGetBehindBall

import hTrack
#import hWhere
#import sFindBall
#import pForward

indCycle = 0
grabObject = None
grabbing_time_when_stop = 0
MAX_GRABBING_TIME_WHEN_STOP = 10
# minimum y is 50 recommended 

counter = 0
shouldGrab = False
RIGHT = 8
LEFT = -8
moveTo = 0
GRAB_FRAME_MAX = 30*2
MIN_POINTS_CONSIDERED_AS_OBSTACLE = 10     
SIDE_MAX_DETECTION = 200
framesLastSentYUV = 0

def localise():
    hTrack.stationaryLocalise(4, 50, -50)
#    hTrack.stationaryLocalise(5, -40, -50)
    #sFindBall.perform()
    #Action.stopLegs()
    
haveObstacle = 0
isCorrectstandingPosition = False
distanceOK = False
localiseTime = 120
lastHeading = None
lostObstacle = 100
balltracking = False
balltrackingcounter = 60
#default boxes of obstacle
allBox       = 0
leftBox      = 0
rightBox     = 0
leftLongBox  = 0
rightLongBox = 0

def calculateObstacleBox():
    global allBox,leftBox,rightBox,leftLongBox,rightLongBox

    point1 = [-SIDE_MAX_DETECTION, 80]
    point2 = [0, 50]
    point3 = [0, 80]
    point4 = [SIDE_MAX_DETECTION, 50]
    point_leftbox_topleftlong = [-SIDE_MAX_DETECTION, 150]
    point_rightbox_topleftlong = [0, 150]
    
    allBox   = VisionLink.getNoObstacleInBox(\
            point_leftbox_topleftlong[0],\
            point_leftbox_topleftlong[1],\
            point4[0],point4[1])
    leftBox  = VisionLink.getNoObstacleInBox(point1[0],point1[1],point2[0],point2[1])
    rightBox = VisionLink.getNoObstacleInBox(point3[0],point3[1],point4[0],point4[1])
    leftLongBox  = VisionLink.getNoObstacleInBox(\
        point_leftbox_topleftlong[0],point_leftbox_topleftlong[1],\
        point2[0],point2[1])
    rightLongBox = VisionLink.getNoObstacleInBox(\
        point_rightbox_topleftlong[0],point_rightbox_topleftlong[1],\
        point4[0],point4[1])
    
def DecideNextAction():
    global indCycle,grabObject,counter,shouldGrab,moveTo
    global haveObstacle, isCorrectstandingPosition, distanceOK, framesLastSentYUV
    global localiseTime, lastHeading,lostObstacle
    global allBox,leftBox,rightBox,leftLongBox,rightLongBox
     
    Indicator.superDebug()
    
    point4 = [SIDE_MAX_DETECTION, 50]
    point_leftbox_topleftlong = [-SIDE_MAX_DETECTION, 150]
    
    shouldHeading = None
    calculateObstacleBox()
        
    if False:
        print
        print "isCorrectstandingPosition:",isCorrectstandingPosition
        print "shouldGrab:",shouldGrab
        print "haveObstacle:",haveObstacle
        print "lostObstacle:",lostObstacle
        print "Global.BallD:",Global.ballD
    
    isCorrectstandingPosition = True 
    if (Global.vBall.isVisible() and Global.ballD > 30):
        isCorrectstandingPosition = False
            
    if shouldGrab or not isCorrectstandingPosition :
        print "shouldGrab or find ball and stop"
        if grabObject == None:
            grabObject = GrabGenerator()
        try:
            grabObject.next()
        except StopIteration:
            reset()
    else : 
        print "stand still"
        Action.standStill()
        result =\
            VisionLink.getPointToNearestObstacleInBox(\
            point_leftbox_topleftlong[0],\
            point_leftbox_topleftlong[1],\
            point4[0],point4[1],
            5\
            )

#         if (leftlong > MIN_POINTS_CONSIDERED_AS_OBSTACLE and leftlong > rightlong):
#             haveObstacle = haveObstacle + 1
#             print "haveObstacle", haveObstacle
#             print "checking left"
#             if result == None:
#                 print "error it should be checking left but return None"
#                 shouldHeading = None
#                 localise()
#             else:
#                 pointX, pointY = result
#                 shouldHeading = hMath.getHeadingToMe(0,0,0,pointX, pointY)
#         elif (rightlong > MIN_POINTS_CONSIDERED_AS_OBSTACLE and rightlong > leftlong):
#             haveObstacle = haveObstacle + 1
#             print "haveObstacle", haveObstacle
#             print "checking right"
#             if result == None:
#                 print "error it should be checking right but return None"
#                 shouldHeading = None
#                 localise()
#             else:
#                 pointX, pointY = result
#                 shouldHeading = hMath.getHeadingToMe(0,0,0,pointX, pointY)
#         else :
#             localise()
#             if haveObstacle > 0:
#                 print "haveObstacle", haveObstacle
#                 haveObstacle = 0
        if result == None:
            if (lostObstacle > 3 and lastHeading != None) :
                shouldHeading = lastHeading
                print "just lost"  
            else:                
                localise()
                print "no nearest obstacle"
            haveObstacle = 0
            lostObstacle = lostObstacle + 1
            lastHeading = None
        else:
            print "result is not none" , result
            pointX, pointY = result
            shouldHeading = hMath.getHeadingToMe(0,0,0,pointX, pointY)
            print "pointing",  -pointX, ":", pointY
            print "heading:", shouldHeading
            lastHeading = shouldHeading    
        if (shouldHeading != None): 
            #localise()
            calculate = int((shouldHeading-90.0)/90 * 80)
            if (framesLastSentYUV > 30):
                 VisionLink.sendYUVPlane()
                 framesLastSentYUV = 0
            hTrack.stationaryLocalise(2, calculate + 3, calculate - 3)
            
            #Action.setHeadParams(-pointX, 0, pointY, Action.HTAbs_xyz)
            
            print "calculate:", calculate
        
        if result != None and (leftBox > MIN_POINTS_CONSIDERED_AS_OBSTACLE and leftBox > rightBox):
            print "move left:",haveObstacle
            haveObstacle = haveObstacle + 1
            lostObstacle = 0
            if (framesLastSentYUV > 30):
                 VisionLink.sendYUVPlane()
                 framesLastSentYUV = 0
            if (haveObstacle > 3):
                shouldGrab = True
                moveTo = LEFT
        elif result != None and (rightBox > MIN_POINTS_CONSIDERED_AS_OBSTACLE and rightBox > leftBox):
            print "move right:",haveObstacle
            haveObstacle = haveObstacle + 1
            lostObstacle = 0
            if (framesLastSentYUV > 30):
                 VisionLink.sendYUVPlane()
                 framesLastSentYUV = 0
            if (haveObstacle > 3):
                shouldGrab = True
                moveTo = RIGHT
    if False : #all > 0 :# and counter == 60:
        print
        print "all:",allBox,"endall"
        print "left:",leftBox,"endleft"
        print "right:",rightBox,"endright"
        print "leftlong:",leftLongBox,"endleft"
        print "rightlong:",rightLongBox,"endright"
        counter = 0
    counter = counter + 1            
    framesLastSentYUV = framesLastSentYUV + 1

def reset():
    global grabObject, shouldGrab,moveTo,isCorrectstandingPosition,distanceOK,grabbing_time_when_stop
    grabObject = None
    Action.closeMouth()
    shouldGrab = False
    moveTo = 0
    distanceOK = False
    grabbing_time_when_stop = 0
        
def GrabGenerator():
    global counter, grabbing_time_when_stop, moveTo,isCorrectstandingPosition
    grab_counter = 0
    sensor_counter = 0
    kickType = None
    # grabbing
    local_isGrabed = False
    if not local_isGrabed:
        sGrab.resetPerform()
        while sGrab.perform() == Constant.STATE_EXECUTING:
            if not isCorrectstandingPosition and Global.ballD <= 30:#sGrab.REALLY_SLOW_DIST :
                isCorrectstandingPosition = True
                print "distance ok to stop"
                return
            yield True
        
    while local_isGrabed: 
        if grabbing_time_when_stop > MAX_GRABBING_TIME_WHEN_STOP : 
            grabbing_time_when_stop = 0
            local_isGrabed = True
        sGrab.moveHeadForward()
        Action.openMouth()
        grabbing_time_when_stop = grabbing_time_when_stop + 1 
        yield True    
        
    while not kickType:    
        
        sGrab.moveHeadForward()
        Action.openMouth()
           
        #posx,posy = Global.selfLoc.getPos()
        #print "float:",posx, posy
        #posx = int(posx)
        #posy = int(posy)
        #print "int:",posx, posy
        if moveTo == None:
            print "WARNING moveTo == None" 
        print "This is broken"
        #sGrabDribble.GrabSideWalk(moveTo)
        #kickType == Action.DiveKickWT
        
        grab_counter += 1
        
        if grab_counter % 15 != 0: 
            if VisionLink.getAnySensor(Constant.ssCHEST_INFRARED) > 138000:
                sensor_counter += 1                      
        else:
            if sensor_counter < 3:
                print "Oops, fail grabbing the ball.."
                reset()
                return
            else:
                sensor_counter = 0
        
        # Check if we've grabbed for too long
        if grab_counter > GRAB_FRAME_MAX :
            kickType = Action.DiveKickWT
            #headColor((True, False, False))
            #Action.standStill()
            #reset()
            #return # Raise a StopIteration exception        
                
        yield True
        
    # Start kick
#    if Debug.TurnAndSideMoveDebug :
    print "kicking"
    if kickType == Action.DiveKickWT:
        kickCount = 15 #30
    else: 
        kickCount = 22

    # Do kick
    for _ in range(kickCount):
        Action.kick(kickType)
        Action.openMouth()
        yield True
        
    isCorrectstandingPosition = False
    Global.lostBall = 0
    reset()
    
