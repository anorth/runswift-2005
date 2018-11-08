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
#  $Id: hFWHead.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2005 UNSW
#  All Rights Reserved.
# 

#===============================================================================
#   Python Behaviours : 2005 (c) 
#
# hFWHead.py
#
# This module controls the forward's head movement.
#
#===============================================================================
import VisionLink
import Debug
import Global
import Constant
import Indicator

import hMath
import hTrack

import sActiveLocalise
import sFindBall

headColor = Debug.doNothing

#===============================================================================
# Some globals that are needed
#===============================================================================

# constants for compulsoryAction
defaultAction       = 0
mustSeeBall         = 1   # Sometimes the head should always look at the ball
#mustLocalise        = 2    # Broken
mustSeeWirelessBall = 3
mustSeeGpsBall      = 4
mustStatLocalise    = 5
mustSeeLocalPoint   = 6   # Set pointX and pointY before using (in local coords)
localiseOnlyBallFar = 7
doNothing           = 8

FOCUS_ON_BEACON_DEFAULT     = 10 # frames
MIN_POS_VAR_DEFAULT         = 55
MIN_HEAD_VAR_DEFAULT        = 25
MIN_BEACON_LOOK_INTERVAL    = 45   # Min. time between active localises
MAX_BEACON_LOOK_INTERVAL    = 120  # Max. time between active localises

# The following can be set by outside modules, but they'll be (and should be)
# reset to default in resetEachFrame() for every frame.
minPosVariance          = MIN_POS_VAR_DEFAULT
minHeadVariance         = MIN_HEAD_VAR_DEFAULT
focusTotalTime          = FOCUS_ON_BEACON_DEFAULT
compulsoryAction        = defaultAction
localPointX             = 0 #should be changed before use
localPointY             = 0 #should be changed before use

# Local timers. Other modules should NOT touch these.
focusDuration           = 0
timerSinceLastLocalise  = 0
isLocalising            = False


#--------------------------------------
# These variables need to be reset in each frame.
def resetEachFrame():
    global minPosVariance, minHeadVariance
    global focusTotalTime
    global compulsoryAction
    global localPointX, localPointY
    
    minPosVariance   = MIN_POS_VAR_DEFAULT
    minHeadVariance  = MIN_HEAD_VAR_DEFAULT
    focusTotalTime   = FOCUS_ON_BEACON_DEFAULT
    compulsoryAction = defaultAction
    localPointX      = 0
    localPointY      = 0


#--------------------------------------
# Set the head parameters.
def DecideNextAction():
    global focusTotalTim 
    global focusDuration 
    global timerSinceLastLocalise 
    global isLocalising
    global minHeadVariance 
    global minPosVariance

    #---------------------------------------------------------------------------
    # Special cases.
    #---------------------------------------------------------------------------
    # Sometime players want to handle the head itself
    if compulsoryAction == doNothing:
        return

    # Sometimes players want to see the ball no matter what.
    if compulsoryAction == mustSeeBall:
        hTrack.trackVisualBall()
        return
    
    # Force to track wireless ball.    
    if compulsoryAction == mustSeeWirelessBall: 
        hTrack.trackWirelessBall()
        return
    
    # Force to track gps ball.
    if compulsoryAction == mustSeeGpsBall:
        hTrack.trackGpsBall()
        return
        
    # Force a beacon localisation - caller will set its own timers.
    # JOSH: WARNING: this does not appear to work. 
    # ALEX: it doesn't work because calling SmartSetBeacon every frame is wrong
    #if compulsoryAction == mustLocalise:
    #    isLocalising    = True
    #    focusDuration   = 0
    #    sActiveLocalise.SmartSetBeacon()
    #    sActiveLocalise.DecideNextAction()
    #    return

    # Force to stationary localise - head swiping
    if compulsoryAction == mustStatLocalise:
        hTrack.stationaryLocalise()
        return
        
    # Force to track a point given in local coords    
    if compulsoryAction == mustSeeLocalPoint:
        global localPointX, localPointY
        if localPointX == 0 and localPointY == 0:
            print "hFWHead: mustSeeLocalPoint: local point is (0,0), possibly uninitialised?"
        hTrack.trackLocalPoint(localPointX, localPointY)
        return
        

    #---------------------------------------------------------------------------
    # Default behaviour of the head.
    #---------------------------------------------------------------------------
    #
    # Notice that winger, supporter etc are using the default behaviour.
    headColor(Indicator.RGB_NONE)    
#    print "posVar: ", Global.selfLoc.getPosVar(), "headVar: ", Global.selfLoc.getHeadingVar()
    ballv = VisionLink.getGPSBallVInfo(Constant.CTLocal)
    #---------------------------------------------------------------------------
    # Already localising.
    if isLocalising:
        if focusDuration < focusTotalTime:
            sActiveLocalise.DecideNextAction()
            focusDuration += 1
            headColor(Indicator.RGB_ORANGE)
            # Counter-act the effect of increasing lostBall counter while
            # localising, if not have this, locateBall will be (inappropriate)
            # quickly triggereds.
            Global.lostBall = hMath.DECREMENT(Global.lostBall)
        else:
            # Reset variables if finished focusing on beacon.
            isLocalising            = False
            timerSinceLastLocalise  = 0
            
            # Track or find the ball
            sFindBall.perform(True)

    #---------------------------------------------------------------------------
    # Trigger the active localise only if
    #   a) Dog not sure where it is, or where it is heading, OR
    #   b) Haven't done it for a while (var is small doesn't mean the position
    #      is definitely correct)
    #   c) AND haven't just localised
    #   d) AND ball velocity is low
    #
    elif (Global.selfLoc.getPosVar() > hMath.get95var(minPosVariance)\
            or Global.selfLoc.getHeadingVar() > hMath.get95var(minHeadVariance)\
            or timerSinceLastLocalise > MAX_BEACON_LOOK_INTERVAL)\
        and not (Global.vBall.isVisible()\
            and Global.ballD <= 40)\
        and not (timerSinceLastLocalise < MIN_BEACON_LOOK_INTERVAL)\
        and ballv[2] < 2:

        focusDuration   = 0     # Increase from this moment on.
        isLocalising    = True
        sActiveLocalise.SmartSetBeacon()
        sActiveLocalise.DecideNextAction()
        
    # Look at the ball
    else:
        headColor(Indicator.RGB_GREEN)
        timerSinceLastLocalise += 1
        
        # Track or find the ball
        sFindBall.perform(True)

# set the local point used in mustSeeLocalPoint
def setLocalPoint(localX, localY):
    global localPointX, localPointY
    localPointX = localX
    localPointY = localY
