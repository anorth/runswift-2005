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
#  $Id: Forward.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2005 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# Forward.py
#
# Our rUNSWift 2005 Forward Player!!
#
# edited 6/1/2005 to work with new behaviours structure
#
#===============================================================================


import Kick
import Debug
import FWHead
import Global
import Constant
import HelpTrack
import SetPlayer
import ReadyPlayer
import BackOff
import Defender
import Supporter
import Striker
import Attacker
import LandmarkTester

import IndicatorAction
import HelpGlobal
import HelpTeam

import Edger

#--------------------------------------
# Hacked in 2005 to run with more organised library files (until this is replaced)


def playGameMode():
    resetEachFrame()    

    shouldIStartDefend = Defender.shouldIStartDefend()
    shouldIEndDefend   = Defender.shouldIEndDefend()
    shouldIStrike      = Striker.shouldIBecomeStriker()
    shouldIBackOff     = BackOff.shouldIVisualBackOff()
    shouldISupport     = Supporter.ShouldIBecomeSupporter()

    headFlag = True
    
    if Attacker.lockMode:
        print "Entering Lock Mode for Attacker!!!"
        Attacker.DecideNextAction()
        headFlag = False  # for grab/turn/kick
        IndicatorAction.showHeadColor((True, True, False))        
    
    elif Edger.lockMode:
        Edger.DecideNextAction()
        headFlag = False
        IndicatorAction.showHeadColor((False, True, False))
        
    elif ((shouldIStartDefend or Defender.lockDefender) and \
        not shouldIEndDefend):
        Global.myRole = Constant.DEFENDER
        Defender.DecideNextAction()
        IndicatorAction.showHeadColor((True, False, False))
        
    elif shouldIStrike:
        Striker.DecideNextAction()
        IndicatorAction.showHeadColor((False, False, True))        
        
    elif shouldIBackOff:
        BackOff.DecideNextAction()
        IndicatorAction.showHeadColor((False, True, True))
        
    elif shouldISupport:
        Supporter.DecideNextAction()
        IndicatorAction.showHeadColor((True, False, True))

    elif Edger.shouldIBecomeEdger():
        Edger.DecideNextAction()
        headFlag = False
        IndicatorAction.showHeadColor((False, True, False))
        
    else:    
        Attacker.DecideNextAction()
        IndicatorAction.showHeadColor((True, True, False))
        
        if BackOff.shouldISideBackOff():
            Global.myRole = Constant.SIDEBACKOFF
            Global.finalAction[Constant.AAForward] = \
                Global.finalAction[Constant.AAForward] * 0.6
            IndicatorAction.showHeadColor((False, True, True))


    if headFlag:            
        FWHead.DecideNextAction()
        
    if Kick.shouldIContinueKick():
        Kick.continueKick()
        
#--------------------------------------
# Act according to different states.

def DecideNextAction():
    if (Global.debugBehaviour_line):
        print "..... new frame: ", Global.frame

    # If first call to ready state, reset the readyplayer's globals
    if (HelpGlobal.getCurrentMode() == Constant.READYSTATE and
        Global.state != Constant.READYSTATE):
        ReadyPlayer.initReadyMatchPlaying()

    Global.frameReset()

    # Ted: In python only three states are supported. Other states are in still
    #          in C++. Why? If I port all the states to python then none of the C++
    #          behaviour would work. I think Kim still need his kimForward.cc
    #          and walkingLearner.cc.
    
    # Finished state - the robot can't move at all.
    if Global.state == Constant.FINISHED:
        IndicatorAction.showBatteryStatus()
        HelpGlobal.resetEverything()

    # Ready state - let me move to my correct position.
    elif Global.state == Constant.READYSTATE or Debug.mustReady:
        ReadyPlayer.readyMatchPlaying()
        Defender.lockDefender = False # shouldn't keep birding on new kickoff

    # Set state - I can't move my legs, but I can move heads to look for ball.
    elif Global.state == Constant.SETSTATE:
        SetPlayer.DecideNextAction()
        Defender.lockDefender = False # shouldn't keep birding on new kickoff

    # Playing state - lets play a game.
    else:
        playGameMode()

    # Show debug information using head lights.
    # If landmarkDebugging is on, no more face pattern for skills will be shown.
    # superDebug will be run inside it also, so don't need to call twice.
    landmarkDebugging = False
    if (landmarkDebugging):
        LandmarkTester.DecideNextAction()
    else:
        pass #IndicatorAction.superDebug()

    # Send wireless info to the teammates.
    HelpTeam.sendWirelessInfo()
    
#--------------------------------------
# General Initialisation
def resetEachFrame():
    Global.DKD = HelpTrack.getDKDRange()
    
    if Global.state != Constant.FINISHED:
        Global.myRole  = Constant.NOROLE
        
    HelpGlobal.setDefaultAction()
    IndicatorAction.setDefaultIndicators()
    FWHead.resetEachFrame()
