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


import Constant
import VisionLink

# Indices for Action.finalValues , depends on PyEmbed.cc::sendAtomicAction()
Command        = 0
WalkType       = 1
MinorWalkType  = 2
Forward        = 3
Left           = 4
TurnCCW        = 5
ForwardStep    = 6
LeftStep       = 7
TurnCCWStep    = 8
HeadType       = 9
Panx           = 10
Tilty          = 11
Cranez         = 12



# Enums for Action.finalValues[Headtype], read actuatorControl/HeadMotion.cc for details.
HTNull_h       = 0
HTRel_h        = 1
HTAbs_h        = 2
HTAbs_pt       = 3
HTAbs_xyz      = 4
HTAbs_xyz_hack = 5
HTRel_z        = 6

# Enums for Action.finalValues[Command], as defined in share/ActionProDef.h
# For the first element of finalAction to use.
cmdStop                  = 0
cmdStill                 = 1
cmdTailUp                = 2
cmdParaWalk              = 3
cmdRelaxed               = 4
cmdTurnOnJointDebug      = 5
cmdSetJointSpeedLimit    = 6
cmdSetAJointGain         = 7
cmdSetHighGain           = 8
cmdSetLowGain            = 9
cmdReloadSpecialAction   = 10
cmdForceStepComplete   = 11

# Enum for Action.finalValues[WalkType], from share/PWalkDef.h
# Don't forget to add the walkTypeToStr in the share/commonData.cc
NormalWalkWT            = 0
CanterWalkWT            = 1
ZoidalWalkWT            = 2
OffsetWalkWT            = 3 
EllipticalWalkWT        = 4 
AlanWalkWT              = 5
ChestPushWT             = 6
FwdFastWT               = 7
FwdHardWT               = 8
GetupFrontWT            = 9
GetupSideWT             = 10
GetupRearWT             = 11
BackKickWT              = 12
HoldBallWT              = 13
CheerWT                 = 14
DanceWT                 = 15
UpennRightWT            = 16
UpennLeftWT             = 17 # Upleft does NOT mean the ball travels left
                             # (in fact it travels right)
                             # It means which hand
BackWalkWT              = 18
SkiWalkWT               = 19
SkellipticalWalkWT      = 20
DiveKickWT              = 21
HandKickRightWT         = 22
HandKickLeftWT          = 23
BlockWT                 = 24
BlockLeftWT             = 25
BlockRightWT            = 26
HeadLeftWT              = 27
HeadRightWT             = 28
GTChestPushWT           = 29
NUFwdKickWT             = 30
BuBuFwdKickWT           = 31
HeadTapWT               = 32
SoftTapWT               = 33
GrabWT                  = 34
TestKickWT              = 35


# Minor Walk Type Indentifiers
DefaultMWT  = 0 
RawCommandMWT = 1

# minor walk type for skelliptical walk
# refer to actuatorControl/SkelliticalWalk.h
SkePG22MWT            = 2
SkeFastForwardMWT     = 3
SkeGrabDribbleMWT     = 4
SkePG31MWT            = 5
SkeTurnWalkMWT        = 6
SkeNeckTurnWalkMWT    = 7
SkeGrabDribbleHoldMWT = 8

# minor walk type for normal walk
NorGrabDribbleMWT     = 2
NorGrabForwardOnlyMWT = 3
NorGrabSideOnlyMWT    = 4
NorGrabTurnOnlyMWT    = 5
NorTurnKickMWT        = 6

# general minor walk type

GrabDribbleMWT     = SkeGrabDribbleMWT
GrabForwardOnlyMWT = SkeGrabDribbleMWT #NorGrabForwardOnlyMWT
GrabSideOnlyMWT    = SkeGrabDribbleMWT #NorGrabSideOnlyMWT
GrabTurnOnlyMWT    = SkeGrabDribbleMWT
TurnKickMWT        = NorTurnKickMWT



# Learning parameter indices for NormalWalkWT
Norm_pg  = 1
Norm_hf  = 2
Norm_hb  = 3
Norm_hdf = 4
Norm_hdb = 5
Norm_ffo = 6
Norm_fso = 7
Norm_bfo = 8
Norm_bso = 9
NormValues = [False] + range(Norm_bso)


