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


# Stuckness and contested ball detection and behaviours

import VisionLink
import Global
import Constant
import hMath
import Action

# This obstacle threshold is tested with gps obstacle excluding shared.
MIN_OBS_CONTESTED = 150

STUCK_FWD = 1
STUCK_FWD_LEFT = 2
STUCK_FWD_RIGHT = 3

gUseContestDetect = True
gUseObstacleDetect = True
gUseStuckDetect = True
gUsePawDetect = True

gLastContestedCalledFrame = 0
gLastObstacleFrame = 0 
gLastStuckFrame = 0
gLastPawFrame = 0

gObsToBall = 0
gObsBack = 0

gflp = 0    # front palm switchs
gfrp = 0
gLastflp = 0    # last frame triggered
gLastfrp = 0

gLastStuckForward = 0   # Frame last sensed stuck forward
gStuckType = None       # Stuck direction

# Sense things every frame
def frameReset():
    global gflp, gfrp, gLastflp, gLastfrp

    gflp = VisionLink.getSwitchSensorCount(Constant.SWITCH_FL_PALM)
    gfrp = VisionLink.getSwitchSensorCount(Constant.SWITCH_FR_PALM) 

    if gflp > 0:
        gLastflp = Global.frame
    if gfrp > 0:
        gLastfrp = Global.frame
    


# This function looks at three different values.
# 1. gps obstacles
# 2. pwm values
# 3. paw sensor values
# And returns true if a run for the ball is likely to be contested
def isBallContested(): 
    global gObsToBall, gObsBack
    
    global gLastObstacleFrame
    global gLastStuckFrame
    global gLastPawFrame
    
    global gLastContestedCalledFrame

    global gflp, gfrp, gLastflp, gLastfrp
    
    if Global.frame != gLastContestedCalledFrame:        
        x, y = hMath.getPointRelative(0,0,Global.ballH,Global.ballD+30)
        # Count obstacles in corridor between us and the ball
        gObsToBall = VisionLink.getNoObstacleBetween(0,0,int(x),int(y),
                                                  50,0,10,Constant.OBS_USE_GPS)
        # Count obstacles in corridor behind me (these hang around in GPS
        # for a while (we think))
        gObsBack = VisionLink.getNoObstacleBetween(0,-35,0,0,
                                                  50,0,10,Constant.OBS_USE_GPS)
        gLastContestedCalledFrame = Global.frame    
        
        if gUseObstacleDetect and gObsToBall + gObsBack > MIN_OBS_CONTESTED:         
            gLastObstacleFrame = Global.frame
        
                
        rlr = VisionLink.getLastMaxStepPWM(Constant.ssRL_ROTATOR)
        rrr = VisionLink.getLastMaxStepPWM(Constant.ssRR_ROTATOR)

        if gUseStuckDetect\
            and Action.finalValues[Action.Forward] > 0\
            and Action.finalValues[Action.ForwardStep] > 0\
            and (rlr[1] > 800 or rrr[1] > 800):
            gLastStuckFrame = Global.frame
            
        if gUsePawDetect and (gflp > 0 or gfrp > 0): 
            gLastPawFrame = Global.frame            
        

    return Global.frame - gLastObstacleFrame < 15\
        or Global.frame - gLastStuckFrame < 15\
        or Global.frame - gLastPawFrame < 15
    
# Returns true if we are stuck moving forward and takes a step backwards.
# This is sense with paw switches. If bothSidesOnly is True then this will
# only happen if both paw switches fire in quick succession.
# There is always a dodge for at least one step (8 vision frames); dodgeTime
# specifies dodge over and above this
def amIStuckForward(bothSidesOnly = False, dodgeTime = 0):
    global gLastStuckForward, gStuckType

    stuck = False

    # Not stuck right after booting
    if Global.frame < 60:
        return False

    # Stay stuck for a bit
    if Global.frame - gLastStuckForward < dodgeTime:
        stuck = True
    
    # Detetct stuck
    if Global.frame - gLastflp < 8 and Global.frame - gLastfrp < 8:
        print "Both paw switches firing. Backing off."
        gLastStuckForward = Global.frame
        gStuckType = STUCK_FWD
        stuck = True
    elif not bothSidesOnly:
        if Global.frame - gLastflp < 12:
            print "Left paw switch firing. Backing off right."
            gLastStuckForward = Global.frame
            gStuckType = STUCK_FWD_LEFT
            stuck = True
        elif Global.frame - gLastfrp < 12:
            print "Right paw switch firing. Backing off left."
            gLastStuckForward = Global.frame
            gStuckType = STUCK_FWD_RIGHT
            stuck = True

    if stuck:
        if gStuckType == STUCK_FWD:
            Action.walk(-Action.MAX_FORWARD, 0, 0,\
                        minorWalkType = Action.SkeFastForwardMWT)
        elif gStuckType == STUCK_FWD_LEFT:
            Action.walk(-Action.MAX_FORWARD, -Action.MAX_LEFT, 0,\
                        minorWalkType = Action.SkeFastForwardMWT)
        elif gStuckType == STUCK_FWD_RIGHT:
            Action.walk(-Action.MAX_FORWARD, Action.MAX_LEFT, 0,\
                        minorWalkType = Action.SkeFastForwardMWT)

    return stuck
