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


import Global
import Constant
import math
import sWLocalise

import VisionLink
import hTrack
import hMath
import Indicator
import Action

DEBUG=False

#--------------------------
# Constants
#--------------------------

# Directions
LEFT  = -1
RIGHT = 1

# Actions
MOVE_HEAD = 0
SPINNING = 1
WALK_FORWARD = 2

# Params
HEAD_SCAN_SPEED = 8         # degrees per frame
FORWARD_SPEED = Action.MAX_FORWARD  # the speed the dog walks to the ball

BALL_LOST_MAYBE = 0.95 # 6
BALL_LOST = 0.85       # 17
BALL_LOST_DEFINITELY = 0.4 # 90
BALL_LOST_VERY_LONG = 0.1 

HALF_FIELD_WIDTH = Constant.FIELD_WIDTH/2
HALF_FIELD_LENGTH = Constant.FIELD_LENGTH/2
WIDTH_TOLERANCE = 20
LENGTH_TOLERANCE = 30

PAN_LIMIT = 80

PARAM_SPIN_FAST = (Action.cmdParaWalk,
             Action.NormalWalkWT,
             0,               # Forward
             0,               # Left
             -180,             # TurnCCW
             5,               # Pg
             90,              # Hf
             110,             # Hb 
             0,
             0,
             55,              # Ffo
             10,               # Fso 
             -55,             # Bfo
             5,               # Bso
             Action.HTAbs_h,
             PAN_LIMIT,
             -50,
             5) 


#--------------------------
# Global Variables
#--------------------------
# Head Moving Params
panDir = RIGHT

#Action Vars
lastAction = None

# Ball State Info (relative to robot)
BallPosConfidence = 0.0
BallPosX = 0
BallPosY = 0
BallVelX = 0
BallDirX = LEFT
BallVelY = 0
BallVelHist = None

lastFrame = -1

#--------------------------
# Main Decision Function
#--------------------------
def DecideNextAction():
    global BallPosX, BallPosY, BallDirX, BallPosConfidence, panDir
    global lastAction, lastFrame
    
    if lastFrame == Global.frame: # if this function has been called
        return

    lastFrame = Global.frame
    
    Indicator.showBallIndicator()

    GetBallInfo()

    if BallPosConfidence < BALL_LOST_DEFINITELY:
        SearchBySpinning(BallDirX)
    elif BallPosConfidence < BALL_LOST:
        Action.stopLegs()
        SearchByHeadMoving(BallDirX)
    else:
        WalkToBall(BallPosX, BallPosY)
        if BALL_LOST_MAYBE < BallPosConfidence < 1:
            Action.setHeadToLastPoint()
        else:
            if lastAction == SPINNING:
                Action.setHeadParams(0,0,0, Action.HTAbs_h)
            else:
                hTrack.trackVisualBall()
                #PointHeadAtBall(BallPosX, BallPosY)
        if canSeeBall() and BallPosY > 80:
            # if not close to the ball
            sWLocalise.DecideNextAction()
            if sWLocalise.isLocalising:
                BallPosConfidence = 1.0



def GetBallInfo():
    global BallPosX, BallPosY, BallPosConfidence
    global BallVelX, BallVelY, BallDirX, BallVelHist
    BallPosConfidence /= 1.01  # reduce the confidence factor

    debug("sWTrack: ball confidence:", BallPosConfidence)
    
    x, y = VisionLink.getProjectedBall()
    debug("sWTrack: x:", x, "y:", y)
    
    # if we see the ball and its location seems reasonable
    if Global.vBall.getConfidence() > 0 and \
           -1000 < x and x < 1000 and -1000 < y and y < 1000:

        BallPosConfidence = 1.0
        # update velocity
        dx, dy = VisionLink.getGPSBallVInfo(Constant.CTLocal)[0:2]

        # keep a history of velocities to calculate moving avg
        if BallVelHist == None:
                BallVelHist = [(dx, dy)]*10
                BallVelX = dx
                BallVelY = dy
        else:
            BallVelX += dx/10 - BallVelHist[0][0]/10
            BallVelY += dy/10 - BallVelHist[0][1]/10
            BallVelHist.pop(0)
            BallVelHist.append((dx,dy))
            
        # finally, set the new ball position to what we saw
        (BallPosX, BallPosY) = (x, y)

    elif BallPosConfidence < BALL_LOST:
        # if we can't see the ball, take a guess of where it is
        BallPosX -= BallVelX *5        # the multiplication by 5 is converting between
        BallPosY += BallVelY *5        # two coordinate systems used

       
    if BallVelX > 0:
        BallDirX = LEFT
    else:
        BallDirX = RIGHT



def canSeeBall():
    return BallPosConfidence >= BALL_LOST_MAYBE



#--------------------------
# Helper Functions
#--------------------------
def PointHeadAtBall(x, z):

    # avoid the head to touch the ball
    adjustedZ = z - 1.0
    if Global.ballD < 25:
        adjustedZ -= 1.5

    Action.setHeadParams(x, -1.6, adjustedZ, Action.HTAbs_xyz)


# Move to the balls location (assumes we can see it)
def WalkToBall(x, y, maxSpeed = 12):
    global lastAction
    angle = -hMath.RAD2DEG(hMath.getHeadingToRelative(x, y))
    dist = math.sqrt(x*x+y*y)
    if math.fabs(angle) > 30:
        if dist < 30 or y < 0: # if close or ball is hehind the camera
            Action.walk(0,0,angle/2) # rotate on spot
        else:
            forward = hMath.CLIP(y, maxSpeed)
            Action.walk(forward, 0, angle/2)
    else:
        Action.walk(FORWARD_SPEED,0,angle)
    lastAction = WALK_FORWARD


def WalkToLocation(x, y):
    global lastAction
    angle = hMath.getHeadingToFaceAt(Global.selfLoc, x, y)
    # if we are outside a reasonable angle, rotate on the spot first
    if math.fabs(angle) > 30:
        Action.walk(0,0,angle/2)
    else:
        Action.walk(FORWARD_SPEED,0,angle, Action.EllipticalWalk)
    lastAction = WALK_FORWARD


def SearchByHeadMoving(direction = RIGHT):
    #print "searching for the ball by moving the head.."
    global panDir, lastAction
    
    if lastAction != MOVE_HEAD:
        panDir = direction
        
    lastAction = MOVE_HEAD
    pan = tilt = crane = 0

    if Action.lastValues[Action.HeadType] == Action.HTAbs_h:
        pan = Action.lastValues[Action.Panx]

    if pan >= PAN_LIMIT:
        pan = PAN_LIMIT
        panDir = RIGHT
    elif pan < -PAN_LIMIT:
        pan = -PAN_LIMIT
        panDir = LEFT
        
    if panDir == LEFT:
        pan += HEAD_SCAN_SPEED
    else:
        pan -= HEAD_SCAN_SPEED

    if panDir != direction:
        tilt = crane = -30

    Action.setHeadParams(pan, tilt, crane, Action.HTAbs_h)


#NEW SPINNING FUNCTION - 19/10/04
def SearchBySpinning(direction = RIGHT):
    global lastAction
    for i in range(18): 
        Action.finalValues[i] = PARAM_SPIN_FAST[i]
    Action.finalValues[Action.TurnCCW] = direction*180
    Action.finalValues[Action.Panx]    = direction*PAN_LIMIT    
    lastAction = SPINNING
    

# debugging output
def debug(*toks):
    if DEBUG:
        for tok in toks:
            print tok,
        print
