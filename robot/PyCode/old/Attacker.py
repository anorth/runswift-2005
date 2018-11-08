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
#  $Id: Attacker.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 





#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# Attacker.py
#
# Called by Forward.py to make decisions base on being the role of attacker.


# Changes: 
# 1. Narrow dkd when lined up, so paw kick, dont call dir kick
# 2. Get behind when in the own half
# 3. Get hehind upenn when in target half, if grab and upenn is not ok (that is when opp near by)
#     to replace fast upenn
# 4. Comment out the goal edge turn kick
# 
#
#===============================================================================
"""
Attacker
"""

import Kick
import Debug
import Global
import FWHead
import DirMove
import PawKick
import Constant
import HelpTrack
import LocateBall
import StealthDog
import HoverToBall
import GetBehindBall
import math

import WalkAction
import IndicatorAction
import HelpMath
import ReadyPlayer
import HelpTeam
import HelpGlobal

import sGrabTurnKick as GrabTurnKick

#===============================================================================
# Global to be used in this module
#===============================================================================

# find/track/localise ?

# active localise
DisallowLocaliseTimer       = 0
AllowLocaliseTimer          = 0

#stationary localise
StLocaliseTimer             = 0
BallInFaceTimer             = 0

# Locate Ball?
ShouldLocateBall            = False
ShouldLocateWirelessBall    = False

# Ball Region
ThisDKDRegion               = None
LastDKDRegion               = None
LastDKDRegionChangesSince   = 0
LockDKDRegion               = Constant.DKDBlank

# Either HOVER_TO_BALL or GET_BEHIND_BALL.
AttackMode                  = Constant.HOVER_TO_BALL

# For edge spin
SpinDirMem                  = None
SpinAngleMem                = None

# For get behind ball
AttackAngleMem              = None
##~ AttackAngleRangeMem         = None
AttackDirMem                = None
AttackDistMem               = None
##~ AttackPosX                  = None
##~ AttackPosY                  = None

# For DirMove
DirAttackAngleMem           = None

# For debug
debugLed                    = None

# For Grabbing the ball
Grabbing_ForHK              = False
Grabbing_ForVOAK            = False

# For stuck resolution
panx                        = 0
panDirection                = Constant.dCLOCKWISE
stuckDetectTimer            = 0
stuckDetect                 = None
stuckSanityCheckTimer       = 0
pastBallDistanceInfo        = []

# max variance limit for an attacker
MAX_POS_VAR                 = 150
MAX_HEAD_VAR                = 100

# Stuck Threashold for reversing
STUCK_RESOLUTION_THRESHOLD  = 300
NUM_STUCK_FRAME             = 10


#===============================================================================
# Functions belong to this module.
#===============================================================================

##############################################
# Nobuyuki (17/02/2005) 
# I think the code below is no longer needed.
# So it will be removed soon.
##############################################

"""
def AttackerStationaryLocalise ():

    global BallInFaceTimer

    # debug led constant
    STATIONARY_LOCALISE         = [3,1,0,1,3]

    BallInFaceTimer = 0
    setLocaliseTimer (0,90) # cant do active localise afterwards for 3 sec
    Global.lostBall = HelpMath.DECREMENT(Global.lostBall)
    FWHead.compulsoryAction = FWHead.mustStatLocalise
    HelpTrack.saGoToTarget(Global.ballX,Global.ballY)
    IndicatorAction.showFacePattern(STATIONARY_LOCALISE)

def ShouldIStationaryLocalise():

    global StLocaliseTimer
    global BallInFaceTimer
    
    debug = False
    if debug:
        print ""
        print "PosVar() %.2f, HeadVar() %.2f" % ( Global.selfLoc.getPosVar(), Global.selfLoc.getHeadingVar() )
        print "StLocaliseTimer %d,   BallInFaceTimer %d" % ( StLocaliseTimer , BallInFaceTimer)
    
    if (StLocaliseTimer > 0):
        StLocaliseTimer -= 1
        AttackerStationaryLocalise()
        if debug:
            print "st localise"
        return True

    # Very lost localise
    
    if ( (Global.selfLoc.getPosVar() > HelpMath.get95cf (MAX_POS_VAR) or
            Global.selfLoc.getHeadingVar() > HelpMath.get95cf(MAX_HEAD_VAR)) and
            len (HelpTeam.whichGpsOpponentWithinDistVar (40, 30)) == 0 and
            StLocaliseTimer == 0 ):
            
        StLocaliseTimer = 50
        AttackerStationaryLocalise()
        if debug:
            print "first call very lost localise " , StLocaliseTimer
        return True
            
        
    # Stuck localise -- if u are stuck by others, and the ball is in front of u, do a localise
        
    if Global.ballD < 20 and Global.lostBall < 5:
        BallInFaceTimer += 1
    else:
        BallInFaceTimer = 0
        
    if BallInFaceTimer == 90 and StLocaliseTimer == 0:
        StLocaliseTimer = 40
        AttackerStationaryLocalise()
        
        if debug:
            print "first call to ball in face localise"
        
        return True
        
    return False
"""

