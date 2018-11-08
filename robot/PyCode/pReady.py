#
#   Copyright 2004 The University of New South Wales (UNSW) and National  
#   ICT Australia (NICTA).
#
#   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
#   redistribute it and/or modify it under the terms of the GNU General  
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
#  $Id: pReady.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# pReady.py
#
# Implementation for the ready state.
#===============================================================================

"""
Game controller behaviours
"""

import VisionLink
import Debug
import Global
import Constant
import hTrack
import hFWHead
import hStuck
#import sStealthDog
import sDodgyDog
#import sFindBall

import hMath
import hTeam

import Action
import Indicator

# Time to scan before moving
INITIAL_LOCALISE_TIME = 150

########################### Possible ready player positions ###################
# Kicking off: We position the centre forward close to the ball (he will move
# closer on visual ball). Position A on the left is back in line with the
# beacons and fairly wide - he will become the winger on kick-off and move
# forward a bit. Position C to the right is up nearly at the half-way line
# and midway between the centre and sidelines. In a set play he will move
# forward to attack the ball wherever the kicker kicks it to, otherwise he
# will drop into the supporter position.
P_KICK_KICKER = (Constant.FIELD_WIDTH / 2.0, Constant.FIELD_LENGTH / 2.0 - 45)

P_KICK_LEFT_BACK     = (110, 0.20 * Constant.FIELD_LENGTH)
P_KICK_RIGHT_FORWARD = (Constant.FIELD_WIDTH - 75, 0.43 * Constant.FIELD_LENGTH)

# We have an alternate version that has the left player forward and the
# right backward, and choose randomly between them
P_KICK_LEFT_FORWARD = (75, 0.43 * Constant.FIELD_LENGTH)
P_KICK_RIGHT_BACK   = (Constant.FIELD_WIDTH - 110, 0.20 * Constant.FIELD_LENGTH)

# Receiving: The players are all in line with the beacons. One forward is
# directly between ball and goal. On one side the forward is just more than
# halfway in from the side. The other forward is much more than halfway in
# so it's asymmetrical. While it appears that defence is stronger on the close
# side (with the close in forward) in fact we will have the centre and wide
# robots charge the ball, the close robot is *already* in the supporter position
# and will not move forward, remaining in defence in case the ball gets
# through the chargers.
P_REC_CENTRE      = (Constant.FIELD_WIDTH / 2.0, 0.21 * Constant.FIELD_LENGTH)

P_REC_LEFT_WIDE     = (100, 0.21 * Constant.FIELD_LENGTH)
P_REC_RIGHT_CLOSE   = (Constant.FIELD_WIDTH - 130, 0.21 * Constant.FIELD_LENGTH)

# Again a symmetric alternate
P_REC_LEFT_CLOSE    = (130, 0.21 * Constant.FIELD_LENGTH)
P_REC_RIGHT_WIDE    = (Constant.FIELD_WIDTH - 100, 0.21 * Constant.FIELD_LENGTH)

P_GOALIE            = (Constant.FIELD_WIDTH / 2.0, 20)   # Goalie


###############################################################################

# Number of kickoff formations to choose between
NUM_FORMATIONS = 2

# Formations, indexed by chosenFormation
kickFormations = [(P_KICK_LEFT_BACK, P_KICK_KICKER, \
                    P_KICK_RIGHT_FORWARD, P_GOALIE), \
                  (P_KICK_LEFT_FORWARD, P_KICK_KICKER, \
                    P_KICK_RIGHT_BACK, P_GOALIE)]

receiveFormations = [(P_REC_LEFT_WIDE, P_REC_CENTRE, P_REC_RIGHT_CLOSE, P_GOALIE), \
                  (P_REC_LEFT_CLOSE, P_REC_CENTRE, P_REC_RIGHT_WIDE, P_GOALIE)]
chosenFormation = None


# Alpha.
alphaInvereseSquare = 80.0

# Locking the fire stealth dog.
lockFireStealth = 0

# Timer for how many frames the robot want to active localise without moving.
# This need to be reset after every goal scored.
timer = 0

# This boolean should be true if the robot think it is in the correct starting
# position.  This need to be reset after every goal scored.
inStartPos = False

# This boolean should be true if the robot think it is facing toward the target
# goal.  This need to be reset after every goal scored.
isFacingTargetGoal = False

# The turning direction in case the robot is not facing toward the target goal.
lastTurningDirection = None

# Want to use stuck detection for obstacle avoidance?
useStuckObstacleAvoidance  = True

# Want to use vision for obstacle avoidance?
useVisionObstacleAvoidance = True

# Want to use gps for obstacle avoidance?
useGPSObstacleAvoidance    = True

gUseDodgyDog = True

# Getting out the goal.
getOutOfGoalCounter   = 0
getOutOfGoalDirection = None

# Initalise all the timers. This should be called whenever a goal has been
# scored.
def initReadyMatchPlaying():
    global timer, inStartPos, isFacingTargetGoal, lastTurningDirection
    timer                = 0
    inStartPos           = False
    isFacingTargetGoal   = False
    lastTurningDirection = None
    Global.kickOffState  = Constant.NOTDECIDED
    Action.closeMouth()

