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
#  $Id: StrikerBecken.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# Striker.py
#
# Called by Forward.py to make decisions base on being the role of striker.
#
#===============================================================================

"""
Striker
"""
import math
import Global
import Constant
import Action
import hTrack as HelpTrack
import sHoverToBall as HoverToBall

import hMath as HelpMath
import hTeam as HelpTeam


STRIKER_ACC_SMALL_CIRCLE = 10
STRIKER_ACC_LARGE_CIRCLE = 20
NORMAL_BACKOFF_YDIST     = 140
MOST_FORWARD_YPOS        = Constant.FIELD_LENGTH / 4.0 * 3
MOST_BACK_YPOS           = Constant.FIELD_LENGTH / 4.0
SWITCH_POS               = Constant.FIELD_LENGTH / 3.0
strikerPointReqAccuracy  = STRIKER_ACC_SMALL_CIRCLE

strikerPtX               = None
strikerPtY               = None

STRIKER_LOST_BALL_LIMIT  = 90

#===============================================================================
# Functions belong to this module.
#===============================================================================




#--------------------------------------
# Called by DecideNextAction in Forward.py
def DecideNextAction():

    global STRIKER_ACC_SMALL_CIRCLE, STRIKER_ACC_LARGE_CIRCLE, strikerPointReqAccuracy
    global strikerPtX, strikerPtY

    Global.myRole = Constant.WINGER
    
    selfX, selfY  = Global.selfLoc.getPos()

    if (strikerPtX is None or strikerPtY is None):
        strikerPtX, strikerPtY = getStrikerPointXPositioning()
    
##~     print "ball    position  : myx: %.2f      myy: %.2f" % (Global.ballX,Global.ballY)
##~     print "striker point : strikeX: %.2f  strikeY: %.2f" % (strikerPtX,strikerPtY)
##~     print ""

    h = HelpMath.normalizeAngle_0_360(HelpMath.RAD2DEG \
        (math.atan2(Global.ballY - selfY, Global.ballX - selfX)))
##~     print "==========="
##~     print "heading is %.2f" % (h)
##~     print "myloc   %.2f, %.2f" % Global.selfLoc.getPos()
##~     print "ballLoc %.2f, %.2f" % (Global.ballX,Global.ballY)
    
    # If facing away the ball, then dont. Use the slower saGoToTargetFacingHeading
    
    angleBetweenStrikerPtBall = HelpMath.absAngleBetweenTwoPointsFromPivotPoint \
        (Global.ballX,Global.ballY,strikerPtX,strikerPtY,selfX,selfY)
    
    if ( angleBetweenStrikerPtBall > 90 ) :
        HelpTrack.saGoToTargetFacingHeading (strikerPtX,strikerPtY,h)
        
    # Otherwsie if you can see the ball constantly, use the fast way
    else:
        HoverToBall.saGoToTargetFacingHeading(strikerPtX, strikerPtY, h)

    # Hysterisis for whether or not your at the striker point.
    dist = math.sqrt(HelpMath.SQUARE(strikerPtX - selfX) \
                   + HelpMath.SQUARE(strikerPtY - selfY))

##~     print "StrikeX: %f" % strikerPtX
##~     print "StrikeY: %f" % strikerPtY
##~     print "Distance: %f" % dist
##~     print "StrikerPointReqAccuracy: %f" % strikerPointReqAccuracy

    if dist <= strikerPointReqAccuracy:
        strikerPointReqAccuracy = STRIKER_ACC_LARGE_CIRCLE
        Action.walk(0, 0, None, None)
    else:
        strikerPointReqAccuracy = STRIKER_ACC_SMALL_CIRCLE


#--------------------------------------
# Return boolean. 
def shouldIStart():

    global strikerPtX, strikerPtY
    
    debug                  = False   
    strikerPtX, strikerPtY = getStrikerPointXPositioning()