LOCALISE_ACTIVATE_BALL_RANGE = 60
LOCALISE_DEACTIVATE_BALL_RANGE = 50 
LOCALISE_TIMER = 45
g_LastActivelyLocalisedFrame = 0
g_ActiveLocaliseTimer = 0
def ShouldIActivelyLocalise():     
    global g_ActiveLocaliseTimer
    
    ######################################
    # Localisation Deactivation Criteria. 
    ######################################  
    if g_ActiveLocaliseTimer > 0: 
        g_ActiveLocaliseTimer -= 1    
        
        if Global.ballD < LOCALISE_DEACTIVATE_BALL_RANGE:    
            pass
        else:
            #print "Actively Localise Now!!!!"
            return True    
    
    
    #################################################
    # Localisation Activation Criteria used in 2003. 
    #################################################       
    
    ####################################
    # 1a The attacker can see the ball.
    ####################################
    elif Global.vBall.getConfidence() > 0\
        and Global.haveBall > 45: 
        
        ####################################################
        # 1b The ball is still far away and straight ahead.
        ####################################################
        ballD, ballH = Global.ballD, Global.ballH
        if ballD > LOCALISE_ACTIVATE_BALL_RANGE\
            and abs(ballH) < 15:
            
            #############################################
            # 1c There are no opponents nearby the ball.
            #############################################
            for opp in Global.vOpponents: 
                if abs(opp.getDistance() - ballD) > 20\
                    and abs(opp.getHeading() - ballH) > 15:
                    
                    break
            else:
            
                ######################################################
                # 1d The attacker has not recently actively localise.
                ######################################################
                if Global.frame - g_LastActivelyLocalisedFrame > 90: 
                    #print "Actively Localise Now!!!!"
                    g_ActiveLocaliseTimer = LOCALISE_TIMER
                    return True
                             
    g_ActiveLocaliseTimer = 0             
    return False
    
    
def ActivelyLocalise(): 
    global g_LastActivelyLocalisedFrame
    global ShouldLocateBall, ShouldLocateWirelessBall
    
    g_LastActivelyLocalisedFrame = Global.frame
    
    ###################################
    # Turn a little if it is required.
    ###################################
    ballH = Global.ballH 
    turnccw = HelpMath.CLIP(ballH,10)
    FWHead.compulsoryAction = FWHead.mustLocalise
    WalkAction.setEllipticalWalk(10,0,turnccw)
    
    
    ShouldLocateBall         = False
    ShouldLocateWirelessBall = False
    Global.lostBall          = HelpMath.DECREMENT(Global.lostBall)

    



#=========================================================
# Set some period of time that allow dog to localise if neccessary ("neccessary"
# as controlled by FWHead), and some period that it should NOT localise.


##############################################
# Nobuyuki (17/02/2005) 
# I think the code below is no longer needed.
# So it will be removed soon.
##############################################
"""
def LocaliseOrTrackOrFindBall ():

    global AllowLocaliseTimer, DisallowLocaliseTimer
    global AttackMode
    global ShouldLocateBall, ShouldLocateWirelessBall
    
    debug = False
    if debug:
        print "===============new frame"
        print "The lostBall counter " , Global.lostBall
        print "Allow for: ", AllowLocaliseTimer, " Disallow for:      ",  DisallowLocaliseTimer
        print "The current ptc is " , Global.degPan, " " , Global.degTilt, " " , Global.degCrane
        
    # Doing the active localisation
    if AllowLocaliseTimer > 0:
        
        FWHead.compulsoryAction = FWHead.mustLocalise
        ShouldLocateBall        = False
        ShouldLocateWirelessBall= False
        Global.lostBall         = HelpMath.DECREMENT(Global.lostBall)
        AllowLocaliseTimer     -= 1
        if debug:
            print "case 1: localise"
        
    # Decide whether it should localise or do other thing when both timers are 0
    elif AllowLocaliseTimer==0 and DisallowLocaliseTimer==0:
        if ( AttackMode == Constant.GET_BEHIND_BALL or
            (len (HelpTeam.whichGpsOpponentWithinDistVar (30, 30)) == 0 and Global.lostBall == 0 )
           ):
            setLocaliseTimer (10,50)
            FWHead.compulsoryAction = FWHead.mustLocalise
            ShouldLocateBall        = False
            ShouldLocateWirelessBall= False
            if debug:
                print "case 2: localise, having seen the ball"
            
        else:
            setLocaliseTimer(0,50)
            TrackOrFindBall()
            if debug:
                print "case 2: call track or find 0 0"

    else: # Case when DisallowLocaliseTimer > 0 and AllowLocaliseTimer==0 
        DisallowLocaliseTimer      -= 1
        TrackOrFindBall()
        if debug:
            print "case 3: call track or find"

"""

#-------------------------------------------------------------------------------------
# return TRUE if track any ball, FALSE if locate/find ball
def TrackOrFindBall():

    global ShouldLocateBall, ShouldLocateWirelessBall

##~     debug = False
    
    lostBall                    = Global.lostBall
    ShouldLocateBall            = False
    ShouldLocateWirelessBall    = False
    myx,myy                     = Global.selfLoc.getPos()
    wirelessBallDist            = HelpMath.getDistanceBetween(myx,myy,Global.sharedBallX,Global.sharedBallY)
    
##~     if debug:
##~         print "sharedBallVar: %.2f , %.2f, %.2f" % (Global.sharedBallVar,Global.sharedBallX, Global.sharedBallY)
    
    if lostBall < 5:
        FWHead.compulsoryAction = FWHead.mustSeeBall
        if Debug.headDebug:
            print "Track visual ball < 5"
        
    elif (lostBall <= Constant.VBALL_LOST and abs(Global.gpsLocalBall.getHeading()) < 90):
        FWHead.compulsoryAction = FWHead.mustSeeGpsBall
        if Debug.headDebug:
            print "GPS ball in attacker!"
        
    elif (lostBall <= Constant.VBALL_LOST):
        FWHead.compulsoryAction = FWHead.mustSeeBall
        if Debug.headDebug:
            print "track visual ball >= 5"
            
    elif (Global.sharedBallVar < Constant.LARGE_VAL and
            wirelessBallDist > 80):
        ShouldLocateWirelessBall = True
##~         if debug:
##~             print "wirelessBallDist: %.2f" % (wirelessBallDist)
##~             print "going to wireless ball at %.2f, %.2f" % (Global.sharedBallX, Global.sharedBallY)
        FWHead.compulsoryAction  = FWHead.fixHead
        LocateBall.headFindBall(-50, -5)
        HelpTrack.saGoToTarget(Global.sharedBallX, Global.sharedBallY)
        if Debug.headDebug:
            print "Wireless ball in attacker!"
 
    else: # the case when lostBall > Constant.VBALL_LOST
        ShouldLocateBall = True
        LocateBall.DecideNextAction()
        setLocaliseTimer (0,2)
        if Debug.headDebug:
            print "Locate ball in attacker!"