# Implementation of the ready state. In this state the robot should move to its
# correct position , then stop and search for ball or localise or even start to
# move again until set state arrived.
def readyMatchPlaying():
    global debug, timer, inStartPos, isFacingTargetGoal, getOutOfGoalDirection, lastTurningDirection, gUseDodgyDog
    
    selfX = Global.selfLoc.getPos()[0]
    selfY = Global.selfLoc.getPos()[1]
    selfH = Global.selfLoc.getHeading()
    Action.closeMouth()

    # Player number detection. The goalie *must* be player one. Freak if not.
    if Global.myPlayerNum == 1 and not Global.isGoalieBehaviour:
        Action.kick(Action.ChestPushWT)
        return
    elif Global.myPlayerNum != 1 and Global.isGoalieBehaviour:
        Action.kick(Action.ChestPushWT)
        return

    if not Global.penaltyShot and hStuck.amIStuckForward(dodgeTime = 9):
        return

    # Need to move toward the field ?
    if shouldITurnTowardTheField():
        Action.walk(0, 0, getOutOfGoalDirection)

    # Am I in the correct starting position and facing toward the target goal
    # (or facing upward)?
    if inStartPos and isFacingTargetGoal:
            
        # use the ball distance to try to move as close as the centre circle
        #if VisionLink.getKickOffTeam() == VisionLink.getTeamColor() and \
        #       useBallDistanceSetKickOffPosition():
        #    return

        #  Do I need to move to a new position if I am in a wrong position?
        if Debug.allowReadyMove:
            if hMath.getDistanceBetween(selfX, selfY, \
                    Global.kickOffPos[0], Global.kickOffPos[1]) \
                        > Constant.POS_OFFSET_OUT_KICK:
                inStartPos = False
                return
       
                
        # Turn myself if I am not facing toward the desired heading. At the
        # moment 90 is the desired heading angle.
        # TODO: Accoiding to different positions, different desired heading.
        if abs(hMath.normalizeAngle_180(selfH - 90)) > \
            Constant.READY_HEADING_OFFSET:
            isFacingTargetGoal = False
            return
    
        inPositionAndFacingTargetLocalise()
        return

    # Am I in the correct starting position but not facing the target goal (or
    # facing upward)?
    if inStartPos:
        #  Do I need to move to a new position if I am in a wrong position?
        if Debug.allowReadyMove:
            if hMath.getDistanceBetween(selfX, selfY, \
                Global.kickOffPos[0], Global.kickOffPos[1])\
                    > Constant.POS_OFFSET_OUT_KICK:
                inStartPos = False
                return
        
        # Is me facing toward the target goal now? If I am facing the right
        # direction, then better reset the turning direction variable.
        if abs(selfH - 90) < Constant.READY_HEADING_OFFSET:
            isFacingTargetGoal   = True
            lastTurningDirection = None
            
        # If I am not facing toward the target goal, then turn slowly without
        # moving forward or left.
        else:
            # Determine the best turning direction and stick with it.
            if lastTurningDirection == None:
                # I haven't determine my turning direction yet. Determine now.
                if Global.selfLoc.getHeading() >= 270 or \
                   Global.selfLoc.getHeading() <= 90: # -90 <= angle <= 90
                    lastTurningDirection = Constant.dANTICLOCKWISE
                    if Debug.readyDebug:
                        print "Anti-clockwise direction!"
                else: 
                    # 90 < angle < 270
                    lastTurningDirection = Constant.dCLOCKWISE
                    if Debug.readyDebug:
                        print "Clockwise direction!"
            
            if lastTurningDirection == Constant.dCLOCKWISE:            
                Action.walk(0,0,-15)
            else:
                Action.walk(0,0,15)
        return
        
    # Phase1: I am not in the correct starting position. First step for me is
    # to stand still and beacon localise.
    if timer <= INITIAL_LOCALISE_TIME:
        if Debug.readyDebug and timer == 1:
            print "Phase 1 has been executed!"
        if timer <= 79:
            Global.kickOffState = Constant.NOTDECIDED
        Action.stopLegs()

        # Need to move toward the field ?
        if shouldITurnTowardTheField():
            Action.walk(0, 0, getOutOfGoalDirection)

        hTrack.stationaryLocalise()
        timer = timer + 1
        if timer >= 80:
            # Now decide my starting kickoff position dynamically now.
            if hTeam.robotAlive() != 1:
                getMyKickOffPosition()
        return
        
    lastTurningDirection = None
                
    # Now we can either use the static result from getMyKickOffPosition() or
    # dynamically calculated again,
    if hTeam.robotAlive() != 1:
        getMyKickOffPosition()
                        
     # If I am unable to find the kickoff position earlier (possibly because of
     # wirless problem), then probably only 2 or less robots are on the field
     # now. Notice that I can't be a goalie in this case.  I must be either
     # player 2 forward or player 3 forward. Not player 4 forward because it
     # determines its kicking position without worrying about any other
     # forwards.
    if Global.kickOffState == Constant.NOTDECIDED or Global.kickOffPos == None:
        alive = hTeam.forwardAlive()
        if alive >= 3:
            if Debug.readyDebug:
                print "Something went wrong, three forwards without assigning! Automatical assign!"
                
