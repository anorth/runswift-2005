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
#  $Id: pSet.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# pSet.py
#
# Implementation for the set state.
#===============================================================================

#import Constant
import Global
import Indicator
import hTrack
import hFWHead
import VisionLink
import rSetPlay
import pReady
import sFindBall

# Global for rSetPlay to access to flag that a kickoff is about to happen. Set
# True every time pSet is active.
doKickOff = False

#-------------------------------------------------------------------------------
# sActiveLocalise + TrackVisual Ball if in Set mode.
def DecideNextAction():
    global doKickOff
    resetEachFrame()
    rSetPlay.resetPerform()

    Global.firstPlayFrame = Global.frame + 1

    #import Constant
    #print rSetPlay.getGapTo(Constant.MAX_GOALBOX_EDGE_X, Constant.TOP_GOALBOX_EDGE_Y)
    #print rSetPlay.getObstaclesToGoal()

    # Receiving team look at the ball
    # FIXME: look to front for obstacles too
    if VisionLink.getKickOffTeam() != VisionLink.getTeamColor():
        if pReady.isCentre(Global.kickOffPos): 
            hTrack.stationaryLocalise()
        else:
            hFWHead.DecideNextAction()
        
    # Kicker looks at ball (no localise). Others scan to see obstacles
    elif pReady.isCentre(Global.kickOffPos):
        sFindBall.perform(True)
    elif pReady.isForward(Global.kickOffPos):
        hTrack.stationaryLocalise(4, 30, -30)
    else:
        hTrack.stationaryLocalise()

    Indicator.showBatteryStatus()
    doKickOff = True

#-------------------------------------------------------------------------------
def resetEachFrame():
    hFWHead.resetEachFrame()
    hFWHead.minPosVariance  = 55
    hFWHead.minHeadVariance = 35
    hFWHead.focusTotalTime  = 20
