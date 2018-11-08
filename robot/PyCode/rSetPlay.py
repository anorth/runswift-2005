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


# Set plays are special choreographed actions taken by the whole team, for
# example a kickoff

import Constant
import Global
import VisionLink
import Indicator
import Action
import hTrack
import hFWHead
import hMath

import pReady
import pSet
import rAttacker
#import rSupporter
#import rWinger
import rDefender
#import sDribble
import sFindBall
import sGrab
#import sGrabSideWalk
import sGrabTurnKick
import sGrabDribble
import sDodgyDog
import sUpennKick
import sBlock

# The set play function to execute. If this is not None then it will be called
# every frame. Execute a setPlay by setting this to one of the functions below
setPlay = None

# Global play counter can be used by roles that run for a set period of time.
# This is incremented automatically by perform().
playCounter = 0

# Timestamp for when we grab, used by many set plays
whenGrabbed = 0

# Constants for finding gaps in the defending lineup
GAP_MIN = 10           # Min gap size (degrees)
GAP_MIN_INTENSITY = 30 # Min obs intensity to be considered
GAP_MIN_DIST = 50      # Min obs distance to be considered
GAP_MAX_DIST = 200

# Execute a set play. Returns True if a set play is executing, False if there
# isn't one to execute
def perform(params = None):
    global setPlay, playCounter
    id(params) # ignore

    # Check for set plays to execute
    if setPlay == None:
        if pSet.doKickOff:
            if VisionLink.getKickOffTeam() == VisionLink.getTeamColor():
                setPlay = selectKickOffSetPlay()
            else:
                #setPlay = receiveKickOffCharge
                setPlay = receiveKickOffPincer
                #setPlay = receiveKickOffEchelon
            pSet.doKickOff = False

        if setPlay != None:
            print "executing set play:", setPlay

    if setPlay != None:
        playCounter += 1
        Indicator.showRedEye(True, True)
        stillGoing = setPlay()
        if not stillGoing:
            print "Set play complete"
            sGrab.resetPerform()
            rAttacker.doKickReset()
            setPlay = None
            return False
        return True

    Indicator.showRedEye(False, False)
    resetPerform()
    return False

def resetPerform():
    global setPlay, playCounter, whenGrabbed
    global bestHeading, gBestHeading
    setPlay = None
    playCounter = whenGrabbed = 0
    bestHeading = gBestHeading = None

#############################################################################
# Specific set plays below here. Must be methods taking no arguments suitable
# for calling from perform(). Set plays *must* determine when they are finished
# and return False. They must return True if they are still executing.

#---------------------------------------------------------------------------
# Recieve kickoff set plays


# Kickoff set play when we are defending. Two dogs rush the ball and
# the third supports. Executes just long enough for the rushing dogs to almost
# reach the ball
def receiveKickOffCharge():
    global playCounter

    if playCounter > 85:
        return False

    # The close player stays
    if pReady.isClose(Global.kickOffPos):
        if playCounter > 65:
            return False
        rDefender.perform()
        return True

    # Others charge
    hFWHead.DecideNextAction()
    sFindBall.walkToBall(Global.ballD, Global.ballH)
    return True

# Kickoff set play when we are defending. Two outside dogs rush the ball and
# the centre defends.
def receiveKickOffPincer():
    global playCounter

    if playCounter > 85:
        return False

    if pReady.isCentre(Global.kickOffPos):
        if playCounter < 65:
            Global.myRole = Global.myLastRole = Constant.DEFENDER
            if sBlock.checkThenBlock(True, 3.0, bothSides = True):
                return
            sFindBall.perform(True)
            hTrack.saGoToTargetFacingHeading(Constant.FIELD_WIDTH/2,
                                            Constant.FIELD_LENGTH * 0.29,
                                            90,
                                            maxSpeed = 20)
            #rDefender.DecideNextAction()
            #Action.walk(0.01, 0, 0) # don't go anywhere
            return True
        return False

    # Wide players
    hFWHead.DecideNextAction()
    sFindBall.walkToBall(Global.ballD, Global.ballH)
    return True