##~             # This case may happen in friendly games where not all four robots are present.
##~             # Firstly if there are three forwards alive, but I am not assigned a position. Then:
##~             #   1. I can't be player 1 because this is a goalie
##~             #   2. I can't be player number 4 forward.
##~             # So I must be either player 2 or player 3 forward.
##~             #
##~             # If I am a player 3, then probably the three forwards are: 1, 2, 3
##~             # If I am a player 2, then probably the three forwards are: 1, 2, 4
##~             
            if Global.myPlayerNum == 3:
                _, Global.kickOffPos, _, _ = getAllPossibleKickingOffPosition()
                Global.kickOffState = Constant.DECIDEPOSITION_B
            
            elif Global.myPlayerNum == 2:
                # Try to get player 4's decision.
                if Global.teamPlayers[3].getCounter() > 0:
                    position = Global.teamPlayers[3].getKickedOff()

                    if position == Constant.DECIDEPOSITION_A:
                        determineMyKickingOffPositionIfOnlyMeForwardAlive(False, True, True)
                    elif position == Constant.DECIDEPOSITION_B:
                        determineMyKickingOffPositionIfOnlyMeForwardAlive(True, False, True)
                    elif position == Constant.DECIDEPOSITION_C:
                        determineMyKickingOffPositionIfOnlyMeForwardAlive(True, True, False)                    
                    else:
                        Debug.goAndMakeTraceBack()
                        return
                        
                else:
                    # Well, lets wait.
                    return
                 
        elif alive == 2:
            if Debug.readyDebug:
                print "Only two alive case - What the hell?"
            if Global.myPlayerNum == 2:
                Global.kickOffPos, b, c, d = getAllPossibleKickingOffPosition()
                Global.kickOffState = Constant.DECIDEPOSITION_A
            elif Global.myPlayerNum == 3:
                a, Global.kickOffPos, c, d = getAllPossibleKickingOffPosition()
                Global.kickOffState = Constant.DECIDEPOSITION_B
            else:
                if Debug.readyDebug:
                    print "Something went wrong, I am not player 2 or 3, why not decided position?"

        elif alive == 1:
            if Debug.readyDebug:
                print "Only one alive case!"
                
            a, b, c, d = getAllPossibleKickingOffPosition()
            
            if Global.myPlayerNum == 2:
                Global.kickOffPos = a
                Global.kickOffState = Constant.DECIDEPOSITION_A
            elif Global.myPlayerNum == 3:
                Global.kickOffPos = b
                Global.kickOffState = Constant.DECIDEPOSITION_B
            elif Global.myPlayerNum == 4:
                Global.kickOffPos = c
                Global.kickOffState = Constant.DECIDEPOSITION_C
            else: # Goalie
                Global.kickOffPos = d
                Global.kickOffState = Constant.DECIDEPOSITION_D

    # At this point the kicking off position must been determined.
    showKickOffHeadIndicator()
    # Set the initial roles based on position
    if Global.kickOffPos == P_REC_CENTRE:
        Global.myRole = Constant.ATTACKER
    elif Global.kickOffPos == P_REC_LEFT_WIDE \
            or Global.kickOffPos == P_REC_RIGHT_WIDE:
        Global.myRole = Constant.DEFENDER
    elif Global.kickOffPos == P_REC_RIGHT_CLOSE \
            or Global.kickOffPos == P_REC_LEFT_CLOSE:
        Global.myRole = Constant.SUPPORTER
    elif Global.kickOffPos == P_KICK_KICKER:
        Global.myRole = Constant.ATTACKER
    elif Global.kickOffPos == P_KICK_LEFT_BACK \
            or Global.kickOffPos == P_KICK_RIGHT_BACK:
        Global.myRole = Constant.DEFENDER
    elif Global.kickOffPos == P_KICK_RIGHT_FORWARD \
            or Global.kickOffPos== P_KICK_LEFT_FORWARD:
        Global.myRole = Constant.SUPPORTER

        
    # Testing the kickoff position without actually moving them.
##~     if Debug.readyDebug and Debug.testKickOffPosition:
##~         if Global.kickOffState == None:
##~             print "My kickoff position is: None %f" %Global.myPlayerNum
##~         else:
##~             print "My player number: %f" %Global.myPlayerNum
##~             print "My kickoff position is: %f" %Global.kickOffState 
##~         return
        
    # Move from phase 2 to phase 3. Am I in the correct starting position now?
    dist = hMath.getDistanceBetween(selfX, selfY, Global.kickOffPos[0], 
                                    Global.kickOffPos[1])
    if dist < Constant.POS_OFFSET_IN_KICK:
        # Ok, I am in the correct starting position. I stop myself now.
        Action.stopLegs()
        inStartPos = True
        
    # Phase2: I need to walk to my starting position. Localise by moving the
    # head while I move.
    else:
        if dist < sDodgyDog.MIN_DODGY_TARGET_DIST:
            hTrack.stationaryLocalise()
        else:
            # Move head less while dodgy to see obstacles
            hTrack.stationaryLocalise(5, 30, -30)

        if gUseDodgyDog and \
                sDodgyDog.shouldIBeDodgy(Global.kickOffPos[0],
                                        Global.kickOffPos[1], True, 
                                        Constant.POS_OFFSET_OUT_KICK):      
            if sDodgyDog.dodgyDogTo(Global.kickOffPos[0], Global.kickOffPos[1],
                                    True, 
            Constant.POS_OFFSET_OUT_KICK) == Constant.STATE_SUCCESS:
                inStartPos = True
        else:
            movingAction()
        
# if the position is correct, then move forward and backward according to ball
# position
def useBallDistanceSetKickOffPosition():
    global inStartPos, isFacingTargetGoal
    #print "Global.ballD", Global.ballD
    if Global.lostBall <= 15 and Global.kickOffState != None and \
    Global.kickOffState == Constant.DECIDEPOSITION_B and inStartPos and \
    isFacingTargetGoal and \
    -Constant.CIRCLE_DIAMETER/2 < Global.ballX - Constant.FIELD_WIDTH/2.0 < Constant.CIRCLE_DIAMETER/2 and \
    -Constant.CIRCLE_DIAMETER/2 < Global.ballY - Constant.FIELD_LENGTH/2.0 < Constant.CIRCLE_DIAMETER/2 \
    :
        if Global.haveBall < 3:
            pass
        if Global.ballD <= Constant.CIRCLE_DIAMETER/2 + 5:
            Action.walk(-7, 0, 0, minorWalkType=Action.SkeFastForwardMWT)
        elif Global.ballD >= Constant.CIRCLE_DIAMETER/2 + 20:
            Action.walk(9, 0, 0, minorWalkType=Action.SkeFastForwardMWT)
        elif Global.ballH > 8:
            Action.walk(0, 0, Global.ballH,
                        minorWalkType=Action.SkeFastForwardMWT)
        elif Global.ballH < -8:
            Action.walk(0, 0, Global.ballH,
                        minorWalkType=Action.SkeFastForwardMWT)
        else:
            #don't move if the ball within the range
            pass
        #hTrack.trackVisualBall()
        #hFWHead.minPosVariance = hFWHead.minHeadVariance = 0
        hFWHead.compulsoryAction = None
        hFWHead.DecideNextAction()
        #sFindBall.perform(True)
        return True
    else:
        return False

def inPositionAndFacingTargetLocalise():
    # Now what localise method to use? We have some selections here.
    # 1. Stationary Localise 2. Smart Localise 3. Beacon Localise
    #if VisionLink.getKickOffTeam() == VisionLink.getTeamColor() and \
    #        Global.kickOffState == Constant.DECIDEPOSITION_B:
    #    # Kicker looks for ball as well as beacons
    #    #hTrack.scanLikeGermanTeam()
    #    hTrack.scan(maxPan = 60, minPan = -60, highCrane = 35,
    #                lowSpeed = 8, highSpeed = 8)
    #else:
        hTrack.stationaryLocalise()        
    #return
        