# Learning parameters indicies for SkellipticalWalkWT 
# To be added.
Skel_halfStepTime      = 1
Skel_frontH            = 2
Skel_backH             = 3
Skel_frontFwdHeight    = 4
Skel_frontTrnLftHeight = 5
Skel_backFwdHeight     = 6
Skel_backTrnLftHeight  = 7
Skel_frontF            = 8
Skel_frontS            = 9
Skel_backF             = 10
Skel_backS             = 11
Skel_frontDutyCycle    = 12
Skel_backDutyCycle     = 13
Skel_frontLeadInFrac   = 14
Skel_frontLeadDownFrac = 15
Skel_frontLeadOutFrac  = 16
Skel_frontLeadUpFrac   = 17
Skel_backLeadInFrac    = 18
Skel_backLeadDownFrac  = 19
Skel_backLeadOutFrac   = 20
Skel_backLeadUpFrac    = 21
Skel_thrustHeight      = 22
Skel_canterHeight      = 23
Skel_sideOffset        = 24
Skel_turnCenterF       = 25
Skel_turnCenterL       = 26
SkelValues = [False] + range(Skel_turnCenterL)



# Walking Parameter Tuples
# WARNING: Any changes to these values will affect the low level GPS update
# in actuatorContol. (e.g. need to tune NormalWalk::getOdometry(),
# if any values changed for parameters with NormalWT walk type)
# The values in the tuples are:
#                     (walkType,          minorWalkType,PG,hF, hB,hdF,hdB,ffo,fso,bfo,bso)
#NormalWalk          = (NormalWalkWT,      DefaultMWT,   40,90,110, 20, 20, 59, 10,-50,  5)
#NormalWalkSlow      = (NormalWalkWT,      DefaultMWT,   40,70,100, 15, 20, 60, 10,-55,  5)
#NormalWalkSideWay   = (NormalWalkWT,      DefaultMWT,   32,95,110, 15, 25, 60,  5,-55,  5)
#NormalWalkSideWay   = (NormalWalkWT,      DefaultMWT,   35,90,110, 15, 20, 60,  5,-55,  5)
#NormalWalkShortStep = (NormalWalkWT,      DefaultMWT,   35,90,110, 10, 20, 65, 15,-40,  5)
#EllipticalWalk      = (EllipticalWalkWT,  DefaultMWT,   80,90,130, 10, 25, 45, 15,-65,  5)
#SkellipticalWalk    = (SkellipticalWalkWT,DefaultMWT,   31,80,115, 15, 30, 55, 15,-55,  5)
#SkellipticalWalk    = (SkellipticalWalkWT,DefaultMWT,   30,90,110, 10, 25, 45, 15,-65,  5)
# GrabTurnFast only tuned for turning, i.e. put forward&left = 0



# Placeholders for automatic clipping. Use these instead of those below.
MAX_FORWARD = Constant.LARGE_VAL
MAX_LEFT = Constant.LARGE_VAL
MAX_TURN = Constant.LARGE_VAL

# Maximum walk speeds. These are the requested speeds that give the actual
# maximum speed in the given axis (forward, left, turn) when used in isolation
# You should not use these outside this file. Use MAX_FORWARD/LEFT/TURN above.
MAX_FORWARD_NORMAL = 19
MAX_LEFT_NORMAL = 13
MAX_TURN_NORMAL = 80

MAX_FORWARD_SKE  = 35 # speed
MAX_BACKWARD_SKE = 18
MAX_LEFT_SKE     = 12
MAX_RIGHT_SKE    = 10
MAX_SIDE_SKE     = 10
MAX_TURNCCW_SKE  = 120
MAX_TURNCW_SKE   = 100
MAX_TURN_SKE     = 100



# Skelliptical speed constants

