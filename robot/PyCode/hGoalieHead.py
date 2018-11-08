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
#  $Id: hGoalieHead.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# hGoalieHead.py
#                                                 
# This module controls the goal keeper's head movement.
#
#===============================================================================

import Global
import Action
import hMath
import hTrack
import sActiveLocalise
import sFindBall
import Constant
import sBlock


goalieHeadDebug = False

#===============================================================================
# Global variables of this module.
#===============================================================================

ORIENT_TIME = 120               # How long goalie localises for at start
BEACON_LOOK_TIME = 9            # Amount of time spent active localising
MIN_BEACON_LOOK_INTERVAL = 50   # Min. time between active localises
MAX_BEACON_LOOK_INTERVAL = 95  # Max. time between active localises
MIN_SELF_95_VAR = 45            # Min. position confidence to localise
MIN_BALL_DISTANCE = 50          # Min. ball distance needed to localise
MAX_BALL_DISTANCE = 250         # Max. ball distance after which stationary localise
HEAD_SEARCH_PERIOD = 70         # Number of frames for one head search cycle

headMode = None             # Can be either Start, Localise or BallTrack
localiseTimer = 0           # Determines when to switch modes
ballSearchTimer = 0         # Timer for ball search function
ballSeenCounter = 0

gbLocalisePt = 0            # which of the 2 corners to look at

#===============================================================================
# Functions belonging to this module.
#===============================================================================
# Sets the goalie's head angles
def DecideNextAction(trackBall = False):
    global headMode
    if headMode == None:
        headMode = start
    elif trackBall:       # Don't localise at all when attacking/blocking etc
        headMode = ballTrack
    # Execute current head movement code.

    #headMode functions should return true if they
    #are localising in some form (excepting start())
    return headMode()

#-------------------------------------------------------------------------------
# Makes the goalie do a stationary localise
def start():
    global headMode, localiseTimer
    hTrack.stationaryLocalise()    
    localiseTimer += 1
    if localiseTimer > ORIENT_TIME:
        headMode = ballTrack
        localiseTimer = 0
        
    return False

    
#-------------------------------------------------------------------------------
# Makes the goalie look directly at the ball.
def ballTrack():
    global headMode, localiseTimer
    global ballSeenCounter, ballSearchTimer

    if goalieHeadDebug:
        print "ballTrack():",

    sFindBall.perform(True)

    # Occasionally localise
    if localiseTimer <= 500:
        localiseTimer += 1
        
    if shouldLocalise():
        #headMode = goalBoxLocalise
        headMode = localise
        localiseTimer = 0
        
    return False

#-------------------------------------------------------------------------------
# Determines if the goalie should stationary localise
def shouldStatLocalise():

    #don't stationary localise if looking at beacon
    if headMode == localise:
        return False
    
    #stationary localise if ball is over max dist away
    if Global.ballD > MAX_BALL_DISTANCE:
        return True
        
    return False
    

#-------------------------------------------------------------------------------
# Determines if the goalie needs to look for a beacon or not
def shouldLocalise():
    global localiseTimer

    #(dx, dy) = pGoalie.getBallV()


    # Don't localise if the ball is close
    if Global.ballD < MIN_BALL_DISTANCE:
        return False
    
    # ... or is coming towards you, soon
    elif sBlock.isBallComingSoon:
        return False
    
    # causes yukky error with new policy of
    # goalie not moving when localising
    # if very lost
    #if  Global.selfLoc.getPosVar() > 2000:
    #    return True

    # Always localise at least this often since we may be wrong in our
    # strong belief
    if localiseTimer > MAX_BEACON_LOOK_INTERVAL:
        return True
    # Otherwise localise at intervals only if unsure of self position.
    elif localiseTimer < MIN_BEACON_LOOK_INTERVAL or \
           Global.selfLoc.getPosVar() <= hMath.get95var(MIN_SELF_95_VAR):
        return False

    return False

    
#-------------------------------------------------------------------------------
# Makes the goalie stationary localise
def statLocalise():
    global headMode
    # always look down first
    hTrack.spinningLocalise(clockwise = False)
    hTrack.panDirction = Constant.dCLOCKWISE
    
    if not(shouldStatLocalise()):
        headMode = ballTrack
        
    return True

#-------------------------------------------------------------------------------
# Makes the goalie look towards a beacon for a short amount of time
def localise():
    global headMode, localiseTimer

    if localiseTimer == 0:
        sActiveLocalise.SmartSetBeacon(80)

    sActiveLocalise.DecideNextAction()
    
    # If we have spent long enough localising, switch back to ball tracking
    localiseTimer += 1
    if localiseTimer > BEACON_LOOK_TIME:
        headMode = ballTrack
        localiseTimer = 0
        
    return True



# Look at the corners of the goal box to try and localise
# This is no longer relevant in 2005 - from the usual goalie position looking
# at the beacons puts the goalbox corner in view anyway
def goalBoxLocalise():
    global headMode, localiseTimer, gbLocalisePt
    # If we have spent long enough localising, switch back to ball tracking for
    # NEXT frame
    localiseTimer += 1
    if localiseTimer > BEACON_LOOK_TIME:
        headMode = ballTrack
        localiseTimer = 0
        gbLocalisePt = (gbLocalisePt + 1) % 2
    
    if gbLocalisePt == 0:
        Action.setHeadParams(-60, 30, -30, Action.HTAbs_h)
    elif gbLocalisePt == 1:
        Action.setHeadParams(60, 30, -30, Action.HTAbs_h)
        
    return True
