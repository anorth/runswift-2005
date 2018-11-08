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
import sGrabTurnKick as GrabTurnKick
import HelpMath
import HeadAction
import MyTrack
import HelpTrack
import WalkAction
import Kick


import rAttacker
import Forward
import FWHead

KICK_DIST = 15

KICK_MIN = 110
KICK_MAX = 130

lockMode = False

def DecideNextAction():
    global lockMode

    Global.myRole = Constant.ATTACKER
    
    if GrabTurnKick.lockMode:
        GrabTurnKick.DecideNextAction()
        lockMode = True
        return

    lockMode = False

    if Global.ballY < Constant.FIELD_LENGTH/2:
        Forward.resetEachFrame()
        rAttacker.DecideNextAction()
        FWHead.DecideNextAction()        
        return

    me_x = Global.selfLoc.getX()
    me_y = Global.selfLoc.getY()
    #ballToGoalDist = HelpMath.getDistanceBetween( \
    #    Global.ballX, Global.ballY, \
    #    Constant.TARGET_GOAL_X, Constant.TARGET_GOAL_Y) 
    
    angleMeBallGoal = \
                    HelpMath.angleBetweenTwoPointsFromPivotPoint( \
            me_x, me_y, \
            Constant.TARGET_GOAL_X, Constant.TARGET_GOAL_Y, \
            Global.ballX, Global.ballY )

    distToGoal = HelpMath.getDistanceBetween(me_x, me_y,
                                             Constant.TARGET_GOAL_X,
                                             Constant.TARGET_GOAL_Y)


    MyTrack.DecideNextAction() # for head controlling

    if canDoKick() and distToGoal < 100:
        if isInRange(angleMeBallGoal, -KICK_MAX, -KICK_MIN):
            Kick.UPright()
        elif isInRange(angleMeBallGoal, KICK_MIN, KICK_MAX):
            Kick.UPleft()
    elif HelpTrack.inTargetGoalBox(Global.ballX, Global.ballY):
        # if ball in the goal box
        goalBoxKick(angleMeBallGoal)
    else:
        GrabTurnKick.DecideNextAction()




def goalBoxKick(angleMeBallGoal):
    if not canDoKick():
        MyTrack.DecideNextAction()
    else:
        FACE_GOAL = 160
        if isInRange(angleMeBallGoal, -KICK_MIN, 0):
            rotateAboutBall(MyTrack.LEFT)
        elif isInRange(angleMeBallGoal, -KICK_MAX, -KICK_MIN):
            Kick.UPright()
        elif isInRange(angleMeBallGoal, -FACE_GOAL, -KICK_MAX):            
            rotateAboutBall(MyTrack.RIGHT)            
            
        elif isInRange(angleMeBallGoal, 0, KICK_MIN):
            rotateAboutBall(MyTrack.RIGHT)
        elif isInRange(angleMeBallGoal, KICK_MIN, KICK_MAX):
            Kick.UPleft()
        elif isInRange(angleMeBallGoal, KICK_MAX, FACE_GOAL):
            rotateAboutBall(MyTrack.LEFT)
        
        else: # if  < -FACE_GOAL or > FACE_GOAL
            WalkAction.setNormalWalk(5, 0, 0)
            

def isInRange(a, min, max):
    return min < a <= max
    

def rotateAboutBall(dir):
    WalkAction.setNormalWalk(0, dir*(-4.5), dir*10)

def canDoKick():
    # Ball must be in front and close
    # can = 0 < MyTrack.BallPosY < KICK_DIST

    can = 0 < Global.ballD < 18
    # and really there
    can = can and MyTrack.canSeeBall()    
    can = can and abs(Global.lastPan) < 15
    return can



def getLocalHeadingToBestGap():
    HeadAction.setHeadParams(90, 0, 0, Constant.HTAbs_h)
    #minValue, maxValue = VisionLink.getHeadingToBestGap()
    #print "min:", minValue, "max:", maxValue
    # min-max is a range where you can shoot
    # +ve is left, -ve is right   26 is the left most. -27 is right most if looking front












