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
#  $Id: sPawKick.py 4572 2005-02-14 19:28:59Z weiming $
# 
#  Copyright (c) 2005 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2005 (c) 
#
#   Non-directional paw kick.
#===============================================================================

import Action
import Constant
import Global
import hMath
import hWhere
import Indicator
import math
import sFindBall


#=========================
# Constants for paw kick module.
#=========================

LEFT_PAW_KICK = 0
RIGHT_PAW_KICK = 1
AUTO_PAW_KICK = 2
pawKickType = LEFT_PAW_KICK


def DecideNextAction(): 
    Indicator.setDefault()
    
    if Global.lostBall < Constant.LOST_BALL_GPS\
        and perform() == Constant.STATE_EXECUTING:
        Indicator.showRedEye() 
    
    else: 
        sFindBall.perform()
            

def resetPerform(): 
    pass

#=======================================================================    
# This function returns a list with two elements. The first element is True or
# False, representing whether it is ok for the robot to do the paw kick. The
# second element is which paw the robot should kick. Notice that the second
# element only makes sense if the first element is True.  In another words,
# don't process the second element if the first element is False.

# Paw kick should be used on edges.
# Paw kick shouldn't be used when the ball is very close and specially it is
# under the chin (isUNSW2004BallUnderChin() in 2003) even the ball and the
# robot are both near the edge.

def isPawKickAtEdgeOk():
    debug = False
    
    # Don't do paw kick if you can't physically see the ball.
    if Global.ballSource != Constant.VISION_BALL:
        return [False, AUTO_PAW_KICK]

    # Ted: Position variance in the following cases have been disabled. Come
    # back later.

    # Paw kick left edge case.
    # 1.) If ball is near the left edge.
    # 2.) If the robot is near the left edge.
    # 3.) If the robot heading is toward the target goal.
    # 4.) If the ball heading relative to the robot is not much (fewer turning).
    if (hWhere.ballOnLEdge() 
            and hWhere.isOnLEdge() 
            and hWhere.isInRange(30,90) # was (60, 120) --weiming
            and ( (abs(Global.ballH) < 30 and Global.ballD > 30) or 
                  (abs(Global.ballH) < 40 and Global.ballD <= 30) )
        ):
        if debug:
            print "Hey, I am doing the left edge paw kick now!\n"    
        return [True, LEFT_PAW_KICK]

    # Paw kick right edge case.
    # 1.) If ball is near the right edge.
    # 2.) If the robot is near the right edge.
    # 3.) If the robot heading is toward the target goal.
    # 4.) If the ball heading relative to the robot is not much (fewer turning).
    if (hWhere.ballOnREdge() 
            and hWhere.isOnREdge() 
            and hWhere.isInRange(90, 150) # was (60,120) --weiming
            and ( (abs(Global.ballH) < 30 and Global.ballD > 30) or 
                  (abs(Global.ballH) < 40 and Global.ballD <= 30) )
        ):
        if debug:
            print "Hey, I am doing the right edge paw kick now!\n"
        return [True, RIGHT_PAW_KICK]
    
    # Paw kick top edge right half.
    # 1.) If ball is on the top right edge.
    # 2.) If the robot is on the top right edge.
    # 3.) If the robot heading is toward the target goal.
    # 4.) If the ball heading relative to the robot is not much (fewer turning).
    # 5.) If the robot is right of the goal box.
    # 6.) If the ball is right of the goal box.    
    # 7.) If the ball heading is not large.
    if (hWhere.ballOnTREdge()
        and hWhere.isOnTREdge() 
        and hWhere.isInRange(180, 220)     # was (150,190) --weiming
        and ( (abs(Global.ballH) < 30 and Global.ballD > 30) or 
              (abs(Global.ballH) < 40 and Global.ballD <= 30) )
        ):
        if debug:
            print "Paw kick on the top edge right half!\n"
        return [True, AUTO_PAW_KICK]

    # Paw kick top edge left half.
    if (hWhere.ballOnTLEdge() 
            and hWhere.isOnTLEdge()
            and not hWhere.isInRange(0, 320) # was (30, 350) --weiming
            and ( (abs(Global.ballH) < 30 and Global.ballD > 30) or 
                  (abs(Global.ballH) < 40 and Global.ballD <= 30) )
        ):
        if debug:
            print "Hey, I am doing the top edge left paw kick now!\n"
        return [True, AUTO_PAW_KICK]
                
    # Paw kick bottom left half
    if (hWhere.ballOnBEdge() 
            and hWhere.isOnBEdge()
            and hWhere.isInRange(50, 90)     # was (150,190) -- weiming
            and Global.selfLoc.getPos()[0] < Constant.FIELD_WIDTH / 2
            and Global.ballX < Constant.MIN_GOALBOX_EDGE_X
            and ( (abs(Global.ballH) < 30 and Global.ballD > 30) or 
                  (abs(Global.ballH) < 40 and Global.ballD <= 30) )
        ):
        if debug:
            print "Hey, I am doing the bottom right edge paw kick now!\n"
        return [True, AUTO_PAW_KICK]
        
    # Paw kick bottom right half.
    if (hWhere.ballOnBEdge()
            and hWhere.isOnBEdge()
            and not hWhere.isInRange(90, 130) #was (30,350) --weiming
            and Global.selfLoc.getPos()[0] > Constant.FIELD_LENGTH / 2
            and Global.ballX > Constant.MAX_GOALBOX_EDGE_X
            and ( (abs(Global.ballH) < 30 and Global.ballD > 30) or 
                  (abs(Global.ballH) < 40 and Global.ballD <= 30) )
        ):
        if debug:
            print "Hey, I am doing the bottom right edge paw kick now!\n"
        return [True, AUTO_PAW_KICK]
        
    return [False, AUTO_PAW_KICK]


      
    