#================================
# Just a helper in setting the timers

def setLocaliseTimer (AllowPeriod, DisallowPeriod):

    global AllowLocaliseTimer, DisallowLocaliseTimer
    
    AllowLocaliseTimer     = AllowPeriod
    DisallowLocaliseTimer  = DisallowPeriod
    


def ShouldIDoGrabTurnKick(): 
    pass    

    
#=========================================================
# If a robot is not a striker, supporter, backoffer then it should be an attacker.
lockMode = False

def DecideNextAction():
    global lockMode
    
    Global.myRole = Constant.ATTACKER
    
    
    #----------------------------------------------------------
    # Grabbing ball for turn and kick
    
    lockMode = GrabTurnKick.lockMode
    
    if lockMode:
        GrabTurnKick.DecideNextAction()
        return
        

    #----------------------------------------------------------
    # stuck Detection (hysteresis)
    global stuckDetectTimer
    
    if stuckDetectTimer <= 30 and stuckDetectTimer != 0:
        StuckResolution()
        return  

    #######################################################################
    # Nobuyuki (16/02/2005)
    # Does the robot have to stop when it is really lost?
    # If it is in the middle of tracking the ball, why stop?
    # If it is not tracking the ball, then finding the ball is much more 
    # important than localising. 
    # Thus, I'll comment the lines below...
    # 
    # I also implemented active localisation criteria used in 2003. 
    #######################################################################
    #if ShouldIStationaryLocalise():  # do it when really lost (high pos / heading var), with no opp nearby 
    #    return
    if ShouldIActivelyLocalise(): 
        ActivelyLocalise()
        return  

    ###################################################################
    # Nobuyuki (17/02/2005)
    # If the robot is not localising then let it do either tracking or 
    # finding.
    ###################################################################
    TrackOrFindBall()
        
    if ShouldLocateBall or ShouldLocateWirelessBall:
        return
        
    #=========================================
    # Mostly VISION_BALL from HERE ONWARDS
    # Other balls?
    # --------------------------------------------------------------------------
    # This is the hysteresis for different approach

    if StillDoingGetBehind():
        return
        
    if StillEdgeSpinning():
        return
        
    # ------------------------------------------------------------
    # ------ CHOOSE APPROACH STRATEGY -----
    # ------------------------------------------------------------

    if Global.ballD < 30 and Global.lostBall == 0:
        setLocaliseTimer (0,2)
    
    if chooseApproach_noSlowDown():
        return
        
    if chooseApproach_slowDown():
        return
        
    HoverToBall.DecideNextAction()

# -------------------------------------------------
# CHOOSE APPROACH STRATEGY



def chooseApproach_noSlowDown():
    
    #------------------------------------
    # stuck into or by other robot
    global stuckDetect
    
    stuckDetect = IsAttackerStuck(STUCK_RESOLUTION_THRESHOLD)

    if (Global.lostBall == 0 and
        not HelpTrack.ballOnBEdge(Constant.FIELD_LENGTH/3.5) and stuckDetect is not None):
        chooseStuckResolution(Global.degPan)
        return True
    #------------------------------------------------
    # EDGE PAW KICK approach
    pawKickResults = PawKick.IsPawKickOk()

    if pawKickResults[0]:
        PawKick.firePawKick(pawKickResults[1])
        return True
    # --------------------------------------------------------------------------
    # EDGE GET_BEHIND_BALL
    ballRgnBounds = InWhichBallRegion(80.0) # input is the decision boundary
    if ballRgnBounds is not None:

        # Do getBehindBall if needed.
        # Once doing it will not stop unless conditions break it out in StillDoingGetBehind().
        if chooseGetBehind(ballRgnBounds):
            return True
    # -----------------------------------------------------------
    # STEATH DOG  case (opponent avoidance). True means the stealth only when see vision ball.
    if StealthDog.stealthDog(True):
        FWHead.compulsoryAction = FWHead.mustSeeBall
        return True
    # --------------------------------------------------------------------------
    # DIR PAW KICK cases (for heading up field)
    global DirAttackAngleMem
    
    if (HelpTrack.isInRange(0,180) and 
        abs(Global.ballH < 75) and
        HelpTrack.ballNotAlongEdge() and 
        HelpTrack.isNotAlongEdge() and
        abs(HelpMath.normalizeAngle_180(Global.selfLoc.getHeading() - Global.DKD[1])) < 20
        # was Global.selfLoc.getHeading() - Global.DKD[0])) < 75
       ):
        
        myy     = Global.selfLoc.getY()
        dkdMin  = Global.DKD[1]
        dkdMax  = Global.DKD[2]
            
        if (myy > Constant.FIELD_LENGTH*0.6 and    # in the upper 60 % (Y) either VOAK range/ to target goal post 
            HelpTrack.isInRange (dkdMin, dkdMax)): # and i am lying inbetween
            PawKick.firePawKick(PawKick.FIRE_PAWKICK_AUTO)
            return True
            
        if (Global.ballD > 30 or DirAttackAngleMem is None):
            DirAttackAngleMem = Global.DKD[0]
        
        DirMove.DecideNextAction(35, DirAttackAngleMem,Constant.dCLOCKWISE,30,9.5,65,1,1)
        
        if Global.ballD < 35 and Global.lostBall == 0:
            setLocaliseTimer (0,2)
            
        return True

        
    return False
    
