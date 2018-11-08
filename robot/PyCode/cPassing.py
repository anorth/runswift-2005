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



import scGrabDribble
import VisionLink
import Constant
import sFindBall
import Global
import hMath
import sGrab
import sActiveLocalise
import hTrack
import Action
import sDodgyDog
import sSelKick
import Indicator 
import sAvoidOwnGoal
import sDribble
import sGrabTurnKick
import sTurnKick
import sPawKick
import sUpennKick
import hTeam
import rAttacker

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

    
#default boxes of obstacle
allBox       = 0
leftBox      = 0
rightBox     = 0
leftLongBox  = 0
rightLongBox = 0

point1 = [-SIDE_MAX_DETECTION, 80]
point2 = [0, 50]
point3 = [0, 80]
point4 = [SIDE_MAX_DETECTION, 50]
point_leftbox_topleftlong = [-SIDE_MAX_DETECTION, 150]
point_rightbox_topleftlong = [0, 150]

LOOK_OFFSET = 50
LOOK_OFFSETX = 50
YDISTANCE_TO_BALL = 200 #(SQUARED)
YDISTANCE_TO_BALL_SQR = YDISTANCE_TO_BALL*YDISTANCE_TO_BALL
POS_ACCURACY = 100
lastGoToCenter = 0
startGoToCenter  = 0
gUseDodgyDog = 0
gLastFrameCalled = 0
gLastActivelyLocalisedFrame = 0
gActiveLocaliseTimer = 0
gSelectedKick = 0

LOCALISE_ACTIVATE_BALL_RANGE = 80
LOCALISE_DEACTIVATE_BALL_RANGE = 60 
gLastBallX = 0 
gLastBallY = 0
pos = None
friendPos = None
isPlayerSet = False
friend = None
friendloc = None
centerPos = None

LOCALISE_TIMER = 8

def localise():
    hTrack.stationaryLocalise(4, 50, -50)

def calculateObstacleBox():
    global allBox,leftBox,rightBox,leftLongBox,rightLongBox

    
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

def resetPerform(): 
    global gSelectedKick
    global gActiveLocaliseTimer 
        
    gSelectedKick = None
    gActiveLocaliseTimer = 0

def shouldIActivelyLocalise():     
    rAttacker.shouldIActivelyLocalise()
    
def doActivelyLocalise(): 
    global gLastActivelyLocalisedFrame
    gLastActivelyLocalisedFrame = Global.frame
       
    sActiveLocalise.DecideNextAction()
    Action.walk(Action.MAX_FORWARD,0,0,minorWalkType=Action.SkeFastForwardMWT)    
    Global.lostBall = 0

def shouldIEndActivelyLocalise():
    global gActiveLocaliseTimer
    gActiveLocaliseTimer -= 1
    if gActiveLocaliseTimer <= 0:
        return True
    if Global.ballD < LOCALISE_DEACTIVATE_BALL_RANGE:
        return True
    return False

def perform(params = None):
    global gLastFrameCalled
    global gLastDecisionFunction
    global pos, friendPos, isPlayerSet, friend, friendloc, centerPos
    hTeam.sendWirelessInfo()
    
    id(params) # ignore
    if not isPlayerSet:
        if Global.myPlayerNum == 3:
            friend = Global.teamPlayers[1]
            friendloc = Global.teammatesLoc[1]
            centerPos = (Constant.FIELD_WIDTH/2,Constant.GOALBOX_DEPTH)
        else:
            friend = Global.teamPlayers[2]
            friendloc = Global.teammatesLoc[2]
            centerPos = (Constant.FIELD_WIDTH/2, Constant.FIELD_LENGTH - Constant.GOALBOX_DEPTH)
        isPlayerSet = True
        print "Player no:", Global.myPlayerNum
        
    
    friendPos = friendloc.getPos()
    pos = Global.selfLoc.getPos()
    #if friend grab, go to center position, while looking at obstacle, get facing ball.
    #if see ball far away, get behind ball (don't pass half the field) while scanning for ball and obstacle.
    #if see ball < 1/2 field, chase and grab, turn no obstacle, until facing friend or gap upfield. and kick if obstacle near, if not stop realease, and scan look for obstacle and ball. 
    #if got wirelessball get behind wireless ball on keep a distance with obstacle, scan.
    #if doesn't see and doesn't have wireless ball, find ball and goto base spot to spin again.
    
    calculateObstacleBox()

    shouldIBeDodgy = gUseDodgyDog

    if gLastFrameCalled != Global.frame - 1: 
        resetPerform()
    gLastFrameCalled = Global.frame
    