# Kickoff set play when we are defending. Centre dog rushes the ball. Side dogs
# attack iff there is a dog near halfway on your side of the field.
# Not fully working.
def receiveKickOffEchelon():
    global playCounter

    if playCounter > 85:
        return False

    # Centre always charges
    if pReady.isCentre(Global.kickOffPos):
        hFWHead.DecideNextAction()
        sFindBall.walkToBall(Global.ballD, Global.ballH)
        return True

    # Wide players charge iff obstacle

    # Count obstacles forward on our side of the field
    if Global.selfLoc.getX() > Constant.FIELD_WIDTH/2:
        tlx = Constant.FIELD_WIDTH/2 + 40
        brx = Constant.FIELD_WIDTH - 40
    else:
        tlx = 40
        brx = Constant.FIELD_WIDTH/2 - 40
    tly = Constant.FIELD_LENGTH/2 + 50
    bry = Constant.FIELD_LENGTH/2 - 100

    nobs = VisionLink.getNoObstacleInBox(tlx, tly, brx, bry,
                                         20, Constant.OBS_USE_SHARED)
    hFWHead.DecideNextAction()
    #print "nobs:", nobs
    if nobs < 40:   # no opp == defend
        rDefender.DecideNextAction()
    else:           # opp = charge
        sFindBall.walkToBall(Global.ballD, Global.ballH)

    return True

#---------------------------------------------------------------------------
# Perform kickoff set plays

# Target position when pushing though gap, set by selectKickOffSetPlay
PUSH_TO_X = None
PUSH_TO_Y = None

bestHeading = None
gBestHeading = None

# Chooses a kickoff set play to perform based on opponent positions
def selectKickOffSetPlay():
    global PUSH_TO_X, PUSH_TO_Y

    if pReady.chosenFormation == 0:
        print "chosenFormation =", pReady.chosenFormation, "- going right"
        (PUSH_TO_X, PUSH_TO_Y) = (Constant.MAX_GOALBOX_EDGE_X, 
                            Constant.TOP_GOALBOX_EDGE_Y)
    else:
        print "chosenFormation =", pReady.chosenFormation, "- going left"
        (PUSH_TO_X, PUSH_TO_Y) = (Constant.MIN_GOALBOX_EDGE_X, 
                            Constant.TOP_GOALBOX_EDGE_Y)

    # Calculate obstacles between us and the goal. If zero we'll shoot
#    nobs = getObstaclesToGoal()
#    if nobs == 0:
#        print "Clear shot to goal! Shooting"
#        return kickOffShootGoal

    # Calculate the gap to the push target. If this is large enough then
    # we'll use that set play. If it's small we'll kick around instead
    gap = getGapTo(PUSH_TO_X, PUSH_TO_Y)
    if gap != None:
        (left, right, best, gapSize) = gap
    if gap != None and gapSize >= 14:
        print "large gap", gap, "- pushing to supporter"
        return kickOffPushToSupporter
    else:
        print "small gap", gap, "- kicking to corner"
        return kickOffKickToCorner

    #setPlay = kickOffShootGoal
    #setPlay = kickOffPushToSupporter
    #setPlay = kickOffUPenn
    #setPlay = kickOffKickToGap
    #setPlay = kickOffKickToCorner
    #return kickOffPushToSupporter

# Kickoff set play when we are kicking off. The kicker grabs and
# chest-pushes for a gap in the opposing team position on the side where
# the supporter is forward. The supporter runs around the other
# team and shoots a brilliant goal
def kickOffPushToSupporter():
    global playCounter, whenGrabbed
    global bestHeading, gBestHeading

    (myX, myY) = Global.selfLoc.getPos()
    myH = Global.selfLoc.getHeading()

    if pReady.chosenFormation == 0:
        (suppX, suppY) = (PUSH_TO_X + 40, PUSH_TO_Y - 40)
    else:
        (suppX, suppY) = (PUSH_TO_X - 40, PUSH_TO_Y - 40)

    if pReady.isCentre(Global.kickOffPos):
        if playCounter > 180:   # 6 sec, just in case
            return False

        if whenGrabbed == 0:
            #print "grabbing"
            if sGrab.grabbingCount == 0:
                # Find the obstacle gap just before we drop the head to grab
                #(gapLeft, gapRight, bestHeading, angle) = \
                #    VisionLink.getBestGap(lKickX, lKickY,
                #                            GAP_MAX_DIST,GAP_MIN_DIST,
                #                            GAP_MIN,GAP_MIN_INTENSITY,
                #                            Constant.OBS_USE_SHARED)    
                gap = getGapTo(PUSH_TO_X, PUSH_TO_Y)
                if gap != None:
                    (gapLeft, gapRight, bestHeading, angle) = gap
                else:
                    bestHeading = hMath.getHeadingToAbsolute(Global.selfLoc,
                                                           PUSH_TO_X, PUSH_TO_Y)
                gBestHeading = hMath.local2GlobalHeading(Global.selfLoc,
                                                        bestHeading)
                print "gap ", bestHeading, gBestHeading
            if sGrab.perform() == Constant.STATE_FAILED:
                return False
            if sGrab.isGrabbed:
                #print "grabbed, best gap at local", bestHeading, \
                #        "global", gBestHeading
                whenGrabbed = playCounter
            return True

        # Don't prevent other forward from being attacker
        Global.myRole = Global.myLastRole = Constant.SUPPORTER

        relH = gBestHeading - Global.selfLoc.getHeading()
        if playCounter - whenGrabbed < 40:
            if sGrabDribble.perform(Action.MAX_FORWARD, 0, relH) \
                    == Constant.STATE_FAILED:
                return False
            return True

        Action.kick(Action.GTChestPushWT)
        if Action.shouldIContinueKick():
            Action.continueKick()
            return True

        sGrab.resetPerform()
        return False

    elif pReady.isForward(Global.kickOffPos):
        if playCounter < 80:
            sDodgyDog.dodgyDogTo(suppX, suppY)
            hTrack.stationaryLocalise(6, 30, -30)
            return True
        elif playCounter < 120:
            if hTrack.canSeeBall and Global.ballD < 30:
                Global.myRole = Global.myLastRole = Constant.ATTACKER
                return False
            h = hMath.normalizeAngle_0_360(myH + Global.ballH)
            hTrack.saGoToTargetFacingHeading(suppX, suppY, h)
            hFWHead.DecideNextAction()
            return True
        else:
            Global.myRole = Global.myLastRole = Constant.ATTACKER
            return False
    else:
        return False

    return False