##~     if (Global.ballSource == Constant.VISION_BALL)
##~         return False

    # If you don't know where the ball is (cant see and no wireless source), don't assume striker role.
    # TOTHINK: Do we want to let the robot to become striker if it uses the wireless ball?
    if (Global.ballSource == Constant.GPS_BALL 
        and Global.lostBall > Constant.VBALL_LOST
        and Global.sharedBallVar > HelpMath.get95cf (20) ):
        if debug:
            print "Striker I don't know where the ball is, I won't be striker!\n"
        return False

    # If ball is very close to target goal, assume striker role.
    # If you don't have ball, another robot has ball, and you are closest to the
    # striker point.
    # ie. If the ball is close to the target goal, try to ensure that there will be a striker.
    #      Not implementated at the moment, since this is really a hack. Put it back when needed.
    # ..
    # .. Not ported to python now. 
    # WHAT THE HELL U ARE TALKING ABOUT TED. U ARE LAZY
    
##~     ballDistToTargetGoal = HelpMath.getDistanceBetween(bx,by,Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y)
##~     
##~     if ballDistToTargetGoal < 65:
##~         if (not HelpTrack.doIHaveTheBall() and HelpLong.teammateHasBall() and 
##~             amIClosestToStrikerPoint(strikerPtX,strikerPtY)):
##~             
##~                 for i in range (Constant.NUM_TEAM_MEMBER):
##~                     if (Global.teamPlayers[i].getCounter() <=0 or
##~                         Global.teamPlayers[i].isGoalie() != 0 or
##~                         i == Global.MyPlayerNum - 1):
##~                         continue
##~                     if (Global.teamPlayers[i].isStriker() and
##~                         Global.MyPlayerNum < i + 1):
##~                         return False
##~                 
##~                 if debug:
##~                     print "Striker TRUE, ball close to goal but no other striker"
##~                 return True

    # If you only know of 1 other forward don't become striker (its better to become supporter)
    if (HelpTeam.forwardAlive() < 3 and Global.framesSince3ForwardsPresent >= 3):
        if debug:
            print "Striker FALSE, can't find enough forwards!\n"
        return False

    # Find the teammate that is furthest from ball
    teammateFurthestFromBallPlayer   = -1
    teammateFurthestFromBallDistance = -1

    # Linear search all the teammates.
    for i in range(Constant.NUM_TEAM_MEMBER):
        # Ignore goalie, yourself, and invalid teammate.
        if (Global.teamPlayers[i].getCounter() <= 0 or
            Global.teamPlayers[i].isGoalie() != 0 or
            i == Global.myPlayerNum - 1):
            continue

        # If they are alrady doing the striker role and they have a high player number than you, don't do.
        # Player number thing is just to break symmetery.
        if Global.teamPlayers[i].isStriker() and Global.myPlayerNum < i + 1:
            if debug:
                print "Striker Someone higher is already a striker!"
            return False

        teammateDist = -1

        # If they can see the ball themselves.
        if Global.teamPlayers[i].getBallDist() < Constant.LOSTBALL_DIST:
            teammateDist = Global.teamPlayers[i].getBallDist()
        else:
            dx = Global.ballX - Global.teammatesLoc[i].getPos()[0]
            dy = Global.ballY - Global.teammatesLoc[i].getPos()[1]
            teammateDist = math.sqrt((dx * dx) + (dy * dy))

        if teammateDist > teammateFurthestFromBallDistance:
            teammateFurthestFromBallPlayer   = i
            teammateFurthestFromBallDistance = teammateDist

    if debug:
        print "Teammate distance furthest from the ball: %f" % teammateFurthestFromBallDistance

    # If you're the only forward alive, don't become striker.
    if teammateFurthestFromBallPlayer == -1:
        if debug:
            print "False I am the only one alive!"
        return False

    # If you are obviously furthest away from the ball, become wing striker.
    if Global.ballD - teammateFurthestFromBallDistance > Constant.FURTHEST_OFFSET:
        if debug:
            print "True I am obviosuly furthest from the ball!\n"
        return True

    # If you are obviosuly not furthest away from the ball, don't become wing striker.
    if teammateFurthestFromBallDistance - Global.ballD > Constant.FURTHEST_OFFSET:
        if debug:
            print "False I am obviosuly not furthest away from the ball!\n"
        return False

    # At this point you are not obviosuly furthest away from the ball.

    # Obtain own distance to striker point.
    selfX, selfY        = Global.selfLoc.getPos()
    ownDistToStrikerPt  = HelpMath.getDistanceBetween(selfX,selfY,strikerPtX,strikerPtY)

    if debug:
        print "My distance to strike point: %f" % ownDistToStrikerPt

    # Obtain teammate distance to striker point.
    # Return global position of the teammate.
    a = strikerPtX - Global.teammatesLoc[teammateFurthestFromBallPlayer].getX()
    b = strikerPtY - Global.teammatesLoc[teammateFurthestFromBallPlayer].getY()
    teammateDistToStrikerPt = math.sqrt((a * a) + (b * b))

    # If closer to the striker point, become striker.
    if ownDistToStrikerPt < teammateDistToStrikerPt:
        if debug:
            print "I am closer than my teammates for the striker point!"
        return True
    else:
        if debug:
            print "I am not closer than my teammates for the striker point!"
        return False