def chooseApproach_slowDown():

    # Use HoverToBall for slow down approaches
    HoverToBall.DecideNextAction()
    
    global Grabbing_ForHK, Grabbing_ForVOAK
    global lockMode
    
    myx , myy = Global.selfLoc.getPos()
    
    #------------------------------------------
    # Should slow down for edge spin (disallow the top edge case, often flicks the ball down field)
    if (
          (not HelpTrack.isInRange(40,320) and HelpTrack.ballOnREdge(15)) or
          (HelpTrack.isInRange(140, 220) and HelpTrack.ballOnLEdge(15)) or
          (HelpTrack.isInRange(50, 130) and (HelpTrack.ballOnTREdge(15) or HelpTrack.ballOnTLEdge(15)))
##~           (HelpTrack.isInRange(230,310) and HelpTrack.ballOnBEdge(15))
         ):
          
        AttackerSlowDown(25.0 , 10.0 , 6.0 , 2.0)
        #shouldEdgeSpin()
        
        return True
        
    #####################################################
    # Nobuyuki (17/02/2005)
    # The decision making shall be reconsidered later!!
    #####################################################
    #---------------------------------------------------------
    # Grab turn kick
    elif (Global.ballY > Constant.FIELD_LENGTH / 2.0 and
          #HelpTrack.ballNotAlongEdge() and         # Cant grap well on edge, should get behind
          #not HelpTrack.isInRange(20,160) and      # going up, dont grap & upen
          len (HelpTeam.whichGpsOpponentWithinDistVar (50, 50)) == 0 # No opps too close by
          #not (myy > Constant.TOP_GOALBOX_EDGE_Y and myx < Constant.MIN_GOALBOX_EDGE_X) and
          #not (myy > Constant.TOP_GOALBOX_EDGE_Y and myx > Constant.MAX_GOALBOX_EDGE_X)
         ):
        
        GrabTurnKick.DecideNextAction()
        lockMode = GrabTurnKick.lockMode
                
        return True
    
    #---------------------------------------------------------
    # getBehind and upenn rather than fast upenn

    elif (
          HelpTrack.ballNotAlongEdge() and         # Cant grap well on edge, should get behind
          not (myy > Constant.TOP_GOALBOX_EDGE_Y and myx < Constant.MIN_GOALBOX_EDGE_X) and
          not (myy > Constant.TOP_GOALBOX_EDGE_Y and myx > Constant.MAX_GOALBOX_EDGE_X)
         ):
        
        ballRgnBounds = InWhichBallRegion(100.0) # input is the decision boundary
        if ballRgnBounds is not None:
        
        # Do getBehindBall if needed.
        # Once doing it will not stop unless conditions break it out in StillDoingGetBehind().
            if chooseGetBehind_Upenn(ballRgnBounds):
                return True        
    return False

    

    
    
#=========================================================
# Given a ball, see which region division I am in, RT,LT, BottomRight or BottomLeft,
# and set it in ThisDKDRegion.
# decisionAngle should be between 90 to 170 in practice, in calculation it should be 0 - 180
#
# (rightDivLineAngle, leftDivLineAngle) is return, return None if: 
# 1. DKD is None        
# 2. Can't find any angle, see the code

def InWhichBallRegion(decisionAngle):

    global LastDKDRegion, ThisDKDRegion, LastDKDRegionChangesSince
    
    regionDebug = False
    
    myx, myy    = Global.selfLoc.getPos()
    
    if LastDKDRegion == ThisDKDRegion:
        LastDKDRegionChangesSince += 1
    else:
        LastDKDRegionChangesSince = 0
    LastDKDRegion = ThisDKDRegion
    
    # Error checking - have DKD?
    if Global.DKD[0] is None:
        print "Error: InWhichBallRegion() the DKD has not been initialised!"
        return None
        
    if regionDebug:
        print "======================="
        print Global.DKD

    if (Global.ballSource==Constant.VISION_BALL or Global.lostBall < Constant.VBALL_LOST):
        angleToBall = HelpMath.getHeadingBetween (Global.ballX, Global.ballY,myx, myy)
    else:
        # If this can happen sometimes anyway (not an error case), print it only if debug mode.
        if regionDebug:
            print "Warning: InWhichBallRegion() no vision ball source"
        
        angleToBall = HelpMath.getHeadingBetween (Global.ballX, Global.ballY, myx, myy)

        # Dan: ?? Statements in both cases are the same!


    # The right & left division lines are return to callers 
    rightDivLineAngle = HelpMath.normalizeAngle_0_360(Global.DKD[0] - decisionAngle)
    letfDivLineAngle  = HelpMath.normalizeAngle_0_360(Global.DKD[0] + decisionAngle)
    
    if regionDebug:
        print "The right & left DivLine Angle & DKD are " , rightDivLineAngle, "  " , letfDivLineAngle ,\
            "  " , Global.DKD[0]
    # Now, the DKD is considered locally (that is it is the x-axis), see which region I falls under
    
    angleToBall = HelpMath.normalizeAngle_0_360(angleToBall - Global.DKD[0])
    
    if angleToBall <= decisionAngle and angleToBall >= 0:
    
        if regionDebug:
            print "DKD Region Left"
            
        ThisDKDRegion = Constant.DKDLeft
        return (rightDivLineAngle,letfDivLineAngle)

    elif angleToBall < 180 and angleToBall > decisionAngle:
    
        if regionDebug:
            print "DKD Region Bottom Left"  
            
        ThisDKDRegion = Constant.DKDBottomLeft
        return (rightDivLineAngle,letfDivLineAngle)

    elif angleToBall < (360.0-decisionAngle) and angleToBall >= 180:
    
        if regionDebug:
            print "DKD Region Bottom Right"
            
        ThisDKDRegion = Constant.DKDBottomRight
        return (rightDivLineAngle,letfDivLineAngle)
        
    elif angleToBall < 360 and angleToBall >= (360.0-decisionAngle):
    
        if regionDebug:
            print "DKD Region Right"
            
        ThisDKDRegion = Constant.DKDRight
        return (rightDivLineAngle,letfDivLineAngle)
        

    print "Warning: InWhichBallRegion() normalised to (0,360) but still out of range" , angleToBall
    return None