# Kickoff set play when we are kicking off. The kicker grabs and
# kicks for the far corner where the supporter is forward. The supporter runs
# around the other team and shoots a brilliant goal
def kickOffKickToCorner():
    global playCounter, whenGrabbed
    global bestHeading, gBestHeading

    # Warning: this set play freezes the attacker when it fails.
    # Don't use it yet

    (myX, myY) = Global.selfLoc.getPos()
    myH = Global.selfLoc.getHeading()

    if pReady.chosenFormation == 0:
        (kickX, kickY) = (Constant.FIELD_WIDTH, Constant.FIELD_LENGTH - 80)
        (suppX, suppY) = (kickX + 40, kickY - 40)
    else:
        (kickX, kickY) = (0, Constant.FIELD_LENGTH - 80)
        (suppX, suppY) = (kickX - 40, kickY - 40)

    if pReady.isCentre(Global.kickOffPos):
        print "kickToCorner: whenGrabbed", whenGrabbed,
        if playCounter > 120:   # 6 sec, just in case
            return False

        if whenGrabbed == 0:
            if sGrab.grabbingCount == 0:
                # Find the obstacle gap just before we drop the head to grab
                #(gapLeft, gapRight, bestHeading, angle) = \
                #    VisionLink.getBestGap(lKickX, lKickY,
                #                            GAP_MAX_DIST,GAP_MIN_DIST,
                #                            GAP_MIN,GAP_MIN_INTENSITY,
                #                            Constant.OBS_USE_SHARED)    
                gap = getGapTo(kickX, kickY)
                if gap != None:
                    (gapLeft, gapRight, bestHeading, angle) = gap
                    bestHeading += hMath.getSign(5 , bestHeading)
                else:
                    bestHeading = hMath.getHeadingToAbsolute(Global.selfLoc,
                                                            kickX, kickY)

                gBestHeading = hMath.local2GlobalHeading(Global.selfLoc,
                                                        bestHeading)
                print "gap", bestHeading, gBestHeading,
            print "grabbing"
            if sGrab.perform() == Constant.STATE_FAILED:
                print "Grab failed in set play. Done."
                sGrab.resetPerform()
                Global.myRole = Global.myLastRole = Constant.ATTACKER
                return False
            if sGrab.isGrabbed:
                print "grabbed, best gap at local", bestHeading, \
                        "global", gBestHeading
                whenGrabbed = playCounter
            return True

        # Don't prevent other forward from being attacker
        Global.myRole = Global.myLastRole = Constant.SUPPORTER

        # Keep updating gap, but not with shared obs
        gap = getGapTo(kickX, kickY)
        if gap != None:
            (gapLeft, gapRight, bestHeading, angle) = gap
            bestHeading += hMath.getSign(5 , bestHeading)
            gBestHeading = hMath.local2GlobalHeading(Global.selfLoc,
                                                    bestHeading)
            print "New gap global", gBestHeading,
        else:
            print "No gap left - using last global", gBestHeading,
            #bestHeading = 0 # Kick now
        relH = gBestHeading - Global.selfLoc.getHeading()
        print "relH", relH,
        if abs(relH) >= 4 and (playCounter - whenGrabbed) < 55:
            print "dribbling"
            if sGrabDribble.perform(0, 0, relH) == Constant.STATE_FAILED:
                print "dribble failed. Done."
                Global.myRole = Global.myLastRole = Constant.ATTACKER
                return False
            return True

        print "Kicking",
        Action.kick(Action.SoftTapWT)
        if Action.shouldIContinueKick():
            Action.continueKick()
            return True

        sGrab.resetPerform()
        print "success. Done."
        return False

    elif pReady.isForward(Global.kickOffPos):
        Global.myRole = Global.myLastRole = Constant.ATTACKER
        if playCounter < 60:
            hTrack.saGoToTarget(suppX, suppY, 25)
            hFWHead.DecideNextAction()
            return True
        elif playCounter < 100:
            if hTrack.canSeeBall and Global.ballD < 30:
                return False
            h = hMath.normalizeAngle_0_360(myH + Global.ballH)
            hTrack.saGoToTargetFacingHeading(suppX, suppY, h)
            hFWHead.DecideNextAction()
            return True
        else:
            rAttacker.perform()
            return False
    else:
        return False

    return False

