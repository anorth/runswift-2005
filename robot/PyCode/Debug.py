#
#   Copyright 2004 The University of New South Wales (UNSW) and National  
#   ICT Australia (NICTA).
#
#   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
#   redistribute it and/or modify it under the terms of the GNU General  
#   Public License as published by the Free Software Foundation; either  
#   version 2 of the License, or (at your option) any later version as  
#   modified below.  As the origidefenderTriggerDebugnal licensors, we add the following  
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
#  $Id: Debug.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




"""
Debugging module, all function that helps debugging:
    Profiling, Stack Tracing, Debugging flag turning on/off
    ...
Will be imported directly from Behaviour for easy access
TODO: please move all debugging stuff from Global to here.
"""

########################################################################
########################################################################
isGameRunning = True


# Debug booleans.
superDebug            = False
sideBackDebug         = False
matchVisualDebug      = False
attackBackOffDebug    = False
stealthTrigger        = False
roleIndicator         = False
stuckDebug            = False
hackStriker           = False
headDebug             = False
startReady            = False
mustReady             = False
directPosDebug        = False
readyDebug            = False
testKickOffPosition   = False
defenderTriggerDebug  = False
defenderDebug         = False
goalBoxDebug          = False
mustDefender          = False
defenderSuperDebug    = False
strikerTrigger        = False
stealthDebug          = False
goalieDebug           = False
goalieDefendDebug     = False
goalieHeadDebug       = False
readyStealthDebug     = False
handKickDebug         = False
doHeadKick            = False
useBallPosWhenKickOff = False
roleChange            = False
backOffTriggerDebug   = False
wingerDebug           = False
supporterDebug        = False
TurnAndSideMoveDebug  = False
grabbingDebug         = False
findballDebug         = False
trackVisualBallDebug  = False
getBehindBallDebug    = False
bGrabTurnKick         = False
dodgyDebug            = False
frameDebug            = False


# Switch booleans.

# Values for debugging.
debugForward = 0
debugLeft    = 0
debugTurnCCW = 0

# Switching booleans (note they are not same as debugging booleans)
turnOnStuckDetect = False
allowReadyMove    = True #the value is affecting ReadyPlayer.py

#########################################################

bDebugOn = False #Master Switch , for Debug module

stateStack = [] #Used in StateBasedAction, to store the state Stack
stateBasedDebugAll = False  #Print out everything in StateBasedAction
bDebugInOutState = False  #Print only In, Out state

DEBUGING_STATE = None # state name, like "OpenChallenger.ControllingBall"
                      # to be instrumented in StateBasedAction

debugMsg = ""


def showDebug(msg, appendText = False):
    global debugMsg
    if appendText:
        debugMsg += "\n" + str(msg)
    else:
        debugMsg = str(msg)
    
def goAndMakeTraceBack():
    zero    = 0
    zero[0] = 0

# takes any number of arguments, and does nothing, how is that? =)
def doNothing(*args):
    pass
