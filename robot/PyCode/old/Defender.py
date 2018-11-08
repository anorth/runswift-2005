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
#  $Id: DefenderHomer.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#===============================================================================

"""
The Bird of Prey... or is that bird of pray?
"""

import math
import Debug
import Global
import Constant
import HoverToBall

import HelpTeam
import WalkAction
import HelpMath

INI_DESIRED_BALL_ANGLE = 45
currentBopMode         = Constant.NORMAL_BOP
lastDBH                = None
insideBound            = 36 * 0.5
outsideBound           = insideBound + 20
maxTurn                = 30
cornerCut              = 10
boxPointOffset         = 15
breakOutMinAngle       = 10
breakOutIncAngle       = 20
minAngle               = 90




# Defender lockmode.
lockDefender = False

###############################################################
# Defender.
###############################################################

# Return True if the robot wants to start defending.
def shouldIStartDefend():
    global lockDefender, currentBopMode, minAngle
        
    if lockDefender:
        return False
 
    if Debug.defenderTriggerDebug:
        print "Start ---------------------"
 
    forwardNum  = HelpTeam.forwardAlive()
##~     defenderNum = numDefenders()
 
    # If you are the only forward alive, don't be the defender. Instead running straight to the
    # ball is a better idea. Notice that more than one or two defenders may be allow.
    if forwardNum == 1:
        if Debug.defenderTriggerDebug:
            print "No defender ever if only single forward alive!"
        return False
        
    # If there are at least two defenders already, don't become the defender. Notice that this
    # function is called shouldIStartDefend() and the code only execute to this point if
    # lockDefender is False. If lockDefender is False, then the robot is not the defender.
    # So numDefenders() below really determine how many defenders at the moment with
    # or without considering yourself.
    #if defenderNum >= 2:
    #    if Debug.defenderTriggerDebug:
    #        print "At least two defenders already, no more defender!"
    #    return False
        
    # Now at this point we are sure that there are at least two forwards alive and at most
    # there is one more defender not including yourself on the field.
    
    # If there are two forwards only (maybe one other forward crash or penalised), then the robot
    # closest to the ball in terms of x distance should not be the defender.
    if forwardNum == 2 and areClosestX():
        if Debug.defenderTriggerDebug:
            print "Two forwards alive but I am the closest!"
        return False
    
    if forwardNum == 3 and areClosestX():
        if Debug.defenderTriggerDebug:
            print "Three forwards alive but I am the closest!"
        return False
        
    # Assume goalie will do it.
    if ballInOwnBox(5):
        if Debug.defenderTriggerDebug:
            print "Start: Ball in own box!"
        return False

    # Reset the state.    
    currentBopMode = Constant.NORMAL_BOP
    minAngle = 90
    
    DEFENCE_OFFSET = 10.0
    DEFENCE_ANGLE  = 150.0

    problem = isDefenceProblem(DEFENCE_OFFSET, DEFENCE_ANGLE, False, True)

    # If there is defense cover problem, then start becoming the defender.
    if problem:
        lockDefender = True
        return True
    else:
        if Debug.defenderTriggerDebug:
            print "Start: No defender this time!"
        return False

# Return True if the robot doesn't want to become the defender anymore.
def shouldIEndDefend():
    global lockDefender, currentBopMode
        
    if Debug.defenderTriggerDebug:
        print "End ---------------------"
     
    # If in place, don't become the bird anymore.
    if currentBopMode == Constant.CIRCLE_BOP and \
       abs(Global.ballH) < 10:
        currentBopMode = Constant.NORMAL_BOP
        if Debug.defenderDebug:
            print "Quit due to now in place and ball distance is small!"
        lockDefender = False
        return True

    # Assume goalie will do it.
    if ballInOwnBox(0):
        if Debug.defenderTriggerDebug:
            print "End: Ball in own box!"
        lockDefender = False
        return True
    
    DEFENCE_OFFSET = 0
    DEFENCE_ANGLE  = 90.0

    problem = isDefenceProblem(DEFENCE_OFFSET, DEFENCE_ANGLE, True, False)

    # If there is defense cover problem, then don't stop becoming defender.
    if problem:
        return False
        
    # No defense cover problem.
    else:
        if Debug.defenderTriggerDebug:
            print "End: Stop becoming defender!"
        lockDefender = False
        return True