def shouldIEnd():
    return True

#--------------------------------------
# Return a list with two elements. Implementing X-Positioning startegy.
def getStrikerPointXPositioning():
    global NORMAL_BACKOFF_YDIST, MOST_BACK_YPOS, MOST_FORWARD_YPOS, SWITCH_POS
    
    lowX  = -1
    lowY  = -1
    highX = -1
    highY = -1
    
    bx = Global.ballX
    by = Global.ballY
    
    # Stay on the opposite sidde from the ball.
    # Draw two lines, one from the defensive point to the center,
    # and the other from the center to the attack point.
    # You will be somewhere on those lines.
    # In the defensive half, you will be normalBackoffYDist behind the ball in the y dir
    # in the offensive half, you will be further forward so that you are at the
    # forward position when the ball hits the far end of the field.
    myY = by - NORMAL_BACKOFF_YDIST
    
    # Clip the y values.
    if myY < MOST_BACK_YPOS:
        myY = MOST_BACK_YPOS
    elif myY > MOST_FORWARD_YPOS:
        myY = MOST_FORWARD_YPOS

    # Scale myY linearly so that we reach the endpoints.
    if myY > SWITCH_POS:
        topOfOldTravel = Constant.TARGET_GOAL_Y - Constant.WALL_THICKNESS - Constant.GOALBOX_DEPTH / 2.0 - NORMAL_BACKOFF_YDIST
        oldYTravel     = topOfOldTravel - SWITCH_POS
        newYTravel     = MOST_FORWARD_YPOS - SWITCH_POS
        alpha          = newYTravel / oldYTravel
        myY            = (myY - SWITCH_POS) * alpha + SWITCH_POS
        
    if myY < SWITCH_POS:
        if bx < Constant.FIELD_WIDTH / 2.0:
            lowX = Constant.FIELD_WIDTH / 2.0 + 80
        else:
            lowX = Constant.FIELD_WIDTH / 2.0 - 80
            
        lowY = MOST_BACK_YPOS
        highX = Constant.FIELD_WIDTH / 2.0
        highY = SWITCH_POS
        
    else:
        if bx < Constant.FIELD_WIDTH / 2.0:
            highX = Constant.MAX_GOALBOX_EDGE_X + 10
        else:
            highX = Constant.MIN_GOALBOX_EDGE_X - 10
             
        highY = MOST_FORWARD_YPOS
        lowX = Constant.FIELD_WIDTH / 2.0
        lowY = SWITCH_POS
        
    strikeY = myY
    upperEquation = myY - lowY
    lowerEquation = highY - lowY
    if lowerEquation == 0:
        lowerEquation += 0.01
    alpha = upperEquation / lowerEquation
    strikeX = alpha * (highX - lowX) + lowX
     
    return [strikeX, strikeY]