# Default Minor walk Type && Fast Forward Minor Walk Type
# cm/full-step
MAX_SKE_FWD_STP = MAX_SKE_FF_FWD_STP = 14.28        #15.087
MAX_SKE_BACK_STP = MAX_SKE_FF_BACK_STP = 10.2       #10.1
MAX_SKE_LEFT_STP = MAX_SKE_FF_LEFT_STP = 7.0        #8.78
MAX_SKE_RIGHT_STP = MAX_SKE_FF_RIGHT_STP = 6.86    #8.35
MAX_SKE_TURNCCW_STP = MAX_SKE_FF_TURNCCW_STP = 74.5 #85.0
MAX_SKE_TURNCW_STP = MAX_SKE_FF_TURNCW_STP = 77.7  #81.0
MAX_SKE_FF_COMBINED_TURNCCW_STP = 30.0
MAX_SKE_FF_COMBINED_TURNCW_STP = 30.0
SKE_PG = SKE_FF_PG = 22.0

MAX_SKE_FWD_SPD     = MAX_SKE_FWD_STP / (SKE_PG * 2.0 * 8.0 / 1000.0)
MAX_SKE_BACK_SPD    = MAX_SKE_BACK_STP / (SKE_PG * 2.0 * 8.0 / 1000.0)
MAX_SKE_LEFT_SPD    = MAX_SKE_LEFT_STP / (SKE_PG * 2.0 * 8.0 / 1000.0)
MAX_SKE_RIGHT_SPD   = MAX_SKE_RIGHT_STP / (SKE_PG * 2.0 * 8.0 / 1000.0)
MAX_SKE_TURNCCW_SPD = MAX_SKE_TURNCCW_STP / (SKE_PG * 2.0 * 8.0 / 1000.0)
MAX_SKE_TURNCW_SPD  = MAX_SKE_TURNCW_STP / (SKE_PG * 2.0 * 8.0 / 1000.0)

# Grab Dribble Minor Walk Type
# cm/full-step
MAX_SKE_GD_FWD_STP = 9.57
MAX_SKE_GD_BACK_STP = 0
MAX_SKE_GD_LEFT_STP = 6.1
MAX_SKE_GD_RIGHT_STP = 6.42
MAX_SKE_GD_TURNCCW_STP = 60.2
MAX_SKE_GD_TURNCW_STP = 63.8
SKE_GD_PG = 18.0


# PG22 Minor Walk Type
# cm/full-step
MAX_SKE_PG22_FWD_STP     = 11.27#8.5
MAX_SKE_PG22_BACK_STP    = 7.45#5
MAX_SKE_PG22_LEFT_STP    = 8.43#3
MAX_SKE_PG22_RIGHT_STP   = 8.26#3
MAX_SKE_PG22_TURNCCW_STP = 66.41#35
MAX_SKE_PG22_TURNCW_STP  = 64.94#29
SKE_PG22_PG = 22.0

# PG31 Minor Walk Type
# cm/full-step
MAX_SKE_PG31_FWD_STP     = 17#8.5
MAX_SKE_PG31_BACK_STP    = 17#5
MAX_SKE_PG31_LEFT_STP    = 8.48#3
MAX_SKE_PG31_RIGHT_STP   = 6.66#3
MAX_SKE_PG31_TURNCCW_STP = 70#35
MAX_SKE_PG31_TURNCW_STP  = 58#29
SKE_PG31_PG = 31.0



# stand still constants
StandStill = (cmdParaWalk,SkellipticalWalkWT,SkeFastForwardMWT,0,0,0,0,0,0,HTAbs_h,0,0,0)


import hMath
import Global
import Indicator

finalValues = list(StandStill)
lastValues = finalValues[:]

framesSinceLastKicking = 0

def frameReset():
    standStill()
    global framesSinceLastKicking
    if not shouldIContinueKick() and framesSinceLastKicking <= 50000:
        framesSinceLastKicking += 1


def framePostset():
    global lastValues
    lastValues = finalValues[:]
    

################################################################################
# These are all actions relating to the head
# - open/close mouth
# - head movement
################################################################################
# Opens the mouth to the specified angle (default all the way)
def openMouth(angle = Indicator.MOUTH_OPEN):
    Indicator.finalValues[Indicator.Mouth] = angle

# Closes the mouth
def closeMouth():
    Indicator.finalValues[Indicator.Mouth] = Indicator.MOUTH_CLOSED

    