#===============================================================
# Choose a get behind defensive strategy, this function is TRIGGERED ONCE, then it
# will lock into this attack mode, until either the ball rolls out, or the move is finished.

def chooseGetBehind(ballRgnBounds):

    global ThisDKDRegion, LockDKDRegion
    global debugLed

    debug           = False
    debugLed        = None
    returnVal       = False
    dkd             = Global.DKD[0]
    rightDivLineAngle, letfDivLineAngle = ballRgnBounds


    if debug:
        print "dkd is %.2f" % (dkd)
    
    # --------------------------------------------------------------------------
    # False - no visual ball.
    if Global.ballSource != Constant.VISION_BALL:
        if debug:
            print "Get behind: I need Vision ball I only got " , Global.ballSource
        returnVal = False

    # --------------------------------------------------------------------------
    # False - ball is too far.
    elif Global.ballD >= 60:
        if debug:
            print "Get behind: I need ballD < 60 " , Global.ballD
        returnVal = False
    
    # --------------------------------------------------------------------------
    # True -  Left edge
    elif HelpTrack.ballOnLEdge() and HelpTrack.isInRange(195,345):
    
        GetBehindBall.getBehindBall(25, dkd, Constant.dCLOCKWISE, False, 60 , 1.2)
        debugLed = Constant.FP_GET_BEHIND_LE
        setGetBehindBallState (25,dkd,Constant.dCLOCKWISE,Constant.DKDBlank)
        returnVal = True
        
        if debug:
            print "GetBehindBall(): left edge LockDKDRegion is " , LockDKDRegion
        
    # --------------------------------------------------------------------------
    # True -  Right edge
    elif HelpTrack.ballOnREdge() and HelpTrack.isInRange(195,345):
    
        GetBehindBall.getBehindBall(25, dkd, Constant.dANTICLOCKWISE, False, 60 , 1.2)
        debugLed = Constant.FP_GET_BEHIND_RE
        setGetBehindBallState (25,dkd,Constant.dANTICLOCKWISE,Constant.DKDBlank)
        returnVal = True
        
        if debug:
            print "GetBehindBall(): right edge LockDKDRegion is " , LockDKDRegion
        
    # --------------------------------------------------------------------------
    # True - Bottom edge
    elif (HelpTrack.isInRange(200,340) and Global.ballY < 70):
        # and (gps->getOppCovMax(oppNum) < get95CF(75) && opp.d < 100)
        debugLed = Constant.FP_GET_BEHIND_BE
        
        if Global.ballX < Constant.FIELD_WIDTH/2.0:
            GetBehindBall.getBehindBall(25,dkd,Constant.dCLOCKWISE, False, 60, 1.2)
            setGetBehindBallState (25,dkd,Constant.dCLOCKWISE,Constant.DKDBlank)
            
            if debug:
                print "GetBehindBall(): bottom left edge LockDKDRegion is " , LockDKDRegion
                
            returnVal = True
            
        elif Global.ballX >= Constant.FIELD_WIDTH/2.0:
            GetBehindBall.getBehindBall(25,dkd,Constant.dANTICLOCKWISE, False, 60, 1.2)
            setGetBehindBallState(25,dkd,Constant.dANTICLOCKWISE,Constant.DKDBlank)
            
            if debug:
                print "GetBehindBall(): bottom right edge LockDKDRegion is " , LockDKDRegion
            
            returnVal = True
            
        else:
            
            returnVal = False

    # --------------------------------------------------------------------------
    # True - Top right corner
    elif (HelpTrack.ballInTRCorner() and 
           (   HelpTrack.isInRange(0,70) 
            or HelpTrack.isInRange(330,360)) # Dan: Why not 180-360?
         ):
        
        GetBehindBall.getBehindBall(25,100,Constant.dANTICLOCKWISE, False, 60 , 1.2)
        debugLed = Constant.FP_GET_BEHIND_TRC
        setGetBehindBallState(25,100,Constant.dANTICLOCKWISE,Constant.DKDBlank)
        returnVal = True
        
        if debug:
            print "GetBehindBall(): top right corner LockDKDRegion is " , LockDKDRegion

    # --------------------------------------------------------------------------
    # True - Top left corner
    elif HelpTrack.ballInTLCorner() and HelpTrack.isInRange(110,210):
        
        GetBehindBall.getBehindBall(25,80,Constant.dCLOCKWISE, False, 60, 1.2)
        debugLed = Constant.FP_GET_BEHIND_TLC
        setGetBehindBallState(25,80,Constant.dCLOCKWISE,Constant.DKDBlank)
        returnVal = True
        
        if debug:
            print "GetBehindBall(): top left corner LockDKDRegion is " , LockDKDRegion

    # --------------------------------------------------------------------------
    # True - Top right edge.
    elif (HelpTrack.ballOnTEdge() and 
##~           Global.ballX > Constant.FIELD_WIDTH / 2.0 and
        Global.ballX > Constant.RIGHT_GOAL_POST and
        (HelpTrack.isInRange(0,80) or HelpTrack.isInRange(330,360)) ):
        
        GetBehindBall.getBehindBall (25,100,Constant.dANTICLOCKWISE, False, 60 ,1.2)
        debugLed = Constant.FP_GET_BEHIND_TRE
        setGetBehindBallState(25,100,Constant.dANTICLOCKWISE,Constant.DKDBlank)
        returnVal = True
        
        if debug:
            print "GetBehindBall(): top right edge LockDKDRegion is " , LockDKDRegion
        
    # --------------------------------------------------------------------------
    # True - Top left edge
    elif (HelpTrack.ballOnTEdge() and 
        Global.ballX < Constant.LEFT_GOAL_POST and 
##~           Global.ballX <= Constant.FIELD_WIDTH / 2.0 and
        HelpTrack.isInRange(100,210) ):
        
        GetBehindBall.getBehindBall(25,80,Constant.dCLOCKWISE, False, 60, 1.2)
        debugLed = Constant.FP_GET_BEHIND_TLE
        setGetBehindBallState(25,80,Constant.dCLOCKWISE,Constant.DKDBlank)
        returnVal = True
        
        if debug:
            print "GetBehindBall(): top left edge LockDKDRegion is " , LockDKDRegion

    if debugLed is not None:
        IndicatorAction.showFacePattern(debugLed)
        
    return returnVal