#     elif shouldIKick():
#         #shouldIBeDodgy = False  #Really?
#         doKick()     
    if (sGrab.isGrabbed):
        print "friend is in :", friendPos[0], " " , friendPos[1]
        r = scGrabDribble.performToPosition((friendPos[0],friendPos[1]))
#         if r == Constant.STATE_EXECUTING:
#             return
#         else: 
#             scGrabDribble.resetPerform()
        #align to friend gps, while avoiding the obstacle.
        #and the pass to him.
        
    elif (sGrab.grabbingCount > 0):
        #print "grabbing"
        r = sGrab.perform(doGetBehindv=sFindBall.GET_BEHIND_NONE)
        if r == Constant.STATE_EXECUTING:
            return
        elif r == Constant.STATE_FAILED: 
            sGrab.resetPerform()
        
    elif shouldIFindSpace():
        #print "Find space"
        doFindSpace()        

    elif not shouldIEndActivelyLocalise()\
        or shouldIActivelyLocalise():
        #print "active loc"
        doActivelyLocalise()    
 
    elif (Global.vBall.isVisible() or Global.lostBall < Constant.LOST_BALL_GPS or friend.hasSeenBall()):
        #print "see a ball"
        #if (friend.getTimeToReachBall() < selfobj.getTimeToReachBall() + 1000):
        if (centerPos[1] < Constant.FIELD_LENGTH/2):
            isOtherHalf = Global.ballY > Constant.FIELD_LENGTH/2
        else:
            isOtherHalf = Global.ballY < Constant.FIELD_LENGTH/2
            
        if (isOtherHalf) : #And no near obstacle and Global.ballD > 30):
            #print "ball the other end"
            #if hMath.getDistSquaredBetween(pos[0],pos[1],Global.ballX,Global.ballY) > YDISTANCE_TO_BALL_SQR:
            if (abs(Global.ballX - pos[0]) > YDISTANCE_TO_BALL):
                sFindBall.perform(True)
                #time to ball ? to get the ball if the other one is slow
                if (centerPos[1] < Constant.FIELD_LENGTH/2):
                    hTrack.saGoToTarget(Global.ballX, max(pos[1],Global.ballY-YDISTANCE_TO_BALL))#and ywhere the same or more if y diff less than 2 meter)
                else:
                    hTrack.saGoToTarget(Global.ballX, max(pos[1],Global.ballY+YDISTANCE_TO_BALL))#and ywhere the same or more if y diff less than 2 meter)
            else:
                sFindBall.perform(True)
                if (centerPos[1] < Constant.FIELD_LENGTH/2):
                    movePoint = (max(LOOK_OFFSETX, min(Global.ballX,Constant.FIELD_WIDTH - LOOK_OFFSETX)), Global.ballY-YDISTANCE_TO_BALL)
                else:
                    movePoint = (max(LOOK_OFFSETX, min(Global.ballX,Constant.FIELD_WIDTH - LOOK_OFFSETX)), Global.ballY+YDISTANCE_TO_BALL)
                
                hTrack.saGoToTargetFacingHeading(movePoint[0], movePoint[1],hMath.getHeadingBetween(pos[0],pos[1],Global.ballX, Global.ballY))
        else:
            #print "Grab"
            r = sGrab.perform(doGetBehindv=sFindBall.GET_BEHIND_NONE)
            if r == Constant.STATE_EXECUTING:
                return
            elif r == Constant.STATE_FAILED: 
                sGrab.resetPerform()
    else: #if shouldIGoToCenter():
        #print "goto center"
        doGotoCenter()
    
    # Use dodgy dog if required
    fwdCmd = Action.finalValues[Action.Forward]
    shouldIBeDodgy = shouldIBeDodgy and (not sGrab.isGrabbed) \
                    and sGrab.grabbingCount == 0 \
                    and Global.ballD > sGrab.CLOSE_DIST \
                    and fwdCmd > Action.MAX_SKE_FWD_SPD/2 \
                    and fwdCmd > Action.finalValues[Action.Left] \
                    and sDodgyDog.shouldIBeDodgyToBall()
                    
    if shouldIBeDodgy:
        sDodgyDog.dodgyDogToBall()
        