# Sets head parameters according to the specified head type
def setHeadParams(panx, tilty, cranez, headType=HTAbs_h):
    global finalValues
    finalValues[HeadType] = headType
    finalValues[Panx]     = panx
    finalValues[Tilty]    = tilty
    finalValues[Cranez]   = cranez

def setHeadToLastPoint():
    global finalValues
    finalValues[HeadType] = lastValues[HeadType]
    finalValues[Panx] = lastValues[Panx]
    finalValues[Tilty] = lastValues[Tilty]
    finalValues[Cranez] = lastValues[Cranez]
    


################################################################################
# These are all actions relating to walking
################################################################################
# standing still, with the head pointing forward.
def standStill():
    global finalValues
    finalValues = list(StandStill)

# Stop walking, not controlling of 'Command' and the head params
def stopLegs():
    global finalValues
    finalValues[WalkType] = lastValues[WalkType]
    finalValues[MinorWalkType] = lastValues[MinorWalkType]
    finalValues[Forward:HeadType] = list(StandStill[Forward:HeadType])


def didIMove(speedThreshold=0.0, stepThreshold=0.0):
    global lastValues
    if lastValues[Forward] <= speedThreshold and \
           lastValues[Left] <= speedThreshold and \
           lastValues[TurnCCW] <= speedThreshold:
        return False
    if lastValues[ForwardStep] <= stepThreshold and \
           lastValues[LeftStep] <= stepThreshold and \
           lastValues[TurnCCWStep] <= stepThreshold:
        return False
    return True


# This function is used for walk calibration and learning.
# It always set the minor walk type to RawCommandMWT.
# The underlying actuatorControl walk modules
# will and should accept the fwd/lft/trn values as uncalibrated.
def rawWalk(fwd, left, turnCCW, walkType=SkellipticalWalkWT):
    global finalValues
    finalValues[ForwardStep] = fwd
    finalValues[LeftStep] = left
    finalValues[TurnCCWStep] = turnCCW
    finalValues[WalkType] = walkType
    finalValues[MinorWalkType] = RawCommandMWT
    