def movingAction():
    # Move to the desired position with obstacle avoidance.
    obstacleAvoidance()
        
def showKickOffHeadIndicator():
    if Global.kickOffState == Constant.DECIDEPOSITION_A:
        Indicator.showFacePattern([1,0,0,0,0])
    elif Global.kickOffState == Constant.DECIDEPOSITION_B:
        Indicator.showFacePattern([1,0,0,0,1])
    elif Global.kickOffState == Constant.DECIDEPOSITION_C:
        Indicator.showFacePattern([1,0,1,0,1])
    elif Global.kickOffState == Constant.DECIDEPOSITION_D:
        Indicator.showFacePattern([1,1,1,1,1])
    else:
        print "Why the kicking off positon is not determined?"
        Debug.goAndMakeTraceBack()
        return

# This function calculates and determine the best kickoff position based on gps.
def getMyKickOffPosition():    
    # If only one forward (ie. me) alive, then goto the centre kick-position.
    if Global.framesSince2ForwardsPresent >= 60 and \
       Global.kickOffState == Constant.NOTDECIDED:
        if Debug.readyDebug:
            print "Execute only me alive case! My kickoff state is unknown!"
        determineMyKickingOffPositionIfOnlyMeForwardAlive(True, True, True)
        
    # If there are two forwards communicate but not all three forwards are
    # alive, then determine the position according to some simple rules.
    elif Global.framesSince3ForwardsPresent >= 60 and \
         Global.kickOffState == Constant.NOTDECIDED:
        if Debug.readyDebug:
            print "Execute only two forwards alive case! My kickoff state is unknown!"
        determineMyKickingOffPositionIfOnly2ForwardsAlive(True, True, True)
    
    # If I am a goalie or highest player number forward or forward with higher
    # player number than me has decided the kickoff position, then I decide my
    # kick-position.
    thirdForwardDecide  = Global.teamPlayers[3].getCounter() > 0 and \
                          Global.teamPlayers[3].getKickedOff() != Constant.NOTDECIDED
    secondForwardDecide = Global.teamPlayers[2].getCounter() > 0 and \
                          Global.teamPlayers[2].getKickedOff() != Constant.NOTDECIDED
    
    if Debug.readyDebug:
        print "Third forward decide:  %f " %thirdForwardDecide
        print "Second forward decide: %f"  %secondForwardDecide
    
    # Just check if any position duplicate.
    if (Global.myPlayerNum == 2 and Global.teamPlayers[3].getCounter() > 0 and
        Global.teamPlayers[3].getKickedOff() == Global.kickOffState):
        if Debug.readyDebug:
            print "Duplicate 1 - 4!"
        determineMyKickingOffPositionBasedOnTeamPositioning()
        return
    
    # Player 1 = goalie. Player 4 = highest player forward.
    if not(Global.myPlayerNum == 1 or Global.myPlayerNum == 4 or (Global.myPlayerNum
           == 3 and thirdForwardDecide) or (Global.myPlayerNum == 2 and secondForwardDecide)):
        return
        
    if Debug.readyDebug:
        print "Now I will decide my start-off position!"
    
    # Now I can decide my start off position.
    determineMyKickingOffPositionBasedOnTeamPositioning()
        
def determineMyKickingOffPositionIfOnlyMeForwardAlive(canChooseA, canChooseB, canChooseC):

    positionA, positionB, positionC, positionD = getAllPossibleKickingOffPosition()
    positionD = None
    
##~     # Distance?
##~     distA = HelpShort.getDistanceBetween(positionA[0], positionA[1], Global.selfLoc.getPos()[0], Global.selfLoc.getPos()[1])
##~     distB = HelpShort.getDistanceBetween(positionB[0], positionB[1], Global.selfLoc.getPos()[0], Global.selfLoc.getPos()[1])
##~     distC = HelpShort.getDistanceBetween(positionC[0], positionC[1], Global.selfLoc.getPos()[0], Global.selfLoc.getPos()[1])
    
    if canChooseB:
        Global.kickOffState = Constant.DECIDEPOSITION_B
        Global.kickOffPos = positionB
        if Debug.readyDebug:
            print "Only me alive, choose B!"
            
    elif canChooseC:
        Global.kickOffState = Constant.DECIDEPOSITION_C
        Global.kickOffPos = positionC
        if Debug.readyDebug:
            print "Only me alive, choose C!"
            
    elif canChooseA:
        Global.kickOffState = Constant.DECIDEPOSITION_A
        Global.kickOffPos = positionA
        if Debug.readyDebug:
            print "Only me alive, choose A!"
        