def chooseGetBehind_Upenn(ballRgnBounds):

    global ThisDKDRegion, LockDKDRegion
    global debugLed

    debug           = False
    debugLed        = None
    returnVal       = False
    dkd             = Global.DKD[0]
    rightDivLineAngle, letfDivLineAngle = ballRgnBounds


    
    if debug:
        print "dkd is %.2f" % (dkd)
    
    # --------------------------------------------------------------------------
    # False - no visual ball.
    if Global.ballSource != Constant.VISION_BALL:
        if debug:
            print "Get behind: I need Vision ball I only got " , Global.ballSource
        returnVal = False

    # --------------------------------------------------------------------------
    # False - ball is too far.
    elif Global.ballD >= 60:
        if debug:
            print "Get behind: I need ballD < 60 " , Global.ballD
        returnVal = False
        
    # ------------------------------------------------------------------------------------------------------------------------------------
    # True - The ball behind you, this is after the edge cases because u dont want upenn along the wall
    elif (ThisDKDRegion==Constant.DKDLeft and HelpTrack.ballNotAlongEdge()
        and Global.ballY < Constant.FIELD_LENGTH * 0.75):
        
        attackAngle = HelpMath.normalizeAngle_0_360 (Global.DKD[0] - 60)     
        GetBehindBall.getBehindBall(15,attackAngle,Constant.dANTICLOCKWISE, False, 60, 1.0)
        setGetBehindBallState(15,attackAngle,Constant.dANTICLOCKWISE,Constant.DKDLeft)
        debugLed      = Constant.FP_DKD_LEFT
        returnVal     = True
        
        if debug:
            print "GetBehindBall(): DKD left LockDKDRegion is " , LockDKDRegion

    # --------------------------------------------------------------------------
    # True - The ball behind you, this is after the edge cases because u dont want upenn along the wall
    elif (ThisDKDRegion==Constant.DKDRight and HelpTrack.ballNotAlongEdge()
        and Global.ballY < Constant.FIELD_LENGTH * 0.75):
        
        attackAngle = HelpMath.normalizeAngle_0_360(Global.DKD[0] + 60)        
        GetBehindBall.getBehindBall(15,attackAngle,Constant.dCLOCKWISE, False, 60, 1.0)
        setGetBehindBallState(15,attackAngle,Constant.dCLOCKWISE,Constant.DKDRight)
        debugLed      = Constant.FP_DKD_RIGHT
        returnVal     = True
        
        if debug:
            print "GetBehindBall(): DKD right LockDKDRegion is " , LockDKDRegion

    if debugLed is not None:
        IndicatorAction.showFacePattern(debugLed)
        
    return returnVal


#===============================================================
# Handy call to getBehindBall.
def setGetBehindBallState (radius,aa,dir,dkdlock):

    global AttackDistMem, AttackAngleMem
    global AttackDirMem, AttackMode
##~     global AttackPosX, AttackPosY
    global DisallowLocaliseTimer, AllowLocaliseTimer
    global LockDKDRegion
    
    AttackDistMem       = radius
    AttackAngleMem      = aa
    AttackDirMem        = dir
    AttackMode          = Constant.GET_BEHIND_BALL
##~     AttackPosX      = Global.ballX
##~     AttackPosY      = Global.ballY
    LockDKDRegion       = dkdlock
    
    AllowLocaliseTimer    = 0
    DisallowLocaliseTimer = 120

#===============================================================
# Should I continue the attack or not
# TRUE means break out the global behaviour and continue either the get behind, or continue the upenn
# FALSE means not doing any get behind attack, but try the next decision

def GetBehindBreakOutAction():
    global debugLed, LockDKDRegion, AttackMode
    
    setLocaliseTimer (10,60)
    debugLed               = None
    LockDKDRegion          = Constant.DKDBlank
    AttackMode             = Constant.HOVER_TO_BALL
##~             HelpLong.setDefaultAction()
##~             StandTimer = 100
    
def StillDoingGetBehind():

    global debugLed
    global AttackMode, LockDKDRegion
    global AllowLocaliseTimer, DisallowLocaliseTimer
    global AttackDistMem, AttackAngleMem, AttackDirMem
##~     global AttackPosX, AttackPosY
##~     global StandTimer
    
    debug = False
        
##~     if debug:
##~         print "", HelpLong.getStuckInfo()
    
    if AttackMode == Constant.GET_BEHIND_BALL:
        
        if debug:
            print ""
            print "In StillDoingGetBehind()"
            print "mode       %d   dkd-region     %d" % (AttackMode,LockDKDRegion)
            print "allowTimer %d   disallowTimer  %d" % (AllowLocaliseTimer,DisallowLocaliseTimer)
            print "radius %d,  angle %.2f,  dir %d" % (AttackDistMem,AttackAngleMem,AttackDirMem)