# Return True if any defensive problem.
def isDefenceProblem(offset, angle, mustHaveBall, includeYou):    
    # Draw the line.
    ang  = 90 - angle / 2.0
    m    = math.tan(HelpMath.DEG2RAD(ang))
    bpos = (Global.ballY + offset) - m * Global.ballX
    bneg = (Global.ballY + offset) + m * Global.ballX
    
    if Global.lostBall > 30:
        if Debug.defenderTriggerDebug:
            print "Lost too many frames - one second!"
        return False
        
    #if Global.ballY > (2.0 / 3.0) * Constant.FIELD_LENGTH:
    #    if Debug.defenderTriggerDebug:
    #        print "Ball far away and close to the field lenth!"
    #    return False
        
    # Check yourself first. If you are far enough back then no problem (hysterisis).
    if ((includeYou and Global.selfLoc.getPos()[1] < 40) or \
       Global.selfLoc.getPos()[1] < 20):
        if Debug.defenderTriggerDebug:
            print "No problem because enough back!"
        return False
    
    # If I am behind the threshold line, no need to become defender.
    if (includeYou and (Global.selfLoc.getPos()[1] < m * Global.selfLoc.getPos()[0] + bpos) \
        and (Global.selfLoc.getPos()[1] < -m * Global.selfLoc.getPos()[0] + bneg)):
        if Debug.defenderTriggerDebug:
            print "False because of the line!"
        return False
       
    for i in range(Constant.NUM_TEAM_MEMBER):
        if Global.teamPlayers[i].isGoalie():
            continue
        
        # Notice that teamPlayers[selfIndex].getCounter() is equal to 0.
        # If the teammate is behind the threshold line.
        if Global.teamPlayers[i].getCounter() > 0 and \
            Global.teammatesLoc[i].getPos()[1] < m * Global.teammatesLoc[i].getPos()[0] + bpos and \
            Global.teammatesLoc[i].getPos()[1] < -m * Global.teammatesLoc[i].getPos()[0] + bneg:
            
            # If teammates have ball, then better do other stuffs instead of moving with curve
            # locus around the ball.
            if ((not mustHaveBall) or Global.teamPlayers[i].hasBall()):
                if Debug.defenderTriggerDebug:
                    print "Teammate has ball, no defender!"
                return False
        
    if Debug.defenderTriggerDebug:
        print "I want to become the defender!"
        # Ok, print all the info why you want to become the defender.
        if includeYou:
            print "------"
            print Global.teamPlayers[0].getCounter()
            print Global.teammatesLoc[0].getPos()
            print Global.teamPlayers[1].getCounter()
            print Global.teammatesLoc[1].getPos()
            print Global.teamPlayers[2].getCounter()
            print Global.teammatesLoc[2].getPos()
            print Global.teamPlayers[3].getCounter()
            print Global.teammatesLoc[3].getPos()
            print Global.selfLoc.getPos()
            print Global.ballSource
            print Global.ballX
            print Global.ballY
            print ang
            print m
            print bpos
            print bneg
            print numDefenders()
    
    return True