# Notice that only forward need to call this function (goalie has only one
# possible kickoff position).    
def determineMyKickingOffPositionIfOnly2ForwardsAlive(canChooseA, canChooseB,
canChooseC, ignoreIndex = None):
    global debug

    # Check preconditions.
    if (not canChooseA and not canChooseB) or (not canChooseA and not canChooseC) \
       or (not canChooseB and not canChooseC):
        Debug.goAndMakeTraceBack()
        return
        
    # My global x value.
    myX       = Global.selfLoc.getPos()[0]
    teammateX = None
        
    # The other teammate's x value. Hopefully its counter is greater than 0.
    for i in range(Constant.NUM_TEAM_MEMBER):
        if Global.teamPlayers[i].isGoalie() or i == Global.myPlayerNum - 1 or \
           Global.teamPlayers[i].getCounter() == 0 or i == ignoreIndex:
            continue
            
        if Global.teamPlayers[i].getCounter() > 0:
            teammateX = Global.teammatesLoc[i].getPos()[0]
        
    # In case if and only if no any teammate's counter is greater than 0.
    if teammateX == None:
        if Debug.readyDebug:
            print "Warning: I think 2 forwards including me alive, but only my counter is greater than zero!"
        # Just treat as me alive only.
        determineMyKickingOffPositionIfOnlyMeForwardAlive(canChooseA, canChooseB, canChooseC)
        return

    if Debug.readyDebug:
        print "My x value: %f" %myX
        print "My teammate x: %f" %teammateX

    # Return all the candidate kicking off positions.
    positionA, positionB, positionC, positionD = getAllPossibleKickingOffPosition()
    
    # Now get rid of those position I can't choose.
    positionD = None
    if not canChooseA:
        positionA = positionC
    elif not canChooseB:
        positionB = positionC
    elif not canChooseC:
        if Debug.readyDebug:
            print "Execute canChooseC case!"
    else:
        # If all positions can choose, then we better choose B and C because we
        # are more toward the target goal if our team is kicking off.
        positionA = positionC
        
    # Sort positionA and positionB in terms of x values.
    if positionA[0] > positionB[0]:
        temp = positionA
        positionA = positionB
        positionB = temp

    # We have positionA and positionB to choose between two robots.  Now we are
    # sure that positionA has smaller or equal x values than positionB, if
    # the other robot is left of me , then I should choose positionB.
    if teammateX <= myX:
        if Debug.readyDebug:
            print "Two forwards case: teammateX <= myX"
                                               
        if not canChooseA:
            # If this condition satisfy, then (positionB, positionC)
            Global.kickOffState = Constant.DECIDEPOSITION_C
        elif not canChooseB:
            # If this condition satisfy, then (positionA, positionC)
            Global.kickOffState = Constant.DECIDEPOSITION_C
        elif not canChooseC:
            # If this condition satisfy, then (positionA, positionB)
            Global.kickOffState = Constant.DECIDEPOSITION_B
        else:
            # If this condition satisfy, then (positionB, positionC)
            Global.kickOffState = Constant.DECIDEPOSITION_C
        Global.kickOffPos = positionB

    else:
        if Debug.readyDebug:
            print "I am smaller X than my teammate!"
        if not canChooseA:
            # If this condition satisfy, then (positionB, positionC)
            Global.kickOffState = Constant.DECIDEPOSITION_B
        elif not canChooseB:
            # If this condition satisfy, then (positionA, positionC)
            Global.kickOffState = Constant.DECIDEPOSITION_A
        elif not canChooseC:
            # If this condition satisfy, then (positionA, positionB)
            Global.kickOffState = Constant.DECIDEPOSITION_A
        else:
            # If this condition satisfy, then (positionB, positionC)
            Global.kickOffState = Constant.DECIDEPOSITION_B
        Global.kickOffPos = positionA
        
# Determine my kickoff position based on team positioning.    
def determineMyKickingOffPositionBasedOnTeamPositioning():
    global debug
    
    if Debug.readyDebug:
        print "Execute determineMyKickingOffPositionBasedOnTeamPositioning()!"

    # Return all the candidate kicking off positions.
    positionA, positionB, positionC, positionD = getAllPossibleKickingOffPosition()
        
    # If i am the goalie, then only one possible kickoff position for me to go.
    # Need to come back for the changing player number.
    # Notice that goalie is player number 1.
    if Global.myPlayerNum == 1:
        if Debug.readyDebug:
            print "I am the player with number 1, I choose DECIDEPOSITION_D!"
        Global.kickOffState = Constant.DECIDEPOSITION_D
        Global.kickOffPos          = positionD
        return
        
    # Determine kick off positions for the forwards.
        
    # So at this point all three forwards are expected to be communicating.
    # If I am the forward with highest player number, then I have three
    # possible places to choose.
    if Global.myPlayerNum == 4:
        if Debug.readyDebug:
            print "Execute player number == 4 case!"
            
        anyForwardLeft, anyForwardRight = anyForwardXAxisLeftRight()
        
        if Debug.readyDebug:
            if anyForwardLeft:
                print "Yes teammate forward left!"
            else:
                print "No teammate forward left!"
            
            if anyForwardRight:
                print "Yes teammate forward right!"
            else:
                print "No teammate forward right!"
        
        if not anyForwardLeft:
            Global.kickOffState = Constant.DECIDEPOSITION_A
            Global.kickOffPos          = positionA
            if Debug.readyDebug:
                print "Player 4 - PositionA case!"
            return
        
        if not anyForwardRight:
            Global.kickOffState = Constant.DECIDEPOSITION_C
            Global.kickOffPos          = positionC
            if Debug.readyDebug:
                print "Player 4 - PositionC case!"
            return
        
        Global.kickOffState = Constant.DECIDEPOSITION_B
        Global.kickOffPos          = positionB
        if Debug.readyDebug:
            print "Player 4 - PositionB case!"
        return
       
    # If I am the forward with second highest player number, then I have two
    # possible places to choose.
    if Global.myPlayerNum == 3:
        if Debug.readyDebug:
            print "Execute player number 3 case!"
            
        # So the player with higher player number than me actually not exist.
        # So I determine my position assuming I am the only forward.
        if Global.teamPlayers[3].getCounter() == 0 and \
           Global.kickOffState == Constant.NOTDECIDED:
            if Debug.readyDebug:
                print "I am player 2, but player 3 not exist, run with single alive case!"
            determineMyKickingOffPositionIfOnlyMeForwardAlive(True, True, True)
        else:
            if Global.teamPlayers[3].getKickedOff() == Constant.DECIDEPOSITION_A:
                if Debug.readyDebug:
                    print "The player 4 choose A, so no A for me!"
                determineMyKickingOffPositionIfOnly2ForwardsAlive(False, True, True, 3)
            elif Global.teamPlayers[3].getKickedOff() == Constant.DECIDEPOSITION_B:
                if Debug.readyDebug:
                    print "The player 4 choose B, so no B for me!"
                determineMyKickingOffPositionIfOnly2ForwardsAlive(True, False, True, 3)
            elif Global.teamPlayers[3].getKickedOff() == Constant.DECIDEPOSITION_C:
                if Debug.readyDebug:
                    print "The player 4 choose C, so no C for me!"
                determineMyKickingOffPositionIfOnly2ForwardsAlive(True, True, False, 3)
            else:
                if Debug.readyDebug:
                    print "So what did the player 4 choose?"
                    print Global.teamPlayers[3].getKickedOff()
                Debug.goAndMakeTraceBack()
                return
            
    # Only one posssible position for me to choose.
    if Global.myPlayerNum == 2:
        if Debug.readyDebug:
            print "Execute player numner 2 case!"
    
        # However if the player with higher player number than me 
        if Global.teamPlayers[2].getCounter() == 0 and \
           Global.kickOffState == Constant.NOTDECIDED:
            if Debug.readyDebug:
                print "I am player 1, but player 2 not exist, run with single alive case!"
        else:
            chooseA, chooseB, chooseC = [True, True, True]
            if Debug.readyDebug:
                print "Execute player number 2 kick off case!"
            for i in [2,3]:
                if Global.teamPlayers[i].getCounter() == 0:
                    if Debug.readyDebug:
                        print "Why 2 or 3 is not alive?"
                    continue

                # Will this get into problems when the wireless is down for a
                # short time?
        
                if Global.teamPlayers[i].getKickedOff() == Constant.DECIDEPOSITION_A:
                    chooseA = False
                    if Debug.readyDebug:
                        print "Higher player choose A, so no A for me!"
                        
                elif Global.teamPlayers[i].getKickedOff() == Constant.DECIDEPOSITION_B:
                    chooseB = False
                    if Debug.readyDebug:
                        print "Higher player choose B, so no B for me!"
                        
                elif Global.teamPlayers[i].getKickedOff() == Constant.DECIDEPOSITION_C:
                    chooseC = False
                    if Debug.readyDebug:
                        print "Higher player choose C, so no C for me!"
                                    
            determineMyKickingOffPositionIfOnlyMeForwardAlive(chooseA, chooseB, chooseC)