# Kickoff set play when we are kicking off. The kicker grabs and
# kicks for a gap in the opposing team position on the side where
# the supporter is forward. The supporter runs around the other
# team and shoots a brilliant goal
def kickOffKickToGap():
    global playCounter, whenGrabbed
    global bestHeading, gBestHeading

    (myX, myY) = Global.selfLoc.getPos()
    myH = Global.selfLoc.getHeading()

    if pReady.chosenFormation == 0:
        print "chosenFormation =", pReady.chosenFormation, "- going right"
        (kickX, kickY) = (Constant.MAX_GOALBOX_EDGE_X, 
                            Constant.TOP_GOALBOX_EDGE_Y)
        (suppX, suppY) = (kickX + 40, kickY - 40)
        (lKickX, lKickY) = hMath.getLocalCoordinate(myX, myY, myH, kickX, kickY)
    else:
        print "chosenFormation =", pReady.chosenFormation, "- going left"
        (kickX, kickY) = (Constant.MIN_GOALBOX_EDGE_X, 
                            Constant.TOP_GOALBOX_EDGE_Y)
        (suppX, suppY) = (kickX - 40, kickY - 40)
        (lKickX, lKickY) = hMath.getLocalCoordinate(myX, myY, myH, kickX, kickY)

    if pReady.isCentre(Global.kickOffPos):
        if playCounter > 120:   # 6 sec, just in case
            return False

        if whenGrabbed == 0:
            #print "grabbing"
            if sGrab.grabbingCount == 0:
                # Find the obstacle gap just before we drop the head to grab
                (gapLeft, gapRight, bestHeading, angle) = \
                    VisionLink.getBestGap(lKickX, lKickY,
                                            GAP_MAX_DIST,GAP_MIN_DIST,
                                            GAP_MIN,GAP_MIN_INTENSITY,
                                            Constant.OBS_USE_SHARED)    
                gBestHeading = hMath.local2GlobalHeading(Global.selfLoc,
                                                        bestHeading)
                #gBestHeading = hMath.getHeadingBetween(Constant.FIELD_WIDTH/2, Constant.FIELD_LENGTH/2, kickX, kickY)
                print "local target", lKickX, lKickY
                print "gap", bestHeading, gBestHeading
            if sGrab.perform() == Constant.STATE_FAILED:
                return False
            if sGrab.isGrabbed:
                print "grabbed, best gap at local", bestHeading, \
                        "global", gBestHeading
                whenGrabbed = playCounter
            return True

        # Don't prevent other forward from being attacker
        Global.myRole = Global.myLastRole = Constant.SUPPORTER

        relH = gBestHeading - Global.selfLoc.getHeading()
        if abs(relH) >= 3:
            if sGrabDribble.perform(0, 0, relH) == Constant.STATE_FAILED:
                return False
            return True

        Action.kick(Action.SoftTapWT)
        if Action.shouldIContinueKick():
            Action.continueKick()
            return True

        sGrab.resetPerform()
        return False

    elif pReady.isForward(Global.kickOffPos):
        if playCounter < 80:
            sDodgyDog.dodgyDogTo(suppX, suppY)
            hTrack.stationaryLocalise(6, 30, -30)
            return True
        elif playCounter < 120:
            if hTrack.canSeeBall and Global.ballD < 30:
                Global.myRole = Global.myLastRole = Constant.ATTACKER
                return False
            h = hMath.normalizeAngle_0_360(myH + Global.ballH)
            hTrack.saGoToTargetFacingHeading(suppX, suppY, h)
            hFWHead.DecideNextAction()
            return True
        else:
            Global.myRole = Global.myLastRole = Constant.ATTACKER
            return False
    else:
        return False

    return False