def DecideNextAction():
    global boxPointOffset, cornerCut, maxTurn, insideBound, outsideBound, lastDBH, \
           currentBopMode, INI_DESIRED_BALL_ANGLE, lockDefender, minAngle, breakOutMinAngle, breakOutIncAngle

    # Checking preconditions.

    desiredBallAngle = INI_DESIRED_BALL_ANGLE
    
    # If in place, don't become the bird anymore.
    if currentBopMode == Constant.CIRCLE_BOP and \
       abs(Global.ballH) < 10:
        currentBopMode = Constant.NORMAL_BOP
        if Debug.defenderDebug:
            print "Quit due to now in place and ball distance is small!"
        lockDefender = False
        WalkAction.setEllipticalWalk(7, 0, HelpMath.CLIP(Global.ballH / 2, 8))
        return
        
    goalx = (Constant.FIELD_WIDTH / 2 + Global.ballX) / 2
    dX    = Global.ballX - goalx
    dY    = Global.ballY
    
    # Determine if we are around the right side of the ball.
    usToGoal = math.sqrt(HelpMath.SQUARE(Global.selfLoc.getPos()[0] - goalx) + \
               HelpMath.SQUARE(Global.selfLoc.getPos()[1]))
    ballToGoal = math.sqrt(HelpMath.SQUARE(dX) + HelpMath.SQUARE(dY))

    if Debug.defenderDebug:
        print "Python usToGoal: %f" %usToGoal
        print "Python ballToGoal: %f" %ballToGoal
        
    if abs(dX) < 0.1:
        if Debug.defenderDebug:
            print "pa"
        if Global.selfLoc.getPos()[0] < goalx:
            if Debug.defenderDebug:
                print "pb"
            desiredBH = desiredBallAngle
        else:
            if Debug.defenderDebug:
                print "pc"
            desiredBH = -desiredBallAngle
            
    else:
        if Debug.defenderDebug:
            print "pd"
        m = dY / dX
        b = Global.ballY - m * Global.ballX
        if abs(m) < 0.01:
            desiredBH = 0
        elif dY < 0:
            # This is an error that appeared .. ball must be close to back line.
            if Global.ballX > Constant.FIELD_WIDTH / 2:
                desiredBH = desiredBallAngle
            else:
                desiredBH = -desiredBallAngle
        elif lastDBH > 0:
            if Global.selfLoc.getPos()[0] < ((Global.selfLoc.getPos()[1] - b) / m) + 5:
                desiredBH = desiredBallAngle
            else:
                desiredBH = -desiredBallAngle
        else:
            if Global.selfLoc.getPos()[0] < ((Global.selfLoc.getPos()[1] - b) / m) - 5:
                desiredBH = desiredBallAngle
            else:
                desiredBH = -desiredBallAngle
        
    if Debug.defenderDebug:
        print "Python desiredBH: %f" %desiredBH 
        
    if usToGoal < ballToGoal:
        if Debug.defenderDebug:
            print "usToGoal < ballToGoal!"
        dp = (Global.selfLoc.getPos()[0] - goalx) * dX + Global.selfLoc.getPos()[1] * dY
        if abs(usToGoal) < 0.1 or abs(ballToGoal) < 0.1:
            lockDefender = False
            if Debug.defenderDebug:
                print "Quit due to something being to close to goal (should never happen)"
            return
        
        # use dot product to calculate angle between
        # robot vector and ball vector (both from defend pt)
        cosAng = dp / (usToGoal * ballToGoal)
        ang    = math.acos(cosAng)
        dist   = usToGoal * math.sin(ang)
        
        if math.degrees(ang) < minAngle:
            minAngle = math.degrees(ang)
        
        if minAngle < breakOutMinAngle:
            lockDefender = False
            if Debug.defenderDebug:
                print "Quit due to crossing the line from ball to goal"
            return
        
        if (ang - minAngle) > breakOutIncAngle:
            lockDefender = False
            if Debug.defenderDebug:
                print "Quit due to angle of line from ball to goal increasing"
            return
        
        if Global.ballD < insideBound * 2 or dist < insideBound or \
            ((currentBopMode == Constant.CIRCLE_BOP) and dist < outsideBound):
            
            if currentBopMode == Constant.CIRCLE_BOP:
                if lastDBH > 0:
                    turnCCW = maxTurn
                else:
                    turnCCW = -maxTurn
            else:
                if desiredBH > 0:
                    turnCCW = maxTurn
                else:
                    turnCCW = -maxTurn
            
            if Debug.defenderDebug:
                print "@@@@@@@@@@@@@@"
                    
            currentBopMode = Constant.CIRCLE_BOP
            WalkAction.setEllipticalWalk(7, 0, turnCCW)
            return
            
        else:
            if currentBopMode == Constant.CIRCLE_BOP:
                currentBopMode = Constant.NORMAL_BOP
                lockDefender = False
                if Debug.defenderDebug:
                    "Quit due to no longer requiring circle!"
                    
    else:
        if Debug.defenderDebug:
            print "Change from circle to normal!"
        if currentBopMode == Constant.CIRCLE_BOP:
            currentBopMode = Constant.NORMAL_BOP
            lockDefender = False
            if Debug.defenderDebug:
                print "Quit due to no longer requiring circle here!"
    
    if Debug.defenderDebug:
        print "Before lastDBH = desiredBH!"
        print "HeHe: currentBopMode: %f" %currentBopMode
    continue_(desiredBH) # function too long for python - so split it in two
    
