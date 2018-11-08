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
import Global
import VisionLink
import math

import hMath
import Action
import Indicator
import hTrack


CHECK_BEACONS = [2, 3, 4, 5] # These are the GPS coordinate array indices of
                             # the beacons to check for active localising.
                             # Note: they're not vob indices.


FOCUS_ON_BEACON_DEFAULT     = 10 # frames
DISALLOWLOCALISE_PERIOD     = 30
MIN_POS_VAR_DEFAULT         = 55
MIN_HEAD_VAR_DEFAULT        = 25


isLocalising = False
focusDuration           = 0
timerSinceLastLocalise  = 0

localiseBeacon = 0

def reset():
    global isLocalising, timerSinceLastLocalise
    isLocalising            = False
    timerSinceLastLocalise  = 0
    Indicator.showRedEye(False)    


# should ONLY be called when seeing the ball
def DecideNextAction():
    global isLocalising, focusDuration, timerSinceLastLocalise
    #print "islocalising: ", isLocalising, "  duration", focusDuration   

    if isLocalising and focusDuration < FOCUS_ON_BEACON_DEFAULT:
        sActiveLocalise()
        focusDuration += 1
        Indicator.showRedEye(True)
        Global.lostBall = hMath.DECREMENT(Global.lostBall)
        
    #---------------------------------------------------------------------------
    # Reset variables if finished focusing on beacon.
    elif isLocalising and focusDuration >= FOCUS_ON_BEACON_DEFAULT:
        reset()

    elif hTrack.canSeeBall() and Global.ballD < 30:
        reset()
        return

    #---------------------------------------------------------------------------
    # Trigger the active localise only if
    # - Have been quite a while since localising last time. AND
    # - a) Dog not sure where it is, or where it is heading, OR
    #   b) Haven't done it for a while (var is small doesn't mean the position is defiinitely correct)
    elif ((timerSinceLastLocalise > DISALLOWLOCALISE_PERIOD)  and
            not (hTrack.canSeeBall() and Global.ballD <= 30)
         and (
                Global.selfLoc.getPosVar()     > hMath.get95var(MIN_POS_VAR_DEFAULT)
                or Global.selfLoc.getHeadingVar() > hMath.get95var(MIN_HEAD_VAR_DEFAULT)
             )
        ):

        focusDuration   = 0     # Increase from this moment on.
        isLocalising    = True
        SmartSetBeacon(90,500)
        sActiveLocalise()
    else:
        timerSinceLastLocalise += 1
        Indicator.showRedEye(False)


def sActiveLocalise():
    global localiseBeacon  

    # Get shared variables from elsewhere.
    beaconPos   = VisionLink.getGPSCoordArray()
    selfLoc     = Global.selfLoc

    if localiseBeacon == 0:
        print "ERROR (sActiveLocalise.DecideNextAction): localiseBeacon = 0"
        return

    print "sWLocalise.sActiveLocalise() at beacon: ", localiseBeacon

    relPos      = map(lambda b,s:b-s, beaconPos[localiseBeacon], selfLoc.getPos())
    relPos      = hMath.rotate(relPos, 90 - selfLoc.getHeading())
    targetX     = -relPos[0]
    targetY     = relPos[1]

    Action.setHeadParams(targetX, Constant.BEACON_HEIGHT, targetY, Action.HTAbs_xyz)
    
        
def SmartSetBeacon(panLimit = 90, distLimit = 550):
    global localiseBeacon

    # Get shared variables from elsewhere.
    selfLoc = Global.selfLoc
    cov     = VisionLink.getGPSSelfCovariance()
    
    # Calculate the direction of the larger axis of the self covariance
    b = -(cov[0][0] + cov[1][1])
    c = cov[0][0] * cov[1][1] - cov[1][0] * cov[1][0]
    det = b * b - 4 * c
    
    if (det < 0.00001):
        sol = -b / 2
    else:
        sol = (-b + math.sqrt(det)) / 2
    
    if abs(cov[0][0] - sol) < 0.01 and cov[0][1] < 0.01:    
        # If self position variance is circular, all beacons are equally good
        # to localise off, and so choose the one closest to current heading.
        if abs(cov[1][1] - sol) < 0.01:
            head = selfLoc.getHeading()
        else:
            head = hMath.RAD2DEG(math.atan2(-cov[1][0], cov[1][1] - sol))           
    else:
        head = hMath.RAD2DEG(math.atan2(sol - cov[0][0], cov[0][1]))
            
    # Find the beacon that is closest to the variance direction.
    beaconPos = VisionLink.getGPSCoordArray()
    curCompHead = 360
    localiseBeacon = 0    
    
    for beacon in CHECK_BEACONS:
        relPos = map(lambda b,s:b-s, beaconPos[beacon], selfLoc.getPos())
        beaconHead = hMath.RAD2DEG(math.atan2(relPos[1], relPos[0]))
        beaconDist = hMath.getLength(relPos)
        localHead  = hMath.normalizeAngle_180(beaconHead - selfLoc.getHeading())
    
        if abs(localHead) < panLimit and beaconDist < distLimit:
            compHead = abs(hMath.normalizeAngle_180(head - beaconHead))
            if compHead > 90:
                compHead = 180 - compHead
            
            if compHead < curCompHead:
                curCompHead  = compHead
                localiseBeacon = beacon