# Return a list of list elements. Notice that the returned positions have no
# relation to the player number at all. This "randomly" picks between our
# presets but all dogs choose the same position set.
def getAllPossibleKickingOffPosition():
    global chosenFormation
    if Debug.readyDebug:
        print "Executed getAllPossibleKickingOffPosition()!"

    num = Global.getSharedRandom()
    chosenFormation = num % NUM_FORMATIONS
    #print "Chosen kickoff formation", chosenFormation, ":", \
    #    kickFormations[chosenFormation]

    # If my team is kicking off.
    if VisionLink.getKickOffTeam() == VisionLink.getTeamColor():
            posA, posB, posC, posD = kickFormations[chosenFormation]
    else:
        # The other team is kicking off.
            posA, posB, posC, posD = receiveFormations[chosenFormation]

    if Debug.readyStealthDebug:
        if Debug.readyStealthDebug == 1:
            posD = [Constant.FIELD_WIDTH / 2.0, 0]
        else:
            posD = [Constant.FIELD_WIDTH / 2.0, Constant.FIELD_LENGTH]

    return [posA, posB, posC, posD]

# Use by the forward with the highest player number.
def anyForwardXAxisLeftRight():
    left, right = [False, False]
    
    if Debug.readyDebug:
        print "------"
    
    for i in range(Constant.NUM_TEAM_MEMBER):
        # Ignore the goalie and myself.
        if Global.teamPlayers[i].isGoalie() or i == Global.myPlayerNum - 1:
            continue
            
        # Assume getPos()[0] is equal the x coordinatives.
        if Global.teamPlayers[i].getCounter() > 0 and \
           Global.teammatesLoc[i].getPos()[0] < Global.selfLoc.getPos()[0]:
            left = True
            
        # Assume getPos()[0] is equal the x coordinatives.
        if (Global.teamPlayers[i].getCounter() > 0 and
           Global.teammatesLoc[i].getPos()[0] > Global.selfLoc.getPos()[0]):
            right = True
        
    if Debug.readyDebug:
        print Global.teammatesLoc[1].getPos()
        print Global.teammatesLoc[2].getPos()

    return [left, right]

    
def amIStuckBasedOnPWMDutyCycle():
    (maxPWM, _) = getStuckInfo()
       
    if Debug.stuckDebug:
        print "MaxPWM: %f" %maxPWM
    
    stuck = (maxPWM >= 1300 and Action.framesSinceLastKicking >= 150)    
    return stuck

    
def getStuckInfo():
    return max(Global.getLastMaxPWMValues()) #return (maxPWM,whichLeg)   
    

    
# This function is the obstacle avoidance function.
def obstacleAvoidance():
    global useStuckObstacleAvoidance, useVisionObstacleAvoidance
    global useGPSObstacleAvoidance, lockFireStealth
    
    #if lockFireStealth > 0:
    #    Global.myRole   = Constant.STEALTHDOG
    #    lockFireStealth = lockFireStealth - 1
    #    Action.walk(0, sStealthDog.leftComp, 0)
    #    return
    
    # First obstacle avoidance - stuck detection.
    if useStuckObstacleAvoidance:
        if amIStuckBasedOnPWMDutyCycle():
            Global.myRole = Constant.STUCKER
            Action.walk(-7, -15, 0)
            return
        
    # Want to try swapping the stealth and the follow moving to straight point?
        
    # Move to my kickoff position directly.
    if not moveToMyPositionDirectly():
        Global.myRole = Constant.ATTACKER
        if Debug.readyDebug:
            print "moveToMyPositionDirectly() no obstacle avoidance!"
        return

    # Second obstacle avoidance - fire stealth dog vision.
    #if useVisionObstacleAvoidance:
    #    if sStealthDog.firesStealthDog():
    #        lockFireStealth = 15
    #        if Debug.readyDebug:
    #            print "Fire stealth turn on!"
    #        return
        
    # Second obstacle avoidance - normal stealth dog vision.
    #if useVisionObstacleAvoidance:
    #    if sStealthDog.stealthDog(False):
    #        if Debug.readyDebug:
    #            print "Stealth dog turn on for ready state!"
    #        return
        
    # Third obstacle avoidance - GPS.
    if useGPSObstacleAvoidance:
        for i in range(Constant.NUM_TEAM_MEMBER):
            if Global.teamPlayers[i].getCounter() > 0 and \
               i != Global.myPlayerNum - 1:
                teamX = Global.teammatesLoc[i].getPos()[0]
                teamY = Global.teammatesLoc[i].getPos()[1]
                
                # Now get the distance between me and this gps teammate.
                distance = hMath.getDistanceBetween(Global.selfLoc.getPos()[0], \
                           Global.selfLoc.getPos()[1], teamX, teamY)
         
                # Want to check heading?
                if distance >= 60:
                    if Debug.readyDebug:
                        print "Skip a gps teammate because it is too far away!"
                    continue
                    
                if Debug.readyDebug:
                    print "***************************** There is a teammate I want to use GPS backof !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"

                Global.myRole = Constant.SUPPORTER
                
                forward, turnCCW, left = calculateTheVector(teamX, teamY)