def continue_(desiredBH):
    global lastDBH, currentBopMode, cornerCut
    
    lastDBH = desiredBH
    box     = (Global.selfLoc.getPos()[1] < ((Constant.GOALBOX_DEPTH + Constant.WALL_THICKNESS) + 25)) or \
               (((currentBopMode == Constant.AVOIDBOX_BOP) or (currentBopMode == Constant.POSTAVOID_BOP)) and \
               (Global.selfLoc.getPos()[1] < ((Constant.GOALBOX_DEPTH + Constant.WALL_THICKNESS) + 50)))

    if box:
        Global.myRole = Constant.DEFENDERGOALBOX
               
    # Global coordinative of where exactly I am heading at.
    desiredHeading = HelpMath.normalizeAngle_0_360(Global.selfLoc.getHeading() + \
                     (Global.ballH - desiredBH))
                     
    # Turning the left direction.
    if (box and (desiredBH > 0) and (Global.selfLoc.getPos()[0] < ((Constant.FIELD_WIDTH + \
        Constant.GOALBOX_WIDTH) / 2.0) - cornerCut) and (desiredHeading > 180) and \
        (desiredHeading < (350))):
        
        currentBopMode = Constant.AVOIDBOX_BOP
        px = ((Constant.FIELD_WIDTH + Constant.GOALBOX_WIDTH) / 2.0) + boxPointOffset
        py = (Constant.GOALBOX_DEPTH + Constant.WALL_THICKNESS) + boxPointOffset
        phead = HelpMath.normalizeAngle_180(HelpMath.RAD2DEG(math.atan2(py - Global.selfLoc.getPos()[1], \
                                            px - Global.selfLoc.getPos()[0])) - Global.selfLoc.getHeading())

        WalkAction.setEllipticalWalk(7, 0, HelpMath.CLIP(phead / 2.0, maxTurn))
        return
        
    elif (box and (desiredBH < 0) and (Global.selfLoc.getPos()[0] > ((Constant.FIELD_WIDTH - \
        Constant.GOALBOX_WIDTH) / 2.0) + cornerCut) and (desiredHeading > 190)):
        
        currentBopMode = Constant.AVOIDBOX_BOP
        px = ((Constant.FIELD_WIDTH - Constant.GOALBOX_WIDTH) / 2.0) - boxPointOffset
        py = (Constant.GOALBOX_DEPTH + Constant.WALL_THICKNESS) + boxPointOffset

        phead = HelpMath.normalizeAngle_180(HelpMath.RAD2DEG(math.atan2(py - \
                Global.selfLoc.getPos()[1], px - Global.selfLoc.getPos()[0])) - \
                Global.selfLoc.getHeading())

        WalkAction.setEllipticalWalk(7, 0, HelpMath.CLIP(phead / 2.0, maxTurn))
        return
        
    elif (box and ((currentBopMode == Constant.AVOIDBOX_BOP) or \
        (currentBopMode == Constant.POSTAVOID_BOP))):
        currentBopMode = Constant.POSTAVOID_BOP
        relh = Global.ballH - desiredBH
        if abs(relh) < 10:
            currentBopMode = Constant.NORMAL_BOP
        else:
            WalkAction.setEllipticalWalk(7, 0, HelpMath.CLIP(relh / 2.0, maxTurn))
            if Debug.defenderDebug:
                print  "!!!!!"
            return
    else:
        currentBopMode = Constant.NORMAL_BOP

    if Debug.defenderDebug:
        print "Before hoverToBall - currentBopMode: %f" %currentBopMode
        
    # Hover To Ball
    relH    = Global.ballH - desiredBH
##~     turnCCW = HelpMath.CLIP(relH / 2.0, 40.0)

    if Debug.defenderDebug:
        print "relH: %f" %relH
  
    Global.ballH = relH
    HoverToBall.DecideNextAction()
        
