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
#  $Id: BackOffAnand.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#   
#   Vishwanathan Anand.
#
#   Contains function to decide if player should backoff, and actually how to 
#   carry out the backoff.
#
#===============================================================================

import Action
import Constant
import Debug
#import sGetBehindBall
import Global
import hFWHead
import hMath
import hTeam
#import hWhere
#import Indicator
#import rSupporter
#import rAttacker




# Calculated using vTeammate in backoffBecozTeammateHasBall
angleTeammateToGoal     = None 
angleMeToGoal           = None 
matchedTmPlayerNumber   = None

# Info got from backoffBecozTeammateHasBall() for other functions to use.
teammateX      = 0 # Teammate's x,y,h on field using visual info.
teammateY      = 0
teammateH      = 0 # h is angle of the teammate relative to my neck base.

# Used to decide whether a dog is in attack region.
BELOW_BALL_THRESHOLD    = 125
ABOVE_BALL_THRESHOLD    = 75

# For get behind backoff
#GetBehindBackOffCounter     = 0
##~ GetBehindDir             = Constant.dCLOCKWISE
##~ GetBehindAngle           = 0
StillGettingBehind      = False


##-------------------
BACKOFF_TIMER = 10
gBackOffTimer = 0

FACE_BACKOFF = 0 
SIDE_BACKOFF = 1
GRAB_BACKOFF = 2
gBackOffType = None


def shouldIStart():
    global gBackOffType
    
    if not Global.vBall.isVisible()\
        and Global.lostBall > 5: 
        return False

    backOffNum = hTeam.countBackOff()
    if backOffNum > 0: 
        return False 
        
    if hTeam.hasTeammateGrabbedBall():
        if Global.ballD > 20: 
            return False   
        else:
            gBackOffType = GRAB_BACKOFF
            resetAction()
            return True
            
    if shouldIWirelessBackOff()\
        or shouldIGpsBackOff():
        
        resetAction()
        return True
        
    return False
          



def shouldIGpsBackOff(): 
    return False


def shouldIWirelessBackOff(): 
    global gBackOffType    
    
    BALL_DISTANCE = 50
    
    ## If you receive wireless information of mate being closer to the 
    ## ball than you and you are behind him
    selfX, selfY = Global.selfLoc.getX(), Global.selfLoc.getY()
    for i in range(Constant.NUM_TEAM_MEMBER):
        mate = Global.teamPlayers[i]
        if i != Global.myPlayerNum - 1\
            and mate.isValid()\
            and not mate.isGoalie()\
            and mate.hasSeenBall()\
            and Global.ballD > mate.getBallDist()\
            and mate.getBallDist() < BALL_DISTANCE:
            #and Global.ballD < BALL_DISTANCE + 5 
            
            mateX, mateY = Global.teammatesLoc[i].getX(), Global.teammatesLoc[i].getY()
            
            ## Compare headings of yourself and your mate from the 
            ## ball to tell whether you two are near by.
            selfH2ball = hMath.normalizeAngle_0_360(\
                            hMath.getHeadingBetween(Global.ballX,Global.ballY,selfX,selfY))
            mateH2ball = hMath.normalizeAngle_0_360(\
                            hMath.getHeadingBetween(Global.ballX,Global.ballY,mateX,mateY))
     
            selfD2mate = hMath.getDistanceBetween(selfX,selfY,mateX,mateY)
                           
            
            if abs(selfH2ball - mateH2ball) < 50: 
               
                if selfD2mate < 5\
                    and i > Global.myPlayerNum - 1:
                    return True
                                   
                elif selfD2mate < 20:
                    gBackOffType = SIDE_BACKOFF
                    if Debug.backOffTriggerDebug: 
                        print "shouldIWirelessBackOff(): True, Side backoff"
                    return True

            if abs(selfH2ball - mateH2ball) < 70:
            
                if selfD2mate < 5\
                    and i > Global.myPlayerNum - 1:
                    return True
                
                elif selfD2mate < 30:
                    gBackOffType = FACE_BACKOFF                 
                    if Debug.backOffTriggerDebug: 
                        print "shouldIWirelessBackOff(): True, Face backoff"
                    return True
                        
    return False
    

    