#=======================================================================    
def perform(paw=AUTO_PAW_KICK):

    global pawKickType

    if Global.lostBall > Constant.LOST_BALL_GPS: 
        return Constant.STATE_FAILED
        
    # Head ball tracking movement    
    sFindBall.perform(True)
    
    PAW_KICK_Y_OFFSET = Constant.BallRadius
    PAW_KICK_X_OFFSET = 7.0

    ballX = math.sin(hMath.DEG2RAD(Global.ballH)) * Global.ballD
    ballY = math.cos(hMath.DEG2RAD(Global.ballH)) * Global.ballD
    
    # The 2003 offset is 6 and it is not too bad.    
    if paw == AUTO_PAW_KICK:
        if pawKickType == LEFT_PAW_KICK and ballX < -PAW_KICK_X_OFFSET / 2:
            pawKickType = RIGHT_PAW_KICK
        elif pawKickType == RIGHT_PAW_KICK and ballX > PAW_KICK_X_OFFSET / 2:
            pawKickType = LEFT_PAW_KICK
    else:
        pawKickType = paw

    if pawKickType == RIGHT_PAW_KICK:
        targetX = ballX + PAW_KICK_X_OFFSET
    else:
        targetX = ballX - PAW_KICK_X_OFFSET

    targetY = ballY - PAW_KICK_Y_OFFSET
    targetD = math.sqrt(hMath.SQUARE(targetX) + hMath.SQUARE(targetY))
    targetH = hMath.RAD2DEG(math.asin(targetX / targetD))
    

    # Once the ball is lined up and well within range for the forward, the
    # robot should commit to charge at the ball at full speed.        
    if abs(targetX) <= 1.0 and targetD < Constant.BallRadius * 1.5:
        turnccw = hMath.CLIP(targetH,10)   
        Action.walk(Action.MAX_FORWARD,0,turnccw,minorWalkType=Action.SkeFastForwardMWT) 
    
    else:
        sFindBall.walkToBall(targetD,targetH,getBehind=sFindBall.GET_BEHIND_LOTS)         

    return Constant.STATE_EXECUTING