##~             print "AttackPosX %.2f,   AttackPosY %.2f" % (AttackPosX, AttackPosY)
            print "myh %.2f" % (Global.selfLoc.getHeading())
    

        # Break out the get behind if:
        # 1. u have done it, a fire paw kick case will catch it if on edges, and it should
        # 2. the ball rolls out far, then chase it rather than get behind
        # 3. the ball keeps rolling out
        
        if abs(HelpMath.normalizeAngle_180(Global.selfLoc.getHeading()-AttackAngleMem)) < 20:
            
            GetBehindBreakOutAction()
            
            if debug:
                print "break out, lined up: the h" , Global.selfLoc.getHeading()
            return False
             
        if Global.ballD > 65:           # Ball has rolled out far away
                    
            if debug:
                print "break out: ballD > 65" , Global.ballD
                
            GetBehindBreakOutAction()
            return False
            
        if Global.lastForward < -3:
        
            GetBehindBreakOutAction()
            return False
       
        # Otherwise, continue getBehindBall, and additionally may do a kick.
        GetBehindBall.getBehindBall(AttackDistMem,AttackAngleMem,AttackDirMem, False, 60 , 1.2)
        IndicatorAction.showFacePattern(debugLed)
        
        if (Global.ballD < 25 and LockDKDRegion != Constant.DKDBlank):
            Global.finalAction[Constant.AAForward] *= 0.7
            Global.finalAction[Constant.AALeft]    *= 0.7
            Global.finalAction[Constant.AATurnCCW] *= 0.7
            
        # Check if ready to do a YouPanCake.
        if LockDKDRegion == Constant.DKDLeft and BallUnderChinForUpenn():
            
            debugLed        = None
            LockDKDRegion   = Constant.DKDBlank
            AttackMode      = Constant.HOVER_TO_BALL
            setLocaliseTimer (0,30)
            if debug:
                print "break out doing upenn"
            Kick.UPright()

        elif LockDKDRegion == Constant.DKDRight and BallUnderChinForUpenn():
            
            debugLed        = None
            LockDKDRegion   = Constant.DKDBlank
            AttackMode      = Constant.HOVER_TO_BALL
            setLocaliseTimer(0,30)
            Kick.UPleft()
                
            if debug:
                print "break out doing upenn"
            
        return True
            
    # then it must be HOVER_TO_BALL.
    else:
        return False



#===================================
# Edge spinning         
# Rather than upenn, get behind, hover, turning
# It should edge spinning




def shouldEdgeSpin():
    
    returnVal   = False
    debug       = False
    
    # Heading wrong and in right edge.
    if (not HelpTrack.isInRange(40,320)  
            and Global.ballX > Constant.FIELD_WIDTH - Constant.WALL_THICKNESS - 15
            and Global.ballD < 16
            and abs(Global.ballH) < 40 ):
        
        WalkAction.setForceStepComplete()    
        edgeSpin (Constant.dANTICLOCKWISE)
        setEdgeSpinState (90, Constant.dANTICLOCKWISE)
        returnVal = True
        
        if debug:
            print "Facing right edge, spin dANTICLOCKWISE"

    # Heading wrong and in left edge. 
    elif (HelpTrack.isInRange(140, 220)
            and Global.ballX < Constant.WALL_THICKNESS + 15
            and Global.ballD < 16
            and abs(Global.ballH) < 40 ):
        
        WalkAction.setForceStepComplete()
        edgeSpin (Constant.dCLOCKWISE)
        setEdgeSpinState (90,Constant.dCLOCKWISE)
        returnVal = True
        
        if debug:
            print "Facing left edge, spin dCLOCKWISE"
        
    # Heading wrong in top edge.
    elif (HelpTrack.isInRange(50, 130)
            and Global.ballY > Constant.FIELD_LENGTH - Constant.WALL_THICKNESS - 15
            and Global.ballD < 16
            and abs(Global.ballH) < 40 ):
            
        # top left edge
        if (Global.ballX < Constant.LEFT_GOAL_POST):
            
            WalkAction.setForceStepComplete()
            edgeSpin (Constant.dCLOCKWISE)
            setEdgeSpinState (30,Constant.dCLOCKWISE)
            returnVal = True
            
            if debug:
                print "Facing top left edge, spin dCLOCKWISE"
        
        # top right edge
        elif (Global.ballX > Constant.RIGHT_GOAL_POST):
            
            WalkAction.setForceStepComplete()
            edgeSpin (Constant.dANTICLOCKWISE)
            setEdgeSpinState (150,Constant.dANTICLOCKWISE)
            returnVal = True
            
            if debug:
                print "Facing top right edge, spin dANTICLOCKWISE"

    # bottom edge
    elif (HelpTrack.isInRange(230,310)
        and Global.ballY < Constant.WALL_THICKNESS + 15
        and Global.ballD < 16 
        and abs(Global.ballH) < 40 ):
        
        # bottom left edge
        if (Global.ballX < Constant.LEFT_GOAL_POST):
        
            edgeSpin (Constant.dCLOCKWISE)
            setEdgeSpinState (180,Constant.dCLOCKWISE)
            returnVal = True
            
            if debug:
                print "Facing bottom left edge, spin dCLOCKWISE"
    
        # bottom right edge
        elif (Global.ballX > Constant.RIGHT_GOAL_POST):
    
            edgeSpin (Constant.dANTICLOCKWISE)
            setEdgeSpinState (0,Constant.dANTICLOCKWISE)
            returnVal = True
            
            if debug:
                print "Facing bottom right edge, spin dANTICLOCKWISE"
        
    return returnVal


def edgeSpin (dir):
    
    debug = False
    
    if dir == Constant.dCLOCKWISE:
    
        Global.finalAction[Constant.AACmd:Constant.AAHeadType] = \
                Constant.TurnKickClockWise[Constant.AACmd:Constant.AAHeadType]
        setLocaliseTimer(0,2)
##~         FWHead.compulsoryAction = FWHead.fixHead
##~         FWHead.fixHeadAt = (0,0,0,Constant.HTAbs_h)
        
        if debug:
            print "Edge spinning dCLOCKWISE"
        
    elif dir == Constant.dANTICLOCKWISE:
    
        Global.finalAction[Constant.AACmd:Constant.AAHeadType] = \
                Constant.TurnKickCounterClockWise[Constant.AACmd:Constant.AAHeadType]
        setLocaliseTimer(0,2)
##~         FWHead.compulsoryAction = FWHead.fixHead
##~         FWHead.fixHeadAt = (0,0,0,Constant.HTAbs_h)
        
        if debug:
            print "Edge spinning dANTICLOCKWISE"
        