##~ # This function should let the robot avoid getting into the goalbox or enable the robot to walk around the goal box edge.
##~ def avoidGoalBox(direction):
##~     # The defender (Bird) should just use the runbehind for curve locus instead of me porting Ross's poorly written
##~     # code to python.
##~     
##~     # Then in the following only the goal avoidance is written.
##~     # Can used the turnDir below.
##~     
##~     # From left ro right combined with runbehindball.
##~     if amIInTheGoalBox() and Global.selfLoc.getHeading() > 180 and \
##~         Global.selfLoc.getHeading() < 350:
##~         
##~         # Right to left.
##~         if direction == Constant.dANTICLOCKWISE:       
##~             Global.myRole = Constant.STRIKER
##~             px = (Constant.FIELD_WIDTH / 2 + Constant.WALL_THICKNESS / 2) + 15
##~             py = (Constant.GOALBOX_DEPTH + Constant.WALL_THICKNESS) + 15
##~             phead = HelpMath.normalizeAngle_180(HelpMath.RAD2DEG(math.atan2(py - Global.selfLoc.getPos()[1], \
##~                     px - Global.selfLoc.getPos()[0])) - Global.selfLoc.getHeading())
##~             Global.finalAction[Constant.AAForward] = 7
##~             Global.finalAction[Constant.AALeft]    = 0
##~             Global.finalAction[Constant.AATurnCCW] = HelpMath.CLIP(phead, 100)
##~         
##~         # Left to right.
##~         else:
##~             Global.myRole = Constant.SUPPORT
##~             px = (Constant.FIELD_WIDTH / 2 - Constant.WALL_THICKNESS / 2) - 15
##~             py = (Constant.GOALBOX_DEPTH + Constant.WALL_THICKNESS) + 15
##~             phead = HelpMath.normalizeAngle_180(HelpMath.RAD2DEG(math.atan2(py - Global.selfLoc.getPos()[1], \
##~                     px - Global.selfLoc.getPos()[0])) - Global.selfLoc.getHeading())
##~             Global.finalAction[Constant.AAForward] = 7
##~             Global.finalAction[Constant.AALeft]    = 0
##~             Global.finalAction[Constant.AATurnCCW] = HelpMath.CLIP(phead, 100)
##~        
##~ def amInTheGoalBoxAndFacingLeftSide():
##~     if amIInTheGoalBox() and Global.selfLoc.getHeading() >= 270 and \
##~         Global.selfLoc.getHeading() <= 360:
##~         return True
##~     return False
##~     
##~ def amInTheGoalBoxAndFacingRightSide():
##~     if amIInTheGoalBox() and Global.selfLoc.getHeading() <= 270 and \
##~         Global.selfLoc.getHeading() > 180:
##~         return True
##~     return False
##~ 
##~ def amIInTheGoalBox():
##~     if Global.selfLoc.getPos()[1] < (Constant.GOALBOX_DEPTH + Constant.WALL_THICKNESS) and \
##~        Global.selfLoc.getPos()[0] <= Constant.RIGHT_GOALBOX_EDGE_X and \
##~        Global.selfLoc.getPos()[0] >= Constant.LEFT_GOALBOX_EDGE_X:
##~         return True
##~     else:
##~         return False
##~          
def ballInOwnBox(errorMargin):
    cond1 = Global.ballY < (Constant.GOALBOX_DEPTH + Constant.WALL_THICKNESS) + \
            errorMargin
    cond2 = Global.ballX < (Constant.FIELD_WIDTH + Constant.GOALBOX_WIDTH) / 2 + \
            errorMargin
    cond3 = Global.ballX > (Constant.FIELD_WIDTH - Constant.GOALBOX_WIDTH) / 2 - \
            errorMargin

    return cond1 and cond2 and cond3

def areFurthestX():
    for i in range(Constant.NUM_TEAM_MEMBER):
        if Global.teamPlayers[i].isGoalie() or \
           Global.teamPlayers[i].getCounter() == 0:
            continue

        if abs(Global.selfLoc.getPos()[0] - Global.ballX) - \
           abs(Global.teammatesLoc[i].getPos()[0] - Global.ballX) < 0:
            return False
            
    return True
    
def areClosestX():
    for i in range(Constant.NUM_TEAM_MEMBER):
        if Global.teamPlayers[i].isGoalie() or \
           Global.teamPlayers[i].getCounter() == 0:
            continue
            
        if abs(Global.selfLoc.getPos()[0] - Global.ballX) - \
           abs(Global.teammatesLoc[i].getPos()[0] - Global.ballX) > 0:
            return False
            
    return True
    
def numDefenders():
    ret = 0
    for i in range(Constant.NUM_TEAM_MEMBER):
        # Don't really need to ignore the goalie as that in 2003. Since the goalie
        # shouln't send the defender signal at all. If it does, then something wrong.
        if Global.teamPlayers[i].getCounter() > 0 and\
            Global.teamPlayers[i].isDefender():
            ret = ret + 1
            
    return ret
  
