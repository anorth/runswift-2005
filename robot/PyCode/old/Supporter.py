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
#  $Id: SupportZidane.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# Supporter.py
#
# Called by Forward.py to make decisions base on being the role of support.
#
#===============================================================================

"""
(Supporter)
"""

import math
import Global
import Constant
import FWHead
import HoverToBall

import HelpMath
import IndicatorAction

#===============================================================================
# Functions belong to this module.
#===============================================================================

supportOfTeammateRatherThanBall = False
supTeammateX                    = 0
supTeammateY                    = 0

#--------------------------------------
# Reset these variables every frame.
def resetEachFrame():

    Global.myRole = Constant.SUPPORT
    FWHead.minPosVariance  = 35
    FWHead.minHeadVariance = 25

#--------------------------------------
# In 2003 this function takes two arguments - the ballX and ballY. But this is nonsense and
# completly suck. Both ballX and ballY are global variables, why the 2003 team pass global
# variables to a method in the same namespace?
def DecideNextAction():
    resetEachFrame()

    # Back off if attacker is really close. Maybe drop the getHeading condition.
    for i in range(Constant.NUM_TEAM_MEMBER):
        if (Global.vTeammates[i].getConfidence() > 0 and Global.vTeammates[i].getDistance() < 60):

            IndicatorAction.showFacePattern(Constant.FP_BACKOFF_IMMEDIATE)
            Global.finalAction[Constant.AACmd:Constant.AAHeadType] = list (Constant.NormalWalkFast[Constant.AACmd:Constant.AAHeadType])
            Global.finalAction[Constant.AAForward] = -6
            Global.myRole = Constant.FACEBACKOFF
            return
                
    global supportOfTeammateRatherThanBall
           
    if supportOfTeammateRatherThanBall:
        supportOfTeammateRatherThanBall = False
        supX = supTeammateX
        supY = supTeammateY
    else:
        supX = Global.ballX
        supY = Global.ballY
                
    # debug = True
    # First draw a straight line from the ball to the goal center.
    # TODISCUSS: Is drawing the line to the best gap and integrate with DKD range a good idea?
    topEquation = Constant.TARGET_GOAL_Y - supY
    bottomEquation = Constant.TARGET_GOAL_X - supX
    if bottomEquation == 0:
        bottomEquation += 0.01
    m = topEquation / bottomEquation
    b = Constant.TARGET_GOAL_Y - m * Constant.TARGET_GOAL_X
    
    # Get the x value matching my global y value.
    XEDGE         = Constant.WALL_THICKNESS + 30
    YEDGE         = Constant.WALL_THICKNESS + 50
    if m == 0:
        m += 0.01
    xMatchingMyY  = (Global.selfLoc.getPos()[1] - b) / m
    leftLine      = None

    # Am I left or right of the support line?
    if Global.selfLoc.getPos()[0] <= xMatchingMyY:
        leftLine = True
    else:
        leftLine = False
        
    # Deal special cases first, the left side support.
    if supY > 0.25 * Constant.FIELD_LENGTH and supX <= \
        Constant.ON_EDGE_OFFSET:
        x, y = (10, min(Constant.FIELD_LENGTH - YEDGE, supY - 80))
        
    # The right side support.
    elif supY > 0.25 * Constant.FIELD_LENGTH and supX >= \
        Constant.FIELD_WIDTH - Constant.ON_EDGE_OFFSET:
        x = Constant.FIELD_WIDTH - 10
        y = min(Constant.FIELD_LENGTH - YEDGE, supY - 80)
    
    # Top horizontal strip.
    elif supY >= (0.75 * Constant.FIELD_LENGTH):
        if leftLine:
            x = max(XEDGE, supX - 60)
            y = min(Constant.FIELD_LENGTH - YEDGE, supY - 80)
        else:
            x = min(Constant.FIELD_WIDTH - XEDGE, supX + 60)
            y = min(Constant.FIELD_LENGTH - YEDGE, supY - 80)
    
    # Bottom horizontal strip.
    elif supY <= (0.25 * Constant.FIELD_LENGTH) and \
         supY >= Constant.WALL_THICKNESS + Constant.GOALBOX_DEPTH:
        if leftLine:
            x = max(XEDGE, supX - 80)
            y = max(YEDGE, supY + 80)
        else:
            x = min(Constant.FIELD_WIDTH - XEDGE, supX + 80)
            y = max(YEDGE, supY + 80)

    # Own goal near case.
    elif supY < Constant.WALL_THICKNESS + Constant.GOALBOX_DEPTH:
        if leftLine:
            x = max(XEDGE, supX - 80)
            y = max(YEDGE, supY + 80)
        else:
            x = min(Constant.FIELD_WIDTH - XEDGE, supX + 80)
            y = max(YEDGE, supY + 80)
    
    # The last case - middle horizontal strip.
    else:
        if leftLine:
            x = max(XEDGE, supX - 70)
            y = min(Constant.FIELD_LENGTH - YEDGE, supY + -50)
        else:
            x = min(Constant.FIELD_WIDTH - XEDGE, supX + 70)
            y = min(Constant.FIELD_LENGTH - YEDGE, supY + -50)

    h = HelpMath.normalizeAngle_0_360(HelpMath.RAD2DEG(math.atan2( \
                            supY - Global.selfLoc.getPos()[1], \
                            supX - Global.selfLoc.getPos()[0])))
                            
    HoverToBall.saGoToTargetFacingHeading(x, y, h)


#--------------------------------------
def ShouldIBecomeSupporter():
    # 2003 condition, become supporter if you are not a striker, not backoff off (although in the backoff
    # the decision tree may switch to supporter), if another robot is trying to become an attacker.
    #
    # Note: teamPlayers[i].hasBall() should be 1 (True) if any robot try to become an attacker.
    
    global supportOfTeammateRatherThanBall, supTeammateX, supTeammateY
    
    for i in range(Constant.NUM_TEAM_MEMBER):
        if (Global.teamPlayers[i].getCounter() > 0 
            and i != Global.myPlayerNum - 1
            and Global.teamPlayers[i].hasBall()):
            
            #----------------------------------------- should be taken care by backoff
            # if u can see the ball, another teammate has ball, 
            # u fall off the backoff/striker case, yr ballD is obvious larger than his, then support
            if (Global.ballSource == Constant.VISION_BALL and 
                Global.ballD - Global.teamPlayers[i].getBallDist() > 15):
                return True
                    
            #-----------------------------------------
            # if u cant see ball but have wireless ball (lostBall > 12), yr teammate has ball,
            # u fall off the backoff/striker case, then support
            if (Global.ballSource == Constant.WIRELESS_BALL):
                return True
            
            # ------------------------------------------
            # If lost ball completely, yr teammate has ball, support him rather than the ball
            if (Global.ballSource == Constant.GPS_BALL and
                  Global.lostBall > Constant.VBALL_ANYWHERE and
                  Global.teammatesLoc[i].getPosVar() < HelpMath.get95cf(50) and
                  HelpMath.getDistanceBetween (Global.selfLoc.getX(),Global.selfLoc.getY(), \
                                                Global.teammatesLoc[i].getX(),Global.teammatesLoc[i].getY()) < 20
               ):
                print "In Supporter line 179 shouldIbecomeSupport()"
                print "Support your teammate becoz I have lost the ball and he has the ball"
                supportOfTeammateRatherThanBall = True
                supTeammateX,supTeammateY       = Global.teammatesLoc[i].getPos()
                return True 
                 
    return False