# Sets forward, sideways and turn parameters for walking. The parameters can
# each be either a speed, in which case the steps will be taken with a constant
# step timing, or maximum step sizes, in which case such steps will be taken at
# a possibly faster timing (pg). The cmdType parameter is a string of three
# characters that specify which type each of the first three parameters are;
# "s" for speed, "d" for distance/degrees.
# If you just want to walk as fast as possible use the
# Action.MAX_{FORWARD|LEFT|TURN} constants and the default cmdType. *Don't*
# clip values to what you think are good maximums unless you want to walk
# slowly - clipping is automatic.
#
# E.g.
# (MAX_FORWARD, 0, 5, "ssd") => Full speed forward, turn 5 degrees this step
# (3, 0, 0, "ssd") => 3cm/sec forward (shortens ground stroke of step)
# (3, 0, 0, "ddd") => 3cm steps, full speed forward (shortens step time (pg))
# (0, 0, MAX_TURN, "sss") => spin CCW
# (0, 0, 15, "ssd") => turn 15 degrees in one step
# (4, 3, 10, "ddd") => move fwd 4cm, left 3cm and turn 10 deg
def walk(fwd = 0, left = 0, turnCCW = 0, cmdType = "ssd", 
         walkType = SkellipticalWalkWT, minorWalkType = DefaultMWT):
    global finalValues

    # Use learning parameters if it is set.
    #if minorWalkType == LearningMWT:
    #    walkLearning(walkType)
        
    if walkType == None:
        walkType = SkellipticalWalkWT

    finalValues[WalkType] = walkType
    finalValues[MinorWalkType] = minorWalkType

    # Clip requested speeds to maximum
    if walkType == NormalWalkWT:
        finalValues[ForwardStep] = hMath.CLIP(fwd, MAX_FORWARD_NORMAL)
        finalValues[LeftStep]    = hMath.CLIP(left, MAX_LEFT_NORMAL)
        finalValues[TurnCCWStep] = hMath.CLIP(turnCCW, MAX_TURN_NORMAL)
    
    elif walkType == SkellipticalWalkWT:
        if minorWalkType == SkePG22MWT: 
            clipWalk(fwd,left,turnCCW,cmdType,SKE_PG22_PG,\
                     MAX_SKE_PG22_FWD_STP,MAX_SKE_PG22_BACK_STP,\
                     MAX_SKE_PG22_LEFT_STP,MAX_SKE_PG22_RIGHT_STP,\
                     MAX_SKE_PG22_TURNCCW_STP,MAX_SKE_PG22_TURNCW_STP)
	elif minorWalkType == SkeGrabDribbleMWT: 
            clipWalk(fwd,left,turnCCW,cmdType,SKE_GD_PG,
                     MAX_SKE_GD_FWD_STP,MAX_SKE_GD_BACK_STP,
                     MAX_SKE_GD_LEFT_STP,MAX_SKE_GD_RIGHT_STP,
                     MAX_SKE_GD_TURNCCW_STP,MAX_SKE_GD_TURNCW_STP) 
        elif minorWalkType == SkePG31MWT:
            clipWalk(fwd,left,turnCCW,cmdType,SKE_PG31_PG,\
                     MAX_SKE_PG31_FWD_STP,MAX_SKE_PG31_BACK_STP,\
                     MAX_SKE_PG31_LEFT_STP,MAX_SKE_PG31_RIGHT_STP,\
                     MAX_SKE_PG31_TURNCCW_STP,MAX_SKE_PG31_TURNCW_STP)            
        else: #minorWalkType == (SkeFastForwardMWT, DefaultMWT, SkeTurnWalkMWT, SkeNeckTurnWalkMWT)
            if abs(fwd) > MAX_SKE_FWD_SPD/2 or abs(left) > MAX_SKE_LEFT_SPD/2:
                clipWalk(fwd,left,turnCCW,cmdType,SKE_FF_PG,
	                 MAX_SKE_FF_FWD_STP,MAX_SKE_FF_BACK_STP,
                         MAX_SKE_FF_LEFT_STP,MAX_SKE_FF_RIGHT_STP,
                         MAX_SKE_FF_COMBINED_TURNCCW_STP,MAX_SKE_FF_COMBINED_TURNCW_STP)
            else:
                # Use turn only max step size, if we are not moving forward and left much
                clipWalk(fwd,left,turnCCW,cmdType,SKE_FF_PG,
	                 MAX_SKE_FF_FWD_STP,MAX_SKE_FF_BACK_STP,
                         MAX_SKE_FF_LEFT_STP,MAX_SKE_FF_RIGHT_STP,
                         MAX_SKE_FF_TURNCCW_STP,MAX_SKE_FF_TURNCW_STP)                     