# Kickoff set play when we are kicking off. The kicker grabs, sidesteps and
# shoots for goal. The supporter (forward dog) charges the goal. The
# winger does normal winger role (defense)
def kickOffShootGoal():
    global playCounter, whenGrabbed

    if pReady.isCentre(Global.kickOffPos):
        if playCounter > 150:
            sGrab.resetPerform()
            return False

        if whenGrabbed == 0:
            #print "grabbing"
            sGrab.perform()
            if sGrab.isGrabbed:
                #print "grabbed"
                whenGrabbed = playCounter
            return True

        if playCounter < whenGrabbed + 40:
            #print "walking"
            sGrabDribble(0,Action.MAX_LEFT,0)
            return True

        # not aiming to visual goal
        if sGrabTurnKick.perform(False) == Constant.STATE_SUCCESS:
            #print "aiming"
            sGrab.resetPerform()
            return False

    elif pReady.isForward(Global.kickOffPos):
        if playCounter > 40:
            return False
        hTrack.saGoToTarget(Constant.TARGET_GOAL_X, Constant.TARGET_GOAL_Y)
    else:
        return False

    hFWHead.DecideNextAction()
    return True


# Kickoff set play when we are kicking off. The kicker does a UPenn kick
# to the supporter. Supporter runs forward. This set play sucks.
def kickOffUPenn():
    global playCounter, whenGrabbed

    if pReady.chosenFormation == 0:
        kickLeft = False
    else:
        kickLeft = True

    if pReady.isCentre(Global.kickOffPos):
        if playCounter > 150:
            return False

        if whenGrabbed == 0:
            print "grabbing"
            if sGrab.perform() == Constant.STATE_FAILED:
                return False
            if sGrab.isGrabbed:
                whenGrabbed = playCounter
            return True

        print "Grabbed!"
        if sUpennKick.perform(kickLeft) == Constant.STATE_SUCCESS:
            sGrab.resetPerform()
            return False

        sFindBall.perform(True)
        return True

    elif pReady.isForward(Global.kickOffPos):
        if playCounter > 40:
            return False
        hTrack.saGoToTarget(Constant.TARGET_GOAL_X, Constant.TARGET_GOAL_Y)
        hFWHead.DecideNextAction()
    else:
        return False

    return True

#----------------------------------------------------------------------------
# Helper functions


# Finds the best gap to the specified position on the field (global) using
# shared obstacles. Returns (gapLeft, gapRight, bestHeading, width)
def getGapTo(targetX, targetY):
    (lTargetX, lTargetY) = hMath.getLocalCoordinate(Global.selfLoc.getX(),
                                                    Global.selfLoc.getY(),
                                                    Global.selfLoc.getHeading(),
                                                    targetX, targetY)
    gap = VisionLink.getBestGap(lTargetX, lTargetY,
                                GAP_MAX_DIST,GAP_MIN_DIST,
                                GAP_MIN,GAP_MIN_INTENSITY,
                                Constant.OBS_USE_SHARED)
    # getBestGap currently has some wierdness, so try to filter it here
    if gap[2] > gap[0] and gap[2] < gap[1]:
        return gap
    return None
    
# Returns the number of obstacles between the field centre and just in front
# of the goal. Relaxed obstacle thresholds since we want to be sure.
# WARNING: zero here doesn't mean there are no robots between you and the goal
# (unfortunately)
def getObstaclesToGoal():
    (lGoalX, lGoalY) = hMath.getLocalCoordinate(Global.selfLoc.getX(),
                                                Global.selfLoc.getY(),
                                                Global.selfLoc.getHeading(),
                                                Constant.TARGET_GOAL_X,
                                                Constant.TARGET_GOAL_Y - 30)
    (lFromX, lFromY) = hMath.getLocalCoordinate(Global.selfLoc.getX(),
                                                Global.selfLoc.getY(),
                                                Global.selfLoc.getHeading(),
                                                Constant.FIELD_WIDTH/2,
                                                Constant.FIELD_LENGTH/2 + 20)
    return VisionLink.getNoObstacleBetween(lFromX, lFromY,
                                          lGoalX, lGoalY,
                                          Constant.GOAL_WIDTH, 0,
                                          5,
                                          Constant.OBS_USE_SHARED)