def shouldIEnd():
    global gBackOffTimer
    
    backOffNum = hTeam.countBackOff()
    if backOffNum > 0: 
        for i in range(Constant.NUM_TEAM_MEMBER):
            mate = Global.teamPlayers[i]
            if i != Global.myPlayerNum - 1\
                and mate.isValid()\
                and mate.isBackOff():
                
                if i < Global.myPlayerNum - 1:
                    return True        
    
    if hTeam.hasTeammateGrabbedBall():
        if Global.ballD > 20: 
            return True   
        else:
            return False
     
    if gBackOffTimer > 0: 
        gBackOffTimer -= 1
        return False
    else: 
        return True
    

def resetAction(): 
    global gBackOffTimer
    
    if gBackOffType == FACE_BACKOFF: 
        gBackOffTimer = BACKOFF_TIMER    
    else:
        gBackOffTimer = BACKOFF_TIMER


def DecideNextAction():           
    perform()

def perform():
    hFWHead.compulsoryAction = hFWHead.mustSeeBall
  
    if gBackOffType == FACE_BACKOFF:
        turnccw = Global.ballH
        Action.walk(3,0,turnccw)
    
    elif gBackOffType == SIDE_BACKOFF:        
        turnccw = Global.ballH
        Action.walk(3,0,turnccw)
    
    elif gBackOffType == GRAB_BACKOFF: 
        turnccw = Global.ballH
        Action.walk(-2,0,turnccw)
        
    else: 
        turnccw = Global.ballH
        Action.walk(3,0,turnccw)
     
        