def clipWalk(fwd,left,turnCCW,cmdType,pg,\
             maxFwdStp,maxBackStp,\
             maxLeftStp,maxRightStp,\
             maxTurnCCWStp,maxTurnCWStp): 
             
    rate = pg * 8.0 * 2.0 / 1000.0    
    maxFwdSpd = maxFwdStp / rate
    maxBackSpd = maxBackStp / rate
    maxLeftSpd = maxLeftStp / rate
    maxRightSpd = maxRightStp / rate
    maxTurnCCWSpd = maxTurnCCWStp / rate
    maxTurnCWSpd = maxTurnCWStp / rate
        
    # Forward
    if cmdType[0] == "s":
        if fwd >= 0:
            finalValues[Forward] = hMath.CLIP(fwd, maxFwdSpd)
            finalValues[ForwardStep] = maxFwdStp
        elif fwd < 0: 
            finalValues[Forward] = hMath.CLIP(fwd, maxBackSpd)
            finalValues[ForwardStep] = maxBackStp
    elif cmdType[0] == "d": 
        if fwd >= 0:
            finalValues[Forward] = maxFwdSpd
            finalValues[ForwardStep] = hMath.CLIP(fwd, maxFwdStp) 
        elif fwd < 0: 
            finalValues[Forward] = -maxBackSpd
            finalValues[ForwardStep] = hMath.CLIP(fwd, maxBackStp)
    else:
        print "ERROR: unknown command type to Action.walk:", cmdType
    # Left
    if cmdType[1] == "s":
        if left >= 0:
            finalValues[Left] = hMath.CLIP(left, maxLeftSpd) 
            finalValues[LeftStep] = maxLeftStp
        elif left < 0: 
            finalValues[Left] = hMath.CLIP(left, maxRightSpd)
            finalValues[LeftStep] = maxRightStp
    elif cmdType[1] == "d": 
        if left >= 0:
            finalValues[Left] = maxLeftSpd
            finalValues[LeftStep] = hMath.CLIP(left, maxLeftStp) 
        elif left < 0: 
            finalValues[Left] = -maxRightSpd
            finalValues[LeftStep] = hMath.CLIP(left, maxRightStp)
    else:
        print "ERROR: unknown command type to Action.walk:", cmdType
    # Turn CCW
    if cmdType[2] == "s":
        if turnCCW >= 0:
            finalValues[TurnCCW] = hMath.CLIP(turnCCW, maxTurnCCWSpd) 
            finalValues[TurnCCWStep] = maxTurnCCWStp
        elif turnCCW < 0: 
            finalValues[TurnCCW] = hMath.CLIP(turnCCW, maxTurnCWSpd)
            finalValues[TurnCCWStep] = maxTurnCWStp
    elif cmdType[2] == "d": 
        if turnCCW >= 0:
            finalValues[TurnCCW] = maxTurnCCWSpd
            finalValues[TurnCCWStep] = hMath.CLIP(turnCCW, maxTurnCCWStp) 
        elif turnCCW < 0: 
            finalValues[TurnCCW] = -maxTurnCWSpd
            finalValues[TurnCCWStep] = hMath.CLIP(turnCCW, maxTurnCWStp)
    else:
        print "ERROR: unknown command type to Action.walk:", cmdType


def setWalkParams(walkType,param,value): 
    if walkType == NormalWalkWT: 
        NormValues[param] = value
    elif walkType == SkellipticalWalkWT:
        SkelValues[param] = value 
        
    
def walkLearning(walkType): 
    if walkType == NormalWalkWT:
        learningParamsStr = "NOR "
        learningParams = NormValues    
    elif walkType == SkellipticalWalkWT:
        learningParamsStr = "SKE "
        learningParams = SkelValues 
    else: 
        print "Action.walkLearning : can't learn this walk type" 
        return
               
    for p in learningParams: 
        learningParamsStr += str(p) + " "
    VisionLink.setWalkLearningParameters(learningParamsStr) 

           
# Forces the current step to be completed
def forceStepComplete():
    global finalValues
    finalValues[Command] = cmdForceStepComplete


#def rotateAboutBall(dir):
#    walk(0, dir*(-4.5), dir*10)    



################################################################################
# These are all actions relating to kicking
################################################################################

# SkellipticalWalkWT means no kick is going on.
currentKick    = SkellipticalWalkWT
   
def kick(kickType):
    global finalValues, currentKick, framesSinceLastKicking
    framesSinceLastKicking = 0

    if Global.pWalkInfo.getCurrentWalkType() == kickType:
        currentKick = SkellipticalWalkWT
        stopLegs() # we don't want to stop the head
    else:
        finalValues[Command] = cmdParaWalk
        if kickType == None:
            print "Kick type is None, set to SkellipticalWalkWT"
            kickType = SkellipticalWalkWT
        currentKick = finalValues[WalkType] = kickType
    
    
    # This will not work!!!
    if kickType == DiveKickWT: # special action for dive kick
        openMouth()
    
#==================================
# Have i done the Kicks I wanted, if not, continue and return True.
def shouldIContinueKick():
    global currentKick
    return currentKick != SkellipticalWalkWT
    
    
#==================================
# Continue to do the desired kick.
def continueKick():
    global currentKick
    kick(currentKick)


# for offline testing
# to use, run 'python Action.py'
if __name__=="__main__":
    walk(10,0,0)
    print finalValues


## Test function to test walk
def DecideNextAction(): 
    walk(0,MAX_FORWARD,0,minorWalkType=SkePG22MWT)