def doFindSpace():
    hTrack.saGoToTargetFacingHeading(centerPos[0],centerPos[1],hMath.getHeadingBetween(pos[0],pos[1],Global.ballX, Global.ballY))
    
def shouldIFindSpace():
    if (friend.hasGrabbedBall()):
        return True
    return False
    
def doTrackBall(headOnly=False):
    sFindBall.perform(headOnly) 

def shouldIKick():        
    global gSelectedKick
    global gLastBallX
    global gLastBallY
    
    if gSelectedKick != None\
        and (sGrab.isGrabbed
            or sGrab.grabbingCount > 0
            or Action.shouldIContinueKick()):
        #print "shouldIKick 1", str(sGrab.isGrabbed), "  ", str(gSelectedKick)
        return True
    
    if Global.lostBall < Constant.LOST_BALL_GPS:
        if gSelectedKick != None:
            # If the distance is less than 20 and 
            # the ball hasn't moved far.
            if Global.ballD <= 30\
                and hMath.getDistanceBetween(Global.ballX,Global.ballY,\
                                             gLastBallX,gLastBallY) < 40:
                #print "shouldIKick 2", str(sGrab.isGrabbed), "  ", str(gSelectedKick)                             
                return True

            # If this chosen kick needs to grab and the ball is close,
            # then don't give up the grab.
            if sSelKick.doINeedToGrab(gSelectedKick[0])\
                and Global.ballD <= sGrab.CLOSE_DIST + 5:
                #print "shouldIKick 3", str(sGrab.isGrabbed), "  ", str(gSelectedKick)
                return True       
        
        # If the ball has moved a lot since the last kick selection, reset.
        selectedKick = sSelKick.perform()
        
        # If the selected kick is different, then do kick reset. 
        if gSelectedKick != None and selectedKick[0] != gSelectedKick[0]:
            doKickReset()    
        gSelectedKick = selectedKick     
        
        
        gLastBallX = Global.ballX
        gLastBallY = Global.ballY
        #print "shouldIKick 4", str(sGrab.isGrabbed), "  ", str(gSelectedKick)
        return True
       
    Indicator.showFacePattern([1,1,1,1,1])        
    gSelectedKick = None        
    return False

def DecideNextAction():
    perform()
    
def doGotoCenter():
    global lastGoToCenter, startGoToCenter
    if Global.frame - lastGoToCenter > 1:
        startGoToCenter = Global.frame
    lastGoToCenter = Global.frame
    
    movePoint = (centerPos[0],centerPos[1])
    distanceToPoint = hMath.getDistanceBetween(movePoint[0],movePoint[1], pos[0], pos[1])
    if (distanceToPoint < POS_ACCURACY):
        sFindBall.perform()
    else :
        hTrack.stationaryLocalise()
        hTrack.saGoToTarget(movePoint[0],movePoint[1])
    
def shouldIGoToCenter():
    pass
# 
#     pos = Global.selfLoc.getPos()
#     if (Global.frame - lastTrackBall > 1):
#         return False
#     elif ((Global.frame - lastTrackBall < 2) and (lastGoToCenter - startGoToCenter > 60)):
#         return False
#     elif (lastTrackBall - startTrackBall > Constant.LOST_BALL_SPIN):
#         return True
#     elif (hMath.getDistanceBetween(pos[0],pos[1],Constant.FIELD_WIDTH/2.0,Constant.FIELD_LENGTH/2.0+50) < 50):
#         return False
#     return False

def doKickReset(): 
    # Resetting all the kicks    
    sAvoidOwnGoal.resetPerform()
    sDribble.resetPerform()
    scGrabDribble.resetPerform()
    sGrabTurnKick.resetPerform()
    sPawKick.resetPerform()
    sTurnKick.resetPerform()
    sUpennKick.resetPerform()     
    
         