##~                 Action.finalValues[Action.Forward] = Action.finalValues[Action.Forward] + forward
##~                 Action.finalValues[Action.TurnCCW] = Action.finalValues[Action.TurnCCW] + turnCCW
                #Action.finalValues[Action.Left] = Action.finalValues[Action.Left] + left

    if Debug.readyDebug:
        print "Forward: ", Action.finalValues[Action.Forward]
        print "Left:    ", Action.finalValues[Action.Left]
        print "TurnCCW: ", Action.finalValues[Action.TurnCCW]
                
def calculateTheVector(teamX, teamY):
    forward, turnCCW, left = [0, 0, 0]
    selfX = Global.selfLoc.getPos()[0]
    selfY = Global.selfLoc.getPos()[1]
    distX = abs(selfX - teamX)
    distY = abs(selfY - teamY)
    
    # Avoid division by zero.
    if distX == 0:
        distX = 0.01
    if distY == 0:
        distY = 0.01

    tooNearLeftEdge  = selfX <= 30
    tooNearRightEdge = abs(selfX - Constant.FIELD_WIDTH) <= 30

    # So my teammates is right of me, I should go a bit left if possible.
    if teamX >= selfX:
        # If I am not very near the left edge already.
        if not(tooNearLeftEdge and tooNearRightEdge):
            # Now if I am facing the own side. I need to walk to my right.
            if Global.selfLoc.getHeading() <= 360 and \
                Global.selfLoc.getHeading() >= 180:
                left = left + (-1 * getTheInverseSquare(distX))
                if Debug.readyDebug:
                    print "I go right side because something left of me!"
            else:
                left = left + getTheInverseSquare(distX)
                if Debug.readyDebug:
                    print "I go left side because something right of me!"
        else:
            if Debug.readyDebug:
                print "Near edge case (1)!"
                
    # Case: teamX < selfX.
    else:
        # If I am not very near the right edge already
        if not(tooNearLeftEdge and tooNearRightEdge):
            # Now if I am facing the own side.
            if Global.selfLoc.getHeading() <= 360 and \
                Global.selfLoc.getHeading() >= 180:
                left = left + (getTheInverseSquare(distX))
                if Debug.readyDebug:
                    print "I go left side because something right of me! (2)"
            else:
                left = left + (-1 * getTheInverseSquare(distX))
                if Debug.readyDebug:
                    print "I go right side because soemthing left of me! (2)"
        else:
            if Debug.readyDebug:
                print "Near edge case (2)!"
      
    #  Maye need to come back later for the forward speed.
       
    # So my teammates is in front of me, I should go a bit backward if possible.
    if teamY >= selfY:
        # If I am not very near the target goal edge already.
        if (selfY >= 30):
            forward = forward + getTheInverseSquare(distY)
        else:
            if Debug.readyDebug:
                print "I speed up my forward speed, because I am near the bottom (own side) edge!"
    else:
        # If I am not very near the target side edge already.
        if (abs(selfY - Constant.FIELD_LENGTH) >= 30):
            forward = forward + (-1 * (getTheInverseSquare(distY) / 2.0))
        else:
            if Debug.readyDebug:
                print "I can't go backward anymore, because I am near the target side!"
            
    # Forward speed clipping.
    if forward > 10:
        forward = 10
        
    if forward < -6:
        forward = -6
            
    return [forward, turnCCW, left]

def anyGPSTeammateBlockingMyPath():
    for i in range(Constant.NUM_TEAM_MEMBER):
        if Global.teamPlayers[i].getCounter() > 0 and \
           i != Global.myPlayerNum - 1:
            distance = hMath.getDistanceBetween(Global.selfLoc.getPos()[0], \
                       Global.selfLoc.getPos()[1], Global.teammatesLoc[i].getPos()[0], \
                       Global.teammatesLoc[i].getPos()[1])
            if distance <= 90:
                return True
            else:
                if Debug.readyDebug:
                    print " ???????????? AnyGPS distance: ", distance
                
    return False

def anyGPSOpponentBlockingMyPath():
    return False
    
def anyVisionRobotBlockingMyPath():
    return False

def anyGPSRobotBlockingMyPath():
    return anyGPSTeammateBlockingMyPath() or anyGPSOpponentBlockingMyPath()
    
# The two invest realted variables are: distance and vector to the opposite direction.
# That is: x = c / d, where d is the distance, c is a constant, x is a side vector pointing
# directly opposite of the side where the dist is calculated.
def getTheInverseSquare(dist):
    return alphaInvereseSquare / dist
    