"""
#--------------------------------------
# Get behind ball so that it's easier to see who is attacker/defender.
def beGetBehindAfterBackoff(stepBack):

    global angleMeToGoal, angleTeammateToGoal
    global ABOVE_BALL_THRESHOLD, BELOW_BALL_THRESHOLD

    direction      = None
    radius         = None
        
    # If both dogs are above ball, then check differently.
    if (angleMeToGoal <= ABOVE_BALL_THRESHOLD
            and angleTeammateToGoal <= ABOVE_BALL_THRESHOLD):

        if (teammateH > 0):
            direction = Constant.dANTICLOCKWISE
        else:
            direction = Constant.dCLOCKWISE

    # Otherwise determine getBehind direction using heading to target goal.
    else:
        #if (gps->tGoal().pos.head >= 0):
        if (Global.selfLoc.getX() < Constant.TARGET_GOAL_X):
            direction = Constant.dANTICLOCKWISE
        else:
            direction = Constant.dCLOCKWISE
                      
    # Set the radius of getBehindBall
    if (angleMeToGoal <= ABOVE_BALL_THRESHOLD):
        radius = 70
    elif (angleMeToGoal <= BELOW_BALL_THRESHOLD):
        radius = 60
    else:
        radius = 60

    # -------------------------------------------------
    # Determine the turn direction on different edges
    
    if hWhere.ballOnLEdge():
        direction = Constant.dCLOCKWISE
        
    elif hWhere.ballOnREdge():
        direction = Constant.dANTICLOCKWISE
        
    elif Global.ballY < 70 and Global.ballX < Constant.FIELD_WIDTH/2.0:
        direction = Constant.dCLOCKWISE
        
    elif Global.ballY < 70 and Global.ballX >= Constant.FIELD_WIDTH/2.0:
        direction = Constant.dANTICLOCKWISE
        
    elif hWhere.ballOnTLEdge():
        direction = Constant.dCLOCKWISE
        
    elif hWhere.ballOnTREdge():
        direction = Constant.dANTICLOCKWISE
        
    sGetBehindBall.perform(radius, Global.DKD[0], direction, stepBack, 100)
    
    # Setting this signal should enable the isBackOff signal sent in HelpLong.
    # Basically the isBackOff signal send if myRole is GETBEHINDBACKOFF
    # or SUPPORTBACKOFF.
    Global.myRole = Constant.GETBEHINDBACKOFF
##~     print "Get behind backoff"
    
#--------------------------------------
# Become a support after some decisions made.
def beSupportAfterBackoff():
    rSupporter.DecideNextAction()
    # Overwrite support role.
    Global.myRole = Constant.SUPPORTBACKOFF
##~     print "Support backoff"
    
#--------------------------------------
# Become an attacker after some decisions made.
# This function is directly correspond to the boMaybeNormal.
def beAttackerAfterBackoff():
    global matchedTmPlayerNumber, angleTeammateToGoal
    
    if matchedTmPlayerNumber >= 0:
        matchedTm = Global.teamPlayers[matchedTmPlayerNumber]
        
        if Debug.attackBackOffDebug:
            print "Ball distance: %f" %matchedTm.getBallDist()
            print "isKickedOff: %f"   %matchedTm.isBackOff()
        
        # This is really depend on the trigger condition, come back later.
        if (not matchedTm.isBackOff() and matchedTm.getBallDist() <= 18):
        
            # If you want to be an attacker, but your matched tm is not backing off, is close
            # to the ball, he lines up well for attack, then I would be support.
            if (angleTeammateToGoal >= BELOW_BALL_THRESHOLD):
                beSupportAfterBackoff()
                if Debug.attackBackOffDebug:
                    print "SUPPORT BACKOFF after attack backoff"
            # If he is not lined up, then i get behind / backoff for her.
            else:
                beGetBehindAfterBackoff(True)
                if Debug.attackBackOffDebug:
                    print "GET BEHIND BACKOFF after attack backoff"
            return
            
    if Debug.attackBackOffDebug:
        print "MatchedTM: %f" %matchedTmPlayerNumber
                
    # He is backoffing, quickly attack, or cant find a gps "he".
    rAttacker.DecideNextAction()
    Global.myRole = Constant.ATTACKBACKOFF
    if Debug.attackBackOffDebug:
        print "ATTACK BACKOFF"


   
#--------------------------------------
# Do the back off.
def DecideNextAction():
    global angleTeammateToGoal, angleMeToGoal, \
            BELOW_BALL_THRESHOLD, ABOVE_BALL_THRESHOLD, \
            matchedTmPlayerNumber, teammateH

    global StillGettingBehind
    # Head light won't show unless getBehindBall is choosen at the end, because 
    # Support/Attacker has their own lights that will overwrite this.
    vBall = Global.vBall
    
    
#    global GetBehindBackOffCounter
    
    if StillDoingGetBehindBackoff():
##~         print "Conuter is " , GetBehindBackOffCounter
        beGetBehindAfterBackoff(False)
        StillGettingBehind = True
#        GetBehindBackOffCounter -= 1
        return

    
    for i in range(Constant.NUM_TEAM_MEMBER):
        if (Global.vTeammates[i].getConfidence() > 0 
                and Global.vTeammates[i].getDistance() < 60):

            Action.walk(-6, None, None)
            Global.myRole = Constant.FACEBACKOFF
            return
        
    # Seeing a teammate in front, lost ball for quite a while, then be a support.
    if (Global.lostBall > Constant.VBALL_ANYWHERE):
##~         beGetBehindAfterBackoff(True)
##~         setGetBehindBackoffSate(30)
        beSupportAfterBackoff()
        return

    #--------------------------------------------------------
    # The following is quite a lot of decisions base on the 2 angles above but at the end, it's either
    # an attacker, or
    # a support, or
    # get behind ball to make it easier to choose 1 from above.
    # The dog may or may not see a ball in the following conditions, that's ok.

    # Calculate angles to goal. In 2004, this should use the new dkd.
    angleMeToGoal, angleTeammateToGoal = calculateAnglesToGoal()

    #-- - -- - -- - -- - -- - -- - -- - -- -
    # If your in the attacking region and they are not then attack
    if (angleMeToGoal >= BELOW_BALL_THRESHOLD 
            and angleTeammateToGoal < BELOW_BALL_THRESHOLD):
        beAttackerAfterBackoff()
##~         print "I am in attack region, they arent, attack"
    # In 2003, there is a striker case here. This striker case is different as the one in the main
    # decision tree. The main one is for non-vision ball only, and the one here is designed for
    # vision-ball too. However with somebody's striker modify, the striker trigger in the
    # main tree accept vision ball too. So no point to put a striker condition here.
        
    #-- - -- - -- - -- - -- - -- - -- - -- -
    # If they are in the attacking region and you are not then support
    elif (angleTeammateToGoal >= BELOW_BALL_THRESHOLD 
            and angleMeToGoal  < BELOW_BALL_THRESHOLD):
##~         beSupportAfterBackoff()
        beGetBehindAfterBackoff(False)
        StillGettingBehind = True
##~         setGetBehindBackoffSate(45)
##~         print "they are in attack region, i am not, getbehind"

    #-- - -- - -- - -- - -- - -- - -- - -- -
    # if your both in the above ball region, and your teammate is infront of you
    # get out of the way incase he does a 180 turn kick etc
    # or in case you got it wrong and he actually is in the attacking region.
    #
    # May change this year since no proper variable turn kick at all.
    elif (angleTeammateToGoal <= ABOVE_BALL_THRESHOLD
           and angleMeToGoal <= ABOVE_BALL_THRESHOLD
           and abs(teammateH) < 25):  # 22 from 2003 codes.

##~         beSupportAfterBackoff()
        StillGettingBehind = True
        beGetBehindAfterBackoff(False)
##~         setGetBehindBackoffSate(45)
##~         print "Both in above regions, getbehind"

    #-- - -- - -- - -- - -- - -- - -- - -- -
    # if you are both in the attacking region
    elif (angleMeToGoal >= BELOW_BALL_THRESHOLD 
            and angleTeammateToGoal >= BELOW_BALL_THRESHOLD):

        # If wireless is down or you cant find anyone in gps that matched
        # the teammate you detected in vision
        if (matchedTmPlayerNumber == -1):
            StillGettingBehind = True
            beGetBehindAfterBackoff(False)
##~             setGetBehindBackoffSate(45)
##~             print "both in attack, cant find a match, get behind"

        # If you did actually find a gps match
        else:
            teammate = Global.teamPlayers[matchedTmPlayerNumber]

            # If you can see ball and they cant then attack
            if (vBall.getConfidence() > 0
                    and teammate.getBallDist() >= Constant.LOSTBALL_TIME):
                beAttackerAfterBackoff()
##~                 print "both in attack, teammate cant see ball, attack"

            # If you cant see ball and they can then support
            elif (vBall.getConfidence() == 0 
                    and teammate.getBallDist() < Constant.LOSTBALL_TIME):
##~                 beGetBehindAfterBackoff(True) 
##~                 setGetBehindBackoffSate(30)
                beSupportAfterBackoff()
##~                 print "both in attack, I cant see but teammate can, support"
                    
            # If your alot closer then attack
            elif (teammate.getBallDist() - vBall.getDistance() >= 20):
                beAttackerAfterBackoff()
##~                 print "both in attack, i am closer, attack"

            # if they are alot closer then support
            elif (vBall.getDistance() - teammate.getBallDist() >= 20):
##~                 beGetBehindAfterBackoff(True)
##~                 setGetBehindBackoffSate(30)
                beSupportAfterBackoff()
##~                 print "both in attack, he is closer, support"

            # if your heading is alot better than theirs then attack
            elif (angleMeToGoal - angleTeammateToGoal >= 30):
                beAttackerAfterBackoff()
##~                 print "both in attack, I have better angle, attack"

            # if their heading is alot better than yours then support
            elif (angleTeammateToGoal - angleMeToGoal >= 30):
##~                 beGetBehindAfterBackoff(True)
##~                 setGetBehindBackoffSate(30)
                beSupportAfterBackoff()
##~                 print "both in attack, he has better angle, support"

            # if your player number is smaller than theirs then attack
            elif (Global.myPlayerNum - 1 < matchedTmPlayerNumber):
                beAttackerAfterBackoff()
##~                 print "both in attack, samller player number, attack"
                
            # else your player number is higher than theirs so support
            else:
##~                beGetBehindAfterBackoff(True) 
##~                setGetBehindBackoffSate(30)
                beSupportAfterBackoff()
##~                 print "both in attack, higher player number, support"

    #-- - -- - -- - -- - -- - -- - -- - -- -
    # In all other cases, ie. nobody in attack region, and not both in above 
    # ball region, get behind ball.
    else:
        beGetBehindAfterBackoff(False)
        StillGettingBehind = True
##~         setGetBehindBackoffSate(30)
##~         print "None in attack region, get behind"



#--------------------------------------
# Calculate angles of self and teammate to goal.
def calculateAnglesToGoal():
    global teammateX, teammateY, matchedTmPlayerNumber

    # Try to match visual teammates with gps info if possible.
    # vTeammate values would have been calculated by  backoffBecozTeammateHasBall()
    matchedTmPlayerNumber = matchVisualTeammateToGpsTeammate(teammateX, teammateY)

    # Change the teammateX,Y to be the ones in GPS (instead of vision) if possible.
    if (matchedTmPlayerNumber >= 0):
        teammateX = Global.teammatesLoc[matchedTmPlayerNumber].getX()
        teammateY = Global.teammatesLoc[matchedTmPlayerNumber].getY()
    
    # CRM: Draw the line to the DKD instead of the centre of the target goal. 
    # CRM: Use teammate's global ball position if its variance is smaller than me.
    #            Hence its global ball position should be better than mine ideally.
    angleMe = hMath.absAngleBetweenTwoPointsFromPivotPoint(
                      Global.selfLoc.getX(),
                      Global.selfLoc.getY(),
                      Constant.TARGET_GOAL_X,
                      Constant.TARGET_GOAL_Y,
                      Global.ballX,
                      Global.ballY
                    )
    angleTeammate = hMath.absAngleBetweenTwoPointsFromPivotPoint(
                      teammateX,
                      teammateY,
                      Constant.TARGET_GOAL_X,
                      Constant.TARGET_GOAL_Y,
                      Global.ballX,
                      Global.ballY
                    )

    return (angleMe, angleTeammate)

# Returns -1 if it cant find a match
#
# Given the local heading and global position of a teammate (ie you get this info 
# from vision), try to match it to one of the gps teammates and return its 
# PLAYER NUMBER ie a number between 1-4. This however doesnt match to the 
# goalie (because this is a tailored function for the backoff) and doesnt match if 
# the distance difference between the vision tm and the closest gps tm is greater 
# than 50 cm or if the heading error is greater than 40 degrees 
#
def matchVisualTeammateToGpsTeammate(teammatex, teammatey):
    bestTeammateSoFarIndex = -1;
    bestTeammateDistance   = Constant.LARGE_VAL;
    
    # Go through the dogs in gps.
    for i in range(Constant.NUM_TEAM_MEMBER):
        teammate    = Global.teamPlayers[i]
        teammateLoc = Global.teammatesLoc[i]
        
        # Skip gps player that is not reliable or a goalie.
        if i != Global.myPlayerNum - 1\
                and teammate.getCounter() > 0 \
                and teammate.isGoalie() == 0 \
                and teammateLoc.getPosVar() <= hMath.get95var(60):
            
            # See how the visual teammate xy is different from the gps one.
            distDiff    = hMath.getDistanceBetween(
                            teammatex, teammatey,
                            teammateLoc.getX(), teammateLoc.getY())

            if (distDiff < bestTeammateDistance):
                bestTeammateSoFarIndex  = i
                bestTeammateDistance    = distDiff

    if Debug.matchVisualDebug and bestTeammateSoFarIndex >= 0:
        print "----------------"
        print teammatex
        print teammatey
        #Debug.printTeammateInfo(bestTeammateSoFarIndex)
        #Debug.printSelfInfo()

    GPS_VISUAL_DIST_THRESHOLD = 50;
    
    if bestTeammateDistance < GPS_VISUAL_DIST_THRESHOLD:
        return bestTeammateSoFarIndex;
    else:
        return -1;

#--------------------------------------
# Called by the Forward.DecideNextAction, 
# Return True if I should back off.
#def shouldIVisualBackOff():
#    Debug = True

#    if backoffBecozTeammateHasBall():
#        if Debug:
#            print "rBackOff: Teammate has ball, backing off."
#        return True

    # this is booo~~~~~~~~~~~~~
    # as it misrecognising other object more oftern than its teammates --weiming
    #if backoffBecozTeammateInFront():
    #    if Debug:
    #        print "rBackOff: Teammate in front, backing off."
    #    return True

    # we should consider the case that a teammate sending
    # a msg saying it will go get
    # the ball and its distance is closer or has better attacker angle
    # then this robot should back off

#    return False

#--------------------------------------
# Return True if a teammate is having ball.
def backoffBecozTeammateHasBall():
    global  angleTeammateToGoal, \
            teammateX, \
            teammateY, \
            teammateH

    # Check if teammate in front of you with ball.
    bestIndex  = -1
    bestAbsAng = -10

    if Global.vBall.getConfidence() > 0\
        and Global.ballD < 100:

        for i in range(Constant.NUM_TEAM_MEMBER):
            
            teammate = Global.vTeammates[i]
            
            # CRM - change back to var.
            if (teammate.getConfidence() > 0
                    and teammate.getVariance() < hMath.get95var(200)
                    and teammate.getDistance() < 100):
                
                matchIndex = matchVisualTeammateToGpsTeammate ( teammate.getX() , teammate.getY() )
                if matchIndex != -1:
                    tX = Global.teammatesLoc[matchIndex].getX()
                    tY = Global.teammatesLoc[matchIndex].getY()
                    tH = Global.teammatesLoc[matchIndex].getHeading()
                else:
                    tX = teammate.getX()
                    tY = teammate.getY()
                    tH = teammate.getHeading()
                
                ang = hMath.absAngleBetweenTwoPointsFromPivotPoint( 
                            tX,  
                            tY, 
                            Constant.FIELD_WIDTH / 2, 
                            Constant.FIELD_LENGTH, 
                            Global.ballX, 
                            Global.ballY 
                        )
                
                # If more than one teammate.. Pick the teammate thats furthest around the ball.
                if (ang > bestAbsAng):
                    bestAbsAng = ang
                    bestIndex = i
    
        if bestIndex >= 0:
            angleTeammateToGoal = ang
            teammateX = tX
            teammateY = tY
            teammateH = tH
            return True

    return False

#--------------------------------------
# Return True if a teammate is in front close enough for myself to backoff.
def backoffBecozTeammateInFront():
    global  teammateX, \
            teammateY, \
            teammateH

    bestIndex = -1
    smallestDistance = Constant.LARGE_VAL
    
    for i in range(Constant.NUM_TEAM_MEMBER):
        
        teammate = Global.vTeammates[i]
        
        if (teammate.getConfidence() > 0 
                and teammate.getVariance() < hMath.get95var(200)
                and abs(teammate.getHeading()) < 45 
                and teammate.getDistance() < 100):
                
            matchIndex = matchVisualTeammateToGpsTeammate ( teammate.getX() , teammate.getY() )
            if matchIndex != -1:
                tX = Global.teammatesLoc[matchIndex].getX()
                tY = Global.teammatesLoc[matchIndex].getY()
                tH = Global.teammatesLoc[matchIndex].getHeading()
            else:
                tX = teammate.getX()
                tY = teammate.getY()
                tH = teammate.getHeading()
                
            if teammate.getDistance() < smallestDistance:
                smallestDistance = teammate.getDistance()
                bestIndex = i

    if bestIndex >= 0:
        teammateX = tX
        teammateY = tY
        teammateH = tH
        return True

    return False
    
    
# Return true if I want to do the side back-off. Notice that two dogs side by side but looking at opposite
# direction shouldn't trigger this function because this function only passes if the dog see a vision ball.
# Unless there are two balls on the field or there is a problem from the vision module, two robots
# side by side but looking at the opposite direction shouldn't see a ball at the same time.
def shouldISideBackOff():
    # If I can't see the ball, or if the ball is not in front of me, don't do the side-backoff.
    if not (Global.ballSource == Constant.VISION_BALL and 
            abs(Global.ballH) <= 20 and Global.ballD >= 20):
        return False
    
    for i in range(Constant.NUM_TEAM_MEMBER):
        distance = hMath.getDistanceBetween(Global.teammatesLoc[i].getX(),\
                    Global.teammatesLoc[i].getY(), Global.selfLoc.getPos()[0], \
                    Global.selfLoc.getPos()[1])
    
        # Check that they are a forward, and that the counter hasn't been run out.
        if (i != Global.myPlayerNum - 1 and not Global.teamPlayers[i].isGoalie()
            and Global.teamPlayers[i].getCounter() > 0 and
            Global.teammatesLoc[i].getPosVar() < hMath.get95var(60) and
            Global.teammatesLoc[i].getHeading() > 65  and
            Global.teammatesLoc[i].getHeading() < 115 and
            distance < 50):
            
            # If they can see the ball.
            if (Global.teamPlayers[i].getBallDist() != Constant.LOSTBALL_TIME):
                # If you are sure you are further.
                if (Global.ballD - Global.teamPlayers[i].getBallDist() > 15):
                    if Debug.sideBackDebug:
                        print "Side backoff because I am further!"
                    return True
                    
                # If they are further.
                elif (Global.teamPlayers[i].getBallDist() - Global.ballD > 15):
                    continue
                    
                # If you have higher player number, don't just side backoff.
                elif Global.myPlayerNum > i:
                    if Debug.sideBackDebug:
                        print "Side backoff because of my player number!"
                    return True
    return False

    
def StillDoingGetBehindBackoff():
        
    if not StillGettingBehind:
        return False
        
    if abs(hMath.normalizeAngle_180(Global.selfLoc.getHeading() -Global.DKD[0])) < 20:
        return False
        
    if Global.ballD > 80:
        return False
        
    else:
        return True
"""