def setEdgeSpinState (angle, dir):

    global SpinAngleMem, SpinDirMem, AttackMode
    
    SpinAngleMem = angle
    SpinDirMem   = dir
    AttackMode   = Constant.FIELD_EDGE_SPIN
    
    

def StillEdgeSpinning():

    global AttackMode, SpinAngleMem, SpinDirMem

    debug = False
    
    if AttackMode == Constant.FIELD_EDGE_SPIN:
    
        if (abs(HelpMath.normalizeAngle_180(Global.selfLoc.getHeading()-SpinAngleMem)) < 20 or 
            Global.ballD > 30):
            AttackMode = Constant.HOVER_TO_BALL
            
            if debug:
                print "breakout the edge spin"
            
            return False
            
        else:
            edgeSpin(SpinDirMem)
            IndicatorAction.showFacePattern([1,0,1,1,3])
            return True
            
    return False



def IsAttackerStuck(limit):
    
    global stuckSanityCheckTimer
    global pastBallDistanceInfo
    
    debug = False
    
    (maxPWM, leg) = ReadyPlayer.getStuckInfo()
    
    if Global.lostBall > 0 or Global.ballD < 80:
        return None
    
    if debug:
        print ""
        print "BallD %.2f" % Global.ballD
        print "MaxPWM: %.2f   leg %d " % (maxPWM,leg)
        print "last (60) max pwm %.2f  leg %d" % Global.lastMaxPWMValues[-1]
        
    # Stuck for consecutive 10 frames to be considered, with visual ball considered in the sanity check
    
    if (maxPWM >= ((Global.lastMaxPWMValues[-1])[0] + limit) and 
        Kick.framesSinceLastKicking >= 150):
        
        if debug:
            print pastBallDistanceInfo
            
        if stuckSanityCheckTimer == NUM_STUCK_FRAME:
            if stuckSanityCheck():
                stuckSanityCheckTimer = 0
                pastBallDistanceInfo  = []
                return (maxPWM,leg)
            else:
                stuckSanityCheckTimer = 0
                pastBallDistanceInfo  = []
                return None
        else:
            pastBallDistanceInfo.append(Global.ballD)
            stuckSanityCheckTimer += 1
            return None
            
    else:
        pastBallDistanceInfo  = []
        stuckSanityCheckTimer = 0
        return None    


def chooseStuckResolution(startPan):
    
    global panx, panDirection
    global stuckDetectTimer
    
    panx             = startPan
    stuckDetectTimer = 0
    if stuckDetect[1] == 1 or stuckDetect[1] == 3:
        panDirection = Constant.dCLOCKWISE
    else:
        panDirection = Constant.dANTICLOCKWISE
    
    StuckResolution()
    
def StuckResolution ():
        
    global stuckDetectTimer
    
    debug = False
    
    irdist = HelpGlobal.getAnySensor(Constant.ssINFRARED_NEAR)
    HelpGlobal.setDefaultAction()
    scanForObstacle(10,-10,-10)
    
    if debug:
        print " " , irdist
        print " " , FWHead.fixHeadAt[0]
    
    stuckDetectTimer += 1
    
    if stuckDetectTimer == 30:
        stuckDetectTimer = 0
    
    if irdist < 200000 and HelpTeam.canSeeOpponentWithinDist(50):
        closestOpp = HelpTeam.closestGpsOpponentWithinVar (50)
        if closestOpp[0] is not None:
            if closestOpp[0].getHeading() <= 0: # on the left
                WalkAction.setNormalWalk (-5,5,0)
            else:
                WalkAction.setNormalWalk (-5,-5,0)


def AttackerSlowDown(start_slow_down,stop_slow_down,max_forward,min_forward):

    if (Global.ballD < start_slow_down and abs(Global.ballH) < 50):
        numerator     = Global.ballD - stop_slow_down
        factor        = numerator / (start_slow_down - stop_slow_down)
        speed         = min_forward + (factor * max_forward)
        
        Global.finalAction[Constant.AACmd:Constant.AAHeadType] = \
            Constant.NormalWalkSlow[Constant.AACmd:Constant.AAHeadType]
        WalkAction.setForwardLeftTurn( speed, 0, HelpMath.CLIP(Global.ballH/1.5,15) )
        WalkAction.setForceStepComplete()



def stuckSanityCheck():

    i              = 0
    differences    = 0
    
    # if the robot is consatntly looking at the ball, and the ball distance is not decreasing, 
    # then is could be a stuck
    
    for i in range (NUM_STUCK_FRAME - 1):
        differences += pastBallDistanceInfo[i] - pastBallDistanceInfo[i+1]
        
    return (differences < 0)        


def BallUnderChinForUpenn():

    localBallX = Global.vBall.getVobDistance() * \
            math.sin(HelpMath.DEG2RAD(-Global.vBall.getHeading()))
    if (Global.ballD < 15 and abs(localBallX) < 3.5):
        return True

##~     if (Global.ballD < 15 and Global.vBall.getConfidence() > 0 and
##~         Global.degTilt < -10 and abs(Global.degPan) < 15 and Global.degCrane < -8):
##~         return True
        
    return False

#-----------------------------------------------
# Stuck resolution for the attacker

def scanForObstacle(speed,t,c):
    
    # use ir sensor (near) to scan for obstacle, check with visual and gps info
    global panDirection, panx
    
    if panx >= 80 and panDirection == Constant.dANTICLOCKWISE:
        panDirection = Constant.dCLOCKWISE
    elif panx <= -80 and panDirection == Constant.dCLOCKWISE:
        panDirection = Constant.dANTICLOCKWISE

    if panDirection == Constant.dCLOCKWISE:
        panx -= speed
    else:
        panx += speed

    FWHead.compulsoryAction = FWHead.fixHead
    FWHead.fixHeadAt = (panx,t,c,Constant.HTAbs_h)