# This function is an attempt for the robot to move to its position directly,
# ie. straight line.  Basically this function draws only a straight vector to
# the kickoff position.
def moveToMyPositionDirectly():
    
    # Firstly get my kickoff position and return a fatal warning if there is none.
    if Global.kickOffPos == None:
        print "Warning: pReady.py - directPositionToMyposition() no kick off position?"
        Debug.goAndMakeTraceBack()
        return False
        
    selfX = Global.selfLoc.getPos()[0]
    selfY = Global.selfLoc.getPos()[1]
    kickX = Global.kickOffPos[0]
    kickY = Global.kickOffPos[1]
        
    # Now I am sure I have the kickoff position, then I should run to it directly.
    # Calculate the distance, heading to the kickoff position.
    distance = hMath.getDistanceBetween(selfX, selfY, kickX, kickY)
    heading  = hMath.getHeadingToFaceAt(Global.selfLoc, kickX, kickY)
    
    if Debug.directPosDebug:
        print "Distance: %f" %distance
        print "Heading: %f"  %heading
    
    # If the distance is small, do saGoToTargetFacingHeading(). With such small
    # distance, better go and turn toward the target goal.
    if distance <= 50:
        if Debug.readyDebug:
            print "Distance too small case!", Global.selfLoc.getPos(), \
                    Global.kickOffPos
        hTrack.saGoToTargetFacingHeading(kickX, kickY, 90, 12, 50)
        return False
        
    # If the heading is too large, it would be better to stationary and turn.
    if abs(heading) >= 30:
        if Debug.readyDebug:
            print "Heading too large case!", heading
        Action.walk(0, 0, hMath.CLIP(heading, 60))
        return False
        
    # Should this or / and sign?
    if not anyVisionRobotBlockingMyPath() and not anyGPSRobotBlockingMyPath() :
        if Debug.readyDebug:
            print "No robot block my path, lets go as quickly as possible!"
        Action.walk(Action.MAX_FORWARD, 0, 0)
        return False

    # Now the heading is not too large, draw a vector directly to the kickoff
    # position.
    Action.walk(Action.MAX_FORWARD, 0, 0)
    return True

def shouldITurnTowardTheFieldFromOwnGoal():
    global getOutOfGoalCounter, getOutOfGoalDirection

    ownGoal = Global.vOGoal
    turnCCW = -1
    
    # If we can see our own goal real big or we've got our face in either goal
    # or we're locked into a get-out-of-goal routine.
    if ((ownGoal.getConfidence() > 0 and (ownGoal.getWidth() >  \
        Constant.WIDTH_MOVE_THRESHOLD or (ownGoal.getHeight() > \
        Constant.HEIGHT_MOVE_THRESHOLD and ownGoal.getWidth() > \
        Constant.IMAGE_WIDTH / 2))) or getOutOfGoalCounter > 0):
        if getOutOfGoalDirection == None:
            if ownGoal.getHeading() >= 0:
                turnCCW = -Action.MAX_TURN_NORMAL/2
            else:
                turnCCW = Action.MAX_TURN_NORMAL/2
            getOutOfGoalDirection = turnCCW
        else:
            turnCCW = getOutOfGoalDirection
                        
        Action.walk(0, 0, turnCCW)
        Action.setHeadParams(0, 40, 0, Action.HTAbs_h)
        getOutOfGoalCounter += 1
        
        MAX_INFRA_RED = 90.0
        irFar = VisionLink.getAnySensor(Constant.ssINFRARED_FAR)
        
        if getOutOfGoalCounter > 60 or Global.vTGoal.getConfidence() > 0 or \
            (irFar / 10000.0) >= MAX_INFRA_RED or Global.seenBeacon:
                getOutOfGoalCounter = 0
        return True
    else:
        getOutOfGoalDirection = None
        return False

def shouldITurnTowardTheFieldFromTargetGoal():
    global getOutOfGoalCounter, getOutOfGoalDirection

    targetGoal = Global.vTGoal
    turnCCW    = -1
    
    # If we can see our target goal real big or we've got our face in either goal or we're locked
    # into a get-out-of-goal routine.
    if ((targetGoal.getConfidence() > 0 and (targetGoal.getWidth() >  \
        Constant.WIDTH_MOVE_THRESHOLD or (targetGoal.getHeight() > \
        Constant.HEIGHT_MOVE_THRESHOLD and targetGoal.getWidth() > \
        Constant.IMAGE_WIDTH / 2))) or getOutOfGoalCounter > 0):
        if getOutOfGoalDirection == None:
            if targetGoal.getHeading() >= 0:
                turnCCW = -Action.MAX_TURN_NORMAL/2
            else:
                turnCCW = Action.MAX_TURN_NORMAL/2
            getOutOfGoalDirection = turnCCW
        else:
            turnCCW = getOutOfGoalDirection
                        
        Action.walk(0, 0, turnCCW)
        Action.setHeadParams(0, 40, 0, Action.HTAbs_h)
        getOutOfGoalCounter = getOutOfGoalCounter + 1
        
        MAX_INFRA_RED = 90.0
        irFar = VisionLink.getAnySensor(Constant.ssINFRARED_FAR)
        
        if getOutOfGoalCounter > 60 or Global.vOGoal.getConfidence() > 0 or \
            (irFar / 10000.0) >= MAX_INFRA_RED or Global.seenBeacon:
                getOutOfGoalCounter = 0
        return True
    else:
        getOutOfGoalDirection = None
        return False
        
def shouldITurnTowardTheField():  
    if shouldITurnTowardTheFieldFromOwnGoal() or \
       shouldITurnTowardTheFieldFromTargetGoal():
        return True
    else:
        return False


#----------------------------------------------------------------------------
# Convenience functions for kickoff positions

# True if this is a wide receiving position
def isWide(pos):
    return pos == P_REC_LEFT_WIDE or pos == P_REC_RIGHT_WIDE

# True if this is a close receiving position
def isClose(pos):
    return pos == P_REC_LEFT_CLOSE or pos == P_REC_RIGHT_CLOSE

# True if this is a forward of beacons kickoff position
def isForward(pos):
    return pos == P_KICK_LEFT_FORWARD or pos == P_KICK_RIGHT_FORWARD

# True if this is a behind beacons kickoff position
def isRear(pos):
    return pos == P_KICK_LEFT_BACK or pos == P_KICK_RIGHT_BACK
    
# True if this is a central receiving position or kickoff position
def isCentre(pos):
    return pos == P_KICK_KICKER or pos == P_REC_CENTRE
    
