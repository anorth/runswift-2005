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


import math
import Global
import Action
import Constant
import hTrack
import Indicator
import VisionLink


#USAGE: (for external use)
#    call checkThenBlock() in appropriate place

isBallComingSoon = False

DEBUG_OUTPUT = False

BLOCK_SIDE_LENGTH = 30
HALF_BODY_WIDTH = 18 / 2.0
TOO_LATE_BLOCK_FRAME = 0.1 * 30
TOO_EARLY_BLOCK_FRAMES = 0.9 * 30

# Check whether the robot should block the ball, and does so if yes.
# If onlyVisualBall then a block will only trigger when the ball is
# seen this frame. If the ball is moving slower than minBallSpeed then
# this won't block. If the ball is nearer than minDist or farther than
# maxDist it won't block. If bothSides is true then it will always block
# both sides at once, else it may choose to block only one side.
# Returns true if it blocks.
def checkThenBlock(onlyVisualBall = True, minBallSpeed = 0.4, minDist = 30,
                    maxDist = 80, bothSides = False, dontBlockCentre = False):
    global isBallComingSoon
    isBallComingSoon = False

    if onlyVisualBall and not hTrack.canSeeBall():
        #debug("I can't see the ball!!")
        return False
    
    if Global.lostBall > Constant.LOST_BALL_LAST_VISUAL - 1: #if > 3
        #debug("I have lost the ball >_<~~")
        return False

    if Global.ballD > maxDist:
        #debug("ball is too far away")
        return False

    if Global.ballD < minDist:
        #debug("ball is too close")
        return False

    _, _, speed, dirDegrees, _, _ = VisionLink.getGPSBallVInfo(Constant.CTLocal) 
            
    if speed > 10:
        debug("Ball moves too fast, possible due to wrong ball")
        return False

    if speed < minBallSpeed:
        debug("Ball moves too slow, possible still ball with noise")
        return False
    
    dx = speed * math.cos(math.radians(dirDegrees))
    dy = speed * math.sin(math.radians(dirDegrees))

    if dy > 0:
        #debug("Ball is moving Away!")
        return False

    debug("Ball heading:", dirDegrees, "speed:", speed)
    
    h = Global.ballH
    d = Global.ballD
    bx = -d * math.sin(math.radians(h))
    by = d * math.cos(math.radians(h))

    if dontBlockCentre and d > 50 and abs(h) > 70:
        debug("The ball is in a large angle and far away")
        return False

    arrivalFrame = by / -dy
    debug("Ball is going to arrive in future frame:", arrivalFrame, "-------------")

    if arrivalFrame < 90: # 3 sec
        isBallComingSoon = True        

    if arrivalFrame < TOO_LATE_BLOCK_FRAME:
        debug("TOO LATE!")
        return False

    if arrivalFrame > TOO_EARLY_BLOCK_FRAMES:
        debug("TOO EARLY!")
        return False

    ix = dx * arrivalFrame + bx

    if ix > BLOCK_SIDE_LENGTH or ix < -BLOCK_SIDE_LENGTH:
        debug("cannot reach this far:", ix, "cm *******")
        return False    

    if -HALF_BODY_WIDTH <= ix <= HALF_BODY_WIDTH:
        if dontBlockCentre:
            debug("won't block b'coz the ball will hit me!!!!!!!!!!!!!!")
            return False
        else:
            block(Constant.MIDDLE)
    elif ix > HALF_BODY_WIDTH:
        if bothSides:
            block(Constant.MIDDLE)
        else:
            block(Constant.RIGHT)
    elif ix < -HALF_BODY_WIDTH:
        if bothSides:
            block(Constant.MIDDLE)
        else:
            block(Constant.LEFT)
    debug("!!!!!!!!!!!!!!!!!!!!!!!!!! BLOCKED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
    return True


def DecideNextAction():
    import sFindBall
    sFindBall.perform(headOnly=True)
    checkThenBlock(False)    

def block(blockDir):    
    if blockDir == Constant.LEFT:
        Action.kick(Action.BlockLeftWT)
    elif blockDir == Constant.RIGHT:
        Action.kick(Action.BlockRightWT)
    else: #MIDDLE
        Action.kick(Action.BlockWT)

    wt = Global.pWalkInfo.getCurrentWalkType()
    #TODO: check other walk as well,
    #      but currently we are using skelliptical walk atm
    if wt == Action.SkellipticalWalkWT: # if we are walking
        Action.forceStepComplete()
    #Indicator.showGreenEyes(blockDir)

def debug(*args):
    if DEBUG_OUTPUT:
        print __name__,
        for a in args:
            print a,
        print
